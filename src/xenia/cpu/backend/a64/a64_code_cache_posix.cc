/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2024 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

// XeniOS a64_code_cache_posix.cc
//
// Integrates xenia-canary's proper DWARF .eh_frame unwind info generation
// (CIE + FDE per function, __register_frame / __deregister_frame) into
// XeniOS's existing PosixA64CodeCache, replacing the previous stub that
// emitted no real unwind records.
//
// Key design points:
//  • The XeniOS Thunk struct (arg_temp[4] + r[18] + xmm[31] = 672 bytes)
//    is preserved exactly. DWARF offsets are computed from its layout.
//  • Three frame kinds are handled:
//      1. Thunk frames  (stack_size == THUNK_STACK_SIZE == 672):
//           all callee-saved GPRs and NEON regs encoded from Thunk offsets.
//      2. Guest frames  (stack_size == GUEST_STACK_SIZE == 112):
//           only LR encoded at HOST_RET_ADDR = sp+104.
//      3. Arbitrary frames (lr_save_offset > 0):
//           LR encoded at the explicitly recorded offset.
//      4. Leaf frames   (stack_size == 0): no instructions needed.
//  • Icache flushing uses sys_icache_invalidate on Apple (matching oaknut's
//    CodeBlock::invalidate()) and __builtin___clear_cache elsewhere.
//  • No xbyak_aarch64 dependency; this file is pure C++.

#include "xenia/cpu/backend/a64/a64_code_cache.h"

#include <sys/mman.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <vector>

#ifdef XE_PLATFORM_APPLE
#include <libkern/OSCacheControl.h>
#include <pthread.h>
#endif

#include "xenia/base/assert.h"
#include "xenia/base/logging.h"
#include "xenia/base/math.h"
#include "xenia/base/memory.h"
#include "xenia/cpu/backend/a64/a64_stack_layout.h"
#include "xenia/cpu/function.h"

// libgcc / libunwind APIs for registering DWARF .eh_frame data.
extern "C" void __register_frame(void*);
extern "C" void __deregister_frame(void*);

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

// ---------------------------------------------------------------------------
// DWARF constants
// ---------------------------------------------------------------------------

// Maximum .eh_frame bytes per function: CIE (~40) + FDE (~80) + terminator(4).
static constexpr uint32_t kMaxUnwindInfoSize = 128;

// AArch64 DWARF register numbers (DWARF standard §3.4.2).
static constexpr uint8_t kDwarfX19 = 19;
static constexpr uint8_t kDwarfX20 = 20;
static constexpr uint8_t kDwarfX21 = 21;
static constexpr uint8_t kDwarfX22 = 22;
static constexpr uint8_t kDwarfX23 = 23;
static constexpr uint8_t kDwarfX24 = 24;
static constexpr uint8_t kDwarfX25 = 25;
static constexpr uint8_t kDwarfX26 = 26;
static constexpr uint8_t kDwarfX27 = 27;
static constexpr uint8_t kDwarfX28 = 28;
static constexpr uint8_t kDwarfFP  = 29;   // x29 frame pointer
static constexpr uint8_t kDwarfLR  = 30;   // x30 link register
static constexpr uint8_t kDwarfSP  = 31;   // stack pointer
static constexpr uint8_t kDwarfD8  = 72;   // d8-d15 callee-saved
static constexpr uint8_t kDwarfD9  = 73;
static constexpr uint8_t kDwarfD10 = 74;
static constexpr uint8_t kDwarfD11 = 75;
static constexpr uint8_t kDwarfD12 = 76;
static constexpr uint8_t kDwarfD13 = 77;
static constexpr uint8_t kDwarfD14 = 78;
static constexpr uint8_t kDwarfD15 = 79;

// DWARF CFA opcodes.
static constexpr uint8_t kDW_CFA_advance_loc1   = 0x02;
static constexpr uint8_t kDW_CFA_advance_loc2   = 0x03;
static constexpr uint8_t kDW_CFA_def_cfa        = 0x0c;
static constexpr uint8_t kDW_CFA_def_cfa_offset = 0x0e;
static constexpr uint8_t kDW_CFA_nop            = 0x00;

// DWARF pointer encoding (FDE PC-begin format).
static constexpr uint8_t kDW_EH_PE_pcrel  = 0x10;
static constexpr uint8_t kDW_EH_PE_sdata4 = 0x0b;

// ---------------------------------------------------------------------------
// LEB128 encoding helpers
// ---------------------------------------------------------------------------

static size_t WriteULEB128(uint8_t* p, uint64_t value) {
  size_t n = 0;
  do {
    uint8_t byte = value & 0x7Fu;
    value >>= 7;
    if (value) byte |= 0x80u;
    p[n++] = byte;
  } while (value);
  return n;
}

static size_t WriteSLEB128(uint8_t* p, int64_t value) {
  size_t n = 0;
  bool more = true;
  while (more) {
    uint8_t byte = value & 0x7F;
    value >>= 7;
    if ((value == 0 && !(byte & 0x40)) || (value == -1 && (byte & 0x40))) {
      more = false;
    } else {
      byte |= 0x80;
    }
    p[n++] = byte;
  }
  return n;
}

// ---------------------------------------------------------------------------
// Icache flush — matches oaknut CodeBlock::invalidate() semantics.
// ---------------------------------------------------------------------------

static void FlushIcache(void* address, size_t size) {
#if defined(XE_PLATFORM_APPLE)
  // Apple: sys_icache_invalidate handles both D-cache clean and I-cache
  // invalidate in one call (same as oaknut's Apple path).
  sys_icache_invalidate(address, size);
#else
  // Linux and other POSIX: compiler intrinsic emits DC CVAU + ISB + IC IVAU.
  __builtin___clear_cache(static_cast<char*>(address),
                          static_cast<char*>(address) + size);
#endif
}

// ---------------------------------------------------------------------------
// PosixA64CodeCache
// ---------------------------------------------------------------------------

class PosixA64CodeCache : public A64CodeCache {
 public:
  PosixA64CodeCache();
  ~PosixA64CodeCache() override;

  bool Initialize() override;

  void* LookupUnwindInfo(uint64_t host_pc) override { return nullptr; }

 protected:
  // Copies code to the write alias and flushes the I-cache so the execute
  // alias immediately sees the new instructions.
  void CopyMachineCode(void* dest, const void* src, size_t size) override;

 private:
  // Encodes a complete DWARF CIE + FDE for one function into the buffer at
  // |unwind_entry_address| (write side), covering code at
  // |code_execute_address| with unwind semantics from |func_info|.
  void InitializeUnwindEntry(uint8_t* unwind_entry_address,
                             void* code_execute_address,
                             const EmitFunctionInfo& func_info);

  UnwindReservation RequestUnwindReservation(
      uint8_t* entry_address) override;

  void PlaceCode(uint32_t guest_address, void* machine_code,
                 const EmitFunctionInfo& func_info,
                 void* code_execute_address,
                 UnwindReservation unwind_reservation) override;

  // Execute-side pointers registered with __register_frame, kept for
  // __deregister_frame on destruction.
  std::vector<void*> registered_frames_;
  uint32_t unwind_table_count_ = 0;
};

// --- Factory -----------------------------------------------------------------

std::unique_ptr<A64CodeCache> A64CodeCache::Create() {
  return std::make_unique<PosixA64CodeCache>();
}

// --- Lifecycle ---------------------------------------------------------------

PosixA64CodeCache::PosixA64CodeCache() = default;

PosixA64CodeCache::~PosixA64CodeCache() {
  for (void* frame : registered_frames_) {
    __deregister_frame(frame);
  }
}

bool PosixA64CodeCache::Initialize() {
  if (!A64CodeCache::Initialize()) {
    return false;
  }
  registered_frames_.reserve(kMaximumFunctionCount);
  return true;
}

// --- CopyMachineCode (with I-cache flush) ------------------------------------

void PosixA64CodeCache::CopyMachineCode(void* dest, const void* src,
                                        size_t size) {
  std::memcpy(dest, src, size);
  // On iOS the write and execute views are separate VA mappings of the same
  // physical pages (dual-map) or the same mapping with toggling permissions
  // (mprotect-flip). In both cases flushing from the write address ensures the
  // I-cache coherency through the execute alias.
  FlushIcache(dest, size);
}

// --- Unwind reservation ------------------------------------------------------

A64CodeCache::UnwindReservation
PosixA64CodeCache::RequestUnwindReservation(uint8_t* entry_address) {
#if defined(NDEBUG)
  if (unwind_table_count_ >= kMaximumFunctionCount) {
    xe::FatalError(
        "DWARF unwind table overflow. Please report this to Xenia developers.");
  }
#else
  assert_false(unwind_table_count_ >= kMaximumFunctionCount);
#endif

  UnwindReservation r;
  r.data_size     = xe::round_up(kMaxUnwindInfoSize, 16);
  r.table_slot    = unwind_table_count_++;
  r.entry_address = entry_address;
  return r;
}

// --- PlaceCode ---------------------------------------------------------------

void PosixA64CodeCache::PlaceCode(uint32_t /*guest_address*/,
                                  void* /*machine_code*/,
                                  const EmitFunctionInfo& func_info,
                                  void* code_execute_address,
                                  UnwindReservation unwind_reservation) {
  // Build the DWARF CIE + FDE into the write-side unwind buffer.
  InitializeUnwindEntry(unwind_reservation.entry_address, code_execute_address,
                        func_info);

  // __register_frame expects the execute-side address of the FDE data.
  void* unwind_execute_address =
      static_cast<uint8_t*>(unwind_reservation.entry_address) -
      generated_code_write_base_ + generated_code_execute_base_;

  __register_frame(unwind_execute_address);
  registered_frames_.push_back(unwind_execute_address);
}

// ---------------------------------------------------------------------------
// DWARF .eh_frame builder
// ---------------------------------------------------------------------------
//
// Layout written per function:  [CIE][FDE][zero-terminator]
//
// Frame classification (using XeniOS StackLayout constants):
//
//  A) THUNK_STACK_SIZE (672):
//       Encodes all callee-saved GPRs (x19-x28, fp, lr) and NEON regs
//       (d8-d15) at their offsets inside the Thunk struct:
//         r_base = sizeof(arg_temp) = 32
//         x30 lives at r_base + 11*8 = 120 = 0x78  (via STP X29,X30,r[10])
//         d8-d15 at xmm_base + 0,8,16,24... (via STP D8,D9 / D10,D11 ...)
//
//  B) GUEST_STACK_SIZE (112):
//       Encodes only LR at HOST_RET_ADDR (104), which is stored by the
//       guest prolog so the unwinder can follow the host call chain.
//
//  C) lr_save_offset > 0:
//       Arbitrary frame — emitter recorded exactly where it stored x30.
//
//  D) stack_size == 0:
//       Leaf function — no CFA or register rules needed.

void PosixA64CodeCache::InitializeUnwindEntry(
    uint8_t* unwind_entry_address, void* code_execute_address,
    const EmitFunctionInfo& func_info) {

  // Execute-side VA of the unwind buffer (needed for pc-relative FDE fields).
  uint8_t* unwind_execute_base =
      unwind_entry_address - generated_code_write_base_ +
      generated_code_execute_base_;

  uint8_t* p         = unwind_entry_address;
  uint8_t* cie_start = p;

  // ===========================================================================
  // CIE — Common Information Entry
  // ===========================================================================

  uint8_t* cie_length_ptr    = p;
  p += 4;
  uint8_t* cie_content_start = p;

  // CIE ID = 0.
  *reinterpret_cast<uint32_t*>(p) = 0;  p += 4;

  // Version 1.
  *p++ = 1;

  // Augmentation "zR": z = augmentation data present, R = FDE pointer encoding.
  *p++ = 'z';  *p++ = 'R';  *p++ = '\0';

  // Code alignment factor: ARM64 instructions are always 4 bytes.
  p += WriteULEB128(p, 4);

  // Data alignment factor: -8 (each saved register occupies 8 bytes).
  p += WriteSLEB128(p, -8);

  // Return address register: x30 (link register).
  p += WriteULEB128(p, kDwarfLR);

  // Augmentation data length = 1 (the FDE pointer encoding byte).
  p += WriteULEB128(p, 1);

  // FDE pointer encoding: PC-relative, signed 32-bit.
  *p++ = kDW_EH_PE_pcrel | kDW_EH_PE_sdata4;

  // Initial CFA rule: CFA = SP + 0 (at function entry, before any push).
  *p++ = kDW_CFA_def_cfa;
  p += WriteULEB128(p, kDwarfSP);
  p += WriteULEB128(p, 0);

  // Pad CIE to pointer-size alignment.
  size_t cie_len    = static_cast<size_t>(p - cie_content_start);
  size_t cie_padded = xe::round_up(cie_len, sizeof(void*));
  while (p < cie_content_start + cie_padded) *p++ = kDW_CFA_nop;

  *reinterpret_cast<uint32_t*>(cie_length_ptr) =
      static_cast<uint32_t>(p - cie_content_start);

  // ===========================================================================
  // FDE — Frame Description Entry
  // ===========================================================================

  uint8_t* fde_length_ptr    = p;
  p += 4;
  uint8_t* fde_content_start = p;

  // CIE pointer: distance back to the CIE.
  *reinterpret_cast<uint32_t*>(p) =
      static_cast<uint32_t>(p - cie_start);
  p += 4;

  // PC-begin: pc-relative signed 32-bit offset from this field's execute VA
  // to the first instruction of the described function.
  uint8_t* pc_begin_exec =
      unwind_execute_base + static_cast<ptrdiff_t>(p - unwind_entry_address);
  *reinterpret_cast<int32_t*>(p) = static_cast<int32_t>(
      reinterpret_cast<intptr_t>(code_execute_address) -
      reinterpret_cast<intptr_t>(pc_begin_exec));
  p += 4;

  // PC range: total byte size of the function.
  *reinterpret_cast<uint32_t*>(p) =
      static_cast<uint32_t>(func_info.code_size.total);
  p += 4;

  // Augmentation data length = 0.
  p += WriteULEB128(p, 0);

  // ---------------------------------------------------------------------------
  // FDE instructions
  // ---------------------------------------------------------------------------

  if (func_info.stack_size > 0) {
    // Advance the unwinder's location to the instruction after the stack
    // allocation. prolog_stack_alloc_offset is in bytes; ARM64 code alignment
    // factor is 4, so the DWARF factored offset is /4.
    const size_t alloc_offset = func_info.prolog_stack_alloc_offset;
    if (alloc_offset > 0) {
      const uint32_t factored = static_cast<uint32_t>(alloc_offset / 4);
      if (factored < 64) {
        *p++ = 0x40u | static_cast<uint8_t>(factored);
      } else if (factored < 256) {
        *p++ = kDW_CFA_advance_loc1;
        *p++ = static_cast<uint8_t>(factored);
      } else {
        *p++ = kDW_CFA_advance_loc2;
        *reinterpret_cast<uint16_t*>(p) = static_cast<uint16_t>(factored);
        p += 2;
      }
    }

    // CFA = SP + stack_size (frame is now established).
    *p++ = kDW_CFA_def_cfa_offset;
    p += WriteULEB128(p, func_info.stack_size);

    // -------------------------------------------------------------------------
    // Frame kind A: thunk frame.
    // All callee-saved registers encoded from the XeniOS Thunk struct layout.
    //
    // EmitSaveNonvolatileRegs stores registers as follows (r_base = 32):
    //   STP(X19, X20, SP, r[0])   → x19@sp+0x20  x20@sp+0x28
    //   STP(X21, X22, SP, r[2])   → x21@sp+0x30  x22@sp+0x38
    //   STP(X23, X24, SP, r[4])   → x23@sp+0x40  x24@sp+0x48
    //   STP(X25, X26, SP, r[6])   → x25@sp+0x50  x26@sp+0x58
    //   STP(X27, X28, SP, r[8])   → x27@sp+0x60  x28@sp+0x68
    //   STP(X29, X30, SP, r[10])  → x29@sp+0x70  x30@sp+0x78
    //   STP(D8,  D9,  SP, xmm[0]) → d8 @sp+0xB0  d9 @sp+0xB8
    //   STP(D10, D11, SP, xmm[1]) → d10@sp+0xC0  d11@sp+0xC8
    //   STP(D12, D13, SP, xmm[2]) → d12@sp+0xD0  d13@sp+0xD8
    //   STP(D14, D15, SP, xmm[3]) → d14@sp+0xE0  d15@sp+0xE8
    //
    // DW_CFA_offset encodes: register r saved at CFA - N*|data_align|.
    //   data_align = -8 → N = (cfa - save_address) / 8.
    // -------------------------------------------------------------------------
    if (func_info.stack_size == StackLayout::THUNK_STACK_SIZE) {
      const size_t cfa = func_info.stack_size;  // 672 = 0x2A0

      // GPRs (from EmitSaveNonvolatileRegs).
      // r_base = offsetof(Thunk, r) = sizeof(arg_temp) = 4*8 = 32 = 0x20.
      static constexpr size_t r_base   = 0x20;
      static constexpr size_t xmm_base = 0xB0;  // r_base + 18*8 = 32+144

      auto encReg = [&](uint8_t dwarf_reg, size_t sp_offset) {
        *p++ = 0x80u | dwarf_reg;
        p += WriteULEB128(p, (cfa - sp_offset) / 8);
      };

      encReg(kDwarfX19, r_base + 0x00);   // x19 @ sp+0x20
      encReg(kDwarfX20, r_base + 0x08);   // x20 @ sp+0x28
      encReg(kDwarfX21, r_base + 0x10);   // x21 @ sp+0x30
      encReg(kDwarfX22, r_base + 0x18);   // x22 @ sp+0x38
      encReg(kDwarfX23, r_base + 0x20);   // x23 @ sp+0x40
      encReg(kDwarfX24, r_base + 0x28);   // x24 @ sp+0x48
      encReg(kDwarfX25, r_base + 0x30);   // x25 @ sp+0x50
      encReg(kDwarfX26, r_base + 0x38);   // x26 @ sp+0x58
      encReg(kDwarfX27, r_base + 0x40);   // x27 @ sp+0x60
      encReg(kDwarfX28, r_base + 0x48);   // x28 @ sp+0x68
      encReg(kDwarfFP,  r_base + 0x50);   // x29 @ sp+0x70
      encReg(kDwarfLR,  r_base + 0x58);   // x30 @ sp+0x78  (== THUNK_LR_NONVOLATILE)
      // NEON d8-d15 (each D reg is lower 64 bits of the saved pair entry).
      encReg(kDwarfD8,  xmm_base + 0x00); // d8  @ sp+0xB0
      encReg(kDwarfD9,  xmm_base + 0x08); // d9  @ sp+0xB8
      encReg(kDwarfD10, xmm_base + 0x10); // d10 @ sp+0xC0
      encReg(kDwarfD11, xmm_base + 0x18); // d11 @ sp+0xC8
      encReg(kDwarfD12, xmm_base + 0x20); // d12 @ sp+0xD0
      encReg(kDwarfD13, xmm_base + 0x28); // d13 @ sp+0xD8
      encReg(kDwarfD14, xmm_base + 0x30); // d14 @ sp+0xE0
      encReg(kDwarfD15, xmm_base + 0x38); // d15 @ sp+0xE8

    // -------------------------------------------------------------------------
    // Frame kind B: guest frame.
    // The guest prolog saves x30 at HOST_RET_ADDR = sp+104.
    // CFA - save_address = GUEST_STACK_SIZE - HOST_RET_ADDR = 112 - 104 = 8.
    // -------------------------------------------------------------------------
    } else if (func_info.stack_size == StackLayout::GUEST_STACK_SIZE) {
      const size_t cfa        = func_info.stack_size;        // 112
      const size_t lr_offset  = StackLayout::HOST_RET_ADDR; // 104
      *p++ = 0x80u | kDwarfLR;
      p += WriteULEB128(p, (cfa - lr_offset) / 8);          // = 1

    // -------------------------------------------------------------------------
    // Frame kind C: emitter-recorded arbitrary frame.
    // lr_save_offset is the SP-relative byte offset where the prolog stored x30.
    // -------------------------------------------------------------------------
    } else if (func_info.lr_save_offset > 0) {
      const size_t lr_cfa_offset =
          func_info.stack_size - func_info.lr_save_offset;
      *p++ = 0x80u | kDwarfLR;
      p += WriteULEB128(p, lr_cfa_offset / 8);
    }
    // Frame kind D (stack_size == 0) needs no register rules.
  }

  // Pad FDE to pointer-size alignment.
  size_t fde_len    = static_cast<size_t>(p - fde_content_start);
  size_t fde_padded = xe::round_up(fde_len, sizeof(void*));
  while (p < fde_content_start + fde_padded) *p++ = kDW_CFA_nop;

  *reinterpret_cast<uint32_t*>(fde_length_ptr) =
      static_cast<uint32_t>(p - fde_content_start);

  // Zero-length terminator marks the end of the .eh_frame section.
  *reinterpret_cast<uint32_t*>(p) = 0;
  p += 4;

  assert_true(static_cast<size_t>(p - unwind_entry_address) <=
              kMaxUnwindInfoSize);
}

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe
