/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project
 ******************************************************************************
 */

#include "xenia/cpu/backend/a64/a64_emitter.h"

#include <cstring>

#if defined(__APPLE__)
#include <libkern/OSCacheControl.h>
#endif

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

A64Emitter::A64Emitter(uint8_t* buffer, size_t capacity)
    : buffer_(buffer), capacity_(capacity), offset_(0) {}

A64Emitter::~A64Emitter() = default;

uint32_t* A64Emitter::Emit(uint32_t instr) {
  if (offset_ + sizeof(uint32_t) > capacity_) {
    return nullptr;
  }

  uint32_t* ptr = reinterpret_cast<uint32_t*>(buffer_ + offset_);
  *ptr = instr;
  offset_ += sizeof(uint32_t);
  return ptr;
}

uint32_t* A64Emitter::EmitPair(uint32_t a, uint32_t b) {
  if (offset_ + sizeof(uint32_t) * 2 > capacity_) {
    return nullptr;
  }

  uint32_t* ptr = reinterpret_cast<uint32_t*>(buffer_ + offset_);
  ptr[0] = a;
  ptr[1] = b;

  offset_ += sizeof(uint32_t) * 2;
  return ptr;
}

uint8_t* A64Emitter::current_address() const {
  return buffer_ + offset_;
}

size_t A64Emitter::offset() const {
  return offset_;
}

void A64Emitter::Reset() {
  offset_ = 0;
}

void A64Emitter::FlushInstructionCache() {
#if defined(__APPLE__)
  sys_icache_invalidate(buffer_, offset_);
#else
#if defined(__GNUC__)
  __builtin___clear_cache(
      reinterpret_cast<char*>(buffer_),
      reinterpret_cast<char*>(buffer_ + offset_));
#endif
#endif
}

void A64Emitter::Finalize() {
  FlushInstructionCache();
}

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe