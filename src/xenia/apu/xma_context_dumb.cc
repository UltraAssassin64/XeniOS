#include "xma_context.h"
#include "xenia/base/logging.h"
#include <cstring>
#include <algorithm>

namespace xe {
namespace apu {
namespace xma {

XmaContext::XmaContext()
    : read_pos_(0), write_pos_(0), buffer_capacity_(48000 * 2) {  // 1 second stereo buffer
    ring_buffer_.resize(buffer_capacity_, 0.0f);
}

XmaContext::~XmaContext() = default;

// Swap endian helper
int16_t XmaContext::SwapEndian16(int16_t val) {
    return (val << 8) | ((val >> 8) & 0xFF);
}

// Decode a single XMA packet
size_t XmaContext::DecodePacket(const uint8_t* packet, size_t packet_size, float* out_buffer, size_t max_frames) {
    if (!packet || !out_buffer) return 0;

    size_t samples_decoded = 0;

    // TODO: Replace this with actual XMA decoding logic from Xenia Edge
    // For demonstration, we simulate decoding 16-bit PCM samples
    size_t total_samples = std::min(packet_size / 2, max_frames * 2);
    for (size_t i = 0; i < total_samples; i++) {
        int16_t sample;
        memcpy(&sample, packet + i * 2, 2);
        sample = SwapEndian16(sample);
        out_buffer[i] = sample / 32768.0f;  // normalize to [-1,1]
        samples_decoded++;
    }

    // Submit decoded frames into ring buffer
    WriteToRingBuffer(out_buffer, samples_decoded / 2);

    return samples_decoded / 2;  // frames
}

// Ring buffer submission
void XmaContext::WriteToRingBuffer(const float* data, size_t frames) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    for (size_t i = 0; i < frames * 2; i++) {
        ring_buffer_[write_pos_] = data[i];
        write_pos_ = (write_pos_ + 1) % buffer_capacity_;
        if (write_pos_ == read_pos_) {
            // Overwrite oldest if full
            read_pos_ = (read_pos_ + 1) % buffer_capacity_;
        }
    }
}

// Pull frames for backend audio callback
void XmaContext::SubmitFrames(const float* data, size_t frames) {
    WriteToRingBuffer(data, frames);
}

}  // namespace xma
}  // namespace apu
}  // namespace xe