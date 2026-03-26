// Copyright 2026 Ben Vanik. All rights reserved.
// Released under the BSD license - see LICENSE in the root for more details.

#ifndef XENIA_BASE_MEMORY_FASTMEM_IOS_H_
#define XENIA_BASE_MEMORY_FASTMEM_IOS_H_

#if XE_PLATFORM_IOS && XE_ARCH_ARM64

// NOTE: This header is included from inside namespace xe::memory in memory.h.
// Do NOT open any namespaces here — declarations go directly into xe::memory.

constexpr size_t kFastmemRegionSize = 0x400000000ULL;  // 16 GiB

bool TestFastmemAvailability();

extern void* g_fastmem_region_base;
extern bool g_fastmem_available;

#endif  // XE_PLATFORM_IOS && XE_ARCH_ARM64

#endif  // XENIA_BASE_MEMORY_FASTMEM_IOS_H_