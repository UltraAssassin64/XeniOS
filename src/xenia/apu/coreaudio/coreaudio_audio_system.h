#pragma once

#include "xenia/apu/audio_system.h"

namespace xe {
namespace apu {
namespace coreaudio {

class CoreAudioAudioSystem : public AudioSystem {
 public:
  explicit CoreAudioAudioSystem(cpu::Processor* processor);
  ~CoreAudioAudioSystem() override;

  // Ensure kernel state is set before operations
  void Initialize() override;

  AudioDriver* CreateDriver(xe::threading::Semaphore* semaphore,
                            uint32_t frequency, uint32_t channels,
                            bool need_format_conversion) override;

  X_STATUS CreateDriver(size_t index, xe::threading::Semaphore* semaphore,
                        AudioDriver** out_driver) override;

  void DestroyDriver(AudioDriver* driver) override;
};

}  // namespace coreaudio
}  // namespace apu
}  // namespace xe
