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
  if (!kernel_state_) {
    XELOGW("CoreAudioAudioSystem::Initialize: kernel_state is null");
    // Continue anyway, but note this for debugging
  }
  AudioSystem::Initialize();
  XELOGI("CoreAudioAudioSystem initialized successfully");
}

X_STATUS CoreAudioAudioSystem::CreateDriver(size_t index,
                                            xe::threading::Semaphore* semaphore,
                                            AudioDriver** out_driver) {
  if (!out_driver) {
    XELOGE("CoreAudioAudioSystem::CreateDriver: out_driver is null");
    return X_STATUS_INVALID_PARAMETER;
  }

  if (!kernel_state_) {
    XELOGW(
        "CoreAudioAudioSystem::CreateDriver: kernel_state is null, using "
        "fallback");
    // Continue with fallback
  }

  assert_not_null(out_driver);

#if XE_PLATFORM_IOS
  if (index > 0) {
    static std::atomic<bool> logged_secondary{false};
    if (!logged_secondary.exchange(true)) {
      XELOGW("CoreAudioAudioSystem: secondary clients use silent fallback");
    }

    *out_driver = new SilentAudioDriver(semaphore);
    return X_STATUS_SUCCESS;
  }
#endif

  auto driver = std::make_unique<CoreAudioDriver>(memory());

  if (!driver->Initialize()) {
    driver->Shutdown();
    XELOGE(
        "CoreAudioAudioSystem::CreateDriver: CoreAudioDriver initialization "
        "failed");

#if XE_PLATFORM_IOS
    XELOGW("CoreAudioAudioSystem: init failed, using silent fallback");
    *out_driver = new SilentAudioDriver(semaphore);
    return X_STATUS_SUCCESS;
#else
    return X_STATUS_UNSUCCESSFUL;
#endif
  }

  *out_driver = driver.release();
  return X_STATUS_SUCCESS;
}

AudioDriver* CoreAudioAudioSystem::CreateDriver(
    xe::threading::Semaphore* semaphore, uint32_t frequency, uint32_t channels,
    bool need_format_conversion) {
  if (!kernel_state_) {
    XELOGW("CoreAudioAudioSystem::CreateDriver: kernel_state is null");
  }

  auto* driver = new CoreAudioDriver(memory());
  driver->SetAudioSystem(this);

  if (!driver->Initialize()) {
#if XE_PLATFORM_IOS
    XELOGE("CoreAudioAudioSystem: direct init failed, using silent fallback");
    delete driver;
    return new SilentAudioDriver(semaphore);
#else
    delete driver;
    return nullptr;
#endif
  }

  return driver;
}

void CoreAudioAudioSystem::DestroyDriver(AudioDriver* driver) {
  driver->Shutdown();
  delete driver;
}

}  // namespace coreaudio
}  // namespace apu
}  // namespace xe
