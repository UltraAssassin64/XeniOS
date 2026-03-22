#include "coreaudio_audio_driver.h"

#include <cstring>
#include <pthread.h>

#include "xenia/base/logging.h"

namespace xe {
namespace apu {
namespace coreaudio {

CoreAudioDriver::CoreAudioDriver(
    Memory* memory,
    xe::threading::Semaphore* semaphore)
    : memory_(memory), semaphore_(semaphore) {}

CoreAudioDriver::~CoreAudioDriver() {
  Shutdown();
}

bool CoreAudioDriver::Initialize() {

  AudioComponentDescription desc{};
  desc.componentType = kAudioUnitType_Output;
  desc.componentSubType = kAudioUnitSubType_RemoteIO;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;

  AudioComponent comp = AudioComponentFindNext(nullptr, &desc);

  if (!comp) {
    XELOGE("CoreAudio: RemoteIO not found");
    return false;
  }

  if (AudioComponentInstanceNew(comp, &audio_unit_) != noErr) {
    XELOGE("CoreAudio: Failed creating AudioUnit");
    return false;
  }

  AURenderCallbackStruct callback{};
  callback.inputProc = RenderCallback;
  callback.inputProcRefCon = this;

  AudioUnitSetProperty(
      audio_unit_,
      kAudioUnitProperty_SetRenderCallback,
      kAudioUnitScope_Input,
      0,
      &callback,
      sizeof(callback));

  AudioStreamBasicDescription format{};
  format.mSampleRate = 48000;
  format.mFormatID = kAudioFormatLinearPCM;
  format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
  format.mFramesPerPacket = 1;
  format.mChannelsPerFrame = 2;
  format.mBitsPerChannel = 32;
  format.mBytesPerFrame = sizeof(float) * 2;
  format.mBytesPerPacket = format.mBytesPerFrame;

  AudioUnitSetProperty(
      audio_unit_,
      kAudioUnitProperty_StreamFormat,
      kAudioUnitScope_Input,
      0,
      &format,
      sizeof(format));

  if (AudioUnitInitialize(audio_unit_) != noErr) {
    XELOGE("CoreAudio: initialization failed");
    return false;
  }

  AudioOutputUnitStart(audio_unit_);

  XELOGI("CoreAudio driver initialized");

  return true;
}

void CoreAudioDriver::Shutdown() {

  if (!audio_unit_) return;

  AudioOutputUnitStop(audio_unit_);
  AudioUnitUninitialize(audio_unit_);
  AudioComponentInstanceDispose(audio_unit_);

  audio_unit_ = nullptr;
}

void CoreAudioDriver::MixFrame(float* input, float* output) {

  for (size_t i = 0; i < 256; i++) {

    float L = input[i * 6 + 0];
    float R = input[i * 6 + 1];

    output[i * 2 + 0] = L;
    output[i * 2 + 1] = R;
  }
}

void CoreAudioDriver::SubmitFrame(float* samples) {

  float stereo[256 * 2];

  MixFrame(samples, stereo);

  ring_buffer_.Push(stereo, 256 * 2);

  if(!ring_buffer_.Push(stereo, 256 * 2)){
    XELOGW("CoreAudio: audio buffer overflow - dropping frame");
    return;
  }

  if (semaphore_) {
    semaphore_->Release(1, nullptr);
  }
}

void CoreAudioDriver::Pause() {

  if (audio_unit_) {
    AudioOutputUnitStop(audio_unit_);
  }
}

void CoreAudioDriver::Resume() {

  if (audio_unit_) {
    AudioOutputUnitStart(audio_unit_);
  }
}

void CoreAudioDriver::SetVolume(float volume) {
  volume_.store(volume);
}

OSStatus CoreAudioDriver::RenderCallback(
    void* inRefCon,
    AudioUnitRenderActionFlags* flags,
    const AudioTimeStamp* ts,
    UInt32 bus,
    UInt32 frames,
    AudioBufferList* data) {

  static bool priority_set = false;
  if (!priority_set) {
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
    priority_set = true;
  }

  auto* driver = reinterpret_cast<CoreAudioDriver*>(inRefCon);

  float* out = reinterpret_cast<float*>(data->mBuffers[0].mData);

  size_t samples_needed = frames * 2;

  size_t popped = driver->ring_buffer_.Pop(out, samples_needed);

  if (popped < samples_needed) {
    std::memset(out + popped, 0,
                (samples_needed - popped) * sizeof(float));

    if (++driver->starvation_count_ > 50) {
    driver->starvation_count_ = 0;
    }
  } else {
    driver->starvation_count_ = 0;
  }

  float volume = driver->volume_.load();

  if (volume != 1.0f) {

    for (size_t i = 0; i < samples_needed; i++) {
      out[i] *= volume;
    }
  }

  return noErr;
}

}
}
}