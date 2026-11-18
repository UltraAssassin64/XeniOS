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

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "third_party/metal-cpp/Metal/Metal.hpp"
#include "xenia/gpu/metal/msl_shader.h"

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
  // 2 bits per attachment (colors 0-3, depth, then stencil): MTL::LoadAction
  // and MTL::StoreAction values.
  uint32_t load_actions = 0;
  uint32_t store_actions = 0;
  // Estimated main memory traffic of the attachments: a tile-based GPU reads
  // a whole attachment for LoadActionLoad and writes it for a store (and the
  // resolve texture for a multisample resolve). Upper bounds, as Apple GPUs
  // may compress attachments.
  uint64_t load_bytes = 0;
  uint64_t store_bytes = 0;
  // Guest draws encoded in the pass (through MetalGpuPassTimer::NoteDraw).
  uint32_t draws = 0;
  uint64_t vertices = 0;
  // Draws per pipeline (shader pair) in the pass, consecutive draws merged.
  struct MetalGpuPassPipeline {
    const void* vertex_translation = nullptr;
    const void* pixel_translation = nullptr;
    // Vertex shader hash and modification, pixel shader hash and
    // modification.
    std::array<uint64_t, 4> shaders = {};
    uint32_t draws = 0;
    uint64_t vertices = 0;
  };
  std::vector<MetalGpuPassPipeline> pipelines;
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
  // Counts a guest draw encoded in the pass of record (may be nullptr).
  void NoteDraw(MetalGpuPassRecord* record,
                MslShader::MslTranslation* vertex_translation,
                MslShader::MslTranslation* pixel_translation,
                uint64_t vertices);
  // Logs and resets the totals for the last window of guest frames, which
  // took window_us of host time.
  void Log(uint32_t frames, uint64_t window_us);

 private:
  struct MetalGpuPassStats {
    uint64_t passes = 0;
    uint64_t draws = 0;
    uint64_t vertices = 0;
    uint64_t total_ticks = 0;
    uint64_t vertex_ticks = 0;
    uint64_t fragment_ticks = 0;
    uint64_t load_bytes = 0;
    uint64_t store_bytes = 0;
    // Pipelines only: pixel shader features (MetalGpuPixelShaderFeatures
    // bits) and texture samples in its source.
    uint32_t features = 0;
    uint32_t texture_samples = 0;
  };

  bool Enabled();
  MetalGpuPassRecord* AllocateSamples(MTL::CommandBuffer* command_buffer,
                                      uint32_t sample_count, const char* label);
  // Adds a completed pass's GPU time to the totals.
  void Complete(MetalGpuPassRecord* record);
  // Frees the pass's sample buffer slots and the record.
  void Release(MetalGpuPassRecord* record);
  // Pixel shader features that are slow on Apple GPUs, from the MSL source:
  // bit 0 discard_fragment (no hidden surface removal for the draw), bit 1
  // depth output, bit 2 raster order groups; and its texture sample calls.
  std::pair<uint32_t, uint32_t> MetalGpuPixelShaderFeatures(
      MslShader::MslTranslation* pixel_translation);

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
  // Per pipeline (shader pair): pass times shared out by draws (fragment)
  // and vertices (vertex), so an estimate unless the pipeline had the pass
  // to itself.
  std::map<std::array<uint64_t, 4>, MetalGpuPassStats> stats_by_pipeline_;
  std::map<const void*, std::pair<uint32_t, uint32_t>> pixel_shader_features_;
};

inline MetalGpuPassTimer metal_gpu_pass_timer;

// Asynchronous shader and pipeline compiles that draws were skipped for:
// how long each was pending (queue wait plus compile) and how many draws
// were skipped meanwhile, to choose between skipping and a bounded wait.
class MetalCompileStallTracker {
 public:
  // {1 (vertex shader), hash, modification, 0, 0}, {2 (pixel shader), hash,
  // modification, 0, 0} or {3 (pipeline), vertex shader hash, modification,
  // pixel shader hash, modification}.
  using MetalCompileKey = std::array<uint64_t, 5>;
  void Queued(const MetalCompileKey& key);
  void Skipped(const MetalCompileKey& key);
  // compile_us: time the compile itself took on its worker.
  void Finished(const MetalCompileKey& key, uint64_t compile_us);
  void Log(uint32_t frames);
  // Forgets everything, for a new title (whose queues start empty).
  void Clear();

 private:
  struct MetalCompileStall {
    MetalCompileKey key = {};
    std::chrono::steady_clock::time_point queued_at;
    uint64_t skipped = 0;
    uint64_t pending_us = 0;
    uint64_t compile_us = 0;
  };
  static std::string Describe(const MetalCompileKey& key);

  std::mutex mutex_;
  std::map<MetalCompileKey, MetalCompileStall> pending_;
  // Finished in the window, by pending time: up to 16, 33, 50, 100, 250,
  // 500, 1000 ms, and longer.
  uint64_t bucket_compiles_[8] = {};
  uint64_t bucket_skipped_[8] = {};
  uint64_t queue_us_ = 0;
  uint64_t compile_us_ = 0;
  std::vector<MetalCompileStall> finished_;
};

inline MetalCompileStallTracker metal_compile_stall_tracker;

}  // namespace metal
}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_METAL_METAL_GPU_PASS_TIMING_H_
