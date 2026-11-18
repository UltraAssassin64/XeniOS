/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2026 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_GPU_METAL_METAL_GPU_PASS_TIMING_H_
#define XENIA_GPU_METAL_METAL_GPU_PASS_TIMING_H_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "third_party/metal-cpp/Metal/Metal.hpp"

namespace xe {
namespace gpu {
namespace metal {

// One timed pass: where its GPU timestamps go and what it was, so the time
// can be attributed when its command buffer completes.
struct MetalGpuPassRecord {
  MTL::CounterSampleBuffer* sample_buffer = nullptr;
  uint32_t sample_buffer_index = 0;
  uint32_t first_sample = 0;
  // 4 for render passes (vertex and fragment start and end), 2 otherwise.
  uint32_t sample_count = 0;
  // Static string naming what the pass is for.
  const char* label = nullptr;
  // Render passes only.
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t samples = 0;
  uint32_t color_formats[4] = {};
  uint32_t depth_format = 0;
  // 2 bits per attachment (colors 0-3, then depth): MTL::LoadAction and
  // MTL::StoreAction values.
  uint32_t load_actions = 0;
  uint32_t store_actions = 0;
  // Guest draws encoded in the pass (filled in by the command processor).
  uint32_t draws = 0;
  uint64_t vertices = 0;
};

// Per-pass GPU time from Metal's stage boundary timestamps (iOS 14+), logged
// with the Metal perf lines: which passes the GPU time goes to, and for
// render passes whether vertex or fragment work dominates. Encoders for the
// passes to time are created through RenderEncoder, ComputeEncoder and
// BlitEncoder, which create plain encoders when timing is off or unsupported.
class MetalGpuPassTimer {
 public:
  void Initialize(MTL::Device* device);
  // record_out receives the pass's record (nullptr if untimed), which stays
  // valid until the encoder has ended.
  MTL::RenderCommandEncoder* RenderEncoder(
      MTL::CommandBuffer* command_buffer, MTL::RenderPassDescriptor* descriptor,
      const char* label, MetalGpuPassRecord** record_out = nullptr);
  MTL::ComputeCommandEncoder* ComputeEncoder(MTL::CommandBuffer* command_buffer,
                                             const char* label);
  MTL::BlitCommandEncoder* BlitEncoder(MTL::CommandBuffer* command_buffer,
                                       const char* label);
  // Logs and resets the totals for the last window of guest frames.
  void Log(uint32_t frames);

 private:
  struct MetalGpuPassStats {
    uint64_t passes = 0;
    uint64_t draws = 0;
    uint64_t vertices = 0;
    uint64_t total_ticks = 0;
    uint64_t vertex_ticks = 0;
    uint64_t fragment_ticks = 0;
  };

  bool Enabled();
  MetalGpuPassRecord* AllocateSamples(MTL::CommandBuffer* command_buffer,
                                      uint32_t sample_count, const char* label);
  // Adds a completed pass's GPU time to the totals.
  void Complete(MetalGpuPassRecord* record);
  // Frees the pass's sample buffer slots and the record.
  void Release(MetalGpuPassRecord* record);

  std::mutex mutex_;
  MTL::Device* device_ = nullptr;
  std::atomic<bool> supported_{false};
  std::vector<MTL::CounterSampleBuffer*> sample_buffers_;
  // Passes whose command buffer hasn't completed yet, per sample buffer.
  std::vector<uint32_t> sample_buffer_outstanding_;
  uint32_t sample_buffer_size_ = 0;
  uint32_t current_sample_buffer_ = 0;
  uint32_t next_sample_ = 0;
  uint64_t untimed_passes_ = 0;
  uint64_t invalid_passes_ = 0;
  // GPU timestamp calibration against the steady clock.
  MTL::Timestamp calibration_gpu_ = 0;
  std::chrono::steady_clock::time_point calibration_time_;
  double ns_per_tick_ = 1.0;
  std::map<std::string, MetalGpuPassStats> stats_by_label_;
  std::map<std::string, MetalGpuPassStats> stats_by_pass_;
};

inline MetalGpuPassTimer metal_gpu_pass_timer;

}  // namespace metal
}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_METAL_METAL_GPU_PASS_TIMING_H_
