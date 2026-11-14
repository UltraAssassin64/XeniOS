/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/apu/sdl/sdl_audio_driver.h"

#include <cstring>

#include "xenia/apu/apu_flags.h"
#include "xenia/apu/conversion.h"
#include "xenia/base/assert.h"
#include "xenia/base/logging.h"
#include "xenia/base/profiling.h"
#include "xenia/helper/sdl/sdl_helper.h"

namespace xe {
namespace apu {
namespace sdl {

#if XE_PLATFORM_IOS
std::mutex SDLAudioDriver::shared_drivers_mutex_;
std::vector<SDLAudioDriver*> SDLAudioDriver::shared_drivers_;
#endif  // XE_PLATFORM_IOS

SDLAudioDriver::SDLAudioDriver(xe::threading::Semaphore* semaphore,
                               uint32_t frequency, uint32_t channels,
                               bool need_format_conversion)
    : semaphore_(semaphore),
      frame_frequency_(frequency),
      frame_channels_(channels),
      need_format_conversion_(need_format_conversion) {
  switch (frame_channels_) {
    case 6:
      channel_samples_ = 256;
      break;
    case 2:
      channel_samples_ = 768;
      break;
    default:
      assert_unhandled_case(frame_channels_);
  }
  frame_size_ = sizeof(float) * frame_channels_ * channel_samples_;
  assert_true(frame_size_ <= kFrameSizeMax);
  assert_true(!need_format_conversion_ || frame_channels_ == 6);
}

SDLAudioDriver::~SDLAudioDriver() {
  assert_true(frames_queued_.empty());
  assert_true(frames_unused_.empty());
};

bool SDLAudioDriver::Initialize() {
  SDL_version ver = {};
  SDL_GetVersion(&ver);
  if ((ver.major < 2) || (ver.major == 2 && ver.minor == 0 && ver.patch < 8)) {
    XELOGW(
        "SDL library version {}.{}.{} is outdated. "
        "You may experience choppy audio.",
        ver.major, ver.minor, ver.patch);
  }

  if (!xe::helper::sdl::SDLHelper::Prepare()) {
    return false;
  }
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
    XELOGE("SDL_InitSubSystem(SDL_INIT_AUDIO) failed: {}", SDL_GetError());
    return false;
  }
  sdl_initialized_ = true;

#if XE_PLATFORM_IOS
  std::lock_guard<std::mutex> shared_lock(shared_drivers_mutex_);
  if (!shared_drivers_.empty()) {
    // The device is already open for an earlier client; join its mix. Frames
    // are summed as-is, so only clients with the same layout can share it.
    if (frame_frequency_ != shared_drivers_.front()->frame_frequency_ ||
        frame_channels_ != shared_drivers_.front()->frame_channels_ ||
        need_format_conversion_ !=
            shared_drivers_.front()->need_format_conversion_) {
      XELOGE("SDLAudioDriver: shared iOS device has a different frame format");
      return false;
    }
    sdl_device_id_ = shared_drivers_.front()->sdl_device_id_;
    sdl_device_channels_ = shared_drivers_.front()->sdl_device_channels_;
    shared_drivers_.push_back(this);
    return true;
  }
#endif  // XE_PLATFORM_IOS

  SDL_AudioSpec desired_spec = {};
  SDL_AudioSpec obtained_spec;
  desired_spec.freq = frame_frequency_;
  desired_spec.format = AUDIO_F32;
  desired_spec.channels = frame_channels_;
  desired_spec.samples = channel_samples_;
  desired_spec.callback = SDLCallback;
  desired_spec.userdata = this;
  // Allow the hardware to decide between 5.1 and stereo,
  // unless the input is stereo
  int allowed_change =
      frame_channels_ != 2 ? SDL_AUDIO_ALLOW_CHANNELS_CHANGE : 0;
  for (int i = 0; i < 2; i++) {
    sdl_device_id_ = SDL_OpenAudioDevice(nullptr, 0, &desired_spec,
                                         &obtained_spec, allowed_change);
    if (sdl_device_id_ <= 0) {
      XELOGE(
          "SDL_OpenAudioDevice failed: {} (requested freq={}, channels={}, "
          "samples={}, allowed_change=0x{:X})",
          SDL_GetError(), desired_spec.freq, desired_spec.channels,
          desired_spec.samples, static_cast<uint32_t>(allowed_change));
      return false;
    }
    if (obtained_spec.channels == 2 || obtained_spec.channels == 6) {
      break;
    }
    // If the system is 4 or 7.1, let SDL convert
    allowed_change = 0;
    SDL_CloseAudioDevice(sdl_device_id_);
    sdl_device_id_ = -1;
  }
  if (sdl_device_id_ <= 0) {
    XELOGE("Failed to get a compatible SDL Audio Device. Last error: {}",
           SDL_GetError());
    return false;
  }
  sdl_device_channels_ = obtained_spec.channels;
#if XE_PLATFORM_IOS
  shared_drivers_.push_back(this);
#endif  // XE_PLATFORM_IOS

  SDL_PauseAudioDevice(sdl_device_id_, 0);

  return true;
}

void SDLAudioDriver::SubmitFrame(float* frame) {
  float* output_frame;
  {
    std::unique_lock<std::mutex> guard(frames_mutex_);
    if (frames_unused_.empty()) {
      output_frame = new float[frame_channels_ * channel_samples_];
    } else {
      output_frame = frames_unused_.top();
      frames_unused_.pop();
    }
  }

  std::memcpy(output_frame, frame, frame_size_);

  {
    std::unique_lock<std::mutex> guard(frames_mutex_);
    frames_queued_.push(output_frame);
  }
}

void SDLAudioDriver::Pause() { SDL_PauseAudioDevice(sdl_device_id_, 1); }

void SDLAudioDriver::Resume() { SDL_PauseAudioDevice(sdl_device_id_, 0); }

void SDLAudioDriver::Shutdown() {
#if XE_PLATFORM_IOS
  {
    // Leave the mix. Only the last client closes the shared device, and it
    // does so after dropping the lock because the close waits for a running
    // SDLCallback, which takes the same lock.
    std::lock_guard<std::mutex> shared_lock(shared_drivers_mutex_);
    for (auto it = shared_drivers_.begin(); it != shared_drivers_.end(); ++it) {
      if (*it == this) {
        shared_drivers_.erase(it);
        break;
      }
    }
    if (!shared_drivers_.empty()) {
      sdl_device_id_ = -1;
    }
  }
#endif  // XE_PLATFORM_IOS
  if (sdl_device_id_ > 0) {
    SDL_CloseAudioDevice(sdl_device_id_);
    sdl_device_id_ = -1;
  }
  if (sdl_initialized_) {
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    sdl_initialized_ = false;
  }
  std::unique_lock<std::mutex> guard(frames_mutex_);
  while (!frames_unused_.empty()) {
    delete[] frames_unused_.top();
    frames_unused_.pop();
  };
  while (!frames_queued_.empty()) {
    delete[] frames_queued_.front();
    frames_queued_.pop();
  };
}

void SDLAudioDriver::SDLCallback(void* userdata, Uint8* stream, int len) {
  SCOPE_profile_cpu_f("apu");
  if (!userdata || !stream) {
    XELOGE("SDLAudioDriver::sdl_callback called with nullptr.");
    return;
  }
#if XE_PLATFORM_IOS
  // One device serves every client: render each and sum them.
  std::lock_guard<std::mutex> shared_lock(shared_drivers_mutex_);
  std::memset(stream, 0, len);
  float mix_frame[kFrameSizeMax / sizeof(float)];
  if (len < 0 || static_cast<size_t>(len) > sizeof(mix_frame)) {
    return;
  }
  auto* mixed = reinterpret_cast<float*>(stream);
  for (SDLAudioDriver* driver : shared_drivers_) {
    driver->RenderFrame(reinterpret_cast<Uint8*>(mix_frame), len);
    for (size_t i = 0; i < static_cast<size_t>(len) / sizeof(float); ++i) {
      mixed[i] += mix_frame[i];
    }
  }
#else
  static_cast<SDLAudioDriver*>(userdata)->RenderFrame(stream, len);
#endif  // XE_PLATFORM_IOS
};

void SDLAudioDriver::RenderFrame(Uint8* stream, int len) {
  assert_true(len == sizeof(float) * channel_samples_ * sdl_device_channels_);

  std::unique_lock<std::mutex> guard(frames_mutex_);
  if (frames_queued_.empty()) {
    std::memset(stream, 0, len);
  } else {
    auto buffer = frames_queued_.front();
    frames_queued_.pop();
    if (cvars::mute) {
      std::memset(stream, 0, len);
    } else if (need_format_conversion_) {
      switch (sdl_device_channels_) {
        case 2:
          conversion::sequential_6_BE_to_interleaved_2_LE(
              reinterpret_cast<float*>(stream), buffer, channel_samples_);
          break;
        case 6:
          conversion::sequential_6_BE_to_interleaved_6_LE(
              reinterpret_cast<float*>(stream), buffer, channel_samples_);
          break;
        default:
          assert_unhandled_case(sdl_device_channels_);
          break;
      }
    } else {
      assert_true(sdl_device_channels_ == frame_channels_);
      if (volume_ != 1.0f) {
        std::memset(stream, 0, len);
        SDL_MixAudioFormat(stream, reinterpret_cast<Uint8*>(buffer), AUDIO_F32,
                           len, static_cast<int>(volume_ * SDL_MIX_MAXVOLUME));
      } else {
        std::memcpy(stream, buffer, len);
      }
    }
    frames_unused_.push(buffer);

    auto ret = semaphore_->Release(1, nullptr);
    assert_true(ret);
  }
};
}  // namespace sdl
}  // namespace apu
}  // namespace xe
