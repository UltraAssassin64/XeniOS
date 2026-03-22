#pragma once

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

#include <atomic>

#include "xenia/apu/audio_driver.h"
#include "coreaudio_ring_buffer.h"
#include "xenia/base/threading.h"

namespace xe {
namespace apu {
namespace coreaudio {

class CoreAudioDriver : public AudioDriver {
 public:
  CoreAudioDriver(Memory* memory, xe::threading::Semaphore* semaphore);
  ~CoreAudioDriver() override;

  bool Initialize() override;
  void Shutdown() override;

  void SubmitFrame(float* samples) override;

  void Pause() override;
  void Resume() override;

  void SetVolume(float volume) override;

 private:
  static OSStatus RenderCallback(
      void* inRefCon,
      AudioUnitRenderActionFlags* ioActionFlags,
      const AudioTimeStamp* inTimeStamp,
      UInt32 inBusNumber,
      UInt32 inNumberFrames,
      AudioBufferList* ioData);

  void MixFrame(float* input, float* output);

 private:
  Memory* memory_;
  xe::threading::Semaphore* semaphore_;

  AudioUnit audio_unit_ = nullptr;

  RingBuffer ring_buffer_{48000 * 2};

  std::atomic<float> volume_{1.0f};

  std::atomic<int> starvation_count_{0};
};

}  
}  
}