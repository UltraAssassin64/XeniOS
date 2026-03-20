#pragma once

#include <atomic>
#include <vector>

#include <AudioUnit/AudioUnit.h>

#include "xenia/apu/audio_driver.h"
#include "xenia/apu/audio_timing_controller.h"

namespace xe {
namespace apu {

class CoreAudioDriver : public AudioDriver {
 public:
  CoreAudioDriver(Memory* memory);
  ~CoreAudioDriver() override;

  bool Initialize() override;
  void Shutdown() override;

  void SubmitFrame(uint32_t frame_ptr) override;

  double GetLatencyMs() const;

  AudioTimingController& timing() { return timing_; }

 private:
  static OSStatus RenderCallback(void* inRefCon,
                                 AudioUnitRenderActionFlags* ioActionFlags,
                                 const AudioTimeStamp* inTimeStamp,
                                 UInt32 inBusNumber,
                                 UInt32 inNumberFrames,
                                 AudioBufferList* ioData);

  void FillAudio(float* out, uint32_t frames);

 private:
  AudioUnit audio_unit_;

  std::vector<float> ring_buffer_;
  std::atomic<size_t> write_pos_{0};
  std::atomic<size_t> read_pos_{0};
  size_t buffer_mask_ = 0;

  AudioTimingController timing_;
};

} // namespace apu
} // namespace xe