#ifndef XENIA_APU_COREAUDIO_COREAUDIO_AUDIO_DRIVER_H_
#define XENIA_APU_COREAUDIO_COREAUDIO_AUDIO_DRIVER_H_

#pragma once

#include <AudioUnit/AudioUnit.h>

#include "xenia/apu/audio_driver.h"
#include "xenia/apu/audio_system.h"

namespace xe {
namespace apu {
namespace coreaudio {

class AudioTimingController {
 public:
  AudioTimingController() = default;
  ~AudioTimingController() = default;
  
 private:
  // Add timing-related members as needed
};

class CoreAudioDriver : public AudioDriver {
 public:
  CoreAudioDriver(Memory* memory);
  ~CoreAudioDriver() override;

  bool Initialize() override;
  void Shutdown() override;
  void SubmitFrame(float* samples) override;
  
  // Add these three methods:
  void Pause() override;
  void Resume() override;
  void SetVolume(float volume) override;

  void SetAudioSystem(AudioSystem* system);

  double GetLatencyMs() const;

 private:
  static OSStatus RenderCallback(void* inRefCon,
                                 AudioUnitRenderActionFlags* ioActionFlags,
                                 const AudioTimeStamp* inTimeStamp,
                                 UInt32 inBusNumber, UInt32 inNumberFrames,
                                 AudioBufferList* ioData);

 private:
  AudioUnit audio_unit_ = nullptr;

  AudioSystem* audio_system_ = nullptr;

  AudioTimingController timing_;
};


}  // namespace coreaudio
}  // namespace apu
}  // namespace xe
#endif  // XENIA_APU_COREAUDIO_COREAUDIO_AUDIO_DRIVER_H_