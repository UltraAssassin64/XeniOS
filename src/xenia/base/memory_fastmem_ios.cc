// Copyright 2026 Ben Vanik. All rights reserved.
// Released under the BSD license - see LICENSE in the root for more details.

#include "xenia/base/memory_fastmem_ios.h"

#if XE_PLATFORM_IOS && XE_ARCH_ARM64

#include <mach/mach.h>

#include "xenia/base/logging.h"

namespace xe {
namespace memory {

// Global fastmem state
void* g_fastmem_region_base = nullptr;
bool g_fastmem_available = false;

bool TestFastmemAvailability() {
  // Attempt to allocate the fastmem region
  // This requires a jailbroken device with extended VA space access
  // (via dynamic-codesigning entitlement or arm64_maxoffset boot arg)
  
  vm_address_t addr = 0;
  kern_return_t retval = vm_allocate(mach_task_self(), &addr, kFastmemRegionSize,
                                     VM_FLAGS_ANYWHERE);
  
  if (retval != KERN_SUCCESS) {
    XELOGI("Fastmem not available (vm_allocate failed: kr={})", retval);
    g_fastmem_available = false;
    g_fastmem_region_base = nullptr;
    return false;
  }
  
  // Successfully allocated - store the base and deallocate
  g_fastmem_region_base = reinterpret_cast<void*>(addr);
  
  // Deallocate the test region (we'll properly allocate it later during setup)
  vm_deallocate(mach_task_self(), addr, kFastmemRegionSize);
  
  g_fastmem_available = true;
  XELOGI("Fastmem available: 0x{:X} bytes at 0x{:X}",
         static_cast<uint32_t>(kFastmemRegionSize),
         reinterpret_cast<uintptr_t>(g_fastmem_region_base));
  
  return true;
}

}  // namespace memory
}  // namespace xe

#endif  // XE_PLATFORM_IOS && XE_ARCH_ARM64