/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2026 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/hid/touch_input_driver_ios.h"

#include <array>
#include <mutex>
#include <vector>

#include "xenia/ui/virtual_key.h"

namespace xe {
namespace hid {
namespace touch {

namespace {

std::mutex touch_state_mutex_;
// Guarded by touch_state_mutex_. The packet number changes with every state
// change, as XInput requires.
X_INPUT_STATE touch_state_ = {};

class TouchInputDriver final : public InputDriver {
 public:
  TouchInputDriver(xe::ui::Window* window, size_t window_z_order)
      : InputDriver(window, window_z_order) {}

  X_STATUS Setup() override { return X_STATUS_SUCCESS; }

  X_RESULT GetCapabilities(uint32_t user_index, uint32_t flags,
                           X_INPUT_CAPABILITIES* out_caps) override {
    if (user_index != 0) {
      return X_ERROR_DEVICE_NOT_CONNECTED;
    }
    if (!out_caps) {
      return X_ERROR_BAD_ARGUMENTS;
    }
    out_caps->type = X_INPUT_DEVTYPE::XINPUT_DEVTYPE_GAMEPAD;
    out_caps->sub_type = X_INPUT_DEVSUBTYPE::XINPUT_DEVSUBTYPE_GAMEPAD;
    out_caps->flags = 0;
    out_caps->gamepad.buttons = 0xFFFF;
    out_caps->gamepad.left_trigger = 0xFF;
    out_caps->gamepad.right_trigger = 0xFF;
    out_caps->gamepad.thumb_lx = int16_t(0xFFFFu);
    out_caps->gamepad.thumb_ly = int16_t(0xFFFFu);
    out_caps->gamepad.thumb_rx = int16_t(0xFFFFu);
    out_caps->gamepad.thumb_ry = int16_t(0xFFFFu);
    out_caps->vibration.left_motor_speed = 0;
    out_caps->vibration.right_motor_speed = 0;
    return X_ERROR_SUCCESS;
  }

  X_RESULT GetState(uint32_t user_index, X_INPUT_STATE* out_state) override {
    if (user_index != 0) {
      return X_ERROR_DEVICE_NOT_CONNECTED;
    }
    if (!out_state) {
      return X_ERROR_BAD_ARGUMENTS;
    }
    std::lock_guard<std::mutex> lock(touch_state_mutex_);
    *out_state = touch_state_;
    return X_ERROR_SUCCESS;
  }

  X_RESULT SetState(uint32_t user_index,
                    X_INPUT_VIBRATION* vibration) override {
    return user_index == 0 ? X_ERROR_SUCCESS : X_ERROR_DEVICE_NOT_CONNECTED;
  }

  X_RESULT GetKeystroke(uint32_t user_index, uint32_t flags,
                        X_INPUT_KEYSTROKE* out_keystroke) override {
    if (user_index != 0 && user_index != XUserIndexAny) {
      return X_ERROR_DEVICE_NOT_CONNECTED;
    }
    if (!out_keystroke) {
      return X_ERROR_BAD_ARGUMENTS;
    }
    // Bits 0-15 follow X_INPUT_GAMEPAD::buttons, 16 and 17 are the triggers.
    static constexpr std::array<ui::VirtualKey, 18> kKeystrokeKeys = {
        ui::VirtualKey::kXInputPadDpadUp,
        ui::VirtualKey::kXInputPadDpadDown,
        ui::VirtualKey::kXInputPadDpadLeft,
        ui::VirtualKey::kXInputPadDpadRight,
        ui::VirtualKey::kXInputPadStart,
        ui::VirtualKey::kXInputPadBack,
        ui::VirtualKey::kXInputPadLThumbPress,
        ui::VirtualKey::kXInputPadRThumbPress,
        ui::VirtualKey::kXInputPadLShoulder,
        ui::VirtualKey::kXInputPadRShoulder,
        ui::VirtualKey::kXInputPadGuide,
        ui::VirtualKey::kNone,
        ui::VirtualKey::kXInputPadA,
        ui::VirtualKey::kXInputPadB,
        ui::VirtualKey::kXInputPadX,
        ui::VirtualKey::kXInputPadY,
        ui::VirtualKey::kXInputPadLTrigger,
        ui::VirtualKey::kXInputPadRTrigger,
    };
    uint32_t keystroke_buttons;
    {
      std::lock_guard<std::mutex> lock(touch_state_mutex_);
      keystroke_buttons = touch_state_.gamepad.buttons;
      if (touch_state_.gamepad.left_trigger >
          X_INPUT_GAMEPAD_TRIGGER_THRESHOLD) {
        keystroke_buttons |= 1u << 16;
      }
      if (touch_state_.gamepad.right_trigger >
          X_INPUT_GAMEPAD_TRIGGER_THRESHOLD) {
        keystroke_buttons |= 1u << 17;
      }
    }
    // Report one change per call, releases before presses.
    for (uint32_t release_pass = 2; release_pass--;) {
      for (uint32_t i = 0; i < kKeystrokeKeys.size(); ++i) {
        const uint32_t keystroke_bit = 1u << i;
        const bool pressed = (keystroke_buttons & keystroke_bit) != 0;
        if (kKeystrokeKeys[i] == ui::VirtualKey::kNone ||
            !((keystroke_buttons ^ last_keystroke_buttons_) & keystroke_bit) ||
            pressed == bool(release_pass)) {
          continue;
        }
        last_keystroke_buttons_ ^= keystroke_bit;
        out_keystroke->virtual_key = uint16_t(kKeystrokeKeys[i]);
        out_keystroke->unicode = 0;
        out_keystroke->flags =
            pressed ? X_INPUT_KEYSTROKE_KEYDOWN : X_INPUT_KEYSTROKE_KEYUP;
        out_keystroke->user_index = 0;
        out_keystroke->hid_code = 0;
        return X_ERROR_SUCCESS;
      }
    }
    return X_ERROR_EMPTY;
  }

  InputType GetInputType() const override { return InputType::Controller; }

  std::vector<InputDeviceInfo> EnumerateDevices() override {
    InputDeviceInfo info;
    info.driver_slot = 0;
    info.stable_id = "ios-touch";
    info.display_name = "On-screen controller";
    info.fallback_auto_bind = true;
    return {info};
  }

 private:
  // Buttons already reported through GetKeystroke, in kKeystrokeKeys order.
  uint32_t last_keystroke_buttons_ = 0;
};

}  // namespace

void SetTouchState(uint16_t buttons, uint8_t left_trigger,
                   uint8_t right_trigger, int16_t thumb_lx, int16_t thumb_ly,
                   int16_t thumb_rx, int16_t thumb_ry) {
  std::lock_guard<std::mutex> lock(touch_state_mutex_);
  X_INPUT_GAMEPAD& gamepad = touch_state_.gamepad;
  if (gamepad.buttons == buttons && gamepad.left_trigger == left_trigger &&
      gamepad.right_trigger == right_trigger && gamepad.thumb_lx == thumb_lx &&
      gamepad.thumb_ly == thumb_ly && gamepad.thumb_rx == thumb_rx &&
      gamepad.thumb_ry == thumb_ry) {
    return;
  }
  gamepad.buttons = buttons;
  gamepad.left_trigger = left_trigger;
  gamepad.right_trigger = right_trigger;
  gamepad.thumb_lx = thumb_lx;
  gamepad.thumb_ly = thumb_ly;
  gamepad.thumb_rx = thumb_rx;
  gamepad.thumb_ry = thumb_ry;
  touch_state_.packet_number = touch_state_.packet_number + 1;
}

std::unique_ptr<InputDriver> Create(xe::ui::Window* window,
                                    size_t window_z_order) {
  return std::make_unique<TouchInputDriver>(window, window_z_order);
}

}  // namespace touch
}  // namespace hid
}  // namespace xe
