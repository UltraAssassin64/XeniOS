/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2026 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_HID_TOUCH_INPUT_DRIVER_IOS_H_
#define XENIA_HID_TOUCH_INPUT_DRIVER_IOS_H_

#include <cstddef>
#include <cstdint>
#include <memory>

#include "xenia/hid/input_driver.h"

namespace xe {
namespace hid {
namespace touch {

// Stores the latest state of the iOS on-screen controller. Called by the UI
// whenever a touch changes it; read by the touch input driver. Buttons are
// X_INPUT_GAMEPAD_* bits, thumbstick axes use the XInput range with up and
// right positive.
void SetTouchState(uint16_t buttons, uint8_t left_trigger,
                   uint8_t right_trigger, int16_t thumb_lx, int16_t thumb_ly,
                   int16_t thumb_rx, int16_t thumb_ry);

// Creates the input driver that presents the on-screen controller as a
// fallback gamepad: InputSystem binds it to player one only while no physical
// controller is connected.
std::unique_ptr<InputDriver> Create(xe::ui::Window* window,
                                    size_t window_z_order);

}  // namespace touch
}  // namespace hid
}  // namespace xe

#endif  // XENIA_HID_TOUCH_INPUT_DRIVER_IOS_H_
