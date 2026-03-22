#pragma once

#include "xenia/apu/audio_system.h"

namespace xe {
namespace apu {
namespace coreaudio {

class CoreAudioAudioSystem : public AudioSystem {
public:
  explicit CoreAudioAudioSystem(cpu::Processor* processor);

  static std::unique_ptr<AudioSystem> Create(cpu::Processor* processor);

  std::string name() const override { return "CoreAudio"; }

  void Initialize() override;

  X_STATUS CreateDriver(
      size_t index,
      xe::threading::Semaphore* semaphore,
      AudioDriver** out_driver) override;

  void DestroyDriver(AudioDriver* driver) override;
};

}
}
}