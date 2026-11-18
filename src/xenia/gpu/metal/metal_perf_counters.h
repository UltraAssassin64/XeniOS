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
