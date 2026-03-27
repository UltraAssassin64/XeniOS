/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2024 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_CPU_BACKEND_A64_A64_STACK_LAYOUT_H_
#define XENIA_CPU_BACKEND_A64_A64_STACK_LAYOUT_H_

#include "xenia/base/vec128.h"

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

class StackLayout {
 public:
  /**
   * ARM64 Thunk Stack Layout (host → guest transition).
   * NOTE: stack must always be 16-byte aligned.
   *
   *  sp+0x000: stp x19, x20
   *  sp+0x010: stp x21, x22
   *  sp+0x020: stp x23, x24
   *  sp+0x030: stp x25, x26
   *  sp+0x040: stp x27, x28
   *  sp+0x050: stp x29 (fp), x30 (lr)
   *  sp+0x060: stp q8,  q9    (full 128-bit)
   *  sp+0x080: stp q10, q11
   *  sp+0x0A0: stp q12, q13
   *  sp+0x0C0: stp q14, q15
   *  ─────────────────────────
   *  Total: 0xE0 = 224 bytes  (16-byte aligned)
   */
  static constexpr size_t THUNK_STACK_SIZE = 224;

  /**
   * ARM64 Guest Stack Layout.
   *
   *  sp+0x000: scratch, 48 bytes  (3 × Q for VMX / FP scratch)
   *  sp+0x030: guest_ret_addr     (guest PPC return address)
   *  sp+0x038: call_ret_addr      (next call's guest PPC return addr)
   *  sp+0x040: host_ret_addr      (host x30 / LR, restored by RET)
   *  sp+0x048: guest_saved_r1     (guest r1 at entry, for longjmp detection)
   *  ─────────────────────────
   *  Total: 0x50 = 80 bytes  (16-byte aligned)
   *
   * Convention: at guest function entry x0 holds the guest PPC return address.
   * The prolog stores it to GUEST_RET_ADDR and saves x30 to HOST_RET_ADDR.
   */
  static constexpr size_t GUEST_STACK_SIZE           = 80;
  static constexpr size_t GUEST_SCRATCH              = 0;   // 48 bytes (3 × Q)
  static constexpr size_t GUEST_RET_ADDR             = 48;
  static constexpr size_t GUEST_CALL_RET_ADDR        = 56;
  static constexpr size_t HOST_RET_ADDR              = 64;
  static constexpr size_t GUEST_SAVED_STACKPOINT_DEPTH = 72;
  static const size_t GUEST_CTX_HOME = 80;
};

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe

#endif  // XENIA_CPU_BACKEND_A64_A64_STACK_LAYOUT_H_
