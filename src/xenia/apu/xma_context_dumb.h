#pragma once
#include <cstdint>
#include <vector>
#include <mutex>

namespace xe {
namespace apu {
namespace xma {

class XmaContextDumb {
public:
    XmaContextDumb();
    ~XmaContextDumb();

    // Decode a full XMA packet
    // Returns number of PCM frames produced
    size_t DecodePacket(const uint8_t* packet, size_t packet_size, float* out_buffer, size_t max_frames);

    // Ring buffer integration for backend
    void SubmitFrames(const float* data, size_t frames);

private:
    std::vector<float> ring_buffer_;
    std::mutex buffer_mutex_;
    size_t read_pos_;
    size_t write_pos_;
    size_t buffer_capacity_;

    // Helper functions
    static int16_t SwapEndian16(int16_t val);
    void WriteToRingBuffer(const float* data, size_t frames);
};

}  // namespace xma
}  // namespace apu
}  // namespace xe