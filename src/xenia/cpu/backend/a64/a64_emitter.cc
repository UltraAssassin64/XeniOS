#include "xenia/cpu/backend/a64/a64_emitter.h"

#include <algorithm>
#include <cstring>

#include "xenia/cpu/backend/a64/a64_backend.h"  // Include full definition
#include "xenia/cpu/backend/a64/a64_op.h"

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

A64Emitter::A64Emitter(A64Backend* backend)
    : oaknut::VectorCodeGenerator(backend->code_cache()->code_buffer()),
      processor_(backend->processor()),
      code_cache_(backend->code_cache()) {}

A64Emitter::~A64Emitter() = default;

uint8_t* A64Emitter::CodeEnd() const { return GetCurr(); }

size_t A64Emitter::GetCodeSize() const { return GetCurrentCodeSize(); }

void A64Emitter::Reset() { ResetCurr(); }

bool A64Emitter::ProtectCode(uint8_t* ptr, size_t length) {
#if XE_PLATFORM_MAC
  sys_icache_invalidate(ptr, length);
#endif
  return true;
}

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe
