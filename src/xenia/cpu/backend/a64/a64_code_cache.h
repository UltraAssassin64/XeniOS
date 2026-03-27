/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2024 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_CPU_BACKEND_A64_A64_CODE_CACHE_H_
#define XENIA_CPU_BACKEND_A64_A64_CODE_CACHE_H_

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "xenia/base/memory.h"
#include "xenia/base/mutex.h"
#include "xenia/cpu/backend/code_cache.h"

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

#if XE_ARCH_ARM64
#define XE_A64_INDIRECTION_64BIT 1
#else
#define XE_A64_INDIRECTION_64BIT 0
#endif

struct EmitFunctionInfo {
  struct _code_size {
    size_t prolog;
    size_t body;
    size_t epilog;
    size_t tail;
    size_t total;
  } code_size;
  size_t prolog_stack_alloc_offset;  // byte offset of instr after stack alloc
  size_t stack_size;
#if XE_ARCH_ARM64
  // Byte offset from the post-alloc SP where x30 (LR) is saved by the prolog.
  // Used by the POSIX DWARF .eh_frame generator to tell libunwind where the
  // host return address lives.  Set to 0 for thunk and leaf frames (those are
  // handled via fixed StackLayout constants in InitializeUnwindEntry).
  size_t lr_save_offset = 0;
#endif
};
enum JitType {
    Legacy,
    LuckNoTXM,
    LuckTXM
};
class A64CodeCache : public CodeCache {
 public:
  ~A64CodeCache() override;

  static std::unique_ptr<A64CodeCache> Create();

  virtual bool Initialize();

  const std::filesystem::path& file_name() const override { return file_name_; }
  uintptr_t execute_base_address() const override {
    return generated_code_execute_base_
               ? reinterpret_cast<uintptr_t>(generated_code_execute_base_)
               : kGeneratedCodeExecuteBase;
  }
  size_t total_size() const override { return kGeneratedCodeSize; }

  bool has_indirection_table() { return indirection_table_base_ != nullptr; }
  void set_indirection_default(uint32_t default_value);
#if XE_A64_INDIRECTION_64BIT
  void set_indirection_default_64(uint64_t default_value);
#endif
  void AddIndirection(uint32_t guest_address, uint32_t host_address);
#if XE_A64_INDIRECTION_64BIT
  void AddIndirection64(uint32_t guest_address, uint64_t host_address);
#endif

  void CommitExecutableRange(uint32_t guest_low, uint32_t guest_high);

  void PlaceHostCode(uint32_t guest_address, void* machine_code,
                     const EmitFunctionInfo& func_info,
                     void*& code_execute_address_out,
                     void*& code_write_address_out);
  void PlaceGuestCode(uint32_t guest_address, void* machine_code,
                      const EmitFunctionInfo& func_info,
                      GuestFunction* function_info,
                      void*& code_execute_address_out,
                      void*& code_write_address_out);
  uint32_t PlaceData(const void* data, size_t length);

  GuestFunction* LookupFunction(uint64_t host_pc) override;

  // Access to indirection table base for the emitter.
  uint8_t* indirection_table_base() const { return indirection_table_base_; }

  // Actual VA of the indirection table (may differ from kIndirectionTableBase
  // on systems where fixed-address allocation fails, e.g. iOS).
  uintptr_t indirection_table_base_address() const {
    return indirection_table_actual_base_;
  }
#if XE_A64_INDIRECTION_64BIT
  uintptr_t indirection_table_base_bias() const {
    return indirection_table_base_bias_;
  }
  uintptr_t external_indirection_table_base_address() const {
    return reinterpret_cast<uintptr_t>(external_indirection_targets_.get());
  }
#endif

 public:
  // All executable code falls within 0x80000000–0x9FFFFFFF, so we only need
  // enough table space for lookups in that range.
  //
  // On ARM64, entries are 32-bit relative offsets from the code-cache execute
  // base (plus tagged external targets for trampolines). This keeps dispatch
  // O(1) while minimising the contiguous VA reservation on constrained iOS
  // devices.
#if XE_A64_INDIRECTION_64BIT
  static const size_t kIndirectionTableSize = 0x20000000;  // 512 MiB
#else
  static const size_t kIndirectionTableSize =
      0x20000000 - 1;  // 512 MiB - 1 (legacy)
#endif

#if XE_A64_INDIRECTION_64BIT
  // Set dynamically at runtime: the OS picks the VA on iOS.
  static uintptr_t kIndirectionTableBase;
#else
  static const uintptr_t kIndirectionTableBase = 0x80000000;
#endif

  // 256 MiB code cache — more than enough for the dozens of MB games
  // typically JIT-compile.
  static const size_t    kGeneratedCodeSize        = 0x0FFFFFFF;
  static const uintptr_t kGeneratedCodeExecuteBase = 0xA0000000;
  // Write alias used when PageAccess::kExecuteReadWrite is not available.
  static const uintptr_t kGeneratedCodeWriteBase =
      kGeneratedCodeExecuteBase + kGeneratedCodeSize + 1;

  // Upper bound on simultaneously live guest functions. Raise if analysis
  // generates unusually many small functions.
  static const size_t kMaximumFunctionCount = 100000;

  struct UnwindReservation {
    size_t   data_size     = 0;
    size_t   table_slot    = 0;
    uint8_t* entry_address = 0;
  };

  A64CodeCache();

  virtual UnwindReservation RequestUnwindReservation(uint8_t* entry_address) {
    return UnwindReservation();
  }
  virtual void PlaceCode(uint32_t guest_address, void* machine_code,
                         const EmitFunctionInfo& func_info,
                         void* code_execute_address,
                         UnwindReservation unwind_reservation) {}

  // Platform-specific code-copy hook. On POSIX/Apple this also flushes the
  // I-cache so the execute alias sees the new instructions.
  virtual void CopyMachineCode(void* dest, const void* src, size_t size) {
    std::memcpy(dest, src, size);
  }

  std::filesystem::path        file_name_;
  xe::memory::FileMappingHandle mapping_ =
      xe::memory::kFileMappingHandleInvalid;

  // Must hold the global critical region when modifying offsets/counts.
  xe::global_critical_region global_critical_region_;

  // Value used to initialise freshly committed indirection-table pages.
  uint32_t indirection_default_value_ = 0xFEEDF00D;

#if XE_A64_INDIRECTION_64BIT
  // rel32 entries for in-cache targets; tagged external-table index for
  // out-of-cache targets (e.g. guest trampolines).
  using indirection_entry_t = uint32_t;
  static constexpr size_t   kIndirectionEntrySize         = 4;
  static constexpr uint32_t kIndirectionExternalTag        = 0x80000000u;
  static constexpr uint32_t kIndirectionExternalIndexMask  = 0x7FFFFFFFu;
  static constexpr uint32_t kIndirectionExternalCapacity   = 0x00010000u;
#else
  using indirection_entry_t = uint32_t;
  static constexpr size_t kIndirectionEntrySize = 4;
#endif

  uint8_t*  indirection_table_base_        = nullptr;
  uintptr_t indirection_table_actual_base_ = 0;
#if XE_A64_INDIRECTION_64BIT
  uintptr_t                   indirection_table_base_bias_ = 0;
  std::unique_ptr<uint64_t[]> external_indirection_targets_;
  std::atomic<uint32_t>       external_indirection_target_count_ = {0};
  std::mutex                  external_indirection_mutex_;

  uint32_t EncodeIndirectionTarget(uint64_t host_address);
#endif

  uint8_t* generated_code_execute_base_ = nullptr;
  uint8_t* generated_code_write_base_   = nullptr;
  // True when dual-mapping was created via vm_remap (iOS fallback). In this
  // mode pages are fully mapped at setup time; incremental commit/protect
  // calls are skipped.
  bool generated_code_uses_vm_remap_fallback_ = false;
  // True when iOS uses single-view mprotect flips (RW↔RX) rather than a
  // dual-alias mapping.
  bool generated_code_uses_mprotect_flip_ = false;
  size_t generated_code_offset_ = 0;
  std::atomic<size_t> generated_code_commit_mark_ = {0};
  // Sorted by host-PC base offset → guest function. Used for bsearch in
  // LookupFunction. Key encoding: [start_offset_u32 | end_offset_u32].
  std::vector<std::pair<uint64_t, GuestFunction*>> generated_code_map_;

#ifdef XE_PLATFORM_IOS
  // iOS JIT strategy resolved at Initialize() time.
  // Helpers for the mprotect-flip W^X path (defined in a64_code_cache.cc).
  bool RegionLockRead(void* address, size_t length);
  bool RegionUnlockWrite(void* address, size_t length);
  bool RegionSetExec(void* address, size_t length);
#endif  // XE_PLATFORM_IOS
};

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe

#endif  // XENIA_CPU_BACKEND_A64_A64_CODE_CACHE_H_
