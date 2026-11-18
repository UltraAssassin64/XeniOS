/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2015 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/util/shim_utils.h"

#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include "third_party/fmt/include/fmt/format.h"
#include "xenia/base/clock.h"
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

namespace {
// What each guest thread is doing in the kernel, readable from any thread so
// a stall report can show every thread. Entries are allocated once per host
// thread and never freed, so a report never reads freed memory.
struct KernelCallThreadState {
  std::atomic<const cpu::Export*> last_export{nullptr};
  std::atomic<uint64_t> call_count{0};
  std::atomic<uint64_t> enter_ms{0};
  std::atomic<bool> in_call{false};
  std::atomic<uint32_t> handle{0};
  std::atomic<uint32_t> lr{0};
  std::string name;
};
std::mutex kernel_call_threads_mutex;
std::vector<KernelCallThreadState*> kernel_call_threads;
thread_local KernelCallThreadState* kernel_call_this_thread = nullptr;

void LogKernelCallThreads() {
  const uint64_t kernel_call_now = Clock::QueryHostUptimeMillis();
  std::lock_guard<std::mutex> kernel_call_lock(kernel_call_threads_mutex);
  for (KernelCallThreadState* kernel_call_state : kernel_call_threads) {
    const cpu::Export* kernel_call_last = kernel_call_state->last_export;
    const bool kernel_call_inside = kernel_call_state->in_call;
    XELOGW(
        "Kernel thread {:08X} ({}): {} {}{} for {} ms, guest lr {:08X}, {} "
        "calls",
        kernel_call_state->handle.load(), kernel_call_state->name,
        kernel_call_inside ? "inside" : "last called",
        kernel_call_last ? kernel_call_last->name : "nothing",
        kernel_call_inside ? "" : ", returned",
        kernel_call_now - kernel_call_state->enter_ms.load(),
        kernel_call_state->lr.load(), kernel_call_state->call_count.load());
  }
}
}  // namespace

void NoteKernelCallDone() {
  if (kernel_call_this_thread) {
    kernel_call_this_thread->enter_ms = Clock::QueryHostUptimeMillis();
    kernel_call_this_thread->in_call = false;
  }
}

void NoteKernelCall(const cpu::Export* export_entry, PPCContext* ppc_context) {
  if (!kernel_call_this_thread) {
    kernel_call_this_thread = new KernelCallThreadState();
    if (XThread* kernel_call_owner = XThread::GetCurrentThread()) {
      kernel_call_this_thread->handle = kernel_call_owner->handle();
      kernel_call_this_thread->name = kernel_call_owner->name();
    }
    std::lock_guard<std::mutex> kernel_call_lock(kernel_call_threads_mutex);
    kernel_call_threads.push_back(kernel_call_this_thread);
  }
  kernel_call_this_thread->last_export = export_entry;
  kernel_call_this_thread->lr = uint32_t(ppc_context->lr);
  kernel_call_this_thread->enter_ms = Clock::QueryHostUptimeMillis();
  kernel_call_this_thread->in_call = true;
  ++kernel_call_this_thread->call_count;

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
    // A thread that only sleeps is idle, not stuck: stay quiet about it.
    bool kernel_call_only_sleeps = true;
    for (uint32_t kernel_call_index = 0; kernel_call_index < kernel_call_period;
         ++kernel_call_index) {
      if (std::strcmp(kernel_call_history[kernel_call_index]->name,
                      "KeDelayExecutionThread")) {
        kernel_call_only_sleeps = false;
      }
    }
    if (kernel_call_only_sleeps) {
      break;
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
      // The guest code around the caller, to see what the loop waits on.
      const uint32_t kernel_call_code =
          (uint32_t(ppc_context->lr) - 0x80) & ~3u;
      for (uint32_t kernel_call_line = 0; kernel_call_line < 64;
           kernel_call_line += 8) {
        std::string kernel_call_words;
        for (uint32_t kernel_call_word = kernel_call_line;
             kernel_call_word < kernel_call_line + 8; ++kernel_call_word) {
          kernel_call_words += fmt::format(
              " {:08X}",
              xe::load_and_swap<uint32_t>(kernel_call_memory->TranslateVirtual(
                  kernel_call_code + kernel_call_word * 4)));
        }
        XELOGW("Kernel calls repeating: code {:08X}:{}",
               kernel_call_code + kernel_call_line * 4, kernel_call_words);
      }
    }
    // At the first report and then at every tenth threshold: what every
    // guest thread is doing in the kernel, to find the one this thread is
    // waiting on.
    if (!(kernel_call_run % (kernel_call_threshold * 10)) ||
        kernel_call_run == kernel_call_threshold) {
      LogKernelCallThreads();
    }
    break;
  }
}

}  // namespace shim
}  // namespace kernel
}  // namespace xe
