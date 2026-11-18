/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2026 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_GPU_METAL_METAL_PERF_COUNTERS_H_
#define XENIA_GPU_METAL_METAL_PERF_COUNTERS_H_

#include <atomic>
#include <chrono>
#include <cstdint>

#include "third_party/metal-cpp/Metal/Metal.hpp"

namespace xe {
namespace gpu {
namespace metal {

// Performance diagnostics: counters filled in across the Metal backend and
// logged, then reset, with the draw stats ("Metal perf over N frames").
// Relaxed atomics, as they are written from several threads.
struct MetalPerfCounters {
  // GPU execution time of completed submission command buffers.
  std::atomic<uint64_t> gpu_us{0};
  std::atomic<uint64_t> gpu_command_buffers{0};
  // Time the CPU spent blocked in waitUntilCompleted.
  std::atomic<uint64_t> cpu_wait_us{0};
  std::atomic<uint64_t> cpu_waits{0};
  // Shader translation and Metal library compilation (MslShader).
  std::atomic<uint64_t> shader_compiles{0};
  std::atomic<uint64_t> shader_compile_us{0};
  // Render pipeline state creation.
  std::atomic<uint64_t> pipeline_compiles{0};
  std::atomic<uint64_t> pipeline_compile_us{0};
  // Draws waiting for a compile (metal_async_compile_wait_ms).
  std::atomic<uint64_t> compile_waits{0};
  std::atomic<uint64_t> compile_wait_us{0};
  // Draws waiting for a free SPIRV-Cross uniform buffer, and command buffers
  // ended because none was free.
  std::atomic<uint64_t> uniform_waits{0};
  std::atomic<uint64_t> uniform_wait_us{0};
  std::atomic<uint64_t> uniform_rollovers{0};
  // metal_render_pass_area_from_scissor: guest draw and transfer render
  // passes, how many got a limited render area and its total height, and
  // draw passes restarted to grow their area.
  std::atomic<uint64_t> render_area_draw_passes{0};
  std::atomic<uint64_t> render_area_draw_limited{0};
  std::atomic<uint64_t> render_area_draw_rows{0};
  std::atomic<uint64_t> render_area_restarts{0};
  std::atomic<uint64_t> render_area_transfer_passes{0};
  std::atomic<uint64_t> render_area_transfer_limited{0};
  std::atomic<uint64_t> render_area_transfer_rows{0};
  // Guest occlusion queries (EVENT_WRITE_ZPD): every event, and the query
  // ends.
  std::atomic<uint64_t> occlusion_query_events{0};
  std::atomic<uint64_t> occlusion_queries{0};
  // occlusion_query_enable: query ends given a counted result, of which with
  // no samples and replacing the guess written at the end, query ends left
  // with the guess, and draw passes restarted to count in.
  std::atomic<uint64_t> zpd_queries_measured{0};
  std::atomic<uint64_t> zpd_queries_hidden{0};
  std::atomic<uint64_t> zpd_queries_corrected{0};
  std::atomic<uint64_t> zpd_queries_guessed{0};
  std::atomic<uint64_t> zpd_pass_restarts{0};
  // Guest memory copied into the GPU's shared memory buffer.
  std::atomic<uint64_t> upload_bytes{0};
};

inline MetalPerfCounters metal_perf_counters;

inline uint64_t MetalPerfMicrosecondsSince(
    std::chrono::steady_clock::time_point metal_perf_begin) {
  return uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::steady_clock::now() - metal_perf_begin)
                      .count());
}

// waitUntilCompleted, counting how long the CPU was blocked on the GPU.
inline void WaitUntilCompletedCounted(
    MTL::CommandBuffer* metal_perf_command_buffer) {
  const auto metal_perf_begin = std::chrono::steady_clock::now();
  metal_perf_command_buffer->waitUntilCompleted();
  metal_perf_counters.cpu_wait_us.fetch_add(
      MetalPerfMicrosecondsSince(metal_perf_begin), std::memory_order_relaxed);
  metal_perf_counters.cpu_waits.fetch_add(1, std::memory_order_relaxed);
}

}  // namespace metal
}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_METAL_METAL_PERF_COUNTERS_H_
