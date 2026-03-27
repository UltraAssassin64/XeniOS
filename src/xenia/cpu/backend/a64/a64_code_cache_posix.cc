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
// XeniOS's existing PosixA64CodeCache, replacing the previous stub
// InitializeUnwindEntry that emitted no real records.
//
// The iOS-specific mprotect-flip, dual-map, and icache-flush logic from
// XeniOS is preserved. The canary DWARF encoder is ported directly; no
// xbyak_aarch64 dependencies exist in this file. Icache flushing uses
// oaknut's documented Apple path (sys_icache_invalidate on Apple,
// __builtin___clear_cache on Linux).

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
// On Apple platforms these are provided by libunwind (part of the OS).
extern "C" void __register_frame(void*);
extern "C" void __deregister_frame(void*);

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

// ---------------------------------------------------------------------------
// DWARF constants
// ---------------------------------------------------------------------------

// Maximum bytes of .eh_frame data per function (CIE + FDE + terminator).
// Sized conservatively: CIE ~40 bytes + FDE ~80 bytes + terminator 4 bytes.
static constexpr uint32_t kMaxUnwindInfoSize = 128;

// AArch64 DWARF register numbers (DWARF standard, §3.4.2).
static constexpr uint8_t kDwarfRegX19 = 19;
static constexpr uint8_t kDwarfRegX20 = 20;
static constexpr uint8_t kDwarfRegX21 = 21;
static constexpr uint8_t kDwarfRegX22 = 22;
static constexpr uint8_t kDwarfRegX23 = 23;
static constexpr uint8_t kDwarfRegX24 = 24;
static constexpr uint8_t kDwarfRegX25 = 25;
static constexpr uint8_t kDwarfRegX26 = 26;
static constexpr uint8_t kDwarfRegX27 = 27;
static constexpr uint8_t kDwarfRegX28 = 28;
static constexpr uint8_t kDwarfRegFP  = 29;  // x29 frame pointer
static constexpr uint8_t kDwarfRegLR  = 30;  // x30 link register
static constexpr uint8_t kDwarfRegSP  = 31;  // stack pointer
static constexpr uint8_t kDwarfRegD8  = 72;  // d8-d15 are callee-saved
static constexpr uint8_t kDwarfRegD9  = 73;
static constexpr uint8_t kDwarfRegD10 = 74;
static constexpr uint8_t kDwarfRegD11 = 75;
static constexpr uint8_t kDwarfRegD12 = 76;
static constexpr uint8_t kDwarfRegD13 = 77;
static constexpr uint8_t kDwarfRegD14 = 78;
static constexpr uint8_t kDwarfRegD15 = 79;

// DWARF CFA opcodes.
static constexpr uint8_t kDW_CFA_advance_loc1  = 0x02;
static constexpr uint8_t kDW_CFA_advance_loc2  = 0x03;
static constexpr uint8_t kDW_CFA_def_cfa       = 0x0c;
static constexpr uint8_t kDW_CFA_def_cfa_offset = 0x0e;
static constexpr uint8_t kDW_CFA_nop            = 0x00;

// DWARF pointer encoding (FDE PC-begin format).
static constexpr uint8_t kDW_EH_PE_pcrel  = 0x10;
static constexpr uint8_t kDW_EH_PE_sdata4 = 0x0b;

// ---------------------------------------------------------------------------
// LEB128 helpers
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
// Icache flush helper
// ---------------------------------------------------------------------------
// Matches oaknut CodeBlock::invalidate() semantics: Apple uses
// sys_icache_invalidate, all other POSIX uses the DC/IC instruction sequence
// via __builtin___clear_cache (which the compiler expands correctly on arm64).

static void FlushIcache(void* address, size_t size) {
#if defined(XE_PLATFORM_APPLE)
  sys_icache_invalidate(address, size);
#else
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
  // Flush icache after writing JIT code (called by PlaceGuestCode).
  void CopyMachineCode(void* dest, const void* src, size_t size) override;

 private:
  // Builds DWARF CIE + FDE into the reserved unwind buffer at
  // |unwind_entry_address| (write side) for code at |code_execute_address|.
  void InitializeUnwindEntry(uint8_t* unwind_entry_address,
                             void* code_execute_address,
                             const EmitFunctionInfo& func_info);

  UnwindReservation RequestUnwindReservation(
      uint8_t* entry_address) override;

  void PlaceCode(uint32_t guest_address, void* machine_code,
                 const EmitFunctionInfo& func_info,
                 void* code_execute_address,
                 UnwindReservation unwind_reservation) override;

  // Registered FDE execute-side pointers, kept for __deregister_frame on
  // destruction.
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

// --- CopyMachineCode (with icache flush) -------------------------------------

void PosixA64CodeCache::CopyMachineCode(void* dest, const void* src,
                                        size_t size) {
  std::memcpy(dest, src, size);
  // On iOS, the write and execute views are separate virtual mappings of the
  // same physical pages (dual-map) or the same mapping switched between RW and
  // RX (mprotect-flip). In either case flushing from the write address ensures
  // the I-cache sees the new instructions through the execute alias.
  FlushIcache(dest, size);
}

// --- Unwind reservation ------------------------------------------------------

A64CodeCache::UnwindReservation
PosixA64CodeCache::RequestUnwindReservation(uint8_t* entry_address) {
#if defined(NDEBUG)
  if (unwind_table_count_ >= kMaximumFunctionCount) {
    xe::FatalError(
        "Unwind table overflow — please report this to Xenia developers");
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

// --- PlaceCode (register DWARF frame) ----------------------------------------

void PosixA64CodeCache::PlaceCode(uint32_t /*guest_address*/,
                                  void* /*machine_code*/,
                                  const EmitFunctionInfo& func_info,
                                  void* code_execute_address,
                                  UnwindReservation unwind_reservation) {
  // Build the DWARF record into the write-side buffer.
  InitializeUnwindEntry(unwind_reservation.entry_address, code_execute_address,
                        func_info);

  // The CIE/FDE was written to the write alias; __register_frame needs the
  // execute-side address of the same bytes.
  void* unwind_execute_address =
      static_cast<uint8_t*>(unwind_reservation.entry_address) -
      generated_code_write_base_ + generated_code_execute_base_;

  __register_frame(unwind_execute_address);
  registered_frames_.push_back(unwind_execute_address);
}

// --- DWARF .eh_frame builder -------------------------------------------------
//
// Layout:  [CIE][FDE][zero-terminator]
//
// Matches xenia-canary's a64_code_cache_posix.cc exactly, adapted for
// XeniOS's stack layout constants (StackLayout::THUNK_STACK_SIZE = 224,
// HOST_RET_ADDR = 64) and EmitFunctionInfo fields.
//
// The FDE encodes:
//   • For thunk frames: all callee-saved GPRs (x19-x28, fp, lr) and
//     NEON registers (d8-d15) at their known stack offsets.
//   • For guest frames with a non-zero lr_save_offset: just the LR location,
//     so the unwinder can find the host return address.
//   • For leaf frames (stack_size == 0): no extra instructions needed.

void PosixA64CodeCache::InitializeUnwindEntry(
    uint8_t* unwind_entry_address, void* code_execute_address,
    const EmitFunctionInfo& func_info) {

  // The execute-side base of the unwind buffer (needed for pc-relative FDE
  // PC-begin field, which is relative to the field's own execute address).
  uint8_t* unwind_execute_base =
      unwind_entry_address - generated_code_write_base_ +
      generated_code_execute_base_;

  uint8_t* p         = unwind_entry_address;
  uint8_t* cie_start = p;

  // =========================================================================
  // CIE — Common Information Entry
  // =========================================================================

  uint8_t* cie_length_ptr    = p;
  p += 4;  // length field (filled in below)
  uint8_t* cie_content_start = p;

  // CIE ID = 0 (marks this as a CIE, not a FDE).
  *reinterpret_cast<uint32_t*>(p) = 0;
  p += 4;

  // Version 1.
  *p++ = 1;

  // Augmentation string "zR": z = augmentation data present, R = FDE pointer
  // encoding follows in the augmentation data.
  *p++ = 'z';
  *p++ = 'R';
  *p++ = '\0';

  // Code alignment factor: ARM64 instructions are 4 bytes wide.
  p += WriteULEB128(p, 4);

  // Data alignment factor: -8 (saved registers occupy 8 bytes each).
  p += WriteSLEB128(p, -8);

  // Return address register: x30 (link register).
  p += WriteULEB128(p, kDwarfRegLR);

  // Augmentation data length = 1 byte (the FDE pointer encoding byte).
  p += WriteULEB128(p, 1);

  // FDE pointer encoding: PC-relative, signed 32-bit.
  *p++ = kDW_EH_PE_pcrel | kDW_EH_PE_sdata4;

  // Initial CFA rule: CFA = SP + 0  (at function entry, before any push).
  *p++ = kDW_CFA_def_cfa;
  p += WriteULEB128(p, kDwarfRegSP);
  p += WriteULEB128(p, 0);

  // Pad CIE body to pointer-size alignment.
  size_t cie_content_len  = static_cast<size_t>(p - cie_content_start);
  size_t cie_padded_len   = xe::round_up(cie_content_len, sizeof(void*));
  while (p < cie_content_start + cie_padded_len) {
    *p++ = kDW_CFA_nop;
  }

  // Write CIE length (excludes the 4-byte length field itself).
  *reinterpret_cast<uint32_t*>(cie_length_ptr) =
      static_cast<uint32_t>(p - cie_content_start);

  // =========================================================================
  // FDE — Frame Description Entry
  // =========================================================================

  uint8_t* fde_length_ptr    = p;
  p += 4;  // length field (filled in below)
  uint8_t* fde_content_start = p;

  // CIE pointer: byte offset from the start of this FDE back to the CIE.
  *reinterpret_cast<uint32_t*>(p) =
      static_cast<uint32_t>(p - cie_start);
  p += 4;

  // PC-begin: pc-relative signed 32-bit offset from *this field's execute
  // address* to the first instruction of the function.
  uint8_t* pc_begin_execute_addr =
      unwind_execute_base + static_cast<ptrdiff_t>(p - unwind_entry_address);
  *reinterpret_cast<int32_t*>(p) = static_cast<int32_t>(
      reinterpret_cast<intptr_t>(code_execute_address) -
      reinterpret_cast<intptr_t>(pc_begin_execute_addr));
  p += 4;

  // PC range: total size of the function in bytes.
  *reinterpret_cast<uint32_t*>(p) =
      static_cast<uint32_t>(func_info.code_size.total);
  p += 4;

  // Augmentation data length = 0 (no extra FDE augmentation data).
  p += WriteULEB128(p, 0);

  // -------------------------------------------------------------------------
  // FDE instructions
  // -------------------------------------------------------------------------

  if (func_info.stack_size > 0) {
    // Advance the unwinder's location counter to the instruction immediately
    // after the stack allocation (STP / SUB SP, SP, #N).
    // prolog_stack_alloc_offset is in bytes; the code alignment factor is 4,
    // so the DWARF factored offset is /4.
    const size_t alloc_offset  = func_info.prolog_stack_alloc_offset;
    if (alloc_offset > 0) {
      const uint32_t factored = static_cast<uint32_t>(alloc_offset / 4);
      if (factored < 64) {
        // DW_CFA_advance_loc, short form (opcode in high 2 bits).
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

    // CFA = SP + stack_size  (after the stack frame is established).
    *p++ = kDW_CFA_def_cfa_offset;
    p += WriteULEB128(p, func_info.stack_size);

    if (func_info.stack_size == StackLayout::THUNK_STACK_SIZE) {
      // -----------------------------------------------------------------------
      // Thunk frame (host→guest transition).
      // See a64_stack_layout.h (canary version) for the exact layout:
      //
      //   sp+0x000: stp x19, x20     → d8-d15 comment omitted for brevity
      //   sp+0x010: stp x21, x22
      //   sp+0x020: stp x23, x24
      //   sp+0x030: stp x25, x26
      //   sp+0x040: stp x27, x28
      //   sp+0x050: stp x29(fp), x30(lr)
      //   sp+0x060: stp q8,  q9    (d8=sp+0x060, d9=sp+0x070)
      //   sp+0x080: stp q10, q11   (d10=sp+0x080, d11=sp+0x090)
      //   sp+0x0A0: stp q12, q13   (d12=sp+0x0A0, d13=sp+0x0B0)
      //   sp+0x0C0: stp q14, q15   (d14=sp+0x0C0, d15=sp+0x0D0)
      //
      // DW_CFA_offset encodes: register r saved at CFA - (N * data_align).
      // data_align = -8 → N = (CFA - save_address) / 8.
      // -----------------------------------------------------------------------
      const size_t cfa = func_info.stack_size;  // 224 = 0xE0

      // GPRs x19-x28.
      *p++ = 0x80u | kDwarfRegX19; p += WriteULEB128(p, (cfa - 0x000) / 8);
      *p++ = 0x80u | kDwarfRegX20; p += WriteULEB128(p, (cfa - 0x008) / 8);
      *p++ = 0x80u | kDwarfRegX21; p += WriteULEB128(p, (cfa - 0x010) / 8);
      *p++ = 0x80u | kDwarfRegX22; p += WriteULEB128(p, (cfa - 0x018) / 8);
      *p++ = 0x80u | kDwarfRegX23; p += WriteULEB128(p, (cfa - 0x020) / 8);
      *p++ = 0x80u | kDwarfRegX24; p += WriteULEB128(p, (cfa - 0x028) / 8);
      *p++ = 0x80u | kDwarfRegX25; p += WriteULEB128(p, (cfa - 0x030) / 8);
      *p++ = 0x80u | kDwarfRegX26; p += WriteULEB128(p, (cfa - 0x038) / 8);
      *p++ = 0x80u | kDwarfRegX27; p += WriteULEB128(p, (cfa - 0x040) / 8);
      *p++ = 0x80u | kDwarfRegX28; p += WriteULEB128(p, (cfa - 0x048) / 8);
      // Frame pointer (x29) and link register (x30).
      *p++ = 0x80u | kDwarfRegFP;  p += WriteULEB128(p, (cfa - 0x050) / 8);
      *p++ = 0x80u | kDwarfRegLR;  p += WriteULEB128(p, (cfa - 0x058) / 8);
      // NEON d8-d15 (low 64 bits of q8-q15; stp stores full 128-bit Q regs).
      *p++ = 0x80u | kDwarfRegD8;  p += WriteULEB128(p, (cfa - 0x060) / 8);
      *p++ = 0x80u | kDwarfRegD9;  p += WriteULEB128(p, (cfa - 0x070) / 8);
      *p++ = 0x80u | kDwarfRegD10; p += WriteULEB128(p, (cfa - 0x080) / 8);
      *p++ = 0x80u | kDwarfRegD11; p += WriteULEB128(p, (cfa - 0x090) / 8);
      *p++ = 0x80u | kDwarfRegD12; p += WriteULEB128(p, (cfa - 0x0A0) / 8);
      *p++ = 0x80u | kDwarfRegD13; p += WriteULEB128(p, (cfa - 0x0B0) / 8);
      *p++ = 0x80u | kDwarfRegD14; p += WriteULEB128(p, (cfa - 0x0C0) / 8);
      *p++ = 0x80u | kDwarfRegD15; p += WriteULEB128(p, (cfa - 0x0D0) / 8);

    } else if (func_info.stack_size == StackLayout::GUEST_STACK_SIZE) {
      // -----------------------------------------------------------------------
      // Guest frame.
      // The guest prolog saves x30 (host LR / return address) at
      // HOST_RET_ADDR = sp+0x040 from the *caller's* SP (i.e. from the
      // frame base before sub sp). After the sub sp instruction completes,
      // CFA = new_SP + GUEST_STACK_SIZE, and x30 lives at new_SP+0x040,
      // so the CFA-relative offset is (GUEST_STACK_SIZE - HOST_RET_ADDR).
      // -----------------------------------------------------------------------
      const size_t cfa       = func_info.stack_size;  // 80
      const size_t lr_offset = cfa - StackLayout::HOST_RET_ADDR;  // 80-64 = 16
      *p++ = 0x80u | kDwarfRegLR;
      p += WriteULEB128(p, lr_offset / 8);

    } else if (func_info.lr_save_offset > 0) {
      // -----------------------------------------------------------------------
      // Arbitrary guest-generated frame where the emitter explicitly recorded
      // where it saved x30.  lr_save_offset is the byte offset from SP (after
      // the stack allocation) where x30 was stored.
      //   CFA - save_address = stack_size - lr_save_offset
      // -----------------------------------------------------------------------
      const size_t lr_cfa_offset =
          func_info.stack_size - func_info.lr_save_offset;
      *p++ = 0x80u | kDwarfRegLR;
      p += WriteULEB128(p, lr_cfa_offset / 8);
    }
  }
  // If stack_size == 0 (leaf function), no CFA/register instructions needed.

  // Pad FDE body to pointer-size alignment.
  size_t fde_content_len = static_cast<size_t>(p - fde_content_start);
  size_t fde_padded_len  = xe::round_up(fde_content_len, sizeof(void*));
  while (p < fde_content_start + fde_padded_len) {
    *p++ = kDW_CFA_nop;
  }

  // Write FDE length (excludes the 4-byte length field itself).
  *reinterpret_cast<uint32_t*>(fde_length_ptr) =
      static_cast<uint32_t>(p - fde_content_start);

  // Terminator: a zero-length CIE/FDE signals end of .eh_frame section.
  *reinterpret_cast<uint32_t*>(p) = 0;
  p += 4;

  assert_true(static_cast<size_t>(p - unwind_entry_address) <=
              kMaxUnwindInfoSize);
}

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe
