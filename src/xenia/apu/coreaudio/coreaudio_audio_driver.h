#pragma once

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

#include <atomic>

#include "coreaudio_ring_buffer.h"
#include "xenia/apu/audio_driver.h"
#include "xenia/base/threading.h"

namespace xe {
namespace apu {
namespace coreaudio {

class CoreAudioDriver : public AudioDriver {
 public:
  CoreAudioDriver(Memory* memory);
  ~CoreAudioDriver() override;

  bool Initialize() override;
  void Shutdown() override;
  void SubmitFrame(float* samples) override;
  void Pause() override;
  void Resume() override;
  void SetVolume(float volume) override;

  void SetAudioSystem(AudioSystem* system) { audio_system_ = system; }

  AudioSystem* audio_system() const { return audio_system_; }

  double GetLatencyMs() const;

 private:
  static OSStatus RenderCallback(void* inRefCon,
                                 AudioUnitRenderActionFlags* ioActionFlags,
                                 const AudioTimeStamp* inTimeStamp,
                                 UInt32 inBusNumber, UInt32 inNumberFrames,
                                 AudioBufferList* ioData);

 private:
  Memory* memory_ = nullptr;
  AudioUnit audio_unit_ = nullptr;
  AudioSystem* audio_system_ = nullptr;

};

}  // namespace coreaudio
}  // namespace apu
}  // namespace xe
