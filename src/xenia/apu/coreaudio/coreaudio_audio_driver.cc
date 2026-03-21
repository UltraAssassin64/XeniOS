#include "xenia/apu/coreaudio/coreaudio_audio_driver.h"

#include "xenia/apu/audio_system.h"

#include <AudioToolbox/AudioToolbox.h>
#include <AVFoundation/AVFoundation.h>
#include <cstring>

namespace xe {
namespace apu {
namespace coreaudio {

CoreAudioDriver::CoreAudioDriver(Memory* memory)
    : AudioDriver(memory) {}

CoreAudioDriver::~CoreAudioDriver() {
  Shutdown();
}

void CoreAudioDriver::SetAudioSystem(AudioSystem* system) {
  audio_system_ = system;
}

bool CoreAudioDriver::Initialize() {

  AVAudioSession* session = [AVAudioSession sharedInstance];
  [session setCategory:AVAudioSessionCategoryPlayback error:nil];
  [session setPreferredSampleRate:48000 error:nil];
  [session setPreferredIOBufferDuration:2048.0 / 48000.0 error:nil];
  [session setActive:YES error:nil];

  timing_.Initialize(48000);
  timing_.SetTarget(2048, 8192);

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

  UInt32 buffer_size = 2048;

  AudioUnitSetProperty(audio_unit_,
                       kAudioDevicePropertyBufferFrameSize,
                       kAudioUnitScope_Global,
                       0,
                       &buffer_size,
                       sizeof(buffer_size));

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

OSStatus CoreAudioDriver::RenderCallback(void* inRefCon,
                                         AudioUnitRenderActionFlags*,
                                         const AudioTimeStamp*,
                                         UInt32,
                                         UInt32 inNumberFrames,
                                         AudioBufferList* ioData) {

  auto* driver = reinterpret_cast<CoreAudioDriver*>(inRefCon);

  float* out = reinterpret_cast<float*>(ioData->mBuffers[0].mData);

  if (driver->audio_system_) {
    driver->audio_system_->Pump(inNumberFrames, out);
  } else {
    std::memset(out, 0, inNumberFrames * 2 * sizeof(float));
  }

  return noErr;
}

double CoreAudioDriver::GetLatencyMs() const {
  return timing_.GetLatencyMs();
}

}  // namespace coreaudio
}  // namespace apu
}  // namespace xe