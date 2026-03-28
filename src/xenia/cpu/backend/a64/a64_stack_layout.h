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
#include "xenia/cpu/backend/a64/a64_backend.h"
#include "xenia/cpu/backend/a64/a64_emitter.h"

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

class StackLayout {
 public:
  /**
   * Thunk stack layout (host ↔ guest transitions).
   * NOTE: must stay 16-byte aligned at all times.
   *
   * Memory map (offsets from post-allocation SP):
   *
   *   sp+0x000  arg_temp[0..3]   4 × uint64_t = 32 bytes  (scratch)
   *   sp+0x020  r[0..17]        18 × uint64_t = 144 bytes
   *   sp+0x0B0  xmm[0..30]      31 × vec128_t = 496 bytes
   *   ─────────────────────────────────────────────────────
   *   Total: 672 bytes  (16-byte aligned)
   *
   * Non-volatile register save (EmitSaveNonvolatileRegs):
   *   r[0..1]   = x19, x20   sp+0x020
   *   r[2..3]   = x21, x22   sp+0x030
   *   r[4..5]   = x23, x24   sp+0x040
   *   r[6..7]   = x25, x26   sp+0x050
   *   r[8..9]   = x27, x28   sp+0x060
   *   r[10..11] = x29, x30   sp+0x070   ← x30 (LR) lives at sp+0x078
   *   r[12]     = x17        sp+0x080
   *   xmm[0..3] = d8..d15               sp+0x0B0
   *
   * Volatile register save (EmitSaveVolatileRegs):
   *   r[0..1]   = x1,  x2    sp+0x020
   *   r[2..3]   = x3,  x4    sp+0x030
   *   r[4..5]   = x5,  x6    sp+0x040
   *   r[6..7]   = x7,  x8    sp+0x050
   *   r[8..9]   = x9,  x10   sp+0x060
   *   r[10..11] = x11, x12   sp+0x070
   *   r[12..13] = x13, x14   sp+0x080
   *   r[14..15] = x15, x30   sp+0x090   ← x30 (LR) lives at sp+0x098
   *   r[16..17] = x27, x28   sp+0x0A0
   *   xmm[0..30]= q1..q31               sp+0x0B0
   */
  XEPACKEDSTRUCT(Thunk, {
    uint64_t arg_temp[4];
    uint64_t r[18];
    vec128_t xmm[31];
  });
  static_assert(sizeof(Thunk) % 16 == 0,
                "sizeof(Thunk) must be a multiple of 16!");
  static const size_t THUNK_STACK_SIZE = sizeof(Thunk);

  // SP-relative byte offset where x30 (LR) is stored by each save path.
  // Used by EmitFunctionInfo::lr_save_offset so the DWARF encoder can
  // tell libunwind where the host return address lives in thunk frames.
  //
  //   Non-volatile: STP(X29, X30, SP, offsetof(Thunk, r[10]))
  //     → x30 at SP + offsetof(r[10]) + 8
  //     = SP + (32 + 10*8) + 8 = SP + 0x78
  static const size_t THUNK_LR_NONVOLATILE = 0x78;  // used by h2g & resolve

  //   Volatile:     STP(X15, X30, SP, offsetof(Thunk, r[14]))
  //     → x30 at SP + offsetof(r[14]) + 8
  //     = SP + (32 + 14*8) + 8 = SP + 0x98
  static const size_t THUNK_LR_VOLATILE = 0x98;  // used by g2h

  /**
   * Guest stack layout.
   *
   *   sp+0x000  arg temp, 3 × 8 = 24 bytes
   *   sp+0x020  scratch, 48 bytes  (kStashOffset)
   *   sp+0x050  X0 / context ptr   (GUEST_CTX_HOME)
   *   sp+0x058  guest return addr  (GUEST_RET_ADDR)
   *   sp+0x060  call return addr   (GUEST_CALL_RET_ADDR)
   *   sp+0x068  host x30 / LR      (HOST_RET_ADDR)
   *   sp+0x070  locals ...
   *   ─────────────────────────────────────────────────────
   *   Total: 112 bytes  (16-byte aligned)
   *
   * HOST_RET_ADDR: the guest prolog stores x30 here so the host call-chain
   * can be unwound by libunwind through guest frames. The DWARF encoder in
   * a64_code_cache_posix.cc references this constant to generate the correct
   * DW_CFA_offset rule for the LR register.
   */
  static const size_t GUEST_STACK_SIZE    = 96 + 16;
  static const size_t GUEST_CTX_HOME      = 80;
  static const size_t GUEST_RET_ADDR      = 88;
  static const size_t GUEST_CALL_RET_ADDR = 96;
  // Offset from post-allocation SP where the guest prolog saves x30.
  static const size_t HOST_RET_ADDR       = 104;
};

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe

#endif  // XENIA_CPU_BACKEND_A64_A64_STACK_LAYOUT_H_
