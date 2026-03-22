/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project
 ******************************************************************************
 */

#include "xenia/cpu/backend/a64/a64_backend.h"

#include <memory>

#include "xenia/base/assert.h"
#include "xenia/base/logging.h"

#include "xenia/cpu/backend/a64/a64_code_cache.h"
#include "xenia/cpu/backend/a64/a64_emitter.h"

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

A64Backend::A64Backend(cpu::Processor* processor) : Backend(processor) {}

A64Backend::~A64Backend() = default;

bool A64Backend::Initialize() {
  const size_t code_cache_size = 128 * 1024 * 1024;

  code_cache_ = std::make_unique<A64CodeCache>(code_cache_size);

  if (!code_cache_->Initialize()) {
    XELOGE("Failed to initialize A64 code cache");
    return false;
  }

  XELOGI("A64 code cache initialized (%zu MB)",
         code_cache_size / (1024 * 1024));

  return true;
}

void A64Backend::Shutdown() { code_cache_.reset(); }

std::unique_ptr<Assembler> A64Backend::CreateAssembler() {
  uint8_t* cache_ptr = code_cache_->data();

  if (!cache_ptr) {
    XELOGE("A64 backend: code cache unavailable");
    return nullptr;
  }

  auto emitter = std::make_unique<A64Emitter>(cache_ptr, code_cache_->size());

  return emitter;
}

void A64Backend::CommitCode() {
  if (!code_cache_) {
    return;
  }

  // Ensure instruction cache visibility after JIT compilation
#if defined(__APPLE__)
  __builtin___clear_cache(
      reinterpret_cast<char*>(code_cache_->data()),
      reinterpret_cast<char*>(code_cache_->data() + code_cache_->size()));
#else
#if defined(__GNUC__)
  __builtin___clear_cache(
      reinterpret_cast<char*>(code_cache_->data()),
      reinterpret_cast<char*>(code_cache_->data() + code_cache_->size()));
#endif
#endif
}

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe
