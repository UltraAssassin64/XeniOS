// src/xenia/cpu/backend/a64/a64_jit_broker.cc
//
// Companion translation unit for a64_code_cache.cc.
// Provides the iOS JIT-broker helpers forward-declared there.

#include "xenia/cpu/backend/a64/a64_code_cache.h"

#ifdef XE_PLATFORM_IOS

#include <atomic>
#include <cstdlib>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <unistd.h>

#include "xenia/base/cvar.h"
#include "xenia/base/logging.h"

// Pull in the cvars declared in a64_code_cache.cc.
DECLARE_bool(ios_jit_brk_prepare_fallback);
DECLARE_bool(ios_jit_brk_use_universal_0xf00d);

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

// ---------------------------------------------------------------------------
// Atomic handshake flags referenced by a64_code_cache.cc
// ---------------------------------------------------------------------------
std::atomic<bool> ios_external_prepare_issued{false};
std::atomic<bool> ios_external_detach_issued{false};
std::atomic<bool> ios_force_universal_prepare_command{false};

// ---------------------------------------------------------------------------
// IOSProductMajorVersion
// Returns the major iOS version integer (e.g. 26 for iOS 26.2).
// ---------------------------------------------------------------------------
int IOSProductMajorVersion() {
  // sysctlbyname is available on all iOS versions and doesn't require ObjC.
  // "kern.osproductversion" returns a string like "26.2.0".
  char buf[64] = {};
  size_t len = sizeof(buf);
  if (sysctlbyname("kern.osproductversion", buf, &len, nullptr, 0) != 0) {
    return 0;
  }
  return std::atoi(buf);  // atoi stops at the first non-digit ('.')
}

// ---------------------------------------------------------------------------
// IOSUseTXMBrokerPath
// Returns true when the TXM hardware JIT path should be used.
// Reads the HAS_TXM env-var that the CI/runtime can inject, then falls
// back to version-based heuristics.
// ---------------------------------------------------------------------------
bool IOSUseTXMBrokerPath() {
  if (const char* env = std::getenv("HAS_TXM")) {
    if (env[0] == '1' && env[1] == '\0') return true;
    if (env[0] == '0' && env[1] == '\0') return false;
  }
  // No TXM hardware detection available at compile time; default to false.
  // Devices with TXM should set HAS_TXM=1 in the launch environment.
  return false;
}

// ---------------------------------------------------------------------------
// ExternalPrepareBreakpointDescription
// Returns a human-readable label for the breakpoint being issued.
// ---------------------------------------------------------------------------
const char* ExternalPrepareBreakpointDescription() {
  const bool use_universal =
      cvars::ios_jit_brk_use_universal_0xf00d ||
      ios_force_universal_prepare_command.load(std::memory_order_relaxed);
  return use_universal ? "brk #0xf00d (x16=1)" : "brk #0x69";
}

// ---------------------------------------------------------------------------
// MaybeRequestExternalJitPrepare
// Issues a breakpoint that signals an external JIT broker (TrollStore /
// StikDebug / AltJIT) to mark [address, address+length) as executable.
// Returns false if the fallback is disabled or the breakpoint is not usable.
// ---------------------------------------------------------------------------
bool MaybeRequestExternalJitPrepare(void* address, size_t length) {
  if (!cvars::ios_jit_brk_prepare_fallback) {
    return false;
  }

  const bool use_universal =
      cvars::ios_jit_brk_use_universal_0xf00d ||
      ios_force_universal_prepare_command.load(std::memory_order_relaxed);

  ios_external_prepare_issued.store(true, std::memory_order_release);

  if (use_universal) {
    // Universal broker: brk #0xf00d with x16=1 (StikDebug / modern path).
    __asm__ volatile(
        "mov x16, #1\n\t"
        "brk #0xf00d\n\t"
        ::: "x16", "memory");
  } else {
    // Legacy broker: brk #0x69 (older TrollStore JIT path).
    __asm__ volatile("brk #0x69\n\t" ::: "memory");
  }

  return true;
}

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe

#endif  // XE_PLATFORM_IOS