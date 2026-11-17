/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2026 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

// Stand-ins for the parts of the emulator that tools/ci/build_cpu_tests.py
// leaves out of the CPU test programs: the system dialogs in xenia-base (which
// use SDL) and XexModule (which needs the kernel). The tests only run
// TestModule and raw binaries, so no XexModule is ever created; the code that
// asks "is this module a XexModule?" still has to link.

#include <cstdio>
#include <string>

#include "xenia/base/system.h"
#include "xenia/cpu/function.h"
#include "xenia/cpu/xex_module.h"

namespace xe {

void LaunchWebBrowser(const std::string_view url) {}

void LaunchFileExplorer(const std::filesystem::path& path) {}

void ShowSimpleMessageBox(SimpleMessageBoxType type, std::string_view message) {
  std::fprintf(stderr, "%.*s\n", static_cast<int>(message.size()),
               message.data());
}

bool SetProcessPriorityClass(const uint32_t priority_class) { return true; }

bool IsUseNexusForGameBarEnabled() { return false; }

namespace cpu {

XexModule::~XexModule() = default;

bool XexModule::GetOptHeader(xex2_header_keys key, void** out_ptr) const {
  return false;
}

bool XexModule::ContainsAddress(uint32_t address) { return false; }

InfoCacheFlags* XexModule::GetInstructionAddressFlags(uint32_t guest_addr) {
  return nullptr;
}

void XexModule::FlushInfoCache() {}

void XexModule::Precompile() {}

std::unique_ptr<Function> XexModule::CreateFunction(uint32_t address) {
  return nullptr;
}

}  // namespace cpu
}  // namespace xe
