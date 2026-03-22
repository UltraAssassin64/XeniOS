#pragma once

#include <atomic>
#include <cstring>
#include <vector>

namespace xe {
namespace apu {
namespace coreaudio {

class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity)
      : buffer_(capacity), capacity_(capacity) {}

  bool Push(const float* data, size_t count) {
    size_t write = write_pos_.load(std::memory_order_relaxed);
    size_t read = read_pos_.load(std::memory_order_acquire);

    if (capacity_ - (write - read) < count) {
      return false;
    }

    for (size_t i = 0; i < count; i++) {
      buffer_[(write + i) % capacity_] = data[i];
    }

    write_pos_.store(write + count, std::memory_order_release);
    return true;
  }

  size_t Pop(float* out, size_t count) {
    size_t read = read_pos_.load(std::memory_order_relaxed);
    size_t write = write_pos_.load(std::memory_order_acquire);

    size_t available = write - read;
    size_t to_read = available < count ? available : count;

    for (size_t i = 0; i < to_read; i++) {
      out[i] = buffer_[(read + i) % capacity_];
    }

    read_pos_.store(read + to_read, std::memory_order_release);
    return to_read;
  }

 private:
  std::vector<float> buffer_;
  size_t capacity_;

  std::atomic<size_t> read_pos_{0};
  std::atomic<size_t> write_pos_{0};
};

}  // namespace coreaudio
}  // namespace apu
}  // namespace xe
