#include "xenia/cpu/backend/a64/a64_code_cache.h"

#include <cstring>
#include <sys/mman.h>

#if defined(__APPLE__)
#include <pthread.h>
#endif

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

A64CodeCache::A64CodeCache(size_t size)
    : size_(size),
      code_cache_(nullptr),
      code_cache_base_(0),
      code_cache_end_(0) {}

A64CodeCache::~A64CodeCache() {
  if (code_cache_) {
    munmap(code_cache_, size_);
    code_cache_ = nullptr;
  }
}

bool A64CodeCache::Initialize() {
  void* ptr = nullptr;

#if defined(__APPLE__)

  // Apple platforms require MAP_JIT for executable writable memory
  ptr = mmap(nullptr,
             size_,
             PROT_READ | PROT_WRITE | PROT_EXEC,
             MAP_PRIVATE | MAP_ANON | MAP_JIT,
             -1,
             0);

#else

  ptr = mmap(nullptr,
             size_,
             PROT_READ | PROT_WRITE | PROT_EXEC,
             MAP_PRIVATE | MAP_ANON,
             -1,
             0);

#endif

  if (ptr == MAP_FAILED) {
    return false;
  }

  code_cache_ = reinterpret_cast<uint8_t*>(ptr);

  code_cache_base_ = reinterpret_cast<uintptr_t>(code_cache_);
  code_cache_end_ = code_cache_base_ + size_;

#if defined(__APPLE__)

  // Enable execute-only mode after allocation
  pthread_jit_write_protect_np(1);

#endif

  return true;
}

uint8_t* A64CodeCache::data() const {
  return code_cache_;
}

uintptr_t A64CodeCache::base_address() const {
  return code_cache_base_;
}

uintptr_t A64CodeCache::end_address() const {
  return code_cache_end_;
}

size_t A64CodeCache::size() const {
  return size_;
}

void A64CodeCache::BeginWrite() {
#if defined(__APPLE__)
  pthread_jit_write_protect_np(0);
#endif
}

void A64CodeCache::EndWrite() {
#if defined(__APPLE__)
  pthread_jit_write_protect_np(1);
#endif
}

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe