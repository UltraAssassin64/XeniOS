/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/xevent.h"

#include "xenia/base/byte_stream.h"
#include "xenia/base/clock.h"
#include "xenia/base/logging.h"
#include "xenia/cpu/thread_state.h"
#include "xenia/kernel/xthread.h"

namespace xe {
namespace kernel {

XEvent::XEvent(KernelState* kernel_state)
    : XObject(kernel_state, kObjectType) {}

XEvent::~XEvent() = default;

void XEvent::Initialize(bool manual_reset, bool initial_state) {
  assert_false(event_);

  this->CreateNative<X_KEVENT>();

  if (manual_reset) {
    event_ = xe::threading::Event::CreateManualResetEvent(initial_state);
  } else {
    event_ = xe::threading::Event::CreateAutoResetEvent(initial_state);
  }
  assert_not_null(event_);
}

void XEvent::InitializeNative(void* native_ptr, X_DISPATCH_HEADER* header) {
  assert_false(event_);

  switch (header->type) {
    case 0x00:  // EventNotificationObject (manual reset)
      manual_reset_ = true;
      break;
    case 0x01:  // EventSynchronizationObject (auto reset)
      manual_reset_ = false;
      break;
    default:
      assert_always();
      return;
  }

  bool initial_state = header->signal_state ? true : false;
  if (manual_reset_) {
    event_ = xe::threading::Event::CreateManualResetEvent(initial_state);
  } else {
    event_ = xe::threading::Event::CreateAutoResetEvent(initial_state);
  }
  assert_not_null(event_);
  SetNativePointer(memory()->HostToGuestVirtual(native_ptr), true);
}

void XEvent::NoteSignal() {
  XThread* signal_thread =
      XThread::IsInThread() ? XThread::GetCurrentThread() : nullptr;
  cpu::ThreadState* thread_state = cpu::ThreadState::Get();
  last_signal_thread_.store(signal_thread ? signal_thread->handle() : 0,
                            std::memory_order_relaxed);
  last_signal_lr_.store(
      thread_state ? uint32_t(thread_state->context()->lr) : 0,
      std::memory_order_relaxed);
  last_signal_ms_.store(Clock::QueryHostUptimeMillis(),
                        std::memory_order_relaxed);
  signal_count_.fetch_add(1, std::memory_order_relaxed);
}

std::string XEvent::DescribeLastSignal() const {
  const uint64_t signal_count = signal_count_.load(std::memory_order_relaxed);
  if (!signal_count) {
    return "never set";
  }
  return fmt::format(
      "set {} times, last by thread {:08X} at guest lr {:08X} {} ms ago",
      signal_count, last_signal_thread_.load(std::memory_order_relaxed),
      last_signal_lr_.load(std::memory_order_relaxed),
      Clock::QueryHostUptimeMillis() -
          last_signal_ms_.load(std::memory_order_relaxed));
}

int32_t XEvent::Set(uint32_t priority_increment, bool wait) {
  NoteSignal();
  event_->Set();
  return 1;
}

int32_t XEvent::Pulse(uint32_t priority_increment, bool wait) {
  NoteSignal();
  event_->Pulse();
  return 1;
}

int32_t XEvent::Reset() {
  event_->Reset();
  return 1;
}
void XEvent::Query(uint32_t* out_type, uint32_t* out_state) {
  auto [type, state] = event_->Query();

  *out_type = type;
  *out_state = state;
}
void XEvent::Clear() { event_->Reset(); }

bool XEvent::Save(ByteStream* stream) {
  XELOGD("XEvent {:08X} ({})", handle(), manual_reset_ ? "manual" : "auto");
  SaveObject(stream);

  bool signaled = true;
  auto result =
      xe::threading::Wait(event_.get(), false, std::chrono::milliseconds(0));
  if (result == xe::threading::WaitResult::kSuccess) {
    signaled = true;
  } else if (result == xe::threading::WaitResult::kTimeout) {
    signaled = false;
  } else {
    assert_always();
  }

  if (signaled) {
    // Reset the event in-case it's an auto-reset.
    event_->Set();
  }

  stream->Write<bool>(signaled);
  stream->Write<bool>(manual_reset_);

  return true;
}

object_ref<XEvent> XEvent::Restore(KernelState* kernel_state,
                                   ByteStream* stream) {
  auto evt = new XEvent(nullptr);
  evt->kernel_state_ = kernel_state;

  evt->RestoreObject(stream);
  bool signaled = stream->Read<bool>();
  evt->manual_reset_ = stream->Read<bool>();

  if (evt->manual_reset_) {
    evt->event_ = xe::threading::Event::CreateManualResetEvent(false);
  } else {
    evt->event_ = xe::threading::Event::CreateAutoResetEvent(false);
  }
  assert_not_null(evt->event_);

  if (signaled) {
    evt->event_->Set();
  }

  return object_ref<XEvent>(evt);
}

}  // namespace kernel
}  // namespace xe
