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
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/util/object_table.h"
#include "xenia/kernel/xobject.h"
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
  // Guest stack pointer and first two arguments at the last call, so a
  // thread blocked in a wait can be shown with its stack and wait object.
  std::atomic<uint32_t> guest_stack_pointer{0};
  std::atomic<uint32_t> guest_arg0{0};
  std::atomic<uint32_t> guest_arg1{0};
  std::string name;
};
std::mutex kernel_call_threads_mutex;
std::vector<KernelCallThreadState*> kernel_call_threads;
thread_local KernelCallThreadState* kernel_call_this_thread = nullptr;

// Guest return addresses up the stack from a frame: the back chain is at
// 0(r1) and each function saves its return address at -8 from its caller's
// frame.
std::string LogKernelCallStack(uint32_t kernel_call_frame) {
  Memory* kernel_call_memory = kernel_memory();
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
  return kernel_call_stack;
}

void LogKernelCallThreads() {
  const uint64_t kernel_call_now = Clock::QueryHostUptimeMillis();
  std::lock_guard<std::mutex> kernel_call_lock(kernel_call_threads_mutex);
  for (KernelCallThreadState* kernel_call_state : kernel_call_threads) {
    const cpu::Export* kernel_call_last = kernel_call_state->last_export;
    const bool kernel_call_inside = kernel_call_state->in_call;
    // ExTerminateThread does not return: the thread has exited.
    if (kernel_call_inside && kernel_call_last &&
        !std::strcmp(kernel_call_last->name, "ExTerminateThread")) {
      continue;
    }
    const uint64_t kernel_call_ms =
        kernel_call_now - kernel_call_state->enter_ms.load();
    XELOGW(
        "Kernel thread {:08X} ({}): {} {}{} for {} ms, guest lr {:08X}, {} "
        "calls",
        kernel_call_state->handle.load(), kernel_call_state->name,
        kernel_call_inside ? "inside" : "last called",
        kernel_call_last ? kernel_call_last->name : "nothing",
        kernel_call_inside ? "" : ", returned", kernel_call_ms,
        kernel_call_state->lr.load(), kernel_call_state->call_count.load());
    // A thread blocked in the kernel for a second or more: what it waits on
    // and how it got there. r3 is the handle for NtWaitForSingleObjectEx;
    // for other calls the object type is shown only if r3 happens to be a
    // handle.
    if (kernel_call_inside && kernel_call_ms >= 1000) {
      const uint32_t kernel_call_handle = kernel_call_state->guest_arg0;
      std::string kernel_call_object = "no object";
      if (auto kernel_call_xobject =
              kernel_state()->object_table()->LookupObject<XObject>(
                  kernel_call_handle)) {
        kernel_call_object = fmt::format("object type {} name \"{}\"",
                                         uint32_t(kernel_call_xobject->type()),
                                         kernel_call_xobject->name());
      }
      XELOGW("Kernel thread {:08X}: r3 {:08X} ({}) r4 {:08X}; guest stack{}",
             kernel_call_state->handle.load(), kernel_call_handle,
             kernel_call_object, kernel_call_state->guest_arg1.load(),
             LogKernelCallStack(kernel_call_state->guest_stack_pointer));
    }
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
  kernel_call_this_thread->guest_stack_pointer = uint32_t(ppc_context->r[1]);
  kernel_call_this_thread->guest_arg0 = uint32_t(ppc_context->r[3]);
  kernel_call_this_thread->guest_arg1 = uint32_t(ppc_context->r[4]);
  const uint64_t kernel_call_now = Clock::QueryHostUptimeMillis();
  kernel_call_this_thread->enter_ms = kernel_call_now;
  kernel_call_this_thread->in_call = true;
  const uint64_t kernel_call_total = ++kernel_call_this_thread->call_count;

  // The last 32 calls on this thread with their guest return addresses,
  // newest at index 0, and for each cycle length 1 to 8 how many calls in a
  // row have matched the call that many places earlier.
  thread_local const cpu::Export* kernel_call_history[32] = {};
  thread_local uint32_t kernel_call_history_lr[32] = {};
  thread_local uint32_t kernel_call_runs[9] = {};
  for (uint32_t kernel_call_period = 1; kernel_call_period <= 8;
       ++kernel_call_period) {
    if (kernel_call_history[kernel_call_period - 1] == export_entry) {
      ++kernel_call_runs[kernel_call_period];
    } else {
      kernel_call_runs[kernel_call_period] = 0;
    }
  }
  for (uint32_t kernel_call_index = 31; kernel_call_index;
       --kernel_call_index) {
    kernel_call_history[kernel_call_index] =
        kernel_call_history[kernel_call_index - 1];
    kernel_call_history_lr[kernel_call_index] =
        kernel_call_history_lr[kernel_call_index - 1];
  }
  kernel_call_history[0] = export_entry;
  kernel_call_history_lr[0] = uint32_t(ppc_context->lr);

  const uint32_t kernel_call_threshold = cvars::kernel_call_repeat_threshold;
  XThread* kernel_call_thread = XThread::GetCurrentThread();

  // A storm: a million calls in under a second, whatever their pattern (a
  // loop longer than eight calls, or one that varies, is not caught below).
  // Reported the first time and then every tenth time.
  thread_local uint64_t kernel_call_storm_ms = 0;
  thread_local uint32_t kernel_call_storm_reports = 0;
  if (!(kernel_call_total % 1000000)) {
    const uint64_t kernel_call_storm_elapsed =
        kernel_call_now - kernel_call_storm_ms;
    kernel_call_storm_ms = kernel_call_now;
    if (kernel_call_storm_elapsed < 1000 &&
        !(kernel_call_storm_reports++ % 10)) {
      std::string kernel_call_names;
      for (uint32_t kernel_call_index = 32; kernel_call_index;
           --kernel_call_index) {
        if (!kernel_call_history[kernel_call_index - 1]) {
          continue;
        }
        kernel_call_names += fmt::format(
            " {}@{:08X}", kernel_call_history[kernel_call_index - 1]->name,
            kernel_call_history_lr[kernel_call_index - 1]);
      }
      XELOGW(
          "Kernel call storm on thread {:08X}: 1000000 calls in {} ms ({} in "
          "total); r3 {:08X} r4 {:08X} r5 {:08X} r6 {:08X}; last 32 calls:{}",
          kernel_call_thread ? kernel_call_thread->handle() : 0,
          kernel_call_storm_elapsed, kernel_call_total,
          uint32_t(ppc_context->r[3]), uint32_t(ppc_context->r[4]),
          uint32_t(ppc_context->r[5]), uint32_t(ppc_context->r[6]),
          kernel_call_names);
      XELOGW("Kernel call storm: guest stack{}",
             LogKernelCallStack(uint32_t(ppc_context->r[1])));
      if (kernel_call_storm_reports == 1) {
        LogKernelCallThreads();
      }
    }
  }

  for (uint32_t kernel_call_period = 1; kernel_call_period <= 8;
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
    XELOGW(
        "Kernel calls repeating {} times in a row on thread {:08X}: {}; guest "
        "lr {:08X} r3 {:08X} r4 {:08X} r5 {:08X} r6 {:08X}",
        kernel_call_run, kernel_call_thread ? kernel_call_thread->handle() : 0,
        kernel_call_names, uint32_t(ppc_context->lr),
        uint32_t(ppc_context->r[3]), uint32_t(ppc_context->r[4]),
        uint32_t(ppc_context->r[5]), uint32_t(ppc_context->r[6]));
    if (kernel_call_run == kernel_call_threshold) {
      XELOGW("Kernel calls repeating: guest stack{}",
             LogKernelCallStack(uint32_t(ppc_context->r[1])));
      // The guest code around the caller, to see what the loop waits on.
      Memory* kernel_call_memory = kernel_memory();
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
