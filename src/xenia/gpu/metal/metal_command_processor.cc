/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/gpu/metal/metal_command_processor.h"
#include "xenia/gpu/gpu_flags.h"
#include "xenia/gpu/metal/msl_bindings.h"

#include <dispatch/dispatch.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <limits>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

#include "third_party/metal-cpp/Foundation/NSProcessInfo.hpp"
#include "third_party/metal-cpp/Foundation/NSURL.hpp"
#include "third_party/metal-cpp/Metal/MTLEvent.hpp"

#include "third_party/fmt/include/fmt/format.h"
#include "xenia/base/assert.h"
#include "xenia/base/cvar.h"
#include "xenia/base/filesystem.h"
#include "xenia/base/logging.h"
#include "xenia/base/math.h"
#include "xenia/base/profiling.h"
#include "xenia/base/xxhash.h"
#include "xenia/emulator.h"
#include "xenia/gpu/draw_util.h"
#include "xenia/gpu/gpu_flags.h"
#include "xenia/gpu/graphics_system.h"
#include "xenia/gpu/metal/metal_gpu_pass_timing.h"
#include "xenia/gpu/metal/metal_graphics_system.h"
#include "xenia/gpu/metal/metal_perf_counters.h"
#include "xenia/gpu/metal/metal_shader_cache.h"
#include "xenia/gpu/packet_disassembler.h"
#include "xenia/gpu/registers.h"
#include "xenia/gpu/texture_util.h"
#include "xenia/gpu/xenos.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/user_module.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/ui/metal/metal_presenter.h"

#if METAL_SHADER_CONVERTER_AVAILABLE
#include "xenia/gpu/metal/metal_shader_converter.h"
using BYTE = uint8_t;
#include "xenia/gpu/metal/d3d12_5_1_bytecode/adaptive_quad_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/adaptive_triangle_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/continuous_quad_1cp_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/continuous_quad_4cp_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/continuous_triangle_1cp_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/continuous_triangle_3cp_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/discrete_quad_1cp_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/discrete_quad_4cp_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/discrete_triangle_1cp_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/discrete_triangle_3cp_hs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/tessellation_adaptive_vs.h"
#include "xenia/gpu/metal/d3d12_5_1_bytecode/tessellation_indexed_vs.h"
// Metal IR Converter Runtime - defines IRDescriptorTableEntry and bind points
#define IR_RUNTIME_METALCPP
#include "third_party/metal-shader-converter/include/metal_irconverter_runtime.h"
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

#if XE_PLATFORM_APPLE
#include <mach/mach.h>
#include <unordered_map>
#endif  // XE_PLATFORM_APPLE
#if XE_PLATFORM_IOS
#include <os/proc.h>
#endif  // XE_PLATFORM_IOS

#ifndef DISPATCH_DATA_DESTRUCTOR_NONE
#define DISPATCH_DATA_DESTRUCTOR_NONE DISPATCH_DATA_DESTRUCTOR_DEFAULT
#endif

DECLARE_bool(clear_memory_page_state);
DECLARE_bool(submit_on_primary_buffer_end);
DECLARE_bool(skip_unchanged_render_target_transfers);
DECLARE_bool(metal_spirv2air_self_test);
DECLARE_string(metal_shader_compiler);
DEFINE_bool(
    metal_gpu_ordered_fences, true,
    "With shared memory aliasing guest RAM: publish the game's shader-done "
    "fences only once the GPU has finished the work before them, so the game "
    "does not reuse vertex, texture or command memory the GPU has yet to "
    "read (exploding geometry, garbage textures when the GPU lags behind).",
    "Metal");
DEFINE_bool(
    metal_inline_shader_constants, true,
    "SPIRV-Cross path: pass each draw's shader constants with "
    "setVertexBytes/setFragmentBytes when they change, instead of through "
    "pooled uniform buffers holding 8 draws each (which ran out every few "
    "hundred draws, ending the command buffer and waiting for the GPU).",
    "Metal");
DEFINE_int32(
    metal_uniform_buffers_max, 0,
    "Most uniform buffers (each holding the constants of 8 draws) the "
    "SPIRV-Cross path may have in flight before draws wait for the GPU to "
    "retire one. 0 for the default (96 on iOS, 12 elsewhere).",
    "Metal");
DEFINE_int32(
    metal_frame_detail_log, 0,
    "Changed to N while a game runs: logs every render pass, draw (shaders, "
    "state and texture fetch constants), resolve, render target transfer and "
    "occlusion query of the next N frames. Change it again for another "
    "capture.",
    "Metal");
DEFINE_int32(
    metal_gpu_trace_frames, 0,
    "Changed to N while a game runs: records the GPU commands and registers "
    "of the next N frames into a trace file under trace_gpu_prefix (relative "
    "to the storage root), for offline analysis. Change it again for another "
    "capture.",
    "Metal");
DEFINE_bool(
    metal_render_pass_area_from_scissor, true,
    "Limit each guest draw render pass to the area its draws' scissors cover, "
    "and each render target transfer pass to the rectangles it draws, instead "
    "of the whole render target (which spans all of EDRAM, 2048 rows at 1280 "
    "wide). A tile-based GPU loads and stores every tile of the area, so "
    "this cuts memory traffic; a draw pass is restarted if a later draw "
    "needs more.",
    "Metal");
// On by default since build 9606: gameplay draw passes loaded and stored
// about 2 GB of 1280x2048 render targets per frame without it.
UPDATE_from_bool(metal_render_pass_area_from_scissor, 2026, 11, 18, 20, false);
DEFINE_int32(
    metal_async_compile_wait_ms, 16,
    "With async shader compilation, how long draws may wait per frame for a "
    "shader or pipeline still compiling before they are skipped (the "
    "blackouts while compiling). 0 never waits; larger values trade skipped "
    "draws for stalls.",
    "Metal");
// 16 since build 9606: most compiles finish within 16 ms (Metal's own cache),
// yet with 0 the draws using them were skipped, leaving half of the frames
// at a cutscene's end empty.
UPDATE_from_int32(metal_async_compile_wait_ms, 2026, 11, 18, 21, 0);
DEFINE_bool(
    metal_resolve_keep_command_buffer, false,
    "SPIRV-Cross path: keep the command buffer open after resolves, loading "
    "textures read from resolved memory into it between render passes, "
    "instead of committing it at each. Fewer command buffers, but a pass is "
    "restarted at each such draw (GTA IV: 24-88 passes a frame, GPU slower), "
    "and Assassin's Creed Revelations' menus flicker with it. Per-game use "
    "only (Spider-Man: Edge of Time ran better with it).",
    "Metal");
DEFINE_int32(
    metal_pipeline_creation_threads, -1,
    "Number of threads used for SPIRV-Cross shader and render pipeline "
    "compilation in the Metal backend. -1 to calculate automatically (75% of "
    "logical CPU cores), a positive number to specify the number of threads "
    "explicitly (up to the number of logical CPU cores), 0 to disable "
    "multithreaded compilation.",
    "Metal");

namespace xe {
namespace gpu {
namespace metal {

namespace {
bool UseSpirvCrossPath() {
#if METAL_SHADER_CONVERTER_AVAILABLE
  return cvars::metal_use_spirvcross;
#else
  return true;
#endif  // METAL_SHADER_CONVERTER_AVAILABLE
}

// metal_uniform_buffers_max, or its default.
size_t MetalUniformBuffersMax() {
  if (cvars::metal_uniform_buffers_max > 0) {
    return size_t(cvars::metal_uniform_buffers_max);
  }
#if XE_PLATFORM_IOS
  // 24 left draws waiting for the GPU hundreds of times per second in
  // gameplay (~3600 draws per frame, 8 per buffer); 96 x 384 KB = 36 MB.
  return 96;
#else
  return 12;
#endif
}

void GetBoundRenderTargetSize(const MetalRenderTargetCache* render_target_cache,
                              uint32_t fallback_width, uint32_t fallback_height,
                              uint32_t& width_out, uint32_t& height_out) {
  width_out = std::max(fallback_width, uint32_t(1));
  height_out = std::max(fallback_height, uint32_t(1));
  if (!render_target_cache) {
    return;
  }
  MTL::Texture* pass_size_texture = render_target_cache->GetColorTarget(0);
  if (!pass_size_texture) {
    pass_size_texture = render_target_cache->GetDepthTarget();
  }
  if (!pass_size_texture) {
    pass_size_texture = render_target_cache->GetDummyColorTarget();
  }
  if (!pass_size_texture) {
    return;
  }
  width_out =
      std::max(static_cast<uint32_t>(pass_size_texture->width()), uint32_t(1));
  height_out =
      std::max(static_cast<uint32_t>(pass_size_texture->height()), uint32_t(1));
}

void ClampScissorToBounds(draw_util::Scissor& scissor, uint32_t width,
                          uint32_t height) {
  width = std::max(width, uint32_t(1));
  height = std::max(height, uint32_t(1));

  scissor.offset[0] = std::min(scissor.offset[0], width);
  scissor.offset[1] = std::min(scissor.offset[1], height);

  uint32_t max_scissor_width = width - scissor.offset[0];
  uint32_t max_scissor_height = height - scissor.offset[1];
  scissor.extent[0] = std::min(scissor.extent[0], max_scissor_width);
  scissor.extent[1] = std::min(scissor.extent[1], max_scissor_height);
}

void LogMetalErrorDetails(const char* label, NS::Error* error) {
  if (!error) {
    return;
  }
  const char* desc = error->localizedDescription()
                         ? error->localizedDescription()->utf8String()
                         : nullptr;
  const char* failure = error->localizedFailureReason()
                            ? error->localizedFailureReason()->utf8String()
                            : nullptr;
  const char* recovery =
      error->localizedRecoverySuggestion()
          ? error->localizedRecoverySuggestion()->utf8String()
          : nullptr;
  const char* domain =
      error->domain() ? error->domain()->utf8String() : nullptr;
  int64_t code = error->code();
  XELOGE("{}: domain={} code={} desc='{}' failure='{}' recovery='{}'", label,
         domain ? domain : "<null>", code, desc ? desc : "<null>",
         failure ? failure : "<null>", recovery ? recovery : "<null>");
  NS::Dictionary* user_info = error->userInfo();
  if (user_info) {
    auto* info_desc = user_info->description();
    XELOGE("{}: userInfo={}", label,
           info_desc ? info_desc->utf8String() : "<null>");
  }
}

constexpr int64_t kMslAsyncLogIntervalNs =
    int64_t(std::chrono::nanoseconds(std::chrono::seconds(1)).count());
#if METAL_SHADER_CONVERTER_AVAILABLE
#if XE_PLATFORM_IOS
constexpr uint64_t kDefaultDrawRingPoolMaxBytes = 256ull * 1024ull * 1024ull;
#else
constexpr uint64_t kDefaultDrawRingPoolMaxBytes = 1024ull * 1024ull * 1024ull;
#endif
constexpr uint32_t kDrawRingPoolPrewarmCount = 2;
constexpr uint32_t kDrawRingPoolWaitTimeoutMs = 8;
constexpr int64_t kDrawRingPoolLogIntervalNs =
    int64_t(std::chrono::nanoseconds(std::chrono::seconds(2)).count());
#endif  // METAL_SHADER_CONVERTER_AVAILABLE
constexpr size_t kResolvedMemoryRangesMax = 8192;

int64_t GetSteadyTimeNs() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

bool ShouldLogRateLimited(std::atomic<int64_t>& last_log_ns,
                          int64_t interval_ns) {
  const int64_t now = GetSteadyTimeNs();
  int64_t previous = last_log_ns.load(std::memory_order_relaxed);
  while (now - previous >= interval_ns) {
    if (last_log_ns.compare_exchange_weak(previous, now,
                                          std::memory_order_relaxed,
                                          std::memory_order_relaxed)) {
      return true;
    }
  }
  return false;
}
void PopulatePipelineFormatsFromRenderPassDescriptor(
    MTL::RenderPassDescriptor* pass_descriptor, MTL::PixelFormat* color_formats,
    uint32_t color_count, MTL::PixelFormat* depth_format,
    MTL::PixelFormat* stencil_format, uint32_t* sample_count) {
  if (!pass_descriptor) {
    return;
  }

  auto update_sample_count = [&](MTL::Texture* texture) {
    if (!texture || !sample_count) {
      return;
    }
    NS::UInteger sc = texture->sampleCount();
    if (sc > 0) {
      *sample_count =
          std::max<uint32_t>(*sample_count, static_cast<uint32_t>(sc));
    }
  };

  if (color_formats) {
    auto* color_attachments = pass_descriptor->colorAttachments();
    for (uint32_t i = 0; i < color_count; ++i) {
      auto* attachment =
          color_attachments ? color_attachments->object(i) : nullptr;
      if (!attachment) {
        continue;
      }
      MTL::Texture* texture = attachment->texture();
      if (!texture) {
        continue;
      }
      color_formats[i] = texture->pixelFormat();
      update_sample_count(texture);
    }
  }

  if (depth_format) {
    if (auto* depth_attachment = pass_descriptor->depthAttachment()) {
      MTL::Texture* texture = depth_attachment->texture();
      if (texture) {
        *depth_format = texture->pixelFormat();
        update_sample_count(texture);
      }
    }
  }

  if (stencil_format) {
    if (auto* stencil_attachment = pass_descriptor->stencilAttachment()) {
      MTL::Texture* texture = stencil_attachment->texture();
      if (texture) {
        *stencil_format = texture->pixelFormat();
        update_sample_count(texture);
      }
    }
  }

  if (depth_format && stencil_format) {
    if (*depth_format != MTL::PixelFormatInvalid &&
        *stencil_format == MTL::PixelFormatInvalid) {
      switch (*depth_format) {
        case MTL::PixelFormatDepth32Float_Stencil8:
        case MTL::PixelFormatDepth24Unorm_Stencil8:
        case MTL::PixelFormatX32_Stencil8:
          *stencil_format = *depth_format;
          break;
        default:
          break;
      }
    } else if (*stencil_format != MTL::PixelFormatInvalid &&
               *depth_format == MTL::PixelFormatInvalid) {
      switch (*stencil_format) {
        case MTL::PixelFormatDepth32Float_Stencil8:
        case MTL::PixelFormatDepth24Unorm_Stencil8:
        case MTL::PixelFormatX32_Stencil8:
          *depth_format = *stencil_format;
          break;
        default:
          break;
      }
    }
  }
}

void EnsureDepthFormatForDepthWritingFragment(const char* pipeline_name,
                                              bool fragment_writes_depth,
                                              MTL::PixelFormat* depth_format) {
  if (!fragment_writes_depth || !depth_format ||
      *depth_format != MTL::PixelFormatInvalid) {
    return;
  }
  // Metal requires a valid depth attachment format if the fragment shader
  // writes depth even when the current render pass has no depth target bound.
  *depth_format = MTL::PixelFormatDepth32Float;
  static bool logged = false;
  if (!logged) {
    logged = true;
    XELOGW(
        "{}: fragment writes depth without a bound depth attachment; "
        "using Depth32Float pipeline fallback",
        pipeline_name);
  }
}

MTL::ComputePipelineState* CreateComputePipelineFromEmbeddedLibrary(
    MTL::Device* device, const void* metallib_data, size_t metallib_size,
    const char* debug_name) {
  if (!device || !metallib_data || !metallib_size) {
    return nullptr;
  }

  NS::Error* error = nullptr;
  dispatch_data_t data = dispatch_data_create(
      metallib_data, metallib_size, nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
  MTL::Library* lib = device->newLibrary(data, &error);
  dispatch_release(data);
  if (!lib) {
    XELOGE("Metal: failed to create {} library: {}", debug_name,
           error ? error->localizedDescription()->utf8String() : "unknown");
    return nullptr;
  }

  // XeSL compute entrypoint name used in the embedded metallibs.
  NS::String* fn_name = NS::String::string("entry_xe", NS::UTF8StringEncoding);
  MTL::Function* fn = lib->newFunction(fn_name);
  if (!fn) {
    XELOGE("Metal: {} missing entry_xe", debug_name);
    lib->release();
    return nullptr;
  }

  MTL::ComputePipelineState* pipeline =
      device->newComputePipelineState(fn, &error);
  fn->release();
  lib->release();

  if (!pipeline) {
    XELOGE("Metal: failed to create {} pipeline: {}", debug_name,
           error ? error->localizedDescription()->utf8String() : "unknown");
    return nullptr;
  }

  return pipeline;
}

constexpr uint32_t kPipelineDiskCacheMagic = 0x43504D58;  // 'XMPC'
constexpr uint32_t kPipelineDiskCacheVersion = 2;
constexpr size_t kPipelineDiskCacheMaxEntrySize = 1 << 20;

XEPACKEDSTRUCT(PipelineDiskCacheHeader, {
  uint32_t magic;
  uint32_t version;
  uint32_t reserved[2];
});

XEPACKEDSTRUCT(PipelineDiskCacheEntryHeader, {
  uint32_t entry_size;
  uint32_t reserved;
});

XEPACKEDSTRUCT(PipelineDiskCacheEntryBase, {
  uint64_t pipeline_key;
  uint64_t vertex_shader_cache_key;
  uint64_t pixel_shader_cache_key;
  uint32_t sample_count;
  uint32_t depth_format;
  uint32_t stencil_format;
  uint32_t color_formats[4];
  uint32_t normalized_color_mask;
  uint32_t alpha_to_mask_enable;
  uint32_t blendcontrol[4];
  uint32_t vertex_attribute_count;
  uint32_t vertex_layout_count;
});

static_assert(sizeof(PipelineDiskCacheHeader) == 16,
              "Unexpected pipeline disk cache header size.");
static_assert(sizeof(PipelineDiskCacheEntryHeader) == 8,
              "Unexpected pipeline disk cache entry header size.");

bool ShaderUsesVertexFetch(const Shader& shader) {
  if (!shader.vertex_bindings().empty()) {
    return true;
  }
  const Shader::ConstantRegisterMap& constant_map =
      shader.constant_register_map();
  for (uint32_t i = 0; i < xe::countof(constant_map.vertex_fetch_bitmap); ++i) {
    if (constant_map.vertex_fetch_bitmap[i] != 0) {
      return true;
    }
  }
  return false;
}

MTL::CompareFunction ToMetalCompareFunction(xenos::CompareFunction compare) {
  static const MTL::CompareFunction kCompareMap[8] = {
      MTL::CompareFunctionNever,         // 0
      MTL::CompareFunctionLess,          // 1
      MTL::CompareFunctionEqual,         // 2
      MTL::CompareFunctionLessEqual,     // 3
      MTL::CompareFunctionGreater,       // 4
      MTL::CompareFunctionNotEqual,      // 5
      MTL::CompareFunctionGreaterEqual,  // 6
      MTL::CompareFunctionAlways,        // 7
  };
  return kCompareMap[uint32_t(compare) & 0x7];
}

MTL::StencilOperation ToMetalStencilOperation(xenos::StencilOp op) {
  static const MTL::StencilOperation kStencilOpMap[8] = {
      MTL::StencilOperationKeep,            // 0
      MTL::StencilOperationZero,            // 1
      MTL::StencilOperationReplace,         // 2
      MTL::StencilOperationIncrementClamp,  // 3
      MTL::StencilOperationDecrementClamp,  // 4
      MTL::StencilOperationInvert,          // 5
      MTL::StencilOperationIncrementWrap,   // 6
      MTL::StencilOperationDecrementWrap,   // 7
  };
  return kStencilOpMap[uint32_t(op) & 0x7];
}

MTL::ColorWriteMask ToMetalColorWriteMask(uint32_t write_mask) {
  MTL::ColorWriteMask mtl_mask = MTL::ColorWriteMaskNone;
  if (write_mask & 0x1) {
    mtl_mask |= MTL::ColorWriteMaskRed;
  }
  if (write_mask & 0x2) {
    mtl_mask |= MTL::ColorWriteMaskGreen;
  }
  if (write_mask & 0x4) {
    mtl_mask |= MTL::ColorWriteMaskBlue;
  }
  if (write_mask & 0x8) {
    mtl_mask |= MTL::ColorWriteMaskAlpha;
  }
  return mtl_mask;
}

MTL::BlendOperation ToMetalBlendOperation(xenos::BlendOp blend_op) {
  // 8 entries for safety since 3 bits from the guest are passed directly.
  static const MTL::BlendOperation kBlendOpMap[8] = {
      MTL::BlendOperationAdd,              // 0
      MTL::BlendOperationSubtract,         // 1
      MTL::BlendOperationMin,              // 2
      MTL::BlendOperationMax,              // 3
      MTL::BlendOperationReverseSubtract,  // 4
      MTL::BlendOperationAdd,              // 5
      MTL::BlendOperationAdd,              // 6
      MTL::BlendOperationAdd,              // 7
  };
  return kBlendOpMap[uint32_t(blend_op) & 0x7];
}

MTL::BlendFactor ToMetalBlendFactorRgb(xenos::BlendFactor blend_factor) {
  // 32 because of 0x1F mask, for safety (all unknown to zero).
  static const MTL::BlendFactor kBlendFactorMap[32] = {
      /*  0 */ MTL::BlendFactorZero,
      /*  1 */ MTL::BlendFactorOne,
      /*  2 */ MTL::BlendFactorZero,  // ?
      /*  3 */ MTL::BlendFactorZero,  // ?
      /*  4 */ MTL::BlendFactorSourceColor,
      /*  5 */ MTL::BlendFactorOneMinusSourceColor,
      /*  6 */ MTL::BlendFactorSourceAlpha,
      /*  7 */ MTL::BlendFactorOneMinusSourceAlpha,
      /*  8 */ MTL::BlendFactorDestinationColor,
      /*  9 */ MTL::BlendFactorOneMinusDestinationColor,
      /* 10 */ MTL::BlendFactorDestinationAlpha,
      /* 11 */ MTL::BlendFactorOneMinusDestinationAlpha,
      /* 12 */ MTL::BlendFactorBlendColor,  // CONSTANT_COLOR
      /* 13 */ MTL::BlendFactorOneMinusBlendColor,
      /* 14 */ MTL::BlendFactorBlendAlpha,  // CONSTANT_ALPHA
      /* 15 */ MTL::BlendFactorOneMinusBlendAlpha,
      /* 16 */ MTL::BlendFactorSourceAlphaSaturated,
  };
  return kBlendFactorMap[uint32_t(blend_factor) & 0x1F];
}

MTL::BlendFactor ToMetalBlendFactorAlpha(xenos::BlendFactor blend_factor) {
  // Like the RGB map, but with color modes changed to alpha.
  static const MTL::BlendFactor kBlendFactorAlphaMap[32] = {
      /*  0 */ MTL::BlendFactorZero,
      /*  1 */ MTL::BlendFactorOne,
      /*  2 */ MTL::BlendFactorZero,  // ?
      /*  3 */ MTL::BlendFactorZero,  // ?
      /*  4 */ MTL::BlendFactorSourceAlpha,
      /*  5 */ MTL::BlendFactorOneMinusSourceAlpha,
      /*  6 */ MTL::BlendFactorSourceAlpha,
      /*  7 */ MTL::BlendFactorOneMinusSourceAlpha,
      /*  8 */ MTL::BlendFactorDestinationAlpha,
      /*  9 */ MTL::BlendFactorOneMinusDestinationAlpha,
      /* 10 */ MTL::BlendFactorDestinationAlpha,
      /* 11 */ MTL::BlendFactorOneMinusDestinationAlpha,
      /* 12 */ MTL::BlendFactorBlendAlpha,
      /* 13 */ MTL::BlendFactorOneMinusBlendAlpha,
      /* 14 */ MTL::BlendFactorBlendAlpha,
      /* 15 */ MTL::BlendFactorOneMinusBlendAlpha,
      /* 16 */ MTL::BlendFactorSourceAlphaSaturated,
  };
  return kBlendFactorAlphaMap[uint32_t(blend_factor) & 0x1F];
}

// snorm16_render_target_full_range: 16_16 and 16_16_16_16 render targets hold
// the guest's -32...32 as -1...1, so the shader output, alpha included, is
// divided by 32 (color_exp_bias), and a blend factor taken from it is 32 times
// too small: an additive src * src alpha blend, as Spider-Man: Edge of Time
// draws its hypersense copies, added 1/32 of what the guest adds. Where only
// the source factor reads the source, the pixel shader multiplies its output
// by that factor of the unscaled output (kSysFlag_Color0Fixed16Scaled) and the
// host source factor is one. Render target 0 only, as the pre-multiply. Sets
// the factors to pre-multiply by (kOne for a channel left as it is) and
// returns whether either is used.
bool GetFixed16SourceFactorPremult(reg::RB_BLENDCONTROL blendcontrol,
                                   xenos::BlendFactor& rgb_factor_out,
                                   xenos::BlendFactor& a_factor_out) {
  rgb_factor_out = xenos::BlendFactor::kOne;
  a_factor_out = xenos::BlendFactor::kOne;
  if (!cvars::snorm16_render_target_full_range) {
    return false;
  }
  auto linear_op = [](xenos::BlendOp op) {
    return op == xenos::BlendOp::kAdd || op == xenos::BlendOp::kSubtract ||
           op == xenos::BlendOp::kRevSubtract;
  };
  auto reads_source = [](xenos::BlendFactor factor) {
    return factor == xenos::BlendFactor::kSrcColor ||
           factor == xenos::BlendFactor::kOneMinusSrcColor ||
           factor == xenos::BlendFactor::kSrcAlpha ||
           factor == xenos::BlendFactor::kOneMinusSrcAlpha ||
           factor == xenos::BlendFactor::kSrcAlphaSaturate;
  };
  if (!linear_op(blendcontrol.color_comb_fcn) ||
      !linear_op(blendcontrol.alpha_comb_fcn) ||
      reads_source(blendcontrol.color_destblend) ||
      reads_source(blendcontrol.alpha_destblend)) {
    return false;
  }
  // Source alpha saturate also reads the destination alpha.
  if (reads_source(blendcontrol.color_srcblend) &&
      blendcontrol.color_srcblend != xenos::BlendFactor::kSrcAlphaSaturate) {
    rgb_factor_out = blendcontrol.color_srcblend;
  }
  if (reads_source(blendcontrol.alpha_srcblend) &&
      blendcontrol.alpha_srcblend != xenos::BlendFactor::kSrcAlphaSaturate) {
    a_factor_out = blendcontrol.alpha_srcblend;
  }
  return rgb_factor_out != xenos::BlendFactor::kOne ||
         a_factor_out != xenos::BlendFactor::kOne;
}

// The host source factors of render target 0 for GetFixed16SourceFactorPremult
// (16_16 and 16_16_16_16 targets are RG16Snorm and RGBA16Snorm).
void ApplyFixed16SourceFactorPremult(MTL::PixelFormat format,
                                     reg::RB_BLENDCONTROL blendcontrol,
                                     MTL::BlendFactor& src_rgb,
                                     MTL::BlendFactor& src_alpha) {
  if (format != MTL::PixelFormatRG16Snorm &&
      format != MTL::PixelFormatRGBA16Snorm) {
    return;
  }
  xenos::BlendFactor rgb_premult, a_premult;
  if (!GetFixed16SourceFactorPremult(blendcontrol, rgb_premult, a_premult)) {
    return;
  }
  if (rgb_premult != xenos::BlendFactor::kOne) {
    src_rgb = MTL::BlendFactorOne;
  }
  if (a_premult != xenos::BlendFactor::kOne) {
    src_alpha = MTL::BlendFactorOne;
  }
}

// Performance diagnostics: host CPU time of every thread in the process
// since the previous call, as a share of window_us of wall time, busiest
// first. Each entry is "name#id/priority=percent" (thread name, low bits of
// the system thread ID, current host scheduling priority). The first call
// counts each thread's whole life so far.
std::string MetalPerfThreadCpu(uint64_t window_us) {
  std::string metal_perf_text;
#if XE_PLATFORM_APPLE
  static std::unordered_map<uint64_t, uint64_t> metal_perf_previous_ns;
  std::unordered_map<uint64_t, uint64_t> metal_perf_current_ns;
  std::vector<std::pair<uint64_t, std::string>> metal_perf_threads;
  thread_act_array_t metal_perf_list = nullptr;
  mach_msg_type_number_t metal_perf_count = 0;
  if (task_threads(mach_task_self(), &metal_perf_list, &metal_perf_count) !=
      KERN_SUCCESS) {
    return " unavailable";
  }
  uint64_t metal_perf_total_ns = 0;
  for (mach_msg_type_number_t metal_perf_index = 0;
       metal_perf_index < metal_perf_count; ++metal_perf_index) {
    thread_identifier_info_data_t metal_perf_identifier;
    mach_msg_type_number_t metal_perf_info_count = THREAD_IDENTIFIER_INFO_COUNT;
    thread_extended_info_data_t metal_perf_extended;
    mach_msg_type_number_t metal_perf_extended_count =
        THREAD_EXTENDED_INFO_COUNT;
    if (thread_info(metal_perf_list[metal_perf_index], THREAD_IDENTIFIER_INFO,
                    reinterpret_cast<thread_info_t>(&metal_perf_identifier),
                    &metal_perf_info_count) == KERN_SUCCESS &&
        thread_info(metal_perf_list[metal_perf_index], THREAD_EXTENDED_INFO,
                    reinterpret_cast<thread_info_t>(&metal_perf_extended),
                    &metal_perf_extended_count) == KERN_SUCCESS) {
      const uint64_t metal_perf_ns = metal_perf_extended.pth_user_time +
                                     metal_perf_extended.pth_system_time;
      metal_perf_current_ns[metal_perf_identifier.thread_id] = metal_perf_ns;
      auto metal_perf_previous =
          metal_perf_previous_ns.find(metal_perf_identifier.thread_id);
      const uint64_t metal_perf_delta =
          metal_perf_previous != metal_perf_previous_ns.end()
              ? metal_perf_ns -
                    std::min(metal_perf_ns, metal_perf_previous->second)
              : metal_perf_ns;
      metal_perf_total_ns += metal_perf_delta;
      if (metal_perf_delta) {
        metal_perf_extended.pth_name[sizeof(metal_perf_extended.pth_name) - 1] =
            '\0';
        metal_perf_threads.emplace_back(
            metal_perf_delta,
            fmt::format("{}#{:X}/{}",
                        metal_perf_extended.pth_name[0]
                            ? metal_perf_extended.pth_name
                            : "?",
                        metal_perf_identifier.thread_id & 0xFFFF,
                        metal_perf_extended.pth_curpri));
      }
    }
    mach_port_deallocate(mach_task_self(), metal_perf_list[metal_perf_index]);
  }
  vm_deallocate(mach_task_self(), vm_address_t(metal_perf_list),
                metal_perf_count * sizeof(thread_act_t));
  metal_perf_previous_ns.swap(metal_perf_current_ns);
  std::sort(metal_perf_threads.begin(), metal_perf_threads.end(),
            [](const auto& metal_perf_left, const auto& metal_perf_right) {
              return metal_perf_left.first > metal_perf_right.first;
            });
  const double metal_perf_window_ns =
      double(std::max<uint64_t>(window_us, 1)) * 1000.0;
  metal_perf_text = fmt::format(
      " total {:.0f}% of one core, {} threads;",
      metal_perf_total_ns * 100.0 / metal_perf_window_ns, metal_perf_count);
  for (size_t metal_perf_index = 0;
       metal_perf_index < std::min<size_t>(metal_perf_threads.size(), 16);
       ++metal_perf_index) {
    metal_perf_text +=
        fmt::format(" {}={:.0f}%", metal_perf_threads[metal_perf_index].second,
                    metal_perf_threads[metal_perf_index].first * 100.0 /
                        metal_perf_window_ns);
  }
#else
  metal_perf_text = " unavailable";
#endif  // XE_PLATFORM_APPLE
  return metal_perf_text;
}

}  // namespace

MetalCommandProcessor::MetalCommandProcessor(
    MetalGraphicsSystem* graphics_system, kernel::KernelState* kernel_state)
    : CommandProcessor(graphics_system, kernel_state) {}

MetalCommandProcessor::~MetalCommandProcessor() {
  // End any active render encoder before releasing
  // Note: Only call endEncoding if the encoder is still active
  // (not already ended by a committed command buffer)
  if (current_render_encoder_) {
    // The encoder may already be ended if the command buffer was committed
    // In that case, just release it
    current_render_encoder_->release();
    current_render_encoder_ = nullptr;
    metal_gpu_pass_record_ = nullptr;
  }
  if (current_command_buffer_) {
    current_command_buffer_->release();
    current_command_buffer_ = nullptr;
  }
  WaitForPendingCompletionHandlers();
  ShutdownMslAsyncCompilation();
  if (render_pass_descriptor_) {
    render_pass_descriptor_->release();
    render_pass_descriptor_ = nullptr;
  }
  if (render_target_texture_) {
    render_target_texture_->release();
    render_target_texture_ = nullptr;
  }
  if (depth_stencil_texture_) {
    depth_stencil_texture_->release();
    depth_stencil_texture_ = nullptr;
  }

#if METAL_SHADER_CONVERTER_AVAILABLE
  // Release MSC pipeline caches
  for (auto& pair : pipeline_cache_) {
    if (pair.second) {
      pair.second->release();
    }
  }
  pipeline_cache_.clear();

  for (auto& pair : geometry_pipeline_cache_) {
    if (pair.second.pipeline) {
      pair.second.pipeline->release();
    }
  }
  geometry_pipeline_cache_.clear();

  for (auto& pair : geometry_vertex_stage_cache_) {
    if (pair.second.library) {
      pair.second.library->release();
    }
    if (pair.second.stage_in_library) {
      pair.second.stage_in_library->release();
    }
  }
  geometry_vertex_stage_cache_.clear();

  for (auto& pair : geometry_shader_stage_cache_) {
    if (pair.second.library) {
      pair.second.library->release();
    }
  }
  geometry_shader_stage_cache_.clear();
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

  for (auto& pair : depth_stencil_state_cache_) {
    if (pair.second) {
      pair.second->release();
    }
  }
  depth_stencil_state_cache_.clear();

  // Release IR Converter runtime buffers and resources
  if (null_buffer_) {
    null_buffer_->release();
    null_buffer_ = nullptr;
  }
  if (spirv2air_texture_indices_) {
    spirv2air_texture_indices_->release();
    spirv2air_texture_indices_ = nullptr;
  }
  if (null_texture_) {
    null_texture_->release();
    null_texture_ = nullptr;
  }
  if (null_sampler_) {
    null_sampler_->release();
    null_sampler_ = nullptr;
  }
#if METAL_SHADER_CONVERTER_AVAILABLE
  {
    std::lock_guard<std::mutex> lock(draw_ring_mutex_);
    FlushPendingDrawRingReleasesLocked();
    active_draw_ring_.reset();
    draw_ring_pool_.clear();
    command_buffer_draw_rings_.clear();
    draw_ring_pool_total_bytes_ = 0;
    draw_ring_pool_in_use_count_ = 0;
    draw_ring_pool_wait_count_ = 0;
    draw_ring_pool_timeout_count_ = 0;
  }
  draw_ring_pool_cv_.notify_all();
  res_heap_ab_ = nullptr;
  smp_heap_ab_ = nullptr;
  cbv_heap_ab_ = nullptr;
  uniforms_buffer_ = nullptr;
  top_level_ab_ = nullptr;
  draw_args_buffer_ = nullptr;

  ShutdownShaderStorage();
#else
  uniforms_buffer_ = nullptr;
  command_buffer_spirv_uniform_buffers_.clear();
  {
    std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
    spirv_uniforms_available_.clear();
    for (MTL::Buffer* pool_uniforms : spirv_uniforms_pool_) {
      if (pool_uniforms) {
        pool_uniforms->release();
      }
    }
    spirv_uniforms_pool_.clear();
    spirv_uniforms_pool_initialized_ = false;
  }
  if (spirv_uniforms_available_semaphore_) {
#if !OS_OBJECT_USE_OBJC
    dispatch_release(spirv_uniforms_available_semaphore_);
#endif
    spirv_uniforms_available_semaphore_ = nullptr;
  }
#endif  // METAL_SHADER_CONVERTER_AVAILABLE
}

void MetalCommandProcessor::InitializeMslAsyncCompilation() {
  ShutdownMslAsyncCompilation();

  if (!cvars::async_shader_compilation) {
    return;
  }

  uint32_t logical_processor_count = std::thread::hardware_concurrency();
  if (!logical_processor_count) {
    logical_processor_count = 6;
  }

  if (cvars::metal_pipeline_creation_threads == 0) {
    return;
  }

  size_t thread_count = 0;
  if (cvars::metal_pipeline_creation_threads < 0) {
    thread_count = std::max<uint32_t>(logical_processor_count * 3 / 4, 1);
  } else {
    thread_count =
        std::min<uint32_t>(uint32_t(cvars::metal_pipeline_creation_threads),
                           logical_processor_count);
  }
  if (!thread_count) {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
    msl_shader_compile_shutdown_ = false;
  }

  msl_shader_compile_threads_.reserve(thread_count);
  for (size_t i = 0; i < thread_count; ++i) {
    msl_shader_compile_threads_.emplace_back(
        [this, i]() { MslShaderCompileThread(i); });
  }

  XELOGI(
      "SPIRV-Cross: async Metal shader/pipeline compilation enabled with {} "
      "worker "
      "thread(s)",
      thread_count);
}

void MetalCommandProcessor::ShutdownMslAsyncCompilation() {
  {
    std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
    msl_shader_compile_shutdown_ = true;
  }
  msl_shader_compile_cv_.notify_all();
  msl_compile_done_cv_.notify_all();

  for (std::thread& thread : msl_shader_compile_threads_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  msl_shader_compile_threads_.clear();

  std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
  std::priority_queue<MslShaderCompileRequest,
                      std::vector<MslShaderCompileRequest>,
                      MslShaderCompileRequestCompare>
      empty_queue;
  std::swap(msl_shader_compile_queue_, empty_queue);
  while (!msl_pipeline_compile_queue_.empty()) {
    auto request = msl_pipeline_compile_queue_.top();
    msl_pipeline_compile_queue_.pop();
    if (request.vertex_function) {
      request.vertex_function->release();
      request.vertex_function = nullptr;
    }
    if (request.fragment_function) {
      request.fragment_function->release();
      request.fragment_function = nullptr;
    }
  }
  msl_shader_compile_pending_.clear();
  msl_shader_compile_failed_.clear();
  msl_pipeline_compile_pending_.clear();
  msl_pipeline_compile_failed_.clear();
  msl_shader_compile_busy_ = 0;
  msl_shader_compile_shutdown_ = false;
}

MetalCommandProcessor::MslShaderCompileStatus
MetalCommandProcessor::GetMslShaderCompileStatus(
    MslShader::MslTranslation* translation) {
  if (!translation) {
    return MslShaderCompileStatus::kFailed;
  }

  std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
  if (msl_shader_compile_failed_.find(translation) !=
      msl_shader_compile_failed_.end()) {
    return MslShaderCompileStatus::kFailed;
  }
  if (msl_shader_compile_pending_.find(translation) !=
      msl_shader_compile_pending_.end()) {
    return MslShaderCompileStatus::kPending;
  }
  return translation->is_valid() ? MslShaderCompileStatus::kReady
                                 : MslShaderCompileStatus::kNotQueued;
}

bool MetalCommandProcessor::WaitForMslCompileProgress() {
  const int32_t wait_ms = cvars::metal_async_compile_wait_ms;
  if (wait_ms <= 0) {
    return false;
  }
  const uint64_t budget_us = uint64_t(wait_ms) * 1000;
  if (msl_compile_wait_used_us_ >= budget_us) {
    return false;
  }
  const auto wait_begin = std::chrono::steady_clock::now();
  {
    std::unique_lock<std::mutex> lock(msl_shader_compile_mutex_);
    if (msl_shader_compile_shutdown_) {
      return false;
    }
    const uint64_t completions = msl_compile_completions_;
    msl_compile_done_cv_.wait_until(
        lock,
        wait_begin +
            std::chrono::microseconds(budget_us - msl_compile_wait_used_us_),
        [&]() {
          return msl_compile_completions_ != completions ||
                 msl_shader_compile_shutdown_;
        });
  }
  const uint64_t waited_us = MetalPerfMicrosecondsSince(wait_begin);
  msl_compile_wait_used_us_ += waited_us;
  metal_perf_counters.compile_waits.fetch_add(1, std::memory_order_relaxed);
  metal_perf_counters.compile_wait_us.fetch_add(waited_us,
                                                std::memory_order_relaxed);
  return true;
}

bool MetalCommandProcessor::EnqueueMslShaderCompilation(
    MslShader::MslTranslation* translation, bool is_ios, uint8_t priority) {
  if (!translation || !cvars::async_shader_compilation ||
      msl_shader_compile_threads_.empty()) {
    return false;
  }

  if (translation->is_valid()) {
    return true;
  }

  {
    std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
    if (msl_shader_compile_failed_.find(translation) !=
        msl_shader_compile_failed_.end()) {
      return false;
    }
    auto pending_it = msl_shader_compile_pending_.find(translation);
    if (pending_it != msl_shader_compile_pending_.end() &&
        pending_it->second >= msl_compile_frame_arrival_) {
      return true;
    }

    MslShaderCompileRequest request;
    request.translation = translation;
    request.shader_hash = translation->shader().ucode_data_hash();
    request.modification = translation->modification();
    request.is_ios = is_ios;
    request.priority = priority;
    request.queue_arrival = msl_compile_arrivals_++;
    msl_shader_compile_queue_.push(request);
    if (pending_it != msl_shader_compile_pending_.end()) {
      // Queued in an earlier frame and still needed: queued again as the
      // latest request, its older entry dropped when it comes up.
      pending_it->second = request.queue_arrival;
    } else {
      msl_shader_compile_pending_.emplace(translation, request.queue_arrival);
      metal_compile_stall_tracker.Queued(
          {translation->shader().type() == xenos::ShaderType::kVertex ? 1u : 2u,
           request.shader_hash, request.modification, 0, 0});
    }
  }
  msl_shader_compile_cv_.notify_one();
  return true;
}

bool MetalCommandProcessor::EnqueueMslPipelineCompilation(
    const MslPipelineCompileRequest& request) {
  if (!cvars::async_shader_compilation || msl_shader_compile_threads_.empty() ||
      !request.vertex_function) {
    return false;
  }

  MslPipelineCompileRequest queued_request = request;
  {
    std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
    if (msl_pipeline_cache_.find(request.pipeline_key) !=
        msl_pipeline_cache_.end()) {
      return true;
    }
    if (msl_pipeline_compile_failed_.find(request.pipeline_key) !=
        msl_pipeline_compile_failed_.end()) {
      return false;
    }
    if (msl_pipeline_compile_pending_.find(request.pipeline_key) !=
        msl_pipeline_compile_pending_.end()) {
      return true;
    }

    queued_request.vertex_function->retain();
    if (queued_request.fragment_function) {
      queued_request.fragment_function->retain();
    }
    queued_request.queue_arrival = msl_compile_arrivals_++;
    msl_pipeline_compile_pending_.insert(request.pipeline_key);
    msl_pipeline_compile_queue_.push(queued_request);
    metal_compile_stall_tracker.Queued(
        {3, request.vertex_shader_hash, request.vertex_modification,
         request.pixel_shader_hash, request.pixel_modification});
  }

  msl_shader_compile_cv_.notify_one();
  return true;
}

MTL::RenderPipelineState* MetalCommandProcessor::CreateMslPipelineState(
    const MslPipelineCompileRequest& request, std::string* error_out) {
  if (error_out) {
    error_out->clear();
  }
  if (!request.vertex_function) {
    if (error_out) {
      *error_out = "missing vertex shader function";
    }
    return nullptr;
  }

  MTL::RenderPipelineDescriptor* desc =
      MTL::RenderPipelineDescriptor::alloc()->init();
  desc->setVertexFunction(request.vertex_function);
  if (request.fragment_function) {
    desc->setFragmentFunction(request.fragment_function);
  }

  for (uint32_t i = 0; i < 4; ++i) {
    desc->colorAttachments()->object(i)->setPixelFormat(
        request.color_formats[i]);
  }
  desc->setDepthAttachmentPixelFormat(request.depth_format);
  desc->setStencilAttachmentPixelFormat(request.stencil_format);
  desc->setSampleCount(request.sample_count);
  desc->setAlphaToCoverageEnabled(request.alpha_to_mask_enable != 0);

  for (uint32_t i = 0; i < 4; ++i) {
    auto* color_attachment = desc->colorAttachments()->object(i);
    if (request.color_formats[i] == MTL::PixelFormatInvalid) {
      color_attachment->setWriteMask(MTL::ColorWriteMaskNone);
      color_attachment->setBlendingEnabled(false);
      continue;
    }

    uint32_t rt_write_mask = (request.normalized_color_mask >> (i * 4)) & 0xF;
    color_attachment->setWriteMask(ToMetalColorWriteMask(rt_write_mask));
    if (!rt_write_mask) {
      color_attachment->setBlendingEnabled(false);
      continue;
    }

    reg::RB_BLENDCONTROL blendcontrol;
    blendcontrol.value = request.blendcontrol[i];

    MTL::BlendFactor src_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_srcblend);
    MTL::BlendFactor dst_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_destblend);
    MTL::BlendOperation op_rgb =
        ToMetalBlendOperation(blendcontrol.color_comb_fcn);
    MTL::BlendFactor src_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_srcblend);
    MTL::BlendFactor dst_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_destblend);
    MTL::BlendOperation op_alpha =
        ToMetalBlendOperation(blendcontrol.alpha_comb_fcn);
    if (i == 0) {
      ApplyFixed16SourceFactorPremult(request.color_formats[0], blendcontrol,
                                      src_rgb, src_alpha);
    }

    bool blending_enabled =
        src_rgb != MTL::BlendFactorOne || dst_rgb != MTL::BlendFactorZero ||
        op_rgb != MTL::BlendOperationAdd || src_alpha != MTL::BlendFactorOne ||
        dst_alpha != MTL::BlendFactorZero || op_alpha != MTL::BlendOperationAdd;
    color_attachment->setBlendingEnabled(blending_enabled);
    if (blending_enabled) {
      color_attachment->setSourceRGBBlendFactor(src_rgb);
      color_attachment->setDestinationRGBBlendFactor(dst_rgb);
      color_attachment->setRgbBlendOperation(op_rgb);
      color_attachment->setSourceAlphaBlendFactor(src_alpha);
      color_attachment->setDestinationAlphaBlendFactor(dst_alpha);
      color_attachment->setAlphaBlendOperation(op_alpha);
    }
  }

#if METAL_SHADER_CONVERTER_AVAILABLE
  {
    std::lock_guard<std::mutex> lock(pipeline_binary_archive_mutex_);
    if (pipeline_binary_archive_) {
      NS::Array* archives = NS::Array::array(pipeline_binary_archive_);
      desc->setBinaryArchives(archives);
      NS::Error* archive_error = nullptr;
      if (pipeline_binary_archive_->addRenderPipelineFunctions(
              desc, &archive_error)) {
        pipeline_binary_archive_dirty_ = true;
      }
    }
  }
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

  NS::Error* error = nullptr;
  const auto pipeline_compile_begin = std::chrono::steady_clock::now();
  MTL::RenderPipelineState* pipeline =
      device_->newRenderPipelineState(desc, &error);
  metal_perf_counters.pipeline_compiles.fetch_add(1, std::memory_order_relaxed);
  metal_perf_counters.pipeline_compile_us.fetch_add(
      MetalPerfMicrosecondsSince(pipeline_compile_begin),
      std::memory_order_relaxed);
  desc->release();

  if (!pipeline && error_out && error) {
    NS::String* description = error->localizedDescription();
    if (description) {
      *error_out = description->utf8String();
    }
  }

  return pipeline;
}

void MetalCommandProcessor::MslShaderCompileThread(size_t thread_index) {
  // Named as in XeniOS, so the thread CPU report shows them.
  xe::threading::set_name(fmt::format("Metal Shaders {}", thread_index));
  while (true) {
    MslShaderCompileRequest shader_request;
    MslPipelineCompileRequest pipeline_request;
    bool process_pipeline_request = false;
    {
      std::unique_lock<std::mutex> lock(msl_shader_compile_mutex_);
      msl_shader_compile_cv_.wait(lock, [this]() {
        return msl_shader_compile_shutdown_ ||
               !msl_shader_compile_queue_.empty() ||
               !msl_pipeline_compile_queue_.empty();
      });
      if (msl_shader_compile_shutdown_) {
        return;
      }
      if (!msl_pipeline_compile_queue_.empty()) {
        process_pipeline_request = true;
        pipeline_request = msl_pipeline_compile_queue_.top();
        msl_pipeline_compile_queue_.pop();
      } else {
        shader_request = msl_shader_compile_queue_.top();
        msl_shader_compile_queue_.pop();
        // An entry a later request of the same shader replaced (or of one
        // compiled already) is dropped: only the latest one compiles.
        auto pending_it =
            msl_shader_compile_pending_.find(shader_request.translation);
        if (pending_it == msl_shader_compile_pending_.end() ||
            pending_it->second != shader_request.queue_arrival) {
          continue;
        }
        // Compiling: later requests queue nothing until it's done.
        pending_it->second = UINT64_MAX;
      }
      ++msl_shader_compile_busy_;
    }

    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    if (process_pipeline_request) {
      std::string pipeline_error;
      const auto compile_begin = std::chrono::steady_clock::now();
      MTL::RenderPipelineState* pipeline =
          CreateMslPipelineState(pipeline_request, &pipeline_error);
      const uint64_t compile_us = MetalPerfMicrosecondsSince(compile_begin);
      bool compiled = pipeline != nullptr;

      {
        std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
        msl_pipeline_compile_pending_.erase(pipeline_request.pipeline_key);
        if (compiled) {
          auto insert_result = msl_pipeline_cache_.emplace(
              pipeline_request.pipeline_key, pipeline);
          if (!insert_result.second && pipeline) {
            pipeline->release();
          }
          msl_pipeline_compile_failed_.erase(pipeline_request.pipeline_key);
        } else {
          msl_pipeline_compile_failed_.insert(pipeline_request.pipeline_key);
        }
        if (msl_shader_compile_busy_) {
          --msl_shader_compile_busy_;
        }
        ++msl_compile_completions_;
      }
      msl_compile_done_cv_.notify_all();
      // After the pending entry is gone, so no draw skips for it afterwards.
      metal_compile_stall_tracker.Finished(
          {3, pipeline_request.vertex_shader_hash,
           pipeline_request.vertex_modification,
           pipeline_request.pixel_shader_hash,
           pipeline_request.pixel_modification},
          compile_us);

      if (pipeline_request.vertex_function) {
        pipeline_request.vertex_function->release();
      }
      if (pipeline_request.fragment_function) {
        pipeline_request.fragment_function->release();
      }

      if (!compiled &&
          ShouldLogRateLimited(msl_pipeline_compile_failure_last_log_ns_,
                               kMslAsyncLogIntervalNs)) {
        if (!pipeline_error.empty()) {
          XELOGE(
              "SPIRV-Cross: async Metal pipeline compile failed on worker {} "
              "(VS {:016X} mod {:016X}, PS {:016X} mod {:016X}): {}",
              thread_index, pipeline_request.vertex_shader_hash,
              pipeline_request.vertex_modification,
              pipeline_request.pixel_shader_hash,
              pipeline_request.pixel_modification, pipeline_error);
        } else {
          XELOGE(
              "SPIRV-Cross: async Metal pipeline compile failed on worker {} "
              "(VS {:016X} mod {:016X}, PS {:016X} mod {:016X})",
              thread_index, pipeline_request.vertex_shader_hash,
              pipeline_request.vertex_modification,
              pipeline_request.pixel_shader_hash,
              pipeline_request.pixel_modification);
        }
      }
    } else {
      bool compiled = false;
      uint64_t compile_us = 0;
      if (shader_request.translation) {
        const auto compile_begin = std::chrono::steady_clock::now();
        compiled = shader_request.translation->CompileToMsl(
            device_, shader_request.is_ios);
        compile_us = MetalPerfMicrosecondsSince(compile_begin);
      }

      {
        std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
        if (shader_request.translation) {
          msl_shader_compile_pending_.erase(shader_request.translation);
          if (!compiled) {
            msl_shader_compile_failed_.insert(shader_request.translation);
          }
        }
        if (msl_shader_compile_busy_) {
          --msl_shader_compile_busy_;
        }
        ++msl_compile_completions_;
      }
      msl_compile_done_cv_.notify_all();
      if (shader_request.translation) {
        metal_compile_stall_tracker.Finished(
            {shader_request.translation->shader().type() ==
                     xenos::ShaderType::kVertex
                 ? 1u
                 : 2u,
             shader_request.shader_hash, shader_request.modification, 0, 0},
            compile_us);
      }

      if (!compiled &&
          ShouldLogRateLimited(msl_shader_compile_failure_last_log_ns_,
                               kMslAsyncLogIntervalNs)) {
        XELOGE(
            "SPIRV-Cross: async Metal compile failed on worker {} (shader "
            "{:016X}, mod {:016X})",
            thread_index, shader_request.shader_hash,
            shader_request.modification);
      }
    }
    pool->release();
  }
}

#if METAL_SHADER_CONVERTER_AVAILABLE
MetalCommandProcessor::DrawRingBuffers::~DrawRingBuffers() {
  if (res_heap_ab) {
    res_heap_ab->release();
    res_heap_ab = nullptr;
  }
  if (smp_heap_ab) {
    smp_heap_ab->release();
    smp_heap_ab = nullptr;
  }
  if (cbv_heap_ab) {
    cbv_heap_ab->release();
    cbv_heap_ab = nullptr;
  }
  if (uniforms_buffer) {
    uniforms_buffer->release();
    uniforms_buffer = nullptr;
  }
  if (top_level_ab) {
    top_level_ab->release();
    top_level_ab = nullptr;
  }
  if (draw_args_buffer) {
    draw_args_buffer->release();
    draw_args_buffer = nullptr;
  }
}
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

MetalCommandProcessor::SpirvArgumentBufferPage::~SpirvArgumentBufferPage() {
  if (buffer) {
    buffer->release();
    buffer = nullptr;
  }
}

void MetalCommandProcessor::TracePlaybackWroteMemory(uint32_t base_ptr,
                                                     uint32_t length) {
  if (shared_memory_) {
    shared_memory_->MemoryInvalidationCallback(base_ptr, length, true);
  }
  if (primitive_processor_) {
    primitive_processor_->MemoryInvalidationCallback(base_ptr, length, true);
  }
}

void MetalCommandProcessor::RestoreEdramSnapshot(const void* snapshot) {
  // Restore the guest EDRAM snapshot captured in the trace into the Metal
  // render-target cache so that subsequent host render targets created from
  // EDRAM (via LoadTiledData) see the same initial contents as other
  // backends like D3D12.
  if (!snapshot) {
    XELOGW(
        "MetalCommandProcessor::RestoreEdramSnapshot called with null "
        "snapshot");
    return;
  }
  if (!render_target_cache_) {
    XELOGW(
        "MetalCommandProcessor::RestoreEdramSnapshot called before render "
        "target "
        "cache initialization");
    return;
  }
  // Trace playback frame boundary: drop resolve-write tracking from previous
  // frame before restoring a new snapshot.
  ClearResolvedMemory();
  render_target_cache_->RestoreEdramSnapshot(snapshot);
}

void MetalCommandProcessor::ClearCaches() {
  CommandProcessor::ClearCaches();
  // TODO(wmarti): Add cache_clear_requested_ flag like D3D12 for deferred
  // clearing of pipeline caches, texture caches, etc.
}

void MetalCommandProcessor::InvalidateGpuMemory() {
  if (shared_memory_) {
    shared_memory_->InvalidateAllPages();
  }
}

void MetalCommandProcessor::ClearReadbackBuffers() {
  // TODO(wmarti): Implement readback buffer clearing when memexport readback
  // is added. See D3D12's readback_buffers_ and memexport_readback_buffers_.
}

ui::metal::MetalProvider& MetalCommandProcessor::GetMetalProvider() const {
  return *static_cast<ui::metal::MetalProvider*>(graphics_system_->provider());
}

uint64_t MetalCommandProcessor::GetCurrentSubmission() const {
  return submission_current_ ? submission_current_ : 1;
}

uint64_t MetalCommandProcessor::GetCompletedSubmission() const {
  return completed_command_buffers_.load(std::memory_order_relaxed);
}

void MetalCommandProcessor::MarkResolvedMemory(uint32_t base_ptr,
                                               uint32_t length) {
  if (length == 0) {
    return;
  }
  constexpr uint64_t kAddressLimit =
      uint64_t(std::numeric_limits<uint32_t>::max()) + 1ull;
  uint64_t merged_base = base_ptr;
  uint64_t merged_end =
      std::min<uint64_t>(merged_base + uint64_t(length), kAddressLimit);
  if (merged_end <= merged_base) {
    return;
  }

  for (size_t i = 0; i < resolved_memory_ranges_.size();) {
    const auto& range = resolved_memory_ranges_[i];
    const uint64_t range_base = range.base;
    const uint64_t range_end =
        std::min<uint64_t>(range_base + uint64_t(range.length), kAddressLimit);
    // Merge overlapping or adjacent ranges.
    if (merged_end + 1 < range_base || range_end + 1 < merged_base) {
      ++i;
      continue;
    }
    merged_base = std::min(merged_base, range_base);
    merged_end = std::max(merged_end, range_end);
    resolved_memory_ranges_.erase(resolved_memory_ranges_.begin() + i);
  }

  const uint64_t merged_length_64 =
      std::min<uint64_t>(merged_end - merged_base, kAddressLimit - merged_base);
  if (!merged_length_64) {
    return;
  }
  ResolvedRange merged_range = {uint32_t(merged_base),
                                uint32_t(merged_length_64)};
  auto insert_it = std::lower_bound(
      resolved_memory_ranges_.begin(), resolved_memory_ranges_.end(),
      merged_range, [](const ResolvedRange& lhs, const ResolvedRange& rhs) {
        return lhs.base < rhs.base;
      });
  resolved_memory_ranges_.insert(insert_it, merged_range);

  if (resolved_memory_ranges_.size() <= kResolvedMemoryRangesMax) {
    return;
  }

  std::sort(resolved_memory_ranges_.begin(), resolved_memory_ranges_.end(),
            [](const ResolvedRange& lhs, const ResolvedRange& rhs) {
              return lhs.base < rhs.base;
            });
  while (resolved_memory_ranges_.size() > kResolvedMemoryRangesMax) {
    size_t best_index = std::numeric_limits<size_t>::max();
    uint64_t best_gap = std::numeric_limits<uint64_t>::max();
    for (size_t i = 0; i + 1 < resolved_memory_ranges_.size(); ++i) {
      const auto& left = resolved_memory_ranges_[i];
      const auto& right = resolved_memory_ranges_[i + 1];
      const uint64_t left_end = uint64_t(left.base) + uint64_t(left.length);
      const uint64_t right_base = uint64_t(right.base);
      const uint64_t gap = right_base > left_end ? right_base - left_end : 0;
      if (gap < best_gap) {
        best_gap = gap;
        best_index = i;
        if (!gap) {
          break;
        }
      }
    }
    if (best_index == std::numeric_limits<size_t>::max()) {
      break;
    }
    auto& left = resolved_memory_ranges_[best_index];
    const auto& right = resolved_memory_ranges_[best_index + 1];
    const uint64_t merged_base_64 =
        std::min<uint64_t>(left.base, uint64_t(right.base));
    const uint64_t merged_end_64 =
        std::max<uint64_t>(uint64_t(left.base) + uint64_t(left.length),
                           uint64_t(right.base) + uint64_t(right.length));
    const uint64_t merged_len_64 = std::min<uint64_t>(
        merged_end_64 - merged_base_64, kAddressLimit - merged_base_64);
    left.base = uint32_t(merged_base_64);
    left.length = uint32_t(std::max<uint64_t>(1, merged_len_64));
    resolved_memory_ranges_.erase(resolved_memory_ranges_.begin() + best_index +
                                  1);
  }
}

bool MetalCommandProcessor::IsResolvedMemory(uint32_t base_ptr,
                                             uint32_t length) const {
  const uint64_t end_ptr = uint64_t(base_ptr) + uint64_t(length);
  for (const auto& range : resolved_memory_ranges_) {
    const uint64_t range_end = uint64_t(range.base) + uint64_t(range.length);
    // Check if ranges overlap
    if (uint64_t(base_ptr) < range_end && end_ptr > uint64_t(range.base)) {
      return true;
    }
  }
  return false;
}

void MetalCommandProcessor::ClearResolvedMemory() {
  resolved_memory_ranges_.clear();
}

void MetalCommandProcessor::ForceIssueSwap() {
  // Force a swap to push any pending render target to presenter
  // This is used by trace dumps to capture output when there's no explicit swap
  if (saw_swap_) {
    return;
  }
  IssueSwap(0, render_target_width_, render_target_height_);
}

void MetalCommandProcessor::SetSwapDestSwap(uint32_t dest_base, bool swap) {
  if (!dest_base) {
    return;
  }
  if (swap_dest_swaps_by_base_.size() > 256) {
    swap_dest_swaps_by_base_.clear();
  }
  swap_dest_swaps_by_base_[dest_base] = swap;
}

bool MetalCommandProcessor::ConsumeSwapDestSwap(uint32_t dest_base,
                                                bool* swap_out) {
  if (!swap_out || !dest_base) {
    return false;
  }
  auto it = swap_dest_swaps_by_base_.find(dest_base);
  if (it == swap_dest_swaps_by_base_.end()) {
    return false;
  }
  *swap_out = it->second;
  swap_dest_swaps_by_base_.erase(it);
  return true;
}

bool MetalCommandProcessor::SetupContext() {
  // A capture starts when the setting changes, not at every launch.
  frame_detail_setting_ = cvars::metal_frame_detail_log;
  saw_swap_ = false;
  last_swap_ptr_ = 0;
  last_swap_width_ = 0;
  last_swap_height_ = 0;
  swap_dest_swaps_by_base_.clear();
  gamma_ramp_256_entry_table_up_to_date_ = false;
  gamma_ramp_pwl_up_to_date_ = false;
  if (!CommandProcessor::SetupContext()) {
    XELOGE("Failed to initialize base command processor context");
    return false;
  }

  const ui::metal::MetalProvider& provider = GetMetalProvider();
  device_ = provider.GetDevice();
  command_queue_ = provider.GetCommandQueue();

  if (!device_ || !command_queue_) {
    XELOGE("MetalCommandProcessor: No Metal device or command queue available");
    return false;
  }

  metal_gpu_pass_timer.Initialize(device_);
  metal_compile_stall_tracker.Clear();

  wait_shared_event_ = device_->newSharedEvent();
  if (wait_shared_event_) {
    wait_shared_event_->setLabel(
        NS::String::string("XeniaWaitEvent", NS::UTF8StringEncoding));
    wait_shared_event_value_ = 0;
  } else {
    XELOGW(
        "MetalCommandProcessor: SharedEvent unavailable; falling back to "
        "waitUntilCompleted");
  }

  bool supports_apple7 = device_->supportsFamily(MTL::GPUFamilyApple7);
  bool supports_mac2 = device_->supportsFamily(MTL::GPUFamilyMac2);
  mesh_shader_supported_ = supports_apple7 || supports_mac2;

  draw_ring_count_ = std::max<int32_t>(1, ::cvars::metal_draw_ring_count);
  if (UseSpirvCrossPath()) {
    // TODO(wmarti): Investigate SPIRV-Cross uniform/constant corruption when
    // allowing very large per-command-buffer ring sizes. Keep the per-command-
    // buffer ring bounded for correctness, and rely on multi-buffer pool growth
    // in EnsureSpirvUniformBuffer* for throughput.
    constexpr size_t kMaxSpirvRingPagesPerCommandBuffer = 8;
    if (draw_ring_count_ > kMaxSpirvRingPagesPerCommandBuffer) {
      XELOGW(
          "SPIRV-Cross: clamping per-command-buffer ring pages from {} to {} "
          "for correctness",
          draw_ring_count_, kMaxSpirvRingPagesPerCommandBuffer);
      draw_ring_count_ = kMaxSpirvRingPagesPerCommandBuffer;
    }
  }
  msl_system_constants_version_ = 1;
  msl_clip_plane_constants_version_ = 1;
  msl_tessellation_constants_version_ = 1;
  msl_constants_versioned_uniform_buffer_ = nullptr;
  msl_system_constants_written_vertex_versions_.assign(draw_ring_count_, 0);
  msl_system_constants_written_pixel_versions_.assign(draw_ring_count_, 0);
  msl_clip_plane_constants_written_vertex_versions_.assign(draw_ring_count_, 0);
  msl_tessellation_constants_written_vertex_versions_.assign(draw_ring_count_,
                                                             0);
  msl_tessellation_constants_written_pixel_versions_.assign(draw_ring_count_,
                                                            0);
  msl_current_float_constant_map_vertex_.fill(0);
  msl_current_float_constant_map_pixel_.fill(0);
  msl_float_constants_dirty_vertex_ = true;
  msl_float_constants_dirty_pixel_ = true;
  msl_bool_loop_constants_dirty_ = true;
  msl_fetch_constants_dirty_ = true;
  msl_bound_vertex_texture_binding_uid_ = 0;
  msl_bound_pixel_texture_binding_uid_ = 0;
  msl_bound_vertex_sampler_binding_uid_ = 0;
  msl_bound_pixel_sampler_binding_uid_ = 0;
  msl_bound_vertex_argument_buffer_offset_ = 0;
  msl_bound_pixel_argument_buffer_offset_ = 0;
  msl_bound_vertex_argument_buffer_offset_valid_ = false;
  msl_bound_pixel_argument_buffer_offset_valid_ = false;
  msl_last_argbuf_vertex_translation_ = nullptr;
  msl_last_argbuf_vertex_encoded_length_ = 0;
  msl_last_argbuf_vertex_layout_uid_ = 0;
  msl_last_argbuf_pixel_translation_ = nullptr;
  msl_last_argbuf_pixel_encoded_length_ = 0;
  msl_last_argbuf_pixel_layout_uid_ = 0;
  msl_bound_uniforms_buffer_ = nullptr;
  msl_bound_uniforms_vs_base_offset_ = 0;
  msl_bound_uniforms_ps_base_offset_ = 0;
  msl_bound_uniforms_offsets_valid_ = false;
#if METAL_SHADER_CONVERTER_AVAILABLE
  draw_ring_pool_max_bytes_ = std::max<uint64_t>(
      kDefaultDrawRingPoolMaxBytes, GetDrawRingAllocationSizeBytes());
  draw_ring_pool_total_bytes_ = 0;
  draw_ring_pool_in_use_count_ = 0;
  draw_ring_pool_wait_count_ = 0;
  draw_ring_pool_timeout_count_ = 0;
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

  // Initialize shared memory
  shared_memory_ =
      std::make_unique<MetalSharedMemory>(*this, *memory_, trace_writer_);
  if (!shared_memory_->Initialize()) {
    XELOGE("Failed to initialize shared memory");
    return false;
  }

  // Initialize primitive processor (index/primitive conversion like D3D12).
  primitive_processor_ = std::make_unique<MetalPrimitiveProcessor>(
      *this, *register_file_, *memory_, trace_writer_, *shared_memory_);
  if (!primitive_processor_->Initialize()) {
    XELOGE("Failed to initialize Metal primitive processor");
    return false;
  }

  texture_cache_ = std::make_unique<MetalTextureCache>(this, *register_file_,
                                                       *shared_memory_, 1, 1);
  if (!texture_cache_->Initialize()) {
    XELOGE("Failed to initialize Metal texture cache");
    return false;
  }

  // Initialize render target cache
  render_target_cache_ = std::make_unique<MetalRenderTargetCache>(
      *register_file_, *memory_, &trace_writer_, 1, 1, *this);
  if (!render_target_cache_->Initialize()) {
    XELOGE("Failed to initialize Metal render target cache");
    return false;
  }

  // Initialize shader translation pipeline
  if (!InitializeShaderTranslation()) {
    XELOGE("Failed to initialize shader translation");
    return false;
  }
  if (UseSpirvCrossPath()) {
    InitializeMslAsyncCompilation();
  }
#if METAL_SHADER_CONVERTER_AVAILABLE
  if (mesh_shader_supported_) {
    uint64_t tess_tables_size = IRRuntimeTessellatorTablesSize();
    tessellator_tables_buffer_ =
        device_->newBuffer(tess_tables_size, MTL::ResourceStorageModeShared);
    if (!tessellator_tables_buffer_) {
      XELOGE("Failed to allocate tessellator tables buffer ({} bytes)",
             tess_tables_size);
      return false;
    }
    tessellator_tables_buffer_->setLabel(
        NS::String::string("XeniaTessellatorTables", NS::UTF8StringEncoding));
    IRRuntimeLoadTessellatorTables(tessellator_tables_buffer_);
  }
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

  // Create render target texture for offscreen rendering
  MTL::TextureDescriptor* color_desc = MTL::TextureDescriptor::alloc()->init();
  color_desc->setTextureType(MTL::TextureType2D);
  color_desc->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
  color_desc->setWidth(render_target_width_);
  color_desc->setHeight(render_target_height_);
  color_desc->setStorageMode(MTL::StorageModePrivate);
  color_desc->setUsage(MTL::TextureUsageRenderTarget |
                       MTL::TextureUsageShaderRead);

  render_target_texture_ = device_->newTexture(color_desc);
  color_desc->release();

  if (!render_target_texture_) {
    XELOGE("Failed to create render target texture");
    return false;
  }
  render_target_texture_->setLabel(
      NS::String::string("XeniaRenderTarget", NS::UTF8StringEncoding));

  // Create depth/stencil texture
  MTL::TextureDescriptor* depth_desc = MTL::TextureDescriptor::alloc()->init();
  depth_desc->setTextureType(MTL::TextureType2D);
  depth_desc->setPixelFormat(MTL::PixelFormatDepth32Float_Stencil8);
  depth_desc->setWidth(render_target_width_);
  depth_desc->setHeight(render_target_height_);
#if XE_PLATFORM_IOS
  // This fallback depth/stencil target is transient (clear/dontcare only) and
  // never sampled, so memoryless is the most efficient iOS storage mode.
  depth_desc->setStorageMode(MTL::StorageModeMemoryless);
#else
  depth_desc->setStorageMode(MTL::StorageModePrivate);
#endif
  depth_desc->setUsage(MTL::TextureUsageRenderTarget);

  depth_stencil_texture_ = device_->newTexture(depth_desc);
  depth_desc->release();

  if (!depth_stencil_texture_) {
    XELOGE("Failed to create depth/stencil texture");
    return false;
  }
  depth_stencil_texture_->setLabel(
      NS::String::string("XeniaDepthStencil", NS::UTF8StringEncoding));

  // Create render pass descriptor
  render_pass_descriptor_ = MTL::RenderPassDescriptor::alloc()->init();

  auto color_attachment =
      render_pass_descriptor_->colorAttachments()->object(0);
  color_attachment->setTexture(render_target_texture_);
  color_attachment->setLoadAction(MTL::LoadActionClear);
  color_attachment->setStoreAction(MTL::StoreActionStore);
  color_attachment->setClearColor(MTL::ClearColor(0.0, 0.0, 0.0, 1.0));

  auto depth_attachment = render_pass_descriptor_->depthAttachment();
  depth_attachment->setTexture(depth_stencil_texture_);
  depth_attachment->setLoadAction(MTL::LoadActionClear);
  depth_attachment->setStoreAction(MTL::StoreActionDontCare);
  depth_attachment->setClearDepth(1.0);

  auto stencil_attachment = render_pass_descriptor_->stencilAttachment();
  stencil_attachment->setTexture(depth_stencil_texture_);
  stencil_attachment->setLoadAction(MTL::LoadActionClear);
  stencil_attachment->setStoreAction(MTL::StoreActionDontCare);
  stencil_attachment->setClearStencil(0);

  // Create a null buffer for unused descriptor entries
  // This prevents shader validation errors when accessing unpopulated
  // descriptors
  null_buffer_ =
      device_->newBuffer(kNullBufferSize, MTL::ResourceStorageModeShared);
  if (!null_buffer_) {
    XELOGE("Failed to create null buffer");
    return false;
  }
  null_buffer_->setLabel(
      NS::String::string("NullBuffer", NS::UTF8StringEncoding));
  std::memset(null_buffer_->contents(), 0, kNullBufferSize);

  // The index records of every spirv2air stage: {binding, binding}
  // (Spirv2airLayout).
  spirv2air_texture_indices_ =
      device_->newBuffer(Spirv2airLayout::kMaxBindings * 2 * sizeof(uint32_t),
                         MTL::ResourceStorageModeShared);
  if (!spirv2air_texture_indices_) {
    XELOGE("Failed to create the spirv2air texture index records");
    return false;
  }
  spirv2air_texture_indices_->setLabel(
      NS::String::string("Spirv2airTextureIndices", NS::UTF8StringEncoding));
  {
    uint32_t* records =
        static_cast<uint32_t*>(spirv2air_texture_indices_->contents());
    for (uint32_t i = 0; i < Spirv2airLayout::kMaxBindings; ++i) {
      records[i * 2] = i;
      records[i * 2 + 1] = i;
    }
  }

  // Create a 1x1x1 placeholder 2D array texture for unbound texture slots
  // Xbox 360 textures are typically 2D arrays (for texture atlases, cubemaps)
  // Using 2DArray prevents "Invalid texture type" validation errors
  MTL::TextureDescriptor* null_tex_desc =
      MTL::TextureDescriptor::alloc()->init();
  null_tex_desc->setTextureType(MTL::TextureType2DArray);
  null_tex_desc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
  null_tex_desc->setWidth(1);
  null_tex_desc->setHeight(1);
  null_tex_desc->setArrayLength(1);  // Single slice in the array
  null_tex_desc->setStorageMode(MTL::StorageModeShared);
  null_tex_desc->setUsage(MTL::TextureUsageShaderRead);

  null_texture_ = device_->newTexture(null_tex_desc);
  null_tex_desc->release();

  if (!null_texture_) {
    XELOGE("Failed to create null texture");
    return false;
  }
  null_texture_->setLabel(
      NS::String::string("NullTexture2DArray", NS::UTF8StringEncoding));

  // Fill the 1x1x1 texture with opaque white (helps debug if sampled)
  uint32_t white_pixel = 0xFFFFFFFF;
  MTL::Region region =
      MTL::Region(0, 0, 0, 1, 1, 1);  // x,y,z origin, w,h,d size
  null_texture_->replaceRegion(region, 0, 0, &white_pixel, 4, 0);  // slice 0

  // Create a default sampler for unbound sampler slots
  // Must set supportsArgumentBuffers=YES for use in argument buffers
  MTL::SamplerDescriptor* null_smp_desc =
      MTL::SamplerDescriptor::alloc()->init();
  null_smp_desc->setMinFilter(MTL::SamplerMinMagFilterLinear);
  null_smp_desc->setMagFilter(MTL::SamplerMinMagFilterLinear);
  null_smp_desc->setMipFilter(MTL::SamplerMipFilterLinear);
  null_smp_desc->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
  null_smp_desc->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
  null_smp_desc->setRAddressMode(MTL::SamplerAddressModeClampToEdge);
  null_smp_desc->setSupportArgumentBuffers(true);

  null_sampler_ = device_->newSamplerState(null_smp_desc);
  null_smp_desc->release();

  if (!null_sampler_) {
    XELOGE("Failed to create null sampler");
    return false;
  }

#if METAL_SHADER_CONVERTER_AVAILABLE
  for (uint32_t i = 0; i < kDrawRingPoolPrewarmCount; ++i) {
    auto pooled_ring = CreateDrawRingBuffers();
    if (!pooled_ring) {
      break;
    }
    std::lock_guard<std::mutex> lock(draw_ring_mutex_);
    const uint64_t ring_bytes = pooled_ring->total_bytes
                                    ? pooled_ring->total_bytes
                                    : GetDrawRingAllocationSizeBytes();
    if (draw_ring_pool_total_bytes_ + ring_bytes > draw_ring_pool_max_bytes_) {
      break;
    }
    draw_ring_pool_total_bytes_ += ring_bytes;
    draw_ring_pool_.push_back(std::move(pooled_ring));
  }

  auto ring = AcquireDrawRingBuffers();
  if (!ring) {
    XELOGE("MetalCommandProcessor: failed to acquire initial draw ring");
    return false;
  }
  SetActiveDrawRing(ring);
#else
  // SPIRV-Cross path: use command-buffer-scoped uniforms buffers so CPU writes
  // to the next submission can't race with in-flight GPU reads.
  if (UseSpirvCrossPath()) {
    if (!EnsureSpirvUniformBuffer()) {
      return false;
    }
  }
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

  return true;
}

bool MetalCommandProcessor::InitializeShaderTranslation() {
  // Initialize DXBC shader translator (use Apple vendor ID for Metal)
  // Must query render_target_cache_ for actual runtime parameters.
  // Metal doesn't use ROV (rasterizer ordered views) path.
  bool edram_rov_used = false;

  // gamma_render_target_as_unorm8: When true, shaders include code to convert
  // linear -> gamma for 8-bit gamma render targets. When false, we use 16-bit
  // UNORM format where hardware handles gamma implicitly.
  bool gamma_render_target_as_unorm8 = !(
      edram_rov_used || render_target_cache_->gamma_render_target_as_unorm16());

  XELOGI(
      "DxbcShaderTranslator init: gamma_as_unorm8={}, msaa_2x={}, scale={}x{}",
      gamma_render_target_as_unorm8, render_target_cache_->msaa_2x_supported(),
      render_target_cache_->draw_resolution_scale_x(),
      render_target_cache_->draw_resolution_scale_y());

#if !METAL_SHADER_CONVERTER_AVAILABLE
  if (!cvars::metal_use_spirvcross) {
    XELOGW(
        "Metal MSC path unavailable in this build; forcing SPIRV-Cross path");
  }
#endif  // !METAL_SHADER_CONVERTER_AVAILABLE

#if METAL_SHADER_CONVERTER_AVAILABLE
  shader_translator_ = std::make_unique<DxbcShaderTranslator>(
      ui::GraphicsProvider::GpuVendorID::kApple,
      false,  // bindless_resources_used - not using bindless for now
      edram_rov_used, gamma_render_target_as_unorm8,
      render_target_cache_->msaa_2x_supported(),
      render_target_cache_->draw_resolution_scale_x(),
      render_target_cache_->draw_resolution_scale_y(),
      false);  // force_emit_source_map

  // Initialize DXBC to DXIL converter
  dxbc_to_dxil_converter_ = std::make_unique<DxbcToDxilConverter>();
  if (!dxbc_to_dxil_converter_->Initialize()) {
    XELOGE("Failed to initialize DXBC to DXIL converter");
    return false;
  }

  // Initialize Metal Shader Converter
  metal_shader_converter_ = std::make_unique<MetalShaderConverter>();
  if (!metal_shader_converter_->Initialize()) {
    XELOGE("Failed to initialize Metal Shader Converter");
    return false;
  }
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

  // Initialize SPIRV-Cross (MSL) path when enabled.
  if (UseSpirvCrossPath()) {
    // Build SpirvShaderTranslator::Features for Metal.
    // Enable all features as a baseline, then disable what Metal doesn't need.
    SpirvShaderTranslator::Features spirv_features(true);
    // Not using fragment shader interlock — we use host render targets.
    spirv_features.fragment_shader_sample_interlock = false;
    // Barycentric interpolation not needed for current Metal path.
    spirv_features.fragment_shader_barycentric = false;
    // Metal fast-math doesn't guarantee IEEE NaN/Inf preservation.
    spirv_features.signed_zero_inf_nan_preserve_float32 = false;
    // Metal fast-math flushes denorms.
    spirv_features.denorm_flush_to_zero_float32 = true;
    // RTE rounding not guaranteed by Metal.
    spirv_features.rounding_mode_rte_float32 = false;

    spirv_shader_translator_ = std::make_unique<SpirvShaderTranslator>(
        spirv_features,
        render_target_cache_->msaa_2x_supported(),  // native_2x_msaa_with_att
        false,                                      // native_2x_msaa_no_att
        false,  // edram_fragment_shader_interlock (host RT path)
        render_target_cache_->draw_resolution_scale_x(),
        render_target_cache_->draw_resolution_scale_y(),
        false);  // spirv_optimize

    XELOGI(
        "SpirvShaderTranslator init (SPIRV-Cross MSL path): msaa_2x={}, "
        "scale={}x{}",
        render_target_cache_->msaa_2x_supported(),
        render_target_cache_->draw_resolution_scale_x(),
        render_target_cache_->draw_resolution_scale_y());

    if (!InitializeMslTessellation()) {
      XELOGW(
          "SPIRV-Cross: Tessellation factor pipelines failed to init; "
          "tessellated draws will be skipped");
    }
  }

#if METAL_SHADER_CONVERTER_AVAILABLE
  // Configure MSC minimum targets to avoid materialization failures on older
  // GPUs/OS versions.
  if (device_) {
    IRGPUFamily min_family = IRGPUFamilyMetal3;
    if (device_->supportsFamily(MTL::GPUFamilyApple10)) {
      min_family = IRGPUFamilyApple10;
    } else if (device_->supportsFamily(MTL::GPUFamilyApple9)) {
      min_family = IRGPUFamilyApple9;
    } else if (device_->supportsFamily(MTL::GPUFamilyApple8)) {
      min_family = IRGPUFamilyApple8;
    } else if (device_->supportsFamily(MTL::GPUFamilyApple7)) {
      min_family = IRGPUFamilyApple7;
    } else if (device_->supportsFamily(MTL::GPUFamilyApple6)) {
      min_family = IRGPUFamilyApple6;
    } else if (device_->supportsFamily(MTL::GPUFamilyMac2) ||
               device_->supportsFamily(MTL::GPUFamilyMetal4) ||
               device_->supportsFamily(MTL::GPUFamilyMetal3)) {
      min_family = IRGPUFamilyMetal3;
    }

    NS::OperatingSystemVersion os_version =
        NS::ProcessInfo::processInfo()->operatingSystemVersion();
    std::ostringstream version_stream;
    version_stream << os_version.majorVersion << "." << os_version.minorVersion
                   << "." << os_version.patchVersion;
    metal_shader_converter_->SetMinimumTarget(
        min_family, IROperatingSystem_macOS, version_stream.str());
  }
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

  return true;
}

void MetalCommandProcessor::PrepareForWait() {
  // Runs on every ring-empty stall, so it must not block on the GPU (as in
  // XeniOS and xenia-edge). It used to wait for all GPU work twice here,
  // which serialized the CPU and the GPU dozens of times per frame and, in
  // the background, where iOS refuses GPU work, never returned.
  EndCommandBuffer();

  CommandProcessor::PrepareForWait();
}

void MetalCommandProcessor::SubmitBeforeShortMemoryPoll() {
  if (!pending_shader_done_fences_.empty()) {
    EndCommandBuffer();
  }
}

bool MetalCommandProcessor::TryWriteShaderDoneFence(uint32_t address,
                                                    uint32_t value) {
  if (!cvars::metal_gpu_ordered_fences || !shared_memory_ ||
      !shared_memory_->is_zero_copy() || trace_writer_.is_open() ||
      (address & 3) || address > SharedMemory::kBufferSize - sizeof(value)) {
    return false;
  }
  if (current_command_buffer_) {
    // Stored when the command buffer completes. If the guest waits for it, the
    // ring runs empty and PrepareForWait commits it, or a short memory poll
    // does (SubmitBeforeShortMemoryPoll).
    pending_shader_done_fences_.emplace_back(address, value);
    return true;
  }
  // Nothing open: everything before the fence is in committed command
  // buffers. An empty one completes after them, the queue running in order.
  MTL::CommandBuffer* fence_command_buffer =
      command_queue_ ? command_queue_->commandBuffer() : nullptr;
  if (!fence_command_buffer) {
    return false;
  }
  pending_shader_done_fences_.emplace_back(address, value);
  AttachShaderDoneFences(fence_command_buffer);
  fence_command_buffer->commit();
  return true;
}

void MetalCommandProcessor::AttachShaderDoneFences(
    MTL::CommandBuffer* command_buffer) {
  if (pending_shader_done_fences_.empty() || !command_buffer) {
    return;
  }
  pending_completion_handlers_.fetch_add(1, std::memory_order_relaxed);
  Memory* memory = memory_;
  std::vector<std::pair<uint32_t, uint32_t>> fences =
      std::move(pending_shader_done_fences_);
  pending_shader_done_fences_.clear();
  command_buffer->addCompletedHandler(
      [this, memory, fences](MTL::CommandBuffer*) {
        for (const auto& fence : fences) {
          xe::store(memory->TranslatePhysical(fence.first), fence.second);
        }
        std::atomic_thread_fence(std::memory_order_seq_cst);
        pending_completion_handlers_.fetch_sub(1, std::memory_order_relaxed);
      });
}

void MetalCommandProcessor::WaitForPendingCompletionHandlers() {
  constexpr auto kMaxWait = std::chrono::seconds(5);
  const auto wait_start = std::chrono::steady_clock::now();
  while (pending_completion_handlers_.load(std::memory_order_acquire) != 0) {
    if (std::chrono::steady_clock::now() - wait_start >= kMaxWait) {
      XELOGW(
          "MetalCommandProcessor: timed out waiting for {} completion "
          "handler(s) during shutdown",
          pending_completion_handlers_.load(std::memory_order_relaxed));
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void MetalCommandProcessor::ShutdownContext() {
  // End any active render encoder before shutdown
  if (current_render_encoder_) {
    current_render_encoder_->endEncoding();
    metal_gpu_pass_record_ = nullptr;
    // Don't release yet - wait until command buffer completes
  }

  // Submit and wait for any pending command buffer
  if (current_command_buffer_) {
    uint64_t wait_value = 0;
    if (wait_shared_event_) {
      wait_value = ++wait_shared_event_value_;
      current_command_buffer_->encodeSignalEvent(wait_shared_event_,
                                                 wait_value);
    }
#if METAL_SHADER_CONVERTER_AVAILABLE
    ScheduleDrawRingRelease(current_command_buffer_);
#else
    if (UseSpirvCrossPath()) {
      ScheduleSpirvUniformBufferRelease(current_command_buffer_);
    }
#endif
    ScheduleSpirvArgumentBufferRelease(current_command_buffer_);
    AttachShaderDoneFences(current_command_buffer_);
    current_command_buffer_->commit();
    if (wait_shared_event_) {
      // Bounded so a GPU that has stopped (in the background) can't hold up
      // quitting. The command buffer keeps what it uses alive.
      if (!wait_shared_event_->waitUntilSignaledValue(wait_value, 2000)) {
        XELOGW(
            "Metal: the last command buffer didn't finish in 2 s at shutdown");
      }
    } else {
      WaitUntilCompletedCounted(current_command_buffer_);
    }
    current_command_buffer_->release();
    current_command_buffer_ = nullptr;
#if METAL_SHADER_CONVERTER_AVAILABLE
    SetActiveDrawRing(nullptr);
#endif
    current_draw_index_ = 0;
    copy_resolve_writes_pending_ = false;
  }

  // Even if we have no active command buffer at this point, there may be
  // previously committed command buffers still in flight. Submit and wait for
  // a dummy command buffer to ensure all GPU work on this queue has completed
  // before tearing down resources on thread exit.
  if (command_queue_) {
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    MTL::CommandBuffer* sync_cmd = command_queue_->commandBuffer();
    if (sync_cmd) {
      uint64_t wait_value = 0;
      if (wait_shared_event_) {
        wait_value = ++wait_shared_event_value_;
        sync_cmd->encodeSignalEvent(wait_shared_event_, wait_value);
      }
      sync_cmd->commit();
      if (wait_shared_event_) {
        // Bounded for the same reason: iOS refuses this one too in the
        // background, and then never signals the event.
        if (!wait_shared_event_->waitUntilSignaledValue(wait_value, 2000)) {
          XELOGW("Metal: the GPU queue didn't drain in 2 s at shutdown");
        }
      } else {
        WaitUntilCompletedCounted(sync_cmd);
      }
    }
    pool->release();
  }

  WaitForPendingCompletionHandlers();

  // Now safe to release encoder and command buffer
  if (current_render_encoder_) {
    current_render_encoder_->release();
    current_render_encoder_ = nullptr;
    metal_gpu_pass_record_ = nullptr;
  }
  if (current_command_buffer_) {
    current_command_buffer_->release();
    current_command_buffer_ = nullptr;
  }
  DrainCommandBufferAutoreleasePool();
  zpd_query_active_ = false;
  zpd_segment_open_ = false;
  render_encoder_has_zpd_visibility_ = false;
  zpd_queries_in_flight_.clear();
  zpd_visibility_pool_.Shutdown();

#if METAL_SHADER_CONVERTER_AVAILABLE
  {
    std::lock_guard<std::mutex> lock(draw_ring_mutex_);
    FlushPendingDrawRingReleasesLocked();
    active_draw_ring_.reset();
    draw_ring_pool_.clear();
    command_buffer_draw_rings_.clear();
    draw_ring_pool_total_bytes_ = 0;
    draw_ring_pool_in_use_count_ = 0;
    draw_ring_pool_wait_count_ = 0;
    draw_ring_pool_timeout_count_ = 0;
  }
  draw_ring_pool_cv_.notify_all();
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

  {
    std::lock_guard<std::mutex> lock(spirv_argbuf_mutex_);
    command_buffer_spirv_argbuf_pages_.clear();
    pending_spirv_argbuf_releases_.clear();
    spirv_argbuf_pool_.clear();
  }

  if (texture_cache_) {
    texture_cache_->Shutdown();
    texture_cache_.reset();
  }

  if (primitive_processor_) {
    primitive_processor_->Shutdown();
    primitive_processor_.reset();
  }
#if METAL_SHADER_CONVERTER_AVAILABLE
  if (tessellator_tables_buffer_) {
    tessellator_tables_buffer_->release();
    tessellator_tables_buffer_ = nullptr;
  }
  if (depth_only_pixel_library_) {
    depth_only_pixel_library_->release();
    depth_only_pixel_library_ = nullptr;
  }
  depth_only_pixel_function_name_.clear();
#endif  // METAL_SHADER_CONVERTER_AVAILABLE
  frame_open_ = false;

  ShutdownMslAsyncCompilation();

#if METAL_SHADER_CONVERTER_AVAILABLE
  shader_cache_.clear();
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

  // SPIRV-Cross resources.
  ShutdownMslTessellation();
  for (auto& [key, pso] : msl_pipeline_cache_) {
    if (pso) {
      pso->release();
    }
  }
  msl_pipeline_cache_.clear();
  msl_shader_cache_.clear();
  spirv_shader_translator_.reset();

#if !METAL_SHADER_CONVERTER_AVAILABLE
  uniforms_buffer_ = nullptr;
  command_buffer_spirv_uniform_buffers_.clear();
  {
    std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
    spirv_uniforms_available_.clear();
    for (MTL::Buffer* pool_uniforms : spirv_uniforms_pool_) {
      if (pool_uniforms) {
        pool_uniforms->release();
      }
    }
    spirv_uniforms_pool_.clear();
    spirv_uniforms_pool_initialized_ = false;
  }
  if (spirv_uniforms_available_semaphore_) {
#if !OS_OBJECT_USE_OBJC
    dispatch_release(spirv_uniforms_available_semaphore_);
#endif
    spirv_uniforms_available_semaphore_ = nullptr;
  }
#endif  // !METAL_SHADER_CONVERTER_AVAILABLE

  shared_memory_.reset();
#if METAL_SHADER_CONVERTER_AVAILABLE
  shader_translator_.reset();
  dxbc_to_dxil_converter_.reset();
  metal_shader_converter_.reset();
#endif  // METAL_SHADER_CONVERTER_AVAILABLE
  if (wait_shared_event_) {
    wait_shared_event_->release();
    wait_shared_event_ = nullptr;
  }

  ClearMslShaderSourceCacheDirectory();

#if METAL_SHADER_CONVERTER_AVAILABLE
  ShutdownShaderStorage();
#endif

  CommandProcessor::ShutdownContext();
}

#if METAL_SHADER_CONVERTER_AVAILABLE
void MetalCommandProcessor::InitializeShaderStorage(
    const std::filesystem::path& cache_root, uint32_t title_id, bool blocking,
    std::function<void()> completion_callback) {
  CommandProcessor::InitializeShaderStorage(cache_root, title_id, blocking,
                                            nullptr);
  InitializeShaderStorageInternal(cache_root, title_id, blocking);
  if (completion_callback) {
    completion_callback();
  }
}
#else
void MetalCommandProcessor::InitializeShaderStorage(
    const std::filesystem::path& cache_root, uint32_t title_id, bool blocking,
    std::function<void()> completion_callback) {
  CommandProcessor::InitializeShaderStorage(cache_root, title_id, blocking,
                                            nullptr);

  if (!device_) {
    XELOGW("Metal shader storage init skipped (no device)");
    if (completion_callback) {
      completion_callback();
    }
    return;
  }

  std::string device_tag = "unknown";
  if (device_->name()) {
    device_tag = device_->name()->utf8String();
  }
  for (char& ch : device_tag) {
    if (!std::isalnum(static_cast<unsigned char>(ch))) {
      ch = '_';
    }
  }

  std::filesystem::path shader_storage_title_root =
      cache_root / "shaders" / "metal" / "local" / device_tag /
      fmt::format("{:08X}", title_id);
  std::error_code ec;
  std::filesystem::create_directories(shader_storage_title_root, ec);
  if (ec) {
    XELOGW("Metal shader storage: Failed to create {}: {}",
           shader_storage_title_root.string(), ec.message());
  } else if (::cvars::metal_shader_disk_cache && g_metal_shader_cache) {
    g_metal_shader_cache->Initialize(shader_storage_title_root / "metallib");
    SetMslShaderSourceCacheDirectory(shader_storage_title_root / "msl_source");
  } else {
    SetMslShaderSourceCacheDirectory(shader_storage_title_root / "msl_source");
  }
  if (cvars::metal_spirv2air_self_test) {
    // Before the title runs, so no other compile shares Metal's compiler
    // service with the pipelines it builds.
    constexpr bool kIsIos =
#if XE_PLATFORM_IOS
        true;
#else
        false;
#endif
    RunSpirv2airSelfTest(device_, kIsIos,
                         shader_storage_title_root / "spirv2air_probe");
  }

  if (completion_callback) {
    completion_callback();
  }
}
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

#if METAL_SHADER_CONVERTER_AVAILABLE
bool MetalCommandProcessor::InitializeShaderStorageInternal(
    const std::filesystem::path& cache_root, uint32_t title_id, bool blocking) {
  ShutdownShaderStorage();

  if (!device_) {
    XELOGW("Metal shader storage init skipped (no device)");
    return false;
  }

  shader_storage_root_ = cache_root / "shaders" / "metal";
  shader_storage_local_root_ =
      shader_storage_root_ / "local" / GetShaderStorageDeviceTag();
  shader_storage_title_root_ =
      shader_storage_local_root_ / fmt::format("{:08X}", title_id);

  std::error_code ec;
  std::filesystem::create_directories(shader_storage_title_root_, ec);
  if (ec) {
    XELOGW("Metal shader storage: Failed to create {}: {}",
           shader_storage_title_root_.string(), ec.message());
    return false;
  }
  SetMslShaderSourceCacheDirectory(shader_storage_title_root_ / "msl_source");

  metallib_cache_dir_ = shader_storage_title_root_ / "metallib";
  if (::cvars::metal_shader_disk_cache && g_metal_shader_cache) {
    g_metal_shader_cache->Initialize(metallib_cache_dir_);
  }

  const char* path_suffix = "rtv";

  pipeline_disk_cache_path_ =
      shader_storage_title_root_ /
      fmt::format("{:08X}.{}.metal.pipelines", title_id, path_suffix);
  pipeline_binary_archive_path_ =
      shader_storage_title_root_ /
      fmt::format("{:08X}.{}.metal.binarchive", title_id, path_suffix);

  if (::cvars::metal_pipeline_disk_cache) {
    LoadPipelineDiskCache(pipeline_disk_cache_path_,
                          &pipeline_disk_cache_entries_);
  }

  if (::cvars::metal_pipeline_binary_archive) {
    InitializePipelineBinaryArchive(pipeline_binary_archive_path_);
  }

  bool prewarm_binary_archive = false;
  {
    std::lock_guard<std::mutex> lock(pipeline_binary_archive_mutex_);
    prewarm_binary_archive = pipeline_binary_archive_ != nullptr;
  }
  if (blocking && prewarm_binary_archive &&
      !pipeline_disk_cache_entries_.empty()) {
    PrewarmPipelineBinaryArchive(pipeline_disk_cache_entries_);
  }

  return true;
}

void MetalCommandProcessor::ShutdownShaderStorage() {
  {
    std::lock_guard<std::mutex> lock(pipeline_binary_archive_mutex_);
    if (pipeline_binary_archive_) {
      if (pipeline_binary_archive_dirty_) {
        NS::String* path_string =
            NS::String::string(pipeline_binary_archive_path_.string().c_str(),
                               NS::UTF8StringEncoding);
        NS::URL* url = NS::URL::fileURLWithPath(path_string);
        NS::Error* error = nullptr;
        if (!pipeline_binary_archive_->serializeToURL(url, &error)) {
          if (error) {
            XELOGW("Metal binary archive serialize failed: {}",
                   error->localizedDescription()->utf8String());
          }
        }
        pipeline_binary_archive_dirty_ = false;
      }
      pipeline_binary_archive_->release();
      pipeline_binary_archive_ = nullptr;
    }
  }
  if (pipeline_disk_cache_file_) {
    std::fclose(pipeline_disk_cache_file_);
    pipeline_disk_cache_file_ = nullptr;
  }
  pipeline_disk_cache_keys_.clear();
  pipeline_disk_cache_entries_.clear();

  if (g_metal_shader_cache) {
    g_metal_shader_cache->Shutdown();
  }
}

std::string MetalCommandProcessor::GetShaderStorageDeviceTag() const {
  std::string tag = "unknown";
  if (device_ && device_->name()) {
    tag = device_->name()->utf8String();
  }

  for (char& ch : tag) {
    if (!std::isalnum(static_cast<unsigned char>(ch))) {
      ch = '_';
    }
  }
  return tag;
}

bool MetalCommandProcessor::LoadPipelineDiskCache(
    const std::filesystem::path& path,
    std::vector<PipelineDiskCacheEntry>* entries) {
  if (!entries) {
    return false;
  }
  entries->clear();
  pipeline_disk_cache_keys_.clear();

  pipeline_disk_cache_file_ = xe::filesystem::OpenFile(path, "a+b");
  if (!pipeline_disk_cache_file_) {
    XELOGW("Metal pipeline disk cache: Failed to open {}", path.string());
    return false;
  }

  PipelineDiskCacheHeader header = {};
  if (std::fread(&header, sizeof(header), 1, pipeline_disk_cache_file_) != 1 ||
      header.magic != kPipelineDiskCacheMagic ||
      header.version != kPipelineDiskCacheVersion) {
    header.magic = kPipelineDiskCacheMagic;
    header.version = kPipelineDiskCacheVersion;
    header.reserved[0] = 0;
    header.reserved[1] = 0;
    xe::filesystem::Seek(pipeline_disk_cache_file_, 0, SEEK_SET);
    std::fwrite(&header, sizeof(header), 1, pipeline_disk_cache_file_);
    std::fflush(pipeline_disk_cache_file_);
    xe::filesystem::Seek(pipeline_disk_cache_file_, 0, SEEK_END);
    return true;
  }

  while (true) {
    PipelineDiskCacheEntryHeader entry_header = {};
    if (std::fread(&entry_header, sizeof(entry_header), 1,
                   pipeline_disk_cache_file_) != 1) {
      break;
    }

    if (entry_header.entry_size < sizeof(PipelineDiskCacheEntryBase) ||
        entry_header.entry_size > kPipelineDiskCacheMaxEntrySize) {
      break;
    }

    PipelineDiskCacheEntryBase base = {};
    if (std::fread(&base, sizeof(base), 1, pipeline_disk_cache_file_) != 1) {
      break;
    }

    size_t expected_size = sizeof(PipelineDiskCacheEntryBase) +
                           size_t(base.vertex_attribute_count) *
                               sizeof(PipelineDiskCacheVertexAttribute) +
                           size_t(base.vertex_layout_count) *
                               sizeof(PipelineDiskCacheVertexLayout);
    if (entry_header.entry_size != expected_size) {
      xe::filesystem::Seek(
          pipeline_disk_cache_file_,
          entry_header.entry_size - sizeof(PipelineDiskCacheEntryBase),
          SEEK_CUR);
      continue;
    }

    PipelineDiskCacheEntry entry = {};
    entry.pipeline_key = base.pipeline_key;
    entry.vertex_shader_cache_key = base.vertex_shader_cache_key;
    entry.pixel_shader_cache_key = base.pixel_shader_cache_key;
    entry.sample_count = base.sample_count;
    entry.depth_format = base.depth_format;
    entry.stencil_format = base.stencil_format;
    std::memcpy(entry.color_formats, base.color_formats,
                sizeof(base.color_formats));
    entry.normalized_color_mask = base.normalized_color_mask;
    entry.alpha_to_mask_enable = base.alpha_to_mask_enable;
    std::memcpy(entry.blendcontrol, base.blendcontrol,
                sizeof(base.blendcontrol));

    entry.vertex_attributes.resize(base.vertex_attribute_count);
    if (base.vertex_attribute_count) {
      if (std::fread(entry.vertex_attributes.data(),
                     sizeof(PipelineDiskCacheVertexAttribute),
                     base.vertex_attribute_count, pipeline_disk_cache_file_) !=
          base.vertex_attribute_count) {
        break;
      }
    }
    entry.vertex_layouts.resize(base.vertex_layout_count);
    if (base.vertex_layout_count) {
      if (std::fread(entry.vertex_layouts.data(),
                     sizeof(PipelineDiskCacheVertexLayout),
                     base.vertex_layout_count,
                     pipeline_disk_cache_file_) != base.vertex_layout_count) {
        break;
      }
    }

    entries->push_back(std::move(entry));
    pipeline_disk_cache_keys_.insert(base.pipeline_key);
  }

  xe::filesystem::Seek(pipeline_disk_cache_file_, 0, SEEK_END);
  return true;
}

bool MetalCommandProcessor::AppendPipelineDiskCacheEntry(
    const PipelineDiskCacheEntry& entry) {
  if (!pipeline_disk_cache_file_) {
    return false;
  }
  if (!pipeline_disk_cache_keys_.insert(entry.pipeline_key).second) {
    return false;
  }

  PipelineDiskCacheEntryBase base = {};
  base.pipeline_key = entry.pipeline_key;
  base.vertex_shader_cache_key = entry.vertex_shader_cache_key;
  base.pixel_shader_cache_key = entry.pixel_shader_cache_key;
  base.sample_count = entry.sample_count;
  base.depth_format = entry.depth_format;
  base.stencil_format = entry.stencil_format;
  std::memcpy(base.color_formats, entry.color_formats,
              sizeof(base.color_formats));
  base.normalized_color_mask = entry.normalized_color_mask;
  base.alpha_to_mask_enable = entry.alpha_to_mask_enable;
  std::memcpy(base.blendcontrol, entry.blendcontrol, sizeof(base.blendcontrol));
  base.vertex_attribute_count =
      static_cast<uint32_t>(entry.vertex_attributes.size());
  base.vertex_layout_count = static_cast<uint32_t>(entry.vertex_layouts.size());

  size_t entry_size =
      sizeof(PipelineDiskCacheEntryBase) +
      entry.vertex_attributes.size() *
          sizeof(PipelineDiskCacheVertexAttribute) +
      entry.vertex_layouts.size() * sizeof(PipelineDiskCacheVertexLayout);
  if (entry_size > kPipelineDiskCacheMaxEntrySize) {
    return false;
  }

  PipelineDiskCacheEntryHeader entry_header = {};
  entry_header.entry_size = static_cast<uint32_t>(entry_size);

  std::fwrite(&entry_header, sizeof(entry_header), 1,
              pipeline_disk_cache_file_);
  std::fwrite(&base, sizeof(base), 1, pipeline_disk_cache_file_);
  if (!entry.vertex_attributes.empty()) {
    std::fwrite(entry.vertex_attributes.data(),
                sizeof(PipelineDiskCacheVertexAttribute),
                entry.vertex_attributes.size(), pipeline_disk_cache_file_);
  }
  if (!entry.vertex_layouts.empty()) {
    std::fwrite(entry.vertex_layouts.data(),
                sizeof(PipelineDiskCacheVertexLayout),
                entry.vertex_layouts.size(), pipeline_disk_cache_file_);
  }
  std::fflush(pipeline_disk_cache_file_);
  pipeline_disk_cache_entries_.push_back(entry);
  return true;
}

bool MetalCommandProcessor::InitializePipelineBinaryArchive(
    const std::filesystem::path& archive_path) {
  if (!device_) {
    return false;
  }
  std::lock_guard<std::mutex> lock(pipeline_binary_archive_mutex_);
  if (pipeline_binary_archive_) {
    pipeline_binary_archive_->release();
    pipeline_binary_archive_ = nullptr;
  }

  MTL::BinaryArchiveDescriptor* desc =
      MTL::BinaryArchiveDescriptor::alloc()->init();
  NS::String* path_string =
      NS::String::string(archive_path.string().c_str(), NS::UTF8StringEncoding);
  NS::URL* url = NS::URL::fileURLWithPath(path_string);
  desc->setUrl(url);

  NS::Error* error = nullptr;
  pipeline_binary_archive_ = device_->newBinaryArchive(desc, &error);
  desc->release();
  if (!pipeline_binary_archive_) {
    if (error) {
      XELOGW("Metal binary archive init failed: {}",
             error->localizedDescription()->utf8String());
    }
    return false;
  }
  pipeline_binary_archive_path_ = archive_path;
  pipeline_binary_archive_dirty_ = false;
  return true;
}

void MetalCommandProcessor::SerializePipelineBinaryArchive() {
  std::lock_guard<std::mutex> lock(pipeline_binary_archive_mutex_);
  if (!pipeline_binary_archive_ || !pipeline_binary_archive_dirty_) {
    return;
  }
  NS::String* path_string = NS::String::string(
      pipeline_binary_archive_path_.string().c_str(), NS::UTF8StringEncoding);
  NS::URL* url = NS::URL::fileURLWithPath(path_string);
  NS::Error* error = nullptr;
  if (!pipeline_binary_archive_->serializeToURL(url, &error)) {
    if (error) {
      XELOGW("Metal binary archive serialize failed: {}",
             error->localizedDescription()->utf8String());
    }
  }
  pipeline_binary_archive_dirty_ = false;
}

void MetalCommandProcessor::PrewarmPipelineBinaryArchive(
    const std::vector<PipelineDiskCacheEntry>& entries) {
  if (entries.empty()) {
    return;
  }
  if (!g_metal_shader_cache || !g_metal_shader_cache->IsInitialized()) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(pipeline_binary_archive_mutex_);
    if (!pipeline_binary_archive_) {
      return;
    }
  }

  size_t prewarmed = 0;
  for (const auto& entry : entries) {
    MetalShaderCache::CachedMetallib vs_cached;
    if (!g_metal_shader_cache->Load(entry.vertex_shader_cache_key,
                                    &vs_cached)) {
      continue;
    }

    NS::Error* error = nullptr;
    dispatch_data_t vs_data = dispatch_data_create(
        vs_cached.metallib_data.data(), vs_cached.metallib_data.size(), nullptr,
        DISPATCH_DATA_DESTRUCTOR_NONE);
    MTL::Library* vs_library = device_->newLibrary(vs_data, &error);
    dispatch_release(vs_data);
    if (!vs_library) {
      continue;
    }
    NS::String* vs_name = NS::String::string(vs_cached.function_name.c_str(),
                                             NS::UTF8StringEncoding);
    MTL::Function* vs_function = vs_library->newFunction(vs_name);
    if (!vs_function) {
      vs_library->release();
      continue;
    }

    MTL::Library* ps_library = nullptr;
    MTL::Function* ps_function = nullptr;
    if (entry.pixel_shader_cache_key) {
      MetalShaderCache::CachedMetallib ps_cached;
      if (g_metal_shader_cache->Load(entry.pixel_shader_cache_key,
                                     &ps_cached)) {
        dispatch_data_t ps_data = dispatch_data_create(
            ps_cached.metallib_data.data(), ps_cached.metallib_data.size(),
            nullptr, DISPATCH_DATA_DESTRUCTOR_NONE);
        ps_library = device_->newLibrary(ps_data, &error);
        dispatch_release(ps_data);
        if (ps_library) {
          NS::String* ps_name = NS::String::string(
              ps_cached.function_name.c_str(), NS::UTF8StringEncoding);
          ps_function = ps_library->newFunction(ps_name);
        }
      }
    }

    MTL::RenderPipelineDescriptor* desc =
        MTL::RenderPipelineDescriptor::alloc()->init();
    desc->setVertexFunction(vs_function);
    if (ps_function) {
      desc->setFragmentFunction(ps_function);
    }

    for (uint32_t i = 0; i < 4; ++i) {
      desc->colorAttachments()->object(i)->setPixelFormat(
          static_cast<MTL::PixelFormat>(entry.color_formats[i]));
    }
    desc->setDepthAttachmentPixelFormat(
        static_cast<MTL::PixelFormat>(entry.depth_format));
    desc->setStencilAttachmentPixelFormat(
        static_cast<MTL::PixelFormat>(entry.stencil_format));
    desc->setSampleCount(entry.sample_count);
    desc->setAlphaToCoverageEnabled(entry.alpha_to_mask_enable != 0);

    for (uint32_t i = 0; i < 4; ++i) {
      auto* color_attachment = desc->colorAttachments()->object(i);
      if (entry.color_formats[i] ==
          static_cast<uint32_t>(MTL::PixelFormatInvalid)) {
        color_attachment->setWriteMask(MTL::ColorWriteMaskNone);
        color_attachment->setBlendingEnabled(false);
        continue;
      }
      uint32_t rt_write_mask = (entry.normalized_color_mask >> (i * 4)) & 0xF;
      color_attachment->setWriteMask(ToMetalColorWriteMask(rt_write_mask));
      if (!rt_write_mask) {
        color_attachment->setBlendingEnabled(false);
        continue;
      }

      reg::RB_BLENDCONTROL blendcontrol = {};
      blendcontrol.value = entry.blendcontrol[i];
      MTL::BlendFactor src_rgb =
          ToMetalBlendFactorRgb(blendcontrol.color_srcblend);
      MTL::BlendFactor dst_rgb =
          ToMetalBlendFactorRgb(blendcontrol.color_destblend);
      MTL::BlendOperation op_rgb =
          ToMetalBlendOperation(blendcontrol.color_comb_fcn);
      MTL::BlendFactor src_alpha =
          ToMetalBlendFactorAlpha(blendcontrol.alpha_srcblend);
      MTL::BlendFactor dst_alpha =
          ToMetalBlendFactorAlpha(blendcontrol.alpha_destblend);
      MTL::BlendOperation op_alpha =
          ToMetalBlendOperation(blendcontrol.alpha_comb_fcn);
      if (i == 0) {
        ApplyFixed16SourceFactorPremult(
            static_cast<MTL::PixelFormat>(entry.color_formats[0]), blendcontrol,
            src_rgb, src_alpha);
      }

      bool blending_enabled = src_rgb != MTL::BlendFactorOne ||
                              dst_rgb != MTL::BlendFactorZero ||
                              op_rgb != MTL::BlendOperationAdd ||
                              src_alpha != MTL::BlendFactorOne ||
                              dst_alpha != MTL::BlendFactorZero ||
                              op_alpha != MTL::BlendOperationAdd;
      color_attachment->setBlendingEnabled(blending_enabled);
      if (blending_enabled) {
        color_attachment->setSourceRGBBlendFactor(src_rgb);
        color_attachment->setDestinationRGBBlendFactor(dst_rgb);
        color_attachment->setRgbBlendOperation(op_rgb);
        color_attachment->setSourceAlphaBlendFactor(src_alpha);
        color_attachment->setDestinationAlphaBlendFactor(dst_alpha);
        color_attachment->setAlphaBlendOperation(op_alpha);
      }
    }

    if (!entry.vertex_attributes.empty() || !entry.vertex_layouts.empty()) {
      MTL::VertexDescriptor* vertex_desc =
          MTL::VertexDescriptor::alloc()->init();
      for (const auto& attr : entry.vertex_attributes) {
        auto* attr_desc =
            vertex_desc->attributes()->object(attr.attribute_index);
        attr_desc->setFormat(static_cast<MTL::VertexFormat>(attr.format));
        attr_desc->setOffset(attr.offset);
        attr_desc->setBufferIndex(attr.buffer_index);
      }
      for (const auto& layout : entry.vertex_layouts) {
        auto* layout_desc = vertex_desc->layouts()->object(layout.buffer_index);
        layout_desc->setStride(layout.stride);
        layout_desc->setStepFunction(
            static_cast<MTL::VertexStepFunction>(layout.step_function));
        layout_desc->setStepRate(layout.step_rate);
      }
      desc->setVertexDescriptor(vertex_desc);
      vertex_desc->release();
    }

    {
      std::lock_guard<std::mutex> lock(pipeline_binary_archive_mutex_);
      if (pipeline_binary_archive_) {
        NS::Array* archives = NS::Array::array(pipeline_binary_archive_);
        desc->setBinaryArchives(archives);
        if (pipeline_binary_archive_->addRenderPipelineFunctions(desc,
                                                                 &error)) {
          pipeline_binary_archive_dirty_ = true;
          ++prewarmed;
        }
      }
    }
    desc->release();
    vs_function->release();
    vs_library->release();
    if (ps_function) {
      ps_function->release();
    }
    if (ps_library) {
      ps_library->release();
    }
  }
}
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

void MetalCommandProcessor::IssueSwap(uint32_t frontbuffer_ptr,
                                      uint32_t frontbuffer_width,
                                      uint32_t frontbuffer_height) {
  ProcessCompletedSubmissions();
  // metal_frame_detail_log: the end of a logged frame, and the start of a
  // capture when the setting changed. draw_stats_ may have been reset since
  // the frame started.
  if (uint32_t frames = metal_perf_counters.frame_detail_frames.load(
          std::memory_order_relaxed)) {
    const DrawStats& frame_start =
        draw_stats_.frames >= frame_detail_stats_.frames ? frame_detail_stats_
                                                         : DrawStats{};
    XELOGI(
        "Frame detail: frame ended ({} more to log) with a swap of {:08X} "
        "({}x{}): {} passes, {} draws issued, {} encoded, {} skipped for "
        "compiles, {} rasterize nothing, {} without vertices, {} resolves",
        frames - 1, frontbuffer_ptr, frontbuffer_width, frontbuffer_height,
        frame_detail_passes_, draw_stats_.issued - frame_start.issued,
        draw_stats_.encoded - frame_start.encoded,
        draw_stats_.compile_pending - frame_start.compile_pending,
        draw_stats_.no_rasterization - frame_start.no_rasterization,
        draw_stats_.no_vertices - frame_start.no_vertices,
        draw_stats_.copies - frame_start.copies);
    metal_perf_counters.frame_detail_frames.store(frames - 1,
                                                  std::memory_order_relaxed);
  }
  if (cvars::metal_frame_detail_log != frame_detail_setting_) {
    frame_detail_setting_ = cvars::metal_frame_detail_log;
    if (frame_detail_setting_ > 0) {
      XELOGI("Frame detail: logging the next {} frames", frame_detail_setting_);
      metal_perf_counters.frame_detail_frames.store(
          uint32_t(frame_detail_setting_), std::memory_order_relaxed);
    }
  }
  frame_detail_passes_ = 0;
  frame_detail_draws_ = 0;
  zpd_frame_query_ends_ = 0;
  // metal_gpu_trace_frames: count the recorded frames down, then the start of
  // a recording when the setting changed. The trace opens at the next primary
  // buffer.
  if (gpu_trace_frames_left_) {
    if (trace_writer_.is_open()) {
      if (!--gpu_trace_frames_left_) {
        EndTracing();
        XELOGI("GPU trace: done");
      }
    } else if (trace_state_ != TraceState::kStreaming) {
      // The file could not be created.
      gpu_trace_frames_left_ = 0;
    }
  }
  if (cvars::metal_gpu_trace_frames != gpu_trace_setting_) {
    gpu_trace_setting_ = cvars::metal_gpu_trace_frames;
    if (gpu_trace_setting_ > 0 && !gpu_trace_frames_left_ &&
        trace_state_ == TraceState::kDisabled) {
      std::filesystem::path trace_path = cvars::trace_gpu_prefix;
      if (trace_path.is_relative() && kernel_state_ &&
          kernel_state_->emulator()) {
        trace_path = kernel_state_->emulator()->storage_root() / trace_path;
      }
      const std::time_t trace_time = std::time(nullptr);
      std::tm trace_local_time = {};
      localtime_r(&trace_time, &trace_local_time);
      char trace_folder[32];
      std::strftime(trace_folder, sizeof(trace_folder), "%Y-%m-%d_%H-%M-%S",
                    &trace_local_time);
      trace_path /= trace_folder;
      BeginTracing(trace_path);
      gpu_trace_frames_left_ = uint32_t(gpu_trace_setting_);
      XELOGI("GPU trace: recording the next {} frames into {}",
             gpu_trace_frames_left_, xe::path_to_utf8(trace_path));
    }
  }
  frame_detail_stats_ = draw_stats_;
  // Host time between guest swaps, for the perf line below.
  const auto metal_perf_swap_time = std::chrono::steady_clock::now();
  if (metal_perf_last_swap_.time_since_epoch().count()) {
    const uint64_t metal_perf_frame_us =
        uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(
                     metal_perf_swap_time - metal_perf_last_swap_)
                     .count());
    draw_stats_.frame_us_total += metal_perf_frame_us;
    draw_stats_.frame_us_max =
        std::max(draw_stats_.frame_us_max, metal_perf_frame_us);
    draw_stats_.frames_over_33ms += metal_perf_frame_us > 33333;
    draw_stats_.frames_over_50ms += metal_perf_frame_us > 50000;
  }
  metal_perf_last_swap_ = metal_perf_swap_time;
  if (++draw_stats_.frames >= kDrawStatsFrames) {
    // "other" is every draw that left the draw path for a reason not counted
    // separately (errors are logged where they happen).
    XELOGI(
        "Metal draw stats over {} frames: issued={} copies={} "
        "no_rasterization={} no_vertices={} compile_pending={} "
        "depth_clear_retargets={} encoded={} other={} encoded_vertices={}",
        draw_stats_.frames, draw_stats_.issued, draw_stats_.copies,
        draw_stats_.no_rasterization, draw_stats_.no_vertices,
        draw_stats_.compile_pending, draw_stats_.depth_clear_retargets,
        draw_stats_.encoded,
        draw_stats_.issued - draw_stats_.copies - draw_stats_.no_rasterization -
            draw_stats_.no_vertices - draw_stats_.compile_pending -
            draw_stats_.depth_clear_retargets - draw_stats_.encoded,
        draw_stats_.encoded_vertices);
    // Performance diagnostics for the same window.
    uint64_t metal_perf_footprint_mb = 0;
#if XE_PLATFORM_APPLE
    task_vm_info_data_t metal_perf_vm_info;
    mach_msg_type_number_t metal_perf_vm_count = TASK_VM_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_VM_INFO,
                  reinterpret_cast<task_info_t>(&metal_perf_vm_info),
                  &metal_perf_vm_count) == KERN_SUCCESS) {
      metal_perf_footprint_mb = metal_perf_vm_info.phys_footprint >> 20;
    }
#endif  // XE_PLATFORM_APPLE
    uint64_t metal_perf_available_mb = 0;
#if XE_PLATFORM_IOS
    metal_perf_available_mb = uint64_t(os_proc_available_memory()) >> 20;
#endif  // XE_PLATFORM_IOS
    const uint64_t metal_perf_window_us =
        std::max<uint64_t>(draw_stats_.frame_us_total, 1);
    const uint64_t metal_perf_gpu_us =
        metal_perf_counters.gpu_us.exchange(0, std::memory_order_relaxed);
    XELOGI(
        "Metal perf over {} frames: frame avg {:.1f} ms, max {:.1f} ms, {} "
        "over 33 ms, {} over 50 ms; GPU busy {:.0f} ms in {} command buffers "
        "({:.0f}% of the time); CPU blocked on GPU {} times, {:.1f} ms; "
        "shaders compiled {} ({:.0f} ms), pipelines created {} ({:.0f} ms); "
        "draws waited for compiles {} times, {:.1f} ms; "
        "uploaded {:.1f} MB; memory footprint {} MB, available {} MB; "
        "thermal state {}",
        draw_stats_.frames,
        draw_stats_.frame_us_total / 1000.0 / draw_stats_.frames,
        draw_stats_.frame_us_max / 1000.0, draw_stats_.frames_over_33ms,
        draw_stats_.frames_over_50ms, metal_perf_gpu_us / 1000.0,
        metal_perf_counters.gpu_command_buffers.exchange(
            0, std::memory_order_relaxed),
        metal_perf_gpu_us * 100.0 / metal_perf_window_us,
        metal_perf_counters.cpu_waits.exchange(0, std::memory_order_relaxed),
        metal_perf_counters.cpu_wait_us.exchange(0, std::memory_order_relaxed) /
            1000.0,
        metal_perf_counters.shader_compiles.exchange(0,
                                                     std::memory_order_relaxed),
        metal_perf_counters.shader_compile_us.exchange(
            0, std::memory_order_relaxed) /
            1000.0,
        metal_perf_counters.pipeline_compiles.exchange(
            0, std::memory_order_relaxed),
        metal_perf_counters.pipeline_compile_us.exchange(
            0, std::memory_order_relaxed) /
            1000.0,
        metal_perf_counters.compile_waits.exchange(0,
                                                   std::memory_order_relaxed),
        metal_perf_counters.compile_wait_us.exchange(
            0, std::memory_order_relaxed) /
            1000.0,
        metal_perf_counters.upload_bytes.exchange(0,
                                                  std::memory_order_relaxed) /
            1048576.0,
        metal_perf_footprint_mb, metal_perf_available_mb,
        uint32_t(NS::ProcessInfo::processInfo()->thermalState()));
    XELOGI("Metal perf threads:{}", MetalPerfThreadCpu(metal_perf_window_us));
    XELOGI(
        "Metal uniforms over {} frames: draws waited for a free uniform buffer "
        "{} times, {:.1f} ms; command buffers ended for want of one {} times "
        "(at most {} buffers)",
        draw_stats_.frames,
        metal_perf_counters.uniform_waits.exchange(0,
                                                   std::memory_order_relaxed),
        metal_perf_counters.uniform_wait_us.exchange(
            0, std::memory_order_relaxed) /
            1000.0,
        metal_perf_counters.uniform_rollovers.exchange(
            0, std::memory_order_relaxed),
        MetalUniformBuffersMax());
    {
      auto take = [](std::atomic<uint64_t>& counter) {
        return counter.exchange(0, std::memory_order_relaxed);
      };
      const uint64_t area_draw_passes =
          take(metal_perf_counters.render_area_draw_passes);
      const uint64_t area_draw_limited =
          take(metal_perf_counters.render_area_draw_limited);
      const uint64_t area_draw_rows =
          take(metal_perf_counters.render_area_draw_rows);
      const uint64_t area_transfer_passes =
          take(metal_perf_counters.render_area_transfer_passes);
      const uint64_t area_transfer_limited =
          take(metal_perf_counters.render_area_transfer_limited);
      const uint64_t area_transfer_rows =
          take(metal_perf_counters.render_area_transfer_rows);
      XELOGI(
          "Metal render area and queries over {} frames: draw passes limited "
          "{} of {} (average {} rows), restarted {}; transfer passes limited "
          "{} of {} (average {} rows); occlusion queries {:.1f} per frame "
          "({}), {} query events",
          draw_stats_.frames, area_draw_limited, area_draw_passes,
          area_draw_limited ? area_draw_rows / area_draw_limited : 0,
          take(metal_perf_counters.render_area_restarts), area_transfer_limited,
          area_transfer_passes,
          area_transfer_limited ? area_transfer_rows / area_transfer_limited
                                : 0,
          double(take(metal_perf_counters.occlusion_queries)) /
              draw_stats_.frames,
          cvars::occlusion_query_enable ? "counted" : "faked as visible",
          take(metal_perf_counters.occlusion_query_events));
      if (render_target_cache_) {
        uint64_t unchanged_transfers, skipped_transfers;
        render_target_cache_->TakeUnchangedTransferCounts(unchanged_transfers,
                                                          skipped_transfers);
        XELOGI(
            "Metal render target transfers over {} frames: {} ranges went "
            "back to a render target whose copy was unchanged, {} of them "
            "skipped (skip_unchanged_render_target_transfers {})",
            draw_stats_.frames, unchanged_transfers, skipped_transfers,
            cvars::skip_unchanged_render_target_transfers ? "on" : "off");
      }
      if (cvars::occlusion_query_enable) {
        XELOGI(
            "Metal occlusion queries over {} frames: {} counted ({} with no "
            "samples), {} guesses corrected, {} left as guesses (draws "
            "skipped for compiles, or no free slot), {} draw passes restarted "
            "to count, {} in flight; guessing by {} (score {})",
            draw_stats_.frames, take(metal_perf_counters.zpd_queries_measured),
            take(metal_perf_counters.zpd_queries_hidden),
            take(metal_perf_counters.zpd_queries_corrected),
            take(metal_perf_counters.zpd_queries_guessed),
            take(metal_perf_counters.zpd_pass_restarts),
            zpd_queries_in_flight_.size(),
            zpd_ordinal_score_ > 0 ? "place in the frame" : "address",
            zpd_ordinal_score_);
      }
    }
    metal_gpu_pass_timer.Log(draw_stats_.frames, metal_perf_window_us);
    metal_compile_stall_tracker.Log(draw_stats_.frames);
    xe::kernel::shim::LogGuestThreadHotspots();
    draw_stats_ = {};
  }
  saw_swap_ = true;
  msl_compile_wait_used_us_ = 0;
  {
    std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
    msl_compile_frame_arrival_ = msl_compile_arrivals_;
  }
  // A frame later, the render areas only remember this frame's.
  for (auto& [area_attachments, known_area] : render_area_by_attachments_) {
    known_area[2] = known_area[0];
    known_area[3] = known_area[1];
    known_area[0] = 0;
    known_area[1] = 0;
  }
  last_swap_ptr_ = frontbuffer_ptr;
  last_swap_width_ = frontbuffer_width;
  last_swap_height_ = frontbuffer_height;

  // End any active render encoder
  EndRenderEncoder();

  // Submit and wait for command buffer
  if (current_command_buffer_) {
#if METAL_SHADER_CONVERTER_AVAILABLE
    ScheduleDrawRingRelease(current_command_buffer_);
#else
    if (UseSpirvCrossPath()) {
      ScheduleSpirvUniformBufferRelease(current_command_buffer_);
    }
#endif
    ScheduleSpirvArgumentBufferRelease(current_command_buffer_);
    AttachShaderDoneFences(current_command_buffer_);
    current_command_buffer_->commit();
    current_command_buffer_->release();
    current_command_buffer_ = nullptr;
#if METAL_SHADER_CONVERTER_AVAILABLE
    SetActiveDrawRing(nullptr);
#endif
    current_draw_index_ = 0;
    copy_resolve_writes_pending_ = false;
  }

  if (primitive_processor_ && frame_open_) {
    primitive_processor_->EndFrame();
    frame_open_ = false;
  }
  // Frame boundary reached - resolved memory tracking is only needed within a
  // frame when trace playback writes memory.
  ClearResolvedMemory();
  if (shared_memory_ && ::cvars::clear_memory_page_state) {
    shared_memory_->SetSystemPageBlocksValidWithGpuDataWritten();
  }

  // Push the rendered frame to the presenter's guest output mailbox
  // This is required for trace dumps to capture the output. Use the
  // MetalRenderTargetCache color target (like D3D12) rather than the
  // legacy standalone render_target_texture_.
  auto* presenter =
      static_cast<ui::metal::MetalPresenter*>(graphics_system_->presenter());
  if (presenter && render_target_cache_) {
    uint32_t output_width =
        frontbuffer_width ? frontbuffer_width : render_target_width_;
    uint32_t output_height =
        frontbuffer_height ? frontbuffer_height : render_target_height_;

    MTL::Texture* source_texture = nullptr;
    bool use_pwl_gamma_ramp = false;
    if (texture_cache_) {
      uint32_t swap_width = 0;
      uint32_t swap_height = 0;
      xenos::TextureFormat swap_format = xenos::TextureFormat::k_8_8_8_8;
      source_texture = texture_cache_->RequestSwapTexture(
          swap_width, swap_height, swap_format);
      if (source_texture) {
        output_width = swap_width;
        output_height = swap_height;
        use_pwl_gamma_ramp =
            swap_format == xenos::TextureFormat::k_2_10_10_10 ||
            swap_format == xenos::TextureFormat::k_2_10_10_10_AS_16_16_16_16;
        static MTL::PixelFormat last_format = MTL::PixelFormatInvalid;
        static uint32_t last_samples = 0;
        static uint32_t last_width = 0;
        static uint32_t last_height = 0;
        static int last_swap_format = -1;
        MTL::PixelFormat src_format = source_texture->pixelFormat();
        uint32_t src_samples = source_texture->sampleCount();
        uint32_t src_width = uint32_t(source_texture->width());
        uint32_t src_height = uint32_t(source_texture->height());
        int swap_format_int = static_cast<int>(swap_format);
        if (src_format != last_format || src_samples != last_samples ||
            src_width != last_width || src_height != last_height ||
            swap_format_int != last_swap_format) {
          last_format = src_format;
          last_samples = src_samples;
          last_width = src_width;
          last_height = src_height;
          last_swap_format = swap_format_int;
        }
        if (presenter) {
          if (!gamma_ramp_256_entry_table_up_to_date_ ||
              !gamma_ramp_pwl_up_to_date_) {
            constexpr size_t kGammaRampTableBytes =
                sizeof(reg::DC_LUT_30_COLOR) * 256;
            constexpr size_t kGammaRampPwlBytes =
                sizeof(reg::DC_LUT_PWL_DATA) * 128 * 3;
            if (presenter->UpdateGammaRamp(
                    gamma_ramp_256_entry_table(), kGammaRampTableBytes,
                    gamma_ramp_pwl_rgb(), kGammaRampPwlBytes)) {
              gamma_ramp_256_entry_table_up_to_date_ = true;
              gamma_ramp_pwl_up_to_date_ = true;
            } else {
              XELOGW("Metal IssueSwap: gamma ramp upload failed");
            }
          }
        }
      }
    }

    bool swap_dest_swap = false;
    const bool has_swap_dest_swap =
        ConsumeSwapDestSwap(frontbuffer_ptr, &swap_dest_swap);
    if (!has_swap_dest_swap && frontbuffer_ptr) {
      static uint32_t swap_dest_miss_count = 0;
      if (swap_dest_miss_count < 8) {
        ++swap_dest_miss_count;
      }
    }
    bool force_swap_rb = has_swap_dest_swap && swap_dest_swap;

    if (!source_texture) {
      static bool missing_swap_logged = false;
      if (!missing_swap_logged) {
        missing_swap_logged = true;
        XELOGW(
            "MetalCommandProcessor::IssueSwap: swap texture unavailable; "
            "presenting inactive (black) output");
      }
      presenter->RefreshGuestOutput(
          0, 0, 0, 0, [](ui::Presenter::GuestOutputRefreshContext&) -> bool {
            return false;
          });
      return;
    }

    if (source_texture) {
      ui::metal::MetalPresenter* metal_presenter = presenter;
      uint32_t source_width = output_width;
      uint32_t source_height = output_height;
      bool force_swap_rb_copy = force_swap_rb;
      bool use_pwl_gamma_ramp_copy = use_pwl_gamma_ramp;
      auto aspect = graphics_system_->GetScaledAspectRatio();
      presenter->RefreshGuestOutput(
          output_width, output_height, aspect.first, aspect.second,
          [source_texture, metal_presenter, source_width, source_height,
           force_swap_rb_copy, use_pwl_gamma_ramp_copy](
              ui::Presenter::GuestOutputRefreshContext& context) -> bool {
            auto& metal_context =
                static_cast<ui::metal::MetalGuestOutputRefreshContext&>(
                    context);
            context.SetIs8bpc(!use_pwl_gamma_ramp_copy);
            uint64_t submission_id = 0;
            bool copy_success = metal_presenter->CopyTextureToGuestOutput(
                source_texture, metal_context.resource_uav_capable(),
                source_width, source_height, force_swap_rb_copy,
                use_pwl_gamma_ramp_copy, &submission_id);
            if (copy_success && submission_id) {
              metal_context.SetSubmissionId(submission_id);
            }
            return copy_success;
          });
    }
  }
}

void MetalCommandProcessor::OnPrimaryBufferEnd() {
  if (!current_command_buffer_) {
    return;
  }

  // In the SPIRV-Cross path, keep command buffers open across primary-buffer
  // boundaries unless a copy->draw visibility boundary is pending.
  if (UseSpirvCrossPath() && !copy_resolve_writes_pending_) {
    return;
  }

  if (!cvars::submit_on_primary_buffer_end) {
    return;
  }

  if (!copy_resolve_writes_pending_ && !CanEndSubmissionImmediately()) {
    return;
  }
  EndCommandBuffer();
}

bool MetalCommandProcessor::CanEndSubmissionImmediately() {
  if (!current_command_buffer_) {
    return true;
  }
  if (!cvars::async_shader_compilation || msl_shader_compile_threads_.empty()) {
    return true;
  }
  std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
  return msl_shader_compile_busy_ == 0 && msl_shader_compile_queue_.empty() &&
         msl_pipeline_compile_queue_.empty() &&
         msl_shader_compile_pending_.empty() &&
         msl_pipeline_compile_pending_.empty();
}

Shader* MetalCommandProcessor::LoadShader(xenos::ShaderType shader_type,
                                          uint32_t guest_address,
                                          const uint32_t* host_address,
                                          uint32_t dword_count) {
  // Create hash for caching using XXH3 (same as D3D12)
  uint64_t hash = XXH3_64bits(host_address, dword_count * sizeof(uint32_t));

  if (UseSpirvCrossPath()) {
    // SPIRV-Cross path: use MslShader (inherits SpirvShader).
    auto it = msl_shader_cache_.find(hash);
    if (it != msl_shader_cache_.end()) {
      return it->second.get();
    }
    auto shader = std::make_unique<MslShader>(shader_type, hash, host_address,
                                              dword_count);
    MslShader* result = shader.get();
    msl_shader_cache_[hash] = std::move(shader);
    XELOGD("Loaded {} shader (SPIRV-Cross) at {:08X} ({} dwords, hash {:016X})",
           shader_type == xenos::ShaderType::kVertex ? "vertex" : "pixel",
           guest_address, dword_count, hash);
    return result;
  }

#if METAL_SHADER_CONVERTER_AVAILABLE
  // MSC (DXBC) path: use MetalShader (inherits DxbcShader).
  auto it = shader_cache_.find(hash);
  if (it != shader_cache_.end()) {
    return it->second.get();
  }

  // Create new shader - analysis and translation happen later when the shader
  // is actually used in a draw call (matching D3D12 pattern)
  auto shader = std::make_unique<MetalShader>(shader_type, hash, host_address,
                                              dword_count);

  MetalShader* result = shader.get();
  shader_cache_[hash] = std::move(shader);

  XELOGD("Loaded {} shader at {:08X} ({} dwords, hash {:016X})",
         shader_type == xenos::ShaderType::kVertex ? "vertex" : "pixel",
         guest_address, dword_count, hash);

  return result;
#else
  XELOGE("MSC shader path not available but metal_use_spirvcross is false");
  return nullptr;
#endif  // METAL_SHADER_CONVERTER_AVAILABLE
}

bool MetalCommandProcessor::IssueDraw(xenos::PrimitiveType primitive_type,
                                      uint32_t index_count,
                                      IndexBufferInfo* index_buffer_info,
                                      bool major_mode_explicit) {
  const RegisterFile& regs = *register_file_;
  uint32_t normalized_color_mask = 0;

  // Check for copy mode
  xenos::EdramMode edram_mode = regs.Get<reg::RB_MODECONTROL>().edram_mode;
  if (edram_mode != xenos::EdramMode::kCopy && copy_resolve_writes_pending_) {
    // Preserve resolve write visibility when transitioning from copy-only
    // bursts to regular draw work.
    // MSC keeps the conservative split behavior; SPIRV-Cross decides in
    // IssueDrawMsl based on actual texture overlap with pending resolve writes.
    if (!UseSpirvCrossPath()) {
      EndCommandBuffer();
    }
  }
  ++draw_stats_.issued;
  if (edram_mode == xenos::EdramMode::kCopy) {
    ++draw_stats_.copies;
    return IssueCopy();
  }

  // Vertex shader analysis — use Shader* base type so both MSC and
  // SPIRV-Cross paths share this common code.
  Shader* vertex_shader = active_vertex_shader();
  if (!vertex_shader) {
    XELOGW("IssueDraw: No vertex shader");
    return false;
  }
  if (!vertex_shader->is_ucode_analyzed()) {
    vertex_shader->AnalyzeUcode(ucode_disasm_buffer_);
  }
  bool memexport_used_vertex = vertex_shader->memexport_eM_written() != 0;

  // Pixel shader analysis.
  bool primitive_polygonal = draw_util::IsPrimitivePolygonal(regs);
  bool is_rasterization_done =
      draw_util::IsRasterizationPotentiallyDone(regs, primitive_polygonal);
  Shader* pixel_shader = nullptr;
  if (is_rasterization_done) {
    if (edram_mode == xenos::EdramMode::kColorDepth) {
      pixel_shader = active_pixel_shader();
      if (pixel_shader) {
        if (!pixel_shader->is_ucode_analyzed()) {
          pixel_shader->AnalyzeUcode(ucode_disasm_buffer_);
        }
        if (!draw_util::IsPixelShaderNeededWithRasterization(*pixel_shader,
                                                             regs)) {
          pixel_shader = nullptr;
        }
      }
    }
  } else {
    if (!memexport_used_vertex) {
      ++draw_stats_.no_rasterization;
      return true;
    }
  }
  bool memexport_used_pixel =
      pixel_shader && (pixel_shader->memexport_eM_written() != 0);
  bool memexport_used = memexport_used_vertex || memexport_used_pixel;
  memexport_ranges_.clear();
  if (memexport_used_vertex) {
    draw_util::AddMemExportRanges(regs, *vertex_shader, memexport_ranges_);
  }
  if (memexport_used_pixel) {
    draw_util::AddMemExportRanges(regs, *pixel_shader, memexport_ranges_);
  }
  // Primitive/index processing (like D3D12/Vulkan).
  PrimitiveProcessor::ProcessingResult primitive_processing_result;
  if (!primitive_processor_) {
    XELOGE("IssueDraw: primitive processor is not initialized");
    return false;
  }
  if (!primitive_processor_->Process(primitive_processing_result)) {
    XELOGE("IssueDraw: primitive processing failed");
    return false;
  }
  if (!primitive_processing_result.host_draw_vertex_count) {
    ++draw_stats_.no_vertices;
    return true;
  }
  if (primitive_processing_result.host_vertex_shader_type ==
      Shader::HostVertexShaderType::kMemExportCompute) {
    primitive_processing_result.host_vertex_shader_type =
        Shader::HostVertexShaderType::kVertex;
  }

  bool use_tessellation_emulation = false;
  if (primitive_processing_result.IsTessellated()) {
    // The MSC path uses mesh shader emulation for tessellation, so it requires
    // mesh shader support. The SPIRV-Cross path uses native Metal tessellation
    // (drawPatches), so mesh shaders are not needed.
    if (!UseSpirvCrossPath() && !mesh_shader_supported_) {
      static bool tess_mesh_logged = false;
      if (!tess_mesh_logged) {
        tess_mesh_logged = true;
        XELOGW(
            "Metal: Tessellation emulation requested but mesh shaders are not "
            "supported on this device");
      }
      return true;
    }
    if (!pixel_shader) {
      static bool tess_no_ps_logged = false;
      if (!tess_no_ps_logged) {
        tess_no_ps_logged = true;
        XELOGW(
            "Metal: Tessellation emulation requested without a pixel shader; "
            "using depth-only PS fallback");
      }
    }
    use_tessellation_emulation = true;
  }

  // Configure render targets via MetalRenderTargetCache, similar to D3D12.
  if (render_target_cache_) {
    auto normalized_depth_control = draw_util::GetNormalizedDepthControl(regs);
    uint32_t ps_writes_color_targets =
        pixel_shader ? pixel_shader->writes_color_targets() : 0;
    normalized_color_mask = pixel_shader ? draw_util::GetNormalizedColorMask(
                                               regs, ps_writes_color_targets)
                                         : 0;
    // depth_clear_retarget_4x: a rectangle with only depth or stencil to
    // write and no occlusion query counting it may be written into the owner
    // of the tiles instead of being drawn.
    render_target_cache_->SetDepthClearRetargetCandidate(
        is_rasterization_done && !pixel_shader && !memexport_used &&
        !use_tessellation_emulation && !zpd_query_active_ &&
        regs.Get<reg::VGT_DRAW_INITIATOR>().prim_type ==
            xenos::PrimitiveType::kRectangleList);
    if (!render_target_cache_->Update(is_rasterization_done,
                                      normalized_depth_control,
                                      normalized_color_mask, *vertex_shader)) {
      XELOGE(
          "MetalCommandProcessor::IssueDraw - RenderTargetCache::Update "
          "failed");
      return false;
    }
    if (render_target_cache_->last_update_depth_clear_retargeted()) {
      ++draw_stats_.depth_clear_retargets;
      return true;
    }
  }

  // The render area this draw needs (metal_render_pass_area_from_scissor),
  // for BeginCommandBuffer, which grows the pass to it if needed.
  render_area_needed_width_ = 0;
  render_area_needed_height_ = 0;
  if (cvars::metal_render_pass_area_from_scissor) {
    draw_util::Scissor area_scissor;
    draw_util::GetScissor(regs, area_scissor);
    uint32_t area_scale_x =
        texture_cache_ ? texture_cache_->draw_resolution_scale_x() : 1;
    uint32_t area_scale_y =
        texture_cache_ ? texture_cache_->draw_resolution_scale_y() : 1;
    if (render_target_cache_) {
      // msaa_samples_as_pixels: drawing to the samples of an MSAA surface.
      area_scale_x <<=
          render_target_cache_->last_update_samples_as_pixels_x_log2();
      area_scale_y <<=
          render_target_cache_->last_update_samples_as_pixels_y_log2();
    }
    uint32_t area_bottom = area_scissor.offset[1] + area_scissor.extent[1];
    // The scissor is often the whole surface; the height the render target
    // cache estimated the draw uses (from the viewport and the vertex
    // shader), which its EDRAM ownership is based on, is usually far lower.
    if (render_target_cache_ &&
        render_target_cache_->last_update_height_used()) {
      area_bottom = std::min(area_bottom,
                             render_target_cache_->last_update_height_used());
    }
    render_area_needed_width_ = std::max(
        (area_scissor.offset[0] + area_scissor.extent[0]) * area_scale_x,
        uint32_t(1));
    render_area_needed_height_ =
        std::max(area_bottom * area_scale_y, uint32_t(1));
  }

  // A draw reading what a resolve in the open command buffer wrote is split
  // from it (IssueDrawMsl). Done before the render pass begins where the
  // shaders' textures are already known (from their first translation): split
  // after, the pass just begun ended with no draws, its whole area loaded and
  // stored for nothing (GTA IV: one per predicated tiling band, its four 2x
  // MSAA targets).
  if (UseSpirvCrossPath() && copy_resolve_writes_pending_) {
    uint32_t known_texture_mask = static_cast<MslShader*>(vertex_shader)
                                      ->GetUsedTextureMaskAfterTranslation();
    if (pixel_shader) {
      known_texture_mask |= static_cast<MslShader*>(pixel_shader)
                                ->GetUsedTextureMaskAfterTranslation();
    }
    if (known_texture_mask &&
        DrawReadsPendingResolveOutput(known_texture_mask, regs)) {
      SplitForPendingResolveOutput(known_texture_mask);
    }
  }

  // Begin command buffer if needed (will use cache-provided render targets).
  BeginCommandBuffer();
  if (!current_command_buffer_ || !current_render_encoder_) {
    static bool spirv_no_command_buffer_logged = false;
    if (!spirv_no_command_buffer_logged) {
      spirv_no_command_buffer_logged = true;
      XELOGE(
          "IssueDraw: failed to begin Metal command buffer/render encoder; "
          "skipping draws until uniforms buffer allocation recovers");
    }
    return UseSpirvCrossPath();
  }
#if METAL_SHADER_CONVERTER_AVAILABLE
  if (!EnsureDrawRingCapacity()) {
    static std::atomic<int64_t> draw_ring_exhaustion_last_log_ns{0};
    if (ShouldLogRateLimited(draw_ring_exhaustion_last_log_ns,
                             kDrawRingPoolLogIntervalNs)) {
      XELOGW("IssueDraw: skipping draw due to draw-ring pool exhaustion");
    }
    return true;
  }
#else
  // spirv2air shaders read their constants from the uniform buffers' tables
  // whatever metal_inline_shader_constants says.
  if (UseSpirvCrossPath() &&
      (!cvars::metal_inline_shader_constants ||
       cvars::metal_shader_compiler == "spirv2air") &&
      !EnsureSpirvUniformBufferCapacity()) {
    XELOGE(
        "IssueDraw: failed to prepare SPIRV-Cross uniforms ring; skipping "
        "draw");
    return true;
  }
#endif

  // =========================================================================
  // SPIRV-Cross (MSL) draw path — bypasses the entire MSC / IRRuntime flow.
  // =========================================================================
  if (UseSpirvCrossPath()) {
    return IssueDrawMsl(vertex_shader, pixel_shader,
                        primitive_processing_result, primitive_polygonal,
                        is_rasterization_done, memexport_used,
                        normalized_color_mask, regs);
  }

#if METAL_SHADER_CONVERTER_AVAILABLE
  // =========================================================================
  // MSC (DXBC → DXIL → Metal IR) draw path — existing code below.
  // =========================================================================
  // Cast to MSC-specific shader types for the rest of this path.
  auto* metal_vertex_shader = static_cast<MetalShader*>(vertex_shader);
  auto* metal_pixel_shader = static_cast<MetalShader*>(pixel_shader);

  MTL::RenderPipelineState* pipeline = nullptr;
  // Select per-draw shader modifications (mirrors D3D12 PipelineCache).
  uint32_t ps_param_gen_pos = UINT32_MAX;
  uint32_t interpolator_mask = 0;
  if (pixel_shader) {
    interpolator_mask = vertex_shader->writes_interpolators() &
                        pixel_shader->GetInterpolatorInputMask(
                            regs.Get<reg::SQ_PROGRAM_CNTL>(),
                            regs.Get<reg::SQ_CONTEXT_MISC>(), ps_param_gen_pos);
  }

  auto normalized_depth_control = draw_util::GetNormalizedDepthControl(regs);
  Shader::HostVertexShaderType host_vertex_shader_type_for_translation =
      primitive_processing_result.host_vertex_shader_type;
  if (host_vertex_shader_type_for_translation ==
          Shader::HostVertexShaderType::kPointListAsTriangleStrip ||
      host_vertex_shader_type_for_translation ==
          Shader::HostVertexShaderType::kRectangleListAsTriangleStrip) {
    if (!mesh_shader_supported_) {
      static bool host_vs_expansion_logged = false;
      if (!host_vs_expansion_logged) {
        host_vs_expansion_logged = true;
        XELOGW(
            "Metal: Host VS expansion requested without mesh shader support; "
            "skipping draw");
      }
      return true;
    }
    // Geometry emulation handles point/rectangle expansion; use the normal
    // vertex shader translation path to avoid unsupported host VS types.
    host_vertex_shader_type_for_translation =
        Shader::HostVertexShaderType::kVertex;
  }
  DxbcShaderTranslator::Modification vertex_shader_modification =
      GetCurrentVertexShaderModification(
          *vertex_shader, host_vertex_shader_type_for_translation,
          interpolator_mask);
  DxbcShaderTranslator::Modification pixel_shader_modification =
      pixel_shader ? GetCurrentPixelShaderModification(
                         *pixel_shader, interpolator_mask, ps_param_gen_pos,
                         normalized_depth_control)
                   : DxbcShaderTranslator::Modification(0);

  PipelineGeometryShader geometry_shader_type = PipelineGeometryShader::kNone;
  if (!primitive_processing_result.IsTessellated()) {
    switch (primitive_processing_result.host_primitive_type) {
      case xenos::PrimitiveType::kPointList:
        geometry_shader_type = PipelineGeometryShader::kPointList;
        break;
      case xenos::PrimitiveType::kRectangleList:
        geometry_shader_type = PipelineGeometryShader::kRectangleList;
        break;
      case xenos::PrimitiveType::kQuadList:
        geometry_shader_type = PipelineGeometryShader::kQuadList;
        break;
      default:
        break;
    }
  }

  GeometryShaderKey geometry_shader_key;
  bool use_geometry_emulation = false;
  if (geometry_shader_type != PipelineGeometryShader::kNone) {
    bool can_build_geometry_shader =
        pixel_shader || !vertex_shader_modification.vertex.interpolator_mask;
    if (!can_build_geometry_shader) {
      static bool geom_interp_mismatch_logged = false;
      if (!geom_interp_mismatch_logged) {
        geom_interp_mismatch_logged = true;
        XELOGW(
            "Metal: geometry emulation skipped because pixel shader is null "
            "but vertex interpolators are present");
      }
    } else {
      use_geometry_emulation =
          GetGeometryShaderKey(geometry_shader_type, vertex_shader_modification,
                               pixel_shader_modification, geometry_shader_key);
    }
  }
  if (use_geometry_emulation && !mesh_shader_supported_) {
    static bool mesh_support_logged = false;
    if (!mesh_support_logged) {
      mesh_support_logged = true;
      XELOGW(
          "Metal: geometry emulation requested but mesh shaders are not "
          "supported on this device");
    }
    use_geometry_emulation = false;
  }
  if (use_geometry_emulation && !pixel_shader) {
    static bool geom_no_ps_logged = false;
    if (!geom_no_ps_logged) {
      geom_no_ps_logged = true;
      XELOGW(
          "Metal: geometry emulation requested without a pixel shader; using "
          "depth-only PS fallback");
    }
  }

  // Get or create shader translations for the selected modifications.
  auto vertex_translation = static_cast<MetalShader::MetalTranslation*>(
      vertex_shader->GetOrCreateTranslation(vertex_shader_modification.value));
  if (!vertex_translation->is_translated()) {
    if (!shader_translator_->TranslateAnalyzedShader(*vertex_translation)) {
      XELOGE("Failed to translate vertex shader to DXBC");
      return false;
    }
  }
  if (!use_tessellation_emulation && !vertex_translation->is_valid()) {
    if (!vertex_translation->TranslateToMetal(device_, *dxbc_to_dxil_converter_,
                                              *metal_shader_converter_)) {
      XELOGE("Failed to translate vertex shader to Metal");
      return false;
    }
  }

  MetalShader::MetalTranslation* pixel_translation = nullptr;
  if (pixel_shader) {
    pixel_translation = static_cast<MetalShader::MetalTranslation*>(
        pixel_shader->GetOrCreateTranslation(pixel_shader_modification.value));
    if (!pixel_translation->is_translated()) {
      if (!shader_translator_->TranslateAnalyzedShader(*pixel_translation)) {
        XELOGE("Failed to translate pixel shader to DXBC");
        return false;
      }
    }
    if (!pixel_translation->is_valid()) {
      if (!pixel_translation->TranslateToMetal(
              device_, *dxbc_to_dxil_converter_, *metal_shader_converter_)) {
        XELOGE("Failed to translate pixel shader to Metal");
        return false;
      }
    }
  }

  TessellationPipelineState* tessellation_pipeline_state = nullptr;
  GeometryPipelineState* geometry_pipeline_state = nullptr;
  if (use_tessellation_emulation) {
    tessellation_pipeline_state = GetOrCreateTessellationPipelineState(
        vertex_translation, pixel_translation, primitive_processing_result,
        regs);
    pipeline = tessellation_pipeline_state
                   ? tessellation_pipeline_state->pipeline
                   : nullptr;
  } else if (use_geometry_emulation) {
    geometry_pipeline_state = GetOrCreateGeometryPipelineState(
        vertex_translation, pixel_translation, geometry_shader_key, regs);
    if (!geometry_pipeline_state || !geometry_pipeline_state->pipeline) {
      static bool geometry_pipeline_failure_logged = false;
      if (!geometry_pipeline_failure_logged) {
        geometry_pipeline_failure_logged = true;
        XELOGW(
            "Metal: geometry emulation pipeline creation failed; skipping "
            "geometry-emulated draws instead of aborting the backend draw "
            "packet");
      }
      return true;
    }
    pipeline = geometry_pipeline_state->pipeline;
  } else {
    pipeline =
        GetOrCreatePipelineState(vertex_translation, pixel_translation, regs);
  }

  if (!pipeline) {
    XELOGE("Failed to create pipeline state");
    return false;
  }

  uint32_t used_texture_mask =
      metal_vertex_shader->GetUsedTextureMaskAfterTranslation();
  if (metal_pixel_shader) {
    used_texture_mask |=
        metal_pixel_shader->GetUsedTextureMaskAfterTranslation();
  }
  if (texture_cache_ && used_texture_mask &&
      texture_cache_->AnyUsedTextureRequestWorkPending(used_texture_mask)) {
    texture_cache_->RequestTextures(used_texture_mask);
  }

  struct VertexBindingRange {
    uint32_t binding_index = 0;
    uint32_t offset = 0;
    uint32_t length = 0;
    uint32_t stride = 0;
  };
  std::array<VertexBindingRange, 32> vertex_ranges;
  uint32_t vertex_range_count = 0;
  const auto& vb_bindings = vertex_shader->vertex_bindings();
  bool uses_vertex_fetch = ShaderUsesVertexFetch(*vertex_shader);

  // Sync shared memory before drawing - ensure GPU has latest data
  // This is particularly important for trace playback where memory is
  // written incrementally
  if (shared_memory_) {
    const Shader::ConstantRegisterMap& constant_map_vertex =
        vertex_shader->constant_register_map();
    for (uint32_t i = 0;
         i < xe::countof(constant_map_vertex.vertex_fetch_bitmap); ++i) {
      uint32_t vfetch_bits_remaining =
          constant_map_vertex.vertex_fetch_bitmap[i];
      uint32_t j;
      while (xe::bit_scan_forward(vfetch_bits_remaining, &j)) {
        vfetch_bits_remaining &= ~(uint32_t(1) << j);
        uint32_t vfetch_index = i * 32 + j;
        xenos::xe_gpu_vertex_fetch_t vfetch = regs.GetVertexFetch(vfetch_index);
        switch (vfetch.type) {
          case xenos::FetchConstantType::kVertex:
            break;
          case xenos::FetchConstantType::kInvalidVertex:
            if (::cvars::gpu_allow_invalid_fetch_constants) {
              break;
            }
            XELOGW(
                "Vertex fetch constant {} ({:08X} {:08X}) has \"invalid\" "
                "type. "
                "Use --gpu_allow_invalid_fetch_constants to bypass.",
                vfetch_index, vfetch.dword_0, vfetch.dword_1);
            return false;
          default:
            XELOGW("Vertex fetch constant {} ({:08X} {:08X}) is invalid.",
                   vfetch_index, vfetch.dword_0, vfetch.dword_1);
            return false;
        }
        uint32_t buffer_offset = vfetch.address << 2;
        uint32_t buffer_length = vfetch.size << 2;
        if (buffer_offset > SharedMemory::kBufferSize ||
            SharedMemory::kBufferSize - buffer_offset < buffer_length) {
          XELOGW(
              "Vertex fetch constant {} out of range (offset=0x{:08X} size={})",
              vfetch_index, buffer_offset, buffer_length);
          return false;
        }
        if (!shared_memory_->RequestRangeForGpuRead(buffer_offset,
                                                    buffer_length)) {
          XELOGE(
              "Failed to request vertex buffer at 0x{:08X} (size {}) in shared "
              "memory",
              buffer_offset, buffer_length);
          return false;
        }
      }
    }

    for (const draw_util::MemExportRange& memexport_range : memexport_ranges_) {
      uint32_t base_bytes = memexport_range.base_address_dwords << 2;
      if (!shared_memory_->RequestRange(base_bytes,
                                        memexport_range.size_bytes)) {
        XELOGE(
            "Failed to request memexport stream at 0x{:08X} (size {}) in "
            "shared "
            "memory",
            base_bytes, memexport_range.size_bytes);
        return false;
      }
    }

    for (const auto& binding : vb_bindings) {
      xenos::xe_gpu_vertex_fetch_t vfetch =
          regs.GetVertexFetch(binding.fetch_constant);
      uint32_t buffer_offset = vfetch.address << 2;
      uint32_t buffer_length = vfetch.size << 2;
      VertexBindingRange range;
      range.binding_index = static_cast<uint32_t>(binding.binding_index);
      range.offset = buffer_offset;
      range.length = buffer_length;
      range.stride = binding.stride_words * 4;
      assert_true(vertex_range_count < vertex_ranges.size());
      vertex_ranges[vertex_range_count++] = range;
    }
  }

  // Set pipeline state on encoder
  current_render_encoder_->setRenderPipelineState(pipeline);
  if (use_tessellation_emulation) {
    if (!tessellator_tables_buffer_) {
      XELOGE("Tessellation emulation requires tessellator tables buffer");
      return false;
    }
    current_render_encoder_->setObjectBuffer(
        tessellator_tables_buffer_, 0, kIRRuntimeTessellatorTablesBindPoint);
    current_render_encoder_->setMeshBuffer(
        tessellator_tables_buffer_, 0, kIRRuntimeTessellatorTablesBindPoint);
    UseRenderEncoderResource(tessellator_tables_buffer_,
                             MTL::ResourceUsageRead);
  }

  // Determine if shared memory should be UAV (for memexport).
  bool shared_memory_is_uav = memexport_used_vertex || memexport_used_pixel;
  MTL::ResourceUsage shared_memory_usage =
      shared_memory_is_uav ? (MTL::ResourceUsageRead | MTL::ResourceUsageWrite)
                           : MTL::ResourceUsageRead;
  // Bind IR Converter runtime resources.
  // The Metal Shader Converter expects resources at specific bind points.
  if (res_heap_ab_ && smp_heap_ab_ && uniforms_buffer_ && shared_memory_) {
    // Determine primitive type characteristics
    bool primitive_polygonal = draw_util::IsPrimitivePolygonal(regs);

    // Get viewport info for NDC transform. Use the actual RT0 dimensions
    // when available so system constants match the current render target.
    uint32_t vp_width = 1;
    uint32_t vp_height = 1;
    GetBoundRenderTargetSize(render_target_cache_.get(), render_target_width_,
                             render_target_height_, vp_width, vp_height);
    draw_util::ViewportInfo viewport_info;
    auto depth_control = draw_util::GetNormalizedDepthControl(regs);
    constexpr uint32_t kViewportBoundsMax = 32767;
    bool host_render_targets_used = true;
    bool convert_z_to_float24 = host_render_targets_used &&
                                ::cvars::depth_float24_convert_in_pixel_shader;
    uint32_t draw_resolution_scale_x =
        texture_cache_ ? texture_cache_->draw_resolution_scale_x() : 1;
    uint32_t draw_resolution_scale_y =
        texture_cache_ ? texture_cache_->draw_resolution_scale_y() : 1;
    draw_util::GetViewportInfoArgs gviargs{};
    gviargs.Setup(
        draw_resolution_scale_x, draw_resolution_scale_y,
        texture_cache_ ? texture_cache_->draw_resolution_scale_x_divisor()
                       : divisors::MagicDiv(1),
        texture_cache_ ? texture_cache_->draw_resolution_scale_y_divisor()
                       : divisors::MagicDiv(1),
        true, kViewportBoundsMax, kViewportBoundsMax, false, depth_control,
        convert_z_to_float24, host_render_targets_used,
        pixel_shader && pixel_shader->writes_depth());
    gviargs.SetupRegisterValues(regs);
    draw_util::GetHostViewportInfo(&gviargs, viewport_info);

    // Apply per-draw viewport and scissor so the Metal viewport
    // matches the guest viewport computed by draw_util.
    draw_util::Scissor scissor;
    draw_util::GetScissor(regs, scissor);
    // draw_resolution_scale_x/y already computed above for viewport.
    scissor.offset[0] *= draw_resolution_scale_x;
    scissor.offset[1] *= draw_resolution_scale_y;
    scissor.extent[0] *= draw_resolution_scale_x;
    scissor.extent[1] *= draw_resolution_scale_y;

    // Clamp scissor to actual render target bounds (Metal requires this),
    // and to a limited render area, which is the pass's size.
    ClampScissorToBounds(scissor,
                         current_render_area_width_
                             ? std::min(vp_width, current_render_area_width_)
                             : vp_width,
                         current_render_area_height_
                             ? std::min(vp_height, current_render_area_height_)
                             : vp_height);

    MTL::Viewport mtl_viewport;
    mtl_viewport.originX = static_cast<double>(viewport_info.xy_offset[0]);
    mtl_viewport.originY = static_cast<double>(viewport_info.xy_offset[1]);
    mtl_viewport.width = static_cast<double>(viewport_info.xy_extent[0]);
    mtl_viewport.height = static_cast<double>(viewport_info.xy_extent[1]);
    mtl_viewport.znear = viewport_info.z_min;
    mtl_viewport.zfar = viewport_info.z_max;
    current_render_encoder_->setViewport(mtl_viewport);

    MTL::ScissorRect mtl_scissor;
    mtl_scissor.x = scissor.offset[0];
    mtl_scissor.y = scissor.offset[1];
    mtl_scissor.width = scissor.extent[0];
    mtl_scissor.height = scissor.extent[1];
    current_render_encoder_->setScissorRect(mtl_scissor);

    ApplyRasterizerState(primitive_polygonal);

    // Fixed-function depth/stencil state is not part of the pipeline state in
    // Metal, so update it per draw.
    ApplyDepthStencilState(primitive_polygonal, depth_control);

    // Update full system constants from GPU registers
    // This populates flags, NDC transform, alpha test, blend constants, etc.
    uint32_t normalized_color_mask =
        pixel_shader ? draw_util::GetNormalizedColorMask(
                           regs, pixel_shader->writes_color_targets())
                     : 0;
    UpdateSystemConstantValues(
        shared_memory_is_uav, primitive_polygonal,
        primitive_processing_result.line_loop_closing_index,
        primitive_processing_result.host_shader_index_endian, viewport_info,
        used_texture_mask, depth_control, normalized_color_mask);

    float blend_constants[] = {
        regs.Get<float>(XE_GPU_REG_RB_BLEND_RED),
        regs.Get<float>(XE_GPU_REG_RB_BLEND_GREEN),
        regs.Get<float>(XE_GPU_REG_RB_BLEND_BLUE),
        regs.Get<float>(XE_GPU_REG_RB_BLEND_ALPHA),
    };
    bool blend_factor_update_needed =
        !ff_blend_factor_valid_ ||
        std::memcmp(ff_blend_factor_, blend_constants, sizeof(float) * 4) != 0;
    if (blend_factor_update_needed) {
      std::memcpy(ff_blend_factor_, blend_constants, sizeof(float) * 4);
      ff_blend_factor_valid_ = true;
      current_render_encoder_->setBlendColor(
          blend_constants[0], blend_constants[1], blend_constants[2],
          blend_constants[3]);
    }

    constexpr size_t kStageVertex = 0;
    constexpr size_t kStagePixel = 1;
    uint32_t ring_index = current_draw_index_ % uint32_t(draw_ring_count_);
    size_t table_index_vertex = size_t(ring_index) * kStageCount + kStageVertex;
    size_t table_index_pixel = size_t(ring_index) * kStageCount + kStagePixel;

    // Uniforms buffer layout (4KB per CBV for alignment):
    //   b0 (offset 0):     System constants (~512 bytes)
    //   b1 (offset 4096):  Float constants (256 float4s = 4KB)
    //   b2 (offset 8192):  Bool/loop constants (~256 bytes)
    //   b3 (offset 12288): Fetch constants (768 bytes)
    //   b4 (offset 16384): Descriptor indices (unused in bindful mode)
    const size_t kCBVSize = kCbvSizeBytes;
    uint8_t* uniforms_base =
        static_cast<uint8_t*>(uniforms_buffer_->contents());
    uint8_t* uniforms_vertex =
        uniforms_base + table_index_vertex * kUniformsBytesPerTable;
    uint8_t* uniforms_pixel =
        uniforms_base + table_index_pixel * kUniformsBytesPerTable;

    // b0: System constants.
    std::memcpy(uniforms_vertex, &system_constants_,
                sizeof(DxbcShaderTranslator::SystemConstants));
    std::memcpy(uniforms_pixel, &system_constants_,
                sizeof(DxbcShaderTranslator::SystemConstants));

    // b1: Float constants at offset 4096 (1 * kCBVSize).
    const size_t kFloatConstantOffset = 1 * kCBVSize;
    // DxbcShaderTranslator uses packed float constants, mirroring the D3D12
    // backend behavior: only the constants actually used by the shader are
    // written sequentially based on Shader::ConstantRegisterMap.
    auto write_packed_float_constants = [&](uint8_t* dst, const Shader& shader,
                                            uint32_t regs_base) {
      std::memset(dst, 0, kCBVSize);
      const Shader::ConstantRegisterMap& map = shader.constant_register_map();
      if (!map.float_count) {
        return;
      }
      uint8_t* out = dst;
      for (uint32_t i = 0; i < 4; ++i) {
        uint64_t bits = map.float_bitmap[i];
        uint32_t constant_index;
        while (xe::bit_scan_forward(bits, &constant_index)) {
          bits &= ~(uint64_t(1) << constant_index);
          if (out + 4 * sizeof(uint32_t) > dst + kCBVSize) {
            return;
          }
          std::memcpy(
              out, &regs.values[regs_base + (i << 8) + (constant_index << 2)],
              4 * sizeof(uint32_t));
          out += 4 * sizeof(uint32_t);
        }
      }
    };

    // Vertex shader uses c0-c255, pixel shader uses c256-c511.
    write_packed_float_constants(uniforms_vertex + kFloatConstantOffset,
                                 *vertex_shader,
                                 XE_GPU_REG_SHADER_CONSTANT_000_X);
    if (pixel_shader) {
      write_packed_float_constants(uniforms_pixel + kFloatConstantOffset,
                                   *pixel_shader,
                                   XE_GPU_REG_SHADER_CONSTANT_256_X);
    } else {
      std::memset(uniforms_pixel + kFloatConstantOffset, 0, kCBVSize);
    }

    // b2: Bool/Loop constants at offset 8192 (2 * kCBVSize).
    const size_t kBoolLoopConstantOffset = 2 * kCBVSize;
    constexpr size_t kBoolLoopConstantsSize = (8 + 32) * sizeof(uint32_t);
    std::memcpy(uniforms_vertex + kBoolLoopConstantOffset,
                &regs.values[XE_GPU_REG_SHADER_CONSTANT_BOOL_000_031],
                kBoolLoopConstantsSize);
    std::memcpy(uniforms_pixel + kBoolLoopConstantOffset,
                &regs.values[XE_GPU_REG_SHADER_CONSTANT_BOOL_000_031],
                kBoolLoopConstantsSize);

    // b3: Fetch constants at offset 12288 (3 * kCBVSize).
    // 32 fetch groups * 6 DWORDs = 192 DWORDs (same data as 96 vertex fetches).
    const size_t kFetchConstantOffset = 3 * kCBVSize;
    const size_t kFetchConstantCount =
        xenos::kTextureFetchConstantCount * 6;  // 192 DWORDs = 768 bytes
    std::memcpy(uniforms_vertex + kFetchConstantOffset,
                &regs.values[XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0],
                kFetchConstantCount * sizeof(uint32_t));
    std::memcpy(uniforms_pixel + kFetchConstantOffset,
                &regs.values[XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0],
                kFetchConstantCount * sizeof(uint32_t));

    auto* res_entries_all =
        reinterpret_cast<IRDescriptorTableEntry*>(res_heap_ab_->contents());
    const size_t kDescriptorTableCount = kStageCount * draw_ring_count_;
    const size_t uav_table_base_index =
        kResourceHeapSlotsPerTable * kDescriptorTableCount;
    auto* uav_entries_all = res_entries_all + uav_table_base_index;
    auto* smp_entries_all =
        reinterpret_cast<IRDescriptorTableEntry*>(smp_heap_ab_->contents());
    auto* cbv_entries_all =
        reinterpret_cast<IRDescriptorTableEntry*>(cbv_heap_ab_->contents());

    IRDescriptorTableEntry* res_entries_vertex =
        res_entries_all + table_index_vertex * kResourceHeapSlotsPerTable;
    IRDescriptorTableEntry* res_entries_pixel =
        res_entries_all + table_index_pixel * kResourceHeapSlotsPerTable;
    IRDescriptorTableEntry* uav_entries_vertex =
        uav_entries_all + table_index_vertex * kResourceHeapSlotsPerTable;
    IRDescriptorTableEntry* uav_entries_pixel =
        uav_entries_all + table_index_pixel * kResourceHeapSlotsPerTable;
    IRDescriptorTableEntry* smp_entries_vertex =
        smp_entries_all + table_index_vertex * kSamplerHeapSlotsPerTable;
    IRDescriptorTableEntry* smp_entries_pixel =
        smp_entries_all + table_index_pixel * kSamplerHeapSlotsPerTable;
    IRDescriptorTableEntry* cbv_entries_vertex =
        cbv_entries_all + table_index_vertex * kCbvHeapSlotsPerTable;
    IRDescriptorTableEntry* cbv_entries_pixel =
        cbv_entries_all + table_index_pixel * kCbvHeapSlotsPerTable;

    uint64_t res_heap_gpu_base_vertex =
        res_heap_ab_->gpuAddress() + table_index_vertex *
                                         kResourceHeapSlotsPerTable *
                                         sizeof(IRDescriptorTableEntry);
    uint64_t res_heap_gpu_base_pixel =
        res_heap_ab_->gpuAddress() + table_index_pixel *
                                         kResourceHeapSlotsPerTable *
                                         sizeof(IRDescriptorTableEntry);
    uint64_t uav_heap_gpu_base_vertex =
        res_heap_ab_->gpuAddress() +
        (uav_table_base_index +
         table_index_vertex * kResourceHeapSlotsPerTable) *
            sizeof(IRDescriptorTableEntry);
    uint64_t uav_heap_gpu_base_pixel =
        res_heap_ab_->gpuAddress() +
        (uav_table_base_index +
         table_index_pixel * kResourceHeapSlotsPerTable) *
            sizeof(IRDescriptorTableEntry);
    uint64_t smp_heap_gpu_base_vertex =
        smp_heap_ab_->gpuAddress() + table_index_vertex *
                                         kSamplerHeapSlotsPerTable *
                                         sizeof(IRDescriptorTableEntry);
    uint64_t smp_heap_gpu_base_pixel =
        smp_heap_ab_->gpuAddress() + table_index_pixel *
                                         kSamplerHeapSlotsPerTable *
                                         sizeof(IRDescriptorTableEntry);

    uint64_t uniforms_gpu_base_vertex =
        uniforms_buffer_->gpuAddress() +
        table_index_vertex * kUniformsBytesPerTable;
    uint64_t uniforms_gpu_base_pixel =
        uniforms_buffer_->gpuAddress() +
        table_index_pixel * kUniformsBytesPerTable;

    MTL::Buffer* shared_mem_buffer = shared_memory_->GetBuffer();
    if (shared_mem_buffer) {
      IRDescriptorTableSetBuffer(&res_entries_vertex[0],
                                 shared_mem_buffer->gpuAddress(),
                                 shared_mem_buffer->length());
      IRDescriptorTableSetBuffer(&res_entries_pixel[0],
                                 shared_mem_buffer->gpuAddress(),
                                 shared_mem_buffer->length());
      IRDescriptorTableSetBuffer(&uav_entries_vertex[0],
                                 shared_mem_buffer->gpuAddress(),
                                 shared_mem_buffer->length());
      IRDescriptorTableSetBuffer(&uav_entries_pixel[0],
                                 shared_mem_buffer->gpuAddress(),
                                 shared_mem_buffer->length());
      UseRenderEncoderResource(shared_mem_buffer, shared_memory_usage);
    }
    if (render_target_cache_) {
      if (MTL::Buffer* edram_buffer = render_target_cache_->GetEdramBuffer()) {
        IRDescriptorTableSetBuffer(&uav_entries_vertex[1],
                                   edram_buffer->gpuAddress(),
                                   edram_buffer->length());
        IRDescriptorTableSetBuffer(&uav_entries_pixel[1],
                                   edram_buffer->gpuAddress(),
                                   edram_buffer->length());
        UseRenderEncoderResource(
            edram_buffer, MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
      }
    }

    std::array<MTL::Texture*, 64> textures_for_encoder;
    uint32_t textures_for_encoder_count = 0;
    auto track_texture_usage = [&](MTL::Texture* texture) {
      if (!texture) {
        return;
      }
      for (uint32_t i = 0; i < textures_for_encoder_count; ++i) {
        if (textures_for_encoder[i] == texture) {
          return;
        }
      }
      assert_true(textures_for_encoder_count < textures_for_encoder.size());
      textures_for_encoder[textures_for_encoder_count++] = texture;
    };

    auto bind_shader_textures = [&](const char* stage, MetalShader* shader,
                                    IRDescriptorTableEntry* stage_res_entries) {
      if (!shader || !texture_cache_) {
        return;
      }
      const auto& shader_texture_bindings =
          shader->GetTextureBindingsAfterTranslation();
      MetalTextureCache* metal_texture_cache = texture_cache_.get();
      for (size_t binding_index = 0;
           binding_index < shader_texture_bindings.size(); ++binding_index) {
        uint32_t srv_slot = 1 + static_cast<uint32_t>(binding_index);
        if (srv_slot >= kResourceHeapSlotsPerTable) {
          break;
        }
        const auto& binding = shader_texture_bindings[binding_index];
        MTL::Texture* texture = texture_cache_->GetTextureForBinding(
            binding.fetch_constant, binding.dimension, binding.is_signed);
        if (!texture) {
          // Use a dimension-compatible null texture to avoid Metal validation
          // errors (for example, cube-array expectations from converted
          // shaders).
          switch (binding.dimension) {
            case xenos::FetchOpDimension::k1D:
            case xenos::FetchOpDimension::k2D:
              texture = metal_texture_cache->GetNullTexture2D();
              break;
            case xenos::FetchOpDimension::k3DOrStacked:
              texture = metal_texture_cache->GetNullTexture3D();
              break;
            case xenos::FetchOpDimension::kCube:
              texture = metal_texture_cache->GetNullTextureCube();
              break;
            default:
              texture = metal_texture_cache->GetNullTexture2D();
              break;
          }
          if (!logged_missing_texture_warning_) {
            XELOGW(
                "Metal: Missing texture for fetch constant {} (dimension {} "
                "signed {})",
                binding.fetch_constant, static_cast<int>(binding.dimension),
                binding.is_signed);
            logged_missing_texture_warning_ = true;
          }
        }
        if (texture) {
          IRDescriptorTableSetTexture(&stage_res_entries[srv_slot], texture,
                                      0.0f, 0);
          track_texture_usage(texture);
        }
      }
    };

    auto bind_shader_samplers = [&](const char* stage, MetalShader* shader,
                                    IRDescriptorTableEntry* stage_smp_entries) {
      if (!shader || !texture_cache_) {
        return;
      }
      const auto& sampler_bindings =
          shader->GetSamplerBindingsAfterTranslation();
      for (size_t sampler_index = 0; sampler_index < sampler_bindings.size();
           ++sampler_index) {
        if (sampler_index >= kSamplerHeapSlotsPerTable) {
          break;
        }
        auto parameters = texture_cache_->GetSamplerParameters(
            sampler_bindings[sampler_index]);
        MTL::SamplerState* sampler_state =
            texture_cache_->GetOrCreateSampler(parameters);
        if (!sampler_state) {
          sampler_state = null_sampler_;
        }
        if (sampler_state) {
          IRDescriptorTableSetSampler(&stage_smp_entries[sampler_index],
                                      sampler_state, 0.0f);
        }
      }
    };

    bind_shader_textures("VS", metal_vertex_shader, res_entries_vertex);
    bind_shader_textures("PS", metal_pixel_shader, res_entries_pixel);
    bind_shader_samplers("VS", metal_vertex_shader, smp_entries_vertex);
    bind_shader_samplers("PS", metal_pixel_shader, smp_entries_pixel);

    for (uint32_t i = 0; i < textures_for_encoder_count; ++i) {
      UseRenderEncoderResource(textures_for_encoder[i], MTL::ResourceUsageRead);
    }

    UseRenderEncoderResource(null_buffer_, MTL::ResourceUsageRead);
    UseRenderEncoderResource(null_texture_, MTL::ResourceUsageRead);
    UseRenderEncoderResource(res_heap_ab_, MTL::ResourceUsageRead);
    UseRenderEncoderResource(smp_heap_ab_, MTL::ResourceUsageRead);
    UseRenderEncoderResource(top_level_ab_, MTL::ResourceUsageRead);
    UseRenderEncoderResource(cbv_heap_ab_, MTL::ResourceUsageRead);
    UseRenderEncoderResource(uniforms_buffer_, MTL::ResourceUsageRead);

    auto write_top_level_and_cbvs = [&](size_t table_index,
                                        uint64_t res_table_gpu_base,
                                        uint64_t uav_table_gpu_base,
                                        uint64_t smp_table_gpu_base,
                                        IRDescriptorTableEntry* cbv_entries,
                                        uint64_t uniforms_gpu_base) {
      size_t top_level_offset = table_index * kTopLevelABBytesPerTable;
      auto* top_level_ptrs = reinterpret_cast<uint64_t*>(
          static_cast<uint8_t*>(top_level_ab_->contents()) + top_level_offset);
      std::memset(top_level_ptrs, 0, kTopLevelABBytesPerTable);

      for (int i = 0; i < 4; ++i) {
        top_level_ptrs[i] = res_table_gpu_base;
      }
      top_level_ptrs[4] = res_table_gpu_base;
      for (int i = 5; i < 9; ++i) {
        top_level_ptrs[i] = uav_table_gpu_base;
      }
      top_level_ptrs[9] = smp_table_gpu_base;

      IRDescriptorTableSetBuffer(&cbv_entries[0],
                                 uniforms_gpu_base + 0 * kCbvSizeBytes,
                                 kCbvSizeBytes);
      IRDescriptorTableSetBuffer(&cbv_entries[1],
                                 uniforms_gpu_base + 1 * kCbvSizeBytes,
                                 kCbvSizeBytes);
      IRDescriptorTableSetBuffer(&cbv_entries[2],
                                 uniforms_gpu_base + 2 * kCbvSizeBytes,
                                 kCbvSizeBytes);
      IRDescriptorTableSetBuffer(&cbv_entries[3],
                                 uniforms_gpu_base + 3 * kCbvSizeBytes,
                                 kCbvSizeBytes);
      IRDescriptorTableSetBuffer(&cbv_entries[4],
                                 uniforms_gpu_base + 4 * kCbvSizeBytes,
                                 kCbvSizeBytes);
      IRDescriptorTableSetBuffer(&cbv_entries[5], null_buffer_->gpuAddress(),
                                 kCbvSizeBytes);
      IRDescriptorTableSetBuffer(&cbv_entries[6], null_buffer_->gpuAddress(),
                                 kCbvSizeBytes);

      uint64_t cbv_table_gpu_base =
          cbv_heap_ab_->gpuAddress() +
          table_index * kCbvHeapSlotsPerTable * sizeof(IRDescriptorTableEntry);
      top_level_ptrs[10] = cbv_table_gpu_base;
      top_level_ptrs[11] = cbv_table_gpu_base;
      top_level_ptrs[12] = cbv_table_gpu_base;
      top_level_ptrs[13] = cbv_table_gpu_base;
    };

    write_top_level_and_cbvs(table_index_vertex, res_heap_gpu_base_vertex,
                             uav_heap_gpu_base_vertex, smp_heap_gpu_base_vertex,
                             cbv_entries_vertex, uniforms_gpu_base_vertex);
    write_top_level_and_cbvs(table_index_pixel, res_heap_gpu_base_pixel,
                             uav_heap_gpu_base_pixel, smp_heap_gpu_base_pixel,
                             cbv_entries_pixel, uniforms_gpu_base_pixel);

    if (use_geometry_emulation || use_tessellation_emulation) {
      current_render_encoder_->setObjectBuffer(
          top_level_ab_, table_index_vertex * kTopLevelABBytesPerTable,
          kIRArgumentBufferBindPoint);
      current_render_encoder_->setMeshBuffer(
          top_level_ab_, table_index_vertex * kTopLevelABBytesPerTable,
          kIRArgumentBufferBindPoint);
      current_render_encoder_->setFragmentBuffer(
          top_level_ab_, table_index_pixel * kTopLevelABBytesPerTable,
          kIRArgumentBufferBindPoint);

      if (use_tessellation_emulation) {
        current_render_encoder_->setObjectBuffer(
            top_level_ab_, table_index_vertex * kTopLevelABBytesPerTable,
            kIRArgumentBufferHullDomainBindPoint);
        current_render_encoder_->setMeshBuffer(
            top_level_ab_, table_index_vertex * kTopLevelABBytesPerTable,
            kIRArgumentBufferHullDomainBindPoint);
      }

      current_render_encoder_->setObjectBuffer(res_heap_ab_, 0,
                                               kIRDescriptorHeapBindPoint);
      current_render_encoder_->setMeshBuffer(res_heap_ab_, 0,
                                             kIRDescriptorHeapBindPoint);
      current_render_encoder_->setFragmentBuffer(res_heap_ab_, 0,
                                                 kIRDescriptorHeapBindPoint);
      current_render_encoder_->setObjectBuffer(smp_heap_ab_, 0,
                                               kIRSamplerHeapBindPoint);
      current_render_encoder_->setMeshBuffer(smp_heap_ab_, 0,
                                             kIRSamplerHeapBindPoint);
      current_render_encoder_->setFragmentBuffer(smp_heap_ab_, 0,
                                                 kIRSamplerHeapBindPoint);
    } else {
      current_render_encoder_->setVertexBuffer(
          top_level_ab_, table_index_vertex * kTopLevelABBytesPerTable,
          kIRArgumentBufferBindPoint);
      current_render_encoder_->setFragmentBuffer(
          top_level_ab_, table_index_pixel * kTopLevelABBytesPerTable,
          kIRArgumentBufferBindPoint);

      current_render_encoder_->setVertexBuffer(res_heap_ab_, 0,
                                               kIRDescriptorHeapBindPoint);
      current_render_encoder_->setFragmentBuffer(res_heap_ab_, 0,
                                                 kIRDescriptorHeapBindPoint);
      current_render_encoder_->setVertexBuffer(smp_heap_ab_, 0,
                                               kIRSamplerHeapBindPoint);
      current_render_encoder_->setFragmentBuffer(smp_heap_ab_, 0,
                                                 kIRSamplerHeapBindPoint);
    }
  }

  // Bind vertex buffers / descriptors.
  if (use_geometry_emulation || use_tessellation_emulation) {
    IRRuntimeVertexBuffers vertex_buffers = {};
    MTL::Buffer* shared_mem_buffer =
        shared_memory_ ? shared_memory_->GetBuffer() : nullptr;
    if (shared_mem_buffer) {
      UseRenderEncoderResource(shared_mem_buffer, shared_memory_usage);
      for (uint32_t i = 0; i < vertex_range_count; ++i) {
        const auto& range = vertex_ranges[i];
        size_t binding_index = range.binding_index;
        if (binding_index <
            (sizeof(vertex_buffers) / sizeof(vertex_buffers[0]))) {
          vertex_buffers[binding_index].addr =
              shared_mem_buffer->gpuAddress() + range.offset;
          vertex_buffers[binding_index].length = range.length;
          vertex_buffers[binding_index].stride = range.stride;
        }
      }
    }
    // MSC manual: bind IRRuntimeVertexBuffers at kIRVertexBufferBindPoint (6)
    // for the object stage when using geometry emulation.
    current_render_encoder_->setObjectBytes(
        vertex_buffers, sizeof(vertex_buffers), kIRVertexBufferBindPoint);
  } else if (uses_vertex_fetch) {
    // Vertex fetch shaders read directly from shared memory via SRV, so avoid
    // stage-in bindings that can trigger invalid buffer loads.
    if (shared_memory_) {
      if (MTL::Buffer* shared_mem_buffer = shared_memory_->GetBuffer()) {
        UseRenderEncoderResource(shared_mem_buffer, shared_memory_usage);
      }
    }
  } else {
    // Bind vertex buffers at kIRVertexBufferBindPoint (index 6+) for stage-in.
    // The pipeline's vertex descriptor expects buffers at these indices,
    // populated from the vertex fetch constants. The buffer addresses come from
    // shared memory.
    if (shared_memory_ && !vb_bindings.empty()) {
      MTL::Buffer* shared_mem_buffer = shared_memory_->GetBuffer();
      if (shared_mem_buffer) {
        // Mark shared memory as used for reading
        UseRenderEncoderResource(shared_mem_buffer, shared_memory_usage);

        // Bind vertex buffers for each binding
        for (uint32_t i = 0; i < vertex_range_count; ++i) {
          const auto& range = vertex_ranges[i];
          uint64_t buffer_index =
              kIRVertexBufferBindPoint + uint64_t(range.binding_index);
          current_render_encoder_->setVertexBuffer(shared_mem_buffer,
                                                   range.offset, buffer_index);
        }
      }
    } else if (shared_memory_) {
      // No vertex bindings, but still mark shared memory as resident
      if (MTL::Buffer* shared_mem_buffer = shared_memory_->GetBuffer()) {
        UseRenderEncoderResource(shared_mem_buffer, shared_memory_usage);
      }
    }
  }

  auto request_guest_index_range = [&](uint64_t index_base,
                                       uint32_t index_count,
                                       MTL::IndexType index_type) -> bool {
    if (!shared_memory_) {
      return false;
    }
    uint32_t index_stride = (index_type == MTL::IndexTypeUInt16)
                                ? sizeof(uint16_t)
                                : sizeof(uint32_t);
    uint64_t index_length = uint64_t(index_count) * index_stride;
    if (index_base > SharedMemory::kBufferSize ||
        SharedMemory::kBufferSize - index_base < index_length) {
      XELOGW(
          "Index buffer range out of bounds (base=0x{:08X} size={} count={})",
          static_cast<uint32_t>(index_base), index_length, index_count);
      return false;
    }
    return shared_memory_->RequestRangeForGpuRead(
        static_cast<uint32_t>(index_base), static_cast<uint32_t>(index_length));
  };

  if (use_tessellation_emulation) {
    IRRuntimePrimitiveType tess_primitive = IRRuntimePrimitiveTypeTriangle;
    switch (primitive_processing_result.host_primitive_type) {
      case xenos::PrimitiveType::kTriangleList:
        tess_primitive = IRRuntimePrimitiveType3ControlPointPatchlist;
        break;
      case xenos::PrimitiveType::kQuadList:
        tess_primitive = IRRuntimePrimitiveType4ControlPointPatchlist;
        break;
      case xenos::PrimitiveType::kTrianglePatch:
        tess_primitive = (primitive_processing_result.tessellation_mode ==
                          xenos::TessellationMode::kAdaptive)
                             ? IRRuntimePrimitiveType3ControlPointPatchlist
                             : IRRuntimePrimitiveType1ControlPointPatchlist;
        break;
      case xenos::PrimitiveType::kQuadPatch:
        tess_primitive = (primitive_processing_result.tessellation_mode ==
                          xenos::TessellationMode::kAdaptive)
                             ? IRRuntimePrimitiveType4ControlPointPatchlist
                             : IRRuntimePrimitiveType1ControlPointPatchlist;
        break;
      default:
        XELOGE(
            "Host tessellated primitive type {} returned by the primitive "
            "processor is not supported by the Metal tessellation path",
            uint32_t(primitive_processing_result.host_primitive_type));
        return false;
    }

    const IRRuntimeTessellationPipelineConfig& tess_config =
        tessellation_pipeline_state->config;

    if (primitive_processing_result.index_buffer_type ==
        PrimitiveProcessor::ProcessedIndexBufferType::kNone) {
      IRRuntimeDrawPatchesTessellationEmulation(
          current_render_encoder_, tess_primitive, tess_config, 1,
          primitive_processing_result.host_draw_vertex_count, 0, 0);
    } else {
      MTL::IndexType index_type =
          (primitive_processing_result.host_index_format ==
           xenos::IndexFormat::kInt16)
              ? MTL::IndexTypeUInt16
              : MTL::IndexTypeUInt32;
      MTL::Buffer* index_buffer = nullptr;
      uint64_t index_offset = 0;
      switch (primitive_processing_result.index_buffer_type) {
        case PrimitiveProcessor::ProcessedIndexBufferType::kGuestDMA:
          index_buffer = shared_memory_ ? shared_memory_->GetBuffer() : nullptr;
          index_offset = primitive_processing_result.guest_index_base;
          if (!request_guest_index_range(
                  index_offset,
                  primitive_processing_result.host_draw_vertex_count,
                  index_type)) {
            XELOGE(
                "IssueDraw: failed to validate guest index buffer range for "
                "tessellation");
            return false;
          }
          break;
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostConverted:
          if (primitive_processor_) {
            index_buffer = primitive_processor_->GetConvertedIndexBuffer(
                primitive_processing_result.host_index_buffer_handle,
                index_offset);
          }
          break;
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostBuiltinForAuto:
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostBuiltinForDMA:
          if (primitive_processor_) {
            index_buffer = primitive_processor_->GetBuiltinIndexBuffer();
            index_offset = primitive_processing_result.host_index_buffer_handle;
          }
          break;
        default:
          XELOGE("Unsupported index buffer type {} for tessellation",
                 uint32_t(primitive_processing_result.index_buffer_type));
          return false;
      }
      if (!index_buffer) {
        XELOGE("IssueDraw: index buffer is null for tessellation");
        return false;
      }
      UseRenderEncoderResource(index_buffer, MTL::ResourceUsageRead);
      uint32_t index_stride = (index_type == MTL::IndexTypeUInt16)
                                  ? sizeof(uint16_t)
                                  : sizeof(uint32_t);
      uint32_t start_index =
          index_stride ? uint32_t(index_offset / index_stride) : 0;
      IRRuntimeDrawIndexedPatchesTessellationEmulation(
          current_render_encoder_, tess_primitive, index_type, index_buffer,
          tess_config, 1, primitive_processing_result.host_draw_vertex_count, 0,
          0, start_index);
    }
  } else if (use_geometry_emulation) {
    IRRuntimePrimitiveType geometry_primitive = IRRuntimePrimitiveTypeTriangle;
    switch (primitive_processing_result.host_primitive_type) {
      case xenos::PrimitiveType::kPointList:
        geometry_primitive = IRRuntimePrimitiveTypePoint;
        break;
      case xenos::PrimitiveType::kRectangleList:
        geometry_primitive = IRRuntimePrimitiveTypeTriangle;
        break;
      case xenos::PrimitiveType::kQuadList:
        geometry_primitive = IRRuntimePrimitiveTypeLineWithAdj;
        break;
      default:
        XELOGE(
            "Host primitive type {} returned by the primitive processor is not "
            "supported by the Metal geometry path",
            uint32_t(primitive_processing_result.host_primitive_type));
        return false;
    }

    IRRuntimeGeometryPipelineConfig geometry_config = {};
    geometry_config.gsVertexSizeInBytes =
        geometry_pipeline_state->gs_vertex_size_in_bytes;
    geometry_config.gsMaxInputPrimitivesPerMeshThreadgroup =
        geometry_pipeline_state->gs_max_input_primitives_per_mesh_threadgroup;

    if (primitive_processing_result.index_buffer_type ==
        PrimitiveProcessor::ProcessedIndexBufferType::kNone) {
      IRRuntimeDrawPrimitivesGeometryEmulation(
          current_render_encoder_, geometry_primitive, geometry_config, 1,
          primitive_processing_result.host_draw_vertex_count, 0, 0);
    } else {
      MTL::IndexType index_type =
          (primitive_processing_result.host_index_format ==
           xenos::IndexFormat::kInt16)
              ? MTL::IndexTypeUInt16
              : MTL::IndexTypeUInt32;
      MTL::Buffer* index_buffer = nullptr;
      uint64_t index_offset = 0;
      switch (primitive_processing_result.index_buffer_type) {
        case PrimitiveProcessor::ProcessedIndexBufferType::kGuestDMA:
          index_buffer = shared_memory_ ? shared_memory_->GetBuffer() : nullptr;
          index_offset = primitive_processing_result.guest_index_base;
          if (!request_guest_index_range(
                  index_offset,
                  primitive_processing_result.host_draw_vertex_count,
                  index_type)) {
            XELOGE("IssueDraw: failed to validate guest index buffer range");
            return false;
          }
          break;
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostConverted:
          if (primitive_processor_) {
            index_buffer = primitive_processor_->GetConvertedIndexBuffer(
                primitive_processing_result.host_index_buffer_handle,
                index_offset);
          }
          break;
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostBuiltinForAuto:
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostBuiltinForDMA:
          if (primitive_processor_) {
            index_buffer = primitive_processor_->GetBuiltinIndexBuffer();
            index_offset = primitive_processing_result.host_index_buffer_handle;
          }
          break;
        default:
          XELOGE("Unsupported index buffer type {}",
                 uint32_t(primitive_processing_result.index_buffer_type));
          return false;
      }
      if (!index_buffer) {
        XELOGE("IssueDraw: index buffer is null for type {}",
               uint32_t(primitive_processing_result.index_buffer_type));
        return false;
      }
      UseRenderEncoderResource(index_buffer, MTL::ResourceUsageRead);
      uint32_t index_stride = (index_type == MTL::IndexTypeUInt16)
                                  ? sizeof(uint16_t)
                                  : sizeof(uint32_t);
      uint32_t start_index =
          index_stride ? uint32_t(index_offset / index_stride) : 0;
      IRRuntimeDrawIndexedPrimitivesGeometryEmulation(
          current_render_encoder_, geometry_primitive, index_type, index_buffer,
          geometry_config, 1,
          primitive_processing_result.host_draw_vertex_count, start_index, 0,
          0);
    }
  } else {
    // Primitive topology - from primitive processor, like D3D12.
    MTL::PrimitiveType mtl_primitive = MTL::PrimitiveTypeTriangle;
    switch (primitive_processing_result.host_primitive_type) {
      case xenos::PrimitiveType::kPointList:
        mtl_primitive = MTL::PrimitiveTypePoint;
        break;
      case xenos::PrimitiveType::kLineList:
        mtl_primitive = MTL::PrimitiveTypeLine;
        break;
      case xenos::PrimitiveType::kLineStrip:
        mtl_primitive = MTL::PrimitiveTypeLineStrip;
        break;
      case xenos::PrimitiveType::kTriangleList:
      case xenos::PrimitiveType::kRectangleList:
        mtl_primitive = MTL::PrimitiveTypeTriangle;
        break;
      case xenos::PrimitiveType::kTriangleStrip:
        mtl_primitive = MTL::PrimitiveTypeTriangleStrip;
        break;
      default:
        XELOGE(
            "Host primitive type {} returned by the primitive processor is not "
            "supported by the Metal command processor",
            uint32_t(primitive_processing_result.host_primitive_type));
        return false;
    }

    // Draw using primitive processor output.
    if (primitive_processing_result.index_buffer_type ==
        PrimitiveProcessor::ProcessedIndexBufferType::kNone) {
      IRRuntimeDrawPrimitives(
          current_render_encoder_, mtl_primitive, NS::UInteger(0),
          NS::UInteger(primitive_processing_result.host_draw_vertex_count));
    } else {
      MTL::IndexType index_type =
          (primitive_processing_result.host_index_format ==
           xenos::IndexFormat::kInt16)
              ? MTL::IndexTypeUInt16
              : MTL::IndexTypeUInt32;
      MTL::Buffer* index_buffer = nullptr;
      uint64_t index_offset = 0;
      switch (primitive_processing_result.index_buffer_type) {
        case PrimitiveProcessor::ProcessedIndexBufferType::kGuestDMA:
          index_buffer = shared_memory_ ? shared_memory_->GetBuffer() : nullptr;
          index_offset = primitive_processing_result.guest_index_base;
          if (!request_guest_index_range(
                  index_offset,
                  primitive_processing_result.host_draw_vertex_count,
                  index_type)) {
            XELOGE("IssueDraw: failed to validate guest index buffer range");
            return false;
          }
          break;
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostConverted:
          if (primitive_processor_) {
            index_buffer = primitive_processor_->GetConvertedIndexBuffer(
                primitive_processing_result.host_index_buffer_handle,
                index_offset);
          }
          break;
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostBuiltinForAuto:
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostBuiltinForDMA:
          if (primitive_processor_) {
            index_buffer = primitive_processor_->GetBuiltinIndexBuffer();
            index_offset = primitive_processing_result.host_index_buffer_handle;
          }
          break;
        default:
          XELOGE("Unsupported index buffer type {}",
                 uint32_t(primitive_processing_result.index_buffer_type));
          return false;
      }
      if (!index_buffer) {
        XELOGE("IssueDraw: index buffer is null for type {}",
               uint32_t(primitive_processing_result.index_buffer_type));
        return false;
      }
      IRRuntimeDrawIndexedPrimitives(
          current_render_encoder_, mtl_primitive,
          NS::UInteger(primitive_processing_result.host_draw_vertex_count),
          index_type, index_buffer, index_offset, NS::UInteger(1), 0, 0);
    }
  }

  if (memexport_used && shared_memory_) {
    for (const draw_util::MemExportRange& memexport_range : memexport_ranges_) {
      shared_memory_->RangeWrittenByGpu(
          memexport_range.base_address_dwords << 2, memexport_range.size_bytes);
    }
  }

  // Advance ring-buffer indices for descriptor and argument buffers.
  ++current_draw_index_;

  return true;
#else   // !METAL_SHADER_CONVERTER_AVAILABLE
  XELOGE("MSC draw path not available");
  return false;
#endif  // METAL_SHADER_CONVERTER_AVAILABLE
}

// ==========================================================================
// SPIRV-Cross (MSL) draw path
// ==========================================================================
bool MetalCommandProcessor::DrawReadsPendingResolveOutput(
    uint32_t used_texture_mask, const RegisterFile& regs) {
  // With metal_resolve_keep_command_buffer the command buffer stays open after
  // resolves, so a texture to be loaded from what a resolve in it wrote must
  // be loaded in it, after the resolve: the render pass is ended for the load
  // (texture uploads go into the current command buffer when no render
  // encoder is open) and begun again, its attachments loaded. Only the slots
  // whose texture would actually be loaded (new, or outdated by a write such
  // as a resolve) are checked; a texture already loaded since the resolve,
  // only bound again, needs no new pass. Checking every used slot whose
  // memory had ever been resolved this frame restarted the pass at nearly
  // every such draw (GTA IV: 150-390 passes a frame instead of ~60).
  const bool resolve_keep_command_buffer =
      cvars::metal_resolve_keep_command_buffer &&
      cvars::metal_texture_upload_via_blit;
  const uint32_t resolve_check_mask =
      !copy_resolve_writes_pending_  ? 0
      : !resolve_keep_command_buffer ? used_texture_mask
      : texture_cache_
          ? texture_cache_->GetUsedTexturePotentialLoadMask(used_texture_mask)
          : 0;
  uint32_t remaining_fetch_bits = resolve_check_mask;
  uint32_t fetch_index = 0;
  while (xe::bit_scan_forward(remaining_fetch_bits, &fetch_index)) {
    remaining_fetch_bits &= ~(uint32_t(1) << fetch_index);
    xenos::xe_gpu_texture_fetch_t fetch = regs.GetTextureFetch(fetch_index);
    uint32_t width_minus_1 = 0;
    uint32_t height_minus_1 = 0;
    uint32_t depth_or_array_size_minus_1 = 0;
    uint32_t base_page = 0;
    uint32_t mip_page = 0;
    uint32_t mip_max_level = 0;
    texture_util::GetSubresourcesFromFetchConstant(
        fetch, &width_minus_1, &height_minus_1, &depth_or_array_size_minus_1,
        &base_page, &mip_page, nullptr, &mip_max_level);
    if (!base_page && !mip_page) {
      continue;
    }
    auto layout = texture_util::GetGuestTextureLayout(
        fetch.dimension, fetch.pitch, width_minus_1 + 1, height_minus_1 + 1,
        depth_or_array_size_minus_1 + 1, fetch.tiled, fetch.format,
        fetch.packed_mips, true, mip_max_level);
    const uint32_t base_size = layout.base.level_data_extent_bytes;
    const uint32_t mip_size = layout.mips_total_extent_bytes;
    if (base_page && base_size &&
        IsResolvedMemory(base_page << 12, base_size)) {
      return true;
    }
    if (mip_page && mip_size && IsResolvedMemory(mip_page << 12, mip_size)) {
      return true;
    }
  }
  return false;
}

void MetalCommandProcessor::SplitForPendingResolveOutput(
    uint32_t used_texture_mask) {
  if (cvars::metal_resolve_keep_command_buffer &&
      cvars::metal_texture_upload_via_blit) {
    EndRenderEncoder();
    if (texture_cache_) {
      texture_cache_->RequestTextures(used_texture_mask);
    }
  } else {
    EndCommandBuffer();
  }
}

bool MetalCommandProcessor::IssueDrawMsl(
    Shader* vertex_shader, Shader* pixel_shader,
    const PrimitiveProcessor::ProcessingResult& primitive_processing_result,
    bool primitive_polygonal, bool is_rasterization_done, bool memexport_used,
    uint32_t normalized_color_mask, const RegisterFile& regs) {
  assert_not_null(vertex_shader);
  // Cast to MslShader for the SPIRV-Cross path.
  auto* msl_vertex_shader = static_cast<MslShader*>(vertex_shader);
  auto* msl_pixel_shader = static_cast<MslShader*>(pixel_shader);

  // Tessellation draws use Metal's native tessellation: tessellation factors
  // are computed on the CPU and the domain shader (TES) runs as the
  // post-tessellation vertex function.
  const bool is_tessellated = primitive_processing_result.IsTessellated();

  // Determine the host vertex shader type for geometry expansion.
  // The primitive processor has already set kPointListAsTriangleStrip or
  // kRectangleListAsTriangleStrip when VS expansion is needed (enabled by
  // setting point_sprites_supported_without_vs_expansion = false in the
  // primitive processor init when spirvcross is active).
  Shader::HostVertexShaderType host_vertex_shader_type =
      primitive_processing_result.host_vertex_shader_type;

  // Compute interpolator mask for shader modifications.
  uint32_t ps_param_gen_pos = UINT32_MAX;
  uint32_t interpolator_mask = 0;
  if (msl_pixel_shader) {
    interpolator_mask = msl_vertex_shader->writes_interpolators() &
                        msl_pixel_shader->GetInterpolatorInputMask(
                            regs.Get<reg::SQ_PROGRAM_CNTL>(),
                            regs.Get<reg::SQ_CONTEXT_MISC>(), ps_param_gen_pos);
  }

  auto normalized_depth_control = draw_util::GetNormalizedDepthControl(regs);

  // Compute SPIRV shader modifications.
  SpirvShaderTranslator::Modification vertex_shader_modification =
      GetCurrentSpirvVertexShaderModification(
          *msl_vertex_shader, host_vertex_shader_type, interpolator_mask);
  SpirvShaderTranslator::Modification pixel_shader_modification =
      msl_pixel_shader
          ? GetCurrentSpirvPixelShaderModification(
                *msl_pixel_shader, interpolator_mask, ps_param_gen_pos,
                normalized_depth_control, normalized_color_mask)
          : SpirvShaderTranslator::Modification(0);

  // Sanity: if a pixel shader writes color targets and any RT is enabled,
  // color_targets_used must be non-zero or fragments will produce no output.
  if (msl_pixel_shader && msl_pixel_shader->writes_color_targets() &&
      normalized_color_mask) {
    assert_not_zero(pixel_shader_modification.pixel.color_targets_used);
  }

  // Get or create shader translations.
  constexpr bool kIsIos =
#if XE_PLATFORM_IOS
      true;
#else
      false;
#endif
  auto* vertex_translation = static_cast<MslShader::MslTranslation*>(
      msl_vertex_shader->GetOrCreateTranslation(
          vertex_shader_modification.value));
  if (!vertex_translation->is_translated()) {
    if (!spirv_shader_translator_->TranslateAnalyzedShader(
            *vertex_translation)) {
      XELOGE("SPIRV-Cross: Failed to translate vertex shader to SPIR-V");
      return false;
    }
  }
  auto ensure_msl_translation_ready =
      [&](MslShader::MslTranslation* translation,
          uint8_t priority) -> MslShaderCompileStatus {
    if (!translation) {
      return MslShaderCompileStatus::kFailed;
    }
    MslShaderCompileStatus status = GetMslShaderCompileStatus(translation);
    if (status == MslShaderCompileStatus::kPending) {
      // Still needed: queued again as the latest request if it was queued in
      // an earlier frame.
      EnqueueMslShaderCompilation(translation, kIsIos, priority);
      return status;
    }
    if (status == MslShaderCompileStatus::kReady ||
        status == MslShaderCompileStatus::kFailed) {
      return status;
    }
    if (EnqueueMslShaderCompilation(translation, kIsIos, priority)) {
      return MslShaderCompileStatus::kPending;
    }
    if (!translation->CompileToMsl(device_, kIsIos)) {
      std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
      msl_shader_compile_failed_.insert(translation);
      return MslShaderCompileStatus::kFailed;
    }
    return MslShaderCompileStatus::kReady;
  };
  auto log_pending_compile = [&](MslShader::MslTranslation* translation,
                                 const char* stage_tag) {
    static auto last_pending_log = std::chrono::steady_clock::time_point{};
    const auto now = std::chrono::steady_clock::now();
    if (now - last_pending_log < std::chrono::seconds(1)) {
      return;
    }
    last_pending_log = now;
    XELOGI(
        "SPIRV-Cross: Skipping draw - {} shader compile pending "
        "(shader={:016X}, mod={:016X})",
        stage_tag, translation->shader().ucode_data_hash(),
        translation->modification());
  };
  MslShaderCompileStatus vertex_compile_status =
      ensure_msl_translation_ready(vertex_translation, 1);
  while (vertex_compile_status == MslShaderCompileStatus::kPending &&
         WaitForMslCompileProgress()) {
    vertex_compile_status = GetMslShaderCompileStatus(vertex_translation);
  }
  if (vertex_compile_status == MslShaderCompileStatus::kPending) {
    log_pending_compile(vertex_translation, "vertex");
    metal_compile_stall_tracker.Skipped(
        {1, vertex_translation->shader().ucode_data_hash(),
         vertex_translation->modification(), 0, 0});
    ++draw_stats_.compile_pending;
    return true;
  }
  if (vertex_compile_status == MslShaderCompileStatus::kFailed) {
    XELOGE("SPIRV-Cross: Failed to prepare vertex shader MSL/library/function");
    return false;
  }

  MslShader::MslTranslation* pixel_translation = nullptr;
  if (msl_pixel_shader) {
    pixel_translation = static_cast<MslShader::MslTranslation*>(
        msl_pixel_shader->GetOrCreateTranslation(
            pixel_shader_modification.value));
    if (!pixel_translation->is_translated()) {
      if (!spirv_shader_translator_->TranslateAnalyzedShader(
              *pixel_translation)) {
        XELOGE("SPIRV-Cross: Failed to translate pixel shader to SPIR-V");
        return false;
      }
    }
    MslShaderCompileStatus pixel_compile_status =
        ensure_msl_translation_ready(pixel_translation, 2);
    while (pixel_compile_status == MslShaderCompileStatus::kPending &&
           WaitForMslCompileProgress()) {
      pixel_compile_status = GetMslShaderCompileStatus(pixel_translation);
    }
    if (pixel_compile_status == MslShaderCompileStatus::kPending) {
      log_pending_compile(pixel_translation, "pixel");
      metal_compile_stall_tracker.Skipped(
          {2, pixel_translation->shader().ucode_data_hash(),
           pixel_translation->modification(), 0, 0});
      ++draw_stats_.compile_pending;
      return true;
    }
    if (pixel_compile_status == MslShaderCompileStatus::kFailed) {
      XELOGE(
          "SPIRV-Cross: Failed to prepare pixel shader MSL/library/function");
      return false;
    }
  }

  // metal_shader_compiler "spirv2air": a pipeline takes all its shaders from
  // spirv2air or all from the Metal Shading Language, as their stage
  // interfaces differ, and tessellated draws only the latter. A shader that
  // only has a spirv2air function gets the other one here, at once, where it
  // goes with one that has none, or where Metal rejects the spirv2air
  // pipeline.
  bool use_spirv2air =
      !is_tessellated && vertex_translation->spirv2air_function() &&
      (!pixel_translation || pixel_translation->spirv2air_function());
  auto compile_msl_for_draw = [&]() -> bool {
    for (MslShader::MslTranslation* translation :
         {vertex_translation, pixel_translation}) {
      if (!translation || translation->metal_function()) {
        continue;
      }
      static std::atomic<uint32_t> spirv2air_msl_fallbacks{0};
      if (spirv2air_msl_fallbacks.fetch_add(1, std::memory_order_relaxed) <
          16) {
        XELOGI(
            "spirv2air: compiling {} shader {:016X} mod {:016X} with the "
            "Metal Shading Language as well, for a draw that can't use "
            "spirv2air",
            translation->shader().type() == xenos::ShaderType::kVertex
                ? "vertex"
                : "pixel",
            translation->shader().ucode_data_hash(),
            translation->modification());
      }
      if (!translation->CompileToMsl(device_, kIsIos)) {
        XELOGE("SPIRV-Cross: Failed to prepare shader MSL/library/function");
        return false;
      }
    }
    return true;
  };
  if (!use_spirv2air && !compile_msl_for_draw()) {
    return false;
  }

  // Create or retrieve pipeline state.
  MTL::RenderPipelineState* pipeline = nullptr;
  MslPipelineCompileStatus pipeline_compile_status =
      MslPipelineCompileStatus::kReady;
  if (is_tessellated) {
    pipeline = GetOrCreateMslTessPipelineState(
        vertex_translation, pixel_translation, host_vertex_shader_type, regs);
  } else {
    for (;;) {
      pipeline = GetOrCreateMslPipelineState(
          vertex_translation, pixel_translation, regs, use_spirv2air,
          &pipeline_compile_status);
      while (!pipeline &&
             pipeline_compile_status == MslPipelineCompileStatus::kPending &&
             WaitForMslCompileProgress()) {
        pipeline = GetOrCreateMslPipelineState(
            vertex_translation, pixel_translation, regs, use_spirv2air,
            &pipeline_compile_status);
      }
      if (pipeline || !use_spirv2air ||
          pipeline_compile_status != MslPipelineCompileStatus::kFailed) {
        break;
      }
      // Metal rejected the spirv2air pipeline (the error is logged once per
      // pipeline): the draw takes the Metal Shading Language shaders.
      DumpSpirv2airPipelineShaders(*vertex_translation, pixel_translation);
      use_spirv2air = false;
      if (!compile_msl_for_draw()) {
        return false;
      }
    }
  }
  if (!pipeline) {
    if (!is_tessellated &&
        pipeline_compile_status == MslPipelineCompileStatus::kPending) {
      if (ShouldLogRateLimited(msl_pipeline_pending_last_log_ns_,
                               kMslAsyncLogIntervalNs)) {
        XELOGI(
            "SPIRV-Cross: Skipping draw - render pipeline compile pending "
            "(VS {:016X} mod {:016X}, PS {:016X} mod {:016X})",
            vertex_translation->shader().ucode_data_hash(),
            vertex_translation->modification(),
            pixel_translation ? pixel_translation->shader().ucode_data_hash()
                              : 0,
            pixel_translation ? pixel_translation->modification() : 0);
      }
      metal_compile_stall_tracker.Skipped(
          {3, vertex_translation->shader().ucode_data_hash(),
           vertex_translation->modification(),
           pixel_translation ? pixel_translation->shader().ucode_data_hash()
                             : 0,
           pixel_translation ? pixel_translation->modification() : 0});
      ++draw_stats_.compile_pending;
      return true;
    }
    XELOGE("SPIRV-Cross: Failed to create pipeline state");
    return false;
  }

  // Request textures used by the shaders.
  uint32_t used_texture_mask =
      msl_vertex_shader->GetUsedTextureMaskAfterTranslation();
  if (msl_pixel_shader) {
    used_texture_mask |= msl_pixel_shader->GetUsedTextureMaskAfterTranslation();
  }

  if (DrawReadsPendingResolveOutput(used_texture_mask, regs)) {
    SplitForPendingResolveOutput(used_texture_mask);
    BeginCommandBuffer();
    if (!current_command_buffer_ || !current_render_encoder_) {
      XELOGE(
          "SPIRV-Cross: failed to re-begin command buffer for copy->draw "
          "sync split");
      return true;
    }
  }

  if (texture_cache_ && used_texture_mask &&
      texture_cache_->AnyUsedTextureRequestWorkPending(used_texture_mask)) {
    texture_cache_->RequestTextures(used_texture_mask);
  }

  // Ensure shared-memory ranges used by translated shaders are synchronized.
  // The SPIRV-Cross path bypasses the MSC setup path, so it must request
  // vertex fetch and memexport ranges explicitly.
  if (shared_memory_) {
    const Shader::ConstantRegisterMap& constant_map_vertex =
        msl_vertex_shader->constant_register_map();
    for (uint32_t i = 0;
         i < xe::countof(constant_map_vertex.vertex_fetch_bitmap); ++i) {
      uint32_t vfetch_bits_remaining =
          constant_map_vertex.vertex_fetch_bitmap[i];
      uint32_t j;
      while (xe::bit_scan_forward(vfetch_bits_remaining, &j)) {
        vfetch_bits_remaining &= ~(uint32_t(1) << j);
        uint32_t vfetch_index = i * 32 + j;
        xenos::xe_gpu_vertex_fetch_t vfetch = regs.GetVertexFetch(vfetch_index);
        switch (vfetch.type) {
          case xenos::FetchConstantType::kVertex:
            break;
          case xenos::FetchConstantType::kInvalidVertex:
            if (::cvars::gpu_allow_invalid_fetch_constants) {
              break;
            }
            XELOGW(
                "SPIRV-Cross: Vertex fetch constant {} ({:08X} {:08X}) has "
                "\"invalid\" type. Use --gpu_allow_invalid_fetch_constants to "
                "bypass.",
                vfetch_index, vfetch.dword_0, vfetch.dword_1);
            return false;
          default:
            XELOGW(
                "SPIRV-Cross: Vertex fetch constant {} ({:08X} {:08X}) is "
                "invalid.",
                vfetch_index, vfetch.dword_0, vfetch.dword_1);
            return false;
        }
        uint32_t buffer_offset = vfetch.address << 2;
        uint32_t buffer_length = vfetch.size << 2;
        if (buffer_offset > SharedMemory::kBufferSize ||
            SharedMemory::kBufferSize - buffer_offset < buffer_length) {
          XELOGW(
              "SPIRV-Cross: Vertex fetch constant {} out of range "
              "(offset=0x{:08X} size={})",
              vfetch_index, buffer_offset, buffer_length);
          return false;
        }
        if (!shared_memory_->RequestRangeForGpuRead(buffer_offset,
                                                    buffer_length)) {
          XELOGE(
              "SPIRV-Cross: Failed to request vertex buffer at 0x{:08X} "
              "(size {}) in shared memory",
              buffer_offset, buffer_length);
          return false;
        }
        // Zero-copy vertex buffers are read in place, never uploaded, so a
        // trace records them here, once a frame.
        if (trace_writer_.is_open() && shared_memory_->is_zero_copy()) {
          trace_writer_.WriteMemoryReadCached(buffer_offset, buffer_length);
        }
      }
    }

    for (const draw_util::MemExportRange& memexport_range : memexport_ranges_) {
      uint32_t base_bytes = memexport_range.base_address_dwords << 2;
      if (!shared_memory_->RequestRange(base_bytes,
                                        memexport_range.size_bytes)) {
        XELOGE(
            "SPIRV-Cross: Failed to request memexport stream at 0x{:08X} "
            "(size {}) in shared memory",
            base_bytes, memexport_range.size_bytes);
        return false;
      }
    }
  }

  // Viewport info for system constants (same as the MSC path).
  draw_util::ViewportInfo viewport_info;
  constexpr uint32_t kViewportBoundsMax = 32767;
  bool convert_z_to_float24 = ::cvars::depth_float24_convert_in_pixel_shader;
  uint32_t draw_resolution_scale_x =
      texture_cache_ ? texture_cache_->draw_resolution_scale_x() : 1;
  uint32_t draw_resolution_scale_y =
      texture_cache_ ? texture_cache_->draw_resolution_scale_y() : 1;
  // msaa_samples_as_pixels: drawing to the samples of an MSAA surface, scaled
  // by the sample grid.
  bool samples_as_pixels =
      render_target_cache_ &&
      render_target_cache_->last_update_samples_as_pixels_y_log2();
  if (samples_as_pixels) {
    draw_resolution_scale_x <<=
        render_target_cache_->last_update_samples_as_pixels_x_log2();
    draw_resolution_scale_y <<=
        render_target_cache_->last_update_samples_as_pixels_y_log2();
  }
  draw_util::GetViewportInfoArgs gviargs{};
  gviargs.Setup(
      draw_resolution_scale_x, draw_resolution_scale_y,
      samples_as_pixels ? divisors::MagicDiv(draw_resolution_scale_x)
      : texture_cache_  ? texture_cache_->draw_resolution_scale_x_divisor()
                        : divisors::MagicDiv(1),
      samples_as_pixels ? divisors::MagicDiv(draw_resolution_scale_y)
      : texture_cache_  ? texture_cache_->draw_resolution_scale_y_divisor()
                        : divisors::MagicDiv(1),
      true, kViewportBoundsMax, kViewportBoundsMax, false,
      normalized_depth_control, convert_z_to_float24, true,
      msl_pixel_shader && msl_pixel_shader->writes_depth());
  gviargs.SetupRegisterValues(regs);
  draw_util::GetHostViewportInfo(&gviargs, viewport_info);

  // Apply viewport and scissor (matching the MSC path).
  {
    draw_util::Scissor scissor;
    draw_util::GetScissor(regs, scissor);
    scissor.offset[0] *= draw_resolution_scale_x;
    scissor.offset[1] *= draw_resolution_scale_y;
    scissor.extent[0] *= draw_resolution_scale_x;
    scissor.extent[1] *= draw_resolution_scale_y;

    // Clamp scissor to actual render target bounds (Metal requires this).
    uint32_t rt_width = 1;
    uint32_t rt_height = 1;
    GetBoundRenderTargetSize(render_target_cache_.get(), render_target_width_,
                             render_target_height_, rt_width, rt_height);
    // With a limited render area (metal_render_pass_area_from_scissor) the
    // pass is that size, and a scissor reaching past it is invalid: on device
    // such draws (Batman: Arkham Asylum's full-size shadow map depth clears)
    // ended up cut to an earlier, smaller scissor.
    if (current_render_area_width_) {
      rt_width = std::min(rt_width, current_render_area_width_);
      rt_height = std::min(rt_height, current_render_area_height_);
    }
    ClampScissorToBounds(scissor, rt_width, rt_height);

    MTL::Viewport mtl_viewport;
    mtl_viewport.originX = static_cast<double>(viewport_info.xy_offset[0]);
    mtl_viewport.originY = static_cast<double>(viewport_info.xy_offset[1]);
    mtl_viewport.width = static_cast<double>(viewport_info.xy_extent[0]);
    mtl_viewport.height = static_cast<double>(viewport_info.xy_extent[1]);
    mtl_viewport.znear = viewport_info.z_min;
    mtl_viewport.zfar = viewport_info.z_max;
    current_render_encoder_->setViewport(mtl_viewport);

    MTL::ScissorRect mtl_scissor;
    mtl_scissor.x = scissor.offset[0];
    mtl_scissor.y = scissor.offset[1];
    mtl_scissor.width = scissor.extent[0];
    mtl_scissor.height = scissor.extent[1];
    if (!msl_scissor_valid_ || msl_scissor_.x != mtl_scissor.x ||
        msl_scissor_.y != mtl_scissor.y ||
        msl_scissor_.width != mtl_scissor.width ||
        msl_scissor_.height != mtl_scissor.height) {
      current_render_encoder_->setScissorRect(mtl_scissor);
      msl_scissor_ = mtl_scissor;
      msl_scissor_valid_ = true;
    }
    if (metal_perf_counters.frame_detail_frames.load(
            std::memory_order_relaxed)) {
      XELOGI(
          "Frame detail draw {} host: viewport {} {} {}x{} z {} {}, scissor "
          "{} {} {}x{}, render area {}x{}; PA_CL_CLIP_CNTL {:08X} "
          "PA_CL_VTE_CNTL {:08X} PA_SC_SCREEN_SCISSOR {:08X} {:08X} "
          "PA_SC_WINDOW_OFFSET {:08X}",
          frame_detail_draws_ + 1, mtl_viewport.originX, mtl_viewport.originY,
          mtl_viewport.width, mtl_viewport.height, mtl_viewport.znear,
          mtl_viewport.zfar, mtl_scissor.x, mtl_scissor.y, mtl_scissor.width,
          mtl_scissor.height, current_render_area_width_,
          current_render_area_height_, regs[XE_GPU_REG_PA_CL_CLIP_CNTL],
          regs[XE_GPU_REG_PA_CL_VTE_CNTL],
          regs[XE_GPU_REG_PA_SC_SCREEN_SCISSOR_TL],
          regs[XE_GPU_REG_PA_SC_SCREEN_SCISSOR_BR],
          regs[XE_GPU_REG_PA_SC_WINDOW_OFFSET]);
    }
  }

  // Apply fixed-function state.
  if (msl_bound_pipeline_state_ != pipeline) {
    current_render_encoder_->setRenderPipelineState(pipeline);
    msl_bound_pipeline_state_ = pipeline;
  }
  ApplyRasterizerState(primitive_polygonal);
  ApplyDepthStencilState(primitive_polygonal, normalized_depth_control);

  // Update SPIRV system constants.
  UpdateSpirvSystemConstantValues(
      primitive_processing_result, primitive_polygonal,
      primitive_processing_result.line_loop_closing_index,
      primitive_processing_result.host_shader_index_endian, viewport_info,
      used_texture_mask, normalized_depth_control, normalized_color_mask);

  // Blend constants (fixed-function, same as MSC path).
  float blend_constants[] = {
      regs.Get<float>(XE_GPU_REG_RB_BLEND_RED),
      regs.Get<float>(XE_GPU_REG_RB_BLEND_GREEN),
      regs.Get<float>(XE_GPU_REG_RB_BLEND_BLUE),
      regs.Get<float>(XE_GPU_REG_RB_BLEND_ALPHA),
  };
  if (!ff_blend_factor_valid_ ||
      std::memcmp(ff_blend_factor_, blend_constants, sizeof(float) * 4) != 0) {
    std::memcpy(ff_blend_factor_, blend_constants, sizeof(float) * 4);
    ff_blend_factor_valid_ = true;
    current_render_encoder_->setBlendColor(
        blend_constants[0], blend_constants[1], blend_constants[2],
        blend_constants[3]);
  }

  // =====================================================================
  // Resource binding — direct Metal encoder calls, no IRDescriptorTable.
  // =====================================================================
  constexpr uint32_t kCBVSize = MslBufferIndex::kCbvSizeBytes;
  uint32_t ring_index = current_draw_index_ % uint32_t(draw_ring_count_);
  constexpr size_t kStageVertex = 0;
  constexpr size_t kStagePixel = 1;
  size_t table_index_vertex = size_t(ring_index) * kStageCount + kStageVertex;
  size_t table_index_pixel = size_t(ring_index) * kStageCount + kStagePixel;

  // metal_inline_shader_constants: each constant block is passed with
  // set*Bytes when it changed since it was last passed in this render encoder,
  // instead of written to this draw's tables in a pooled uniform buffer.
  // spirv2air shaders read the blocks through pointers, so from the tables.
  const bool inline_constants =
      cvars::metal_inline_shader_constants && !use_spirv2air;
  uint8_t* uniforms_vertex = nullptr;
  uint8_t* uniforms_pixel = nullptr;
  if (!inline_constants) {
    if (!uniforms_buffer_) {
      XELOGE("SPIRV-Cross: no uniforms buffer for the draw's constants");
      return false;
    }
    uint8_t* uniforms_base =
        static_cast<uint8_t*>(uniforms_buffer_->contents());
    uniforms_vertex =
        uniforms_base + table_index_vertex * kUniformsBytesPerTable;
    uniforms_pixel = uniforms_base + table_index_pixel * kUniformsBytesPerTable;
  }
  if (!inline_constants &&
      msl_constants_versioned_uniform_buffer_ != uniforms_buffer_) {
    msl_constants_versioned_uniform_buffer_ = uniforms_buffer_;
    std::fill(msl_system_constants_written_vertex_versions_.begin(),
              msl_system_constants_written_vertex_versions_.end(), uint64_t(0));
    std::fill(msl_system_constants_written_pixel_versions_.begin(),
              msl_system_constants_written_pixel_versions_.end(), uint64_t(0));
    std::fill(msl_clip_plane_constants_written_vertex_versions_.begin(),
              msl_clip_plane_constants_written_vertex_versions_.end(),
              uint64_t(0));
    std::fill(msl_tessellation_constants_written_vertex_versions_.begin(),
              msl_tessellation_constants_written_vertex_versions_.end(),
              uint64_t(0));
    std::fill(msl_tessellation_constants_written_pixel_versions_.begin(),
              msl_tessellation_constants_written_pixel_versions_.end(),
              uint64_t(0));
  }
  auto ensure_uniform_versions_size = [&](std::vector<uint64_t>& versions) {
    if (versions.size() != draw_ring_count_) {
      versions.assign(draw_ring_count_, 0);
    }
  };
  ensure_uniform_versions_size(msl_system_constants_written_vertex_versions_);
  ensure_uniform_versions_size(msl_system_constants_written_pixel_versions_);
  ensure_uniform_versions_size(
      msl_clip_plane_constants_written_vertex_versions_);
  ensure_uniform_versions_size(
      msl_tessellation_constants_written_vertex_versions_);
  ensure_uniform_versions_size(
      msl_tessellation_constants_written_pixel_versions_);
  const size_t ring_index_size = size_t(ring_index);
  auto copy_uniform_block_if_stale =
      [&](uint8_t* dst, const void* src, size_t size,
          std::vector<uint64_t>& written_versions, uint64_t source_version) {
        if (ring_index_size >= written_versions.size()) {
          return;
        }
        if (written_versions[ring_index_size] != source_version) {
          std::memcpy(dst, src, size);
          written_versions[ring_index_size] = source_version;
        }
      };
  // One constant block of a stage: slot in msl_inline_constants_bound_
  // versions_, Metal buffer index, offset in the stage's table, its data and
  // version, and for the table path the per-ring-page versions if the block
  // is only copied when stale.
  auto write_constant_block =
      [&](bool pixel_stage, size_t slot, NS::UInteger buffer_index,
          size_t offset, const void* data, size_t size, uint64_t version,
          std::vector<uint64_t>* written_versions) {
        if (!size) {
          return;
        }
        if (inline_constants) {
          if (msl_inline_constants_bound_versions_[slot] == version) {
            return;
          }
          if (pixel_stage) {
            current_render_encoder_->setFragmentBytes(data, size, buffer_index);
          } else {
            current_render_encoder_->setVertexBytes(data, size, buffer_index);
          }
          msl_inline_constants_bound_versions_[slot] = version;
          return;
        }
        uint8_t* table = pixel_stage ? uniforms_pixel : uniforms_vertex;
        if (written_versions) {
          copy_uniform_block_if_stale(table + offset, data, size,
                                      *written_versions, version);
        } else {
          std::memcpy(table + offset, data, size);
        }
      };

  // b0 (msl_buffer 1): System constants.
  write_constant_block(false, 0, MslBufferIndex::kSystemConstants, 0,
                       &spirv_system_constants_,
                       sizeof(SpirvShaderTranslator::SystemConstants),
                       msl_system_constants_version_,
                       &msl_system_constants_written_vertex_versions_);
  write_constant_block(true, 1, MslBufferIndex::kSystemConstants, 0,
                       &spirv_system_constants_,
                       sizeof(SpirvShaderTranslator::SystemConstants),
                       msl_system_constants_version_,
                       &msl_system_constants_written_pixel_versions_);

  // b1 (msl_buffer 2/3): Float constants.
  // SpirvShaderTranslator uses packed float constants like Vulkan.
  const size_t kFloatConstantOffset = 1 * kCBVSize;
  const Shader::ConstantRegisterMap& float_constant_map_vertex =
      msl_vertex_shader->constant_register_map();
  for (uint32_t i = 0; i < 4; ++i) {
    if (msl_current_float_constant_map_vertex_[i] !=
        float_constant_map_vertex.float_bitmap[i]) {
      msl_current_float_constant_map_vertex_[i] =
          float_constant_map_vertex.float_bitmap[i];
      msl_float_constants_dirty_vertex_ = true;
    }
  }
  if (msl_pixel_shader) {
    const Shader::ConstantRegisterMap& float_constant_map_pixel =
        msl_pixel_shader->constant_register_map();
    for (uint32_t i = 0; i < 4; ++i) {
      if (msl_current_float_constant_map_pixel_[i] !=
          float_constant_map_pixel.float_bitmap[i]) {
        msl_current_float_constant_map_pixel_[i] =
            float_constant_map_pixel.float_bitmap[i];
        msl_float_constants_dirty_pixel_ = true;
      }
    }
  } else {
    for (uint32_t i = 0; i < 4; ++i) {
      if (msl_current_float_constant_map_pixel_[i] != 0) {
        msl_current_float_constant_map_pixel_[i] = 0;
        msl_float_constants_dirty_pixel_ = true;
      }
    }
  }

  auto rebuild_packed_float_constants =
      [&](std::array<uint8_t, kCbvSizeBytes>& dst, const Shader* shader,
          uint32_t regs_base) {
        std::memset(dst.data(), 0, kCBVSize);
        if (!shader) {
          return;
        }
        const Shader::ConstantRegisterMap& map =
            shader->constant_register_map();
        if (!map.float_count) {
          return;
        }
        uint8_t* out = dst.data();
        for (uint32_t i = 0; i < 4; ++i) {
          uint64_t bits = map.float_bitmap[i];
          uint32_t constant_index;
          while (xe::bit_scan_forward(bits, &constant_index)) {
            bits &= ~(uint64_t(1) << constant_index);
            if (out + 4 * sizeof(uint32_t) > dst.data() + kCBVSize) {
              return;
            }
            std::memcpy(
                out, &regs.values[regs_base + (i << 8) + (constant_index << 2)],
                4 * sizeof(uint32_t));
            out += 4 * sizeof(uint32_t);
          }
        }
      };
  if (msl_float_constants_dirty_vertex_) {
    rebuild_packed_float_constants(msl_cached_float_constants_vertex_,
                                   msl_vertex_shader,
                                   XE_GPU_REG_SHADER_CONSTANT_000_X);
    msl_float_constants_dirty_vertex_ = false;
    ++msl_float_constants_version_vertex_;
  }
  if (msl_float_constants_dirty_pixel_) {
    rebuild_packed_float_constants(msl_cached_float_constants_pixel_,
                                   msl_pixel_shader,
                                   XE_GPU_REG_SHADER_CONSTANT_256_X);
    msl_float_constants_dirty_pixel_ = false;
    ++msl_float_constants_version_pixel_;
  }
  // Only the packed constants the shaders read (4 KB each was copied for every
  // draw, ~29 MB per frame at 3600 draws).
  const size_t float_constants_vertex_size = std::min<size_t>(
      kCBVSize, size_t(float_constant_map_vertex.float_count) * 16);
  write_constant_block(
      false, 2, MslBufferIndex::kFloatConstantsVertex, kFloatConstantOffset,
      msl_cached_float_constants_vertex_.data(), float_constants_vertex_size,
      msl_float_constants_version_vertex_, nullptr);
  const size_t float_constants_pixel_size =
      msl_pixel_shader
          ? std::min<size_t>(
                kCBVSize,
                size_t(msl_pixel_shader->constant_register_map().float_count) *
                    16)
          : 0;
  write_constant_block(
      true, 3, MslBufferIndex::kFloatConstantsPixel, kFloatConstantOffset,
      msl_cached_float_constants_pixel_.data(), float_constants_pixel_size,
      msl_float_constants_version_pixel_, nullptr);

  // b2 (msl_buffer 4): Bool/loop constants.
  const size_t kBoolLoopConstantOffset = 2 * kCBVSize;
  constexpr size_t kBoolLoopConstantsSize = (8 + 32) * sizeof(uint32_t);
  if (msl_bool_loop_constants_dirty_) {
    std::memcpy(msl_cached_bool_loop_constants_.data(),
                &regs.values[XE_GPU_REG_SHADER_CONSTANT_BOOL_000_031],
                kBoolLoopConstantsSize);
    msl_bool_loop_constants_dirty_ = false;
    ++msl_bool_loop_constants_version_;
  }
  write_constant_block(
      false, 4, MslBufferIndex::kBoolLoopConstants, kBoolLoopConstantOffset,
      msl_cached_bool_loop_constants_.data(), kBoolLoopConstantsSize,
      msl_bool_loop_constants_version_, nullptr);
  write_constant_block(
      true, 5, MslBufferIndex::kBoolLoopConstants, kBoolLoopConstantOffset,
      msl_cached_bool_loop_constants_.data(), kBoolLoopConstantsSize,
      msl_bool_loop_constants_version_, nullptr);

  // b3 (msl_buffer 5): Fetch constants.
  const size_t kFetchConstantOffset = 3 * kCBVSize;
  const size_t kFetchConstantCount = xenos::kTextureFetchConstantCount * 6;
  const size_t kFetchConstantsSize = kFetchConstantCount * sizeof(uint32_t);
  if (msl_fetch_constants_dirty_) {
    std::memcpy(msl_cached_fetch_constants_.data(),
                &regs.values[XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0],
                kFetchConstantsSize);
    msl_fetch_constants_dirty_ = false;
    ++msl_fetch_constants_version_;
  }
  write_constant_block(false, 6, MslBufferIndex::kFetchConstants,
                       kFetchConstantOffset, msl_cached_fetch_constants_.data(),
                       kFetchConstantsSize, msl_fetch_constants_version_,
                       nullptr);
  write_constant_block(true, 7, MslBufferIndex::kFetchConstants,
                       kFetchConstantOffset, msl_cached_fetch_constants_.data(),
                       kFetchConstantsSize, msl_fetch_constants_version_,
                       nullptr);

  // b5 (msl_buffer 6): Clip plane constants (separate buffer for SPIR-V path).
  const size_t kClipPlaneConstantOffset = 4 * kCBVSize;
  write_constant_block(false, 8, MslBufferIndex::kClipPlaneConstants,
                       kClipPlaneConstantOffset, &spirv_clip_plane_constants_,
                       sizeof(SpirvShaderTranslator::ClipPlaneConstants),
                       msl_clip_plane_constants_version_,
                       &msl_clip_plane_constants_written_vertex_versions_);

  // b6 (msl_buffer 7): Tessellation constants.
  const size_t kTessellationConstantOffset = 5 * kCBVSize;
  write_constant_block(false, 9, MslBufferIndex::kTessellationConstants,
                       kTessellationConstantOffset,
                       &spirv_tessellation_constants_,
                       sizeof(SpirvShaderTranslator::TessellationConstants),
                       msl_tessellation_constants_version_,
                       &msl_tessellation_constants_written_vertex_versions_);
  write_constant_block(true, 10, MslBufferIndex::kTessellationConstants,
                       kTessellationConstantOffset,
                       &spirv_tessellation_constants_,
                       sizeof(SpirvShaderTranslator::TessellationConstants),
                       msl_tessellation_constants_version_,
                       &msl_tessellation_constants_written_pixel_versions_);

  // Keep binding behavior conservative while using per-encoder dedupe caches.
  const bool msl_bind_dedupe = true;

  // Bind shared memory buffer at msl_buffer 0.
  MTL::Buffer* shared_mem_buffer =
      shared_memory_ ? shared_memory_->GetBuffer() : nullptr;
  MTL::ResourceUsage shared_memory_usage = MTL::ResourceUsageRead;
  if (memexport_used) {
    shared_memory_usage |= MTL::ResourceUsageWrite;
  }
  if (!msl_bind_dedupe ||
      msl_bound_shared_memory_buffer_ != shared_mem_buffer) {
    current_render_encoder_->setVertexBuffer(shared_mem_buffer, 0,
                                             MslBufferIndex::kSharedMemory);
    current_render_encoder_->setFragmentBuffer(shared_mem_buffer, 0,
                                               MslBufferIndex::kSharedMemory);
    msl_bound_shared_memory_buffer_ = shared_mem_buffer;
  }
  if (shared_mem_buffer) {
    UseRenderEncoderResource(shared_mem_buffer, shared_memory_usage);
  }

  // Bind a null buffer at the EDRAM slot (msl_buffer 30) as a safety measure.
  // FSI/EDRAM is disabled on this path (fragment_shader_sample_interlock =
  // false), so no shader should reference it, but binding a dummy prevents
  // GPU faults if any code path unexpectedly accesses buffer(30).
  if (!msl_bind_dedupe || msl_bound_null_buffer_ != null_buffer_) {
    current_render_encoder_->setFragmentBuffer(null_buffer_, 0, 30);
    msl_bound_null_buffer_ = null_buffer_;
  }

  // Bind uniforms buffer at the appropriate indices.
  NS::UInteger vs_base_offset = table_index_vertex * kUniformsBytesPerTable;
  NS::UInteger ps_base_offset = table_index_pixel * kUniformsBytesPerTable;
  if (inline_constants) {
    // The set*Bytes bindings replaced the table bindings.
    msl_bound_uniforms_offsets_valid_ = false;
  } else if (!msl_bind_dedupe || !msl_bound_uniforms_offsets_valid_ ||
             msl_bound_uniforms_buffer_ != uniforms_buffer_ ||
             msl_bound_uniforms_vs_base_offset_ != vs_base_offset ||
             msl_bound_uniforms_ps_base_offset_ != ps_base_offset) {
    // System constants (msl_buffer 1).
    current_render_encoder_->setVertexBuffer(uniforms_buffer_,
                                             vs_base_offset + 0 * kCBVSize,
                                             MslBufferIndex::kSystemConstants);
    current_render_encoder_->setFragmentBuffer(
        uniforms_buffer_, ps_base_offset + 0 * kCBVSize,
        MslBufferIndex::kSystemConstants);

    // Float constants vertex (msl_buffer 2).
    current_render_encoder_->setVertexBuffer(
        uniforms_buffer_, vs_base_offset + 1 * kCBVSize,
        MslBufferIndex::kFloatConstantsVertex);
    // Float constants pixel (msl_buffer 3).
    current_render_encoder_->setFragmentBuffer(
        uniforms_buffer_, ps_base_offset + 1 * kCBVSize,
        MslBufferIndex::kFloatConstantsPixel);

    // Bool/loop constants (msl_buffer 4).
    current_render_encoder_->setVertexBuffer(
        uniforms_buffer_, vs_base_offset + 2 * kCBVSize,
        MslBufferIndex::kBoolLoopConstants);
    current_render_encoder_->setFragmentBuffer(
        uniforms_buffer_, ps_base_offset + 2 * kCBVSize,
        MslBufferIndex::kBoolLoopConstants);

    // Fetch constants (msl_buffer 5).
    current_render_encoder_->setVertexBuffer(uniforms_buffer_,
                                             vs_base_offset + 3 * kCBVSize,
                                             MslBufferIndex::kFetchConstants);
    current_render_encoder_->setFragmentBuffer(uniforms_buffer_,
                                               ps_base_offset + 3 * kCBVSize,
                                               MslBufferIndex::kFetchConstants);

    // Clip plane constants (msl_buffer 6) — vertex shader only.
    // The SPIR-V translator uses a separate constant buffer for clip planes.
    current_render_encoder_->setVertexBuffer(
        uniforms_buffer_, vs_base_offset + 4 * kCBVSize,
        MslBufferIndex::kClipPlaneConstants);

    // Tessellation constants (msl_buffer 7).
    current_render_encoder_->setVertexBuffer(
        uniforms_buffer_, vs_base_offset + 5 * kCBVSize,
        MslBufferIndex::kTessellationConstants);
    current_render_encoder_->setFragmentBuffer(
        uniforms_buffer_, ps_base_offset + 5 * kCBVSize,
        MslBufferIndex::kTessellationConstants);

    msl_bound_uniforms_buffer_ = uniforms_buffer_;
    msl_bound_uniforms_vs_base_offset_ = vs_base_offset;
    msl_bound_uniforms_ps_base_offset_ = ps_base_offset;
    msl_bound_uniforms_offsets_valid_ = true;
    msl_inline_constants_bound_versions_.fill(0);
  }

  if (!inline_constants) {
    UseRenderEncoderResource(uniforms_buffer_, MTL::ResourceUsageRead);
  }

  const bool vertex_uses_argbuf =
      vertex_translation && vertex_translation->uses_argument_buffers();
  const bool pixel_uses_argbuf =
      pixel_translation && pixel_translation->uses_argument_buffers();
  auto get_msl_binding_layout_uid =
      [](const MslShader::MslTranslation* translation) -> uint64_t {
    if (!translation) {
      return 0;
    }
    const auto& texture_binding_indices =
        translation->texture_binding_indices_for_msl_slots();
    const auto& sampler_binding_indices =
        translation->sampler_binding_indices_for_msl_slots();
    uint64_t uid = XXH3_64bits(
        texture_binding_indices.data(),
        texture_binding_indices.size() * sizeof(texture_binding_indices[0]));
    uid = XXH3_64bits_withSeed(
        sampler_binding_indices.data(),
        sampler_binding_indices.size() * sizeof(sampler_binding_indices[0]),
        uid);
    return uid;
  };

  auto bind_msl_argument_buffer = [&](MslShader* shader,
                                      MslShader::MslTranslation* translation,
                                      bool is_pixel_stage) -> bool {
    if (!shader || !translation || !translation->uses_argument_buffers() ||
        !texture_cache_) {
      return true;
    }

    MTL::ArgumentEncoder* arg_encoder = translation->argument_encoder();
    uint32_t encoded_length = translation->argument_encoder_encoded_length();
    if (!arg_encoder || encoded_length == 0) {
      return true;
    }

    const auto& texture_bindings = shader->GetTextureBindingsAfterTranslation();
    const auto& texture_binding_indices =
        translation->texture_binding_indices_for_msl_slots();
    uint32_t texture_count = std::min(uint32_t(texture_binding_indices.size()),
                                      MslTextureIndex::kMaxPerStage);
    std::array<const MTL::Texture*, MslTextureIndex::kMaxPerStage> textures =
        {};

    MetalTextureCache* metal_texture_cache = texture_cache_.get();
    for (uint32_t slot = 0; slot < texture_count; ++slot) {
      MTL::Texture* texture = nullptr;
      int32_t texture_binding_index = texture_binding_indices[slot];
      if (texture_binding_index >= 0 &&
          size_t(texture_binding_index) < texture_bindings.size()) {
        const auto& binding = texture_bindings[size_t(texture_binding_index)];
        texture = texture_cache_->GetTextureForBinding(
            binding.fetch_constant, binding.dimension, binding.is_signed);
        if (!texture) {
          switch (binding.dimension) {
            case xenos::FetchOpDimension::k3DOrStacked:
              texture = metal_texture_cache->GetNullTexture3D();
              break;
            case xenos::FetchOpDimension::kCube:
              texture = metal_texture_cache->GetNullTextureCube();
              break;
            default:
              texture = metal_texture_cache->GetNullTexture2D();
              break;
          }
        }
      } else {
        texture = metal_texture_cache->GetNullTexture2D();
      }
      textures[slot] = texture;
      // UseRenderEncoderResource must always be called for hazard tracking,
      // even when we skip re-encoding. The dedup map handles per-encoder
      // deduplication.
      if (texture) {
        UseRenderEncoderResource(texture, MTL::ResourceUsageRead);
      }
    }

    const auto& sampler_bindings = shader->GetSamplerBindingsAfterTranslation();
    const auto& sampler_binding_indices =
        translation->sampler_binding_indices_for_msl_slots();
    uint32_t sampler_count = std::min(uint32_t(sampler_binding_indices.size()),
                                      MslSamplerIndex::kMaxPerStage);
    std::array<const MTL::SamplerState*, MslSamplerIndex::kMaxPerStage>
        samplers = {};
    for (uint32_t smp_index = 0; smp_index < sampler_count; ++smp_index) {
      MTL::SamplerState* sampler_state = null_sampler_;
      uint32_t sampler_binding_index = sampler_binding_indices[smp_index];
      if (sampler_binding_index < sampler_bindings.size()) {
        auto parameters = texture_cache_->GetSamplerParameters(
            sampler_bindings[sampler_binding_index]);
        sampler_state = texture_cache_->GetOrCreateSampler(parameters);
        if (!sampler_state) {
          sampler_state = null_sampler_;
        }
      }
      samplers[smp_index] = sampler_state;
    }

    // Check if textures and samplers match the cached content from the last
    // encoding. If so, skip the expensive acquire+encode and reuse the previous
    // argument buffer slice.
    auto& cached_textures = is_pixel_stage ? msl_last_argbuf_pixel_textures_
                                           : msl_last_argbuf_vertex_textures_;
    auto& cached_texture_count = is_pixel_stage
                                     ? msl_last_argbuf_pixel_texture_count_
                                     : msl_last_argbuf_vertex_texture_count_;
    auto& cached_samplers = is_pixel_stage ? msl_last_argbuf_pixel_samplers_
                                           : msl_last_argbuf_vertex_samplers_;
    auto& cached_sampler_count = is_pixel_stage
                                     ? msl_last_argbuf_pixel_sampler_count_
                                     : msl_last_argbuf_vertex_sampler_count_;
    auto& cached_argbuf_buffer = is_pixel_stage
                                     ? msl_last_argbuf_pixel_buffer_
                                     : msl_last_argbuf_vertex_buffer_;
    auto& cached_argbuf_offset = is_pixel_stage
                                     ? msl_last_argbuf_pixel_offset_
                                     : msl_last_argbuf_vertex_offset_;
    auto& cached_translation = is_pixel_stage
                                   ? msl_last_argbuf_pixel_translation_
                                   : msl_last_argbuf_vertex_translation_;
    auto& cached_encoded_length = is_pixel_stage
                                      ? msl_last_argbuf_pixel_encoded_length_
                                      : msl_last_argbuf_vertex_encoded_length_;
    auto& cached_layout_uid = is_pixel_stage
                                  ? msl_last_argbuf_pixel_layout_uid_
                                  : msl_last_argbuf_vertex_layout_uid_;
    const uint64_t layout_uid = get_msl_binding_layout_uid(translation);

    bool content_changed = cached_translation != translation ||
                           cached_encoded_length != encoded_length ||
                           layout_uid != cached_layout_uid ||
                           texture_count != cached_texture_count ||
                           sampler_count != cached_sampler_count ||
                           !cached_argbuf_buffer;
    if (!content_changed && texture_count > 0) {
      content_changed = std::memcmp(textures.data(), cached_textures.data(),
                                    texture_count * sizeof(textures[0])) != 0;
    }
    if (!content_changed && sampler_count > 0) {
      content_changed = std::memcmp(samplers.data(), cached_samplers.data(),
                                    sampler_count * sizeof(samplers[0])) != 0;
    }

    MTL::Buffer* argbuf_buffer;
    NS::UInteger argbuf_offset;

    if (content_changed) {
      // Content changed — acquire a new slice and re-encode.
      argbuf_buffer = nullptr;
      argbuf_offset = 0;
      if (!AcquireSpirvArgumentBufferSlice(
              encoded_length, translation->argument_encoder_alignment(),
              &argbuf_buffer, &argbuf_offset)) {
        XELOGE(
            "SPIRV-Cross: Failed to allocate argument buffer slice ({} bytes)",
            encoded_length);
        return false;
      }

      arg_encoder->setArgumentBuffer(argbuf_buffer, argbuf_offset);
      if (texture_count) {
        arg_encoder->setTextures(
            textures.data(),
            NS::Range::Make(MslTextureIndex::kBase, texture_count));
      }
      if (sampler_count) {
        arg_encoder->setSamplerStates(
            samplers.data(),
            NS::Range::Make(MslArgumentBufferId::kSamplerBase, sampler_count));
      }

      // Update the cache.
      std::memcpy(cached_textures.data(), textures.data(),
                  texture_count * sizeof(textures[0]));
      if (texture_count < cached_texture_count) {
        std::memset(&cached_textures[texture_count], 0,
                    (cached_texture_count - texture_count) *
                        sizeof(cached_textures[0]));
      }
      cached_texture_count = texture_count;
      std::memcpy(cached_samplers.data(), samplers.data(),
                  sampler_count * sizeof(samplers[0]));
      if (sampler_count < cached_sampler_count) {
        std::memset(&cached_samplers[sampler_count], 0,
                    (cached_sampler_count - sampler_count) *
                        sizeof(cached_samplers[0]));
      }
      cached_sampler_count = sampler_count;
      cached_argbuf_buffer = argbuf_buffer;
      cached_argbuf_offset = argbuf_offset;
      cached_translation = translation;
      cached_encoded_length = encoded_length;
      cached_layout_uid = layout_uid;
    } else {
      // Content unchanged — reuse the previous argument buffer slice.
      argbuf_buffer = cached_argbuf_buffer;
      argbuf_offset = cached_argbuf_offset;
    }

    if (is_pixel_stage) {
      if (argbuf_buffer != msl_bound_pixel_argument_buffer_) {
        current_render_encoder_->setFragmentBuffer(
            argbuf_buffer, 0, MslBufferIndex::kArgumentBufferTexturesSamplers);
        msl_bound_pixel_argument_buffer_ = argbuf_buffer;
        msl_bound_pixel_argument_buffer_offset_valid_ = false;
      }
      if (!msl_bound_pixel_argument_buffer_offset_valid_ ||
          msl_bound_pixel_argument_buffer_offset_ != argbuf_offset) {
        current_render_encoder_->setFragmentBufferOffset(
            argbuf_offset, MslBufferIndex::kArgumentBufferTexturesSamplers);
        msl_bound_pixel_argument_buffer_offset_ = argbuf_offset;
        msl_bound_pixel_argument_buffer_offset_valid_ = true;
      }
    } else {
      if (argbuf_buffer != msl_bound_vertex_argument_buffer_) {
        current_render_encoder_->setVertexBuffer(
            argbuf_buffer, 0, MslBufferIndex::kArgumentBufferTexturesSamplers);
        msl_bound_vertex_argument_buffer_ = argbuf_buffer;
        msl_bound_vertex_argument_buffer_offset_valid_ = false;
      }
      if (!msl_bound_vertex_argument_buffer_offset_valid_ ||
          msl_bound_vertex_argument_buffer_offset_ != argbuf_offset) {
        current_render_encoder_->setVertexBufferOffset(
            argbuf_offset, MslBufferIndex::kArgumentBufferTexturesSamplers);
        msl_bound_vertex_argument_buffer_offset_ = argbuf_offset;
        msl_bound_vertex_argument_buffer_offset_valid_ = true;
      }
    }

    UseRenderEncoderResource(argbuf_buffer, MTL::ResourceUsageRead);
    return true;
  };
  // spirv2air: each stage gets its top-level argument buffer (the stage's
  // constant tables, shared memory and the {binding, binding} index records)
  // and one heap of its textures and then its samplers (Spirv2airLayout),
  // bound as both the resource and the sampler heap. A shader compiled with
  // Spirv2airLayout::kDirectTextures takes texture k at texture index k and
  // sampler j at sampler index j instead, and no heap.
  auto bind_spirv2air_stage = [&](MslShader* shader,
                                  MslShader::MslTranslation* translation,
                                  bool is_pixel_stage) -> bool {
    const bool direct_textures = (translation->spirv2air_options() &
                                  Spirv2airLayout::kDirectTextures) != 0;
    const auto& texture_bindings = shader->GetTextureBindingsAfterTranslation();
    const auto& sampler_bindings = shader->GetSamplerBindingsAfterTranslation();
    const uint32_t texture_count = uint32_t(texture_bindings.size());
    const uint32_t record_count =
        texture_count + uint32_t(sampler_bindings.size());
    if (record_count > Spirv2airLayout::kMaxBindings) {
      XELOGE("spirv2air: {} textures and samplers, more than {}", record_count,
             Spirv2airLayout::kMaxBindings);
      return false;
    }
    // Constant buffer offsets are aligned to 256 bytes on macOS.
    constexpr uint32_t kSliceAlignment = kIsIos ? 16 : 256;
    constexpr uint32_t kHeapOffset =
        (Spirv2airLayout::kArgumentBufferBytes + kSliceAlignment - 1) &
        ~(kSliceAlignment - 1);
    MTL::Buffer* slice_buffer = nullptr;
    NS::UInteger slice_offset = 0;
    if (!AcquireSpirvArgumentBufferSlice(
            kHeapOffset + (direct_textures
                               ? 0
                               : std::max(record_count, 1u) *
                                     Spirv2airLayout::kHeapRecordBytes),
            kSliceAlignment, &slice_buffer, &slice_offset)) {
      XELOGE("spirv2air: Failed to allocate an argument buffer slice");
      return false;
    }
    uint8_t* slice =
        static_cast<uint8_t*>(slice_buffer->contents()) + slice_offset;

    uint64_t argument_buffer[Spirv2airLayout::kArgumentBufferBytes /
                             sizeof(uint64_t)] = {};
    const uint64_t uniforms_address = uniforms_buffer_->gpuAddress();
    const uint64_t stage_uniforms =
        uniforms_address + (is_pixel_stage ? ps_base_offset : vs_base_offset);
    // SPIR-V bindings 0 to 6: system, float vertex, float pixel, bool/loop,
    // fetch, clip plane and tessellation constants, from the stage's table
    // (a stage's float constants are its table's second block).
    argument_buffer[0] = stage_uniforms + 0 * kCBVSize;
    argument_buffer[1] = uniforms_address + vs_base_offset + 1 * kCBVSize;
    argument_buffer[2] = uniforms_address + ps_base_offset + 1 * kCBVSize;
    argument_buffer[3] = stage_uniforms + 2 * kCBVSize;
    argument_buffer[4] = stage_uniforms + 3 * kCBVSize;
    argument_buffer[5] = stage_uniforms + 4 * kCBVSize;
    argument_buffer[6] = stage_uniforms + 5 * kCBVSize;
    const uint64_t shared_memory_address =
        shared_mem_buffer ? shared_mem_buffer->gpuAddress() : 0;
    argument_buffer[Spirv2airLayout::kSharedMemoryReadOffset / 8] =
        shared_memory_address;
    argument_buffer[Spirv2airLayout::kSharedMemoryWriteOffset / 8] =
        shared_memory_address;
    const uint64_t indices_address = spirv2air_texture_indices_->gpuAddress();
    argument_buffer[Spirv2airLayout::kVertexTextureIndicesOffset / 8] =
        indices_address;
    argument_buffer[Spirv2airLayout::kFragmentTextureIndicesOffset / 8] =
        indices_address;
    std::memcpy(slice, argument_buffer, sizeof(argument_buffer));

    uint8_t* heap = slice + kHeapOffset;
    if (!direct_textures) {
      std::memset(heap, 0, record_count * Spirv2airLayout::kHeapRecordBytes);
    }
    MetalTextureCache* metal_texture_cache = texture_cache_.get();
    // Slots bound directly are what the Metal Shading Language path's binding
    // cache would otherwise take as still bound by it.
    auto* bound_textures = is_pixel_stage ? &msl_bound_pixel_textures_
                                          : &msl_bound_vertex_textures_;
    auto* bound_samplers = is_pixel_stage ? &msl_bound_pixel_samplers_
                                          : &msl_bound_vertex_samplers_;
    for (uint32_t i = 0; i < texture_count; ++i) {
      const auto& binding = texture_bindings[i];
      MTL::Texture* texture = texture_cache_->GetTextureForBinding(
          binding.fetch_constant, binding.dimension, binding.is_signed);
      if (!texture) {
        switch (binding.dimension) {
          case xenos::FetchOpDimension::k3DOrStacked:
            texture = metal_texture_cache->GetNullTexture3D();
            break;
          case xenos::FetchOpDimension::kCube:
            texture = metal_texture_cache->GetNullTextureCube();
            break;
          default:
            texture = metal_texture_cache->GetNullTexture2D();
            break;
        }
      }
      if (direct_textures) {
        if (is_pixel_stage) {
          current_render_encoder_->setFragmentTexture(texture, i);
        } else {
          current_render_encoder_->setVertexTexture(texture, i);
        }
        (*bound_textures)[i] = texture;
        continue;
      }
      if (!texture) {
        continue;
      }
      const MTL::ResourceID handle = texture->gpuResourceID();
      std::memcpy(heap + i * Spirv2airLayout::kHeapRecordBytes +
                      Spirv2airLayout::kHeapTextureHandleOffset,
                  &handle, sizeof(handle));
      UseRenderEncoderResource(texture, MTL::ResourceUsageRead);
    }
    for (uint32_t i = texture_count; i < record_count; ++i) {
      MTL::SamplerState* sampler_state = texture_cache_->GetOrCreateSampler(
          texture_cache_->GetSamplerParameters(
              sampler_bindings[i - texture_count]));
      if (!sampler_state) {
        sampler_state = null_sampler_;
      }
      if (direct_textures) {
        const uint32_t sampler_index = i - texture_count;
        if (is_pixel_stage) {
          current_render_encoder_->setFragmentSamplerState(sampler_state,
                                                           sampler_index);
        } else {
          current_render_encoder_->setVertexSamplerState(sampler_state,
                                                         sampler_index);
        }
        (*bound_samplers)[sampler_index] = sampler_state;
        continue;
      }
      const MTL::ResourceID handle = sampler_state->gpuResourceID();
      std::memcpy(heap + i * Spirv2airLayout::kHeapRecordBytes, &handle,
                  sizeof(handle));
    }

    if (direct_textures) {
      if (is_pixel_stage) {
        current_render_encoder_->setFragmentBuffer(
            slice_buffer, slice_offset, Spirv2airLayout::kArgumentBuffer);
        msl_bound_pixel_texture_count_ =
            std::max(msl_bound_pixel_texture_count_, texture_count);
        msl_bound_pixel_sampler_count_ = std::max(
            msl_bound_pixel_sampler_count_, record_count - texture_count);
        msl_bound_pixel_texture_binding_uid_ = 0;
        msl_bound_pixel_sampler_binding_uid_ = 0;
      } else {
        current_render_encoder_->setVertexBuffer(
            slice_buffer, slice_offset, Spirv2airLayout::kArgumentBuffer);
        msl_bound_vertex_texture_count_ =
            std::max(msl_bound_vertex_texture_count_, texture_count);
        msl_bound_vertex_sampler_count_ = std::max(
            msl_bound_vertex_sampler_count_, record_count - texture_count);
        msl_bound_vertex_texture_binding_uid_ = 0;
        msl_bound_vertex_sampler_binding_uid_ = 0;
      }
    } else if (is_pixel_stage) {
      current_render_encoder_->setFragmentBuffer(
          slice_buffer, slice_offset, Spirv2airLayout::kArgumentBuffer);
      current_render_encoder_->setFragmentBuffer(
          slice_buffer, slice_offset + kHeapOffset,
          Spirv2airLayout::kResourceHeapBuffer);
      current_render_encoder_->setFragmentBuffer(
          slice_buffer, slice_offset + kHeapOffset,
          Spirv2airLayout::kSamplerHeapBuffer);
    } else {
      current_render_encoder_->setVertexBuffer(
          slice_buffer, slice_offset, Spirv2airLayout::kArgumentBuffer);
      current_render_encoder_->setVertexBuffer(
          slice_buffer, slice_offset + kHeapOffset,
          Spirv2airLayout::kResourceHeapBuffer);
      current_render_encoder_->setVertexBuffer(
          slice_buffer, slice_offset + kHeapOffset,
          Spirv2airLayout::kSamplerHeapBuffer);
    }
    UseRenderEncoderResource(slice_buffer, MTL::ResourceUsageRead);
    return true;
  };
  if (use_spirv2air) {
    if (!texture_cache_ ||
        !bind_spirv2air_stage(msl_vertex_shader, vertex_translation, false) ||
        (msl_pixel_shader &&
         !bind_spirv2air_stage(msl_pixel_shader, pixel_translation, true))) {
      return false;
    }
    UseRenderEncoderResource(spirv2air_texture_indices_,
                             MTL::ResourceUsageRead);
  }

  if (!use_spirv2air && vertex_uses_argbuf &&
      !bind_msl_argument_buffer(msl_vertex_shader, vertex_translation, false)) {
    return false;
  }
  if (!use_spirv2air && pixel_uses_argbuf &&
      !bind_msl_argument_buffer(msl_pixel_shader, pixel_translation, true)) {
    return false;
  }

  // Bind textures and samplers directly.
  auto bind_msl_textures = [&](MslShader* shader,
                               MslShader::MslTranslation* translation,
                               bool is_pixel_stage) {
    auto bind_texture_slot = [&](uint32_t slot, MTL::Texture* texture) {
      if (is_pixel_stage) {
        current_render_encoder_->setFragmentTexture(texture, slot);
      } else {
        current_render_encoder_->setVertexTexture(texture, slot);
      }
    };
    uint32_t* previous_bound_count = is_pixel_stage
                                         ? &msl_bound_pixel_texture_count_
                                         : &msl_bound_vertex_texture_count_;
    uint64_t* cached_binding_uid = is_pixel_stage
                                       ? &msl_bound_pixel_texture_binding_uid_
                                       : &msl_bound_vertex_texture_binding_uid_;
    auto* bound_textures = is_pixel_stage ? &msl_bound_pixel_textures_
                                          : &msl_bound_vertex_textures_;
    const uint64_t layout_uid = get_msl_binding_layout_uid(translation);
    const uint64_t binding_uid =
        (shader && translation && texture_cache_)
            ? (uint64_t(reinterpret_cast<uintptr_t>(pipeline)) *
                   UINT64_C(11400714819323198485) ^
               layout_uid)
            : 0;
    const bool force_rebind = *cached_binding_uid != binding_uid;
    auto clear_slots_from = [&](uint32_t start, uint32_t end_exclusive) {
      for (uint32_t slot = start; slot < end_exclusive; ++slot) {
        if (force_rebind || (*bound_textures)[slot] != nullptr) {
          bind_texture_slot(slot, nullptr);
          (*bound_textures)[slot] = nullptr;
        }
      }
    };

    if (!shader || !translation || !texture_cache_) {
      clear_slots_from(0, *previous_bound_count);
      *previous_bound_count = 0;
      *cached_binding_uid = binding_uid;
      return;
    }

    const auto& texture_bindings = shader->GetTextureBindingsAfterTranslation();
    const auto& texture_binding_indices =
        translation->texture_binding_indices_for_msl_slots();
    uint32_t bound_count = std::min(uint32_t(texture_binding_indices.size()),
                                    MslTextureIndex::kMaxPerStage);
    if (*previous_bound_count > bound_count) {
      clear_slots_from(bound_count, *previous_bound_count);
    }

    MetalTextureCache* metal_texture_cache = texture_cache_.get();
    for (uint32_t slot = 0; slot < bound_count; ++slot) {
      uint32_t tex_index = MslTextureIndex::kBase + slot;
      MTL::Texture* texture = nullptr;
      int32_t texture_binding_index = texture_binding_indices[slot];
      if (texture_binding_index >= 0 &&
          size_t(texture_binding_index) < texture_bindings.size()) {
        const auto& binding = texture_bindings[size_t(texture_binding_index)];
        texture = texture_cache_->GetTextureForBinding(
            binding.fetch_constant, binding.dimension, binding.is_signed);
        if (!texture) {
          switch (binding.dimension) {
            case xenos::FetchOpDimension::k3DOrStacked:
              texture = metal_texture_cache->GetNullTexture3D();
              break;
            case xenos::FetchOpDimension::kCube:
              texture = metal_texture_cache->GetNullTextureCube();
              break;
            default:
              texture = metal_texture_cache->GetNullTexture2D();
              break;
          }
        }
      } else {
        texture = metal_texture_cache->GetNullTexture2D();
      }
      if (force_rebind || (*bound_textures)[tex_index] != texture) {
        bind_texture_slot(tex_index, texture);
        (*bound_textures)[tex_index] = texture;
      }
      if (texture) {
        UseRenderEncoderResource(texture, MTL::ResourceUsageRead);
      }
    }
    *previous_bound_count = bound_count;
    *cached_binding_uid = binding_uid;
  };

  auto bind_msl_samplers = [&](MslShader* shader,
                               MslShader::MslTranslation* translation,
                               bool is_pixel_stage) {
    auto bind_sampler_slot = [&](uint32_t slot, MTL::SamplerState* sampler) {
      if (is_pixel_stage) {
        current_render_encoder_->setFragmentSamplerState(sampler, slot);
      } else {
        current_render_encoder_->setVertexSamplerState(sampler, slot);
      }
    };

    uint32_t* previous_bound_count = is_pixel_stage
                                         ? &msl_bound_pixel_sampler_count_
                                         : &msl_bound_vertex_sampler_count_;
    uint64_t* cached_binding_uid = is_pixel_stage
                                       ? &msl_bound_pixel_sampler_binding_uid_
                                       : &msl_bound_vertex_sampler_binding_uid_;
    auto* bound_samplers = is_pixel_stage ? &msl_bound_pixel_samplers_
                                          : &msl_bound_vertex_samplers_;
    const uint64_t layout_uid = get_msl_binding_layout_uid(translation);
    const uint64_t binding_uid =
        (shader && translation && texture_cache_)
            ? (uint64_t(reinterpret_cast<uintptr_t>(pipeline)) *
                   UINT64_C(11400714819323198485) ^
               layout_uid)
            : 0;
    const bool force_rebind = *cached_binding_uid != binding_uid;

    if (!shader || !translation || !texture_cache_) {
      for (uint32_t slot = 0; slot < *previous_bound_count; ++slot) {
        if (force_rebind || (*bound_samplers)[slot] != null_sampler_) {
          bind_sampler_slot(slot, null_sampler_);
          (*bound_samplers)[slot] = null_sampler_;
        }
      }
      *previous_bound_count = 0;
      *cached_binding_uid = binding_uid;
      return;
    }
    // Samplers are remapped to compact Metal indices 0..M-1 in
    // MslShader::AddResourceBindings. Use the reflected SPIR-V remap order
    // captured in the translation, not the raw translator array order, because
    // SPIRV-Cross may drop/reorder separate samplers.
    const auto& sampler_bindings = shader->GetSamplerBindingsAfterTranslation();
    const auto& sampler_binding_indices =
        translation->sampler_binding_indices_for_msl_slots();
    uint32_t bound_count = std::min(uint32_t(sampler_binding_indices.size()),
                                    MslSamplerIndex::kMaxPerStage);
    if (*previous_bound_count > bound_count) {
      for (uint32_t slot = bound_count; slot < *previous_bound_count; ++slot) {
        if (force_rebind || (*bound_samplers)[slot] != null_sampler_) {
          bind_sampler_slot(slot, null_sampler_);
          (*bound_samplers)[slot] = null_sampler_;
        }
      }
    }

    for (uint32_t smp_index = 0; smp_index < bound_count; ++smp_index) {
      MTL::SamplerState* sampler_state = null_sampler_;
      uint32_t sampler_binding_index = sampler_binding_indices[smp_index];
      if (sampler_binding_index < sampler_bindings.size()) {
        auto parameters = texture_cache_->GetSamplerParameters(
            sampler_bindings[sampler_binding_index]);
        sampler_state = texture_cache_->GetOrCreateSampler(parameters);
        if (!sampler_state) {
          sampler_state = null_sampler_;
        }
      }
      if (force_rebind || (*bound_samplers)[smp_index] != sampler_state) {
        bind_sampler_slot(smp_index, sampler_state);
        (*bound_samplers)[smp_index] = sampler_state;
      }
    }
    *previous_bound_count = bound_count;
    *cached_binding_uid = binding_uid;
  };

  if (!use_spirv2air && !vertex_uses_argbuf) {
    bind_msl_textures(msl_vertex_shader, vertex_translation, false);
    bind_msl_samplers(msl_vertex_shader, vertex_translation, false);
  }
  if (!use_spirv2air && !pixel_uses_argbuf) {
    bind_msl_textures(msl_pixel_shader, pixel_translation, true);
    bind_msl_samplers(msl_pixel_shader, pixel_translation, true);
  }

  if (metal_perf_counters.frame_detail_frames.load(std::memory_order_relaxed)) {
    // Texture fetch constants as they are (6 dwords), marked when their base
    // is in memory a resolve wrote.
    std::string frame_detail_textures;
    uint32_t fetch_bits = used_texture_mask;
    uint32_t fetch_index = 0;
    while (xe::bit_scan_forward(fetch_bits, &fetch_index)) {
      fetch_bits &= ~(uint32_t(1) << fetch_index);
      const uint32_t* fetch =
          &regs.values[XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 + fetch_index * 6];
      fmt::format_to(
          std::back_inserter(frame_detail_textures),
          " t{} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X}{}", fetch_index,
          fetch[0], fetch[1], fetch[2], fetch[3], fetch[4], fetch[5],
          IsResolvedMemory(regs.GetTextureFetch(fetch_index).base_address << 12,
                           1)
              ? " resolved"
              : "");
    }
    XELOGI(
        "Frame detail draw {} (pass {}): VS {:016X} mod {:X}, PS {:016X} mod "
        "{:X}, guest primitive {} ({} vertices), host {} ({} vertices), "
        "COLOR_MASK {:08X} DEPTHCONTROL {:08X} STENCILREFMASK {:08X} "
        "BLENDCONTROL0 {:08X} COLORCONTROL {:08X} SC_MODE_CNTL {:08X} "
        "scissor {:08X} {:08X}, viewport scale {} {} {} offset {} {} {}, "
        "counting a query {}; textures:{}",
        ++frame_detail_draws_, frame_detail_passes_,
        vertex_translation->shader().ucode_data_hash(),
        vertex_translation->modification(),
        pixel_translation ? pixel_translation->shader().ucode_data_hash() : 0,
        pixel_translation ? pixel_translation->modification() : 0,
        uint32_t(primitive_processing_result.guest_primitive_type),
        primitive_processing_result.guest_draw_vertex_count,
        uint32_t(primitive_processing_result.host_primitive_type),
        primitive_processing_result.host_draw_vertex_count,
        regs.values[XE_GPU_REG_RB_COLOR_MASK],
        regs.values[XE_GPU_REG_RB_DEPTHCONTROL],
        regs.values[XE_GPU_REG_RB_STENCILREFMASK],
        regs.values[XE_GPU_REG_RB_BLENDCONTROL0],
        regs.values[XE_GPU_REG_RB_COLORCONTROL],
        regs.values[XE_GPU_REG_PA_SU_SC_MODE_CNTL],
        regs.values[XE_GPU_REG_PA_SC_WINDOW_SCISSOR_TL],
        regs.values[XE_GPU_REG_PA_SC_WINDOW_SCISSOR_BR],
        regs.Get<float>(XE_GPU_REG_PA_CL_VPORT_XSCALE),
        regs.Get<float>(XE_GPU_REG_PA_CL_VPORT_YSCALE),
        regs.Get<float>(XE_GPU_REG_PA_CL_VPORT_ZSCALE),
        regs.Get<float>(XE_GPU_REG_PA_CL_VPORT_XOFFSET),
        regs.Get<float>(XE_GPU_REG_PA_CL_VPORT_YOFFSET),
        regs.Get<float>(XE_GPU_REG_PA_CL_VPORT_ZOFFSET), zpd_query_active_,
        frame_detail_textures);
    // Small draws (clears, full-screen rectangles): the vertex data of their
    // vertex shader's first binding, as floats.
    const auto& frame_detail_bindings =
        vertex_translation->shader().vertex_bindings();
    if (primitive_processing_result.guest_draw_vertex_count <= 12 &&
        !frame_detail_bindings.empty()) {
      const auto& binding = frame_detail_bindings.front();
      xenos::xe_gpu_vertex_fetch_t vertex_fetch =
          regs.GetVertexFetch(binding.fetch_constant);
      const uint32_t vertex_address = vertex_fetch.address << 2;
      const uint32_t vertex_words = std::min<uint32_t>(
          {uint32_t(vertex_fetch.size),
           primitive_processing_result.guest_draw_vertex_count *
               binding.stride_words,
           48});
      std::string frame_detail_vertices;
      if (vertex_address <= SharedMemory::kBufferSize - vertex_words * 4) {
        const uint32_t* words =
            memory_->TranslatePhysical<const uint32_t*>(vertex_address);
        for (uint32_t i = 0; i < vertex_words; ++i) {
          const uint32_t word = xenos::GpuSwap(words[i], vertex_fetch.endian);
          float word_float;
          std::memcpy(&word_float, &word, sizeof(word_float));
          fmt::format_to(std::back_inserter(frame_detail_vertices), "{}{}",
                         (binding.stride_words && i % binding.stride_words == 0)
                             ? " |"
                             : " ",
                         word_float);
        }
      }
      XELOGI(
          "Frame detail draw {} vertices (fetch constant {} at {:08X}, stride "
          "{} words):{}",
          frame_detail_draws_, binding.fetch_constant, vertex_address,
          binding.stride_words, frame_detail_vertices);
    }
    // The float constants of shaders listed in metal_log_shader_hashes.
    for (MslShader* frame_detail_shader :
         {msl_vertex_shader, msl_pixel_shader}) {
      if (!frame_detail_shader ||
          !ShouldLogMslShader(frame_detail_shader->ucode_data_hash())) {
        continue;
      }
      const bool is_pixel = frame_detail_shader == msl_pixel_shader;
      std::string frame_detail_constants;
      const Shader::ConstantRegisterMap& map =
          frame_detail_shader->constant_register_map();
      for (uint32_t i = 0; i < 256; ++i) {
        if (!(map.float_bitmap[i >> 6] & (uint64_t(1) << (i & 63)))) {
          continue;
        }
        const uint32_t reg = (is_pixel ? XE_GPU_REG_SHADER_CONSTANT_256_X
                                       : XE_GPU_REG_SHADER_CONSTANT_000_X) +
                             i * 4;
        fmt::format_to(std::back_inserter(frame_detail_constants),
                       " c{} ({} {} {} {})", i, regs.Get<float>(reg),
                       regs.Get<float>(reg + 1), regs.Get<float>(reg + 2),
                       regs.Get<float>(reg + 3));
      }
      XELOGI("Frame detail draw {} {} {:016X} float constants:{}",
             frame_detail_draws_, is_pixel ? "PS" : "VS",
             frame_detail_shader->ucode_data_hash(), frame_detail_constants);
    }
  }

  // occlusion_query_enable: count the draw's samples for the guest's query,
  // in a slot of its own for each render pass.
  if (zpd_query_active_ && !zpd_segment_open_ && !zpd_query_.inexact) {
    uint32_t slot_index = 0;
    uint32_t slot_generation = 0;
    size_t slot_offset = 0;
    if (render_encoder_has_zpd_visibility_ &&
        zpd_visibility_pool_.Acquire(slot_index, slot_generation,
                                     slot_offset)) {
      current_render_encoder_->setVisibilityResultMode(
          MTL::VisibilityResultModeCounting, NS::UInteger(slot_offset));
      zpd_query_.slots.emplace_back(slot_index, slot_generation);
      zpd_segment_open_ = true;
    } else {
      zpd_query_.inexact = true;
    }
  }

  // =====================================================================
  // Draw dispatch — native Metal encoder calls (no IRRuntime).
  // =====================================================================
  if (is_tessellated) {
    // ---------------------------------------------------------------
    // Tessellated draw: fill tessellation factor buffer, then drawPatches.
    // ---------------------------------------------------------------
    // Xenos tess levels are 0-based; add 1 to match other backends.
    float max_tess = std::max(
        1.0f, regs.Get<float>(XE_GPU_REG_VGT_HOS_MAX_TESS_LEVEL) + 1.0f);

    // Determine control points per patch and patch count from the draw.
    // The primitive processor passes patch count in host_draw_vertex_count
    // for tessellated draws (vertex count = cp_per_patch * patch_count).
    uint32_t cp_per_patch = 1;
    bool is_quad_domain = false;
    switch (host_vertex_shader_type) {
      case Shader::HostVertexShaderType::kTriangleDomainCPIndexed:
      case Shader::HostVertexShaderType::kTriangleDomainPatchIndexed:
        cp_per_patch = 3;
        break;
      case Shader::HostVertexShaderType::kQuadDomainCPIndexed:
      case Shader::HostVertexShaderType::kQuadDomainPatchIndexed:
        cp_per_patch = 4;
        is_quad_domain = true;
        break;
      case Shader::HostVertexShaderType::kLineDomainCPIndexed:
      case Shader::HostVertexShaderType::kLineDomainPatchIndexed:
        cp_per_patch = 2;
        break;
      default:
        break;
    }
    uint32_t vertex_count = primitive_processing_result.host_draw_vertex_count;
    uint32_t patch_count = cp_per_patch > 0 ? vertex_count / cp_per_patch : 0;
    if (patch_count == 0) {
      return true;  // Nothing to draw.
    }

    // Ensure tessellation factor buffer is large enough.
    if (!EnsureTessFactorBuffer(patch_count)) {
      XELOGE(
          "SPIRV-Cross: Failed to allocate tess factor buffer for {} "
          "patches",
          patch_count);
      return false;
    }

    // IEEE 754 float32 → float16 conversion for tessellation factors.
    // Uses round-to-nearest-even and handles subnormals for accuracy
    // near tessellation factor boundaries.
    auto f32_to_f16 = [](float v) -> uint16_t {
      uint32_t b;
      std::memcpy(&b, &v, 4);
      uint16_t s = (b >> 16) & 0x8000u;
      int e = int((b >> 23) & 0xFFu) - 127 + 15;
      uint32_t m = b & 0x7FFFFFu;
      if (e <= 0) {
        // Subnormal or zero in half precision.
        if (e < -10) return s;  // Too small, flush to signed zero.
        // Subnormal half: shift mantissa (with implicit leading 1) right.
        m = (m | 0x800000u) >> (1 - e);
        // Round to nearest even.
        if ((m & 0x1FFFu) > 0x1000u ||
            ((m & 0x1FFFu) == 0x1000u && (m & 0x2000u))) {
          m += 0x2000u;
        }
        return uint16_t(s | (m >> 13));
      }
      if (e >= 31) {
        // Overflow → infinity (or NaN passthrough).
        if (e == 31 && m != 0) {
          // NaN: preserve at least one mantissa bit.
          return uint16_t(s | 0x7C00u | std::max(m >> 13, uint32_t(1)));
        }
        return uint16_t(s | 0x7C00u);
      }
      // Round to nearest even: check the 13 bits being truncated.
      if ((m & 0x1FFFu) > 0x1000u ||
          ((m & 0x1FFFu) == 0x1000u && (m & 0x2000u))) {
        m += 0x2000u;
        if (m & 0x800000u) {
          m = 0;
          e++;
          if (e >= 31) return uint16_t(s | 0x7C00u);
        }
      }
      return uint16_t(s | (e << 10) | (m >> 13));
    };

    // Determine tessellation mode from the register file.
    auto tess_mode = regs.Get<reg::VGT_HOS_CNTL>().tess_mode;

    uint8_t* factor_data =
        static_cast<uint8_t*>(tess_factor_buffer_->contents());

    if (tess_mode == xenos::TessellationMode::kAdaptive && shared_memory_) {
      // ------------------------------------------------------------------
      // Adaptive tessellation: per-edge factors from shared memory.
      // The guest "index buffer" is repurposed as a factor buffer
      // containing big-endian float32 edge factors.
      // ------------------------------------------------------------------
      xenos::Endian index_endian =
          primitive_processing_result.host_shader_index_endian;
      uint32_t factor_base = primitive_processing_result.guest_index_base;
      const uint8_t* xbox_ram = shared_memory_->GetXboxRamBase();
      // Minimum factor from VGT_HOS_MIN_TESS_LEVEL + 1.0 (Xbox 360
      // convention). For fractional_even partitioning, must be >= 2.0.
      float factor_min = std::max(
          2.0f, regs.Get<float>(XE_GPU_REG_VGT_HOS_MIN_TESS_LEVEL) + 1.0f);
      float factor_max = max_tess;

      if (is_quad_domain) {
        // Quad: 4 edge factors per patch.
        struct QuadFactors {
          uint16_t edge[4];
          uint16_t inside[2];
        };
        static_assert(sizeof(QuadFactors) == 12);
        auto* factors = reinterpret_cast<QuadFactors*>(factor_data);
        for (uint32_t i = 0; i < patch_count; ++i) {
          // Read 4 edge factors from guest memory.
          float ef[4];
          for (uint32_t j = 0; j < 4; ++j) {
            uint32_t addr = factor_base + (i * 4 + j) * sizeof(float);
            float raw;
            std::memcpy(&raw, xbox_ram + addr, sizeof(float));
            // Endian-swap per the index endian mode.
            raw = xenos::GpuSwap(raw, index_endian);
            // Add 1.0 per Xbox 360 convention, clamp.
            ef[j] = std::clamp(raw + 1.0f, factor_min, factor_max);
          }
          // Map Xbox 360 edge order to Metal:
          //   edge[i] = input[(i+3) & 3]
          //   (from adaptive_quad.hs.glsl)
          factors[i].edge[0] = f32_to_f16(ef[3]);
          factors[i].edge[1] = f32_to_f16(ef[0]);
          factors[i].edge[2] = f32_to_f16(ef[1]);
          factors[i].edge[3] = f32_to_f16(ef[2]);
          // Inside factors: minimum of opposing edges.
          // inside[0] along U = min(mapped_edge[1], mapped_edge[3])
          // inside[1] along V = min(mapped_edge[0], mapped_edge[2])
          factors[i].inside[0] = f32_to_f16(std::min(ef[0], ef[2]));
          factors[i].inside[1] = f32_to_f16(std::min(ef[3], ef[1]));
        }
      } else {
        // Triangle: 3 edge factors per patch.
        struct TriFactors {
          uint16_t edge[3];
          uint16_t inside;
        };
        static_assert(sizeof(TriFactors) == 8);
        auto* factors = reinterpret_cast<TriFactors*>(factor_data);
        for (uint32_t i = 0; i < patch_count; ++i) {
          // Read 3 edge factors from guest memory.
          float ef[3];
          for (uint32_t j = 0; j < 3; ++j) {
            uint32_t addr = factor_base + (i * 3 + j) * sizeof(float);
            float raw;
            std::memcpy(&raw, xbox_ram + addr, sizeof(float));
            raw = xenos::GpuSwap(raw, index_endian);
            ef[j] = std::clamp(raw + 1.0f, factor_min, factor_max);
          }
          // Map Xbox 360 edge order to Metal:
          //   Metal edge[0] = U0 (v1->v2) = ef[1]
          //   Metal edge[1] = V0 (v2->v0) = ef[2]
          //   Metal edge[2] = W0 (v0->v1) = ef[0]
          //   (from adaptive_triangle.hs.glsl)
          factors[i].edge[0] = f32_to_f16(ef[1]);
          factors[i].edge[1] = f32_to_f16(ef[2]);
          factors[i].edge[2] = f32_to_f16(ef[0]);
          // Inside factor = minimum of all edge factors.
          factors[i].inside =
              f32_to_f16(std::min(std::min(ef[0], ef[1]), ef[2]));
        }
      }
    } else {
      // ------------------------------------------------------------------
      // Uniform tessellation (discrete / continuous modes).
      // All patches get the same factor from VGT_HOS_MAX_TESS_LEVEL.
      // ------------------------------------------------------------------
      uint16_t ef = f32_to_f16(max_tess);
      if (is_quad_domain) {
        struct QuadFactors {
          uint16_t edge[4];
          uint16_t inside[2];
        };
        static_assert(sizeof(QuadFactors) == 12);
        auto* factors = reinterpret_cast<QuadFactors*>(factor_data);
        for (uint32_t i = 0; i < patch_count; ++i) {
          factors[i].edge[0] = ef;
          factors[i].edge[1] = ef;
          factors[i].edge[2] = ef;
          factors[i].edge[3] = ef;
          factors[i].inside[0] = ef;
          factors[i].inside[1] = ef;
        }
      } else {
        struct TriFactors {
          uint16_t edge[3];
          uint16_t inside;
        };
        static_assert(sizeof(TriFactors) == 8);
        auto* factors = reinterpret_cast<TriFactors*>(factor_data);
        for (uint32_t i = 0; i < patch_count; ++i) {
          factors[i].edge[0] = ef;
          factors[i].edge[1] = ef;
          factors[i].edge[2] = ef;
          factors[i].inside = ef;
        }
      }
    }

    // Draw with tessellation.
    assert_not_null(tess_factor_buffer_);
    UseRenderEncoderResource(tess_factor_buffer_, MTL::ResourceUsageRead);
    current_render_encoder_->setTessellationFactorBuffer(tess_factor_buffer_, 0,
                                                         0);
    // drawPatches signature:
    //   numberOfPatchControlPoints, patchStart, patchCount,
    //   patchIndexBuffer, patchIndexBufferOffset,
    //   instanceCount, baseInstance
    // patchIndexBuffer = nullptr since patches are not indexed (the domain
    // shader reads control points from shared memory via vertex ID).
    current_render_encoder_->drawPatches(
        NS::UInteger(cp_per_patch), NS::UInteger(0), NS::UInteger(patch_count),
        nullptr,           // patchIndexBuffer (non-indexed patches)
        0,                 // patchIndexBufferOffset
        NS::UInteger(1),   // instanceCount
        NS::UInteger(0));  // baseInstance
  } else {
    // ---------------------------------------------------------------
    // Non-tessellated draw: standard primitives.
    // ---------------------------------------------------------------
    MTL::PrimitiveType mtl_primitive = MTL::PrimitiveTypeTriangle;
    switch (primitive_processing_result.host_primitive_type) {
      case xenos::PrimitiveType::kPointList:
        mtl_primitive = MTL::PrimitiveTypePoint;
        break;
      case xenos::PrimitiveType::kLineList:
        mtl_primitive = MTL::PrimitiveTypeLine;
        break;
      case xenos::PrimitiveType::kLineStrip:
        mtl_primitive = MTL::PrimitiveTypeLineStrip;
        break;
      case xenos::PrimitiveType::kTriangleList:
      case xenos::PrimitiveType::kRectangleList:
        mtl_primitive = MTL::PrimitiveTypeTriangle;
        break;
      case xenos::PrimitiveType::kTriangleStrip:
        mtl_primitive = MTL::PrimitiveTypeTriangleStrip;
        break;
      default:
        XELOGE("SPIRV-Cross: Unsupported host primitive type {}",
               uint32_t(primitive_processing_result.host_primitive_type));
        return false;
    }

    auto request_guest_index_range = [&](uint64_t index_base,
                                         uint32_t index_count,
                                         MTL::IndexType index_type) -> bool {
      if (!shared_memory_) {
        return false;
      }
      uint32_t index_stride = (index_type == MTL::IndexTypeUInt16)
                                  ? sizeof(uint16_t)
                                  : sizeof(uint32_t);
      uint64_t index_length = uint64_t(index_count) * index_stride;
      if (index_base > SharedMemory::kBufferSize ||
          SharedMemory::kBufferSize - index_base < index_length) {
        return false;
      }
      return shared_memory_->RequestRangeForGpuRead(
          static_cast<uint32_t>(index_base),
          static_cast<uint32_t>(index_length));
    };

    bool use_expansion_triangle_list_fallback = false;
    uint32_t draw_index_count =
        primitive_processing_result.host_draw_vertex_count;
    if ((host_vertex_shader_type ==
             Shader::HostVertexShaderType::kPointListAsTriangleStrip ||
         host_vertex_shader_type ==
             Shader::HostVertexShaderType::kRectangleListAsTriangleStrip) &&
        (primitive_processing_result.index_buffer_type ==
             PrimitiveProcessor::ProcessedIndexBufferType::
                 kHostBuiltinForAuto ||
         primitive_processing_result.index_buffer_type ==
             PrimitiveProcessor::ProcessedIndexBufferType::
                 kHostBuiltinForDMA)) {
      // Expansion strips normally rely on primitive restart separators.
      // Keep a Metal-local triangle-list fallback to avoid dependence on strip
      // restart behavior in this SPIRV-Cross path.
      uint32_t strip_index_count = draw_index_count;
      uint32_t expanded_primitive_count =
          strip_index_count ? (strip_index_count + 1u) / 5u : 0u;
      draw_index_count = expanded_primitive_count * 6u;
      mtl_primitive = MTL::PrimitiveTypeTriangle;
      use_expansion_triangle_list_fallback = true;
      static bool logged_expansion_triangle_list_fallback = false;
      if (!logged_expansion_triangle_list_fallback) {
        logged_expansion_triangle_list_fallback = true;
        XELOGW(
            "SPIRV-Cross: Using triangle-list fallback for VS primitive "
            "expansion draws to avoid strip-restart dependency");
      }
    }

    if (primitive_processing_result.index_buffer_type ==
        PrimitiveProcessor::ProcessedIndexBufferType::kNone) {
      // Non-indexed draw.
      current_render_encoder_->drawPrimitives(
          mtl_primitive, NS::UInteger(0),
          NS::UInteger(primitive_processing_result.host_draw_vertex_count));
    } else {
      // Indexed draw.
      MTL::IndexType index_type =
          (primitive_processing_result.host_index_format ==
           xenos::IndexFormat::kInt16)
              ? MTL::IndexTypeUInt16
              : MTL::IndexTypeUInt32;
      MTL::Buffer* index_buffer = nullptr;
      uint64_t index_offset = 0;
      switch (primitive_processing_result.index_buffer_type) {
        case PrimitiveProcessor::ProcessedIndexBufferType::kGuestDMA:
          index_buffer = shared_memory_ ? shared_memory_->GetBuffer() : nullptr;
          index_offset = primitive_processing_result.guest_index_base;
          if (!request_guest_index_range(index_offset, draw_index_count,
                                         index_type)) {
            XELOGE("SPIRV-Cross: Failed to validate guest index buffer range");
            return false;
          }
          break;
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostConverted:
          if (primitive_processor_) {
            index_buffer = primitive_processor_->GetConvertedIndexBuffer(
                primitive_processing_result.host_index_buffer_handle,
                index_offset);
          }
          break;
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostBuiltinForAuto:
        case PrimitiveProcessor::ProcessedIndexBufferType::kHostBuiltinForDMA:
          if (primitive_processor_) {
            if (use_expansion_triangle_list_fallback) {
              index_buffer =
                  primitive_processor_->GetExpansionTriangleListIndexBuffer();
              index_offset = 0;
              index_type = MTL::IndexTypeUInt32;
            } else {
              index_buffer = primitive_processor_->GetBuiltinIndexBuffer();
              index_offset =
                  primitive_processing_result.host_index_buffer_handle;
            }
          }
          break;
        default:
          XELOGE("SPIRV-Cross: Unsupported index buffer type {}",
                 uint32_t(primitive_processing_result.index_buffer_type));
          return false;
      }
      if (!index_buffer) {
        XELOGE("SPIRV-Cross: Index buffer is null");
        return false;
      }
      UseRenderEncoderResource(index_buffer, MTL::ResourceUsageRead);
      current_render_encoder_->drawIndexedPrimitives(
          mtl_primitive, NS::UInteger(draw_index_count), index_type,
          index_buffer, NS::UInteger(index_offset));
    }
  }
  ++draw_stats_.encoded;
  draw_stats_.encoded_vertices +=
      primitive_processing_result.host_draw_vertex_count;
  metal_gpu_pass_timer.NoteDraw(
      metal_gpu_pass_record_, vertex_translation, pixel_translation,
      primitive_processing_result.host_draw_vertex_count);

  // Handle memexport.
  if (memexport_used && shared_memory_) {
    for (const draw_util::MemExportRange& memexport_range : memexport_ranges_) {
      shared_memory_->RangeWrittenByGpu(
          memexport_range.base_address_dwords << 2, memexport_range.size_bytes);
    }
  }

  ++current_draw_index_;
  return true;
}

bool MetalCommandProcessor::IssueCopy() {
  // Finish any in-flight rendering so render target contents are visible to
  // resolve logic.
  EndRenderEncoder();
  MTL::CommandBuffer* copy_command_buffer = EnsureCommandBuffer();
  if (!copy_command_buffer) {
    XELOGE("MetalCommandProcessor::IssueCopy: failed to get command buffer");
    return false;
  }

  if (!render_target_cache_) {
    XELOGW("MetalCommandProcessor::IssueCopy - No render target cache");
    return true;
  }

  uint32_t written_address = 0;
  uint32_t written_length = 0;

  if (!render_target_cache_->Resolve(*memory_, written_address, written_length,
                                     copy_command_buffer)) {
    XELOGE("MetalCommandProcessor::IssueCopy - Resolve failed");
    return false;
  }
  if (metal_perf_counters.frame_detail_frames.load(std::memory_order_relaxed)) {
    const uint32_t* values = register_file_->values;
    XELOGI(
        "Frame detail resolve after draw {}: COPY_CONTROL {:08X} DEST_BASE "
        "{:08X} DEST_PITCH {:08X} DEST_INFO {:08X} SURFACE_SLICE {:08X} "
        "SURFACE_INFO {:08X} COLOR_INFO {:08X} {:08X} {:08X} {:08X} "
        "DEPTH_INFO {:08X}; wrote {:08X} to {:08X}",
        frame_detail_draws_, values[XE_GPU_REG_RB_COPY_CONTROL],
        values[XE_GPU_REG_RB_COPY_DEST_BASE],
        values[XE_GPU_REG_RB_COPY_DEST_PITCH],
        values[XE_GPU_REG_RB_COPY_DEST_INFO],
        values[XE_GPU_REG_RB_COPY_SURFACE_SLICE],
        values[XE_GPU_REG_RB_SURFACE_INFO], values[XE_GPU_REG_RB_COLOR_INFO],
        values[XE_GPU_REG_RB_COLOR1_INFO], values[XE_GPU_REG_RB_COLOR2_INFO],
        values[XE_GPU_REG_RB_COLOR3_INFO], values[XE_GPU_REG_RB_DEPTH_INFO],
        written_address, written_address + written_length);
    // Depth resolves (the scene depth, shadow maps) are sampled once done.
    if (register_file_->Get<reg::RB_COPY_CONTROL>().copy_src_select >=
            xenos::kMaxColorRenderTargets &&
        written_length) {
      const auto dest_pitch = register_file_->Get<reg::RB_COPY_DEST_PITCH>();
      const uint32_t dest_endian = uint32_t(
          register_file_->Get<reg::RB_COPY_DEST_INFO>().copy_dest_endian);
      MetalResolveProbe probe;
      probe.address = values[XE_GPU_REG_RB_COPY_DEST_BASE] & 0x1FFFFFFF;
      probe.written_begin = written_address;
      probe.written_end = written_address + written_length;
      probe.width = dest_pitch.copy_dest_pitch;
      probe.height = dest_pitch.copy_dest_height;
      probe.endian =
          dest_endian <= 3 ? xenos::Endian(dest_endian) : xenos::Endian::kNone;
      probe.is_float = register_file_->Get<reg::RB_DEPTH_INFO>().depth_format ==
                       xenos::DepthRenderTargetFormat::kD24FS8;
      probe.draw = frame_detail_draws_;
      probe.submission = GetCurrentSubmission();
      // The buffers are tracked, so the blit runs after the dump and resolve.
      MTL::BlitCommandEncoder* snapshot_blit =
          copy_command_buffer->blitCommandEncoder();
      if (snapshot_blit && shared_memory_ && shared_memory_->GetBuffer()) {
        probe.snapshot =
            device_->newBuffer(written_length, MTL::ResourceStorageModeShared);
        if (probe.snapshot) {
          snapshot_blit->copyFromBuffer(shared_memory_->GetBuffer(),
                                        written_address, probe.snapshot, 0,
                                        written_length);
        }
        MTL::Buffer* edram = render_target_cache_->GetEdramBuffer();
        const auto surface_info = register_file_->Get<reg::RB_SURFACE_INFO>();
        if (edram && !probe.is_float &&
            surface_info.msaa_samples == xenos::MsaaSamples::k1X &&
            texture_cache_ && texture_cache_->draw_resolution_scale_x() == 1 &&
            texture_cache_->draw_resolution_scale_y() == 1) {
          const uint32_t tile_bytes = xenos::kEdramTileWidthSamples *
                                      xenos::kEdramTileHeightSamples *
                                      sizeof(uint32_t);
          probe.edram_base_tiles =
              register_file_->Get<reg::RB_DEPTH_INFO>().depth_base;
          probe.edram_pitch_tiles =
              (surface_info.surface_pitch + xenos::kEdramTileWidthSamples - 1) /
              xenos::kEdramTileWidthSamples;
          const uint32_t tiles = std::min(
              probe.edram_pitch_tiles *
                  ((probe.height + xenos::kEdramTileHeightSamples - 1) /
                   xenos::kEdramTileHeightSamples),
              xenos::kEdramTileCount);
          if (tiles) {
            probe.edram_snapshot = device_->newBuffer(
                tiles * tile_bytes, MTL::ResourceStorageModeShared);
          }
          if (probe.edram_snapshot) {
            // EDRAM addressing wraps around.
            const uint32_t first_tiles = std::min(
                tiles, xenos::kEdramTileCount - probe.edram_base_tiles);
            snapshot_blit->copyFromBuffer(
                edram, probe.edram_base_tiles * tile_bytes,
                probe.edram_snapshot, 0, first_tiles * tile_bytes);
            if (tiles > first_tiles) {
              snapshot_blit->copyFromBuffer(edram, 0, probe.edram_snapshot,
                                            first_tiles * tile_bytes,
                                            (tiles - first_tiles) * tile_bytes);
            }
          }
        }
      }
      if (snapshot_blit) {
        snapshot_blit->endEncoding();
      }
      resolve_probes_.push_back(probe);
    }
  }

  ReadbackResolveMode readback_mode = GetReadbackResolveMode();
  bool do_readback = (readback_mode != ReadbackResolveMode::kDisabled);
  bool readback_scaled = false;
  bool readback_scaled_gpu = false;
  bool use_gpu_downscale = false;
  bool readback_scheduled = false;
  uint32_t write_index = 0;
  uint32_t read_index = 0;
  bool use_delayed_sync = false;
  bool wait_for_completion = false;
  bool should_copy = false;
  bool is_cache_miss = false;
  uint32_t source_length = 0;
  uint32_t readback_length = 0;
  uint32_t tile_count = 0;
  uint32_t pixel_size_log2 = 0;
  uint32_t scale_x = 1;
  uint32_t scale_y = 1;
  bool half_pixel_offset = false;
  uint32_t source_offset_bytes = 0;
  uint64_t scaled_range_offset_bytes = 0;
  uint64_t readback_base_offset_bytes = 0;
  uint64_t scaled_copy_length = 0;
  size_t source_buffer_binding_offset = 0;
  uint64_t source_offset_bytes_log = 0;

  if (do_readback) {
    // Early check: if destination memory is not accessible, skip readback.
    VirtualHeap* physical_heap = memory_->GetPhysicalHeap();
    bool memory_accessible = false;
    if (physical_heap) {
      HeapAllocationInfo alloc_info;
      if (physical_heap->QueryRegionInfo(written_address, &alloc_info) &&
          (alloc_info.state & kMemoryAllocationCommit) &&
          IsWritableProtect(alloc_info.protect)) {
        uint32_t end_address = written_address + written_length;
        uint32_t region_end = alloc_info.base_address + alloc_info.region_size;
        if (end_address <= region_end) {
          memory_accessible = true;
        }
      }
    }
    if (!memory_accessible) {
      do_readback = false;
    }
  }
  if (!written_length) {
    // Keep the submission open for no-op copies and let primary-buffer end,
    // swap, or explicit sync points choose the commit boundary.
    return true;
  }

  // Track this resolved region so the trace player can avoid overwriting it
  // with stale MemoryRead commands from the trace file.
  MarkResolvedMemory(written_address, written_length);

  // The resolve writes into guest memory via the shared memory buffer.
  // Any cached views of this memory (especially textures sourced from it)
  // must be invalidated, otherwise subsequent render-to-texture / postprocess
  // passes will sample stale host textures and produce corrupted output.
  //   if (shared_memory_) {
  //     shared_memory_->MemoryInvalidationCallback(written_address,
  //     written_length, true);
  //   }
  //   if (primitive_processor_) {
  //     primitive_processor_->MemoryInvalidationCallback(written_address,
  //     written_length, true);
  //   }

  // Keep copy-only resolve bursts open so multiple resolves can be coalesced,
  // but commit draw-containing submissions so subsequent work observes the
  // resolved guest memory immediately.
  // With metal_resolve_keep_command_buffer, a draw-containing one stays open
  // too: IssueDrawMsl loads the textures that read the output into it after
  // the resolve, and the guest's fences are written when it completes.
  if ((current_draw_index_ == 0
#if METAL_SHADER_CONVERTER_AVAILABLE
       && command_buffer_draw_rings_.empty()
#endif
       ) ||
      (UseSpirvCrossPath() && cvars::metal_resolve_keep_command_buffer &&
       cvars::metal_texture_upload_via_blit)) {
    copy_resolve_writes_pending_ = true;
    return true;
  }

  // Resolve touched guest memory in a draw-containing submission; commit now
  // so following packets don't observe stale resolve results.
#if METAL_SHADER_CONVERTER_AVAILABLE
  ScheduleDrawRingRelease(copy_command_buffer);
#else
  if (UseSpirvCrossPath()) {
    ScheduleSpirvUniformBufferRelease(copy_command_buffer);
  }
#endif
  AttachShaderDoneFences(copy_command_buffer);
  copy_command_buffer->commit();
  copy_command_buffer->release();
  current_command_buffer_ = nullptr;
#if METAL_SHADER_CONVERTER_AVAILABLE
  SetActiveDrawRing(nullptr);
#endif
  current_draw_index_ = 0;
  copy_resolve_writes_pending_ = false;

  return true;
}

void MetalCommandProcessor::OnGammaRamp256EntryTableValueWritten() {
  gamma_ramp_256_entry_table_up_to_date_ = false;
}

void MetalCommandProcessor::OnGammaRampPWLValueWritten() {
  gamma_ramp_pwl_up_to_date_ = false;
}

void MetalCommandProcessor::WriteRegister(uint32_t index, uint32_t value) {
  CommandProcessor::WriteRegister(index, value);

  if (index >= XE_GPU_REG_SHADER_CONSTANT_000_X &&
      index <= XE_GPU_REG_SHADER_CONSTANT_511_W) {
    const uint32_t float_constant_index =
        (index - XE_GPU_REG_SHADER_CONSTANT_000_X) >> 2;
    const uint32_t stage_constant_index = float_constant_index & 0xFF;
    const uint32_t map_index = stage_constant_index >> 6;
    const uint64_t map_bit = uint64_t(1) << (stage_constant_index & 63);
    if (float_constant_index >= 256) {
      if (msl_current_float_constant_map_pixel_[map_index] & map_bit) {
        msl_float_constants_dirty_pixel_ = true;
      }
    } else {
      if (msl_current_float_constant_map_vertex_[map_index] & map_bit) {
        msl_float_constants_dirty_vertex_ = true;
      }
    }
  } else if (index >= XE_GPU_REG_SHADER_CONSTANT_BOOL_000_031 &&
             index <= XE_GPU_REG_SHADER_CONSTANT_LOOP_31) {
    msl_bool_loop_constants_dirty_ = true;
  } else if (index >= XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0 &&
             index <= XE_GPU_REG_SHADER_CONSTANT_FETCH_31_5) {
    msl_fetch_constants_dirty_ = true;
    if (texture_cache_) {
      texture_cache_->TextureFetchConstantWritten(
          (index - XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0) / 6);
    }
  }
}

MTL::CommandBuffer* MetalCommandProcessor::EnsureCommandBuffer() {
  ProcessCompletedSubmissions();
  if (current_command_buffer_) {
    return current_command_buffer_;
  }
  if (!command_queue_) {
    XELOGE("EnsureCommandBuffer: no command queue");
    return nullptr;
  }

  EnsureCommandBufferAutoreleasePool();

  // Note: commandBuffer() returns an autoreleased object, we must retain it.
  current_command_buffer_ = command_queue_->commandBuffer();
  if (!current_command_buffer_) {
    XELOGE("EnsureCommandBuffer: failed to create command buffer");
    DrainCommandBufferAutoreleasePool();
    return nullptr;
  }
  current_command_buffer_->retain();
  current_command_buffer_->setLabel(
      NS::String::string("XeniaCommandBuffer", NS::UTF8StringEncoding));

#if !METAL_SHADER_CONVERTER_AVAILABLE
  if (UseSpirvCrossPath() &&
      (!cvars::metal_inline_shader_constants ||
       cvars::metal_shader_compiler == "spirv2air") &&
      !EnsureSpirvUniformBuffer()) {
    static auto last_ensure_uniforms_fail_log =
        std::chrono::steady_clock::time_point{};
    const auto now = std::chrono::steady_clock::now();
    if (now - last_ensure_uniforms_fail_log >= std::chrono::seconds(1)) {
      last_ensure_uniforms_fail_log = now;
      XELOGE(
          "EnsureCommandBuffer: failed to prepare SPIRV-Cross uniforms "
          "buffer");
    }
    current_command_buffer_->release();
    current_command_buffer_ = nullptr;
    DrainCommandBufferAutoreleasePool();
    return nullptr;
  }
#endif  // !METAL_SHADER_CONVERTER_AVAILABLE

  ++submission_current_;
  pending_completion_handlers_.fetch_add(1, std::memory_order_relaxed);
  current_command_buffer_->addCompletedHandler(
      [this](MTL::CommandBuffer* completed_buffer) {
        completed_command_buffers_.fetch_add(1, std::memory_order_relaxed);
        const double metal_perf_gpu_seconds =
            completed_buffer->GPUEndTime() - completed_buffer->GPUStartTime();
        if (metal_perf_gpu_seconds > 0.0) {
          metal_perf_counters.gpu_us.fetch_add(
              uint64_t(metal_perf_gpu_seconds * 1e6),
              std::memory_order_relaxed);
        }
        metal_perf_counters.gpu_command_buffers.fetch_add(
            1, std::memory_order_relaxed);
        pending_completion_handlers_.fetch_sub(1, std::memory_order_relaxed);
      });

  if (primitive_processor_) {
    primitive_processor_->BeginSubmission();
  }
  if (texture_cache_) {
    texture_cache_->BeginSubmission(submission_current_);
  }
  if (primitive_processor_ && !frame_open_) {
    primitive_processor_->BeginFrame();
    if (render_target_cache_) {
      render_target_cache_->BeginFrame();
    }
    if (texture_cache_) {
      texture_cache_->BeginFrame();
    }
    frame_open_ = true;
  }

  return current_command_buffer_;
}

void MetalCommandProcessor::ProcessCompletedSubmissions() {
  const uint64_t completed =
      completed_command_buffers_.load(std::memory_order_relaxed);
  if (completed <= submission_completed_processed_) {
    return;
  }
  submission_completed_processed_ = completed;
  if (primitive_processor_) {
    primitive_processor_->CompletedSubmissionUpdated();
  }
  if (texture_cache_) {
    texture_cache_->CompletedSubmissionUpdated(completed);
  }
  ProcessCompletedZPDQueries();
  LogResolveProbes();
}

void MetalCommandProcessor::LogResolveProbes() {
  if (resolve_probes_.empty() || !shared_memory_ ||
      !shared_memory_->GetBuffer()) {
    return;
  }
  const uint64_t completed = GetCompletedSubmission();
  // The resolve's writes are visible once its command buffer completed.
  std::atomic_thread_fence(std::memory_order_acquire);
  const uint8_t* memory =
      static_cast<const uint8_t*>(shared_memory_->GetBuffer()->contents());
  for (auto probe = resolve_probes_.begin(); probe != resolve_probes_.end();) {
    if (probe->submission > completed || !probe->width || !probe->height) {
      if (probe->submission > completed) {
        ++probe;
      } else {
        if (probe->snapshot) {
          probe->snapshot->release();
        }
        if (probe->edram_snapshot) {
          probe->edram_snapshot->release();
        }
        probe = resolve_probes_.erase(probe);
      }
      continue;
    }
    // The snapshot taken right after the resolve if there is one.
    const uint8_t* probe_memory = memory;
    uint32_t probe_memory_base = 0;
    if (probe->snapshot) {
      probe_memory = static_cast<const uint8_t*>(probe->snapshot->contents());
      probe_memory_base = probe->written_begin;
    }
    // A 16-wide grid, as many rows as keep it about square.
    const uint32_t probe_cols = 16;
    const uint32_t probe_rows =
        probe->pass
            ? 0
            : std::clamp<uint32_t>(
                  (16 * probe->height + probe->width / 2) / probe->width, 1,
                  16);
    if (!probe->pass) {
      XELOGI(
          "Frame detail resolve probe after draw {}: {:08X} ({:08X} to {:08X} "
          "written), {}x{} {} depth, {}x{} samples ('-' outside what was "
          "written):",
          probe->draw, probe->address, probe->written_begin, probe->written_end,
          probe->width, probe->height, probe->is_float ? "20e4 float" : "unorm",
          probe_cols, probe_rows);
    }
    for (uint32_t row = 0; row < probe_rows; ++row) {
      const uint32_t y = (2 * row + 1) * probe->height / (2 * probe_rows);
      std::string probe_line;
      for (uint32_t col = 0; col < probe_cols; ++col) {
        const uint32_t x = (2 * col + 1) * probe->width / (2 * probe_cols);
        const uint32_t texel_address =
            probe->address + uint32_t(texture_util::GetTiledOffset2D(
                                 int32_t(x), int32_t(y), probe->width, 2));
        if (texel_address < probe->written_begin ||
            texel_address + 4 > probe->written_end ||
            texel_address + 4 > SharedMemory::kBufferSize) {
          probe_line += " -";
          continue;
        }
        uint32_t texel;
        std::memcpy(&texel, probe_memory + (texel_address - probe_memory_base),
                    sizeof(texel));
        const uint32_t depth24 = xenos::GpuSwap(texel, probe->endian) >> 8;
        fmt::format_to(std::back_inserter(probe_line), " {:.4f}",
                       probe->is_float ? xenos::Float20e4To32(depth24)
                                       : float(depth24) / 16777215.0f);
      }
      XELOGI("Frame detail resolve probe {:08X} y {}:{}", probe->address, y,
             probe_line);
    }
    if (!probe->is_float) {
      // Unorm depth (shadow maps): where the clear value (1.0) is missing, to
      // the texel at the edges, which the grid above skips.
      auto read_depth24 = [&](uint32_t x, uint32_t y, uint32_t& depth24) {
        const uint32_t texel_address =
            probe->address + uint32_t(texture_util::GetTiledOffset2D(
                                 int32_t(x), int32_t(y), probe->width, 2));
        if (texel_address < probe->written_begin ||
            texel_address + 4 > probe->written_end ||
            texel_address + 4 > SharedMemory::kBufferSize) {
          return false;
        }
        uint32_t texel;
        std::memcpy(&texel, probe_memory + (texel_address - probe_memory_base),
                    sizeof(texel));
        depth24 = xenos::GpuSwap(texel, probe->endian) >> 8;
        return true;
      };
      uint32_t written_rows = probe->pass ? probe->height : 0;
      for (uint32_t y = 0; y < probe->height && !probe->pass; ++y) {
        uint32_t depth24;
        if (read_depth24(0, y, depth24)) {
          written_rows = y + 1;
        }
      }
      // The EDRAM the resolve read: the dumped host render target, 1x.
      const uint32_t edram_tile_bytes = xenos::kEdramTileWidthSamples *
                                        xenos::kEdramTileHeightSamples *
                                        sizeof(uint32_t);
      auto read_edram = [&](uint32_t x, uint32_t y, uint32_t& texel) {
        if (!probe->edram_snapshot) {
          return false;
        }
        const uint32_t tile =
            (y / xenos::kEdramTileHeightSamples) * probe->edram_pitch_tiles +
            x / xenos::kEdramTileWidthSamples;
        if (x >= probe->edram_pitch_tiles * xenos::kEdramTileWidthSamples ||
            (tile + 1) * edram_tile_bytes > probe->edram_snapshot->length()) {
          return false;
        }
        // Depth tiles have their 40-sample halves swapped (edram.xesli).
        uint32_t tile_x = x % xenos::kEdramTileWidthSamples;
        tile_x = (tile_x + xenos::kEdramTileWidthSamples / 2) %
                 xenos::kEdramTileWidthSamples;
        std::memcpy(
            &texel,
            static_cast<const uint8_t*>(probe->edram_snapshot->contents()) +
                tile * edram_tile_bytes +
                ((y % xenos::kEdramTileHeightSamples) *
                     xenos::kEdramTileWidthSamples +
                 tile_x) *
                    sizeof(uint32_t),
            sizeof(texel));
        return true;
      };
      auto read_edram_depth24 = [&](uint32_t x, uint32_t y, uint32_t& depth24) {
        uint32_t texel;
        if (!read_edram(x, y, texel)) {
          return false;
        }
        depth24 = texel >> 8;
        return true;
      };
      if (written_rows && !probe->pass) {
        std::string texels_line;
        const uint32_t positions[][2] = {{2, written_rows / 2},
                                         {6, written_rows / 2},
                                         {probe->width - 3, written_rows / 2},
                                         {probe->width / 2, 2},
                                         {probe->width / 2, 6},
                                         {probe->width / 2, written_rows - 3}};
        for (const auto& position : positions) {
          uint32_t depth24 = 0, edram_texel = 0;
          const bool in_resolve =
              read_depth24(position[0], position[1], depth24);
          const bool in_edram =
              read_edram(position[0], position[1], edram_texel);
          fmt::format_to(std::back_inserter(texels_line), " ({},{}) {} / {};",
                         position[0], position[1],
                         in_resolve ? fmt::format("{:06X}", depth24) : "-",
                         in_edram ? fmt::format("{:08X}", edram_texel) : "-");
        }
        XELOGI(
            "Frame detail resolve texels {:08X} (x,y depth written / EDRAM "
            "dword read, {}):{}",
            probe->address,
            probe->snapshot ? "copied at the resolve" : "read at completion",
            texels_line);
      }
      static const char* const kEdgeNames[] = {"left", "right", "top",
                                               "bottom"};
      auto log_coverage = [&](const char* source, const auto& read) {
        for (uint32_t edge = 0; edge < 4 && written_rows; ++edge) {
          std::string edge_line;
          for (uint32_t depth_in = 0; depth_in < 8; ++depth_in) {
            const bool vertical = edge < 2;
            const uint32_t length = vertical ? written_rows : probe->width;
            uint32_t not_clear = 0;
            uint32_t min_depth24 = 0xFFFFFF;
            for (uint32_t i = 0; i < length; ++i) {
              uint32_t x = vertical
                               ? (edge ? probe->width - 1 - depth_in : depth_in)
                               : i;
              uint32_t y = vertical ? i
                                    : (edge == 3 ? written_rows - 1 - depth_in
                                                 : depth_in);
              uint32_t depth24;
              if (!read(x, y, depth24)) {
                continue;
              }
              if (depth24 != 0xFFFFFF) {
                ++not_clear;
                min_depth24 = std::min(min_depth24, depth24);
              }
            }
            fmt::format_to(std::back_inserter(edge_line), " {}/{} min {:.4f};",
                           not_clear, length, float(min_depth24) / 16777215.0f);
          }
          XELOGI(
              "Frame detail {} edge {:08X} {} (8 rows or columns in, texels "
              "not 1.0 / all):{}",
              source, probe->address, kEdgeNames[edge], edge_line);
        }
        // 8x8 blocks: '#' all 1.0, '.' none, '+' some, '-' not written.
        for (uint32_t block_y = 0; block_y * 8 < written_rows; ++block_y) {
          std::string map_line;
          for (uint32_t block_x = 0; block_x * 8 < probe->width; ++block_x) {
            uint32_t clear = 0;
            uint32_t written = 0;
            for (uint32_t y = block_y * 8;
                 y < std::min(block_y * 8 + 8, written_rows); ++y) {
              for (uint32_t x = block_x * 8;
                   x < std::min(block_x * 8 + 8, probe->width); ++x) {
                uint32_t depth24;
                if (read(x, y, depth24)) {
                  ++written;
                  clear += depth24 == 0xFFFFFF;
                }
              }
            }
            map_line += !written           ? '-'
                        : clear == written ? '#'
                        : !clear           ? '.'
                                           : '+';
          }
          XELOGI("Frame detail {} map {:08X} y {}: {}", source, probe->address,
                 block_y * 8, map_line);
        }
      };
      if (probe->pass) {
        log_coverage(fmt::format("pass {} start edram", probe->pass).c_str(),
                     read_edram_depth24);
      } else {
        log_coverage("resolve", read_depth24);
        if (probe->edram_snapshot) {
          log_coverage("edram", read_edram_depth24);
        }
      }
    }
    if (probe->snapshot) {
      probe->snapshot->release();
    }
    if (probe->edram_snapshot) {
      probe->edram_snapshot->release();
    }
    probe = resolve_probes_.erase(probe);
  }
}

bool MetalCommandProcessor::ExecutePacketType3_EVENT_WRITE_ZPD(uint32_t packet,
                                                               uint32_t count) {
  // D3D sets both ZPass lanes (ZFail in older versions, 4D5307D5) to
  // 0xFFFFFEED before the event that ends a query, and waits for them to
  // change. The report is little-endian.
  const uint32_t kQueryFinished = xe::byte_swap(0xFFFFFEEDu);
  const uint32_t sample_count_addr =
      register_file_->values[XE_GPU_REG_RB_SAMPLE_COUNT_ADDR];
  auto* sample_counts =
      memory_->TranslatePhysical<xenos::xe_gpu_depth_sample_counts*>(
          sample_count_addr);
  const bool is_end =
      sample_counts && ((sample_counts->ZPass_A == kQueryFinished &&
                         sample_counts->ZPass_B == kQueryFinished) ||
                        (sample_counts->ZFail_A == kQueryFinished &&
                         sample_counts->ZFail_B == kQueryFinished));
  metal_perf_counters.occlusion_query_events.fetch_add(
      1, std::memory_order_relaxed);
  if (is_end) {
    metal_perf_counters.occlusion_queries.fetch_add(1,
                                                    std::memory_order_relaxed);
  }

  if (zpd_segment_open_) {
    // Disabled at the segment's own offset: the offset given is still written
    // by the pass, so 0 would clear slot 0.
    current_render_encoder_->setVisibilityResultMode(
        MTL::VisibilityResultModeDisabled,
        NS::UInteger(zpd_query_.slots.back().first) * sizeof(uint64_t));
    zpd_segment_open_ = false;
  }
  const bool real_queries =
      cvars::occlusion_query_enable && UseSpirvCrossPath() && sample_counts &&
      zpd_visibility_pool_.EnsureInitialized(device_, 4096);
  if (zpd_query_active_ && (!real_queries || !is_end)) {
    // Abandoned (switched off, or begun again): its slots are only released.
    zpd_query_active_ = false;
    zpd_query_.address = 0;
    zpd_query_.submission = submission_current_;
    if (!zpd_query_.slots.empty()) {
      zpd_queries_in_flight_.push_back(std::move(zpd_query_));
    }
  }
  if (!real_queries) {
    if (metal_perf_counters.frame_detail_frames.load(
            std::memory_order_relaxed)) {
      XELOGI("Frame detail query {} at {:08X} after draw {} (faked)",
             is_end ? "end" : "begin", sample_count_addr, frame_detail_draws_);
    }
    return CommandProcessor::ExecutePacketType3_EVENT_WRITE_ZPD(packet, count);
  }

  assert_true(count == 1);
  const uint32_t initiator = reader_.ReadAndSwap<uint32_t>();
  WriteEventInitiator(initiator & 0x3F);
  ProcessCompletedZPDQueries();
  if (!is_end) {
    // D3D subtracts the begin report from the end report.
    std::memset(sample_counts, 0, sizeof(*sample_counts));
    zpd_query_ = {};
    zpd_query_active_ = true;
    zpd_compile_pending_at_begin_ = draw_stats_.compile_pending;
    if (metal_perf_counters.frame_detail_frames.load(
            std::memory_order_relaxed)) {
      XELOGI("Frame detail query begin at {:08X} after draw {}",
             sample_count_addr, frame_detail_draws_);
    }
    return true;
  }

  MetalZPDQuery query;
  if (zpd_query_active_) {
    query = std::move(zpd_query_);
    zpd_query_active_ = false;
    if (draw_stats_.compile_pending != zpd_compile_pending_at_begin_) {
      query.inexact = true;
    }
  } else {
    query.inexact = true;
  }
  if (zpd_results_by_address_.size() >= 4096 &&
      !zpd_results_by_address_.count(sample_count_addr)) {
    zpd_results_by_address_.clear();
  }
  std::pair<uint32_t, uint64_t>& address_result =
      zpd_results_by_address_[sample_count_addr];
  query.address = sample_count_addr;
  query.ticket = ++zpd_query_ends_;
  address_result.second = query.ticket;
  query.ordinal = zpd_frame_query_ends_++;
  if (query.ordinal >= zpd_results_by_ordinal_.size() && query.ordinal < 4096) {
    zpd_results_by_ordinal_.resize(query.ordinal + 1, UINT32_MAX);
  }
  uint32_t samples = 0;
  if (query.slots.empty() && !query.inexact) {
    // Nothing was drawn: no samples, known now.
    address_result.first = 0;
    if (query.ordinal < zpd_results_by_ordinal_.size()) {
      zpd_results_by_ordinal_[query.ordinal] = 0;
    }
    metal_perf_counters.zpd_queries_measured.fetch_add(
        1, std::memory_order_relaxed);
    metal_perf_counters.zpd_queries_hidden.fetch_add(1,
                                                     std::memory_order_relaxed);
  } else {
    // Unknown until the GPU is done: guess the last count at this address,
    // or at this place in the frame if that has been closer, or visible.
    query.address_guess = std::max<uint32_t>(address_result.first, 1);
    if (query.ordinal < zpd_results_by_ordinal_.size()) {
      query.ordinal_guess = zpd_results_by_ordinal_[query.ordinal];
    }
    samples = zpd_ordinal_score_ > 0 && query.ordinal_guess != UINT32_MAX
                  ? std::max<uint32_t>(query.ordinal_guess, 1)
                  : query.address_guess;
    query.guess = samples;
    query.submission = submission_current_;
    zpd_queries_in_flight_.push_back(std::move(query));
  }
  sample_counts->Total_A = samples;
  sample_counts->Total_B = 0;
  sample_counts->ZFail_A = 0;
  sample_counts->ZFail_B = 0;
  sample_counts->StencilFail_A = 0;
  sample_counts->StencilFail_B = 0;
  sample_counts->ZPass_B = 0;
  // The lane D3D waits on, last.
  std::atomic_thread_fence(std::memory_order_release);
  sample_counts->ZPass_A = samples;
  if (metal_perf_counters.frame_detail_frames.load(std::memory_order_relaxed)) {
    XELOGI(
        "Frame detail query end at {:08X} after draw {}: wrote {} (0 if "
        "nothing was drawn, else a guess)",
        sample_count_addr, frame_detail_draws_, samples);
  }
  return true;
}

void MetalCommandProcessor::ProcessCompletedZPDQueries() {
  if (zpd_queries_in_flight_.empty()) {
    return;
  }
  const uint64_t completed = GetCompletedSubmission();
  // The visibility results were copied before the command buffer completed.
  std::atomic_thread_fence(std::memory_order_acquire);
  const uint64_t scale_area =
      texture_cache_ ? uint64_t(texture_cache_->draw_resolution_scale_x()) *
                           texture_cache_->draw_resolution_scale_y()
                     : 1;
  while (!zpd_queries_in_flight_.empty() &&
         zpd_queries_in_flight_.front().submission <= completed) {
    const MetalZPDQuery& query = zpd_queries_in_flight_.front();
    uint64_t samples = 0;
    bool counted = !query.inexact;
    for (const auto& [slot_index, slot_generation] : query.slots) {
      uint64_t slot_samples = 0;
      if (zpd_visibility_pool_.IsGenerationCurrent(slot_index,
                                                   slot_generation) &&
          zpd_visibility_pool_.Read(slot_index, slot_samples)) {
        samples += slot_samples;
      } else {
        counted = false;
      }
      zpd_visibility_pool_.Release(slot_index, slot_generation);
    }
    if (query.address && !counted) {
      metal_perf_counters.zpd_queries_guessed.fetch_add(
          1, std::memory_order_relaxed);
    } else if (query.address) {
      if (scale_area > 1 && samples) {
        samples =
            std::max<uint64_t>(1, (samples + scale_area / 2) / scale_area);
      }
      const uint32_t count = uint32_t(std::min<uint64_t>(samples, UINT32_MAX));
      metal_perf_counters.zpd_queries_measured.fetch_add(
          1, std::memory_order_relaxed);
      if (!count) {
        metal_perf_counters.zpd_queries_hidden.fetch_add(
            1, std::memory_order_relaxed);
      }
      if (query.ordinal_guess != UINT32_MAX) {
        // Score the two guesses against the count, within -8...8.
        auto miss = [count](uint32_t guess) {
          return guess > count ? guess - count : count - guess;
        };
        const uint32_t ordinal_miss =
            miss(std::max<uint32_t>(query.ordinal_guess, 1));
        const uint32_t address_miss = miss(query.address_guess);
        if (ordinal_miss < address_miss) {
          zpd_ordinal_score_ = std::min(zpd_ordinal_score_ + 1, 8);
        } else if (address_miss < ordinal_miss) {
          zpd_ordinal_score_ = std::max(zpd_ordinal_score_ - 1, -8);
        }
      }
      if (query.ordinal < zpd_results_by_ordinal_.size()) {
        zpd_results_by_ordinal_[query.ordinal] = count;
      }
      auto address_result = zpd_results_by_address_.find(query.address);
      if (address_result != zpd_results_by_address_.end()) {
        address_result->second.first = count;
        // Only over the guess of the address's latest query, if the guest
        // has not written the address since (to begin another query).
        auto* sample_counts =
            memory_->TranslatePhysical<xenos::xe_gpu_depth_sample_counts*>(
                query.address);
        if (address_result->second.second == query.ticket &&
            count != query.guess && sample_counts->ZPass_A == query.guess &&
            sample_counts->Total_A == query.guess) {
          sample_counts->Total_A = count;
          std::atomic_thread_fence(std::memory_order_release);
          sample_counts->ZPass_A = count;
          metal_perf_counters.zpd_queries_corrected.fetch_add(
              1, std::memory_order_relaxed);
        }
        if (metal_perf_counters.frame_detail_frames.load(
                std::memory_order_relaxed)) {
          XELOGI(
              "Frame detail query at {:08X} counted {} (guessed {}; query {} "
              "of the frame, by address {}, by place {}, score {})",
              query.address, count, query.guess, query.ordinal,
              query.address_guess, int64_t(int32_t(query.ordinal_guess)),
              zpd_ordinal_score_);
        }
      }
    }
    zpd_queries_in_flight_.pop_front();
  }
}

void MetalCommandProcessor::EnsureCommandBufferAutoreleasePool() {
  if (command_buffer_autorelease_pool_) {
    return;
  }
  command_buffer_autorelease_pool_ = NS::AutoreleasePool::alloc()->init();
}

void MetalCommandProcessor::DrainCommandBufferAutoreleasePool() {
  if (!command_buffer_autorelease_pool_) {
    return;
  }
  command_buffer_autorelease_pool_->release();
  command_buffer_autorelease_pool_ = nullptr;
}

void MetalCommandProcessor::ResetMslRenderEncoderStateCache() {
  msl_bound_vertex_texture_count_ = 0;
  msl_bound_pixel_texture_count_ = 0;
  msl_bound_vertex_sampler_count_ = 0;
  msl_bound_pixel_sampler_count_ = 0;
  msl_bound_vertex_texture_binding_uid_ = 0;
  msl_bound_pixel_texture_binding_uid_ = 0;
  msl_bound_vertex_sampler_binding_uid_ = 0;
  msl_bound_pixel_sampler_binding_uid_ = 0;
  msl_bound_vertex_textures_.fill(nullptr);
  msl_bound_pixel_textures_.fill(nullptr);
  msl_bound_vertex_samplers_.fill(nullptr);
  msl_bound_pixel_samplers_.fill(nullptr);
  msl_bound_shared_memory_buffer_ = nullptr;
  msl_bound_vertex_argument_buffer_ = nullptr;
  msl_bound_pixel_argument_buffer_ = nullptr;
  msl_bound_vertex_argument_buffer_offset_ = 0;
  msl_bound_pixel_argument_buffer_offset_ = 0;
  msl_bound_vertex_argument_buffer_offset_valid_ = false;
  msl_bound_pixel_argument_buffer_offset_valid_ = false;
  msl_bound_null_buffer_ = nullptr;
  msl_bound_uniforms_buffer_ = nullptr;
  msl_bound_uniforms_vs_base_offset_ = 0;
  msl_bound_uniforms_ps_base_offset_ = 0;
  msl_bound_uniforms_offsets_valid_ = false;
  msl_inline_constants_bound_versions_.fill(0);
  msl_bound_pipeline_state_ = nullptr;
  msl_viewport_valid_ = false;
  msl_scissor_valid_ = false;
  msl_rasterizer_state_valid_ = false;
  msl_depth_stencil_state_ = nullptr;
  msl_stencil_reference_valid_ = false;
  msl_stencil_reference_ = 0;
  ff_blend_factor_valid_ = false;
  ResetRenderEncoderResourceUsage();
}

void MetalCommandProcessor::ResetMslCrossEncoderReuseCaches() {
  msl_last_argbuf_vertex_textures_.fill(nullptr);
  msl_last_argbuf_vertex_texture_count_ = 0;
  msl_last_argbuf_vertex_samplers_.fill(nullptr);
  msl_last_argbuf_vertex_sampler_count_ = 0;
  msl_last_argbuf_vertex_buffer_ = nullptr;
  msl_last_argbuf_vertex_offset_ = 0;
  msl_last_argbuf_vertex_translation_ = nullptr;
  msl_last_argbuf_vertex_encoded_length_ = 0;
  msl_last_argbuf_vertex_layout_uid_ = 0;
  msl_last_argbuf_pixel_textures_.fill(nullptr);
  msl_last_argbuf_pixel_texture_count_ = 0;
  msl_last_argbuf_pixel_samplers_.fill(nullptr);
  msl_last_argbuf_pixel_sampler_count_ = 0;
  msl_last_argbuf_pixel_buffer_ = nullptr;
  msl_last_argbuf_pixel_offset_ = 0;
  msl_last_argbuf_pixel_translation_ = nullptr;
  msl_last_argbuf_pixel_encoded_length_ = 0;
  msl_last_argbuf_pixel_layout_uid_ = 0;
}

void MetalCommandProcessor::EndRenderEncoder() {
  if (!current_render_encoder_) {
    return;
  }
  current_render_encoder_->endEncoding();
  if (render_encoder_has_zpd_visibility_) {
    // Copies the pass's occlusion query results before the next pass resets
    // them. A query still going resumes in a new slot in the next pass.
    zpd_visibility_pool_.EndRenderPass(current_command_buffer_);
    render_encoder_has_zpd_visibility_ = false;
  }
  zpd_segment_open_ = false;
  current_render_encoder_->release();
  current_render_encoder_ = nullptr;
  metal_gpu_pass_record_ = nullptr;
  current_render_area_width_ = 0;
  current_render_area_height_ = 0;
  current_render_pass_descriptor_ = nullptr;
  ResetMslRenderEncoderStateCache();
}

void MetalCommandProcessor::ResetRenderEncoderResourceUsage() {
  render_encoder_resource_usage_.clear();
  render_encoder_heap_usage_.clear();
}

void MetalCommandProcessor::UseRenderEncoderResource(MTL::Resource* resource,
                                                     MTL::ResourceUsage usage) {
  if (!current_render_encoder_ || !resource) {
    return;
  }
  UseRenderEncoderHeap(resource->heap());
  uint32_t usage_bits = static_cast<uint32_t>(usage);
  auto it = render_encoder_resource_usage_.find(resource);
  if (it != render_encoder_resource_usage_.end()) {
    if ((it->second & usage_bits) == usage_bits) {
      return;
    }
    it->second |= usage_bits;
  } else {
    render_encoder_resource_usage_.emplace(resource, usage_bits);
  }
  current_render_encoder_->useResource(resource, usage);
}

void MetalCommandProcessor::UseRenderEncoderHeap(MTL::Heap* heap) {
  if (!current_render_encoder_ || !heap) {
    return;
  }
  if (!render_encoder_heap_usage_.insert(heap).second) {
    return;
  }
  current_render_encoder_->useHeap(heap);
}

void MetalCommandProcessor::UseRenderEncoderAttachmentHeaps(
    MTL::RenderPassDescriptor* descriptor) {
  if (!current_render_encoder_ || !descriptor) {
    return;
  }
  auto* color_attachments = descriptor->colorAttachments();
  for (uint32_t i = 0; i < 8; ++i) {
    auto* attachment = color_attachments->object(i);
    if (!attachment) {
      continue;
    }
    MTL::Texture* texture = attachment->texture();
    if (texture) {
      UseRenderEncoderHeap(texture->heap());
    }
  }
  auto* depth_attachment = descriptor->depthAttachment();
  if (depth_attachment && depth_attachment->texture()) {
    UseRenderEncoderHeap(depth_attachment->texture()->heap());
  }
  auto* stencil_attachment = descriptor->stencilAttachment();
  if (stencil_attachment && stencil_attachment->texture()) {
    UseRenderEncoderHeap(stencil_attachment->texture()->heap());
  }
}

void MetalCommandProcessor::BeginCommandBuffer() {
  if (!EnsureCommandBuffer()) {
    return;
  }

  if (!current_render_encoder_ && (!render_encoder_resource_usage_.empty() ||
                                   !render_encoder_heap_usage_.empty())) {
    ResetRenderEncoderResourceUsage();
  }

#if METAL_SHADER_CONVERTER_AVAILABLE
  EnsureActiveDrawRing();
#endif

  // Obtain the render pass descriptor. Prefer the one provided by
  // MetalRenderTargetCache (host render-target path), falling back to the
  // legacy descriptor if needed.
  MTL::RenderPassDescriptor* pass_descriptor = render_pass_descriptor_;
  if (render_target_cache_) {
    if (MTL::RenderPassDescriptor* cache_desc =
            render_target_cache_->GetRenderPassDescriptor(1)) {
      pass_descriptor = cache_desc;
    }
  }
  if (!pass_descriptor) {
    XELOGE("BeginCommandBuffer: No render pass descriptor available");
    return;
  }

  // Detect Reverse-Z usage and update clear depth.
  if (register_file_) {
    auto depth_control = register_file_->Get<reg::RB_DEPTHCONTROL>();
    bool reverse_z =
        depth_control.z_enable &&
        (depth_control.zfunc == xenos::CompareFunction::kGreater ||
         depth_control.zfunc == xenos::CompareFunction::kGreaterEqual);
    if (auto* da = pass_descriptor->depthAttachment()) {
      if (reverse_z) {
        da->setClearDepth(0.0);
      } else {
        da->setClearDepth(1.0);
      }
    }
  }

  // If the render pass configuration has changed since the current render
  // encoder was created (e.g. dummy RT0 -> real RTs, depth/stencil binding),
  // restart the render encoder with the updated descriptor.
  if (current_render_encoder_ &&
      current_render_pass_descriptor_ != pass_descriptor) {
    EndRenderEncoder();
  }
  // occlusion_query_enable: a query counts only in a render encoder created
  // with the visibility buffer, so one created before the query began is
  // restarted.
  const bool zpd_pass = zpd_query_active_ && !zpd_query_.inexact &&
                        zpd_visibility_pool_.is_initialized();
  if (current_render_encoder_ && zpd_pass &&
      !render_encoder_has_zpd_visibility_) {
    EndRenderEncoder();
    metal_perf_counters.zpd_pass_restarts.fetch_add(1,
                                                    std::memory_order_relaxed);
  }
  // A pass with a limited render area that the current draw needs more of is
  // restarted with a larger area (its attachments are loaded, see below).
  if (current_render_encoder_ && current_render_area_width_ &&
      (render_area_needed_width_ > current_render_area_width_ ||
       render_area_needed_height_ > current_render_area_height_)) {
    EndRenderEncoder();
    metal_perf_counters.render_area_restarts.fetch_add(
        1, std::memory_order_relaxed);
  }

  if (!current_render_encoder_) {
    // If some path cleared the encoder without going through EndRenderEncoder,
    // avoid leaking cached binding state into the new encoder.
    ResetMslRenderEncoderStateCache();
    // Limit the render area to what the draws need, unless an attachment is
    // cleared on load (a restart to grow the area would clear it again).
    uint32_t area_width = 0;
    uint32_t area_height = 0;
    if (render_area_needed_width_ && render_area_needed_height_) {
      uint32_t attachments_width = UINT32_MAX;
      uint32_t attachments_height = UINT32_MAX;
      bool clears_on_load = false;
      auto limit_to_attachment =
          [&](MTL::RenderPassAttachmentDescriptor* attachment) {
            MTL::Texture* texture =
                attachment ? attachment->texture() : nullptr;
            if (!texture) {
              return;
            }
            attachments_width =
                std::min(attachments_width, uint32_t(texture->width()));
            attachments_height =
                std::min(attachments_height, uint32_t(texture->height()));
            clears_on_load |= attachment->loadAction() == MTL::LoadActionClear;
          };
      for (uint32_t i = 0; i < 4; ++i) {
        limit_to_attachment(pass_descriptor->colorAttachments()->object(i));
      }
      limit_to_attachment(pass_descriptor->depthAttachment());
      limit_to_attachment(pass_descriptor->stencilAttachment());
      if (!clears_on_load && attachments_width != UINT32_MAX) {
        // By the attachments, not the descriptor: the render target cache
        // makes a new descriptor when the targets change, which may get the
        // address of one just freed, and GTA IV's predicated tiling bands
        // then took over the area of the depth clear pass between them (736
        // rows loaded and stored for 256 drawn).
        std::array<MTL::Texture*, 6> area_attachments = {};
        for (uint32_t i = 0; i < 4; ++i) {
          area_attachments[i] =
              pass_descriptor->colorAttachments()->object(i)->texture();
        }
        if (auto* depth_attachment = pass_descriptor->depthAttachment()) {
          area_attachments[4] = depth_attachment->texture();
        }
        if (auto* stencil_attachment = pass_descriptor->stencilAttachment()) {
          area_attachments[5] = stencil_attachment->texture();
        }
        auto& known_area = render_area_by_attachments_[area_attachments];
        known_area[0] =
            std::max(known_area[0], xe::align(render_area_needed_width_, 32u));
        known_area[1] =
            std::max(known_area[1], xe::align(render_area_needed_height_, 32u));
        area_width =
            std::min(std::max(known_area[0], known_area[2]), attachments_width);
        area_height = std::min(std::max(known_area[1], known_area[3]),
                               attachments_height);
        if (area_width >= attachments_width &&
            area_height >= attachments_height) {
          area_width = 0;
          area_height = 0;
        }
      }
    }
    if (area_width) {
      pass_descriptor->setRenderTargetWidth(area_width);
      pass_descriptor->setRenderTargetHeight(area_height);
    }
    if (zpd_pass) {
      pass_descriptor->setVisibilityResultBuffer(
          zpd_visibility_pool_.visibility_buffer());
    }
    if (render_target_cache_) {
      render_target_cache_->ApplyXenosSamplePositions(pass_descriptor);
    }
    // Frame detail: 24-bit unorm depth (shadow maps) as the pass starts, after
    // its render target transfers: dumped to EDRAM and copied for the log.
    if (metal_perf_counters.frame_detail_frames.load(
            std::memory_order_relaxed) &&
        register_file_ && render_target_cache_ &&
        render_target_cache_->GetEdramBuffer() &&
        pass_descriptor->depthAttachment() &&
        pass_descriptor->depthAttachment()->texture() && texture_cache_ &&
        texture_cache_->draw_resolution_scale_x() == 1 &&
        texture_cache_->draw_resolution_scale_y() == 1) {
      const auto depth_info = register_file_->Get<reg::RB_DEPTH_INFO>();
      const auto surface_info = register_file_->Get<reg::RB_SURFACE_INFO>();
      if (depth_info.depth_format == xenos::DepthRenderTargetFormat::kD24S8) {
        MetalResolveProbe probe;
        probe.pass = frame_detail_passes_ + 1;
        probe.edram_base_tiles = depth_info.depth_base;
        // EDRAM samples, which match 1x pixels: 2x and 4x double the rows,
        // 4x the columns.
        probe.width = surface_info.surface_pitch << uint32_t(
                          surface_info.msaa_samples >= xenos::MsaaSamples::k4X);
        probe.edram_pitch_tiles =
            (probe.width + xenos::kEdramTileWidthSamples - 1) /
            xenos::kEdramTileWidthSamples;
        probe.height =
            (area_height
                 ? area_height
                 : uint32_t(
                       pass_descriptor->depthAttachment()->texture()->height()))
            << (uint32_t(surface_info.msaa_samples >= xenos::MsaaSamples::k2X) -
                render_target_cache_->last_update_samples_as_pixels_y_log2());
        const uint32_t tile_bytes = xenos::kEdramTileWidthSamples *
                                    xenos::kEdramTileHeightSamples *
                                    sizeof(uint32_t);
        const uint32_t tiles =
            std::min(probe.edram_pitch_tiles *
                         ((probe.height + xenos::kEdramTileHeightSamples - 1) /
                          xenos::kEdramTileHeightSamples),
                     xenos::kEdramTileCount);
        if (tiles) {
          render_target_cache_->DumpRenderTargets(
              probe.edram_base_tiles, probe.edram_pitch_tiles,
              tiles / probe.edram_pitch_tiles, probe.edram_pitch_tiles,
              current_command_buffer_);
          probe.edram_snapshot = device_->newBuffer(
              tiles * tile_bytes, MTL::ResourceStorageModeShared);
        }
        MTL::BlitCommandEncoder* snapshot_blit =
            probe.edram_snapshot ? current_command_buffer_->blitCommandEncoder()
                                 : nullptr;
        if (snapshot_blit) {
          MTL::Buffer* edram = render_target_cache_->GetEdramBuffer();
          // EDRAM addressing wraps around.
          const uint32_t first_tiles =
              std::min(tiles, xenos::kEdramTileCount - probe.edram_base_tiles);
          snapshot_blit->copyFromBuffer(
              edram, probe.edram_base_tiles * tile_bytes, probe.edram_snapshot,
              0, first_tiles * tile_bytes);
          if (tiles > first_tiles) {
            snapshot_blit->copyFromBuffer(edram, 0, probe.edram_snapshot,
                                          first_tiles * tile_bytes,
                                          (tiles - first_tiles) * tile_bytes);
          }
          snapshot_blit->endEncoding();
          probe.submission = GetCurrentSubmission();
          resolve_probes_.push_back(probe);
        } else if (probe.edram_snapshot) {
          probe.edram_snapshot->release();
        }
      }
    }
    // Note: renderCommandEncoder() returns an autoreleased object, we must
    // retain it.
    current_render_encoder_ = metal_gpu_pass_timer.RenderEncoder(
        current_command_buffer_, pass_descriptor, "draws",
        &metal_gpu_pass_record_);
    if (area_width) {
      // The descriptor is the render target cache's, reused for other passes.
      pass_descriptor->setRenderTargetWidth(0);
      pass_descriptor->setRenderTargetHeight(0);
    }
    if (zpd_pass) {
      pass_descriptor->setVisibilityResultBuffer(nullptr);
    }
    render_encoder_has_zpd_visibility_ =
        zpd_pass && current_render_encoder_ != nullptr;
    current_render_area_width_ = current_render_encoder_ ? area_width : 0;
    current_render_area_height_ = current_render_encoder_ ? area_height : 0;
    if (render_area_needed_width_ && current_render_encoder_) {
      metal_perf_counters.render_area_draw_passes.fetch_add(
          1, std::memory_order_relaxed);
      if (area_width) {
        metal_perf_counters.render_area_draw_limited.fetch_add(
            1, std::memory_order_relaxed);
        metal_perf_counters.render_area_draw_rows.fetch_add(
            area_height, std::memory_order_relaxed);
      }
    }
    if (!current_render_encoder_) {
      XELOGE("Failed to create render command encoder");
      return;
    }
    current_render_encoder_->retain();
    current_render_encoder_->setLabel(
        NS::String::string("XeniaRenderEncoder", NS::UTF8StringEncoding));
    ff_blend_factor_valid_ = false;
    current_render_pass_descriptor_ = pass_descriptor;
    UseRenderEncoderAttachmentHeaps(pass_descriptor);
    if (metal_perf_counters.frame_detail_frames.load(
            std::memory_order_relaxed) &&
        register_file_) {
      // Host attachments: size, MTL::PixelFormat, samples, load and store
      // actions; guest render target registers as they are.
      std::string frame_detail_attachments;
      auto add_attachment =
          [&](const char* name,
              MTL::RenderPassAttachmentDescriptor* attachment) {
            MTL::Texture* texture =
                attachment ? attachment->texture() : nullptr;
            if (texture) {
              fmt::format_to(std::back_inserter(frame_detail_attachments),
                             " {} {}x{} format {} x{} load {} store {}", name,
                             texture->width(), texture->height(),
                             uint32_t(texture->pixelFormat()),
                             texture->sampleCount(),
                             uint32_t(attachment->loadAction()),
                             uint32_t(attachment->storeAction()));
            }
          };
      static const char* const kColorNames[] = {"c0", "c1", "c2", "c3"};
      for (uint32_t i = 0; i < 4; ++i) {
        add_attachment(kColorNames[i],
                       pass_descriptor->colorAttachments()->object(i));
      }
      add_attachment("depth", pass_descriptor->depthAttachment());
      const uint32_t* values = register_file_->values;
      XELOGI(
          "Frame detail pass {}: area {}x{} (0 for all), SURFACE_INFO {:08X} "
          "COLOR_INFO {:08X} {:08X} {:08X} {:08X} DEPTH_INFO {:08X} "
          "MODECONTROL {:08X}, query buffer {};{}",
          ++frame_detail_passes_, area_width, area_height,
          values[XE_GPU_REG_RB_SURFACE_INFO], values[XE_GPU_REG_RB_COLOR_INFO],
          values[XE_GPU_REG_RB_COLOR1_INFO], values[XE_GPU_REG_RB_COLOR2_INFO],
          values[XE_GPU_REG_RB_COLOR3_INFO], values[XE_GPU_REG_RB_DEPTH_INFO],
          values[XE_GPU_REG_RB_MODECONTROL], render_encoder_has_zpd_visibility_,
          frame_detail_attachments);
    }
  }

  // Derive viewport/scissor from the actual bound render target rather than
  // a hard-coded 1280x720. Prefer color RT 0 from the MetalRenderTargetCache,
  // falling back to depth (depth-only passes) and then legacy
  // render_target_width_/height_ when needed.
  uint32_t rt_width = 1;
  uint32_t rt_height = 1;
  GetBoundRenderTargetSize(render_target_cache_.get(), render_target_width_,
                           render_target_height_, rt_width, rt_height);
  if (current_render_area_width_) {
    rt_width = std::min(rt_width, current_render_area_width_);
    rt_height = std::min(rt_height, current_render_area_height_);
  }

  // Set viewport
  MTL::Viewport viewport = {
      0.0, 0.0, static_cast<double>(rt_width), static_cast<double>(rt_height),
      0.0, 1.0};
  current_render_encoder_->setViewport(viewport);

  // Set scissor (must not exceed render pass dimensions)
  MTL::ScissorRect scissor = {0, 0, rt_width, rt_height};
  current_render_encoder_->setScissorRect(scissor);
}

#if METAL_SHADER_CONVERTER_AVAILABLE
bool MetalCommandProcessor::EnsureDrawRingCapacity() {
  if (current_draw_index_ < draw_ring_count_ && active_draw_ring_) {
    return true;
  }

  auto ring = AcquireDrawRingBuffers();
  if (!ring) {
    // If the pool is bounded and exhausted, split work into a new command
    // buffer so completed submissions can recycle rings.
    EndCommandBuffer();
    BeginCommandBuffer();
    if (!current_command_buffer_ || !current_render_encoder_) {
      XELOGE(
          "Metal draw ring exhausted and failed to begin a new command buffer");
      return false;
    }
    if (current_draw_index_ < draw_ring_count_ && active_draw_ring_) {
      return true;
    }
    ring = AcquireDrawRingBuffers();
    if (!ring) {
      XELOGE("Metal draw ring exhausted after command-buffer split");
      return false;
    }
  }

  SetActiveDrawRing(ring);
  command_buffer_draw_rings_.push_back(ring);
  current_draw_index_ = 0;
  return true;
}
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

#if !METAL_SHADER_CONVERTER_AVAILABLE
bool MetalCommandProcessor::EnsureSpirvUniformBuffer() {
  if (uniforms_buffer_) {
    return true;
  }
  if (!device_) {
    XELOGE("EnsureSpirvUniformBuffer: Metal device is null");
    return false;
  }

  // Keep this aligned with the SPIRV-Cross descriptor table layout used by
  // IssueDrawMsl (6 x 4KB CBVs + texture/sampler descriptor blocks).
  constexpr size_t kUniformsBytesPerTable = 24576;
  constexpr size_t kStageCount = 2;

  if (!draw_ring_count_) {
    XELOGW("SPIRV-Cross: draw ring count was zero, forcing to 1");
    draw_ring_count_ = 1;
  }

  // Keep a slightly larger initial pool on iOS to reduce early-frame pressure.
#if XE_PLATFORM_IOS
  constexpr size_t kUniformsBuffersInFlightInitial = 6;
#else
  constexpr size_t kUniformsBuffersInFlightInitial = 4;
#endif
  const size_t kUniformsBuffersInFlightMax = MetalUniformBuffersMax();

  if (!spirv_uniforms_pool_initialized_) {
    size_t requested_ring_count = std::max<size_t>(1, draw_ring_count_);
    while (requested_ring_count >= 1) {
      const size_t descriptor_table_count = kStageCount * requested_ring_count;
      const size_t uniforms_buffer_size =
          kUniformsBytesPerTable * descriptor_table_count;

      std::vector<MTL::Buffer*> new_pool;
      new_pool.reserve(kUniformsBuffersInFlightInitial);
      bool allocation_failed = false;
      for (size_t i = 0; i < kUniformsBuffersInFlightInitial; ++i) {
        MTL::Buffer* buffer = device_->newBuffer(
            uniforms_buffer_size, MTL::ResourceStorageModeShared);
        if (!buffer) {
          allocation_failed = true;
          break;
        }
        buffer->setLabel(
            NS::String::string("MslUniformsBuffer", NS::UTF8StringEncoding));
        std::memset(buffer->contents(), 0, uniforms_buffer_size);
        new_pool.push_back(buffer);
      }

      if (!allocation_failed &&
          new_pool.size() == kUniformsBuffersInFlightInitial) {
        {
          std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
          for (MTL::Buffer* old_buffer : spirv_uniforms_pool_) {
            if (old_buffer) {
              old_buffer->release();
            }
          }
          spirv_uniforms_pool_ = std::move(new_pool);
          spirv_uniforms_available_.clear();
          spirv_uniforms_available_.insert(spirv_uniforms_available_.end(),
                                           spirv_uniforms_pool_.begin(),
                                           spirv_uniforms_pool_.end());
        }

        if (spirv_uniforms_available_semaphore_) {
#if !OS_OBJECT_USE_OBJC
          dispatch_release(spirv_uniforms_available_semaphore_);
#endif
          spirv_uniforms_available_semaphore_ = nullptr;
        }
        spirv_uniforms_available_semaphore_ = dispatch_semaphore_create(
            static_cast<long>(kUniformsBuffersInFlightInitial));
        if (!spirv_uniforms_available_semaphore_) {
          XELOGE(
              "SPIRV-Cross: failed to create uniforms availability semaphore");
          return false;
        }

        if (requested_ring_count != draw_ring_count_) {
          XELOGW(
              "SPIRV-Cross: reduced uniforms ring from {} to {} pages after "
              "allocation pressure",
              draw_ring_count_, requested_ring_count);
          draw_ring_count_ = requested_ring_count;
        }
        spirv_uniforms_pool_initialized_ = true;
        break;
      }

      for (MTL::Buffer* buffer : new_pool) {
        if (buffer) {
          buffer->release();
        }
      }
      if (requested_ring_count == 1) {
        break;
      }
      const size_t fallback_ring_count =
          std::max<size_t>(1, requested_ring_count / 2);
      XELOGW(
          "SPIRV-Cross: failed to allocate uniforms pool with {} ring pages, "
          "retrying with {}",
          requested_ring_count, fallback_ring_count);
      requested_ring_count = fallback_ring_count;
    }

    if (!spirv_uniforms_pool_initialized_) {
      XELOGE(
          "Failed to create uniforms buffer pool for SPIRV-Cross path (ring "
          "pages={}, bytes per table={})",
          draw_ring_count_, kUniformsBytesPerTable);
      return false;
    }
  }

  if (!spirv_uniforms_available_semaphore_) {
    XELOGE("SPIRV-Cross: uniforms pool semaphore is not initialized");
    return false;
  }

  const auto try_grow_uniforms_pool = [&]() -> bool {
    size_t pool_size = 0;
    {
      std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
      pool_size = spirv_uniforms_pool_.size();
      if (pool_size >= kUniformsBuffersInFlightMax) {
        return false;
      }
    }
    const size_t descriptor_table_count =
        kStageCount * std::max<size_t>(size_t(1), draw_ring_count_);
    const size_t uniforms_buffer_size =
        kUniformsBytesPerTable * descriptor_table_count;
    MTL::Buffer* buffer = device_->newBuffer(uniforms_buffer_size,
                                             MTL::ResourceStorageModeShared);
    if (!buffer) {
      return false;
    }
    buffer->setLabel(
        NS::String::string("MslUniformsBufferGrow", NS::UTF8StringEncoding));
    std::memset(buffer->contents(), 0, uniforms_buffer_size);
    size_t new_pool_size = 0;
    {
      std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
      spirv_uniforms_pool_.push_back(buffer);
      spirv_uniforms_available_.push_back(buffer);
      new_pool_size = spirv_uniforms_pool_.size();
    }
    dispatch_semaphore_signal(spirv_uniforms_available_semaphore_);
    XELOGW("SPIRV-Cross: grew uniforms pool to {} buffers under load",
           new_pool_size);
    return true;
  };

  if (dispatch_semaphore_wait(spirv_uniforms_available_semaphore_,
                              DISPATCH_TIME_NOW) != 0) {
    bool acquired_after_grow = false;
    if (try_grow_uniforms_pool()) {
      acquired_after_grow =
          dispatch_semaphore_wait(spirv_uniforms_available_semaphore_,
                                  DISPATCH_TIME_NOW) == 0;
    }
    if (!acquired_after_grow) {
      // Last resort: block until one in-flight command buffer retires.
      // D3D12-style behavior is to avoid this in common paths by growing first.
      static auto last_wait_log = std::chrono::steady_clock::time_point{};
      static uint32_t suppressed_wait_logs = 0;
      const auto now = std::chrono::steady_clock::now();
      if (now - last_wait_log >= std::chrono::seconds(10)) {
        last_wait_log = now;
        size_t pool_size = 0;
        size_t available_size = 0;
        {
          std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
          pool_size = spirv_uniforms_pool_.size();
          available_size = spirv_uniforms_available_.size();
        }
        XELOGW(
            "SPIRV-Cross: uniforms pool busy; waiting for an in-flight command "
            "buffer to retire (in-use={}, total={}, available={}, ring "
            "pages={}, suppressed_wait_logs={})",
            pool_size - available_size, pool_size, available_size,
            draw_ring_count_, suppressed_wait_logs);
        suppressed_wait_logs = 0;
      } else {
        ++suppressed_wait_logs;
      }
      const auto uniform_wait_begin = std::chrono::steady_clock::now();
      dispatch_semaphore_wait(spirv_uniforms_available_semaphore_,
                              DISPATCH_TIME_FOREVER);
      metal_perf_counters.uniform_waits.fetch_add(1, std::memory_order_relaxed);
      metal_perf_counters.uniform_wait_us.fetch_add(
          MetalPerfMicrosecondsSince(uniform_wait_begin),
          std::memory_order_relaxed);
    }
  }

  {
    std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
    if (spirv_uniforms_available_.empty()) {
      XELOGE(
          "SPIRV-Cross: uniforms semaphore signaled but no reusable buffer is "
          "available");
      return false;
    }
    uniforms_buffer_ = spirv_uniforms_available_.back();
    spirv_uniforms_available_.pop_back();
  }
  if (uniforms_buffer_) {
    command_buffer_spirv_uniform_buffers_.push_back(uniforms_buffer_);
  }

  return uniforms_buffer_ != nullptr;
}

bool MetalCommandProcessor::EnsureSpirvUniformBufferCapacity() {
  if (!draw_ring_count_) {
    draw_ring_count_ = 1;
  }
  if (!uniforms_buffer_) {
    return EnsureSpirvUniformBuffer();
  }
  if (current_draw_index_ == 0) {
    return true;
  }
  const uint32_t ring_index =
      current_draw_index_ % uint32_t(std::max<size_t>(1, draw_ring_count_));
  if (ring_index != 0) {
    return true;
  }

  // Try to rotate to another uniforms buffer in the current command buffer to
  // avoid forcing a split at every ring wrap.
  uniforms_buffer_ = nullptr;
  const auto try_acquire_uniforms_buffer = [&]() -> bool {
    if (!spirv_uniforms_available_semaphore_ ||
        dispatch_semaphore_wait(spirv_uniforms_available_semaphore_,
                                DISPATCH_TIME_NOW) != 0) {
      return false;
    }
    std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
    if (!spirv_uniforms_available_.empty()) {
      uniforms_buffer_ = spirv_uniforms_available_.back();
      spirv_uniforms_available_.pop_back();
      command_buffer_spirv_uniform_buffers_.push_back(uniforms_buffer_);
      return true;
    }
    // Keep semaphore state consistent if availability changed concurrently.
    dispatch_semaphore_signal(spirv_uniforms_available_semaphore_);
    return false;
  };

  const size_t kUniformsBuffersInFlightMax = MetalUniformBuffersMax();
  const auto try_grow_uniforms_pool = [&]() -> bool {
    if (!device_) {
      return false;
    }
    size_t pool_size = 0;
    {
      std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
      pool_size = spirv_uniforms_pool_.size();
      if (pool_size >= kUniformsBuffersInFlightMax) {
        return false;
      }
    }
    const size_t descriptor_table_count =
        kStageCount * std::max<size_t>(size_t(1), draw_ring_count_);
    const size_t uniforms_buffer_size =
        kUniformsBytesPerTable * descriptor_table_count;
    MTL::Buffer* buffer = device_->newBuffer(uniforms_buffer_size,
                                             MTL::ResourceStorageModeShared);
    if (!buffer) {
      return false;
    }
    buffer->setLabel(
        NS::String::string("MslUniformsBufferGrow", NS::UTF8StringEncoding));
    std::memset(buffer->contents(), 0, uniforms_buffer_size);
    size_t new_pool_size = 0;
    {
      std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
      spirv_uniforms_pool_.push_back(buffer);
      spirv_uniforms_available_.push_back(buffer);
      new_pool_size = spirv_uniforms_pool_.size();
    }
    dispatch_semaphore_signal(spirv_uniforms_available_semaphore_);
    XELOGW(
        "SPIRV-Cross: grew uniforms pool to {} buffers at ring-wrap pressure",
        new_pool_size);
    return true;
  };

  if (try_acquire_uniforms_buffer()) {
    return true;
  }
  if (try_grow_uniforms_pool() && try_acquire_uniforms_buffer()) {
    return true;
  }

  metal_perf_counters.uniform_rollovers.fetch_add(1, std::memory_order_relaxed);
  static bool rollover_logged = false;
  if (!rollover_logged) {
    rollover_logged = true;
    XELOGW(
        "SPIRV-Cross: uniforms ring exhausted; rotating Metal command buffer");
  }

  EndCommandBuffer();
  BeginCommandBuffer();
  if (!current_command_buffer_ || !current_render_encoder_ ||
      !uniforms_buffer_) {
    XELOGE(
        "SPIRV-Cross: failed to restart command buffer after uniforms ring "
        "rollover");
    return false;
  }
  return true;
}

void MetalCommandProcessor::ScheduleSpirvUniformBufferRelease(
    MTL::CommandBuffer* command_buffer) {
  if (!command_buffer) {
    return;
  }

  std::vector<MTL::Buffer*> submitted_uniforms;
  if (!command_buffer_spirv_uniform_buffers_.empty()) {
    submitted_uniforms.swap(command_buffer_spirv_uniform_buffers_);
  } else if (uniforms_buffer_) {
    submitted_uniforms.reserve(1);
    submitted_uniforms.push_back(uniforms_buffer_);
  }
  uniforms_buffer_ = nullptr;

  if (submitted_uniforms.empty()) {
    return;
  }

  pending_completion_handlers_.fetch_add(1, std::memory_order_relaxed);
  command_buffer->addCompletedHandler(
      [this, submitted_uniforms =
                 std::move(submitted_uniforms)](MTL::CommandBuffer*) mutable {
        size_t returned_count = 0;
        {
          std::lock_guard<std::mutex> lock(spirv_uniforms_mutex_);
          for (MTL::Buffer* uniforms : submitted_uniforms) {
            if (!uniforms) {
              continue;
            }
            spirv_uniforms_available_.push_back(uniforms);
            ++returned_count;
          }
        }
        if (spirv_uniforms_available_semaphore_) {
          for (size_t i = 0; i < returned_count; ++i) {
            dispatch_semaphore_signal(spirv_uniforms_available_semaphore_);
          }
        }
        pending_completion_handlers_.fetch_sub(1, std::memory_order_relaxed);
      });
}
#endif  // !METAL_SHADER_CONVERTER_AVAILABLE

bool MetalCommandProcessor::AcquireSpirvArgumentBufferSlice(
    uint32_t bytes, uint32_t alignment, MTL::Buffer** buffer_out,
    NS::UInteger* offset_out) {
  if (!buffer_out || !offset_out) {
    return false;
  }
  *buffer_out = nullptr;
  *offset_out = 0;
  if (!device_ || !current_command_buffer_ || bytes == 0) {
    return false;
  }

  const size_t align = std::max<size_t>(1, size_t(alignment));
  auto align_up = [](size_t value, size_t alignment) -> size_t {
    return ((value + alignment - 1) / alignment) * alignment;
  };

  if (!command_buffer_spirv_argbuf_pages_.empty()) {
    auto& page = command_buffer_spirv_argbuf_pages_.back();
    const size_t aligned_offset = align_up(page->offset, align);
    if (aligned_offset + bytes <= page->bytes) {
      page->offset = aligned_offset + bytes;
      *buffer_out = page->buffer;
      *offset_out = NS::UInteger(aligned_offset);
      return *buffer_out != nullptr;
    }
  }

  constexpr size_t kDefaultSpirvArgumentBufferPageBytes = 1024 * 1024;
  const size_t required_page_bytes = align_up(bytes, align);
  const size_t page_bytes =
      std::max(kDefaultSpirvArgumentBufferPageBytes, required_page_bytes);

  std::shared_ptr<SpirvArgumentBufferPage> page;
  {
    std::lock_guard<std::mutex> lock(spirv_argbuf_mutex_);
    for (auto it = spirv_argbuf_pool_.begin(); it != spirv_argbuf_pool_.end();
         ++it) {
      if ((*it) && (*it)->bytes >= page_bytes) {
        page = *it;
        spirv_argbuf_pool_.erase(it);
        break;
      }
    }
  }
  if (!page) {
    page = std::make_shared<SpirvArgumentBufferPage>();
    page->bytes = page_bytes;
    page->buffer =
        device_->newBuffer(page_bytes, MTL::ResourceStorageModeShared);
    if (!page->buffer) {
      return false;
    }
  }
  page->offset = 0;
  command_buffer_spirv_argbuf_pages_.push_back(page);

  const size_t aligned_offset = align_up(page->offset, align);
  if (aligned_offset + bytes > page->bytes) {
    return false;
  }
  page->offset = aligned_offset + bytes;
  *buffer_out = page->buffer;
  *offset_out = NS::UInteger(aligned_offset);
  return *buffer_out != nullptr;
}

void MetalCommandProcessor::ScheduleSpirvArgumentBufferRelease(
    MTL::CommandBuffer* command_buffer) {
  if (!command_buffer || command_buffer_spirv_argbuf_pages_.empty()) {
    return;
  }

  std::vector<std::shared_ptr<SpirvArgumentBufferPage>> pages;
  pages.swap(command_buffer_spirv_argbuf_pages_);

  bool add_handler = false;
  {
    std::lock_guard<std::mutex> lock(spirv_argbuf_mutex_);
    auto& pending = pending_spirv_argbuf_releases_[command_buffer];
    add_handler = pending.empty();
    pending.reserve(pending.size() + pages.size());
    for (auto& page : pages) {
      pending.push_back(std::move(page));
    }
  }

  if (add_handler) {
    pending_completion_handlers_.fetch_add(1, std::memory_order_relaxed);
    command_buffer->addCompletedHandler(
        [this](MTL::CommandBuffer* completed_cmd) {
          {
            std::lock_guard<std::mutex> lock(spirv_argbuf_mutex_);
            auto it = pending_spirv_argbuf_releases_.find(completed_cmd);
            if (it != pending_spirv_argbuf_releases_.end()) {
              for (auto& page : it->second) {
                if (page) {
                  page->offset = 0;
                  spirv_argbuf_pool_.push_back(std::move(page));
                }
              }
              pending_spirv_argbuf_releases_.erase(it);
            }
          }
          pending_completion_handlers_.fetch_sub(1, std::memory_order_relaxed);
        });
  }
}

void MetalCommandProcessor::EndCommandBuffer() {
  EndRenderEncoder();
  ResetMslCrossEncoderReuseCaches();

  if (current_command_buffer_) {
#if METAL_SHADER_CONVERTER_AVAILABLE
    ScheduleDrawRingRelease(current_command_buffer_);
#else
    if (UseSpirvCrossPath()) {
      ScheduleSpirvUniformBufferRelease(current_command_buffer_);
    }
#endif
    ScheduleSpirvArgumentBufferRelease(current_command_buffer_);
    AttachShaderDoneFences(current_command_buffer_);
    current_command_buffer_->commit();
    current_command_buffer_->release();
    current_command_buffer_ = nullptr;
#if METAL_SHADER_CONVERTER_AVAILABLE
    SetActiveDrawRing(nullptr);
#endif
    current_draw_index_ = 0;
  }
  copy_resolve_writes_pending_ = false;
  DrainCommandBufferAutoreleasePool();
}

void MetalCommandProcessor::ApplyDepthStencilState(
    bool primitive_polygonal, reg::RB_DEPTHCONTROL normalized_depth_control) {
  if (!current_render_encoder_ || !device_) {
    return;
  }

  const RegisterFile& regs = *register_file_;
  auto stencil_ref_mask_front = regs.Get<reg::RB_STENCILREFMASK>();
  auto stencil_ref_mask_back =
      regs.Get<reg::RB_STENCILREFMASK>(XE_GPU_REG_RB_STENCILREFMASK_BF);
  auto depth_control = normalized_depth_control;

  bool has_stencil_attachment = false;
  if (current_render_pass_descriptor_) {
    if (auto* stencil_attachment =
            current_render_pass_descriptor_->stencilAttachment()) {
      has_stencil_attachment = stencil_attachment->texture() != nullptr;
    }
  }

  if (!has_stencil_attachment && depth_control.stencil_enable) {
    static bool no_stencil_logged = false;
    if (!no_stencil_logged) {
      no_stencil_logged = true;
      XELOGW(
          "Metal: stencil enabled but no stencil attachment bound; disabling "
          "stencil for this pass");
    }
    depth_control.stencil_enable = 0;
    depth_control.backface_enable = 0;
    depth_control.stencilfunc = xenos::CompareFunction::kAlways;
    depth_control.stencilfail = xenos::StencilOp::kKeep;
    depth_control.stencilzpass = xenos::StencilOp::kKeep;
    depth_control.stencilzfail = xenos::StencilOp::kKeep;
    depth_control.stencilfunc_bf = xenos::CompareFunction::kAlways;
    depth_control.stencilfail_bf = xenos::StencilOp::kKeep;
    depth_control.stencilzpass_bf = xenos::StencilOp::kKeep;
    depth_control.stencilzfail_bf = xenos::StencilOp::kKeep;
    stencil_ref_mask_front.value = 0;
    stencil_ref_mask_back.value = 0;
  }

  DepthStencilStateKey key;
  key.depth_control = depth_control.value;
  key.stencil_ref_mask_front = stencil_ref_mask_front.value;
  key.stencil_ref_mask_back = stencil_ref_mask_back.value;
  key.polygonal_and_backface = (primitive_polygonal ? 1u : 0u) |
                               (depth_control.backface_enable ? 2u : 0u);

  MTL::DepthStencilState* state = nullptr;
  auto it = depth_stencil_state_cache_.find(key);
  if (it != depth_stencil_state_cache_.end()) {
    state = it->second;
  } else {
    MTL::DepthStencilDescriptor* ds_desc =
        MTL::DepthStencilDescriptor::alloc()->init();
    if (depth_control.z_enable) {
      ds_desc->setDepthCompareFunction(
          ToMetalCompareFunction(depth_control.zfunc));
      ds_desc->setDepthWriteEnabled(depth_control.z_write_enable != 0);
    } else {
      ds_desc->setDepthCompareFunction(MTL::CompareFunctionAlways);
      ds_desc->setDepthWriteEnabled(false);
    }

    if (depth_control.stencil_enable) {
      auto* front = MTL::StencilDescriptor::alloc()->init();
      front->setStencilCompareFunction(
          ToMetalCompareFunction(depth_control.stencilfunc));
      front->setStencilFailureOperation(
          ToMetalStencilOperation(depth_control.stencilfail));
      front->setDepthFailureOperation(
          ToMetalStencilOperation(depth_control.stencilzfail));
      front->setDepthStencilPassOperation(
          ToMetalStencilOperation(depth_control.stencilzpass));
      front->setReadMask(stencil_ref_mask_front.stencilmask);
      front->setWriteMask(stencil_ref_mask_front.stencilwritemask);

      ds_desc->setFrontFaceStencil(front);

      if (primitive_polygonal && depth_control.backface_enable) {
        auto* back = MTL::StencilDescriptor::alloc()->init();
        back->setStencilCompareFunction(
            ToMetalCompareFunction(depth_control.stencilfunc_bf));
        back->setStencilFailureOperation(
            ToMetalStencilOperation(depth_control.stencilfail_bf));
        back->setDepthFailureOperation(
            ToMetalStencilOperation(depth_control.stencilzfail_bf));
        back->setDepthStencilPassOperation(
            ToMetalStencilOperation(depth_control.stencilzpass_bf));
        back->setReadMask(stencil_ref_mask_back.stencilmask);
        back->setWriteMask(stencil_ref_mask_back.stencilwritemask);
        ds_desc->setBackFaceStencil(back);
        back->release();
      } else {
        ds_desc->setBackFaceStencil(front);
      }

      front->release();
    }

    state = device_->newDepthStencilState(ds_desc);
    ds_desc->release();

    if (!state) {
      XELOGE("Failed to create Metal depth/stencil state");
      return;
    }
    depth_stencil_state_cache_.emplace(key, state);
  }

  current_render_encoder_->setDepthStencilState(state);

  if (depth_control.stencil_enable) {
    uint32_t ref_front = stencil_ref_mask_front.stencilref;
    uint32_t ref_back = stencil_ref_mask_back.stencilref;
    auto pa_su_sc_mode_cntl = regs.Get<reg::PA_SU_SC_MODE_CNTL>();
    uint32_t ref = ref_front;
    if (primitive_polygonal && depth_control.backface_enable &&
        pa_su_sc_mode_cntl.cull_front && !pa_su_sc_mode_cntl.cull_back) {
      ref = ref_back;
    } else if (primitive_polygonal && depth_control.backface_enable &&
               ref_front != ref_back) {
      static bool mismatch_logged = false;
      if (!mismatch_logged) {
        mismatch_logged = true;
        XELOGW(
            "Metal: front/back stencil ref differ (front={}, back={}); using "
            "front for both",
            ref_front, ref_back);
      }
    }
    current_render_encoder_->setStencilReferenceValue(ref);
  }
}

void MetalCommandProcessor::ApplyRasterizerState(bool primitive_polygonal) {
  if (!current_render_encoder_ || !render_target_cache_) {
    return;
  }

  const RegisterFile& regs = *register_file_;
  auto pa_su_sc_mode_cntl = regs.Get<reg::PA_SU_SC_MODE_CNTL>();
  auto pa_cl_clip_cntl = regs.Get<reg::PA_CL_CLIP_CNTL>();

  MTL::CullMode cull_mode = MTL::CullModeNone;
  if (primitive_polygonal) {
    bool cull_front = pa_su_sc_mode_cntl.cull_front;
    bool cull_back = pa_su_sc_mode_cntl.cull_back;
    if (cull_front && !cull_back) {
      cull_mode = MTL::CullModeFront;
    } else if (cull_back && !cull_front) {
      cull_mode = MTL::CullModeBack;
    }
  }
  current_render_encoder_->setCullMode(cull_mode);

  current_render_encoder_->setFrontFacingWinding(
      pa_su_sc_mode_cntl.face ? MTL::WindingClockwise
                              : MTL::WindingCounterClockwise);

  MTL::TriangleFillMode fill_mode = MTL::TriangleFillModeFill;
  if (primitive_polygonal &&
      pa_su_sc_mode_cntl.poly_mode == xenos::PolygonModeEnable::kDualMode) {
    xenos::PolygonType polygon_type = xenos::PolygonType::kTriangles;
    if (!pa_su_sc_mode_cntl.cull_front) {
      polygon_type =
          std::min(polygon_type, pa_su_sc_mode_cntl.polymode_front_ptype);
    }
    if (!pa_su_sc_mode_cntl.cull_back) {
      polygon_type =
          std::min(polygon_type, pa_su_sc_mode_cntl.polymode_back_ptype);
    }
    if (polygon_type != xenos::PolygonType::kTriangles) {
      fill_mode = MTL::TriangleFillModeLines;
    }
  }
  current_render_encoder_->setTriangleFillMode(fill_mode);

  float polygon_offset_scale = 0.0f;
  float polygon_offset = 0.0f;
  draw_util::GetPreferredFacePolygonOffset(
      regs, primitive_polygonal, polygon_offset_scale, polygon_offset);
  float depth_bias_factor = regs.Get<reg::RB_DEPTH_INFO>().depth_format ==
                                    xenos::DepthRenderTargetFormat::kD24S8
                                ? draw_util::kD3D10PolygonOffsetFactorUnorm24
                                : draw_util::kD3D10PolygonOffsetFactorFloat24;
  float depth_bias_constant = polygon_offset * depth_bias_factor;
  // msaa_samples_as_pixels: drawing to the samples of an MSAA surface, scaled
  // by the sample grid.
  float depth_bias_slope =
      polygon_offset_scale * xenos::kPolygonOffsetScaleSubpixelUnit *
      float(std::max(
          render_target_cache_->draw_resolution_scale_x()
              << render_target_cache_->last_update_samples_as_pixels_x_log2(),
          render_target_cache_->draw_resolution_scale_y()
              << render_target_cache_->last_update_samples_as_pixels_y_log2()));
  current_render_encoder_->setDepthBias(depth_bias_constant, depth_bias_slope,
                                        0.0f);

  current_render_encoder_->setDepthClipMode(pa_cl_clip_cntl.clip_disable
                                                ? MTL::DepthClipModeClamp
                                                : MTL::DepthClipModeClip);
}

MTL::RenderPassDescriptor*
MetalCommandProcessor::GetCurrentRenderPassDescriptor() {
  return render_pass_descriptor_;
}

#if METAL_SHADER_CONVERTER_AVAILABLE
MTL::RenderPipelineState* MetalCommandProcessor::GetOrCreatePipelineState(
    MetalShader::MetalTranslation* vertex_translation,
    MetalShader::MetalTranslation* pixel_translation,
    const RegisterFile& regs) {
  if (!vertex_translation || !vertex_translation->metal_function()) {
    XELOGE("No valid vertex shader function");
    return nullptr;
  }

  // Determine attachment formats and sample count from the render target cache
  // so the pipeline matches the actual render pass. If no real RT is bound,
  // fall back to the dummy RT0 format used by the cache.
  uint32_t sample_count = 1;
  MTL::PixelFormat color_formats[4] = {
      MTL::PixelFormatInvalid, MTL::PixelFormatInvalid, MTL::PixelFormatInvalid,
      MTL::PixelFormatInvalid};
  MTL::PixelFormat depth_format = MTL::PixelFormatInvalid;
  MTL::PixelFormat stencil_format = MTL::PixelFormatInvalid;
  if (render_target_cache_) {
    for (uint32_t i = 0; i < 4; ++i) {
      if (MTL::Texture* rt = render_target_cache_->GetColorTargetForDraw(i)) {
        color_formats[i] = rt->pixelFormat();
        if (rt->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(rt->sampleCount()));
        }
      }
    }
    if (color_formats[0] == MTL::PixelFormatInvalid) {
      if (MTL::Texture* dummy =
              render_target_cache_->GetDummyColorTargetForDraw()) {
        color_formats[0] = dummy->pixelFormat();
        if (dummy->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(dummy->sampleCount()));
        }
      }
    }
    if (MTL::Texture* depth_tex =
            render_target_cache_->GetDepthTargetForDraw()) {
      depth_format = depth_tex->pixelFormat();
      switch (depth_format) {
        case MTL::PixelFormatDepth32Float_Stencil8:
        case MTL::PixelFormatDepth24Unorm_Stencil8:
        case MTL::PixelFormatX32_Stencil8:
          stencil_format = depth_format;
          break;
        default:
          stencil_format = MTL::PixelFormatInvalid;
          break;
      }
      if (depth_tex->sampleCount() > 0) {
        sample_count = std::max<uint32_t>(
            sample_count, static_cast<uint32_t>(depth_tex->sampleCount()));
      }
    }
  }

  MTL::RenderPassDescriptor* pass_descriptor = current_render_pass_descriptor_;
  if (!pass_descriptor) {
    pass_descriptor = render_pass_descriptor_;
  }
  if (pass_descriptor) {
    // Rebuild strictly from the active encoder descriptor to prevent stale
    // formats from cache-derived fallback state.
    sample_count = 1;
    for (uint32_t i = 0; i < 4; ++i) {
      color_formats[i] = MTL::PixelFormatInvalid;
    }
    depth_format = MTL::PixelFormatInvalid;
    stencil_format = MTL::PixelFormatInvalid;
    PopulatePipelineFormatsFromRenderPassDescriptor(
        pass_descriptor, color_formats, 4, &depth_format, &stencil_format,
        &sample_count);
  }
  bool pixel_shader_writes_depth =
      pixel_translation && pixel_translation->shader().writes_depth();
  EnsureDepthFormatForDepthWritingFragment(
      "Pipeline", pixel_shader_writes_depth, &depth_format);

  struct PipelineKey {
    const void* vs;
    const void* ps;
    uint32_t sample_count;
    uint32_t depth_format;
    uint32_t stencil_format;
    uint32_t color_formats[4];
    uint32_t normalized_color_mask;
    uint32_t alpha_to_mask_enable;
    uint32_t blendcontrol[4];
  } key_data = {};
  key_data.vs = vertex_translation;
  key_data.ps = pixel_translation;
  key_data.sample_count = sample_count;
  key_data.depth_format = uint32_t(depth_format);
  key_data.stencil_format = uint32_t(stencil_format);
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.color_formats[i] = uint32_t(color_formats[i]);
  }
  uint32_t pixel_shader_writes_color_targets =
      pixel_translation ? pixel_translation->shader().writes_color_targets()
                        : 0;
  key_data.normalized_color_mask = 0;
  if (pixel_shader_writes_color_targets) {
    key_data.normalized_color_mask = draw_util::GetNormalizedColorMask(
        regs, pixel_shader_writes_color_targets);
  }
  auto rb_colorcontrol = regs.Get<reg::RB_COLORCONTROL>();
  key_data.alpha_to_mask_enable = rb_colorcontrol.alpha_to_mask_enable ? 1 : 0;
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.blendcontrol[i] =
        regs.Get<reg::RB_BLENDCONTROL>(
                reg::RB_BLENDCONTROL::rt_register_indices[i])
            .value;
  }
  uint64_t key = XXH3_64bits(&key_data, sizeof(key_data));

  PipelineDiskCacheEntry disk_entry;
  bool record_disk_entry =
      ::cvars::metal_pipeline_disk_cache && pipeline_disk_cache_file_;
  if (record_disk_entry) {
    disk_entry.pipeline_key = key;
    disk_entry.vertex_shader_cache_key = MetalShaderCache::GetCacheKey(
        vertex_translation->shader().ucode_data_hash(),
        vertex_translation->modification(),
        static_cast<uint32_t>(vertex_translation->shader().type()));
    if (pixel_translation) {
      disk_entry.pixel_shader_cache_key = MetalShaderCache::GetCacheKey(
          pixel_translation->shader().ucode_data_hash(),
          pixel_translation->modification(),
          static_cast<uint32_t>(pixel_translation->shader().type()));
    }
    disk_entry.sample_count = key_data.sample_count;
    disk_entry.depth_format = key_data.depth_format;
    disk_entry.stencil_format = key_data.stencil_format;
    std::memcpy(disk_entry.color_formats, key_data.color_formats,
                sizeof(key_data.color_formats));
    disk_entry.normalized_color_mask = key_data.normalized_color_mask;
    disk_entry.alpha_to_mask_enable = key_data.alpha_to_mask_enable;
    std::memcpy(disk_entry.blendcontrol, key_data.blendcontrol,
                sizeof(key_data.blendcontrol));
  }

  // Check cache
  auto it = pipeline_cache_.find(key);
  if (it != pipeline_cache_.end()) {
    return it->second;
  }

  // Create pipeline descriptor
  MTL::RenderPipelineDescriptor* desc =
      MTL::RenderPipelineDescriptor::alloc()->init();

  desc->setVertexFunction(vertex_translation->metal_function());

  if (pixel_translation && pixel_translation->metal_function()) {
    desc->setFragmentFunction(pixel_translation->metal_function());
  }

  // Set render target formats and sample count to match bound RTs.
  for (uint32_t i = 0; i < 4; ++i) {
    desc->colorAttachments()->object(i)->setPixelFormat(color_formats[i]);
  }
  desc->setDepthAttachmentPixelFormat(depth_format);
  desc->setStencilAttachmentPixelFormat(stencil_format);
  desc->setSampleCount(sample_count);
  desc->setAlphaToCoverageEnabled(key_data.alpha_to_mask_enable != 0);

  // Fixed-function blending and color write masks.
  // These are part of the render pipeline state, so the cache key must include
  // the relevant register-derived values (mask, RB_BLENDCONTROL, A2C).
  for (uint32_t i = 0; i < 4; ++i) {
    auto* color_attachment = desc->colorAttachments()->object(i);
    if (color_formats[i] == MTL::PixelFormatInvalid) {
      color_attachment->setWriteMask(MTL::ColorWriteMaskNone);
      color_attachment->setBlendingEnabled(false);
      continue;
    }

    uint32_t rt_write_mask = (key_data.normalized_color_mask >> (i * 4)) & 0xF;
    color_attachment->setWriteMask(ToMetalColorWriteMask(rt_write_mask));
    if (!rt_write_mask) {
      color_attachment->setBlendingEnabled(false);
      continue;
    }

    auto blendcontrol = regs.Get<reg::RB_BLENDCONTROL>(
        reg::RB_BLENDCONTROL::rt_register_indices[i]);
    MTL::BlendFactor src_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_srcblend);
    MTL::BlendFactor dst_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_destblend);
    MTL::BlendOperation op_rgb =
        ToMetalBlendOperation(blendcontrol.color_comb_fcn);
    MTL::BlendFactor src_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_srcblend);
    MTL::BlendFactor dst_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_destblend);
    MTL::BlendOperation op_alpha =
        ToMetalBlendOperation(blendcontrol.alpha_comb_fcn);

    bool blending_enabled =
        src_rgb != MTL::BlendFactorOne || dst_rgb != MTL::BlendFactorZero ||
        op_rgb != MTL::BlendOperationAdd || src_alpha != MTL::BlendFactorOne ||
        dst_alpha != MTL::BlendFactorZero || op_alpha != MTL::BlendOperationAdd;
    color_attachment->setBlendingEnabled(blending_enabled);
    if (blending_enabled) {
      color_attachment->setSourceRGBBlendFactor(src_rgb);
      color_attachment->setDestinationRGBBlendFactor(dst_rgb);
      color_attachment->setRgbBlendOperation(op_rgb);
      color_attachment->setSourceAlphaBlendFactor(src_alpha);
      color_attachment->setDestinationAlphaBlendFactor(dst_alpha);
      color_attachment->setAlphaBlendOperation(op_alpha);
    }
  }

  // Configure vertex fetch layout for MSC stage-in.
  // NOTE: The translated shaders use vfetch (buffer load) to read vertices
  // directly from shared memory via SRV descriptors, NOT stage-in attributes.
  // This vertex descriptor may be unnecessary.
  const Shader& vertex_shader_ref = vertex_translation->shader();
  const auto& vertex_bindings = vertex_shader_ref.vertex_bindings();
  if (!ShaderUsesVertexFetch(vertex_shader_ref) && !vertex_bindings.empty()) {
    auto map_vertex_format =
        [](const ParsedVertexFetchInstruction::Attributes& attrs)
        -> MTL::VertexFormat {
      using xenos::VertexFormat;
      switch (attrs.data_format) {
        case VertexFormat::k_8_8_8_8:
          if (attrs.is_integer) {
            return attrs.is_signed ? MTL::VertexFormatChar4
                                   : MTL::VertexFormatUChar4;
          }
          return attrs.is_signed ? MTL::VertexFormatChar4Normalized
                                 : MTL::VertexFormatUChar4Normalized;
        case VertexFormat::k_2_10_10_10:
          // Metal only supports normalized variants of 10:10:10:2.
          return attrs.is_signed ? MTL::VertexFormatInt1010102Normalized
                                 : MTL::VertexFormatUInt1010102Normalized;
        case VertexFormat::k_10_11_11:
        case VertexFormat::k_11_11_10:
          return MTL::VertexFormatFloatRG11B10;
        case VertexFormat::k_16_16:
          if (attrs.is_integer) {
            return attrs.is_signed ? MTL::VertexFormatShort2
                                   : MTL::VertexFormatUShort2;
          }
          return attrs.is_signed ? MTL::VertexFormatShort2Normalized
                                 : MTL::VertexFormatUShort2Normalized;
        case VertexFormat::k_16_16_16_16:
          if (attrs.is_integer) {
            return attrs.is_signed ? MTL::VertexFormatShort4
                                   : MTL::VertexFormatUShort4;
          }
          return attrs.is_signed ? MTL::VertexFormatShort4Normalized
                                 : MTL::VertexFormatUShort4Normalized;
        case VertexFormat::k_16_16_FLOAT:
          return MTL::VertexFormatHalf2;
        case VertexFormat::k_16_16_16_16_FLOAT:
          return MTL::VertexFormatHalf4;
        case VertexFormat::k_32:
          if (attrs.is_integer) {
            return attrs.is_signed ? MTL::VertexFormatInt
                                   : MTL::VertexFormatUInt;
          }
          return MTL::VertexFormatFloat;
        case VertexFormat::k_32_32:
          if (attrs.is_integer) {
            return attrs.is_signed ? MTL::VertexFormatInt2
                                   : MTL::VertexFormatUInt2;
          }
          return MTL::VertexFormatFloat2;
        case VertexFormat::k_32_32_32_FLOAT:
          return MTL::VertexFormatFloat3;
        case VertexFormat::k_32_32_32_32:
          if (attrs.is_integer) {
            return attrs.is_signed ? MTL::VertexFormatInt4
                                   : MTL::VertexFormatUInt4;
          }
          return MTL::VertexFormatFloat4;
        case VertexFormat::k_32_32_32_32_FLOAT:
          return MTL::VertexFormatFloat4;
        default:
          return MTL::VertexFormatInvalid;
      }
    };

    MTL::VertexDescriptor* vertex_desc = MTL::VertexDescriptor::alloc()->init();
    if (record_disk_entry) {
      disk_entry.vertex_attributes.clear();
      disk_entry.vertex_layouts.clear();
    }

    uint32_t attr_index = static_cast<uint32_t>(kIRStageInAttributeStartIndex);
    for (const auto& binding : vertex_bindings) {
      uint64_t buffer_index =
          kIRVertexBufferBindPoint + uint64_t(binding.binding_index);
      bool used_any_attribute = false;

      for (const auto& attr : binding.attributes) {
        MTL::VertexFormat fmt = map_vertex_format(attr.fetch_instr.attributes);
        if (fmt == MTL::VertexFormatInvalid) {
          ++attr_index;
          continue;
        }
        auto attr_desc = vertex_desc->attributes()->object(attr_index);
        attr_desc->setFormat(fmt);
        attr_desc->setOffset(
            static_cast<NS::UInteger>(attr.fetch_instr.attributes.offset * 4));
        attr_desc->setBufferIndex(static_cast<NS::UInteger>(buffer_index));
        if (record_disk_entry) {
          PipelineDiskCacheVertexAttribute cached_attr = {};
          cached_attr.attribute_index = attr_index;
          cached_attr.format = static_cast<uint32_t>(fmt);
          cached_attr.offset = attr.fetch_instr.attributes.offset * 4;
          cached_attr.buffer_index = static_cast<uint32_t>(buffer_index);
          disk_entry.vertex_attributes.push_back(cached_attr);
        }
        used_any_attribute = true;
        ++attr_index;
      }

      if (used_any_attribute) {
        auto layout = vertex_desc->layouts()->object(buffer_index);
        layout->setStride(binding.stride_words * 4);
        layout->setStepFunction(MTL::VertexStepFunctionPerVertex);
        layout->setStepRate(1);
        if (record_disk_entry) {
          PipelineDiskCacheVertexLayout cached_layout = {};
          cached_layout.buffer_index = static_cast<uint32_t>(buffer_index);
          cached_layout.stride = binding.stride_words * 4;
          cached_layout.step_function =
              static_cast<uint32_t>(MTL::VertexStepFunctionPerVertex);
          cached_layout.step_rate = 1;
          disk_entry.vertex_layouts.push_back(cached_layout);
        }
      }
    }

    desc->setVertexDescriptor(vertex_desc);
    vertex_desc->release();
  }

  {
    std::lock_guard<std::mutex> lock(pipeline_binary_archive_mutex_);
    if (pipeline_binary_archive_) {
      NS::Array* archives = NS::Array::array(pipeline_binary_archive_);
      desc->setBinaryArchives(archives);
      NS::Error* archive_error = nullptr;
      if (pipeline_binary_archive_->addRenderPipelineFunctions(
              desc, &archive_error)) {
        pipeline_binary_archive_dirty_ = true;
      }
    }
  }

  // Create pipeline state
  NS::Error* error = nullptr;
  MTL::RenderPipelineState* pipeline = nullptr;
  const auto pipeline_compile_begin = std::chrono::steady_clock::now();
  pipeline = device_->newRenderPipelineState(desc, &error);
  metal_perf_counters.pipeline_compiles.fetch_add(1, std::memory_order_relaxed);
  metal_perf_counters.pipeline_compile_us.fetch_add(
      MetalPerfMicrosecondsSince(pipeline_compile_begin),
      std::memory_order_relaxed);
  desc->release();

  if (!pipeline) {
    if (error) {
      XELOGE("Failed to create pipeline state: {}",
             error->localizedDescription()->utf8String());
    } else {
      XELOGE("Failed to create pipeline state (unknown error)");
    }
    return nullptr;
  }

  pipeline_cache_[key] = pipeline;
  if (record_disk_entry) {
    AppendPipelineDiskCacheEntry(disk_entry);
  }

  return pipeline;
}

MetalCommandProcessor::GeometryPipelineState*
MetalCommandProcessor::GetOrCreateGeometryPipelineState(
    MetalShader::MetalTranslation* vertex_translation,
    MetalShader::MetalTranslation* pixel_translation,
    GeometryShaderKey geometry_shader_key, const RegisterFile& regs) {
  if (!vertex_translation) {
    XELOGE("No valid vertex shader translation for geometry pipeline");
    return nullptr;
  }
  bool use_fallback_pixel_shader = (pixel_translation == nullptr);
  MTL::Library* pixel_library =
      use_fallback_pixel_shader ? nullptr : pixel_translation->metal_library();
  const char* pixel_function = use_fallback_pixel_shader
                                   ? nullptr
                                   : pixel_translation->function_name().c_str();
  if (use_fallback_pixel_shader) {
    if (!EnsureDepthOnlyPixelShader()) {
      XELOGE("Geometry pipeline: failed to create depth-only PS");
      return nullptr;
    }
    pixel_library = depth_only_pixel_library_;
    pixel_function = depth_only_pixel_function_name_.c_str();
  } else if (!pixel_library) {
    XELOGE("No valid pixel shader translation for geometry pipeline");
    return nullptr;
  }

  uint32_t sample_count = 1;
  MTL::PixelFormat color_formats[4] = {
      MTL::PixelFormatInvalid, MTL::PixelFormatInvalid, MTL::PixelFormatInvalid,
      MTL::PixelFormatInvalid};
  MTL::PixelFormat depth_format = MTL::PixelFormatInvalid;
  MTL::PixelFormat stencil_format = MTL::PixelFormatInvalid;
  if (render_target_cache_) {
    for (uint32_t i = 0; i < 4; ++i) {
      if (MTL::Texture* rt = render_target_cache_->GetColorTargetForDraw(i)) {
        color_formats[i] = rt->pixelFormat();
        if (rt->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(rt->sampleCount()));
        }
      }
    }
    if (color_formats[0] == MTL::PixelFormatInvalid) {
      if (MTL::Texture* dummy =
              render_target_cache_->GetDummyColorTargetForDraw()) {
        color_formats[0] = dummy->pixelFormat();
        if (dummy->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(dummy->sampleCount()));
        }
      }
    }
    if (MTL::Texture* depth_tex =
            render_target_cache_->GetDepthTargetForDraw()) {
      depth_format = depth_tex->pixelFormat();
      switch (depth_format) {
        case MTL::PixelFormatDepth32Float_Stencil8:
        case MTL::PixelFormatDepth24Unorm_Stencil8:
        case MTL::PixelFormatX32_Stencil8:
          stencil_format = depth_format;
          break;
        default:
          stencil_format = MTL::PixelFormatInvalid;
          break;
      }
      if (depth_tex->sampleCount() > 0) {
        sample_count = std::max<uint32_t>(
            sample_count, static_cast<uint32_t>(depth_tex->sampleCount()));
      }
    }
  }

  MTL::RenderPassDescriptor* pass_descriptor = current_render_pass_descriptor_;
  if (!pass_descriptor) {
    pass_descriptor = render_pass_descriptor_;
  }
  if (pass_descriptor) {
    sample_count = 1;
    for (uint32_t i = 0; i < 4; ++i) {
      color_formats[i] = MTL::PixelFormatInvalid;
    }
    depth_format = MTL::PixelFormatInvalid;
    stencil_format = MTL::PixelFormatInvalid;
    PopulatePipelineFormatsFromRenderPassDescriptor(
        pass_descriptor, color_formats, 4, &depth_format, &stencil_format,
        &sample_count);
  }
  bool pixel_shader_writes_depth =
      use_fallback_pixel_shader ||
      (pixel_translation && pixel_translation->shader().writes_depth());
  EnsureDepthFormatForDepthWritingFragment(
      "Geometry pipeline", pixel_shader_writes_depth, &depth_format);

  struct GeometryPipelineKey {
    const void* vs;
    const void* ps;
    uint32_t geometry_key;
    uint32_t sample_count;
    uint32_t depth_format;
    uint32_t stencil_format;
    uint32_t color_formats[4];
    uint32_t normalized_color_mask;
    uint32_t alpha_to_mask_enable;
    uint32_t blendcontrol[4];
  } key_data = {};

  key_data.vs = vertex_translation;
  key_data.ps = use_fallback_pixel_shader
                    ? static_cast<const void*>(pixel_library)
                    : static_cast<const void*>(pixel_translation);
  key_data.geometry_key = geometry_shader_key.key;
  key_data.sample_count = sample_count;
  key_data.depth_format = uint32_t(depth_format);
  key_data.stencil_format = uint32_t(stencil_format);
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.color_formats[i] = uint32_t(color_formats[i]);
  }
  uint32_t pixel_shader_writes_color_targets =
      use_fallback_pixel_shader
          ? 0
          : (pixel_translation
                 ? pixel_translation->shader().writes_color_targets()
                 : 0);
  key_data.normalized_color_mask =
      pixel_shader_writes_color_targets
          ? draw_util::GetNormalizedColorMask(regs,
                                              pixel_shader_writes_color_targets)
          : 0;
  auto rb_colorcontrol = regs.Get<reg::RB_COLORCONTROL>();
  key_data.alpha_to_mask_enable = rb_colorcontrol.alpha_to_mask_enable ? 1 : 0;
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.blendcontrol[i] =
        regs.Get<reg::RB_BLENDCONTROL>(
                reg::RB_BLENDCONTROL::rt_register_indices[i])
            .value;
  }
  uint64_t key = XXH3_64bits(&key_data, sizeof(key_data));

  auto it = geometry_pipeline_cache_.find(key);
  if (it != geometry_pipeline_cache_.end()) {
    return &it->second;
  }

  auto get_vertex_stage = [&]() -> GeometryVertexStageState* {
    auto vertex_it = geometry_vertex_stage_cache_.find(vertex_translation);
    if (vertex_it != geometry_vertex_stage_cache_.end()) {
      return &vertex_it->second;
    }

    std::vector<uint8_t> dxil_data = vertex_translation->dxil_data();
    if (dxil_data.empty()) {
      std::string dxil_error;
      if (!dxbc_to_dxil_converter_->Convert(
              vertex_translation->translated_binary(), dxil_data,
              &dxil_error)) {
        XELOGE("Geometry VS: DXBC to DXIL conversion failed: {}", dxil_error);
        return nullptr;
      }
    }

    struct InputAttribute {
      uint32_t input_slot = 0;
      uint32_t offset = 0;
      IRFormat format = IRFormatUnknown;
    };
    std::vector<InputAttribute> attribute_map;
    attribute_map.reserve(32);

    auto map_ir_format =
        [](const ParsedVertexFetchInstruction::Attributes& attrs) -> IRFormat {
      using xenos::VertexFormat;
      switch (attrs.data_format) {
        case VertexFormat::k_8_8_8_8:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR8G8B8A8Sint
                                   : IRFormatR8G8B8A8Uint;
          }
          return attrs.is_signed ? IRFormatR8G8B8A8Snorm
                                 : IRFormatR8G8B8A8Unorm;
        case VertexFormat::k_2_10_10_10:
          if (attrs.is_integer) {
            return IRFormatR10G10B10A2Uint;
          }
          return IRFormatR10G10B10A2Unorm;
        case VertexFormat::k_10_11_11:
        case VertexFormat::k_11_11_10:
          return IRFormatR11G11B10Float;
        case VertexFormat::k_16_16:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR16G16Sint : IRFormatR16G16Uint;
          }
          return attrs.is_signed ? IRFormatR16G16Snorm : IRFormatR16G16Unorm;
        case VertexFormat::k_16_16_16_16:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR16G16B16A16Sint
                                   : IRFormatR16G16B16A16Uint;
          }
          return attrs.is_signed ? IRFormatR16G16B16A16Snorm
                                 : IRFormatR16G16B16A16Unorm;
        case VertexFormat::k_16_16_FLOAT:
          return IRFormatR16G16Float;
        case VertexFormat::k_16_16_16_16_FLOAT:
          return IRFormatR16G16B16A16Float;
        case VertexFormat::k_32:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR32Sint : IRFormatR32Uint;
          }
          return IRFormatR32Float;
        case VertexFormat::k_32_32:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR32G32Sint : IRFormatR32G32Uint;
          }
          return IRFormatR32G32Float;
        case VertexFormat::k_32_32_32_FLOAT:
          return IRFormatR32G32B32Float;
        case VertexFormat::k_32_32_32_32:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR32G32B32A32Sint
                                   : IRFormatR32G32B32A32Uint;
          }
          return IRFormatR32G32B32A32Float;
        case VertexFormat::k_32_32_32_32_FLOAT:
          return IRFormatR32G32B32A32Float;
        default:
          return IRFormatUnknown;
      }
    };

    const Shader& vertex_shader_ref = vertex_translation->shader();
    const auto& vertex_bindings = vertex_shader_ref.vertex_bindings();
    uint32_t attr_index = 0;
    for (const auto& binding : vertex_bindings) {
      for (const auto& attr : binding.attributes) {
        if (attr_index >= 31) {
          break;
        }
        InputAttribute mapped = {};
        mapped.input_slot = static_cast<uint32_t>(binding.binding_index);
        mapped.offset =
            static_cast<uint32_t>(attr.fetch_instr.attributes.offset * 4);
        mapped.format = map_ir_format(attr.fetch_instr.attributes);
        attribute_map.push_back(mapped);
        ++attr_index;
      }
      if (attr_index >= 31) {
        break;
      }
    }

    IRInputTopology input_topology = IRInputTopologyUndefined;
    switch (geometry_shader_key.type) {
      case PipelineGeometryShader::kPointList:
        input_topology = IRInputTopologyPoint;
        break;
      case PipelineGeometryShader::kRectangleList:
        input_topology = IRInputTopologyTriangle;
        break;
      case PipelineGeometryShader::kQuadList:
        // Quad lists use LineWithAdjacency in DXBC; MSC input topology doesn't
        // model adjacency, so leave undefined to avoid mismatches.
        input_topology = IRInputTopologyUndefined;
        break;
      default:
        input_topology = IRInputTopologyUndefined;
        break;
    }
    MetalShaderConversionResult vertex_result;
    MetalShaderReflectionInfo vertex_reflection;

    // First pass: get reflection for vertex inputs.
    if (!metal_shader_converter_->ConvertWithStageEx(
            MetalShaderStage::kVertex, dxil_data, vertex_result,
            &vertex_reflection, nullptr, nullptr, true,
            static_cast<int>(input_topology))) {
      XELOGE("Geometry VS: DXIL to Metal conversion failed: {}",
             vertex_result.error_message);
      return nullptr;
    }

    IRVersionedInputLayoutDescriptor input_layout = {};
    input_layout.version = IRInputLayoutDescriptorVersion_1;
    input_layout.desc_1_0.numElements = 0;
    std::vector<std::string> semantic_names_storage;
    if (!vertex_reflection.vertex_inputs.empty()) {
      semantic_names_storage.reserve(vertex_reflection.vertex_inputs.size());
      uint32_t element_count = 0;
      for (const auto& input : vertex_reflection.vertex_inputs) {
        if (element_count >= 31) {
          break;
        }
        if (input.attribute_index >= attribute_map.size()) {
          XELOGW("Geometry VS: vertex input {} out of range (max {})",
                 input.attribute_index, attribute_map.size());
          continue;
        }
        const InputAttribute& mapped = attribute_map[input.attribute_index];
        if (mapped.format == IRFormatUnknown) {
          XELOGW("Geometry VS: unknown IRFormat for vertex input {}",
                 input.attribute_index);
          continue;
        }
        std::string semantic_base = input.name;
        uint32_t semantic_index = 0;
        if (!semantic_base.empty()) {
          size_t digit_pos = semantic_base.size();
          while (digit_pos > 0 && std::isdigit(static_cast<unsigned char>(
                                      semantic_base[digit_pos - 1]))) {
            --digit_pos;
          }
          if (digit_pos < semantic_base.size()) {
            semantic_index = static_cast<uint32_t>(
                std::strtoul(semantic_base.c_str() + digit_pos, nullptr, 10));
            semantic_base.resize(digit_pos);
          }
        }
        if (semantic_base.empty()) {
          semantic_base = "TEXCOORD";
        }
        semantic_names_storage.push_back(std::move(semantic_base));
        input_layout.desc_1_0.semanticNames[element_count] =
            semantic_names_storage.back().c_str();
        IRInputElementDescriptor1& element =
            input_layout.desc_1_0.inputElementDescs[element_count];
        element.semanticIndex = semantic_index;
        element.format = mapped.format;
        element.inputSlot = mapped.input_slot;
        element.alignedByteOffset = mapped.offset;
        element.instanceDataStepRate = 0;
        element.inputSlotClass = IRInputClassificationPerVertexData;
        ++element_count;
      }
      input_layout.desc_1_0.numElements = element_count;
    }

    // Second pass: synthesize stage-in using the input layout.
    std::vector<uint8_t> stage_in_metallib;
    if (!metal_shader_converter_->ConvertWithStageEx(
            MetalShaderStage::kVertex, dxil_data, vertex_result,
            &vertex_reflection, &input_layout, &stage_in_metallib, true,
            static_cast<int>(input_topology))) {
      XELOGE("Geometry VS: DXIL to Metal conversion failed: {}",
             vertex_result.error_message);
      return nullptr;
    }
    if (stage_in_metallib.empty()) {
      XELOGE(
          "Geometry VS: Failed to synthesize stage-in function "
          "(vertex_inputs={}, output_size={})",
          vertex_reflection.vertex_input_count,
          vertex_reflection.vertex_output_size_in_bytes);
      return nullptr;
    }

    NS::Error* error = nullptr;
    dispatch_data_t vertex_data = dispatch_data_create(
        vertex_result.metallib_data.data(), vertex_result.metallib_data.size(),
        nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    MTL::Library* vertex_library = device_->newLibrary(vertex_data, &error);
    dispatch_release(vertex_data);
    if (!vertex_library) {
      XELOGE("Geometry VS: Failed to create Metal library: {}",
             error ? error->localizedDescription()->utf8String()
                   : "unknown error");
      return nullptr;
    }

    NS::Error* stage_in_error = nullptr;
    dispatch_data_t stage_in_data =
        dispatch_data_create(stage_in_metallib.data(), stage_in_metallib.size(),
                             nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    MTL::Library* stage_in_library =
        device_->newLibrary(stage_in_data, &stage_in_error);
    dispatch_release(stage_in_data);
    if (!stage_in_library) {
      XELOGE("Geometry VS: Failed to create stage-in library: {}",
             stage_in_error
                 ? stage_in_error->localizedDescription()->utf8String()
                 : "unknown error");
      vertex_library->release();
      return nullptr;
    }

    GeometryVertexStageState state;
    state.library = vertex_library;
    state.stage_in_library = stage_in_library;
    state.function_name = vertex_result.function_name;
    state.vertex_output_size_in_bytes =
        vertex_reflection.vertex_output_size_in_bytes;
    if (state.vertex_output_size_in_bytes == 0) {
      XELOGE(
          "Geometry VS: reflection returned zero output size "
          "(vertex_inputs={})",
          vertex_reflection.vertex_input_count);
    }

    auto [inserted_it, inserted] = geometry_vertex_stage_cache_.emplace(
        vertex_translation, std::move(state));
    return &inserted_it->second;
  };

  auto get_geometry_stage = [&]() -> GeometryShaderStageState* {
    auto geom_it = geometry_shader_stage_cache_.find(geometry_shader_key);
    if (geom_it != geometry_shader_stage_cache_.end()) {
      return &geom_it->second;
    }

    const std::vector<uint32_t>& dxbc_dwords =
        GetGeometryShader(geometry_shader_key);
    std::vector<uint8_t> dxbc_bytes(dxbc_dwords.size() * sizeof(uint32_t));
    std::memcpy(dxbc_bytes.data(), dxbc_dwords.data(), dxbc_bytes.size());

    std::vector<uint8_t> dxil_data;
    std::string dxil_error;
    if (!dxbc_to_dxil_converter_->Convert(dxbc_bytes, dxil_data, &dxil_error)) {
      XELOGE("Geometry GS: DXBC to DXIL conversion failed: {}", dxil_error);
      return nullptr;
    }

    IRInputTopology input_topology = IRInputTopologyUndefined;
    switch (geometry_shader_key.type) {
      case PipelineGeometryShader::kPointList:
        input_topology = IRInputTopologyPoint;
        break;
      case PipelineGeometryShader::kRectangleList:
        input_topology = IRInputTopologyTriangle;
        break;
      case PipelineGeometryShader::kQuadList:
        // Quad lists use LineWithAdjacency in DXBC; MSC input topology doesn't
        // model adjacency, so leave undefined to avoid mismatches.
        input_topology = IRInputTopologyUndefined;
        break;
      default:
        input_topology = IRInputTopologyUndefined;
        break;
    }
    MetalShaderConversionResult geometry_result;
    MetalShaderReflectionInfo geometry_reflection;
    if (!metal_shader_converter_->ConvertWithStageEx(
            MetalShaderStage::kGeometry, dxil_data, geometry_result,
            &geometry_reflection, nullptr, nullptr, true,
            static_cast<int>(input_topology))) {
      XELOGE("Geometry GS: DXIL to Metal conversion failed: {}",
             geometry_result.error_message);
      return nullptr;
    }
    if (!geometry_result.has_mesh_stage &&
        !geometry_result.has_geometry_stage) {
      XELOGE(
          "Geometry GS: MSC did not emit mesh or geometry stage (mesh={}, "
          "geometry={})",
          geometry_result.has_mesh_stage, geometry_result.has_geometry_stage);
      return nullptr;
    }
    if (!geometry_result.has_mesh_stage) {
      static bool mesh_missing_logged = false;
      if (!mesh_missing_logged) {
        mesh_missing_logged = true;
        XELOGW(
            "Geometry GS: MSC did not emit mesh stage; using geometry stage "
            "library");
      }
    }

    NS::Error* error = nullptr;
    dispatch_data_t geometry_data =
        dispatch_data_create(geometry_result.metallib_data.data(),
                             geometry_result.metallib_data.size(), nullptr,
                             DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    MTL::Library* geometry_library = device_->newLibrary(geometry_data, &error);
    dispatch_release(geometry_data);
    if (!geometry_library) {
      XELOGE("Geometry GS: Failed to create Metal library: {}",
             error ? error->localizedDescription()->utf8String()
                   : "unknown error");
      return nullptr;
    }

    GeometryShaderStageState state;
    state.library = geometry_library;
    state.function_name = geometry_result.function_name;
    state.max_input_primitives_per_mesh_threadgroup =
        geometry_reflection.gs_max_input_primitives_per_mesh_threadgroup;
    state.function_constants = geometry_reflection.function_constants;
    if (state.max_input_primitives_per_mesh_threadgroup == 0) {
      XELOGE("Geometry GS: reflection returned zero max input primitives");
    }

    auto [inserted_it, inserted] = geometry_shader_stage_cache_.emplace(
        geometry_shader_key, std::move(state));
    return &inserted_it->second;
  };

  GeometryVertexStageState* vertex_stage = get_vertex_stage();
  if (!vertex_stage || !vertex_stage->library ||
      !vertex_stage->stage_in_library) {
    return nullptr;
  }
  GeometryShaderStageState* geometry_stage = get_geometry_stage();
  if (!geometry_stage || !geometry_stage->library) {
    return nullptr;
  }

  MTL::MeshRenderPipelineDescriptor* desc =
      MTL::MeshRenderPipelineDescriptor::alloc()->init();

  for (uint32_t i = 0; i < 4; ++i) {
    desc->colorAttachments()->object(i)->setPixelFormat(color_formats[i]);
  }
  desc->setDepthAttachmentPixelFormat(depth_format);
  desc->setStencilAttachmentPixelFormat(stencil_format);
  desc->setRasterSampleCount(sample_count);
  desc->setAlphaToCoverageEnabled(key_data.alpha_to_mask_enable != 0);

  for (uint32_t i = 0; i < 4; ++i) {
    auto* color_attachment = desc->colorAttachments()->object(i);
    if (color_formats[i] == MTL::PixelFormatInvalid) {
      color_attachment->setWriteMask(MTL::ColorWriteMaskNone);
      color_attachment->setBlendingEnabled(false);
      continue;
    }

    uint32_t rt_write_mask = (key_data.normalized_color_mask >> (i * 4)) & 0xF;
    color_attachment->setWriteMask(ToMetalColorWriteMask(rt_write_mask));
    if (!rt_write_mask) {
      color_attachment->setBlendingEnabled(false);
      continue;
    }

    auto blendcontrol = regs.Get<reg::RB_BLENDCONTROL>(
        reg::RB_BLENDCONTROL::rt_register_indices[i]);
    MTL::BlendFactor src_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_srcblend);
    MTL::BlendFactor dst_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_destblend);
    MTL::BlendOperation op_rgb =
        ToMetalBlendOperation(blendcontrol.color_comb_fcn);
    MTL::BlendFactor src_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_srcblend);
    MTL::BlendFactor dst_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_destblend);
    MTL::BlendOperation op_alpha =
        ToMetalBlendOperation(blendcontrol.alpha_comb_fcn);

    bool blending_enabled =
        src_rgb != MTL::BlendFactorOne || dst_rgb != MTL::BlendFactorZero ||
        op_rgb != MTL::BlendOperationAdd || src_alpha != MTL::BlendFactorOne ||
        dst_alpha != MTL::BlendFactorZero || op_alpha != MTL::BlendOperationAdd;
    color_attachment->setBlendingEnabled(blending_enabled);
    if (blending_enabled) {
      color_attachment->setSourceRGBBlendFactor(src_rgb);
      color_attachment->setDestinationRGBBlendFactor(dst_rgb);
      color_attachment->setRgbBlendOperation(op_rgb);
      color_attachment->setSourceAlphaBlendFactor(src_alpha);
      color_attachment->setDestinationAlphaBlendFactor(dst_alpha);
      color_attachment->setAlphaBlendOperation(op_alpha);
    }
  }
  if (!vertex_stage->vertex_output_size_in_bytes ||
      !geometry_stage->max_input_primitives_per_mesh_threadgroup) {
    XELOGE(
        "Geometry pipeline: invalid reflection (vs_output={}, gs_max_input={})",
        vertex_stage->vertex_output_size_in_bytes,
        geometry_stage->max_input_primitives_per_mesh_threadgroup);
    return nullptr;
  }

  IRGeometryEmulationPipelineDescriptor ir_desc = {};
  ir_desc.stageInLibrary = vertex_stage->stage_in_library;
  ir_desc.vertexLibrary = vertex_stage->library;
  ir_desc.vertexFunctionName = vertex_stage->function_name.c_str();
  ir_desc.geometryLibrary = geometry_stage->library;
  ir_desc.geometryFunctionName = geometry_stage->function_name.c_str();
  ir_desc.fragmentLibrary = pixel_library;
  ir_desc.fragmentFunctionName = pixel_function;
  ir_desc.basePipelineDescriptor = desc;
  ir_desc.pipelineConfig.gsVertexSizeInBytes =
      vertex_stage->vertex_output_size_in_bytes;
  ir_desc.pipelineConfig.gsMaxInputPrimitivesPerMeshThreadgroup =
      geometry_stage->max_input_primitives_per_mesh_threadgroup;

  NS::Error* error = nullptr;
  MTL::RenderPipelineState* pipeline =
      IRRuntimeNewGeometryEmulationPipeline(device_, &ir_desc, &error);
  desc->release();

  if (!pipeline) {
    XELOGE(
        "Failed to create geometry pipeline state: {}",
        error ? error->localizedDescription()->utf8String() : "unknown error");
    XELOGE(
        "Geometry pipeline details: vs_fn='{}' gs_fn='{}' ps_fn='{}' "
        "depth_format={} stencil_format={} samples={}",
        vertex_stage->function_name, geometry_stage->function_name,
        pixel_function ? pixel_function : "<null>", uint32_t(depth_format),
        uint32_t(stencil_format), sample_count);
    LogMetalErrorDetails("Geometry pipeline error", error);
    return nullptr;
  }

  GeometryPipelineState state;
  state.pipeline = pipeline;
  state.gs_vertex_size_in_bytes = ir_desc.pipelineConfig.gsVertexSizeInBytes;
  state.gs_max_input_primitives_per_mesh_threadgroup =
      ir_desc.pipelineConfig.gsMaxInputPrimitivesPerMeshThreadgroup;

  auto [inserted_it, inserted] =
      geometry_pipeline_cache_.emplace(key, std::move(state));
  return &inserted_it->second;
}

MetalCommandProcessor::TessellationPipelineState*
MetalCommandProcessor::GetOrCreateTessellationPipelineState(
    MetalShader::MetalTranslation* domain_translation,
    MetalShader::MetalTranslation* pixel_translation,
    const PrimitiveProcessor::ProcessingResult& primitive_processing_result,
    const RegisterFile& regs) {
  if (!domain_translation) {
    XELOGE("No valid domain shader translation for tessellation pipeline");
    return nullptr;
  }
  bool use_fallback_pixel_shader = (pixel_translation == nullptr);
  MTL::Library* pixel_library =
      use_fallback_pixel_shader ? nullptr : pixel_translation->metal_library();
  const char* pixel_function = use_fallback_pixel_shader
                                   ? nullptr
                                   : pixel_translation->function_name().c_str();
  if (use_fallback_pixel_shader) {
    if (!EnsureDepthOnlyPixelShader()) {
      XELOGE("Tessellation pipeline: failed to create depth-only PS");
      return nullptr;
    }
    pixel_library = depth_only_pixel_library_;
    pixel_function = depth_only_pixel_function_name_.c_str();
  } else if (!pixel_library) {
    XELOGE("No valid pixel shader translation for tessellation pipeline");
    return nullptr;
  }

  uint32_t sample_count = 1;
  MTL::PixelFormat color_formats[4] = {
      MTL::PixelFormatInvalid, MTL::PixelFormatInvalid, MTL::PixelFormatInvalid,
      MTL::PixelFormatInvalid};
  MTL::PixelFormat depth_format = MTL::PixelFormatInvalid;
  MTL::PixelFormat stencil_format = MTL::PixelFormatInvalid;
  if (render_target_cache_) {
    for (uint32_t i = 0; i < 4; ++i) {
      if (MTL::Texture* rt = render_target_cache_->GetColorTargetForDraw(i)) {
        color_formats[i] = rt->pixelFormat();
        if (rt->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(rt->sampleCount()));
        }
      }
    }
    if (color_formats[0] == MTL::PixelFormatInvalid) {
      if (MTL::Texture* dummy =
              render_target_cache_->GetDummyColorTargetForDraw()) {
        color_formats[0] = dummy->pixelFormat();
        if (dummy->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(dummy->sampleCount()));
        }
      }
    }
    if (MTL::Texture* depth_tex =
            render_target_cache_->GetDepthTargetForDraw()) {
      depth_format = depth_tex->pixelFormat();
      switch (depth_format) {
        case MTL::PixelFormatDepth32Float_Stencil8:
        case MTL::PixelFormatDepth24Unorm_Stencil8:
        case MTL::PixelFormatX32_Stencil8:
          stencil_format = depth_format;
          break;
        default:
          stencil_format = MTL::PixelFormatInvalid;
          break;
      }
      if (depth_tex->sampleCount() > 0) {
        sample_count = std::max<uint32_t>(
            sample_count, static_cast<uint32_t>(depth_tex->sampleCount()));
      }
    }
  }

  MTL::RenderPassDescriptor* pass_descriptor = current_render_pass_descriptor_;
  if (!pass_descriptor) {
    pass_descriptor = render_pass_descriptor_;
  }
  if (pass_descriptor) {
    sample_count = 1;
    for (uint32_t i = 0; i < 4; ++i) {
      color_formats[i] = MTL::PixelFormatInvalid;
    }
    depth_format = MTL::PixelFormatInvalid;
    stencil_format = MTL::PixelFormatInvalid;
    PopulatePipelineFormatsFromRenderPassDescriptor(
        pass_descriptor, color_formats, 4, &depth_format, &stencil_format,
        &sample_count);
  }
  bool pixel_shader_writes_depth =
      use_fallback_pixel_shader ||
      (pixel_translation && pixel_translation->shader().writes_depth());
  EnsureDepthFormatForDepthWritingFragment(
      "Tessellation pipeline", pixel_shader_writes_depth, &depth_format);

  struct TessellationPipelineKey {
    const void* ds;
    const void* ps;
    uint32_t host_vs_type;
    uint32_t tessellation_mode;
    uint32_t host_prim;
    uint32_t sample_count;
    uint32_t depth_format;
    uint32_t stencil_format;
    uint32_t color_formats[4];
    uint32_t normalized_color_mask;
    uint32_t alpha_to_mask_enable;
    uint32_t blendcontrol[4];
  } key_data = {};

  key_data.ds = domain_translation;
  key_data.ps = use_fallback_pixel_shader
                    ? static_cast<const void*>(pixel_library)
                    : static_cast<const void*>(pixel_translation);
  key_data.host_vs_type =
      uint32_t(primitive_processing_result.host_vertex_shader_type);
  key_data.tessellation_mode =
      uint32_t(primitive_processing_result.tessellation_mode);
  key_data.host_prim =
      uint32_t(primitive_processing_result.host_primitive_type);
  key_data.sample_count = sample_count;
  key_data.depth_format = uint32_t(depth_format);
  key_data.stencil_format = uint32_t(stencil_format);
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.color_formats[i] = uint32_t(color_formats[i]);
  }
  uint32_t pixel_shader_writes_color_targets =
      use_fallback_pixel_shader
          ? 0
          : (pixel_translation
                 ? pixel_translation->shader().writes_color_targets()
                 : 0);
  key_data.normalized_color_mask =
      pixel_shader_writes_color_targets
          ? draw_util::GetNormalizedColorMask(regs,
                                              pixel_shader_writes_color_targets)
          : 0;
  auto rb_colorcontrol = regs.Get<reg::RB_COLORCONTROL>();
  key_data.alpha_to_mask_enable = rb_colorcontrol.alpha_to_mask_enable ? 1 : 0;
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.blendcontrol[i] =
        regs.Get<reg::RB_BLENDCONTROL>(
                reg::RB_BLENDCONTROL::rt_register_indices[i])
            .value;
  }
  uint64_t key = XXH3_64bits(&key_data, sizeof(key_data));

  auto it = tessellation_pipeline_cache_.find(key);
  if (it != tessellation_pipeline_cache_.end()) {
    return &it->second;
  }

  xenos::TessellationMode tessellation_mode =
      primitive_processing_result.tessellation_mode;

  auto get_vertex_stage = [&]() -> TessellationVertexStageState* {
    struct VertexStageKey {
      const void* shader;
      uint32_t tessellation_mode;
    } vertex_key = {domain_translation, uint32_t(tessellation_mode)};
    uint32_t vertex_key_hash =
        uint32_t(XXH3_64bits(&vertex_key, sizeof(vertex_key)));
    auto vertex_it = tessellation_vertex_stage_cache_.find(vertex_key_hash);
    if (vertex_it != tessellation_vertex_stage_cache_.end()) {
      return &vertex_it->second;
    }

    const uint8_t* vs_bytes = nullptr;
    size_t vs_size = 0;
    if (tessellation_mode == xenos::TessellationMode::kAdaptive) {
      vs_bytes = ::tessellation_adaptive_vs;
      vs_size = sizeof(::tessellation_adaptive_vs);
    } else {
      vs_bytes = ::tessellation_indexed_vs;
      vs_size = sizeof(::tessellation_indexed_vs);
    }
    std::vector<uint8_t> dxbc_bytes(vs_bytes, vs_bytes + vs_size);
    std::vector<uint8_t> dxil_data;
    std::string dxil_error;
    if (!dxbc_to_dxil_converter_->Convert(dxbc_bytes, dxil_data, &dxil_error)) {
      XELOGE("Tessellation VS: DXBC to DXIL conversion failed: {}", dxil_error);
      return nullptr;
    }

    struct InputAttribute {
      uint32_t input_slot = 0;
      uint32_t offset = 0;
      IRFormat format = IRFormatUnknown;
    };
    std::vector<InputAttribute> attribute_map;
    attribute_map.reserve(32);

    auto map_ir_format =
        [](const ParsedVertexFetchInstruction::Attributes& attrs) -> IRFormat {
      using xenos::VertexFormat;
      switch (attrs.data_format) {
        case VertexFormat::k_8_8_8_8:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR8G8B8A8Sint
                                   : IRFormatR8G8B8A8Uint;
          }
          return attrs.is_signed ? IRFormatR8G8B8A8Snorm
                                 : IRFormatR8G8B8A8Unorm;
        case VertexFormat::k_2_10_10_10:
          if (attrs.is_integer) {
            return IRFormatR10G10B10A2Uint;
          }
          return IRFormatR10G10B10A2Unorm;
        case VertexFormat::k_10_11_11:
        case VertexFormat::k_11_11_10:
          return IRFormatR11G11B10Float;
        case VertexFormat::k_16_16:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR16G16Sint : IRFormatR16G16Uint;
          }
          return attrs.is_signed ? IRFormatR16G16Snorm : IRFormatR16G16Unorm;
        case VertexFormat::k_16_16_16_16:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR16G16B16A16Sint
                                   : IRFormatR16G16B16A16Uint;
          }
          return attrs.is_signed ? IRFormatR16G16B16A16Snorm
                                 : IRFormatR16G16B16A16Unorm;
        case VertexFormat::k_16_16_FLOAT:
          return IRFormatR16G16Float;
        case VertexFormat::k_16_16_16_16_FLOAT:
          return IRFormatR16G16B16A16Float;
        case VertexFormat::k_32:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR32Sint : IRFormatR32Uint;
          }
          return IRFormatR32Float;
        case VertexFormat::k_32_32:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR32G32Sint : IRFormatR32G32Uint;
          }
          return IRFormatR32G32Float;
        case VertexFormat::k_32_32_32_FLOAT:
          return IRFormatR32G32B32Float;
        case VertexFormat::k_32_32_32_32:
          if (attrs.is_integer) {
            return attrs.is_signed ? IRFormatR32G32B32A32Sint
                                   : IRFormatR32G32B32A32Uint;
          }
          return IRFormatR32G32B32A32Float;
        case VertexFormat::k_32_32_32_32_FLOAT:
          return IRFormatR32G32B32A32Float;
        default:
          return IRFormatUnknown;
      }
    };

    const Shader& vertex_shader_ref = domain_translation->shader();
    const auto& vertex_bindings = vertex_shader_ref.vertex_bindings();
    uint32_t attr_index = 0;
    for (const auto& binding : vertex_bindings) {
      for (const auto& attr : binding.attributes) {
        if (attr_index >= 31) {
          break;
        }
        InputAttribute mapped = {};
        mapped.input_slot = static_cast<uint32_t>(binding.binding_index);
        mapped.offset =
            static_cast<uint32_t>(attr.fetch_instr.attributes.offset * 4);
        mapped.format = map_ir_format(attr.fetch_instr.attributes);
        attribute_map.push_back(mapped);
        ++attr_index;
      }
      if (attr_index >= 31) {
        break;
      }
    }

    IRInputTopology input_topology = IRInputTopologyUndefined;

    MetalShaderConversionResult vertex_result;
    MetalShaderReflectionInfo vertex_reflection;
    if (!metal_shader_converter_->ConvertWithStageEx(
            MetalShaderStage::kVertex, dxil_data, vertex_result,
            &vertex_reflection, nullptr, nullptr, true,
            static_cast<int>(input_topology))) {
      XELOGE("Tessellation VS: DXIL to Metal conversion failed: {}",
             vertex_result.error_message);
      return nullptr;
    }

    IRVersionedInputLayoutDescriptor input_layout = {};
    input_layout.version = IRInputLayoutDescriptorVersion_1;
    input_layout.desc_1_0.numElements = 0;
    std::vector<std::string> semantic_names_storage;
    if (!vertex_reflection.vertex_inputs.empty()) {
      semantic_names_storage.reserve(vertex_reflection.vertex_inputs.size());
      uint32_t element_count = 0;
      for (const auto& input : vertex_reflection.vertex_inputs) {
        if (element_count >= 31) {
          break;
        }
        if (input.attribute_index >= attribute_map.size()) {
          XELOGW("Tessellation VS: vertex input {} out of range (max {})",
                 input.attribute_index, attribute_map.size());
          continue;
        }
        const InputAttribute& mapped = attribute_map[input.attribute_index];
        if (mapped.format == IRFormatUnknown) {
          XELOGW("Tessellation VS: unknown IRFormat for vertex input {}",
                 input.attribute_index);
          continue;
        }
        std::string semantic_base = input.name;
        uint32_t semantic_index = 0;
        if (!semantic_base.empty()) {
          size_t digit_pos = semantic_base.size();
          while (digit_pos > 0 && std::isdigit(static_cast<unsigned char>(
                                      semantic_base[digit_pos - 1]))) {
            --digit_pos;
          }
          if (digit_pos < semantic_base.size()) {
            semantic_index = static_cast<uint32_t>(
                std::strtoul(semantic_base.c_str() + digit_pos, nullptr, 10));
            semantic_base.resize(digit_pos);
          }
        }
        if (semantic_base.empty()) {
          semantic_base = "TEXCOORD";
        }
        semantic_names_storage.push_back(std::move(semantic_base));
        input_layout.desc_1_0.semanticNames[element_count] =
            semantic_names_storage.back().c_str();
        IRInputElementDescriptor1& element =
            input_layout.desc_1_0.inputElementDescs[element_count];
        element.semanticIndex = semantic_index;
        element.format = mapped.format;
        element.inputSlot = mapped.input_slot;
        element.alignedByteOffset = mapped.offset;
        element.instanceDataStepRate = 0;
        element.inputSlotClass = IRInputClassificationPerVertexData;
        ++element_count;
      }
      input_layout.desc_1_0.numElements = element_count;
    }

    std::vector<uint8_t> stage_in_metallib;
    if (!metal_shader_converter_->ConvertWithStageEx(
            MetalShaderStage::kVertex, dxil_data, vertex_result,
            &vertex_reflection, &input_layout, &stage_in_metallib, true,
            static_cast<int>(input_topology))) {
      XELOGE("Tessellation VS: DXIL to Metal conversion failed: {}",
             vertex_result.error_message);
      return nullptr;
    }
    if (stage_in_metallib.empty()) {
      XELOGE("Tessellation VS: Failed to synthesize stage-in function");
      return nullptr;
    }

    NS::Error* error = nullptr;
    dispatch_data_t vertex_data = dispatch_data_create(
        vertex_result.metallib_data.data(), vertex_result.metallib_data.size(),
        nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    MTL::Library* vertex_library = device_->newLibrary(vertex_data, &error);
    dispatch_release(vertex_data);
    if (!vertex_library) {
      XELOGE("Tessellation VS: Failed to create Metal library: {}",
             error ? error->localizedDescription()->utf8String()
                   : "unknown error");
      return nullptr;
    }

    NS::Error* stage_in_error = nullptr;
    dispatch_data_t stage_in_data =
        dispatch_data_create(stage_in_metallib.data(), stage_in_metallib.size(),
                             nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    MTL::Library* stage_in_library =
        device_->newLibrary(stage_in_data, &stage_in_error);
    dispatch_release(stage_in_data);
    if (!stage_in_library) {
      XELOGE("Tessellation VS: Failed to create stage-in library: {}",
             stage_in_error
                 ? stage_in_error->localizedDescription()->utf8String()
                 : "unknown error");
      vertex_library->release();
      return nullptr;
    }

    TessellationVertexStageState state;
    state.library = vertex_library;
    state.stage_in_library = stage_in_library;
    state.function_name = vertex_result.function_name;
    state.vertex_output_size_in_bytes =
        vertex_reflection.vertex_output_size_in_bytes;
    if (state.vertex_output_size_in_bytes == 0) {
      XELOGE("Tessellation VS: reflection returned zero output size");
    }

    auto [inserted_it, inserted] = tessellation_vertex_stage_cache_.emplace(
        vertex_key_hash, std::move(state));
    return &inserted_it->second;
  };

  auto get_hull_stage = [&]() -> TessellationHullStageState* {
    struct HullStageKey {
      uint32_t host_vs_type;
      uint32_t tessellation_mode;
    } hull_key = {uint32_t(primitive_processing_result.host_vertex_shader_type),
                  uint32_t(tessellation_mode)};
    uint64_t hull_key_hash = XXH3_64bits(&hull_key, sizeof(hull_key));
    auto hull_it = tessellation_hull_stage_cache_.find(hull_key_hash);
    if (hull_it != tessellation_hull_stage_cache_.end()) {
      return &hull_it->second;
    }

    const uint8_t* hs_bytes = nullptr;
    size_t hs_size = 0;
    switch (tessellation_mode) {
      case xenos::TessellationMode::kDiscrete:
        switch (primitive_processing_result.host_vertex_shader_type) {
          case Shader::HostVertexShaderType::kTriangleDomainCPIndexed:
            hs_bytes = ::discrete_triangle_3cp_hs;
            hs_size = sizeof(::discrete_triangle_3cp_hs);
            break;
          case Shader::HostVertexShaderType::kTriangleDomainPatchIndexed:
            hs_bytes = ::discrete_triangle_1cp_hs;
            hs_size = sizeof(::discrete_triangle_1cp_hs);
            break;
          case Shader::HostVertexShaderType::kQuadDomainCPIndexed:
            hs_bytes = ::discrete_quad_4cp_hs;
            hs_size = sizeof(::discrete_quad_4cp_hs);
            break;
          case Shader::HostVertexShaderType::kQuadDomainPatchIndexed:
            hs_bytes = ::discrete_quad_1cp_hs;
            hs_size = sizeof(::discrete_quad_1cp_hs);
            break;
          default:
            XELOGE(
                "Tessellation HS: unsupported host vertex shader type {}",
                uint32_t(primitive_processing_result.host_vertex_shader_type));
            return nullptr;
        }
        break;
      case xenos::TessellationMode::kContinuous:
        switch (primitive_processing_result.host_vertex_shader_type) {
          case Shader::HostVertexShaderType::kTriangleDomainCPIndexed:
            hs_bytes = ::continuous_triangle_3cp_hs;
            hs_size = sizeof(::continuous_triangle_3cp_hs);
            break;
          case Shader::HostVertexShaderType::kTriangleDomainPatchIndexed:
            hs_bytes = ::continuous_triangle_1cp_hs;
            hs_size = sizeof(::continuous_triangle_1cp_hs);
            break;
          case Shader::HostVertexShaderType::kQuadDomainCPIndexed:
            hs_bytes = ::continuous_quad_4cp_hs;
            hs_size = sizeof(::continuous_quad_4cp_hs);
            break;
          case Shader::HostVertexShaderType::kQuadDomainPatchIndexed:
            hs_bytes = ::continuous_quad_1cp_hs;
            hs_size = sizeof(::continuous_quad_1cp_hs);
            break;
          default:
            XELOGE(
                "Tessellation HS: unsupported host vertex shader type {}",
                uint32_t(primitive_processing_result.host_vertex_shader_type));
            return nullptr;
        }
        break;
      case xenos::TessellationMode::kAdaptive:
        switch (primitive_processing_result.host_vertex_shader_type) {
          case Shader::HostVertexShaderType::kTriangleDomainPatchIndexed:
            hs_bytes = ::adaptive_triangle_hs;
            hs_size = sizeof(::adaptive_triangle_hs);
            break;
          case Shader::HostVertexShaderType::kQuadDomainPatchIndexed:
            hs_bytes = ::adaptive_quad_hs;
            hs_size = sizeof(::adaptive_quad_hs);
            break;
          default:
            XELOGE(
                "Tessellation HS: unsupported host vertex shader type {}",
                uint32_t(primitive_processing_result.host_vertex_shader_type));
            return nullptr;
        }
        break;
      default:
        XELOGE("Tessellation HS: unsupported tessellation mode {}",
               uint32_t(tessellation_mode));
        return nullptr;
    }

    std::vector<uint8_t> dxbc_bytes(hs_bytes, hs_bytes + hs_size);
    std::vector<uint8_t> dxil_data;
    std::string dxil_error;
    if (!dxbc_to_dxil_converter_->Convert(dxbc_bytes, dxil_data, &dxil_error)) {
      XELOGE("Tessellation HS: DXBC to DXIL conversion failed: {}", dxil_error);
      return nullptr;
    }

    MetalShaderConversionResult hull_result;
    MetalShaderReflectionInfo hull_reflection;
    if (!metal_shader_converter_->ConvertWithStageEx(
            MetalShaderStage::kHull, dxil_data, hull_result, &hull_reflection,
            nullptr, nullptr, true,
            static_cast<int>(IRInputTopologyUndefined))) {
      XELOGE("Tessellation HS: DXIL to Metal conversion failed: {}",
             hull_result.error_message);
      return nullptr;
    }

    NS::Error* error = nullptr;
    dispatch_data_t hull_data = dispatch_data_create(
        hull_result.metallib_data.data(), hull_result.metallib_data.size(),
        nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    MTL::Library* hull_library = device_->newLibrary(hull_data, &error);
    dispatch_release(hull_data);
    if (!hull_library) {
      XELOGE("Tessellation HS: Failed to create Metal library: {}",
             error ? error->localizedDescription()->utf8String()
                   : "unknown error");
      return nullptr;
    }

    TessellationHullStageState state;
    state.library = hull_library;
    state.function_name = hull_result.function_name;
    state.reflection = hull_reflection;
    if (!state.reflection.has_hull_info) {
      XELOGE("Tessellation HS: reflection missing hull info");
    }

    auto [inserted_it, inserted] =
        tessellation_hull_stage_cache_.emplace(hull_key_hash, std::move(state));
    return &inserted_it->second;
  };

  auto get_domain_stage = [&]() -> TessellationDomainStageState* {
    uint64_t domain_key =
        XXH3_64bits(&domain_translation, sizeof(domain_translation));
    auto domain_it = tessellation_domain_stage_cache_.find(domain_key);
    if (domain_it != tessellation_domain_stage_cache_.end()) {
      return &domain_it->second;
    }

    std::vector<uint8_t> dxil_data = domain_translation->dxil_data();
    if (dxil_data.empty()) {
      std::string dxil_error;
      if (!dxbc_to_dxil_converter_->Convert(
              domain_translation->translated_binary(), dxil_data,
              &dxil_error)) {
        XELOGE("Tessellation DS: DXBC to DXIL conversion failed: {}",
               dxil_error);
        return nullptr;
      }
    }

    MetalShaderConversionResult domain_result;
    MetalShaderReflectionInfo domain_reflection;
    if (!metal_shader_converter_->ConvertWithStageEx(
            MetalShaderStage::kDomain, dxil_data, domain_result,
            &domain_reflection, nullptr, nullptr, true,
            static_cast<int>(IRInputTopologyUndefined))) {
      XELOGE("Tessellation DS: DXIL to Metal conversion failed: {}",
             domain_result.error_message);
      return nullptr;
    }

    NS::Error* error = nullptr;
    dispatch_data_t domain_data = dispatch_data_create(
        domain_result.metallib_data.data(), domain_result.metallib_data.size(),
        nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    MTL::Library* domain_library = device_->newLibrary(domain_data, &error);
    dispatch_release(domain_data);
    if (!domain_library) {
      XELOGE("Tessellation DS: Failed to create Metal library: {}",
             error ? error->localizedDescription()->utf8String()
                   : "unknown error");
      return nullptr;
    }

    TessellationDomainStageState state;
    state.library = domain_library;
    state.function_name = domain_result.function_name;
    state.reflection = domain_reflection;
    if (!state.reflection.has_domain_info) {
      XELOGE("Tessellation DS: reflection missing domain info");
    }

    auto [inserted_it, inserted] =
        tessellation_domain_stage_cache_.emplace(domain_key, std::move(state));
    return &inserted_it->second;
  };

  TessellationVertexStageState* vertex_stage = get_vertex_stage();
  if (!vertex_stage || !vertex_stage->library ||
      !vertex_stage->stage_in_library) {
    return nullptr;
  }
  TessellationHullStageState* hull_stage = get_hull_stage();
  if (!hull_stage || !hull_stage->library) {
    return nullptr;
  }
  TessellationDomainStageState* domain_stage = get_domain_stage();
  if (!domain_stage || !domain_stage->library) {
    return nullptr;
  }

  IRRuntimeTessellatorOutputPrimitive output_primitive =
      IRRuntimeTessellatorOutputUndefined;
  switch (hull_stage->reflection.hs_tessellator_output_primitive) {
    case IRRuntimeTessellatorOutputPoint:
      output_primitive = IRRuntimeTessellatorOutputPoint;
      break;
    case IRRuntimeTessellatorOutputLine:
      output_primitive = IRRuntimeTessellatorOutputLine;
      break;
    case IRRuntimeTessellatorOutputTriangleCW:
      output_primitive = IRRuntimeTessellatorOutputTriangleCW;
      break;
    case IRRuntimeTessellatorOutputTriangleCCW:
      output_primitive = IRRuntimeTessellatorOutputTriangleCCW;
      break;
    default:
      XELOGE("Tessellation pipeline: unsupported tessellator output {}",
             hull_stage->reflection.hs_tessellator_output_primitive);
      return nullptr;
  }

  IRRuntimePrimitiveType geometry_primitive = IRRuntimePrimitiveTypeTriangle;
  const char* geometry_function = kIRTrianglePassthroughGeometryShader;
  switch (output_primitive) {
    case IRRuntimeTessellatorOutputPoint:
      geometry_primitive = IRRuntimePrimitiveTypePoint;
      geometry_function = kIRPointPassthroughGeometryShader;
      break;
    case IRRuntimeTessellatorOutputLine:
      geometry_primitive = IRRuntimePrimitiveTypeLine;
      geometry_function = kIRLinePassthroughGeometryShader;
      break;
    case IRRuntimeTessellatorOutputTriangleCW:
    case IRRuntimeTessellatorOutputTriangleCCW:
      geometry_primitive = IRRuntimePrimitiveTypeTriangle;
      geometry_function = kIRTrianglePassthroughGeometryShader;
      break;
    default:
      break;
  }

  if (!IRRuntimeValidateTessellationPipeline(
          output_primitive, geometry_primitive,
          hull_stage->reflection.hs_output_control_point_size,
          domain_stage->reflection.ds_input_control_point_size,
          hull_stage->reflection.hs_patch_constants_size,
          domain_stage->reflection.ds_patch_constants_size,
          hull_stage->reflection.hs_output_control_point_count,
          domain_stage->reflection.ds_input_control_point_count)) {
    XELOGE("Tessellation pipeline: validation failed for HS/DS pairing");
    return nullptr;
  }

  MTL::MeshRenderPipelineDescriptor* desc =
      MTL::MeshRenderPipelineDescriptor::alloc()->init();
  for (uint32_t i = 0; i < 4; ++i) {
    desc->colorAttachments()->object(i)->setPixelFormat(color_formats[i]);
  }
  desc->setDepthAttachmentPixelFormat(depth_format);
  desc->setStencilAttachmentPixelFormat(stencil_format);
  desc->setRasterSampleCount(sample_count);
  desc->setAlphaToCoverageEnabled(key_data.alpha_to_mask_enable != 0);

  for (uint32_t i = 0; i < 4; ++i) {
    auto* color_attachment = desc->colorAttachments()->object(i);
    if (color_formats[i] == MTL::PixelFormatInvalid) {
      color_attachment->setWriteMask(MTL::ColorWriteMaskNone);
      color_attachment->setBlendingEnabled(false);
      continue;
    }
    uint32_t rt_write_mask = (key_data.normalized_color_mask >> (i * 4)) & 0xF;
    color_attachment->setWriteMask(ToMetalColorWriteMask(rt_write_mask));
    if (!rt_write_mask) {
      color_attachment->setBlendingEnabled(false);
      continue;
    }

    auto blendcontrol = regs.Get<reg::RB_BLENDCONTROL>(
        reg::RB_BLENDCONTROL::rt_register_indices[i]);
    MTL::BlendFactor src_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_srcblend);
    MTL::BlendFactor dst_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_destblend);
    MTL::BlendOperation op_rgb =
        ToMetalBlendOperation(blendcontrol.color_comb_fcn);
    MTL::BlendFactor src_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_srcblend);
    MTL::BlendFactor dst_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_destblend);
    MTL::BlendOperation op_alpha =
        ToMetalBlendOperation(blendcontrol.alpha_comb_fcn);

    bool blending_enabled =
        src_rgb != MTL::BlendFactorOne || dst_rgb != MTL::BlendFactorZero ||
        op_rgb != MTL::BlendOperationAdd || src_alpha != MTL::BlendFactorOne ||
        dst_alpha != MTL::BlendFactorZero || op_alpha != MTL::BlendOperationAdd;
    color_attachment->setBlendingEnabled(blending_enabled);
    if (blending_enabled) {
      color_attachment->setSourceRGBBlendFactor(src_rgb);
      color_attachment->setDestinationRGBBlendFactor(dst_rgb);
      color_attachment->setRgbBlendOperation(op_rgb);
      color_attachment->setSourceAlphaBlendFactor(src_alpha);
      color_attachment->setDestinationAlphaBlendFactor(dst_alpha);
      color_attachment->setAlphaBlendOperation(op_alpha);
    }
  }
  IRGeometryTessellationEmulationPipelineDescriptor ir_desc = {};
  ir_desc.stageInLibrary = vertex_stage->stage_in_library;
  ir_desc.vertexLibrary = vertex_stage->library;
  ir_desc.vertexFunctionName = vertex_stage->function_name.c_str();
  ir_desc.hullLibrary = hull_stage->library;
  ir_desc.hullFunctionName = hull_stage->function_name.c_str();
  ir_desc.domainLibrary = domain_stage->library;
  ir_desc.domainFunctionName = domain_stage->function_name.c_str();
  ir_desc.geometryLibrary = nullptr;
  ir_desc.geometryFunctionName = geometry_function;
  ir_desc.fragmentLibrary = pixel_library;
  ir_desc.fragmentFunctionName = pixel_function;
  ir_desc.basePipelineDescriptor = desc;
  ir_desc.pipelineConfig.outputPrimitiveType = output_primitive;
  ir_desc.pipelineConfig.vsOutputSizeInBytes =
      vertex_stage->vertex_output_size_in_bytes;
  ir_desc.pipelineConfig.gsMaxInputPrimitivesPerMeshThreadgroup =
      domain_stage->reflection.ds_max_input_prims_per_mesh_threadgroup;
  ir_desc.pipelineConfig.hsMaxPatchesPerObjectThreadgroup =
      hull_stage->reflection.hs_max_patches_per_object_threadgroup;
  ir_desc.pipelineConfig.hsInputControlPointCount =
      hull_stage->reflection.hs_input_control_point_count;
  ir_desc.pipelineConfig.hsMaxObjectThreadsPerThreadgroup =
      hull_stage->reflection.hs_max_object_threads_per_patch;
  ir_desc.pipelineConfig.hsMaxTessellationFactor =
      hull_stage->reflection.hs_max_tessellation_factor;
  ir_desc.pipelineConfig.gsInstanceCount = 1;

  if (!ir_desc.pipelineConfig.vsOutputSizeInBytes ||
      !ir_desc.pipelineConfig.gsMaxInputPrimitivesPerMeshThreadgroup ||
      !ir_desc.pipelineConfig.hsMaxPatchesPerObjectThreadgroup ||
      !ir_desc.pipelineConfig.hsInputControlPointCount ||
      !ir_desc.pipelineConfig.hsMaxObjectThreadsPerThreadgroup) {
    XELOGE(
        "Tessellation pipeline: invalid reflection values (vs_output={}, "
        "gs_max_input={}, hs_patches={}, hs_cp_count={}, hs_threads={})",
        ir_desc.pipelineConfig.vsOutputSizeInBytes,
        ir_desc.pipelineConfig.gsMaxInputPrimitivesPerMeshThreadgroup,
        ir_desc.pipelineConfig.hsMaxPatchesPerObjectThreadgroup,
        ir_desc.pipelineConfig.hsInputControlPointCount,
        ir_desc.pipelineConfig.hsMaxObjectThreadsPerThreadgroup);
    desc->release();
    return nullptr;
  }

  NS::Error* error = nullptr;
  MTL::RenderPipelineState* pipeline =
      IRRuntimeNewGeometryTessellationEmulationPipeline(device_, &ir_desc,
                                                        &error);
  desc->release();
  if (!pipeline) {
    XELOGE(
        "Failed to create tessellation pipeline state: {}",
        error ? error->localizedDescription()->utf8String() : "unknown error");
    return nullptr;
  }

  TessellationPipelineState state;
  state.pipeline = pipeline;
  state.config = ir_desc.pipelineConfig;
  state.primitive = geometry_primitive;

  auto [inserted_it, inserted] =
      tessellation_pipeline_cache_.emplace(key, std::move(state));
  return &inserted_it->second;
}

bool MetalCommandProcessor::EnsureDepthOnlyPixelShader() {
  if (depth_only_pixel_library_) {
    return true;
  }
  if (!shader_translator_ || !dxbc_to_dxil_converter_ ||
      !metal_shader_converter_) {
    XELOGE("Depth-only PS: shader translation not initialized");
    return false;
  }

  std::vector<uint8_t> dxbc_data =
      shader_translator_->CreateDepthOnlyPixelShader();
  if (dxbc_data.empty()) {
    XELOGE("Depth-only PS: failed to create DXBC");
    return false;
  }

  std::vector<uint8_t> dxil_data;
  std::string dxil_error;
  if (!dxbc_to_dxil_converter_->Convert(dxbc_data, dxil_data, &dxil_error)) {
    XELOGE("Depth-only PS: DXBC to DXIL conversion failed: {}", dxil_error);
    return false;
  }

  MetalShaderConversionResult result;
  if (!metal_shader_converter_->ConvertWithStage(MetalShaderStage::kFragment,
                                                 dxil_data, result)) {
    XELOGE("Depth-only PS: DXIL to Metal conversion failed: {}",
           result.error_message);
    return false;
  }

  NS::Error* error = nullptr;
  dispatch_data_t lib_data = dispatch_data_create(
      result.metallib_data.data(), result.metallib_data.size(), nullptr,
      DISPATCH_DATA_DESTRUCTOR_DEFAULT);
  depth_only_pixel_library_ = device_->newLibrary(lib_data, &error);
  dispatch_release(lib_data);
  if (!depth_only_pixel_library_) {
    XELOGE("Depth-only PS: Failed to create Metal library: {}",
           error ? error->localizedDescription()->utf8String() : "unknown");
    return false;
  }
  depth_only_pixel_function_name_ = result.function_name;
  if (depth_only_pixel_function_name_.empty()) {
    XELOGE("Depth-only PS: missing function name");
    return false;
  }
  return true;
}

bool MetalCommandProcessor::CreateIRConverterBuffers() {
  // Buffer creation is now done inline in SetupContext
  // This function exists for header compatibility
  return res_heap_ab_ && smp_heap_ab_ && uniforms_buffer_;
}

uint64_t MetalCommandProcessor::GetDrawRingAllocationSizeBytes() const {
  const uint64_t descriptor_table_count =
      uint64_t(kStageCount) * uint64_t(std::max<size_t>(1, draw_ring_count_));
  const uint64_t resource_heap_slots =
      uint64_t(kResourceHeapSlotsPerTable) * descriptor_table_count;
  const uint64_t resource_heap_slots_total = resource_heap_slots * 2;
  const uint64_t resource_heap_bytes =
      resource_heap_slots_total * uint64_t(sizeof(IRDescriptorTableEntry));
  const uint64_t sampler_heap_slots =
      uint64_t(kSamplerHeapSlotsPerTable) * descriptor_table_count;
  const uint64_t sampler_heap_bytes =
      sampler_heap_slots * uint64_t(sizeof(IRDescriptorTableEntry));
  const uint64_t uniforms_buffer_size =
      uint64_t(kUniformsBytesPerTable) * descriptor_table_count;
  const uint64_t top_level_ab_total_bytes =
      uint64_t(kTopLevelABBytesPerTable) * descriptor_table_count;
  const uint64_t draw_args_size = 64;
  const uint64_t cbv_heap_slots =
      uint64_t(kCbvHeapSlotsPerTable) * descriptor_table_count;
  const uint64_t cbv_heap_bytes =
      cbv_heap_slots * uint64_t(sizeof(IRDescriptorTableEntry));
  return resource_heap_bytes + sampler_heap_bytes + uniforms_buffer_size +
         top_level_ab_total_bytes + draw_args_size + cbv_heap_bytes;
}

std::shared_ptr<MetalCommandProcessor::DrawRingBuffers>
MetalCommandProcessor::CreateDrawRingBuffers() {
  if (!device_) {
    XELOGE("CreateDrawRingBuffers: Metal device is null");
    return nullptr;
  }
  if (!null_buffer_ || !null_texture_ || !null_sampler_) {
    XELOGE("CreateDrawRingBuffers: Null resources not initialized");
    return nullptr;
  }

  static uint32_t ring_id = 0;
  auto ring = std::make_shared<DrawRingBuffers>();

  const size_t kDescriptorTableCount = kStageCount * draw_ring_count_;
  const size_t kResourceHeapSlots =
      kResourceHeapSlotsPerTable * kDescriptorTableCount;
  const size_t kUavTableBaseIndex = kResourceHeapSlots;
  const size_t kResourceHeapSlotsTotal = kResourceHeapSlots * 2;
  const size_t kResourceHeapBytes =
      kResourceHeapSlotsTotal * sizeof(IRDescriptorTableEntry);
  const size_t kSamplerHeapSlots =
      kSamplerHeapSlotsPerTable * kDescriptorTableCount;
  const size_t kSamplerHeapBytes =
      kSamplerHeapSlots * sizeof(IRDescriptorTableEntry);
  const size_t kUniformsBufferSize =
      kUniformsBytesPerTable * kDescriptorTableCount;
  const size_t kTopLevelABTotalBytes =
      kTopLevelABBytesPerTable * kDescriptorTableCount;
  const size_t kDrawArgsSize = 64;  // Enough for draw arguments struct
  const size_t kCBVHeapSlots = kCbvHeapSlotsPerTable * kDescriptorTableCount;
  const size_t kCBVHeapBytes = kCBVHeapSlots * sizeof(IRDescriptorTableEntry);
  ring->total_bytes = GetDrawRingAllocationSizeBytes();

  ring->res_heap_ab =
      device_->newBuffer(kResourceHeapBytes, MTL::ResourceStorageModeShared);
  if (!ring->res_heap_ab) {
    XELOGE("Failed to create resource descriptor heap buffer");
    return nullptr;
  }
  std::string ring_label_suffix = std::to_string(ring_id);
  ring->res_heap_ab->setLabel(NS::String::string(
      ("ResourceDescriptorHeap_" + ring_label_suffix).c_str(),
      NS::UTF8StringEncoding));

  // Initialize all tables:
  // - Slot 0: null buffer (will be replaced with shared memory per draw).
  // - Slots 1+: null texture (safe default for any accidental access).
  auto* res_entries =
      reinterpret_cast<IRDescriptorTableEntry*>(ring->res_heap_ab->contents());
  auto* uav_entries = res_entries + kUavTableBaseIndex;
  for (size_t table = 0; table < kDescriptorTableCount; ++table) {
    IRDescriptorTableEntry* table_entries =
        res_entries + table * kResourceHeapSlotsPerTable;
    IRDescriptorTableSetBuffer(&table_entries[0], null_buffer_->gpuAddress(),
                               kNullBufferSize);
    for (size_t i = 1; i < kResourceHeapSlotsPerTable; ++i) {
      IRDescriptorTableSetTexture(&table_entries[i], null_texture_, 0.0f, 0);
    }
    IRDescriptorTableEntry* uav_table_entries =
        uav_entries + table * kResourceHeapSlotsPerTable;
    for (size_t i = 0; i < kResourceHeapSlotsPerTable; ++i) {
      IRDescriptorTableSetBuffer(&uav_table_entries[i],
                                 null_buffer_->gpuAddress(), kNullBufferSize);
    }
  }

  ring->smp_heap_ab =
      device_->newBuffer(kSamplerHeapBytes, MTL::ResourceStorageModeShared);
  if (!ring->smp_heap_ab) {
    XELOGE("Failed to create sampler descriptor heap buffer");
    return nullptr;
  }
  ring->smp_heap_ab->setLabel(
      NS::String::string(("SamplerDescriptorHeap_" + ring_label_suffix).c_str(),
                         NS::UTF8StringEncoding));
  auto* smp_entries =
      reinterpret_cast<IRDescriptorTableEntry*>(ring->smp_heap_ab->contents());
  for (size_t i = 0; i < kSamplerHeapSlots; ++i) {
    IRDescriptorTableSetSampler(&smp_entries[i], null_sampler_, 0.0f);
  }

  ring->uniforms_buffer =
      device_->newBuffer(kUniformsBufferSize, MTL::ResourceStorageModeShared);
  if (!ring->uniforms_buffer) {
    XELOGE("Failed to create uniforms buffer");
    return nullptr;
  }
  ring->uniforms_buffer->setLabel(NS::String::string(
      ("UniformsBuffer_" + ring_label_suffix).c_str(), NS::UTF8StringEncoding));
  std::memset(ring->uniforms_buffer->contents(), 0, kUniformsBufferSize);

  ring->top_level_ab =
      device_->newBuffer(kTopLevelABTotalBytes, MTL::ResourceStorageModeShared);
  if (!ring->top_level_ab) {
    XELOGE("Failed to create top-level argument buffer");
    return nullptr;
  }
  ring->top_level_ab->setLabel(NS::String::string(
      ("TopLevelArgumentBuffer_" + ring_label_suffix).c_str(),
      NS::UTF8StringEncoding));
  std::memset(ring->top_level_ab->contents(), 0, kTopLevelABTotalBytes);

  ring->draw_args_buffer =
      device_->newBuffer(kDrawArgsSize, MTL::ResourceStorageModeShared);
  if (!ring->draw_args_buffer) {
    XELOGE("Failed to create draw arguments buffer");
    return nullptr;
  }
  ring->draw_args_buffer->setLabel(
      NS::String::string(("DrawArgumentsBuffer_" + ring_label_suffix).c_str(),
                         NS::UTF8StringEncoding));
  std::memset(ring->draw_args_buffer->contents(), 0, kDrawArgsSize);

  ring->cbv_heap_ab =
      device_->newBuffer(kCBVHeapBytes, MTL::ResourceStorageModeShared);
  if (!ring->cbv_heap_ab) {
    XELOGE("Failed to create CBV descriptor heap buffer");
    return nullptr;
  }
  ring->cbv_heap_ab->setLabel(
      NS::String::string(("CBVDescriptorHeap_" + ring_label_suffix).c_str(),
                         NS::UTF8StringEncoding));
  std::memset(ring->cbv_heap_ab->contents(), 0, kCBVHeapBytes);

  ++ring_id;

  return ring;
}

std::shared_ptr<MetalCommandProcessor::DrawRingBuffers>
MetalCommandProcessor::AcquireDrawRingBuffers() {
  const uint64_t ring_bytes = GetDrawRingAllocationSizeBytes();
  if (!ring_bytes) {
    return nullptr;
  }

  for (;;) {
    bool reserve_allocation = false;
    {
      std::unique_lock<std::mutex> lock(draw_ring_mutex_);
      if (!draw_ring_pool_.empty()) {
        auto ring = draw_ring_pool_.back();
        draw_ring_pool_.pop_back();
        ++draw_ring_pool_in_use_count_;
        return ring;
      }
      if (draw_ring_pool_total_bytes_ + ring_bytes <=
          draw_ring_pool_max_bytes_) {
        draw_ring_pool_total_bytes_ += ring_bytes;
        reserve_allocation = true;
      } else {
        ++draw_ring_pool_wait_count_;
        if (!draw_ring_pool_cv_.wait_for(
                lock, std::chrono::milliseconds(kDrawRingPoolWaitTimeoutMs),
                [&]() { return !draw_ring_pool_.empty(); })) {
          ++draw_ring_pool_timeout_count_;
          if (ShouldLogRateLimited(draw_ring_pool_last_log_ns_,
                                   kDrawRingPoolLogIntervalNs)) {
            XELOGW(
                "Metal draw-ring pool timed out (limit={} MiB, total={} MiB, "
                "in_use={}, waits={}, timeouts={})",
                draw_ring_pool_max_bytes_ / (1024ull * 1024ull),
                draw_ring_pool_total_bytes_ / (1024ull * 1024ull),
                draw_ring_pool_in_use_count_, draw_ring_pool_wait_count_,
                draw_ring_pool_timeout_count_);
          }
          return nullptr;
        }
      }
    }

    if (!reserve_allocation) {
      continue;
    }
    auto ring = CreateDrawRingBuffers();
    if (!ring) {
      std::lock_guard<std::mutex> lock(draw_ring_mutex_);
      draw_ring_pool_total_bytes_ -= ring_bytes;
      draw_ring_pool_cv_.notify_all();
      return nullptr;
    }
    if (!ring->total_bytes) {
      ring->total_bytes = ring_bytes;
    }
    {
      std::lock_guard<std::mutex> lock(draw_ring_mutex_);
      ++draw_ring_pool_in_use_count_;
    }
    return ring;
  }
}

void MetalCommandProcessor::SetActiveDrawRing(
    const std::shared_ptr<DrawRingBuffers>& ring) {
  active_draw_ring_ = ring;
  res_heap_ab_ = ring ? ring->res_heap_ab : nullptr;
  smp_heap_ab_ = ring ? ring->smp_heap_ab : nullptr;
  cbv_heap_ab_ = ring ? ring->cbv_heap_ab : nullptr;
  uniforms_buffer_ = ring ? ring->uniforms_buffer : nullptr;
  top_level_ab_ = ring ? ring->top_level_ab : nullptr;
  draw_args_buffer_ = ring ? ring->draw_args_buffer : nullptr;
}

void MetalCommandProcessor::EnsureActiveDrawRing() {
  if (!active_draw_ring_) {
    auto ring = AcquireDrawRingBuffers();
    if (!ring) {
      return;
    }
    SetActiveDrawRing(ring);
  }
  if (command_buffer_draw_rings_.empty()) {
    command_buffer_draw_rings_.push_back(active_draw_ring_);
    current_draw_index_ = 0;
  }
}

void MetalCommandProcessor::FlushPendingDrawRingReleasesLocked() {
  for (auto& [command_buffer, rings] : pending_draw_ring_releases_) {
    if (draw_ring_pool_in_use_count_ >= rings.size()) {
      draw_ring_pool_in_use_count_ -= uint32_t(rings.size());
    } else {
      draw_ring_pool_in_use_count_ = 0;
    }
    for (auto& ring : rings) {
      draw_ring_pool_.push_back(std::move(ring));
    }
  }
  pending_draw_ring_releases_.clear();
}

void MetalCommandProcessor::ScheduleDrawRingRelease(
    MTL::CommandBuffer* command_buffer) {
  if (!command_buffer || command_buffer_draw_rings_.empty()) {
    return;
  }
  std::vector<std::shared_ptr<DrawRingBuffers>> rings;
  rings.swap(command_buffer_draw_rings_);
  bool add_handler = false;
  {
    std::lock_guard<std::mutex> lock(draw_ring_mutex_);
    auto& pending = pending_draw_ring_releases_[command_buffer];
    add_handler = pending.empty();
    pending.reserve(pending.size() + rings.size());
    for (auto& ring : rings) {
      pending.push_back(std::move(ring));
    }
  }
  if (add_handler) {
    pending_completion_handlers_.fetch_add(1, std::memory_order_relaxed);
    command_buffer->addCompletedHandler(
        [this](MTL::CommandBuffer* completed_cmd) {
          bool notify_waiters = false;
          {
            std::lock_guard<std::mutex> lock(draw_ring_mutex_);
            auto it = pending_draw_ring_releases_.find(completed_cmd);
            if (it != pending_draw_ring_releases_.end()) {
              if (draw_ring_pool_in_use_count_ >= it->second.size()) {
                draw_ring_pool_in_use_count_ -= uint32_t(it->second.size());
              } else {
                draw_ring_pool_in_use_count_ = 0;
              }
              for (auto& ring : it->second) {
                draw_ring_pool_.push_back(std::move(ring));
              }
              pending_draw_ring_releases_.erase(it);
              notify_waiters = true;
            }
          }
          if (notify_waiters) {
            draw_ring_pool_cv_.notify_all();
          }
          pending_completion_handlers_.fetch_sub(1, std::memory_order_relaxed);
        });
  }
}

void MetalCommandProcessor::PopulateIRConverterBuffers() {
  if (!res_heap_ab_ || !smp_heap_ab_ || !uniforms_buffer_ || !shared_memory_) {
    return;
  }

  // Get shared memory buffer for vertex data fetching
  MTL::Buffer* shared_mem_buffer = shared_memory_->GetBuffer();
  if (!shared_mem_buffer) {
    XELOGW("PopulateIRConverterBuffers: No shared memory buffer available");
    return;
  }

  // Populate resource descriptor heap slot 0 with shared memory buffer
  // Xbox 360 shaders use vfetch instructions that read from this buffer
  IRDescriptorTableEntry* res_heap =
      static_cast<IRDescriptorTableEntry*>(res_heap_ab_->contents());

  // Set slot 0 (t0) to shared memory for vertex buffer fetching
  // The metadata encodes buffer size for bounds checking
  uint64_t shared_mem_gpu_addr = shared_mem_buffer->gpuAddress();
  uint64_t shared_mem_size = shared_mem_buffer->length();

  // Use IRDescriptorTableSetBuffer to properly encode the descriptor
  // metadata = buffer size in low 32 bits for bounds checking
  IRDescriptorTableSetBuffer(&res_heap[0], shared_mem_gpu_addr,
                             shared_mem_size);

  // Populate uniforms buffer with system constants and fetch constants
  // Layout matches DxbcShaderTranslator::SystemConstants (b0)
  uint8_t* uniforms = static_cast<uint8_t*>(uniforms_buffer_->contents());

  // For now, populate minimal system constants for passthrough rendering
  // This structure needs to match what the translated shaders expect
  struct MinimalSystemConstants {
    uint32_t flags;                      // 0x00
    float tessellation_factor_range[2];  // 0x04
    uint32_t line_loop_closing_index;    // 0x0C

    uint32_t vertex_index_endian;  // 0x10
    uint32_t vertex_index_offset;  // 0x14
    uint32_t vertex_index_min;     // 0x18
    uint32_t vertex_index_max;     // 0x1C

    float user_clip_planes[6][4];  // 0x20 - 6 clip planes * 4 floats = 96 bytes

    float ndc_scale[3];               // 0x80
    float point_vertex_diameter_min;  // 0x8C

    float ndc_offset[3];              // 0x90
    float point_vertex_diameter_max;  // 0x9C
  };

  MinimalSystemConstants* sys_const =
      reinterpret_cast<MinimalSystemConstants*>(uniforms);

  // Initialize to zero
  std::memset(sys_const, 0, sizeof(MinimalSystemConstants));

  // Set passthrough NDC transform (identity)
  sys_const->ndc_scale[0] = 1.0f;
  sys_const->ndc_scale[1] = 1.0f;
  sys_const->ndc_scale[2] = 1.0f;
  sys_const->ndc_offset[0] = 0.0f;
  sys_const->ndc_offset[1] = 0.0f;
  sys_const->ndc_offset[2] = 0.0f;

  // Set reasonable vertex index bounds
  sys_const->vertex_index_min = 0;
  sys_const->vertex_index_max = 0xFFFFFFFF;

  // Copy fetch constants from register file (b3 in DXBC)
  // Fetch constants start at register XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0
  // There are 32 fetch constants, each 6 DWORDs = 192 DWORDs total = 768 bytes
  const uint32_t* regs = register_file_->values;
  const size_t kFetchConstantOffset = 512;    // After system constants (b0)
  const size_t kFetchConstantCount = 32 * 6;  // 32 fetch constants * 6 DWORDs

  std::memcpy(uniforms + kFetchConstantOffset,
              &regs[XE_GPU_REG_SHADER_CONSTANT_FETCH_00_0],
              kFetchConstantCount * sizeof(uint32_t));

  // Bind IR Converter runtime buffers to render encoder
  if (current_render_encoder_) {
    // Bind resource descriptor heap at index 0 (kIRDescriptorHeapBindPoint)
    current_render_encoder_->setVertexBuffer(res_heap_ab_, 0,
                                             kIRDescriptorHeapBindPoint);
    current_render_encoder_->setFragmentBuffer(res_heap_ab_, 0,
                                               kIRDescriptorHeapBindPoint);

    // Bind sampler heap at index 1 (kIRSamplerHeapBindPoint)
    current_render_encoder_->setVertexBuffer(smp_heap_ab_, 0,
                                             kIRSamplerHeapBindPoint);
    current_render_encoder_->setFragmentBuffer(smp_heap_ab_, 0,
                                               kIRSamplerHeapBindPoint);

    // Bind uniforms at index 5 (kIRArgumentBufferUniformsBindPoint)
    current_render_encoder_->setVertexBuffer(
        uniforms_buffer_, 0, kIRArgumentBufferUniformsBindPoint);
    current_render_encoder_->setFragmentBuffer(
        uniforms_buffer_, 0, kIRArgumentBufferUniformsBindPoint);

    // Make shared memory resident for GPU access
    UseRenderEncoderResource(shared_mem_buffer, MTL::ResourceUsageRead);
  }
}

DxbcShaderTranslator::Modification
MetalCommandProcessor::GetCurrentVertexShaderModification(
    const Shader& shader, Shader::HostVertexShaderType host_vertex_shader_type,
    uint32_t interpolator_mask) const {
  const auto& regs = *register_file_;

  DxbcShaderTranslator::Modification modification(
      shader_translator_->GetDefaultVertexShaderModification(
          shader.GetDynamicAddressableRegisterCount(
              regs.Get<reg::SQ_PROGRAM_CNTL>().vs_num_reg),
          host_vertex_shader_type));

  modification.vertex.interpolator_mask = interpolator_mask;

  auto pa_cl_clip_cntl = regs.Get<reg::PA_CL_CLIP_CNTL>();
  uint32_t user_clip_planes =
      pa_cl_clip_cntl.clip_disable ? 0 : pa_cl_clip_cntl.ucp_ena;
  modification.vertex.user_clip_plane_count = xe::bit_count(user_clip_planes);
  modification.vertex.user_clip_plane_cull =
      uint32_t(user_clip_planes && pa_cl_clip_cntl.ucp_cull_only_ena);
  modification.vertex.vertex_kill_and =
      uint32_t((shader.writes_point_size_edge_flag_kill_vertex() & 0b100) &&
               !pa_cl_clip_cntl.vtx_kill_or);

  modification.vertex.output_point_size =
      uint32_t((shader.writes_point_size_edge_flag_kill_vertex() & 0b001) &&
               regs.Get<reg::VGT_DRAW_INITIATOR>().prim_type ==
                   xenos::PrimitiveType::kPointList);

  return modification;
}

DxbcShaderTranslator::Modification
MetalCommandProcessor::GetCurrentPixelShaderModification(
    const Shader& shader, uint32_t interpolator_mask, uint32_t param_gen_pos,
    reg::RB_DEPTHCONTROL normalized_depth_control) const {
  const auto& regs = *register_file_;

  DxbcShaderTranslator::Modification modification(
      shader_translator_->GetDefaultPixelShaderModification(
          shader.GetDynamicAddressableRegisterCount(
              regs.Get<reg::SQ_PROGRAM_CNTL>().ps_num_reg)));

  modification.pixel.interpolator_mask = interpolator_mask;
  modification.pixel.interpolators_centroid =
      interpolator_mask &
      ~xenos::GetInterpolatorSamplingPattern(
          regs.Get<reg::RB_SURFACE_INFO>().msaa_samples,
          regs.Get<reg::SQ_CONTEXT_MISC>().sc_sample_cntl,
          regs.Get<reg::SQ_INTERPOLATOR_CNTL>().sampling_pattern);

  if (param_gen_pos < xenos::kMaxInterpolators) {
    modification.pixel.param_gen_enable = 1;
    modification.pixel.param_gen_interpolator = param_gen_pos;
    modification.pixel.param_gen_point =
        uint32_t(regs.Get<reg::VGT_DRAW_INITIATOR>().prim_type ==
                 xenos::PrimitiveType::kPointList);
  } else {
    modification.pixel.param_gen_enable = 0;
    modification.pixel.param_gen_interpolator = 0;
    modification.pixel.param_gen_point = 0;
  }

  using DepthStencilMode = DxbcShaderTranslator::Modification::DepthStencilMode;
  if (shader.implicit_early_z_write_allowed() &&
      (!shader.writes_color_target(0) ||
       !draw_util::DoesCoverageDependOnAlpha(
           regs.Get<reg::RB_COLORCONTROL>()))) {
    modification.pixel.depth_stencil_mode = DepthStencilMode::kEarlyHint;
  } else {
    modification.pixel.depth_stencil_mode = DepthStencilMode::kNoModifiers;
  }

  // Initialize MIN/MAX blend pre-multiply factors to kOne (no pre-multiply).
  // These must be explicitly set, as zero-initialized bits would be kZero
  // which causes the shader to multiply output color by zero (black).
  modification.pixel.rt0_blend_rgb_factor_for_premult =
      xenos::BlendFactor::kOne;
  modification.pixel.rt0_blend_a_factor_for_premult = xenos::BlendFactor::kOne;

  return modification;
}
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

// ==========================================================================
// SPIRV-Cross tessellation support.
// ==========================================================================

// Tessellation factor compute kernels are defined in msl_tess_factor_kernels.h.
#include "xenia/gpu/metal/msl_tess_factor_kernels.h"

bool MetalCommandProcessor::InitializeMslTessellation() {
  if (!device_) return false;

  auto compile_kernel =
      [&](const char* source,
          const char* function_name) -> MTL::ComputePipelineState* {
    NS::Error* error = nullptr;
    auto* src = NS::String::string(source, NS::UTF8StringEncoding);
    auto* opts = MTL::CompileOptions::alloc()->init();
    opts->setFastMathEnabled(true);
    MTL::Library* lib = device_->newLibrary(src, opts, &error);
    opts->release();
    if (!lib) {
      if (error) {
        XELOGE("Tessellation kernel compile error: {}",
               error->localizedDescription()->utf8String());
      }
      return nullptr;
    }
    auto* fn_name = NS::String::string(function_name, NS::UTF8StringEncoding);
    MTL::Function* fn = lib->newFunction(fn_name);
    lib->release();
    if (!fn) {
      XELOGE("Tessellation kernel: function '{}' not found", function_name);
      return nullptr;
    }
    MTL::ComputePipelineState* pso =
        device_->newComputePipelineState(fn, &error);
    fn->release();
    if (!pso && error) {
      XELOGE("Tessellation kernel PSO error: {}",
             error->localizedDescription()->utf8String());
    }
    return pso;
  };

  // Uniform factor kernels (discrete / continuous modes).
  tess_factor_pipeline_tri_ =
      compile_kernel(kMslTessFactorUniformTriangle, "tess_factor_triangle");
  tess_factor_pipeline_quad_ =
      compile_kernel(kMslTessFactorUniformQuad, "tess_factor_quad");

  if (!tess_factor_pipeline_tri_ || !tess_factor_pipeline_quad_) {
    XELOGW(
        "SPIRV-Cross: Failed to create uniform tessellation factor "
        "pipelines");
    return false;
  }

  // Adaptive factor kernels (per-edge factors from shared memory).
  tess_factor_pipeline_adaptive_tri_ = compile_kernel(
      kMslTessFactorAdaptiveTriangle, "tess_factor_adaptive_triangle");
  tess_factor_pipeline_adaptive_quad_ =
      compile_kernel(kMslTessFactorAdaptiveQuad, "tess_factor_adaptive_quad");

  if (!tess_factor_pipeline_adaptive_tri_ ||
      !tess_factor_pipeline_adaptive_quad_) {
    XELOGW(
        "SPIRV-Cross: Failed to create adaptive tessellation factor "
        "pipelines (adaptive tessellation will fall back to uniform)");
    // Non-fatal — adaptive tessellation will degrade to uniform factors.
  }

  XELOGI("SPIRV-Cross: Tessellation factor pipelines initialized");
  return true;
}

void MetalCommandProcessor::ShutdownMslTessellation() {
  for (auto& [key, pso] : msl_tess_pipeline_cache_) {
    if (pso) pso->release();
  }
  msl_tess_pipeline_cache_.clear();
  if (tess_factor_buffer_) {
    tess_factor_buffer_->release();
    tess_factor_buffer_ = nullptr;
    tess_factor_buffer_patch_capacity_ = 0;
  }
  if (tess_factor_pipeline_tri_) {
    tess_factor_pipeline_tri_->release();
    tess_factor_pipeline_tri_ = nullptr;
  }
  if (tess_factor_pipeline_quad_) {
    tess_factor_pipeline_quad_->release();
    tess_factor_pipeline_quad_ = nullptr;
  }
  if (tess_factor_pipeline_adaptive_tri_) {
    tess_factor_pipeline_adaptive_tri_->release();
    tess_factor_pipeline_adaptive_tri_ = nullptr;
  }
  if (tess_factor_pipeline_adaptive_quad_) {
    tess_factor_pipeline_adaptive_quad_->release();
    tess_factor_pipeline_adaptive_quad_ = nullptr;
  }
}

bool MetalCommandProcessor::EnsureTessFactorBuffer(uint32_t patch_count) {
  // MTLQuadTessellationFactorsHalf is the larger of the two (12 bytes vs 8).
  constexpr size_t kMaxFactorSize = 12;
  size_t needed = size_t(patch_count) * kMaxFactorSize;
  if (tess_factor_buffer_ && tess_factor_buffer_->length() >= needed) {
    // Buffer already large enough; don't reduce the tracked capacity.
    return true;
  }
  if (tess_factor_buffer_) {
    tess_factor_buffer_->release();
  }
  // Round up and over-allocate for future growth.
  size_t alloc_size = std::max(needed, size_t(4096));
  alloc_size = (alloc_size + 4095) & ~size_t(4095);
  // Use Shared storage so the CPU can fill tessellation factors directly
  // (avoids needing a separate compute encoder for uniform factors).
  tess_factor_buffer_ =
      device_->newBuffer(alloc_size, MTL::ResourceStorageModeShared);
  if (!tess_factor_buffer_) {
    XELOGE("Failed to allocate tessellation factor buffer ({} bytes)",
           alloc_size);
    return false;
  }
  tess_factor_buffer_->setLabel(
      NS::String::string("Xenia Tess Factor Buffer", NS::UTF8StringEncoding));
  tess_factor_buffer_patch_capacity_ = uint32_t(alloc_size / kMaxFactorSize);
  return true;
}

MTL::RenderPipelineState*
MetalCommandProcessor::GetOrCreateMslTessPipelineState(
    MslShader::MslTranslation* domain_translation,
    MslShader::MslTranslation* pixel_translation,
    Shader::HostVertexShaderType host_vertex_shader_type,
    const RegisterFile& regs) {
  if (!domain_translation || !domain_translation->metal_function()) {
    XELOGE("SPIRV-Cross tess: No domain shader function");
    return nullptr;
  }

  // Determine attachment formats from render target cache (same pattern as
  // GetOrCreateMslPipelineState).
  uint32_t sample_count = 1;
  MTL::PixelFormat color_formats[4] = {
      MTL::PixelFormatInvalid, MTL::PixelFormatInvalid, MTL::PixelFormatInvalid,
      MTL::PixelFormatInvalid};
  MTL::PixelFormat depth_format = MTL::PixelFormatInvalid;
  MTL::PixelFormat stencil_format = MTL::PixelFormatInvalid;
  if (render_target_cache_) {
    for (uint32_t i = 0; i < 4; ++i) {
      if (MTL::Texture* rt = render_target_cache_->GetColorTargetForDraw(i)) {
        color_formats[i] = rt->pixelFormat();
        if (rt->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(rt->sampleCount()));
        }
      }
    }
    if (color_formats[0] == MTL::PixelFormatInvalid) {
      if (MTL::Texture* dummy =
              render_target_cache_->GetDummyColorTargetForDraw()) {
        color_formats[0] = dummy->pixelFormat();
        if (dummy->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(dummy->sampleCount()));
        }
      }
    }
    if (MTL::Texture* depth_tex =
            render_target_cache_->GetDepthTargetForDraw()) {
      depth_format = depth_tex->pixelFormat();
      switch (depth_format) {
        case MTL::PixelFormatDepth32Float_Stencil8:
        case MTL::PixelFormatDepth24Unorm_Stencil8:
        case MTL::PixelFormatX32_Stencil8:
          stencil_format = depth_format;
          break;
        default:
          stencil_format = MTL::PixelFormatInvalid;
          break;
      }
      if (depth_tex->sampleCount() > 0) {
        sample_count = std::max<uint32_t>(
            sample_count, static_cast<uint32_t>(depth_tex->sampleCount()));
      }
    }
  }

  // Keep tessellation PSO attachment formats/sample count in sync with the
  // active render pass descriptor to satisfy Metal validation.
  MTL::RenderPassDescriptor* pass_descriptor = current_render_pass_descriptor_;
  if (!pass_descriptor) {
    pass_descriptor = render_pass_descriptor_;
  }
  if (pass_descriptor) {
    sample_count = 1;
    for (uint32_t i = 0; i < 4; ++i) {
      color_formats[i] = MTL::PixelFormatInvalid;
    }
    depth_format = MTL::PixelFormatInvalid;
    stencil_format = MTL::PixelFormatInvalid;
    PopulatePipelineFormatsFromRenderPassDescriptor(
        pass_descriptor, color_formats, 4, &depth_format, &stencil_format,
        &sample_count);
  }
  bool fragment_writes_depth =
      (pixel_translation && pixel_translation->shader().writes_depth()) ||
      (!pixel_translation && depth_only_pixel_library_ &&
       !depth_only_pixel_function_name_.empty());
  EnsureDepthFormatForDepthWritingFragment(
      "SPIRV-Cross tess pipeline", fragment_writes_depth, &depth_format);

  // Build cache key incorporating RT formats, tessellation mode, and blend
  // state (same blend fields as GetOrCreateMslPipelineState).
  auto tess_mode = regs.Get<reg::VGT_HOS_CNTL>().tess_mode;
  uint32_t pixel_shader_writes_color_targets =
      pixel_translation ? pixel_translation->shader().writes_color_targets()
                        : 0;
  uint32_t normalized_color_mask = 0;
  if (pixel_shader_writes_color_targets) {
    normalized_color_mask = draw_util::GetNormalizedColorMask(
        regs, pixel_shader_writes_color_targets);
  }
  auto rb_colorcontrol = regs.Get<reg::RB_COLORCONTROL>();
  struct TessPipelineKey {
    const void* ds;
    const void* ps;
    uint32_t host_vertex_shader_type;
    uint32_t tess_mode;
    uint32_t sample_count;
    uint32_t depth_format;
    uint32_t stencil_format;
    uint32_t color_formats[4];
    uint32_t normalized_color_mask;
    uint32_t alpha_to_mask_enable;
    uint32_t blendcontrol[4];
  } key_data = {};
  key_data.ds = domain_translation;
  key_data.ps = pixel_translation;
  key_data.host_vertex_shader_type = uint32_t(host_vertex_shader_type);
  key_data.tess_mode = uint32_t(tess_mode);
  key_data.sample_count = sample_count;
  key_data.depth_format = uint32_t(depth_format);
  key_data.stencil_format = uint32_t(stencil_format);
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.color_formats[i] = uint32_t(color_formats[i]);
  }
  key_data.normalized_color_mask = normalized_color_mask;
  key_data.alpha_to_mask_enable = rb_colorcontrol.alpha_to_mask_enable ? 1 : 0;
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.blendcontrol[i] =
        regs.Get<reg::RB_BLENDCONTROL>(
                reg::RB_BLENDCONTROL::rt_register_indices[i])
            .value;
  }
  uint64_t key = XXH3_64bits(&key_data, sizeof(key_data));
  auto it = msl_tess_pipeline_cache_.find(key);
  if (it != msl_tess_pipeline_cache_.end()) {
    return it->second;
  }

  auto* desc = MTL::RenderPipelineDescriptor::alloc()->init();
  // The post-tessellation vertex function IS the domain shader.
  desc->setVertexFunction(domain_translation->metal_function());

  if (pixel_translation && pixel_translation->metal_function()) {
    desc->setFragmentFunction(pixel_translation->metal_function());
  } else if (depth_only_pixel_function_name_.size() &&
             depth_only_pixel_library_) {
    auto* fn_name = NS::String::string(depth_only_pixel_function_name_.c_str(),
                                       NS::UTF8StringEncoding);
    MTL::Function* depth_fn = depth_only_pixel_library_->newFunction(fn_name);
    if (depth_fn) {
      desc->setFragmentFunction(depth_fn);
      depth_fn->release();
    }
  }

  // Tessellation configuration.
  desc->setMaxTessellationFactor(64);
  desc->setTessellationFactorStepFunction(
      MTL::TessellationFactorStepFunctionPerPatch);

  switch (tess_mode) {
    case xenos::TessellationMode::kDiscrete:
      desc->setTessellationPartitionMode(MTL::TessellationPartitionModeInteger);
      break;
    case xenos::TessellationMode::kContinuous:
      desc->setTessellationPartitionMode(
          MTL::TessellationPartitionModeFractionalEven);
      break;
    case xenos::TessellationMode::kAdaptive:
      desc->setTessellationPartitionMode(
          MTL::TessellationPartitionModeFractionalEven);
      break;
  }

  // Control point index type (not needed for our use case since the
  // domain shader reads control points from shared memory).
  desc->setTessellationControlPointIndexType(
      MTL::TessellationControlPointIndexTypeNone);

  // Render target attachments with blend state (same as non-tess pipeline).
  desc->setAlphaToCoverageEnabled(key_data.alpha_to_mask_enable != 0);
  for (uint32_t i = 0; i < 4; ++i) {
    auto* color_attachment = desc->colorAttachments()->object(i);
    color_attachment->setPixelFormat(color_formats[i]);
    if (color_formats[i] == MTL::PixelFormatInvalid) {
      color_attachment->setWriteMask(MTL::ColorWriteMaskNone);
      color_attachment->setBlendingEnabled(false);
      continue;
    }
    uint32_t rt_write_mask = (normalized_color_mask >> (i * 4)) & 0xF;
    color_attachment->setWriteMask(ToMetalColorWriteMask(rt_write_mask));
    if (!rt_write_mask) {
      color_attachment->setBlendingEnabled(false);
      continue;
    }
    auto blendcontrol = regs.Get<reg::RB_BLENDCONTROL>(
        reg::RB_BLENDCONTROL::rt_register_indices[i]);
    MTL::BlendFactor src_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_srcblend);
    MTL::BlendFactor dst_rgb =
        ToMetalBlendFactorRgb(blendcontrol.color_destblend);
    MTL::BlendOperation op_rgb =
        ToMetalBlendOperation(blendcontrol.color_comb_fcn);
    MTL::BlendFactor src_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_srcblend);
    MTL::BlendFactor dst_alpha =
        ToMetalBlendFactorAlpha(blendcontrol.alpha_destblend);
    MTL::BlendOperation op_alpha =
        ToMetalBlendOperation(blendcontrol.alpha_comb_fcn);
    if (i == 0) {
      ApplyFixed16SourceFactorPremult(color_formats[0], blendcontrol, src_rgb,
                                      src_alpha);
    }
    bool blending_enabled =
        src_rgb != MTL::BlendFactorOne || dst_rgb != MTL::BlendFactorZero ||
        op_rgb != MTL::BlendOperationAdd || src_alpha != MTL::BlendFactorOne ||
        dst_alpha != MTL::BlendFactorZero || op_alpha != MTL::BlendOperationAdd;
    color_attachment->setBlendingEnabled(blending_enabled);
    if (blending_enabled) {
      color_attachment->setSourceRGBBlendFactor(src_rgb);
      color_attachment->setDestinationRGBBlendFactor(dst_rgb);
      color_attachment->setRgbBlendOperation(op_rgb);
      color_attachment->setSourceAlphaBlendFactor(src_alpha);
      color_attachment->setDestinationAlphaBlendFactor(dst_alpha);
      color_attachment->setAlphaBlendOperation(op_alpha);
    }
  }
  desc->setDepthAttachmentPixelFormat(depth_format);
  desc->setStencilAttachmentPixelFormat(stencil_format);
  desc->setSampleCount(sample_count);

  NS::Error* error = nullptr;
  MTL::RenderPipelineState* pso = device_->newRenderPipelineState(desc, &error);
  desc->release();

  if (!pso) {
    if (error) {
      XELOGE("SPIRV-Cross tess pipeline error: {}",
             error->localizedDescription()->utf8String());
    }
    return nullptr;
  }

  msl_tess_pipeline_cache_[key] = pso;
  return pso;
}

// ==========================================================================
// SPIRV-Cross (MSL) path — shader modification + system constants helpers.
// ==========================================================================

SpirvShaderTranslator::Modification
MetalCommandProcessor::GetCurrentSpirvVertexShaderModification(
    const Shader& shader, Shader::HostVertexShaderType host_vertex_shader_type,
    uint32_t interpolator_mask) const {
  const auto& regs = *register_file_;

  SpirvShaderTranslator::Modification modification(
      spirv_shader_translator_->GetDefaultVertexShaderModification(
          shader.GetDynamicAddressableRegisterCount(
              regs.Get<reg::SQ_PROGRAM_CNTL>().vs_num_reg),
          host_vertex_shader_type));

  modification.vertex.interpolator_mask = interpolator_mask;

  auto pa_cl_clip_cntl = regs.Get<reg::PA_CL_CLIP_CNTL>();
  uint32_t user_clip_planes =
      pa_cl_clip_cntl.clip_disable ? 0 : pa_cl_clip_cntl.ucp_ena;
  modification.vertex.user_clip_plane_count = xe::bit_count(user_clip_planes);
  modification.vertex.user_clip_plane_cull =
      uint32_t(user_clip_planes && pa_cl_clip_cntl.ucp_cull_only_ena);

  modification.vertex.output_point_parameters =
      uint32_t((shader.writes_point_size_edge_flag_kill_vertex() & 0b001) &&
               regs.Get<reg::VGT_DRAW_INITIATOR>().prim_type ==
                   xenos::PrimitiveType::kPointList);

  return modification;
}

SpirvShaderTranslator::Modification
MetalCommandProcessor::GetCurrentSpirvPixelShaderModification(
    const Shader& shader, uint32_t interpolator_mask, uint32_t param_gen_pos,
    reg::RB_DEPTHCONTROL normalized_depth_control,
    uint32_t normalized_color_mask) const {
  const auto& regs = *register_file_;

  SpirvShaderTranslator::Modification modification(
      spirv_shader_translator_->GetDefaultPixelShaderModification(
          shader.GetDynamicAddressableRegisterCount(
              regs.Get<reg::SQ_PROGRAM_CNTL>().ps_num_reg)));

  modification.pixel.interpolator_mask = interpolator_mask;
  modification.pixel.interpolators_centroid =
      interpolator_mask &
      ~xenos::GetInterpolatorSamplingPattern(
          regs.Get<reg::RB_SURFACE_INFO>().msaa_samples,
          regs.Get<reg::SQ_CONTEXT_MISC>().sc_sample_cntl,
          regs.Get<reg::SQ_INTERPOLATOR_CNTL>().sampling_pattern);

  if (param_gen_pos < xenos::kMaxInterpolators) {
    modification.pixel.param_gen_enable = 1;
    modification.pixel.param_gen_interpolator = param_gen_pos;
    modification.pixel.param_gen_point =
        uint32_t(regs.Get<reg::VGT_DRAW_INITIATOR>().prim_type ==
                 xenos::PrimitiveType::kPointList);
  } else {
    modification.pixel.param_gen_enable = 0;
    modification.pixel.param_gen_interpolator = 0;
    modification.pixel.param_gen_point = 0;
  }

  using DepthStencilMode =
      SpirvShaderTranslator::Modification::DepthStencilMode;
  if (shader.implicit_early_z_write_allowed() &&
      (!shader.writes_color_target(0) ||
       !draw_util::DoesCoverageDependOnAlpha(
           regs.Get<reg::RB_COLORCONTROL>()))) {
    modification.pixel.depth_stencil_mode = DepthStencilMode::kEarlyHint;
  } else {
    modification.pixel.depth_stencil_mode = DepthStencilMode::kNoModifiers;
  }

  // Initialize MIN/MAX blend pre-multiply factors to kOne (no pre-multiply).
  modification.pixel.rt0_blend_rgb_factor_for_premult =
      xenos::BlendFactor::kOne;
  modification.pixel.rt0_blend_a_factor_for_premult = xenos::BlendFactor::kOne;

  bool rt0_minmax_premult_rgb_expected = false;
  bool rt0_minmax_premult_a_expected = false;
  if (shader.writes_color_target(0)) {
    auto blend_control = regs.Get<reg::RB_BLENDCONTROL>(
        reg::RB_BLENDCONTROL::rt_register_indices[0]);
    rt0_minmax_premult_rgb_expected =
        (blend_control.color_comb_fcn == xenos::BlendOp::kMin ||
         blend_control.color_comb_fcn == xenos::BlendOp::kMax) &&
        blend_control.color_srcblend == xenos::BlendFactor::kSrcAlpha &&
        blend_control.color_destblend == xenos::BlendFactor::kOne;
    rt0_minmax_premult_a_expected =
        (blend_control.alpha_comb_fcn == xenos::BlendOp::kMin ||
         blend_control.alpha_comb_fcn == xenos::BlendOp::kMax) &&
        blend_control.alpha_srcblend == xenos::BlendFactor::kSrcAlpha &&
        blend_control.alpha_destblend == xenos::BlendFactor::kOne;
    if (rt0_minmax_premult_rgb_expected) {
      modification.pixel.rt0_blend_rgb_factor_for_premult =
          xenos::BlendFactor::kSrcAlpha;
    }
    if (rt0_minmax_premult_a_expected) {
      modification.pixel.rt0_blend_a_factor_for_premult =
          xenos::BlendFactor::kSrcAlpha;
    }
  }
  // A source factor of a 16_16 / 16_16_16_16 target the host divides by 32
  // (GetFixed16SourceFactorPremult).
  if (shader.writes_color_target(0) && !rt0_minmax_premult_rgb_expected &&
      !rt0_minmax_premult_a_expected && (normalized_color_mask & 0xF) &&
      render_target_cache_ &&
      render_target_cache_->GetPath() ==
          RenderTargetCache::Path::kHostRenderTargets) {
    xenos::ColorRenderTargetFormat rt0_format =
        regs.Get<reg::RB_COLOR_INFO>(reg::RB_COLOR_INFO::rt_register_indices[0])
            .color_format;
    xenos::BlendFactor rgb_premult, a_premult;
    if ((rt0_format == xenos::ColorRenderTargetFormat::k_16_16 ||
         rt0_format == xenos::ColorRenderTargetFormat::k_16_16_16_16) &&
        GetFixed16SourceFactorPremult(
            regs.Get<reg::RB_BLENDCONTROL>(
                reg::RB_BLENDCONTROL::rt_register_indices[0]),
            rgb_premult, a_premult)) {
      modification.pixel.rt0_blend_rgb_factor_for_premult = rgb_premult;
      modification.pixel.rt0_blend_a_factor_for_premult = a_premult;
    }
  }
  if (rt0_minmax_premult_rgb_expected || rt0_minmax_premult_a_expected) {
    XELOGD(
        "SPIRV-Cross PS mod diagnostic: shader={:016X} expected_premult(rgb={},"
        "a={}) selected(rgb={},a={})",
        shader.ucode_data_hash(), rt0_minmax_premult_rgb_expected ? 1 : 0,
        rt0_minmax_premult_a_expected ? 1 : 0,
        uint32_t(modification.pixel.rt0_blend_rgb_factor_for_premult),
        uint32_t(modification.pixel.rt0_blend_a_factor_for_premult));
  }

  // Extract 1 bit per RT from the 4-bits-per-RT normalized_color_mask.
  // Without this, color_targets_used defaults to 0 and the SPIR-V translator
  // declares NO fragment color outputs, producing black/transparent rendering.
  modification.pixel.color_targets_used =
      (((normalized_color_mask >> 0) & 0xF) ? 1 : 0) |
      (((normalized_color_mask >> 4) & 0xF) ? 2 : 0) |
      (((normalized_color_mask >> 8) & 0xF) ? 4 : 0) |
      (((normalized_color_mask >> 12) & 0xF) ? 8 : 0);

  return modification;
}

MTL::RenderPipelineState* MetalCommandProcessor::GetOrCreateMslPipelineState(
    MslShader::MslTranslation* vertex_translation,
    MslShader::MslTranslation* pixel_translation, const RegisterFile& regs,
    bool spirv2air, MslPipelineCompileStatus* compile_status_out) {
  if (compile_status_out) {
    *compile_status_out = MslPipelineCompileStatus::kFailed;
  }
  MTL::Function* vertex_function =
      vertex_translation ? (spirv2air ? vertex_translation->spirv2air_function()
                                      : vertex_translation->metal_function())
                         : nullptr;
  MTL::Function* fragment_function =
      pixel_translation ? (spirv2air ? pixel_translation->spirv2air_function()
                                     : pixel_translation->metal_function())
                        : nullptr;
  if (!vertex_function || (pixel_translation && !fragment_function)) {
    XELOGE("SPIRV-Cross: No valid vertex or pixel shader function");
    return nullptr;
  }

  // Determine attachment formats from render target cache.
  uint32_t sample_count = 1;
  MTL::PixelFormat color_formats[4] = {
      MTL::PixelFormatInvalid, MTL::PixelFormatInvalid, MTL::PixelFormatInvalid,
      MTL::PixelFormatInvalid};
  MTL::PixelFormat depth_format = MTL::PixelFormatInvalid;
  MTL::PixelFormat stencil_format = MTL::PixelFormatInvalid;
  if (render_target_cache_) {
    for (uint32_t i = 0; i < 4; ++i) {
      if (MTL::Texture* rt = render_target_cache_->GetColorTargetForDraw(i)) {
        color_formats[i] = rt->pixelFormat();
        if (rt->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(rt->sampleCount()));
        }
      }
    }
    if (color_formats[0] == MTL::PixelFormatInvalid) {
      if (MTL::Texture* dummy =
              render_target_cache_->GetDummyColorTargetForDraw()) {
        color_formats[0] = dummy->pixelFormat();
        if (dummy->sampleCount() > 0) {
          sample_count = std::max<uint32_t>(
              sample_count, static_cast<uint32_t>(dummy->sampleCount()));
        }
      }
    }
    if (MTL::Texture* depth_tex =
            render_target_cache_->GetDepthTargetForDraw()) {
      depth_format = depth_tex->pixelFormat();
      switch (depth_format) {
        case MTL::PixelFormatDepth32Float_Stencil8:
        case MTL::PixelFormatDepth24Unorm_Stencil8:
        case MTL::PixelFormatX32_Stencil8:
          stencil_format = depth_format;
          break;
        default:
          stencil_format = MTL::PixelFormatInvalid;
          break;
      }
      if (depth_tex->sampleCount() > 0) {
        sample_count = std::max<uint32_t>(
            sample_count, static_cast<uint32_t>(depth_tex->sampleCount()));
      }
    }
  }

  // Match the active render pass attachments exactly to avoid
  // setRenderPipelineState validation failures when the pass descriptor differs
  // from the cache snapshot (for instance, after attachment reconfiguration).
  MTL::RenderPassDescriptor* pass_descriptor = current_render_pass_descriptor_;
  if (!pass_descriptor) {
    pass_descriptor = render_pass_descriptor_;
  }
  if (pass_descriptor) {
    sample_count = 1;
    for (uint32_t i = 0; i < 4; ++i) {
      color_formats[i] = MTL::PixelFormatInvalid;
    }
    depth_format = MTL::PixelFormatInvalid;
    stencil_format = MTL::PixelFormatInvalid;
    PopulatePipelineFormatsFromRenderPassDescriptor(
        pass_descriptor, color_formats, 4, &depth_format, &stencil_format,
        &sample_count);
  }
  bool pixel_shader_writes_depth =
      pixel_translation && pixel_translation->shader().writes_depth();
  EnsureDepthFormatForDepthWritingFragment(
      "SPIRV-Cross pipeline", pixel_shader_writes_depth, &depth_format);

  // Build pipeline cache key.
  struct MslPipelineKey {
    const void* vs;
    const void* ps;
    uint32_t sample_count;
    uint32_t depth_format;
    uint32_t stencil_format;
    uint32_t color_formats[4];
    uint32_t normalized_color_mask;
    uint32_t alpha_to_mask_enable;
    uint32_t blendcontrol[4];
    uint32_t spirv2air;
  } key_data = {};
  key_data.vs = vertex_translation;
  key_data.ps = pixel_translation;
  key_data.spirv2air = spirv2air ? 1 : 0;
  key_data.sample_count = sample_count;
  key_data.depth_format = uint32_t(depth_format);
  key_data.stencil_format = uint32_t(stencil_format);
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.color_formats[i] = uint32_t(color_formats[i]);
  }
  uint32_t pixel_shader_writes_color_targets =
      pixel_translation ? pixel_translation->shader().writes_color_targets()
                        : 0;
  key_data.normalized_color_mask = 0;
  if (pixel_shader_writes_color_targets) {
    key_data.normalized_color_mask = draw_util::GetNormalizedColorMask(
        regs, pixel_shader_writes_color_targets);
  }
  auto rb_colorcontrol = regs.Get<reg::RB_COLORCONTROL>();
  key_data.alpha_to_mask_enable = rb_colorcontrol.alpha_to_mask_enable ? 1 : 0;
  for (uint32_t i = 0; i < 4; ++i) {
    key_data.blendcontrol[i] =
        regs.Get<reg::RB_BLENDCONTROL>(
                reg::RB_BLENDCONTROL::rt_register_indices[i])
            .value;
  }
  uint64_t key = XXH3_64bits(&key_data, sizeof(key_data));

  bool use_async = cvars::async_shader_compilation &&
                   !msl_shader_compile_threads_.empty() &&
                   pixel_translation != nullptr;

  {
    std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
    auto it = msl_pipeline_cache_.find(key);
    if (it != msl_pipeline_cache_.end()) {
      if (compile_status_out) {
        *compile_status_out = MslPipelineCompileStatus::kReady;
      }
      return it->second;
    }
    if (msl_pipeline_compile_pending_.find(key) !=
        msl_pipeline_compile_pending_.end()) {
      if (compile_status_out) {
        *compile_status_out = MslPipelineCompileStatus::kPending;
      }
      return nullptr;
    }
    if (msl_pipeline_compile_failed_.find(key) !=
        msl_pipeline_compile_failed_.end()) {
      return nullptr;
    }
  }

  MslPipelineCompileRequest request = {};
  request.pipeline_key = key;
  request.vertex_shader_hash = vertex_translation->shader().ucode_data_hash();
  request.vertex_modification = vertex_translation->modification();
  if (pixel_translation) {
    request.pixel_shader_hash = pixel_translation->shader().ucode_data_hash();
    request.pixel_modification = pixel_translation->modification();
  }
  request.vertex_function = vertex_function;
  request.fragment_function = fragment_function;
  request.sample_count = sample_count;
  request.depth_format = depth_format;
  request.stencil_format = stencil_format;
  request.normalized_color_mask = key_data.normalized_color_mask;
  request.alpha_to_mask_enable = key_data.alpha_to_mask_enable;
  request.priority = pixel_translation ? 2 : 1;
  for (uint32_t i = 0; i < 4; ++i) {
    request.color_formats[i] = color_formats[i];
    request.blendcontrol[i] = key_data.blendcontrol[i];
  }

  if (use_async) {
    if (EnqueueMslPipelineCompilation(request)) {
      std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
      auto it = msl_pipeline_cache_.find(key);
      if (it != msl_pipeline_cache_.end()) {
        if (compile_status_out) {
          *compile_status_out = MslPipelineCompileStatus::kReady;
        }
        return it->second;
      }
      if (msl_pipeline_compile_failed_.find(key) !=
          msl_pipeline_compile_failed_.end()) {
        return nullptr;
      }
      if (compile_status_out) {
        *compile_status_out = MslPipelineCompileStatus::kPending;
      }
      return nullptr;
    }
    std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
    if (msl_pipeline_compile_failed_.find(key) !=
        msl_pipeline_compile_failed_.end()) {
      return nullptr;
    }
  }

  std::string error_message;
  MTL::RenderPipelineState* pipeline =
      CreateMslPipelineState(request, &error_message);
  if (!pipeline) {
    if (!error_message.empty()) {
      XELOGE("SPIRV-Cross: Failed to create pipeline: {}", error_message);
    } else {
      XELOGE("SPIRV-Cross: Failed to create pipeline (unknown error)");
    }
    std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
    msl_pipeline_compile_failed_.insert(key);
    return nullptr;
  }

  {
    std::lock_guard<std::mutex> lock(msl_shader_compile_mutex_);
    auto [it, inserted] = msl_pipeline_cache_.emplace(key, pipeline);
    if (!inserted) {
      pipeline->release();
      pipeline = it->second;
    }
    msl_pipeline_compile_failed_.erase(key);
  }
  if (compile_status_out) {
    *compile_status_out = MslPipelineCompileStatus::kReady;
  }
  return pipeline;
}

void MetalCommandProcessor::UpdateSpirvSystemConstantValues(
    const PrimitiveProcessor::ProcessingResult& primitive_processing_result,
    bool primitive_polygonal, uint32_t line_loop_closing_index,
    xenos::Endian index_endian, const draw_util::ViewportInfo& viewport_info,
    uint32_t used_texture_mask, reg::RB_DEPTHCONTROL normalized_depth_control,
    uint32_t normalized_color_mask) {
  const SpirvShaderTranslator::SystemConstants previous_system_constants =
      spirv_system_constants_;
  const SpirvShaderTranslator::ClipPlaneConstants
      previous_clip_plane_constants = spirv_clip_plane_constants_;
  const SpirvShaderTranslator::TessellationConstants
      previous_tessellation_constants = spirv_tessellation_constants_;

  const RegisterFile& regs = *register_file_;
  auto pa_cl_vte_cntl = regs.Get<reg::PA_CL_VTE_CNTL>();
  auto rb_alpha_ref = regs.Get<float>(XE_GPU_REG_RB_ALPHA_REF);
  auto rb_colorcontrol = regs.Get<reg::RB_COLORCONTROL>();
  auto rb_depth_info = regs.Get<reg::RB_DEPTH_INFO>();
  auto rb_surface_info = regs.Get<reg::RB_SURFACE_INFO>();
  auto vgt_draw_initiator = regs.Get<reg::VGT_DRAW_INITIATOR>();

  auto& consts = spirv_system_constants_;
  std::memset(&consts, 0, sizeof(consts));

  // Build flags (matching Vulkan backend's SpirvShaderTranslator kSysFlag_*).
  uint32_t flags = 0;

  // Coordinate format.
  if (pa_cl_vte_cntl.vtx_xy_fmt) {
    flags |= SpirvShaderTranslator::kSysFlag_XYDividedByW;
  }
  if (pa_cl_vte_cntl.vtx_z_fmt) {
    flags |= SpirvShaderTranslator::kSysFlag_ZDividedByW;
  }
  if (pa_cl_vte_cntl.vtx_w0_fmt) {
    flags |= SpirvShaderTranslator::kSysFlag_WNotReciprocal;
  }

  // Primitive type.
  if (primitive_polygonal) {
    flags |= SpirvShaderTranslator::kSysFlag_PrimitivePolygonal;
  }
  if (vgt_draw_initiator.prim_type == xenos::PrimitiveType::kLineList ||
      vgt_draw_initiator.prim_type == xenos::PrimitiveType::kLineStrip ||
      vgt_draw_initiator.prim_type == xenos::PrimitiveType::kLineLoop ||
      vgt_draw_initiator.prim_type == xenos::PrimitiveType::k2DLineStrip) {
    flags |= SpirvShaderTranslator::kSysFlag_PrimitiveLine;
  }

  // MSAA sample count.
  flags |= uint32_t(rb_surface_info.msaa_samples)
           << SpirvShaderTranslator::kSysFlag_MsaaSamples_Shift;
  // msaa_samples_as_pixels: drawing to the samples of an MSAA surface.
  if (render_target_cache_) {
    if (render_target_cache_->last_update_samples_as_pixels_x_log2()) {
      flags |= SpirvShaderTranslator::kSysFlag_SamplesAsPixelsX;
    }
    if (render_target_cache_->last_update_samples_as_pixels_y_log2()) {
      flags |= SpirvShaderTranslator::kSysFlag_SamplesAsPixelsY;
    }
  }

  // Depth format.
  if (rb_depth_info.depth_format == xenos::DepthRenderTargetFormat::kD24FS8) {
    flags |= SpirvShaderTranslator::kSysFlag_DepthFloat24;
  }

  // Alpha test — pack the CompareFunction value directly into the flag bits
  // (matching Vulkan backend behavior).
  xenos::CompareFunction alpha_test_function =
      rb_colorcontrol.alpha_test_enable ? rb_colorcontrol.alpha_func
                                        : xenos::CompareFunction::kAlways;
  flags |= uint32_t(alpha_test_function)
           << SpirvShaderTranslator::kSysFlag_AlphaPassIfLess_Shift;

  // Gamma correction for render targets.
  reg::RB_COLOR_INFO color_infos[4];
  for (uint32_t i = 0; i < 4; ++i) {
    color_infos[i] = regs.Get<reg::RB_COLOR_INFO>(
        reg::RB_COLOR_INFO::rt_register_indices[i]);
  }
  for (uint32_t i = 0; i < 4; ++i) {
    if (color_infos[i].color_format ==
        xenos::ColorRenderTargetFormat::k_8_8_8_8_GAMMA) {
      flags |= SpirvShaderTranslator::kSysFlag_ConvertColor0ToGamma << i;
    }
  }
  // Blend factors of render target 0 pre-multiplied in the shader are of the
  // output before it is divided by 32 (GetFixed16SourceFactorPremult).
  if (render_target_cache_ &&
      render_target_cache_->GetPath() ==
          RenderTargetCache::Path::kHostRenderTargets &&
      ((color_infos[0].color_format ==
            xenos::ColorRenderTargetFormat::k_16_16 &&
        !render_target_cache_->IsFixedRG16TruncatedToMinus1To1()) ||
       (color_infos[0].color_format ==
            xenos::ColorRenderTargetFormat::k_16_16_16_16 &&
        !render_target_cache_->IsFixedRGBA16TruncatedToMinus1To1()))) {
    flags |= SpirvShaderTranslator::kSysFlag_Color0Fixed16Scaled;
  }

  // Vertex index loading for VS-based primitive expansion (point sprites,
  // rectangle lists).  When the primitive processor builds a host-side
  // index buffer for DMA-based VS expansion the shader must load the
  // original guest vertex index from shared memory.
  if (primitive_processing_result.index_buffer_type ==
      PrimitiveProcessor::ProcessedIndexBufferType::kHostBuiltinForDMA) {
    flags |= SpirvShaderTranslator::kSysFlag_ComputeOrPrimitiveVertexIndexLoad;
    if (vgt_draw_initiator.index_size == xenos::IndexFormat::kInt32) {
      flags |= SpirvShaderTranslator::
          kSysFlag_ComputeOrPrimitiveVertexIndexLoad32Bit;
    }
  }

  consts.flags = flags;

  // Vertex index.
  consts.vertex_index_endian = index_endian;
  consts.vertex_base_index = regs.Get<reg::VGT_INDX_OFFSET>().indx_offset;
  const bool is_vs_expansion_draw =
      primitive_processing_result.host_vertex_shader_type ==
          Shader::HostVertexShaderType::kPointListAsTriangleStrip ||
      primitive_processing_result.host_vertex_shader_type ==
          Shader::HostVertexShaderType::kRectangleListAsTriangleStrip;
  consts.vertex_index_count =
      is_vs_expansion_draw ? primitive_processing_result.guest_draw_vertex_count
                           : primitive_processing_result.host_draw_vertex_count;

  // Vertex index load address (for VS-based primitive expansion).
  if (flags &
      (SpirvShaderTranslator::kSysFlag_VertexIndexLoad |
       SpirvShaderTranslator::kSysFlag_ComputeOrPrimitiveVertexIndexLoad)) {
    consts.vertex_index_load_address =
        primitive_processing_result.guest_index_base;
  }

  // NDC scale/offset.
  for (uint32_t i = 0; i < 3; ++i) {
    consts.ndc_scale[i] = viewport_info.ndc_scale[i];
    consts.ndc_offset[i] = viewport_info.ndc_offset[i];
  }

  // Point rendering (matching Vulkan backend).
  auto pa_su_point_size = regs.Get<reg::PA_SU_POINT_SIZE>();
  auto pa_su_point_minmax = regs.Get<reg::PA_SU_POINT_MINMAX>();
  consts.point_vertex_diameter_min =
      float(pa_su_point_minmax.min_size) * (2.0f / 16.0f);
  consts.point_vertex_diameter_max =
      float(pa_su_point_minmax.max_size) * (2.0f / 16.0f);
  consts.point_constant_diameter[0] =
      float(pa_su_point_size.width) * (2.0f / 16.0f);
  consts.point_constant_diameter[1] =
      float(pa_su_point_size.height) * (2.0f / 16.0f);
  // 2 because 1 in the NDC is half of the viewport's axis, 0.5 for diameter
  // to radius conversion — matching the Vulkan backend formula.
  uint32_t draw_resolution_scale_x =
      texture_cache_ ? texture_cache_->draw_resolution_scale_x() : 1;
  uint32_t draw_resolution_scale_y =
      texture_cache_ ? texture_cache_->draw_resolution_scale_y() : 1;
  if (render_target_cache_) {
    // msaa_samples_as_pixels, as for the viewport.
    draw_resolution_scale_x <<=
        render_target_cache_->last_update_samples_as_pixels_x_log2();
    draw_resolution_scale_y <<=
        render_target_cache_->last_update_samples_as_pixels_y_log2();
  }
  consts.point_screen_diameter_to_ndc_radius[0] =
      float(draw_resolution_scale_x) /
      float(std::max(viewport_info.xy_extent[0], uint32_t(1)));
  consts.point_screen_diameter_to_ndc_radius[1] =
      float(draw_resolution_scale_y) /
      float(std::max(viewport_info.xy_extent[1], uint32_t(1)));

  // Texture swizzled signs and swizzles — retrieved from the texture cache
  // (matching Vulkan backend behavior).
  if (texture_cache_) {
    for (uint32_t i = 0; i < 32; ++i) {
      if (!(used_texture_mask & (uint32_t(1) << i))) {
        continue;
      }
      // Swizzled signs: 8 bits per texture, 4 textures per uint32.
      uint8_t texture_signs = texture_cache_->GetActiveTextureSwizzledSigns(i);
      uint32_t signs_shift = 8 * (i & 3);
      consts.texture_swizzled_signs[i >> 2] |= uint32_t(texture_signs)
                                               << signs_shift;

      // Host swizzles: 12 bits per texture, 2 textures per uint32.
      uint32_t texture_swizzle = texture_cache_->GetActiveTextureHostSwizzle(i);
      uint32_t swizzle_shift = 12 * (i & 1);
      consts.texture_swizzles[i >> 1] |= (texture_swizzle & 0xFFF)
                                         << swizzle_shift;

      // Integer textures (num_format): the width of their components in
      // bytes, for the shader's integer scale, in the bits 24:27 (even) or
      // 28:31 (odd) the swizzles leave free. Only formats with 8 or 16 bits in
      // every component, and not with a constant 1 in the swizzle, which must
      // stay 1.
      xenos::xe_gpu_texture_fetch_t texture_fetch = regs.GetTextureFetch(i);
      if (texture_fetch.num_format) {
        uint32_t texture_integer_bytes = 0;
        switch (texture_fetch.format) {
          case xenos::TextureFormat::k_8:
          case xenos::TextureFormat::k_8_A:
          case xenos::TextureFormat::k_8_B:
          case xenos::TextureFormat::k_8_8:
          case xenos::TextureFormat::k_8_8_8_8:
          case xenos::TextureFormat::k_8_8_8_8_A:
          case xenos::TextureFormat::k_8_8_8_8_AS_16_16_16_16:
            texture_integer_bytes = 1;
            break;
          case xenos::TextureFormat::k_16:
          case xenos::TextureFormat::k_16_16:
          case xenos::TextureFormat::k_16_16_16_16:
            texture_integer_bytes = 2;
            break;
          default:
            break;
        }
        for (uint32_t swizzle_component = 0; swizzle_component < 4;
             ++swizzle_component) {
          if (((texture_fetch.swizzle >> (3 * swizzle_component)) & 7) ==
              xenos::XE_GPU_TEXTURE_SWIZZLE_1) {
            texture_integer_bytes = 0;
          }
        }
        consts.texture_swizzles[i >> 1] |= texture_integer_bytes
                                           << (24 + 4 * (i & 1));
      }
    }
  }

  // Textures resolved — which textures are from scaled resolve operations
  // (matching Vulkan backend).
  if (texture_cache_) {
    uint32_t textures_resolved = 0;
    uint32_t textures_remaining = used_texture_mask;
    uint32_t texture_index;
    while (xe::bit_scan_forward(textures_remaining, &texture_index)) {
      textures_remaining &= ~(UINT32_C(1) << texture_index);
      textures_resolved |=
          uint32_t(
              texture_cache_->IsActiveTextureResolutionScaled(texture_index))
          << texture_index;
    }
    consts.textures_resolved = textures_resolved;
  }

  // Alpha test reference.
  consts.alpha_test_reference = rb_alpha_ref;

  // Alpha to mask — if enabled, bits 0:7 are sample offsets, bit 8 = 1.
  // (matching Vulkan backend / MSC path).
  if (rb_colorcontrol.alpha_to_mask_enable) {
    consts.alpha_to_mask = (rb_colorcontrol.value >> 24) | (1 << 8);
  }

  // Color exponent bias (matching Vulkan backend).
  for (uint32_t i = 0; i < 4; ++i) {
    int32_t color_exp_bias = color_infos[i].color_exp_bias;
    if (render_target_cache_->GetPath() ==
            RenderTargetCache::Path::kHostRenderTargets &&
        ((color_infos[i].color_format ==
              xenos::ColorRenderTargetFormat::k_16_16 &&
          !render_target_cache_->IsFixedRG16TruncatedToMinus1To1()) ||
         (color_infos[i].color_format ==
              xenos::ColorRenderTargetFormat::k_16_16_16_16 &&
          !render_target_cache_->IsFixedRGBA16TruncatedToMinus1To1()))) {
      color_exp_bias -= 5;
    }
    float color_exp_bias_scale;
    *reinterpret_cast<int32_t*>(&color_exp_bias_scale) =
        UINT32_C(0x3F800000) + (color_exp_bias << 23);
    consts.color_exp_bias[i] = color_exp_bias_scale;
  }

  // Clip plane constants (separate buffer for the SPIR-V translator).
  // The SPIR-V translator reads clip planes from a dedicated uniform buffer
  // (kConstantBufferClipPlanes), not from system constants.
  auto pa_cl_clip_cntl = regs.Get<reg::PA_CL_CLIP_CNTL>();
  std::memset(&spirv_clip_plane_constants_, 0,
              sizeof(spirv_clip_plane_constants_));
  if (!pa_cl_clip_cntl.clip_disable && pa_cl_clip_cntl.ucp_ena) {
    float* clip_plane_write_ptr =
        spirv_clip_plane_constants_.user_clip_planes[0];
    uint32_t clip_planes_remaining = pa_cl_clip_cntl.ucp_ena;
    uint32_t clip_plane_index;
    while (xe::bit_scan_forward(clip_planes_remaining, &clip_plane_index)) {
      clip_planes_remaining &= ~(UINT32_C(1) << clip_plane_index);
      const float* clip_plane_regs = reinterpret_cast<const float*>(
          &regs.values[XE_GPU_REG_PA_CL_UCP_0_X + clip_plane_index * 4]);
      std::memcpy(clip_plane_write_ptr, clip_plane_regs, 4 * sizeof(float));
      clip_plane_write_ptr += 4;
    }
  }

  // Tessellation constants (separate buffer for the SPIR-V translator).
  // Mirror Vulkan constant buffer population to keep shader inputs identical.
  std::memset(&spirv_tessellation_constants_, 0,
              sizeof(spirv_tessellation_constants_));
  float tess_factor_min =
      regs.Get<float>(XE_GPU_REG_VGT_HOS_MIN_TESS_LEVEL) + 1.0f;
  float tess_factor_max =
      regs.Get<float>(XE_GPU_REG_VGT_HOS_MAX_TESS_LEVEL) + 1.0f;
  spirv_tessellation_constants_.tessellation_factor_range[0] = tess_factor_min;
  spirv_tessellation_constants_.tessellation_factor_range[1] = tess_factor_max;
  auto vgt_dma_size = regs.Get<reg::VGT_DMA_SIZE>();
  spirv_tessellation_constants_.vertex_index_endian =
      static_cast<uint32_t>(vgt_dma_size.swap_mode);
  spirv_tessellation_constants_.vertex_index_offset =
      regs[XE_GPU_REG_VGT_INDX_OFFSET];
  spirv_tessellation_constants_.vertex_index_min_max[0] =
      regs[XE_GPU_REG_VGT_MIN_VTX_INDX];
  spirv_tessellation_constants_.vertex_index_min_max[1] =
      regs[XE_GPU_REG_VGT_MAX_VTX_INDX];

  if (std::memcmp(&previous_system_constants, &spirv_system_constants_,
                  sizeof(spirv_system_constants_)) != 0) {
    ++msl_system_constants_version_;
    if (msl_system_constants_version_ == 0) {
      msl_system_constants_version_ = 1;
    }
  }
  if (std::memcmp(&previous_clip_plane_constants, &spirv_clip_plane_constants_,
                  sizeof(spirv_clip_plane_constants_)) != 0) {
    ++msl_clip_plane_constants_version_;
    if (msl_clip_plane_constants_version_ == 0) {
      msl_clip_plane_constants_version_ = 1;
    }
  }
  if (std::memcmp(&previous_tessellation_constants,
                  &spirv_tessellation_constants_,
                  sizeof(spirv_tessellation_constants_)) != 0) {
    ++msl_tessellation_constants_version_;
    if (msl_tessellation_constants_version_ == 0) {
      msl_tessellation_constants_version_ = 1;
    }
  }
}

#if METAL_SHADER_CONVERTER_AVAILABLE
void MetalCommandProcessor::UpdateSystemConstantValues(
    bool shared_memory_is_uav, bool primitive_polygonal,
    uint32_t line_loop_closing_index, xenos::Endian index_endian,
    const draw_util::ViewportInfo& viewport_info, uint32_t used_texture_mask,
    reg::RB_DEPTHCONTROL normalized_depth_control,
    uint32_t normalized_color_mask) {
  const RegisterFile& regs = *register_file_;
  auto pa_cl_clip_cntl = regs.Get<reg::PA_CL_CLIP_CNTL>();
  auto pa_cl_vte_cntl = regs.Get<reg::PA_CL_VTE_CNTL>();
  auto rb_alpha_ref = regs.Get<float>(XE_GPU_REG_RB_ALPHA_REF);
  auto rb_colorcontrol = regs.Get<reg::RB_COLORCONTROL>();
  auto rb_depth_info = regs.Get<reg::RB_DEPTH_INFO>();
  auto rb_surface_info = regs.Get<reg::RB_SURFACE_INFO>();
  auto vgt_draw_initiator = regs.Get<reg::VGT_DRAW_INITIATOR>();
  uint32_t vgt_indx_offset = regs.Get<reg::VGT_INDX_OFFSET>().indx_offset;
  uint32_t vgt_max_vtx_indx = regs.Get<reg::VGT_MAX_VTX_INDX>().max_indx;
  uint32_t vgt_min_vtx_indx = regs.Get<reg::VGT_MIN_VTX_INDX>().min_indx;

  // Get color info for each render target
  reg::RB_COLOR_INFO color_infos[4];
  for (uint32_t i = 0; i < 4; ++i) {
    color_infos[i] = regs.Get<reg::RB_COLOR_INFO>(
        reg::RB_COLOR_INFO::rt_register_indices[i]);
  }

  // Build flags
  uint32_t flags = 0;

  // Shared memory mode - determines whether shaders read from SRV (T0) or UAV
  // (U0)
  if (shared_memory_is_uav) {
    flags |= DxbcShaderTranslator::kSysFlag_SharedMemoryIsUAV;
  }

  // W0 division control from PA_CL_VTE_CNTL
  if (pa_cl_vte_cntl.vtx_xy_fmt) {
    flags |= DxbcShaderTranslator::kSysFlag_XYDividedByW;
  }
  if (pa_cl_vte_cntl.vtx_z_fmt) {
    flags |= DxbcShaderTranslator::kSysFlag_ZDividedByW;
  }
  if (pa_cl_vte_cntl.vtx_w0_fmt) {
    flags |= DxbcShaderTranslator::kSysFlag_WNotReciprocal;
  }

  // Primitive type flags
  if (primitive_polygonal) {
    flags |= DxbcShaderTranslator::kSysFlag_PrimitivePolygonal;
  }
  if (draw_util::IsPrimitiveLine(regs)) {
    flags |= DxbcShaderTranslator::kSysFlag_PrimitiveLine;
  }

  // Depth format
  if (rb_depth_info.depth_format == xenos::DepthRenderTargetFormat::kD24FS8) {
    flags |= DxbcShaderTranslator::kSysFlag_DepthFloat24;
  }

  // Alpha test - encode compare function in flags
  xenos::CompareFunction alpha_test_function =
      rb_colorcontrol.alpha_test_enable ? rb_colorcontrol.alpha_func
                                        : xenos::CompareFunction::kAlways;
  flags |= uint32_t(alpha_test_function)
           << DxbcShaderTranslator::kSysFlag_AlphaPassIfLess_Shift;

  // Gamma conversion flags for render targets
  if (!render_target_cache_->gamma_render_target_as_unorm16()) {
    for (uint32_t i = 0; i < 4; ++i) {
      if (color_infos[i].color_format ==
          xenos::ColorRenderTargetFormat::k_8_8_8_8_GAMMA) {
        flags |= DxbcShaderTranslator::kSysFlag_ConvertColor0ToGamma << i;
      }
    }
  }

  system_constants_.flags = flags;

  // Tessellation factor range
  float tessellation_factor_min =
      regs.Get<float>(XE_GPU_REG_VGT_HOS_MIN_TESS_LEVEL) + 1.0f;
  float tessellation_factor_max =
      regs.Get<float>(XE_GPU_REG_VGT_HOS_MAX_TESS_LEVEL) + 1.0f;
  system_constants_.tessellation_factor_range_min = tessellation_factor_min;
  system_constants_.tessellation_factor_range_max = tessellation_factor_max;

  // Line loop closing index
  system_constants_.line_loop_closing_index = line_loop_closing_index;

  // Vertex index configuration
  system_constants_.vertex_index_endian = index_endian;
  system_constants_.vertex_index_offset = vgt_indx_offset;
  system_constants_.vertex_index_min = vgt_min_vtx_indx;
  system_constants_.vertex_index_max = vgt_max_vtx_indx;

  // User clip planes (when not CLIP_DISABLE)
  if (!pa_cl_clip_cntl.clip_disable) {
    float* user_clip_plane_write_ptr = system_constants_.user_clip_planes[0];
    uint32_t user_clip_planes_remaining = pa_cl_clip_cntl.ucp_ena;
    uint32_t user_clip_plane_index;
    while (xe::bit_scan_forward(user_clip_planes_remaining,
                                &user_clip_plane_index)) {
      user_clip_planes_remaining &= ~(UINT32_C(1) << user_clip_plane_index);
      const float* user_clip_plane_regs = reinterpret_cast<const float*>(
          &regs.values[XE_GPU_REG_PA_CL_UCP_0_X + user_clip_plane_index * 4]);
      std::memcpy(user_clip_plane_write_ptr, user_clip_plane_regs,
                  4 * sizeof(float));
      user_clip_plane_write_ptr += 4;
    }
  }

  // NDC scale and offset from viewport info
  for (uint32_t i = 0; i < 3; ++i) {
    system_constants_.ndc_scale[i] = viewport_info.ndc_scale[i];
    system_constants_.ndc_offset[i] = viewport_info.ndc_offset[i];
  }

  // Point size parameters
  if (vgt_draw_initiator.prim_type == xenos::PrimitiveType::kPointList) {
    auto pa_su_point_minmax = regs.Get<reg::PA_SU_POINT_MINMAX>();
    auto pa_su_point_size = regs.Get<reg::PA_SU_POINT_SIZE>();
    system_constants_.point_vertex_diameter_min =
        float(pa_su_point_minmax.min_size) * (2.0f / 16.0f);
    system_constants_.point_vertex_diameter_max =
        float(pa_su_point_minmax.max_size) * (2.0f / 16.0f);
    system_constants_.point_constant_diameter[0] =
        float(pa_su_point_size.width) * (2.0f / 16.0f);
    system_constants_.point_constant_diameter[1] =
        float(pa_su_point_size.height) * (2.0f / 16.0f);
    // Screen to NDC radius conversion.
    // 2 because 1 in the NDC is half of the viewport's axis, 0.5 for diameter
    // to radius conversion to avoid multiplying the per-vertex diameter by an
    // additional constant in the shader. Include draw_resolution_scale to
    // match D3D12 behavior.
    uint32_t point_draw_resolution_scale_x =
        render_target_cache_ ? render_target_cache_->draw_resolution_scale_x()
                             : 1;
    uint32_t point_draw_resolution_scale_y =
        render_target_cache_ ? render_target_cache_->draw_resolution_scale_y()
                             : 1;
    system_constants_.point_screen_diameter_to_ndc_radius[0] =
        (/* 0.5f * 2.0f * */ float(point_draw_resolution_scale_x)) /
        std::max(viewport_info.xy_extent[0], uint32_t(1));
    system_constants_.point_screen_diameter_to_ndc_radius[1] =
        (/* 0.5f * 2.0f * */ float(point_draw_resolution_scale_y)) /
        std::max(viewport_info.xy_extent[1], uint32_t(1));
  }

  // Texture signedness / resolution scaling (mirror D3D12 logic).
  // Always update textures_resolution_scaled, even when used_texture_mask is 0,
  // to avoid stale values from previous draws.
  uint32_t textures_resolution_scaled = 0;
  uint32_t textures_remaining = used_texture_mask;
  uint32_t texture_index;
  while (xe::bit_scan_forward(textures_remaining, &texture_index)) {
    textures_remaining &= ~(uint32_t(1) << texture_index);
    if (texture_cache_) {
      uint32_t& texture_signs_uint =
          system_constants_.texture_swizzled_signs[texture_index >> 2];
      uint32_t texture_signs_shift = (texture_index & 3) * 8;
      uint8_t texture_signs =
          texture_cache_->GetActiveTextureSwizzledSigns(texture_index);
      uint32_t texture_signs_shifted = uint32_t(texture_signs)
                                       << texture_signs_shift;
      uint32_t texture_signs_mask = uint32_t(0xFF) << texture_signs_shift;
      texture_signs_uint =
          (texture_signs_uint & ~texture_signs_mask) | texture_signs_shifted;
      textures_resolution_scaled |=
          uint32_t(
              texture_cache_->IsActiveTextureResolutionScaled(texture_index))
          << texture_index;
    }
  }
  system_constants_.textures_resolution_scaled = textures_resolution_scaled;

  // Sample count log2 for alpha to mask
  uint32_t sample_count_log2_x =
      rb_surface_info.msaa_samples >= xenos::MsaaSamples::k4X ? 1 : 0;
  uint32_t sample_count_log2_y =
      rb_surface_info.msaa_samples >= xenos::MsaaSamples::k2X ? 1 : 0;
  system_constants_.sample_count_log2[0] = sample_count_log2_x;
  system_constants_.sample_count_log2[1] = sample_count_log2_y;

  // Alpha test reference
  system_constants_.alpha_test_reference = rb_alpha_ref;

  // Alpha to mask
  uint32_t alpha_to_mask = rb_colorcontrol.alpha_to_mask_enable
                               ? (rb_colorcontrol.value >> 24) | (1 << 8)
                               : 0;
  system_constants_.alpha_to_mask = alpha_to_mask;

  // Color exponent bias
  for (uint32_t i = 0; i < 4; ++i) {
    int32_t color_exp_bias = color_infos[i].color_exp_bias;
    // Fixed-point render targets (k_16_16 / k_16_16_16_16) are backed by
    // *_SNORM in the host render targets path. If full-range emulation is
    // requested, remap from -32...32 to -1...1 by dividing the output values
    // by 32.
    if (color_infos[i].color_format ==
        xenos::ColorRenderTargetFormat::k_16_16) {
      if (!render_target_cache_->IsFixedRG16TruncatedToMinus1To1()) {
        color_exp_bias -= 5;
      }
    } else if (color_infos[i].color_format ==
               xenos::ColorRenderTargetFormat::k_16_16_16_16) {
      if (!render_target_cache_->IsFixedRGBA16TruncatedToMinus1To1()) {
        color_exp_bias -= 5;
      }
    }
    auto color_exp_bias_scale = xe::memory::Reinterpret<float>(
        int32_t(0x3F800000 + (color_exp_bias << 23)));
    system_constants_.color_exp_bias[i] = color_exp_bias_scale;
  }

  // Blend constants (used by EDRAM and for host blending)
  system_constants_.edram_blend_constant[0] =
      regs.Get<float>(XE_GPU_REG_RB_BLEND_RED);
  system_constants_.edram_blend_constant[1] =
      regs.Get<float>(XE_GPU_REG_RB_BLEND_GREEN);
  system_constants_.edram_blend_constant[2] =
      regs.Get<float>(XE_GPU_REG_RB_BLEND_BLUE);
  system_constants_.edram_blend_constant[3] =
      regs.Get<float>(XE_GPU_REG_RB_BLEND_ALPHA);

  system_constants_dirty_ = true;
}
#endif  // METAL_SHADER_CONVERTER_AVAILABLE

#define COMMAND_PROCESSOR MetalCommandProcessor
#define XE_GPU_OVERRIDES_EVENT_WRITE_ZPD
#include "../pm4_command_processor_implement.h"
#undef XE_GPU_OVERRIDES_EVENT_WRITE_ZPD
#undef COMMAND_PROCESSOR

}  // namespace metal
}  // namespace gpu
}  // namespace xe
