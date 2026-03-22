/**
 ******************************************************************************
 * UltraXeniOS : CoreAudio Audio System                                       *
 ******************************************************************************
 */

#include "xenia/apu/coreaudio/coreaudio_audio_system.h"

#include <atomic>

#include "xenia/apu/coreaudio/coreaudio_audio_driver.h"
#include "xenia/base/logging.h"
#include "xenia/base/threading.h"

namespace xe {
namespace apu {
namespace coreaudio {

namespace {

#if XE_PLATFORM_IOS
// Silent fallback (matches SDL behavior)
class SilentAudioDriver final : public AudioDriver {
 public:
  explicit SilentAudioDriver(xe::threading::Semaphore* semaphore)
      : semaphore_(semaphore) {}

  bool Initialize() override { return true; }
  void Shutdown() override {}

  void SubmitFrame(uint32_t frame_ptr) override {
    (void)frame_ptr;
    if (semaphore_) {
      semaphore_->Release(1, nullptr);
    }
  }

  void Pause() override {}
  void Resume() override {}
  void SetVolume(float volume) override { (void)volume; }

 private:
  xe::threading::Semaphore* semaphore_ = nullptr;
};
#endif  // XE_PLATFORM_IOS

}  // namespace

//------------------------------------------------------------------------------
// Factory
//------------------------------------------------------------------------------

std::unique_ptr<AudioSystem> CoreAudioAudioSystem::Create(
    cpu::Processor* processor) {
  return std::make_unique<CoreAudioAudioSystem>(processor);
}

//------------------------------------------------------------------------------
// Lifecycle
//------------------------------------------------------------------------------

CoreAudioAudioSystem::CoreAudioAudioSystem(cpu::Processor* processor)
    : AudioSystem(processor) {}

CoreAudioAudioSystem::~CoreAudioAudioSystem() = default;

void CoreAudioAudioSystem::Initialize() { AudioSystem::Initialize(); }

//------------------------------------------------------------------------------
// Driver Creation (multi-client path)
//------------------------------------------------------------------------------

X_STATUS CoreAudioAudioSystem::CreateDriver(size_t index,
                                            xe::threading::Semaphore* semaphore,
                                            AudioDriver** out_driver) {
  assert_not_null(out_driver);

#if XE_PLATFORM_IOS
  // Same logic as SDL: only one real device allowed
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

//------------------------------------------------------------------------------
// Driver Creation (direct path used by APU)
//------------------------------------------------------------------------------

AudioDriver* CoreAudioAudioSystem::CreateDriver(
    xe::threading::Semaphore* semaphore, uint32_t frequency, uint32_t channels,
    bool need_format_conversion) {
  auto* driver = new CoreAudioDriver(memory());
  driver->SetAudioSystem(this);

  if (!driver->Initialize()) {
#if XE_PLATFORM_IOS
    XELOGW("CoreAudioAudioSystem: direct init failed, silent fallback");
    return new SilentAudioDriver(semaphore);
#else
    delete driver;
    return nullptr;
#endif
  }

  return driver;
}
void CoreAudioAudioSystem::Pump(uint32_t frames, float* out_buffer) {
  if (frames > 4096) frames = 4096;

  uint32_t samples = frames * 2;

  auto* mix = reinterpret_cast<float*>(GetMixBuffer());

  std::memcpy(out_buffer, mix, samples * sizeof(float));
}
//------------------------------------------------------------------------------
// Driver Destruction
//------------------------------------------------------------------------------

void CoreAudioAudioSystem::DestroyDriver(AudioDriver* driver) {
  assert_not_null(driver);

#if XE_PLATFORM_IOS
  if (auto* silent = dynamic_cast<SilentAudioDriver*>(driver)) {
    silent->Shutdown();
    delete silent;
    return;
  }
#endif

  auto* core = dynamic_cast<CoreAudioDriver*>(driver);
  assert_not_null(core);

  core->Shutdown();
  delete core;
}

}  // namespace coreaudio
}  // namespace apu
}  // namespace xe
