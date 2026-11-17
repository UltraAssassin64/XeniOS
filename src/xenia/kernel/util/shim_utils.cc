/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2015 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/util/shim_utils.h"

#include <string>

#include "third_party/fmt/include/fmt/format.h"
#include "xenia/kernel/kernel_flags.h"
#include "xenia/kernel/xthread.h"
namespace xe {
namespace kernel {
namespace shim {

thread_local StringBuffer string_buffer_;

StringBuffer* thread_local_string_buffer() { return &string_buffer_; }

XThread* ContextParam::CurrentXThread() const {
  return XThread::GetCurrentThread();
}

void NoteKernelCall(const cpu::Export* export_entry, PPCContext* ppc_context) {
  // The last eight calls on this thread, newest at index 0, and for each
  // cycle length 1 to 4 how many calls in a row have matched the call that
  // many places earlier.
  thread_local const cpu::Export* kernel_call_history[8] = {};
  thread_local uint32_t kernel_call_runs[5] = {};
  for (uint32_t kernel_call_period = 1; kernel_call_period <= 4;
       ++kernel_call_period) {
    if (kernel_call_history[kernel_call_period - 1] == export_entry) {
      ++kernel_call_runs[kernel_call_period];
    } else {
      kernel_call_runs[kernel_call_period] = 0;
    }
  }
  for (uint32_t kernel_call_index = 7; kernel_call_index; --kernel_call_index) {
    kernel_call_history[kernel_call_index] =
        kernel_call_history[kernel_call_index - 1];
  }
  kernel_call_history[0] = export_entry;

  const uint32_t kernel_call_threshold = cvars::kernel_call_repeat_threshold;
  for (uint32_t kernel_call_period = 1; kernel_call_period <= 4;
       ++kernel_call_period) {
    const uint32_t kernel_call_run = kernel_call_runs[kernel_call_period];
    // A cycle of length p also matches at 2p, 3p, ...: report only the
    // shortest one, once at the threshold and then every 100 thresholds.
    if (kernel_call_run != kernel_call_threshold &&
        (kernel_call_run < kernel_call_threshold ||
         kernel_call_run % (kernel_call_threshold * 100))) {
      continue;
    }
    std::string kernel_call_names;
    for (uint32_t kernel_call_index = kernel_call_period; kernel_call_index;
         --kernel_call_index) {
      if (!kernel_call_names.empty()) {
        kernel_call_names += " -> ";
      }
      kernel_call_names += kernel_call_history[kernel_call_index - 1]->name;
    }
    XThread* kernel_call_thread = XThread::GetCurrentThread();
    XELOGW(
        "Kernel calls repeating {} times in a row on thread {:08X}: {}; guest "
        "lr {:08X} r3 {:08X} r4 {:08X} r5 {:08X} r6 {:08X}",
        kernel_call_run, kernel_call_thread ? kernel_call_thread->handle() : 0,
        kernel_call_names, uint32_t(ppc_context->lr),
        uint32_t(ppc_context->r[3]), uint32_t(ppc_context->r[4]),
        uint32_t(ppc_context->r[5]), uint32_t(ppc_context->r[6]));
    if (kernel_call_run == kernel_call_threshold) {
      // Guest stack: the back chain is at 0(r1) and each function saves its
      // return address at -8 from its caller's frame.
      Memory* kernel_call_memory = kernel_memory();
      uint32_t kernel_call_frame = uint32_t(ppc_context->r[1]);
      std::string kernel_call_stack;
      for (uint32_t kernel_call_depth = 0; kernel_call_depth < 12;
           ++kernel_call_depth) {
        if (!kernel_call_frame || kernel_call_frame >= 0xF0000000u) {
          break;
        }
        uint32_t kernel_call_caller_frame = xe::load_and_swap<uint32_t>(
            kernel_call_memory->TranslateVirtual(kernel_call_frame));
        if (kernel_call_caller_frame <= kernel_call_frame ||
            kernel_call_caller_frame >= 0xF0000000u) {
          break;
        }
        kernel_call_stack += fmt::format(
            " {:08X}",
            xe::load_and_swap<uint32_t>(kernel_call_memory->TranslateVirtual(
                kernel_call_caller_frame - 8)));
        kernel_call_frame = kernel_call_caller_frame;
      }
      XELOGW("Kernel calls repeating: guest stack{}", kernel_call_stack);
    }
    break;
  }
}

}  // namespace shim
}  // namespace kernel
}  // namespace xe
