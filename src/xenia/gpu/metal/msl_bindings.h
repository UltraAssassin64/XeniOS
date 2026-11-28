/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_GPU_METAL_MSL_BINDINGS_H_
#define XENIA_GPU_METAL_MSL_BINDINGS_H_

#include <cstdint>

namespace xe {
namespace gpu {
namespace metal {

// Metal buffer/texture/sampler binding indices for the SPIRV-Cross MSL path.
// These must match the remappings applied in msl_shader.cc AddResourceBindings.
//
// The SPIR-V translator uses 4 descriptor sets:
//   Set 0: Shared memory and EDRAM (storage buffers)
//   Set 1: Constant buffers (system, float, bool/loop, fetch, clip, tess)
//   Set 2: Vertex stage textures
//   Set 3: Pixel stage textures
//
// SPIRV-Cross with argument_buffers=false maps each binding to a direct
// Metal buffer/texture/sampler index. We control these indices via
// add_msl_resource_binding().

namespace MslBufferIndex {
// Buffer indices for vertex and fragment shaders.
constexpr uint32_t kSharedMemory = 0;
constexpr uint32_t kSystemConstants = 1;
constexpr uint32_t kFloatConstantsVertex = 2;
constexpr uint32_t kFloatConstantsPixel = 3;
constexpr uint32_t kBoolLoopConstants = 4;
constexpr uint32_t kFetchConstants = 5;
constexpr uint32_t kClipPlaneConstants = 6;
constexpr uint32_t kTessellationConstants = 7;
// Buffer index for the shared argument buffer (textures + samplers) when
// SPIRV-Cross argument buffers are enabled.
constexpr uint32_t kArgumentBufferTexturesSamplers = 8;

// Total constant buffer slots used, for computing uniform buffer offsets.
constexpr uint32_t kConstantBufferCount = 8;

// Size of each constant buffer slice in the uniforms buffer (4KB, matching
// the D3D12/MSC layout for easy migration).
constexpr uint32_t kCbvSizeBytes = 4096;

// Total size of the uniforms buffer region for one stage (one draw).
// system(4KB) + float(4KB) + bool_loop(4KB) + fetch(4KB) + clip(4KB) +
// tessellation(4KB) = 24KB.
constexpr uint32_t kUniformsBytesPerStage = 6 * kCbvSizeBytes;
}  // namespace MslBufferIndex

namespace MslTextureIndex {
// Textures start at index 0 within each shader stage.
constexpr uint32_t kBase = 0;
// Maximum textures per stage for the SPIRV-Cross path.
// The SPIR-V translator may create multiple texture bindings per fetch
// constant (for signed/unsigned and 2D/3D variants), so this can exceed 32.
constexpr uint32_t kMaxPerStage = 128;
}  // namespace MslTextureIndex

namespace MslSamplerIndex {
// Samplers start at index 0 within each shader stage.
constexpr uint32_t kBase = 0;
// Maximum samplers per stage.
constexpr uint32_t kMaxPerStage = 16;
}  // namespace MslSamplerIndex

// Resource IDs used within the argument buffer `[[id(N)]]` namespace when
// SPIRV-Cross argument buffers are enabled.
namespace MslArgumentBufferId {
// Reserve [0, kSamplerBase) for textures, and place samplers after.
constexpr uint32_t kSamplerBase = MslTextureIndex::kMaxPerStage;
}  // namespace MslArgumentBufferId

// Shaders compiled with spirv2air (metal_shader_compiler "spirv2air") read
// their resources through a top-level argument buffer of 64-bit pointers: the
// seven constant buffers by SPIR-V binding, shared memory for reading and for
// writing, and the vertex and fragment stages' texture index records. Textures
// and samplers come from heaps of 24-byte records, a texture's handle 8 bytes
// in and a sampler's at the start. Textures and then samplers go in one heap,
// so a resource's heap index is its SPIR-V binding and every stage's index
// records are {binding, binding}. The buffer indices are ones the Metal
// Shading Language path doesn't use.
namespace Spirv2airLayout {
constexpr uint32_t kResourceHeapBuffer = 9;
constexpr uint32_t kSamplerHeapBuffer = 10;
constexpr uint32_t kArgumentBuffer = 11;
constexpr uint32_t kConstantBufferCount = 7;
constexpr uint32_t kSharedMemoryReadOffset = kConstantBufferCount * 8;
constexpr uint32_t kSharedMemoryWriteOffset = kSharedMemoryReadOffset + 8;
constexpr uint32_t kVertexTextureIndicesOffset = kSharedMemoryWriteOffset + 8;
constexpr uint32_t kFragmentTextureIndicesOffset =
    kVertexTextureIndicesOffset + 8;
constexpr uint32_t kArgumentBufferBytes = kFragmentTextureIndicesOffset + 8;
constexpr uint32_t kHeapRecordBytes = 24;
constexpr uint32_t kHeapTextureHandleOffset = 8;
// Index records in the {binding, binding} table, enough for any stage's
// textures and samplers.
constexpr uint32_t kMaxBindings = 256;
// metal_spirv2air_options bits. kDirectSharedMemory: shared memory is the
// stage's MslBufferIndex::kSharedMemory buffer instead of pointers in the
// argument buffer. kDirectTextures: texture k is [[texture(k)]] and sampler j
// [[sampler(j)]], instead of the heaps and index records.
// kConstantTextureIndices: the index records are read through a constant
// pointer instead of a device one. kPreciseMath: the precise math intrinsics
// instead of the fast ones. Bits 4-5 (from
// metal_spirv2air_declaration_attributes): spirv2air's declarationAttributes,
// which of the attributes Apple's current compiler writes on intrinsic
// declarations are written.
constexpr uint32_t kDirectSharedMemory = 1;
constexpr uint32_t kDirectTextures = 2;
constexpr uint32_t kConstantTextureIndices = 4;
constexpr uint32_t kPreciseMath = 8;
constexpr uint32_t kOptionMask = 15;
constexpr uint32_t kDeclarationAttributesShift = 4;
}  // namespace Spirv2airLayout

}  // namespace metal
}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_METAL_MSL_BINDINGS_H_
