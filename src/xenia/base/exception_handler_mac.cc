/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2026 Ben Vanik. All rights reserved.
 * Released under the BSD license - see LICENSE in the root for more details.
 ******************************************************************************
 */

#define _XOPEN_SOURCE 600
// Keep the Darwin extensions _XOPEN_SOURCE hides (dladdr, Dl_info,
// pthread_get_stackaddr_np) for the unhandled fault backtrace.
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif

#include "xenia/base/exception_handler.h"

#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <ucontext.h>
#include <atomic>
#include <cstdint>
#include <cstring>

#include "xenia/base/assert.h"
#include "xenia/base/host_thread_context.h"
#include "xenia/base/logging.h"
#include "xenia/base/math.h"
#include "xenia/base/platform.h"

namespace xe {

bool signal_handlers_installed_ = false;
struct sigaction original_sigill_handler_;
struct sigaction original_sigbus_handler_;
struct sigaction original_sigsegv_handler_;

// This can be as large as needed, but isn't often needed.
// As we will be sometimes firing many exceptions we want to avoid having to
// scan the table too much or invoke many custom handlers.
constexpr size_t kMaxHandlerCount = 8;

// All custom handlers, left-aligned and null terminated.
// Executed in order.
std::pair<ExceptionHandler::Handler, void*> handlers_[kMaxHandlerCount];

static void ExceptionHandlerCallback(int signal_number, siginfo_t* signal_info,
                                     void* signal_context) {
#if XE_ARCH_ARM64
  // On ARM64, ucontext_t requires 16-byte alignment but the kernel may provide
  // an unaligned pointer in the signal handler. Copy to an aligned local to
  // avoid undefined behavior from misaligned access. mcontext_t is a pointer
  // on Darwin, so the copy's uc_mcontext still points at the kernel's saved
  // registers: writes through it reach the kernel, and the copy itself must
  // never be written back (see below).
  alignas(16) ucontext_t ucontext_storage;
  std::memcpy(&ucontext_storage, signal_context, sizeof(ucontext_t));
  ucontext_t* ucontext = &ucontext_storage;
#else
  ucontext_t* ucontext = reinterpret_cast<ucontext_t*>(signal_context);
#endif
  mcontext_t& mcontext = ucontext->uc_mcontext;

  HostThreadContext thread_context;

#if XE_ARCH_ARM64
  // Extract the general-purpose registers from mcontext
  thread_context.pc = mcontext->__ss.__pc;
  thread_context.sp = mcontext->__ss.__sp;
  thread_context.pstate = mcontext->__ss.__cpsr;

  // x0 - x28
  for (int i = 0; i < 29; ++i) {
    thread_context.x[i] = mcontext->__ss.__x[i];
  }
  // x29 is the frame pointer
  thread_context.x[29] = mcontext->__ss.__fp;
  // x30 is the link register
  thread_context.x[30] = mcontext->__ss.__lr;

  // Copy the vector registers
  for (int i = 0; i < 32; ++i) {
    thread_context.v[i].low =
        static_cast<uint64_t>(mcontext->__ns.__v[i] & 0xFFFFFFFFFFFFFFFF);
    thread_context.v[i].high = static_cast<uint64_t>(
        (mcontext->__ns.__v[i] >> 64) & 0xFFFFFFFFFFFFFFFF);
  }

  thread_context.fpsr = mcontext->__ns.__fpsr;
  thread_context.fpcr = mcontext->__ns.__fpcr;
#elif XE_ARCH_AMD64
  thread_context.rip = mcontext->__ss.__rip;
  thread_context.eflags = static_cast<uint32_t>(mcontext->__ss.__rflags);
  thread_context.rax = mcontext->__ss.__rax;
  thread_context.rcx = mcontext->__ss.__rcx;
  thread_context.rdx = mcontext->__ss.__rdx;
  thread_context.rbx = mcontext->__ss.__rbx;
  thread_context.rsp = mcontext->__ss.__rsp;
  thread_context.rbp = mcontext->__ss.__rbp;
  thread_context.rsi = mcontext->__ss.__rsi;
  thread_context.rdi = mcontext->__ss.__rdi;
  thread_context.r8 = mcontext->__ss.__r8;
  thread_context.r9 = mcontext->__ss.__r9;
  thread_context.r10 = mcontext->__ss.__r10;
  thread_context.r11 = mcontext->__ss.__r11;
  thread_context.r12 = mcontext->__ss.__r12;
  thread_context.r13 = mcontext->__ss.__r13;
  thread_context.r14 = mcontext->__ss.__r14;
  thread_context.r15 = mcontext->__ss.__r15;
#endif  // XE_ARCH

  Exception ex;
  switch (signal_number) {
    case SIGILL:
      ex.InitializeIllegalInstruction(&thread_context);
      break;
    case SIGBUS:
    case SIGSEGV: {
      Exception::AccessViolationOperation access_violation_operation =
          Exception::AccessViolationOperation::kUnknown;
#if XE_ARCH_ARM64
      // For a Data Abort (EC - ESR_EL1 bits 31:26 - 0b100100 from a lower
      // Exception Level, 0b100101 without a change in the Exception Level),
      // bit 6 is 0 for reading from a memory location, 1 for writing to a
      // memory location.
      const uint64_t esr = mcontext->__es.__esr;
      if (((esr >> 26) & 0b111110) == 0b100100) {
        access_violation_operation =
            (esr & (UINT64_C(1) << 6))
                ? Exception::AccessViolationOperation::kWrite
                : Exception::AccessViolationOperation::kRead;
      }
#elif XE_ARCH_AMD64
      // x86 page fault error code: bit 1 set indicates write.
      constexpr uint64_t kX86PageFaultErrorCodeWrite = UINT64_C(1) << 1;
      access_violation_operation =
          (uint64_t(mcontext->__es.__err) & kX86PageFaultErrorCodeWrite)
              ? Exception::AccessViolationOperation::kWrite
              : Exception::AccessViolationOperation::kRead;
#endif  // XE_ARCH

      ex.InitializeAccessViolation(
          &thread_context, reinterpret_cast<uint64_t>(signal_info->si_addr),
          access_violation_operation);
    } break;
    default:
      assert_unhandled_case(signal_number);
  }

  for (size_t i = 0; i < xe::countof(handlers_) && handlers_[i].first; ++i) {
    if (handlers_[i].first(&ex, handlers_[i].second)) {
      // Exception handled.
#if XE_ARCH_ARM64
      // Write back the modified registers to mcontext
      mcontext->__ss.__pc = thread_context.pc;
      mcontext->__ss.__sp = thread_context.sp;
      mcontext->__ss.__cpsr = (__uint32_t)thread_context.pstate;

      uint32_t modified_register_index;
      uint32_t modified_x_registers_remaining = ex.modified_x_registers();
      while (xe::bit_scan_forward(modified_x_registers_remaining,
                                  &modified_register_index)) {
        modified_x_registers_remaining &=
            ~(UINT32_C(1) << modified_register_index);
        if (modified_register_index < 29) {
          mcontext->__ss.__x[modified_register_index] =
              thread_context.x[modified_register_index];
        } else if (modified_register_index == 29) {
          mcontext->__ss.__fp = thread_context.x[29];
        } else if (modified_register_index == 30) {
          mcontext->__ss.__lr = thread_context.x[30];
        }
      }

      if (ex.modified_v_registers()) {
        uint32_t modified_v_register_index;
        uint32_t modified_v_registers_remaining = ex.modified_v_registers();
        while (xe::bit_scan_forward(modified_v_registers_remaining,
                                    &modified_v_register_index)) {
          modified_v_registers_remaining &=
              ~(UINT32_C(1) << modified_v_register_index);
          mcontext->__ns.__v[modified_v_register_index] =
              (static_cast<__uint128_t>(
                   thread_context.v[modified_v_register_index].high)
               << 64) |
              static_cast<__uint128_t>(
                  thread_context.v[modified_v_register_index].low);
        }
        mcontext->__ns.__fpsr = thread_context.fpsr;
        mcontext->__ns.__fpcr = thread_context.fpcr;
      }
      // Do not copy ucontext_storage back over signal_context. With
      // _XOPEN_SOURCE, ucontext_t includes a copy of the machine context
      // taken before the handlers ran; writing it back replaced the
      // registers changed above (the resume PC among them) with their old
      // values, so a fault handled by emulating the instruction and moving
      // the PC on, such as an MMIO store, re-executed the same instruction
      // forever. As in upstream, the writes above go to the kernel's saved
      // state through the mcontext pointer and that is all that is needed.
#elif XE_ARCH_AMD64
      mcontext->__ss.__rip = thread_context.rip;
      mcontext->__ss.__rflags = thread_context.eflags;
      uint32_t modified_register_index;
      uint16_t modified_int_registers_remaining = ex.modified_int_registers();
      uint64_t* kIntRegisterMap[] = {
          &mcontext->__ss.__rax, &mcontext->__ss.__rcx, &mcontext->__ss.__rdx,
          &mcontext->__ss.__rbx, &mcontext->__ss.__rsp, &mcontext->__ss.__rbp,
          &mcontext->__ss.__rsi, &mcontext->__ss.__rdi, &mcontext->__ss.__r8,
          &mcontext->__ss.__r9,  &mcontext->__ss.__r10, &mcontext->__ss.__r11,
          &mcontext->__ss.__r12, &mcontext->__ss.__r13, &mcontext->__ss.__r14,
          &mcontext->__ss.__r15,
      };
      while (xe::bit_scan_forward(modified_int_registers_remaining,
                                  &modified_register_index)) {
        modified_int_registers_remaining &=
            ~(UINT16_C(1) << modified_register_index);
        *kIntRegisterMap[modified_register_index] =
            thread_context.int_registers[modified_register_index];
      }
#endif  // XE_ARCH
      return;
    }
  }

  // Nobody handled it. The faulting instruction is retried when we return, so
  // a genuine host crash shows up as a hang; record where it happened. The
  // retry repeats millions of times a second, so log the first ten and then
  // one in a million (build 9565 wrote 24 million of these lines, 2.4 GB).
  static std::atomic<uint64_t> unhandled_host_exception_count{0};
  const uint64_t unhandled_count =
      unhandled_host_exception_count.fetch_add(1, std::memory_order_relaxed) +
      1;
  if (unhandled_count <= 10 || !(unhandled_count % 1000000)) {
#if XE_ARCH_ARM64
    // The link register says who called a null or bad function pointer (pc
    // 0): a host return address, or one in the JIT code cache.
    XELOGE(
        "Unhandled host exception: signal={} pc=0x{:016X} address=0x{:016X} "
        "lr=0x{:016X} sp=0x{:016X} ({} so far)",
        signal_number, uint64_t(ex.pc()),
        reinterpret_cast<uint64_t>(signal_info->si_addr), thread_context.x[30],
        thread_context.sp, unhandled_count);
    if (unhandled_count == 1) {
      // Name the host code involved: the faulting instruction, the link
      // register and the return address of each frame, walking the frame
      // pointer chain (which Apple's ARM64 ABI always keeps) only within this
      // thread's stack. Return addresses into system libraries carry a pointer
      // authentication code in the upper bits, which the mask removes.
      Dl_info unhandled_symbol;
      std::string unhandled_backtrace;
      auto unhandled_name = [&](uint64_t address) {
        address &= UINT64_C(0x0000007FFFFFFFFF);
        if (dladdr(reinterpret_cast<void*>(address), &unhandled_symbol) &&
            unhandled_symbol.dli_sname) {
          unhandled_backtrace +=
              fmt::format(" {}+{:X}", unhandled_symbol.dli_sname,
                          address - uint64_t(unhandled_symbol.dli_saddr));
        } else {
          unhandled_backtrace += fmt::format(" {:X}", address);
        }
      };
      unhandled_name(ex.pc());
      unhandled_name(thread_context.x[30]);
      const uint64_t unhandled_stack_top =
          uint64_t(pthread_get_stackaddr_np(pthread_self()));
      uint64_t unhandled_frame = thread_context.x[29];
      for (uint32_t unhandled_depth = 0;
           unhandled_depth < 16 && unhandled_frame >= thread_context.sp &&
           unhandled_frame + 16 <= unhandled_stack_top &&
           !(unhandled_frame & 7);
           ++unhandled_depth) {
        unhandled_name(reinterpret_cast<const uint64_t*>(unhandled_frame)[1]);
        if (reinterpret_cast<const uint64_t*>(unhandled_frame)[0] <=
            unhandled_frame) {
          break;
        }
        unhandled_frame = reinterpret_cast<const uint64_t*>(unhandled_frame)[0];
      }
      XELOGE("Unhandled host exception: {}; host code (pc, lr, frames):{}",
             ex.code() != Exception::Code::kAccessViolation
                 ? "not an access violation"
             : ex.access_violation_operation() ==
                     Exception::AccessViolationOperation::kWrite
                 ? "write"
             : ex.access_violation_operation() ==
                     Exception::AccessViolationOperation::kRead
                 ? "read"
                 : "unknown access",
             unhandled_backtrace);
    }
#else
    XELOGE(
        "Unhandled host exception: signal={} pc=0x{:016X} address=0x{:016X} "
        "({} so far)",
        signal_number, uint64_t(ex.pc()),
        reinterpret_cast<uint64_t>(signal_info->si_addr), unhandled_count);
#endif  // XE_ARCH_ARM64
  }
}

void ExceptionHandler::Install(Handler fn, void* data) {
  if (!signal_handlers_installed_) {
    struct sigaction signal_handler;

    std::memset(&signal_handler, 0, sizeof(signal_handler));
    signal_handler.sa_sigaction = ExceptionHandlerCallback;
    signal_handler.sa_flags = SA_SIGINFO;

    if (sigaction(SIGILL, &signal_handler, &original_sigill_handler_) != 0) {
      assert_always("Failed to install new SIGILL handler");
    }
    if (sigaction(SIGBUS, &signal_handler, &original_sigbus_handler_) != 0) {
      assert_always("Failed to install new SIGBUS handler");
    }
    if (sigaction(SIGSEGV, &signal_handler, &original_sigsegv_handler_) != 0) {
      assert_always("Failed to install new SIGSEGV handler");
    }
    signal_handlers_installed_ = true;
  }

  for (size_t i = 0; i < xe::countof(handlers_); ++i) {
    if (!handlers_[i].first) {
      handlers_[i].first = fn;
      handlers_[i].second = data;
      return;
    }
  }
  assert_always("Too many exception handlers installed");
}

void ExceptionHandler::Uninstall(Handler fn, void* data) {
  for (size_t i = 0; i < xe::countof(handlers_); ++i) {
    if (handlers_[i].first == fn && handlers_[i].second == data) {
      for (; i < xe::countof(handlers_) - 1; ++i) {
        handlers_[i] = handlers_[i + 1];
      }
      handlers_[i].first = nullptr;
      handlers_[i].second = nullptr;
      break;
    }
  }

  bool has_any = false;
  for (size_t i = 0; i < xe::countof(handlers_); ++i) {
    if (handlers_[i].first) {
      has_any = true;
      break;
    }
  }
  if (!has_any) {
    if (signal_handlers_installed_) {
      if (sigaction(SIGILL, &original_sigill_handler_, NULL) != 0) {
        assert_always("Failed to restore original SIGILL handler");
      }
      if (sigaction(SIGBUS, &original_sigbus_handler_, NULL) != 0) {
        assert_always("Failed to restore original SIGBUS handler");
      }
      if (sigaction(SIGSEGV, &original_sigsegv_handler_, NULL) != 0) {
        assert_always("Failed to restore original SIGSEGV handler");
      }
      signal_handlers_installed_ = false;
    }
  }
}

}  // namespace xe
