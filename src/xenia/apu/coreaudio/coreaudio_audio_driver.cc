#include "xenia/apu/coreaudio_audio_driver.h"

#include <AudioToolbox/AudioToolbox.h>
#include <cstring>

namespace xe {
namespace apu {

CoreAudioDriver::CoreAudioDriver(Memory* memory)
    : AudioDriver(memory) {}

CoreAudioDriver::~CoreAudioDriver() {
  Shutdown();
}

bool CoreAudioDriver::Initialize() {
  timing_.Initialize(48000);
  timing_.SetTarget(2048, 4096);

  ring_buffer_.resize(16384);
  buffer_mask_ = ring_buffer_.size() - 1;

  AudioComponentDescription desc = {};
  desc.componentType = kAudioUnitType_Output;
  desc.componentSubType = kAudioUnitSubType_RemoteIO;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;

  AudioComponent comp = AudioComponentFindNext(nullptr, &desc);
  if (!comp) return false;

  AudioComponentInstanceNew(comp, &audio_unit_);

  AURenderCallbackStruct callback = {};
  callback.inputProc = RenderCallback;
  callback.inputProcRefCon = this;

  AudioUnitSetProperty(audio_unit_,
                       kAudioUnitProperty_SetRenderCallback,
                       kAudioUnitScope_Input,
                       0,
                       &callback,
                       sizeof(callback));

  AudioStreamBasicDescription fmt = {};
  fmt.mSampleRate = 48000;
  fmt.mFormatID = kAudioFormatLinearPCM;
  fmt.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
  fmt.mBitsPerChannel = 32;
  fmt.mChannelsPerFrame = 2;
  fmt.mFramesPerPacket = 1;
  fmt.mBytesPerFrame = 8;
  fmt.mBytesPerPacket = 8;

  AudioUnitSetProperty(audio_unit_,
                       kAudioUnitProperty_StreamFormat,
                       kAudioUnitScope_Input,
                       0,
                       &fmt,
                       sizeof(fmt));

  AudioUnitInitialize(audio_unit_);
  AudioOutputUnitStart(audio_unit_);

  return true;
}

void CoreAudioDriver::Shutdown() {
  if (audio_unit_) {
    AudioOutputUnitStop(audio_unit_);
    AudioUnitUninitialize(audio_unit_);
    AudioComponentInstanceDispose(audio_unit_);
    audio_unit_ = nullptr;
  }
}

void CoreAudioDriver::SubmitFrame(uint32_t frame_ptr) {
  float* samples =
      reinterpret_cast<float*>(memory()->Translate(frame_ptr));

  size_t write = write_pos_.load();

  for (int i = 0; i < 512 * 2; i++) {
    ring_buffer_[write & buffer_mask_] = samples[i];
    write++;
  }

  write_pos_.store(write);

  timing_.OnSubmit(512);
}

OSStatus CoreAudioDriver::RenderCallback(void* inRefCon,
                                         AudioUnitRenderActionFlags*,
                                         const AudioTimeStamp*,
                                         UInt32,
                                         UInt32 inNumberFrames,
                                         AudioBufferList* ioData) {
  auto* driver = reinterpret_cast<CoreAudioDriver*>(inRefCon);

  float* out = reinterpret_cast<float*>(ioData->mBuffers[0].mData);

  driver->FillAudio(out, inNumberFrames);

  return noErr;
}

void CoreAudioDriver::FillAudio(float* out, uint32_t frames) {
  size_t read = read_pos_.load();
  size_t write = write_pos_.load();

  size_t available = write - read;

  if (available < frames * 2) {
    std::memset(out, 0, frames * 2 * sizeof(float));
    return;
  }

  for (uint32_t i = 0; i < frames * 2; i++) {
    out[i] = ring_buffer_[read & buffer_mask_];
    read++;
  }

  read_pos_.store(read);

  timing_.OnConsume(frames);
}

double CoreAudioDriver::GetLatencyMs() const {
  return timing_.GetLatencyMs();
}

} // namespace apu
} // namespace xe