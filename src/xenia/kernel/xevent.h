/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_KERNEL_XEVENT_H_
#define XENIA_KERNEL_XEVENT_H_

#include <atomic>
#include <string>

#include "xenia/base/threading.h"
#include "xenia/kernel/xobject.h"
#include "xenia/xbox.h"

namespace xe {
namespace kernel {

// https://www.nirsoft.net/kernel_struct/vista/KEVENT.html
struct X_KEVENT {
  X_DISPATCH_HEADER header;
};
static_assert_size(X_KEVENT, 0x10);

class XEvent : public XObject {
 public:
  static const XObject::Type kObjectType = XObject::Type::Event;

  explicit XEvent(KernelState* kernel_state);
  ~XEvent() override;

  void Initialize(bool manual_reset, bool initial_state);
  void InitializeNative(void* native_ptr, X_DISPATCH_HEADER* header);

  int32_t Set(uint32_t priority_increment, bool wait);
  int32_t Pulse(uint32_t priority_increment, bool wait);
  int32_t Reset();
  void Query(uint32_t* out_type, uint32_t* out_state);
  void Clear();

  // For the stuck thread report: the guest thread and return address that
  // last set or pulsed this event, how long ago, and how many times in all.
  std::string DescribeLastSignal() const;

  bool Save(ByteStream* stream) override;
  static object_ref<XEvent> Restore(KernelState* kernel_state,
                                    ByteStream* stream);

 protected:
  xe::threading::WaitHandle* GetWaitHandle() override { return event_.get(); }

 private:
  void NoteSignal();

  bool manual_reset_ = false;
  std::unique_ptr<xe::threading::Event> event_;
  std::atomic<uint32_t> last_signal_thread_{0};
  std::atomic<uint32_t> last_signal_lr_{0};
  std::atomic<uint64_t> last_signal_ms_{0};
  std::atomic<uint64_t> signal_count_{0};
};

}  // namespace kernel
}  // namespace xe

#endif  // XENIA_KERNEL_XEVENT_H_
