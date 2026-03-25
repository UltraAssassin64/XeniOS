// Copyright 2026 Ben Vanik. All rights reserved.
// Released under the BSD license - see LICENSE in the root for more details.

#ifndef XENIA_BASE_MEMORY_FASTMEM_IOS_H_
#define XENIA_BASE_MEMORY_FASTMEM_IOS_H_

#if XE_PLATFORM_IOS && XE_ARCH_ARM64

namespace xe {
namespace memory {

// The "fastmem region" - a large contiguous VA space for fast memory access
// This is what Dolphin-iOS tests during initialization
// Size: 16 GiB (0x400000000 bytes)
constexpr size_t kFastmemRegionSize = 0x400000000ULL;

// Test if the iOS jailbroken device supports allocating large VA regions
// Returns true if allocation succeeded
bool TestFastmemAvailability();

// The actual fastmem region base (set after successful allocation)
// Only valid if TestFastmemAvailability() returned true
extern void* g_fastmem_region_base;
extern bool g_fastmem_available;

}  // namespace memory
}  // namespace xe

#endif  // XE_PLATFORM_IOS && XE_ARCH_ARM64

#endif  // XENIA_BASE_MEMORY_FASTMEM_IOS_H_