#pragma once

#include <memory>

#include "xenia/apu/audio_system.h"

namespace xe {
namespace apu {
namespace coreaudio {

class CoreAudioAudioSystem : public AudioSystem {
 public:
  static std::unique_ptr<AudioSystem> Create(cpu::Processor* processor);
  std::string name() const override { return "CoreAudio"; }

  explicit CoreAudioAudioSystem(cpu::Processor* processor);
  ~CoreAudioAudioSystem() override;

  static bool IsAvailable() { return true; }

  void Initialize() override;

  X_STATUS CreateDriver(size_t index,
                        xe::threading::Semaphore* semaphore,
                        AudioDriver** out_driver) override;

  AudioDriver* CreateDriver(xe::threading::Semaphore* semaphore,
                            uint32_t frequency, uint32_t channels,
                            bool need_format_conversion) override;

  void DestroyDriver(AudioDriver* driver) override;
};

}  // namespace coreaudio
}  // namespace apu
}  // namespace xe