#include "coreaudio_audio_system.h"
#include "coreaudio_audio_driver.h"

#include "xenia/apu/audio_system.h"

namespace xe {
namespace apu {
namespace coreaudio {

CoreAudioAudioSystem::CoreAudioAudioSystem(cpu::Processor* processor)
    : AudioSystem(processor) {}

std::unique_ptr<AudioSystem> CoreAudioAudioSystem::Create(
    cpu::Processor* processor) {
  return std::make_unique<CoreAudioAudioSystem>(processor);
}

void CoreAudioAudioSystem::Initialize() {
  AudioSystem::Initialize();
}
AudioDriver* CoreAudioAudioSystem::CreateDriver(
    xe::threading::Semaphore* semaphore,
    uint32_t frequency,
    uint32_t channels,
    bool need_format_conversion) {

  auto driver = new CoreAudioDriver(memory(), semaphore);

  if (!driver->Initialize()) {
    delete driver;
    return nullptr;
  }

  return driver;
}

X_STATUS CoreAudioAudioSystem::CreateDriver(
    size_t index,
    xe::threading::Semaphore* semaphore,
    AudioDriver** out_driver) {

  auto driver = new CoreAudioDriver(memory(), semaphore);

  if (!driver->Initialize()) {
    delete driver;
    return X_STATUS_UNSUCCESSFUL;
  }

  *out_driver = driver;
  return X_STATUS_SUCCESS;
}

void CoreAudioAudioSystem::DestroyDriver(AudioDriver* driver) {

  driver->Shutdown();
  delete driver;
}

}
}
}