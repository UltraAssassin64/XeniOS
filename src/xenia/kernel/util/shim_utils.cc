/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2015 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/util/shim_utils.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "xenia/base/platform.h"
#if XE_PLATFORM_APPLE && XE_ARCH_ARM64
#include <mach/mach.h>
#endif
#if !XE_PLATFORM_WIN32
#include <dlfcn.h>
#endif

#include "third_party/fmt/include/fmt/format.h"
#include "xenia/base/clock.h"
#include "xenia/base/mutex.h"
#include "xenia/base/threading.h"
#include "xenia/cpu/backend/backend.h"
#include "xenia/cpu/backend/code_cache.h"
#include "xenia/cpu/function.h"
#include "xenia/cpu/processor.h"
#include "xenia/kernel/kernel_flags.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/util/object_table.h"
#include "xenia/kernel/xevent.h"
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

}  // namespace

// Guest return addresses up the stack from a frame: the back chain is at
// 0(r1) and each function saves its return address at -8 from its caller's
// frame.
// Whether guest memory [address, address + size) is committed and readable.
// A thread that has exited may have had its stack released, and reading a
// released page faults. The caller holds the global critical region.
bool KernelCallReadable(uint32_t address, uint32_t size) {
  BaseHeap* kernel_call_heap = kernel_memory()->LookupHeap(address);
  if (!kernel_call_heap) {
    return false;
  }
  return kernel_call_heap->QueryRangeAccess(address, address + size - 1) !=
         xe::memory::PageAccess::kNoAccess;
}

std::string LogKernelCallStack(uint32_t kernel_call_frame) {
  Memory* kernel_call_memory = kernel_memory();
  std::string kernel_call_stack;
  // The page table is guarded by the global lock; a stuck thread may hold
  // it, so never wait for it here.
  auto& kernel_call_global = xe::global_critical_region::mutex();
  if (!kernel_call_global.try_lock()) {
    return " (memory busy)";
  }
  for (uint32_t kernel_call_depth = 0; kernel_call_depth < 12;
       ++kernel_call_depth) {
    if (!kernel_call_frame || kernel_call_frame >= 0xF0000000u) {
      break;
    }
    if (!KernelCallReadable(kernel_call_frame, 4)) {
      kernel_call_stack +=
          fmt::format(" (unreadable {:08X})", kernel_call_frame);
      break;
    }
    uint32_t kernel_call_caller_frame = xe::load_and_swap<uint32_t>(
        kernel_call_memory->TranslateVirtual(kernel_call_frame));
    if (kernel_call_caller_frame <= kernel_call_frame ||
        kernel_call_caller_frame >= 0xF0000000u ||
        !KernelCallReadable(kernel_call_caller_frame - 8, 4)) {
      break;
    }
    kernel_call_stack += fmt::format(
        " {:08X}",
        xe::load_and_swap<uint32_t>(kernel_call_memory->TranslateVirtual(
            kernel_call_caller_frame - 8)));
    kernel_call_frame = kernel_call_caller_frame;
  }
  kernel_call_global.unlock();
  return kernel_call_stack;
}

void LogKernelCallStackCode(uint32_t kernel_call_lr,
                            uint32_t kernel_call_frame) {
  Memory* kernel_call_memory = kernel_memory();
  auto& kernel_call_global = xe::global_critical_region::mutex();
  if (!kernel_call_global.try_lock()) {
    return;
  }
  // The caller, then each return address up the stack (as
  // LogKernelCallStack walks it).
  std::vector<uint32_t> kernel_call_returns = {kernel_call_lr};
  for (uint32_t kernel_call_depth = 0; kernel_call_depth < 12;
       ++kernel_call_depth) {
    if (!kernel_call_frame || kernel_call_frame >= 0xF0000000u ||
        !KernelCallReadable(kernel_call_frame, 4)) {
      break;
    }
    uint32_t kernel_call_caller_frame = xe::load_and_swap<uint32_t>(
        kernel_call_memory->TranslateVirtual(kernel_call_frame));
    if (kernel_call_caller_frame <= kernel_call_frame ||
        kernel_call_caller_frame >= 0xF0000000u ||
        !KernelCallReadable(kernel_call_caller_frame - 8, 4)) {
      break;
    }
    kernel_call_returns.push_back(xe::load_and_swap<uint32_t>(
        kernel_call_memory->TranslateVirtual(kernel_call_caller_frame - 8)));
    kernel_call_frame = kernel_call_caller_frame;
  }
  // 64 words before each return address and 8 after, each address once per
  // run, so a stuck loop's condition can be disassembled from the log.
  static std::unordered_set<uint32_t> kernel_call_code_logged;
  for (uint32_t kernel_call_return : kernel_call_returns) {
    const uint32_t kernel_call_code = (kernel_call_return - 0x100) & ~3u;
    if (kernel_call_return < 0x80000000u || kernel_call_return >= 0xF0000000u ||
        !kernel_call_code_logged.insert(kernel_call_return).second ||
        !KernelCallReadable(kernel_call_code, 0x120)) {
      continue;
    }
    for (uint32_t kernel_call_line = 0; kernel_call_line < 72;
         kernel_call_line += 8) {
      std::string kernel_call_words;
      for (uint32_t kernel_call_word = kernel_call_line;
           kernel_call_word < kernel_call_line + 8; ++kernel_call_word) {
        kernel_call_words += fmt::format(
            " {:08X}",
            xe::load_and_swap<uint32_t>(kernel_call_memory->TranslateVirtual(
                kernel_call_code + kernel_call_word * 4)));
      }
      XELOGW("Kernel call storm: code for {:08X} at {:08X}:{}",
             kernel_call_return, kernel_call_code + kernel_call_line * 4,
             kernel_call_words);
    }
  }
  kernel_call_global.unlock();
}

namespace {

// The host program counter of another thread, read while it is stopped for
// a moment; nothing that could take a lock runs while it is stopped. Apple
// ARM64 only; 0 elsewhere or on failure.
uint64_t SampleKernelCallHostPc(
    xe::threading::Thread* kernel_call_host_thread) {
  uint64_t kernel_call_host_pc = 0;
#if XE_PLATFORM_APPLE && XE_ARCH_ARM64
  const mach_port_t kernel_call_port =
      mach_port_t(kernel_call_host_thread->system_id());
  if (kernel_call_port &&
      kernel_call_port != xe::threading::current_thread_system_id() &&
      thread_suspend(kernel_call_port) == KERN_SUCCESS) {
    arm_thread_state64_t kernel_call_arm_state;
    mach_msg_type_number_t kernel_call_state_count = ARM_THREAD_STATE64_COUNT;
    if (thread_get_state(
            kernel_call_port, ARM_THREAD_STATE64,
            reinterpret_cast<thread_state_t>(&kernel_call_arm_state),
            &kernel_call_state_count) == KERN_SUCCESS) {
      kernel_call_host_pc =
          uint64_t(arm_thread_state64_get_pc(kernel_call_arm_state));
    }
    thread_resume(kernel_call_port);
  }
#endif
  return kernel_call_host_pc;
}

}  // namespace

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
      // The object table shares the global lock with the kernel; a stuck
      // thread may hold it, so never wait for it here.
      auto& kernel_call_global = xe::global_critical_region::mutex();
      if (kernel_call_global.try_lock()) {
        if (auto kernel_call_xobject =
                kernel_state()->object_table()->LookupObject<XObject>(
                    kernel_call_handle, true)) {
          kernel_call_object =
              fmt::format("object type {} name \"{}\"",
                          uint32_t(kernel_call_xobject->type()),
                          kernel_call_xobject->name());
          if (kernel_call_xobject->type() == XObject::Type::Event) {
            kernel_call_object +=
                ", " + static_cast<XEvent*>(kernel_call_xobject.get())
                           ->DescribeLastSignal();
          }
        }
        kernel_call_global.unlock();
      } else {
        kernel_call_object = "object table busy";
      }
      XELOGW("Kernel thread {:08X}: r3 {:08X} ({}) r4 {:08X}; guest stack{}",
             kernel_call_state->handle.load(), kernel_call_handle,
             kernel_call_object, kernel_call_state->guest_arg1.load(),
             LogKernelCallStack(kernel_call_state->guest_stack_pointer));
      // NtWaitForMultipleObjectsEx: r3 is the count, r4 the handle array.
      if (kernel_call_last &&
          !std::strcmp(kernel_call_last->name, "NtWaitForMultipleObjectsEx") &&
          kernel_call_handle && kernel_call_handle <= 64 &&
          kernel_call_global.try_lock()) {
        std::string kernel_call_wait_objects;
        const uint32_t kernel_call_handles = kernel_call_state->guest_arg1;
        if (KernelCallReadable(kernel_call_handles, kernel_call_handle * 4)) {
          for (uint32_t i = 0; i < kernel_call_handle; ++i) {
            const uint32_t kernel_call_wait_handle =
                xe::load_and_swap<uint32_t>(
                    kernel_memory()->TranslateVirtual(kernel_call_handles) +
                    i * 4);
            auto kernel_call_wait_object =
                kernel_state()->object_table()->LookupObject<XObject>(
                    kernel_call_wait_handle, true);
            fmt::format_to(std::back_inserter(kernel_call_wait_objects),
                           " {:08X} ", kernel_call_wait_handle);
            if (kernel_call_wait_object) {
              fmt::format_to(
                  std::back_inserter(kernel_call_wait_objects),
                  "(type {} name \"{}\"{})",
                  uint32_t(kernel_call_wait_object->type()),
                  kernel_call_wait_object->name(),
                  kernel_call_wait_object->type() == XObject::Type::Event
                      ? ", " +
                            static_cast<XEvent*>(kernel_call_wait_object.get())
                                ->DescribeLastSignal()
                      : std::string());
            } else {
              kernel_call_wait_objects += "(no object)";
            }
          }
        } else {
          kernel_call_wait_objects = " (array unreadable)";
        }
        kernel_call_global.unlock();
        XELOGW("Kernel thread {:08X}: waits on{}",
               kernel_call_state->handle.load(), kernel_call_wait_objects);
      }
    }
  }

  // Every guest thread the kernel knows, including ones that have never made
  // a kernel call (a thread that only runs game code, or never started):
  // whether it runs, its suspend count and priority, and where its host
  // thread is right now, mapped back to the guest function.
  auto& kernel_call_global = xe::global_critical_region::mutex();
  if (!kernel_call_global.try_lock()) {
    XELOGW("Guest threads: object table busy");
    return;
  }
  auto kernel_call_xthreads =
      kernel_state()->object_table()->GetObjectsByType<XThread>();
  kernel_call_global.unlock();
  for (auto& kernel_call_xthread : kernel_call_xthreads) {
    if (!kernel_call_xthread->is_guest_thread()) {
      continue;
    }
    // Only a running thread still has a live host thread to sample.
    xe::threading::Thread* kernel_call_host_thread =
        kernel_call_xthread->is_running() ? kernel_call_xthread->thread()
                                          : nullptr;
    const uint64_t kernel_call_host_pc =
        kernel_call_host_thread
            ? SampleKernelCallHostPc(kernel_call_host_thread)
            : 0;
    std::string kernel_call_object = "host code";
    auto* kernel_call_code_cache =
        kernel_state()->processor()->backend()->code_cache();
    cpu::GuestFunction* kernel_call_function =
        kernel_call_host_pc
            ? kernel_call_code_cache->LookupFunction(kernel_call_host_pc)
            : nullptr;
    uint32_t kernel_call_guest_pc = 0;
    if (kernel_call_function) {
      kernel_call_guest_pc = kernel_call_function->MapMachineCodeToGuestAddress(
          uintptr_t(kernel_call_host_pc));
      kernel_call_object =
          fmt::format("guest function {:08X} near {:08X}",
                      kernel_call_function->address(), kernel_call_guest_pc);
      // A thread running game code: sample it seven more times, 1 ms apart,
      // to show the range of the loop it is in.
      kernel_call_object += "; then";
      for (uint32_t kernel_call_sample = 0; kernel_call_sample < 7;
           ++kernel_call_sample) {
        xe::threading::Sleep(std::chrono::milliseconds(1));
        const uint64_t kernel_call_sample_pc =
            SampleKernelCallHostPc(kernel_call_host_thread);
        kernel_call_function =
            kernel_call_sample_pc
                ? kernel_call_code_cache->LookupFunction(kernel_call_sample_pc)
                : nullptr;
        kernel_call_object +=
            kernel_call_function
                ? fmt::format(
                      " {:08X}",
                      kernel_call_function->MapMachineCodeToGuestAddress(
                          uintptr_t(kernel_call_sample_pc)))
                : std::string(" host");
      }
    }
#if !XE_PLATFORM_WIN32
    else if (kernel_call_host_pc) {
      // Outside JIT code: name the host function, e.g. a lock or wait.
      Dl_info kernel_call_symbol;
      if (dladdr(reinterpret_cast<void*>(kernel_call_host_pc),
                 &kernel_call_symbol) &&
          kernel_call_symbol.dli_sname) {
        kernel_call_object = fmt::format(
            "host code {}+{:X}", kernel_call_symbol.dli_sname,
            kernel_call_host_pc - uint64_t(kernel_call_symbol.dli_saddr));
      }
    }
#endif
    // The thread's kernel object and processor control region live in
    // guest memory, which an exiting thread may already have released.
    std::string kernel_call_names = "kernel object unreadable";
    if (kernel_call_global.try_lock()) {
      if (KernelCallReadable(kernel_call_xthread->guest_object(),
                             sizeof(X_KTHREAD)) &&
          KernelCallReadable(kernel_call_xthread->pcr_ptr(), sizeof(X_KPCR))) {
        kernel_call_names =
            fmt::format("suspend count {}, priority {}, cpu {}, start {:08X}",
                        kernel_call_xthread->suspend_count(),
                        kernel_call_xthread->priority(),
                        uint32_t(kernel_call_xthread->active_cpu()),
                        kernel_call_xthread->start_address());
      }
      kernel_call_global.unlock();
    } else {
      kernel_call_names = "kernel object busy";
    }
    const cpu::ppc::PPCContext* kernel_call_context =
        kernel_call_xthread->thread_state()
            ? kernel_call_xthread->thread_state()->context()
            : nullptr;
    XELOGW(
        "Guest thread {:08X}: {}, {}; host pc {:016X} in {}; guest lr {:08X}; "
        "guest stack{}",
        kernel_call_xthread->handle(),
        kernel_call_xthread->is_running() ? "running" : "not running",
        kernel_call_names, kernel_call_host_pc, kernel_call_object,
        kernel_call_context ? uint32_t(kernel_call_context->lr) : 0,
        kernel_call_context
            ? LogKernelCallStack(uint32_t(kernel_call_context->r[1]))
            : std::string());
    // The game code around the first sample, to disassemble the loop.
    if (kernel_call_guest_pc && kernel_call_global.try_lock()) {
      Memory* kernel_call_memory = kernel_memory();
      const uint32_t kernel_call_code = (kernel_call_guest_pc - 0x40) & ~3u;
      if (KernelCallReadable(kernel_call_code, 0x80)) {
        for (uint32_t kernel_call_line = 0; kernel_call_line < 32;
             kernel_call_line += 8) {
          std::string kernel_call_words;
          for (uint32_t kernel_call_word = kernel_call_line;
               kernel_call_word < kernel_call_line + 8; ++kernel_call_word) {
            kernel_call_words += fmt::format(
                " {:08X}", xe::load_and_swap<uint32_t>(
                               kernel_call_memory->TranslateVirtual(
                                   kernel_call_code + kernel_call_word * 4)));
          }
          XELOGW("Guest thread {:08X} code {:08X}:{}",
                 kernel_call_xthread->handle(),
                 kernel_call_code + kernel_call_line * 4, kernel_call_words);
        }
      }
      kernel_call_global.unlock();
    }
  }
}

void LogGuestThreadHotspots() {
  if (!cvars::log_guest_thread_hotspots) {
    return;
  }
#if XE_PLATFORM_APPLE && XE_ARCH_ARM64
  auto& hotspot_global = xe::global_critical_region::mutex();
  if (!hotspot_global.try_lock()) {
    return;
  }
  auto hotspot_xthreads =
      kernel_state()->object_table()->GetObjectsByType<XThread>();
  hotspot_global.unlock();
  struct HotspotThread {
    object_ref<XThread> xthread;
    xe::threading::Thread* host_thread;
    // Where each sample was: a guest function's address, or a host symbol.
    std::map<std::string, uint32_t> places;
    uint32_t guest_samples = 0;
    uint32_t samples = 0;
  };
  std::vector<HotspotThread> hotspot_threads;
  for (auto& hotspot_xthread : hotspot_xthreads) {
    if (hotspot_xthread->is_guest_thread() && hotspot_xthread->is_running() &&
        hotspot_xthread->thread()) {
      hotspot_threads.push_back(
          {hotspot_xthread, hotspot_xthread->thread(), {}, 0, 0});
    }
  }
  if (hotspot_threads.empty()) {
    return;
  }
  auto* hotspot_code_cache =
      kernel_state()->processor()->backend()->code_cache();
  // Each round stops every thread just long enough to read its program
  // counter; the lookups run after it has resumed.
  constexpr uint32_t kHotspotRounds = 16;
  for (uint32_t hotspot_round = 0; hotspot_round < kHotspotRounds;
       ++hotspot_round) {
    if (hotspot_round) {
      xe::threading::Sleep(std::chrono::milliseconds(1));
    }
    for (HotspotThread& hotspot_thread : hotspot_threads) {
      const uint64_t hotspot_pc =
          SampleKernelCallHostPc(hotspot_thread.host_thread);
      if (!hotspot_pc) {
        continue;
      }
      ++hotspot_thread.samples;
      std::string hotspot_place = "host";
      if (cpu::GuestFunction* hotspot_function =
              hotspot_code_cache->LookupFunction(hotspot_pc)) {
        hotspot_place = fmt::format("{:08X}", hotspot_function->address());
        ++hotspot_thread.guest_samples;
      } else {
        Dl_info hotspot_symbol;
        if (dladdr(reinterpret_cast<void*>(hotspot_pc), &hotspot_symbol) &&
            hotspot_symbol.dli_sname) {
          hotspot_place = hotspot_symbol.dli_sname;
        }
      }
      ++hotspot_thread.places[hotspot_place];
    }
  }
  for (HotspotThread& hotspot_thread : hotspot_threads) {
    // A thread whose samples were all in a host wait is idle.
    bool hotspot_busy = hotspot_thread.guest_samples != 0;
    for (const auto& [hotspot_place, hotspot_count] : hotspot_thread.places) {
      if (hotspot_place.find("wait") == std::string::npos &&
          hotspot_place.find("mach_msg") == std::string::npos &&
          hotspot_place.find("nanosleep") == std::string::npos &&
          hotspot_place.find("switch_pri") == std::string::npos &&
          hotspot_place.find("thread_switch") == std::string::npos) {
        hotspot_busy = true;
      }
    }
    if (!hotspot_busy) {
      continue;
    }
    std::vector<std::pair<uint32_t, std::string>> hotspot_sorted;
    for (const auto& [hotspot_place, hotspot_count] : hotspot_thread.places) {
      hotspot_sorted.emplace_back(hotspot_count, hotspot_place);
    }
    std::sort(hotspot_sorted.rbegin(), hotspot_sorted.rend());
    std::string hotspot_text;
    for (size_t i = 0; i < std::min<size_t>(hotspot_sorted.size(), 6); ++i) {
      hotspot_text += fmt::format("{} {} x{}", i ? "," : "",
                                  hotspot_sorted[i].second,
                                  hotspot_sorted[i].first);
    }
    XELOGI(
        "Guest thread hotspots {:08X} ({}): {}/{} samples in guest code;{}",
        hotspot_thread.xthread->handle(), hotspot_thread.host_thread->name(),
        hotspot_thread.guest_samples, hotspot_thread.samples, hotspot_text);
  }
#endif  // XE_PLATFORM_APPLE && XE_ARCH_ARM64
}

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
        LogKernelCallStackCode(uint32_t(ppc_context->lr),
                               uint32_t(ppc_context->r[1]));
        LogKernelCallThreads();
      }
    }
  }

  for (uint32_t kernel_call_period = 1; kernel_call_period <= 8;
       ++kernel_call_period) {
    const uint32_t kernel_call_run = kernel_call_runs[kernel_call_period];
    // A cycle of length p also matches at 2p, 3p, ...: report only the
    // shortest one, once at the threshold and then every 100 thresholds.
    if (kernel_call_run < kernel_call_threshold) {
      continue;
    }
    // The shortest repeating cycle wins; its multiples also match but are
    // not reported again.
    if (kernel_call_run != kernel_call_threshold &&
        kernel_call_run % (kernel_call_threshold * 100)) {
      break;
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
