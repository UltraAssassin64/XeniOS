/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2026 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_GPU_METAL_METAL_ZPD_VISIBILITY_POOL_H_
#define XENIA_GPU_METAL_METAL_ZPD_VISIBILITY_POOL_H_

#include <cstddef>
#include <cstdint>
#include <vector>

namespace MTL {
class Buffer;
class CommandBuffer;
class Device;
}  // namespace MTL

namespace xe {
namespace gpu {
namespace metal {

// Small virtual query pool over one Metal visibility result buffer. Each
// acquired slot maps to one 8-byte visibility-buffer offset and one physical
// ZPD query segment, never to a guest ZPD address.
// Ported from upstream XeniOS without its accumulate result type path (iOS 26
// and newer): each render pass resets the visibility results, so the pool
// copies the slots of every pass that used them to a CPU-read buffer.
class MetalZPDVisibilityPool {
 public:
  MetalZPDVisibilityPool() = default;
  MetalZPDVisibilityPool(const MetalZPDVisibilityPool&) = delete;
  MetalZPDVisibilityPool& operator=(const MetalZPDVisibilityPool&) = delete;
  ~MetalZPDVisibilityPool() { Shutdown(); }

  bool EnsureInitialized(MTL::Device* device, uint32_t requested_capacity);
  void Shutdown();

  bool is_initialized() const {
    return visibility_buffer_ != nullptr && visibility_mapping_ != nullptr &&
           capacity_ != 0;
  }

  MTL::Buffer* visibility_buffer() const { return visibility_buffer_; }

  // Call after ending every render pass that acquired slots, to preserve its
  // results before the next pass resets them.
  void EndRenderPass(MTL::CommandBuffer* command_buffer);

  bool Acquire(uint32_t& index, uint32_t& generation, size_t& offset);
  void Release(uint32_t index, uint32_t generation);
  bool IsGenerationCurrent(uint32_t index, uint32_t generation) const;

  // Read the 64-bit sample count from the shared buffer at the given index.
  // The command buffer containing this slot's work must have completed before
  // calling this.
  // False if preserving this pass's results failed; the caller uses its normal
  // conservative query fallback rather than treating missing results as zero.
  bool Read(uint32_t index, uint64_t& samples) const;

 private:
  MTL::Buffer* visibility_buffer_ = nullptr;
  MTL::Buffer* readback_buffer_ = nullptr;
  uint64_t* visibility_mapping_ = nullptr;
  uint32_t capacity_ = 0;

  std::vector<uint32_t> free_indices_;
  std::vector<uint32_t> render_pass_indices_;
  std::vector<bool> failed_results_;

  // Bumped on release so stale readbacks from a recycled slot get dropped.
  std::vector<uint32_t> generations_;
};

}  // namespace metal
}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_METAL_METAL_ZPD_VISIBILITY_POOL_H_
