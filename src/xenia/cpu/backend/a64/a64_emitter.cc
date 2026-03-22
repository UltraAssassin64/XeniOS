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

A64Emitter::A64Emitter(A64Backend* backend)
    : processor_(backend->processor()),
      backend_(backend),
      code_cache_(backend->code_cache()) {
  feature_flags_ = 0;  // TODO: set features

  epilog_label_ = new oaknut::Label();
}

A64Emitter::~A64Emitter() = default;

uint8_t* A64Emitter::current_address() const { return getCode() + getSize(); }

size_t A64Emitter::offset() const { return getSize(); }

void A64Emitter::Reset() { reset(); }

void A64Emitter::FlushInstructionCache() {
#if defined(__APPLE__)
  sys_icache_invalidate(getCode(), getSize());
#else
#if defined(__GNUC__)
  __builtin___clear_cache(reinterpret_cast<char*>(getCode()),
                          reinterpret_cast<char*>(getCode() + getSize()));
#endif
#endif
}

void A64Emitter::Finalize() { FlushInstructionCache(); }

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe
