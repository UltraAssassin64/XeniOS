/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/apu/xma_decoder.h"

#include <atomic>
#include <chrono>
#include <string>

#include "xenia/apu/xma_context.h"
#include "xenia/apu/xma_context_fake.h"
#include "xenia/apu/xma_context_master.h"
#include "xenia/apu/xma_context_new.h"
#include "xenia/apu/xma_context_old.h"

#include "xenia/base/cvar.h"
#include "xenia/base/logging.h"
#include "xenia/base/math.h"
#include "xenia/base/profiling.h"
#include "xenia/base/ring_buffer.h"
#include "xenia/base/string_buffer.h"
#include "xenia/cpu/processor.h"
#include "xenia/cpu/thread_state.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/xthread.h"

#if XE_PLATFORM_APPLE || XE_PLATFORM_LINUX
#include <dlfcn.h>
#include <execinfo.h>
#endif
extern "C" {
#include "third_party/FFmpeg/libavutil/cpu.h"
#include "third_party/FFmpeg/libavutil/log.h"
}  // extern "C"

// As with normal Microsoft, there are like twelve different ways to access
// the audio APIs. Early games use XMA*() methods almost exclusively to touch
// decoders. Later games use XAudio*() and direct memory writes to the XMA
// structures (as opposed to the XMA* calls), meaning that we have to support
// both.
//
// The XMA*() functions just manipulate the audio system in the guest context
// and let the normal XmaDecoder handling take it, to prevent duplicate
// implementations. They can be found in xboxkrnl_audio_xma.cc
//
// XMA details:
// https://devel.nuclex.org/external/svn/directx/trunk/include/xma2defs.h
// https://github.com/gdawg/fsbext/blob/master/src/xma_header.h
//
// XAudio2 uses XMA under the covers, and seems to map with the same
// restrictions of frame/subframe/etc:
// https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.xaudio2.xaudio2_buffer(v=vs.85).aspx
//
// XMA contexts are 64b in size and tight bitfields. They are in physical
// memory not usually available to games. Games will use MmMapIoSpace to get
// the 64b pointer in user memory so they can party on it. If the game doesn't
// do this, it's likely they are either passing the context to XAudio or
// using the XMA* functions.

DEFINE_bool(ffmpeg_verbose, false, "Verbose FFmpeg output (debug and above)",
            "APU");

// FFmpeg's NEON MDCT and float vector routines are built for ARM64 and are
// faster than its C ones; disabling them gives output bit-identical to x86 at
// that cost, paid by the guest audio thread that kicked the context (the
// decode runs there). XeniOS HEAD keeps NEON on.
constexpr bool kDisableFfmpegNeonDefault = false;
DEFINE_bool(ffmpeg_disable_neon_on_arm64, kDisableFfmpegNeonDefault,
            "Disable FFmpeg NEON runtime paths on ARM64 for decode parity "
            "with x86 (slower XMA decoding).",
            "APU");
// Configs saved before build 9681 hold the old ARM64 default (true). The update
// variable is named after the hour only (cvar.h), so it must differ from
// xma_decoder's below (12).
UPDATE_from_bool(ffmpeg_disable_neon_on_arm64, 2026, 11, 27, 13, true);

#if XE_ARCH_ARM64
constexpr bool kUseDedicatedXmaThreadDefault = false;
#else
constexpr bool kUseDedicatedXmaThreadDefault = true;
#endif

DEFINE_bool(use_dedicated_xma_thread, kUseDedicatedXmaThreadDefault,
            "Enables XMA decoding on separate thread. Disabled should produce "
            "better results, but decrease performance a bit.",
            "APU");

DEFINE_string(
    xma_decoder, "new",
    "Decoder version used to process XMA audio.\n"
    "Use: [fake, master, old, new]\n"
    " fake: \n  No audio will be decoded.\n"
    " master: \n  Version of decoder exactly like on base version of Xenia.\n"
    " old: \n  Decoder based on master version of decoder with few "
    "improvements.\n"
    " new: \n  New version of decoder. Provides highest stability, but isn't "
    "yet finished.\n",
    "APU");

UPDATE_from_string(xma_decoder, 2026, 2, 16, 12, "old");

namespace xe {
namespace apu {

XmaDecoder::XmaDecoder(cpu::Processor* processor)
    : memory_(processor->memory()), processor_(processor) {}

XmaDecoder::~XmaDecoder() = default;

void av_log_callback(void* avcl, int level, const char* fmt, va_list va) {
  if (!cvars::ffmpeg_verbose && level > AV_LOG_WARNING) {
    return;
  }

  char level_char = '?';
  LogLevel log_level;
  switch (level) {
    case AV_LOG_ERROR: {
      level_char = '!';
      log_level = xe::LogLevel::Error;
      break;
    }
    case AV_LOG_WARNING: {
      level_char = 'w';
      log_level = xe::LogLevel::Warning;
      break;
    }
    case AV_LOG_INFO: {
      level_char = 'i';
      log_level = xe::LogLevel::Info;
      break;
    }
    case AV_LOG_VERBOSE: {
      level_char = 'v';
      log_level = xe::LogLevel::Debug;
      break;
    }
    case AV_LOG_DEBUG: {
      level_char = 'd';
      log_level = xe::LogLevel::Debug;
      break;
    }
    default: {
      level_char = '?';
      log_level = xe::LogLevel::Debug;
      break;
    }
  }

  StringBuffer buff;
  buff.AppendVarargs(fmt, va);
  xe::logging::AppendLogLineFormat(LogSrc::Apu, log_level, level_char,
                                   "ffmpeg: {}", buff.to_string_view());
}

X_STATUS XmaDecoder::Setup(kernel::KernelState* kernel_state) {
  // Setup ffmpeg logging callback
  av_log_set_callback(av_log_callback);

#if XE_ARCH_ARM64
  if (cvars::ffmpeg_disable_neon_on_arm64) {
    const int cpu_flags = av_get_cpu_flags();
    av_force_cpu_flags(cpu_flags & ~AV_CPU_FLAG_NEON);
  }
#endif

  // Let the processor know we want register access callbacks.
  memory_->AddVirtualMappedRange(
      0x7FEA0000, 0xFFFF0000, 0x0000FFFF, this,
      reinterpret_cast<cpu::MMIOReadCallback>(MMIOReadRegisterThunk),
      reinterpret_cast<cpu::MMIOWriteCallback>(MMIOWriteRegisterThunk));

  // Setup XMA context data.
  // The Xbox 360 kernel allocates the contexts with X_PAGE_NOCACHE |
  // X_PAGE_READWRITE and writes MmGetPhysicalAddress for the address to the
  // register.
  context_data_first_ptr_ = memory()->SystemHeapAlloc(
      sizeof(XMA_CONTEXT_DATA) * kContextCount, 256, kSystemHeapPhysical);
  context_data_last_ptr_ =
      context_data_first_ptr_ + (sizeof(XMA_CONTEXT_DATA) * kContextCount - 1);
  register_file_[XmaRegister::ContextArrayAddress] =
      memory()->GetPhysicalAddress(context_data_first_ptr_);

  // Setup XMA contexts.
  for (int i = 0; i < kContextCount; ++i) {
    if (cvars::xma_decoder == "fake") {
      contexts_[i] = new XmaContextFake();
    } else if (cvars::xma_decoder == "master") {
      contexts_[i] = new XmaContextMaster();
    } else if (cvars::xma_decoder == "old") {
      contexts_[i] = new XmaContextOld();
    } else if (cvars::xma_decoder == "new") {
      contexts_[i] = new XmaContextNew();
    } else if (cvars::xma_decoder == "fake") {
      contexts_[i] = new XmaContextFake();
    } else {
      contexts_[i] = new XmaContextNew();
    }

    uint32_t guest_ptr = context_data_first_ptr_ + i * sizeof(XMA_CONTEXT_DATA);
    if (contexts_[i]->Setup(i, memory(), guest_ptr)) {
      assert_always();
    }
  }
  register_file_[XmaRegister::NextContextIndex] = 1;
  context_bitmap_.Resize(kContextCount);

  worker_running_ = true;
  work_event_ = xe::threading::Event::CreateAutoResetEvent(false);
  assert_not_null(work_event_);
  worker_thread_ =
      kernel::object_ref<kernel::XHostThread>(new kernel::XHostThread(
          kernel_state, 128 * 1024, 0,
          [this]() {
            if (cvars::use_dedicated_xma_thread) {
              WorkerThreadMain();
            }
            return 0;
          },
          kernel_state
              ->GetIdleProcess()));  // this one doesnt need any process
                                     // actually. never calls any guest code
  worker_thread_->set_name("XMA Decoder");
  worker_thread_->set_can_debugger_suspend(true);
  worker_thread_->Create();

  return X_STATUS_SUCCESS;
}

void XmaDecoder::WorkerThreadMain() {
  while (worker_running_) {
    // Okay, let's loop through XMA contexts to find ones we need to decode!
    bool did_work = false;
    for (uint32_t n = 0; n < kContextCount; n++) {
      bool worked = contexts_[n]->Work();
      if (worked) {
        contexts_[n]->SignalWorkDone();
      }
      did_work = did_work || worked;
    }

    if (paused_.load(std::memory_order_acquire)) {
      pause_fence_.Signal();
      resume_fence_.Wait();
    }

    if (did_work) {
      continue;
    }
    xe::threading::Wait(work_event_.get(), false);
  }
}

void XmaDecoder::Shutdown() {
  worker_running_ = false;

  if (work_event_) {
    work_event_->Set();
  }

  if (paused_.load(std::memory_order_acquire)) {
    Resume();
  }

  if (worker_thread_) {
    // Wait for work thread.
    xe::threading::Wait(worker_thread_->thread(), false);
    worker_thread_.reset();
  }

  if (context_data_first_ptr_) {
    memory()->SystemHeapFree(context_data_first_ptr_);
  }

  context_data_first_ptr_ = 0;
  context_data_last_ptr_ = 0;
}

int XmaDecoder::GetContextId(uint32_t guest_ptr) {
  static_assert_size(XMA_CONTEXT_DATA, 64);
  if (guest_ptr < context_data_first_ptr_ ||
      guest_ptr > context_data_last_ptr_) {
    return -1;
  }
  assert_zero(guest_ptr & 0x3F);
  return (guest_ptr - context_data_first_ptr_) >> 6;
}

uint32_t XmaDecoder::AllocateContext() {
  size_t index = context_bitmap_.Acquire();
  if (index == -1) {
    // Out of contexts.
    return 0;
  }

  XmaContext& context = *contexts_[index];
  assert_false(context.is_allocated());
  context.set_is_allocated(true);
  std::memset(memory()->TranslateVirtual(context.guest_ptr()), 0,
              sizeof(XMA_CONTEXT_DATA));
  return context.guest_ptr();
}

void XmaDecoder::ReleaseContext(uint32_t guest_ptr) {
  auto context_id = GetContextId(guest_ptr);
  assert_true(context_id >= 0);

  XmaContext& context = *contexts_[context_id];
  assert_true(context.is_allocated());
  context.Release();
  context_bitmap_.Release(context_id);
}

bool XmaDecoder::BlockOnContext(uint32_t guest_ptr, bool poll) {
  auto context_id = GetContextId(guest_ptr);
  assert_true(context_id >= 0);

  XmaContext& context = *contexts_[context_id];
  return context.Block(poll);
}

uint32_t XmaDecoder::ReadRegister(uint32_t addr) {
  auto r = (addr & 0xFFFF) / 4;

  assert_true(r < XmaRegisterFile::kRegisterCount);

  switch (r) {
    case XmaRegister::ContextArrayAddress:
      break;
    case XmaRegister::CurrentContextIndex: {
      // 0606h (1818h) is rotating context processing # set to hardware ID of
      // context being processed.
      // If bit 200h is set, the locking code will possibly collide on hardware
      // IDs and error out, so we should never set it (I think?).
      uint32_t& current_context_index =
          register_file_[XmaRegister::CurrentContextIndex];
      uint32_t& next_context_index =
          register_file_[XmaRegister::NextContextIndex];
      // To prevent games from seeing a stuck XMA context, return a rotating
      // number.
      current_context_index = next_context_index;
      next_context_index = (next_context_index + 1) % kContextCount;
      break;
    }
    default:
      const auto register_info = register_file_.GetRegisterInfo(r);
      if (register_info) {
        XELOGW("XMA: Read from unhandled register ({:04X}, {})", r,
               register_info->name);
      } else {
        XELOGW("XMA: Read from unknown register ({:04X})", r);
      }
      break;
  }

  return xe::byte_swap(register_file_[r]);
}

void XmaDecoder::WriteRegister(uint32_t addr, uint32_t value) {
  SCOPE_profile_cpu_f("apu");

  uint32_t r = (addr & 0xFFFF) / 4;
  value = xe::byte_swap(value);

  // Hang diagnostics: after the first cutscene of Batman: Arkham Asylum the
  // main thread was seen writing the same context-clear value millions of
  // times without the guest moving on. Whatever path the write takes (fault
  // handler or inline MMIO call), report the native call stack when one
  // register/value pair is written many times in a row.
  static std::atomic<uint64_t> xma_repeat_key{0};
  static std::atomic<uint32_t> xma_repeat_count{0};
  const uint64_t xma_key = (uint64_t(r) << 32) | value;
  if (xma_repeat_key.exchange(xma_key) != xma_key) {
    xma_repeat_count = 0;
  } else if (++xma_repeat_count == 1000 || !(xma_repeat_count % 500000)) {
    cpu::ThreadState* xma_thread_state = cpu::ThreadState::Get();
    XELOGW(
        "XMA register {:04X} written {:08X} {} times in a row; guest lr {:08X}",
        r, value, xma_repeat_count.load(),
        xma_thread_state ? uint32_t(xma_thread_state->context()->lr) : 0);
#if XE_PLATFORM_APPLE || XE_PLATFORM_LINUX
    if (xma_repeat_count == 1000) {
      void* xma_frames[24];
      int xma_frame_count = backtrace(xma_frames, 24);
      for (int xma_frame = 0; xma_frame < xma_frame_count; ++xma_frame) {
        Dl_info xma_symbol = {};
        if (dladdr(xma_frames[xma_frame], &xma_symbol) &&
            xma_symbol.dli_fbase) {
          XELOGW("XMA repeat frame {}: {} + {:X} ({})", xma_frame,
                 xma_symbol.dli_sname ? xma_symbol.dli_sname : "?",
                 uintptr_t(xma_frames[xma_frame]) -
                     uintptr_t(xma_symbol.dli_saddr ? xma_symbol.dli_saddr
                                                    : xma_symbol.dli_fbase),
                 xma_symbol.dli_fname ? xma_symbol.dli_fname : "?");
        } else {
          XELOGW("XMA repeat frame {}: {:016X} (generated code)", xma_frame,
                 uintptr_t(xma_frames[xma_frame]));
        }
      }
    }
#endif  // XE_PLATFORM_APPLE || XE_PLATFORM_LINUX
  }

  assert_true(r < XmaRegisterFile::kRegisterCount);
  register_file_[r] = value;

  if (r >= XmaRegister::Context0Kick && r <= XmaRegister::Context9Kick) {
    // Context kick command.
    // This will kick off the given hardware contexts.
    // Basically, this kicks the SPU and says "hey, decode that audio!"
    // XMAEnableContext

    // The context ID is a bit in the range of the entire context array.
    const uint32_t base_context_id = (r - XmaRegister::Context0Kick) * 32;
    const uint32_t kicked_value = value;
    while (value) {
      const uint32_t context_id = base_context_id + std::countr_zero(value);
      auto& context = *contexts_[context_id];
      context.Enable();
      if (!cvars::use_dedicated_xma_thread) {
        context.Work();
      }
      value &= value - 1;
    }
    // Signal the decoder thread to start processing.
    work_event_->SetBoostPriority();
    if (cvars::use_dedicated_xma_thread) {
      // Block until the worker finishes, so the game sees updated context data.
      uint32_t remaining = kicked_value;
      while (remaining) {
        const uint32_t context_id =
            base_context_id + std::countr_zero(remaining);
        contexts_[context_id]->WaitForWorkDone();
        remaining &= remaining - 1;
      }
    }
    // Census every 10 s: a voice whose context holds input but never moves
    // never ends, so a title waiting for it to finish waits forever. A kick
    // decodes at once and clears the enabled flag, so a context is counted by
    // its state, not by that flag.
    static std::atomic<uint64_t> xma_census_next_ms{0};
    const uint64_t xma_census_now_ms = uint64_t(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
    uint64_t xma_census_due_ms = xma_census_next_ms.load();
    if (xma_census_now_ms >= xma_census_due_ms &&
        xma_census_next_ms.compare_exchange_strong(xma_census_due_ms,
                                                   xma_census_now_ms + 10000)) {
      // Per context: the state at the previous census, read offset and output
      // offsets with the valid bits.
      static uint64_t xma_census_keys[kContextCount];
      uint32_t xma_census_allocated = 0, xma_census_holding = 0,
               xma_census_stalled = 0;
      std::string xma_census_list;
      for (uint32_t n = 0; n < kContextCount; ++n) {
        XmaContext& xma_census_context = *contexts_[n];
        if (!xma_census_context.is_allocated()) {
          xma_census_keys[n] = 0;
          continue;
        }
        ++xma_census_allocated;
        XMA_CONTEXT_DATA xma_census_data(
            memory()->TranslateVirtual(xma_census_context.guest_ptr()));
        const uint64_t xma_census_key =
            (uint64_t(xma_census_data.input_buffer_read_offset) << 32) |
            (uint64_t(xma_census_data.output_buffer_write_offset) << 16) |
            (uint64_t(xma_census_data.output_buffer_read_offset) << 8) |
            (uint64_t(xma_census_data.input_buffer_0_valid) << 2) |
            (uint64_t(xma_census_data.input_buffer_1_valid) << 1) |
            uint64_t(xma_census_data.output_buffer_valid) | (1ull << 63);
        if (xma_census_data.input_buffer_0_valid ||
            xma_census_data.input_buffer_1_valid) {
          // Here: holding input to decode.
          ++xma_census_holding;
          if (xma_census_key == xma_census_keys[n] &&
              ++xma_census_stalled <= 3) {
            xma_census_list += fmt::format(
                "; context {}: in0 {}/{} in1 {}/{} current {} read {:X} out "
                "{} w{} r{} blocks {} loop {} {:X}-{:X} error {}/{}",
                n, uint32_t(xma_census_data.input_buffer_0_valid),
                uint32_t(xma_census_data.input_buffer_0_packet_count),
                uint32_t(xma_census_data.input_buffer_1_valid),
                uint32_t(xma_census_data.input_buffer_1_packet_count),
                uint32_t(xma_census_data.current_buffer),
                uint32_t(xma_census_data.input_buffer_read_offset),
                uint32_t(xma_census_data.output_buffer_valid),
                uint32_t(xma_census_data.output_buffer_write_offset),
                uint32_t(xma_census_data.output_buffer_read_offset),
                uint32_t(xma_census_data.output_buffer_block_count),
                uint32_t(xma_census_data.loop_count),
                uint32_t(xma_census_data.loop_start),
                uint32_t(xma_census_data.loop_end),
                uint32_t(xma_census_data.error_set),
                uint32_t(xma_census_data.error_status));
          }
        }
        xma_census_keys[n] = xma_census_key;
      }
      XELOGI(
          "XMA census at {} s: {} contexts allocated, {} holding input, {} of "
          "them unchanged for 10 s{}",
          xma_census_now_ms / 1000, xma_census_allocated, xma_census_holding,
          xma_census_stalled, xma_census_list);
    }
  } else if (r >= XmaRegister::Context0Lock && r <= XmaRegister::Context9Lock) {
    // Context lock command.
    // This requests a lock by flagging the context.
    // XMADisableContext
    const uint32_t base_context_id = (r - XmaRegister::Context0Lock) * 32;
    while (value) {
      const uint32_t context_id = base_context_id + std::countr_zero(value);
      auto& context = *contexts_[context_id];
      context.Disable();
      // Ensure the worker isn't mid-processing this context.
      context.Block(false);
      value &= value - 1;
    }
  } else if (r >= XmaRegister::Context0Clear &&
             r <= XmaRegister::Context9Clear) {
    // Context clear command.
    // This will reset the given hardware contexts.
    const uint32_t base_context_id = (r - XmaRegister::Context0Clear) * 32;
    while (value) {
      const uint32_t context_id = base_context_id + std::countr_zero(value);
      auto& context = *contexts_[context_id];
      context.Clear();
      value &= value - 1;
    }
  } else {
    // 0601h (1804h) is written to with 0x02000000 and 0x03000000 around a lock
    // operation
    switch (r) {
      case 0x601:
        break;
      default: {
        const auto register_info = register_file_.GetRegisterInfo(r);
        if (register_info) {
          XELOGW("XMA: Write to unhandled register ({:04X}, {}): {:08X}", r,
                 register_info->name, value);
        } else {
          XELOGW("XMA: Write to unknown register ({:04X}): {:08X}", r, value);
        }
        break;
      }
#pragma warning(suppress : 4065)
    }
  }
}

void XmaDecoder::Pause() {
  if (paused_.exchange(true, std::memory_order_acq_rel)) {
    return;
  }

  pause_fence_.Wait();
}

void XmaDecoder::Resume() {
  if (!paused_.exchange(false, std::memory_order_acq_rel)) {
    return;
  }

  resume_fence_.Signal();
}

}  // namespace apu
}  // namespace xe
