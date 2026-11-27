/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/gpu/render_target_cache.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "xenia/base/assert.h"
#include "xenia/base/cvar.h"
#include "xenia/base/logging.h"
#include "xenia/base/math.h"
#include "xenia/gpu/draw_util.h"
#include "xenia/gpu/gpu_flags.h"
#include "xenia/gpu/register_file.h"
#include "xenia/gpu/registers.h"
#include "xenia/gpu/xenos.h"

DEFINE_bool(
    depth_clear_retarget_4x, true,
    "When the guest clears depth, stencil or both by drawing one rectangle "
    "that writes a constant (depth 0 or 1 with depth function always, "
    "stencil replaced with function always, no color, no pixel shader, whole "
    "pixels) through a 4x MSAA view of EDRAM tiles that a 1x or 2x depth "
    "render target of the same format, base and pitch owns, write the "
    "constant into the owner instead of transferring ownership to the 4x view "
    "and back.",
    "GPU");
DEFINE_bool(
    depth_transfer_not_equal_test, true,
    "When transferring data between depth render targets, use the \"not "
    "equal\" test to avoid writing rewriting depth via shader depth output if "
    "it's the same as the one currently in the depth buffer in case of round "
    "trips of the data.\n"
    "Settings this to true may make transfer round trips more friendly to "
    "depth compression depending on how the GPU implements it (as arbitrary "
    "depth output may result in it being disabled completely), which is "
    "beneficial to subsequent rendering, while setting this to false may "
    "reduce bandwidth usage during transfers as the previous depth won't need "
    "to be read.",
    "GPU");
DEFINE_bool(
    skip_unchanged_render_target_transfers, false,
    "With host render targets, when a render target takes back an EDRAM range "
    "that another render target used only for reading since it got the range "
    "from it (no depth, stencil or color written, no clear), keep the first "
    "render target's own copy instead of copying the data back. Saves GPU "
    "time when a game alternates two views of the same EDRAM data, such as a "
    "depth buffer used at full resolution and as a 4x MSAA half-resolution "
    "buffer.",
    "GPU");
DEFINE_bool(
    msaa_samples_as_pixels, false,
    "With host render targets, draw MSAA clears and other rectangle lists "
    "(whose result doesn't depend on where the samples are) into the 1x "
    "render target of the same EDRAM, whose pixels are the samples (2x2 per "
    "pixel at 4x MSAA, 1x2 at 2x), scaled by that grid, instead of copying "
    "the data to a host MSAA render target and back. Kept on host MSAA: other "
    "draws (real geometry, which games may place relative to the Xbox 360 "
    "sample positions), and rectangle lists while the host MSAA render target "
    "holds the data. Supported by the Metal SPIRV-Cross path only.",
    "GPU");
// Lossless round trip: 545407F2.
// Lossy round trip with the "greater or equal" test afterwards: 4D530919.
// Lossy round trip with the "equal" test afterwards: 535107F5, 565507EF.
DEFINE_bool(
    depth_float24_round, false,
    "Whether to round to the nearest even, rather than truncating (rounding "
    "towards zero), the depth when converting it to 24-bit floating-point "
    "(20e4) from the host precision (32-bit floating point) when using a host "
    "depth buffer.\n"
    "false:\n"
    " Recommended.\n"
    " The conversion may move the depth values farther away from the camera.\n"
    " Without depth_float24_convert_in_pixel_shader:\n"
    "  The \"greater or equal\" depth test function continues to work fine if "
    "the full host precision depth data is lost, it's still possible to draw "
    "another pass of the same geometry with it.\n"
    "  (See the description of depth_float24_convert_in_pixel_shader for more "
    "information about full precision depth data loss.)\n"
    " With depth_float24_convert_in_pixel_shader:\n"
    "  Faster - the pixel shader for hidden surfaces may still be skipped "
    "(using conservative depth output).\n"
    "true:\n"
    " Only for special cases of issues caused by minor 32-bit floating-point "
    "rounding errors, for instance, when the game tries to draw something at "
    "the camera plane by setting Z of the vertex position to W.\n"
    " The conversion may move the depth values closer or farther.\n"
    " Using the same rounding mode as in the Direct3D 9 reference rasterizer.\n"
    " Without depth_float24_convert_in_pixel_shader:\n"
    "  Not possible to recover from a full host precision depth data loss - in "
    "subsequent passes of rendering the same geometry, half of the samples "
    "will be failing the depth test with the \"greater or equal\" depth test "
    "function.\n"
    " With depth_float24_convert_in_pixel_shader:\n"
    "  Slower - depth rejection before the pixel shader is not possible.\n"
    "When the depth buffer is emulated in software (via the fragment shader "
    "interlock / rasterizer-ordered view), this is ignored, and rounding to "
    "the nearest even is always done.",
    "GPU");
// With MSAA, when converting the depth in pixel shaders, they must run at
// sample frequency - otherwise, if the depth is the same for the entire pixel,
// intersections of polygons cannot be antialiased.
//
// Important usage note: When using this mode, bounds of the fixed-function
// viewport must be converted to and back from float24 too (preferably using
// rounding to the nearest even regardless of whether truncation was requested
// for the values, to reduce the error already caused by truncation rather than
// to amplify it). This ensures that clamping to the viewport bounds, which
// happens after the pixel shader even if it overwrites the resulting depth, is
// never done to a value not representable as float24 (for example, if the
// minimum Z is a number too small to be represented as float24, but not zero,
// it won't be possible to write what should become 0x000000 to the depth
// buffer). Note that this may add some error to the depth values from the
// rasterizer; however, modifying Z in the vertex shader to make interpolated
// depth values would cause clipping to be done to different bounds, which may
// be more undesirable, especially in cases when Z is explicitly set to a value
// like 0 or W (in such cases, the adjusted polygon may go outside 0...W in clip
// space and disappear).
//
// If false, doing the depth test at the host precision, converting to 20e4 to
// support reinterpretation, but keeping track of both the last color (or
// non-20e4 depth) value (let's call it stored_f24) and the last host depth
// value (stored_host) for each EDRAM pixel, reloading the last host depth value
// if stored_f24 == to_f24(stored_host) (otherwise it was overwritten by
// something else, like clearing, or an actually used color buffer; this is
// inexact though, and will incorrectly load pixels that were overwritten by
// something else in the EDRAM, but turned out to have the same value on the
// guest as before - an outdated host-precision value will be loaded in these
// cases instead).
DEFINE_bool(
    depth_float24_convert_in_pixel_shader, false,
    "Whether to convert the depth values to 24-bit floating-point (20e4) from "
    "the host precision (32-bit floating point) directly in the pixel shaders "
    "of guest draws when using a host depth buffer.\n"
    "This prevents visual artifacts (interleaved stripes of parts of surfaces "
    "rendered and parts not rendered, having either the same width in case of "
    "the \"greater or equal\" depth test function, or the former being much "
    "thinner than the latter with the \"equal\" function) if the full host "
    "precision depth data is lost.\n"
    "This issue may happen if the game reloads the depth data previously "
    "evicted from the EDRAM to the RAM back to the EDRAM, but the EDRAM region "
    "that previously contained that depth buffer was overwritten by another "
    "depth buffer, or the game loads it to a different location in the EDRAM "
    "than it was previously placed at, thus Xenia is unable to restore the "
    "depth data with the original precision, and instead falls back to "
    "converting the lower-precision values, so in subsequent rendering passes "
    "for the same geometry, the actual depth values of the surfaces don't "
    "match those stored in the depth buffer anymore.\n"
    "This is a costly option because it makes the GPU unable to use depth "
    "buffer compression, and also with MSAA, forces the pixel shader to run "
    "for every subpixel sample rather than for the entire pixel, making pixel "
    "shading 2 or 4 times heavier depending on the MSAA sample count.\n"
    "The rounding direction is controlled by the depth_float24_round "
    "configuration variable.\n"
    "Note that with depth_float24_round = true, this becomes even more costly "
    "because pixel shaders must be executed regardless of whether the surface "
    "is behind the previously drawn surfaces. With depth_float24_round = "
    "false, conservative depth output is used, however, so depth rejection "
    "before the pixel shader may still work.\n"
    "If sample-rate shading is not supported by the host GPU, the conversion "
    "in the pixel shader is done only when MSAA is not used.\n"
    "When the depth buffer is emulated in software (via the fragment shader "
    "interlock / rasterizer-ordered view), this is ignored because 24-bit "
    "depth is always used directly.",
    "GPU");
DEFINE_bool(
    draw_resolution_scaled_texture_offsets, true,
    "Apply offsets from texture fetch instructions taking resolution scale "
    "into account for render-to-texture, for more correct shadow filtering, "
    "bloom, etc., in some cases.",
    "GPU");
DEFINE_bool(
    gamma_render_target_as_unorm16, true,
    "When the host can't write 8 bits per component pixels with piecewise "
    "linear gamma encoding directly with correct blending, use the 16-bit "
    "unsigned normalized format, if supported, for conceptually correct "
    "8_8_8_8_GAMMA render target format blending in linear color space. "
    "Greatly increases accuracy for this format, but may result in render "
    "target copying costs if the game switches between 8_8_8_8_GAMMA and "
    "8_8_8_8 views for the same EDRAM render target.",
    "GPU");
DEFINE_bool(
    mrt_edram_used_range_clamp_to_min, true,
    "With host render targets, if multiple render targets are bound, estimate "
    "the EDRAM range modified in any of them to be not bigger than the "
    "distance between any two render targets in the EDRAM, rather than "
    "allowing the last one claim the rest of the EDRAM.\n"
    "Has effect primarily on draws without viewport clipping.\n"
    "Setting this to false results in higher accuracy in rare cases, but may "
    "increase the amount of copying that needs to be done sometimes.",
    "GPU");
DEFINE_bool(
    native_2x_msaa, true,
    "Use host 2x MSAA when available. Can be disabled for scalability testing "
    "on host GPU APIs where 2x is not mandatory, in this case, 2 samples of 4x "
    "MSAA will be used instead (with similar or worse quality and higher "
    "memory usage).",
    "GPU");
DEFINE_bool(
    native_stencil_value_output, true,
    "Use pixel shader stencil reference output where available for purposes "
    "like copying between render targets. Can be disabled for scalability "
    "testing, in this case, much more expensive drawing of 8 quads will be "
    "done.",
    "GPU");
DEFINE_bool(
    snorm16_render_target_full_range, true,
    "When the host can only support 16_16 and 16_16_16_16 render targets as "
    "-1...1, remap -32...32 to -1...1 to use the full possible range of "
    "values, at the expense of multiplicative blending correctness.",
    "GPU");
// Enabled by default as the GPU is overall usually the bottleneck when the
// pixel shader interlock render backend implementation is used, anything that
// may improve GPU performance is favorable.
DEFINE_bool(
    execute_unclipped_draw_vs_on_cpu_for_psi_render_backend, true,
    "If execute_unclipped_draw_vs_on_cpu is enabled, execute the vertex shader "
    "for unclipped draws on the CPU even when using the pixel shader interlock "
    "(rasterizer-ordered view) implementation of the render backend on the "
    "host, for which no expensive copying between host render targets is "
    "needed when the ownership of a EDRAM range is changed.\n"
    "If this is enabled, excessive barriers may be eliminated when switching "
    "between different render targets in separate EDRAM locations.",
    "GPU");

namespace xe {
namespace gpu {

void RenderTargetCache::GetPSIColorFormatInfo(
    xenos::ColorRenderTargetFormat format, uint32_t write_mask,
    float& clamp_rgb_low, float& clamp_alpha_low, float& clamp_rgb_high,
    float& clamp_alpha_high, uint32_t& keep_mask_low,
    uint32_t& keep_mask_high) {
  keep_mask_low = keep_mask_high = 0;
  switch (format) {
    case xenos::ColorRenderTargetFormat::k_8_8_8_8:
    case xenos::ColorRenderTargetFormat::k_8_8_8_8_GAMMA: {
      clamp_rgb_low = clamp_alpha_low = 0.0f;
      clamp_rgb_high = clamp_alpha_high = 1.0f;
      for (uint32_t i = 0; i < 4; ++i) {
        if (!(write_mask & (1 << i))) {
          keep_mask_low |= uint32_t(0xFF) << (i * 8);
        }
      }
    } break;
    case xenos::ColorRenderTargetFormat::k_2_10_10_10:
    case xenos::ColorRenderTargetFormat::k_2_10_10_10_AS_10_10_10_10: {
      clamp_rgb_low = clamp_alpha_low = 0.0f;
      clamp_rgb_high = clamp_alpha_high = 1.0f;
      for (uint32_t i = 0; i < 3; ++i) {
        if (!(write_mask & (1 << i))) {
          keep_mask_low |= uint32_t(0x3FF) << (i * 10);
        }
      }
      if (!(write_mask & 0b1000)) {
        keep_mask_low |= uint32_t(3) << 30;
      }
    } break;
    case xenos::ColorRenderTargetFormat::k_2_10_10_10_FLOAT:
    case xenos::ColorRenderTargetFormat::k_2_10_10_10_FLOAT_AS_16_16_16_16: {
      clamp_rgb_low = clamp_alpha_low = 0.0f;
      clamp_rgb_high = 31.875f;
      clamp_alpha_high = 1.0f;
      for (uint32_t i = 0; i < 3; ++i) {
        if (!(write_mask & (1 << i))) {
          keep_mask_low |= uint32_t(0x3FF) << (i * 10);
        }
      }
      if (!(write_mask & 0b1000)) {
        keep_mask_low |= uint32_t(3) << 30;
      }
    } break;
    case xenos::ColorRenderTargetFormat::k_16_16:
    case xenos::ColorRenderTargetFormat::k_16_16_16_16:
      // Alpha clamping affects blending source, so it's non-zero for alpha for
      // k_16_16 (the render target is fixed-point). There's one deviation from
      // how Direct3D 11.3 functional specification defines SNorm conversion
      // (NaN should be 0, not the lowest negative number), and that needs to be
      // handled separately.
      clamp_rgb_low = clamp_alpha_low = -32.0f;
      clamp_rgb_high = clamp_alpha_high = 32.0f;
      if (!(write_mask & 0b0001)) {
        keep_mask_low |= 0xFFFFu;
      }
      if (!(write_mask & 0b0010)) {
        keep_mask_low |= 0xFFFF0000u;
      }
      if (format == xenos::ColorRenderTargetFormat::k_16_16_16_16) {
        if (!(write_mask & 0b0100)) {
          keep_mask_high |= 0xFFFFu;
        }
        if (!(write_mask & 0b1000)) {
          keep_mask_high |= 0xFFFF0000u;
        }
      } else {
        write_mask &= 0b0011;
      }
      break;
    case xenos::ColorRenderTargetFormat::k_16_16_FLOAT:
    case xenos::ColorRenderTargetFormat::k_16_16_16_16_FLOAT:
      // No NaNs on the Xbox 360 GPU, though can't use the extended range with
      // Direct3D and Vulkan conversions.
      // TODO(Triang3l): Use the extended-range encoding in all implementations.
      clamp_rgb_low = clamp_alpha_low = -65504.0f;
      clamp_rgb_high = clamp_alpha_high = 65504.0f;
      if (!(write_mask & 0b0001)) {
        keep_mask_low |= 0xFFFFu;
      }
      if (!(write_mask & 0b0010)) {
        keep_mask_low |= 0xFFFF0000u;
      }
      if (format == xenos::ColorRenderTargetFormat::k_16_16_16_16_FLOAT) {
        if (!(write_mask & 0b0100)) {
          keep_mask_high |= 0xFFFFu;
        }
        if (!(write_mask & 0b1000)) {
          keep_mask_high |= 0xFFFF0000u;
        }
      } else {
        write_mask &= 0b0011;
      }
      break;
    case xenos::ColorRenderTargetFormat::k_32_FLOAT:
      // No clamping - let min/max always pick the original value.
      clamp_rgb_low = clamp_alpha_low = clamp_rgb_high = clamp_alpha_high =
          std::nanf("");
      write_mask &= 0b0001;
      if (!(write_mask & 0b0001)) {
        keep_mask_low = ~uint32_t(0);
      }
      break;
    case xenos::ColorRenderTargetFormat::k_32_32_FLOAT:
      // No clamping - let min/max always pick the original value.
      clamp_rgb_low = clamp_alpha_low = clamp_rgb_high = clamp_alpha_high =
          std::nanf("");
      write_mask &= 0b0011;
      if (!(write_mask & 0b0001)) {
        keep_mask_low = ~uint32_t(0);
      }
      if (!(write_mask & 0b0010)) {
        keep_mask_high = ~uint32_t(0);
      }
      break;
    default:
      assert_unhandled_case(format);
      // Disable invalid render targets.
      write_mask = 0;
      break;
  }
  // Special case handled in the shaders for empty write mask to completely skip
  // a disabled render target: all keep bits are set.
  if (!write_mask) {
    keep_mask_low = keep_mask_high = ~uint32_t(0);
  }
}

uint32_t RenderTargetCache::Transfer::GetRangeRectangles(
    uint32_t start_tiles, uint32_t end_tiles, uint32_t base_tiles,
    uint32_t pitch_tiles, xenos::MsaaSamples msaa_samples, bool is_64bpp,
    Rectangle* rectangles_out, const Rectangle* cutout) {
  // EDRAM addressing wrapping must be handled by doing GetRangeRectangles for
  // two clamped ranges, in this case start_tiles == end_tiles will also
  // unambiguously mean an empty range rather than the entire EDRAM.
  assert_true(start_tiles < xenos::kEdramTileCount);
  assert_true(end_tiles <= xenos::kEdramTileCount);
  assert_true(start_tiles <= end_tiles);
  // If start_tiles < base_tiles, this is the tail after EDRAM addressing
  // wrapping.
  assert_true(start_tiles >= base_tiles || end_tiles <= base_tiles);
  assert_not_zero(pitch_tiles);
  if (start_tiles == end_tiles) {
    return 0;
  }
  uint32_t tile_width =
      xenos::kEdramTileWidthSamples >>
      (uint32_t(msaa_samples >= xenos::MsaaSamples::k4X) + uint32_t(is_64bpp));
  uint32_t tile_height = xenos::kEdramTileHeightSamples >>
                         uint32_t(msaa_samples >= xenos::MsaaSamples::k2X);
  // If the first and / or the last rows have the same X spans as the middle
  // part, merge them with it.
  uint32_t rectangle_count = 0;
  // If start_tiles < base_tiles, this is the tail after EDRAM addressing
  // wrapping.
  uint32_t local_offset = start_tiles < base_tiles ? xenos::kEdramTileCount : 0;
  uint32_t local_start = local_offset + start_tiles - base_tiles;
  uint32_t local_end = local_offset + end_tiles - base_tiles;
  // Inclusive.
  uint32_t rows_start = local_start / pitch_tiles;
  // Exclusive.
  uint32_t rows_end = (local_end + (pitch_tiles - 1)) / pitch_tiles;
  uint32_t row_first_start = local_start - rows_start * pitch_tiles;
  uint32_t row_last_end = pitch_tiles - (rows_end * pitch_tiles - local_end);
  uint32_t rows = rows_end - rows_start;
  if (rows == 1 || row_first_start) {
    Rectangle rectangle_first;
    rectangle_first.x_pixels = row_first_start * tile_width;
    rectangle_first.y_pixels = rows_start * tile_height;
    rectangle_first.width_pixels =
        ((rows == 1 ? row_last_end : pitch_tiles) - row_first_start) *
        tile_width;
    rectangle_first.height_pixels = tile_height;
    rectangle_count += AddRectangle(
        rectangle_first,
        rectangles_out ? rectangles_out + rectangle_count : nullptr, cutout);
    if (rows == 1) {
      return rectangle_count;
    }
  }
  uint32_t mid_rows_start = rows_start + 1;
  uint32_t mid_rows = rows - 2;
  if (!row_first_start) {
    --mid_rows_start;
    ++mid_rows;
  }
  if (row_last_end == pitch_tiles) {
    ++mid_rows;
  }
  if (mid_rows) {
    Rectangle rectangle_mid;
    rectangle_mid.x_pixels = 0;
    rectangle_mid.y_pixels = mid_rows_start * tile_height;
    rectangle_mid.width_pixels = pitch_tiles * tile_width;
    rectangle_mid.height_pixels = mid_rows * tile_height;
    rectangle_count += AddRectangle(
        rectangle_mid,
        rectangles_out ? rectangles_out + rectangle_count : nullptr, cutout);
  }
  if (row_last_end != pitch_tiles) {
    Rectangle rectangle_last;
    rectangle_last.x_pixels = 0;
    rectangle_last.y_pixels = (rows_end - 1) * tile_height;
    rectangle_last.width_pixels = row_last_end * tile_width;
    rectangle_last.height_pixels = tile_height;
    rectangle_count += AddRectangle(
        rectangle_last,
        rectangles_out ? rectangles_out + rectangle_count : nullptr, cutout);
  }
  assert_true(rectangle_count <= (cutout ? kMaxRectanglesWithCutout
                                         : kMaxRectanglesWithoutCutout));
  return rectangle_count;
}

uint32_t RenderTargetCache::Transfer::AddRectangle(const Rectangle& rectangle,
                                                   Rectangle* rectangles_out,
                                                   const Rectangle* cutout) {
  uint32_t rectangle_right = rectangle.x_pixels + rectangle.width_pixels;
  uint32_t rectangle_bottom = rectangle.y_pixels + rectangle.height_pixels;
  // If nothing to cut out (no region specified, or no intersection - if the
  // cutout region is in the middle on Y, but completely to the left / right on
  // X, don't split), add the whole rectangle.
  if (!cutout || !cutout->width_pixels || !cutout->height_pixels ||
      cutout->x_pixels >= rectangle_right ||
      cutout->x_pixels + cutout->width_pixels <= rectangle.x_pixels ||
      cutout->y_pixels >= rectangle_bottom ||
      cutout->y_pixels + cutout->height_pixels <= rectangle.y_pixels) {
    if (rectangles_out) {
      rectangles_out[0] = rectangle;
    }
    return 1;
  }
  uint32_t rectangle_count = 0;
  uint32_t cutout_right = cutout->x_pixels + cutout->width_pixels;
  uint32_t cutout_bottom = cutout->y_pixels + cutout->height_pixels;
  // Upper part after cutout.
  if (cutout->y_pixels > rectangle.y_pixels) {
    // The completely outside case has already been checked.
    assert_true(cutout->y_pixels < rectangle_bottom);
    if (rectangles_out) {
      Rectangle& rectangle_upper = rectangles_out[rectangle_count];
      rectangle_upper.x_pixels = rectangle.x_pixels;
      rectangle_upper.y_pixels = rectangle.y_pixels;
      rectangle_upper.width_pixels = rectangle.width_pixels;
      // cutout->y_pixels is already known to be < rectangle_bottom, no need for
      // min(cutout->y_pixels - rectangle.y_pixels, rectangle.height_pixels).
      rectangle_upper.height_pixels = cutout->y_pixels - rectangle.y_pixels;
    }
    ++rectangle_count;
  }
  // Middle part after cutout.
  uint32_t middle_top = std::max(cutout->y_pixels, rectangle.y_pixels);
  uint32_t middle_height =
      std::min(cutout_bottom, rectangle_bottom) - middle_top;
  // Middle left.
  if (cutout->x_pixels > rectangle.x_pixels) {
    assert_true(cutout->x_pixels < rectangle_right);
    if (rectangles_out) {
      Rectangle& rectangle_middle_left = rectangles_out[rectangle_count];
      rectangle_middle_left.x_pixels = rectangle.x_pixels;
      rectangle_middle_left.y_pixels = middle_top;
      rectangle_middle_left.width_pixels =
          cutout->x_pixels - rectangle.x_pixels;
      rectangle_middle_left.height_pixels = middle_height;
    }
    ++rectangle_count;
  }
  // Middle right.
  if (cutout_right < rectangle_right) {
    assert_true(cutout_right > rectangle.x_pixels);
    if (rectangles_out) {
      Rectangle& rectangle_middle_right = rectangles_out[rectangle_count];
      rectangle_middle_right.x_pixels = cutout_right;
      rectangle_middle_right.y_pixels = middle_top;
      rectangle_middle_right.width_pixels = rectangle_right - cutout_right;
      rectangle_middle_right.height_pixels = middle_height;
    }
    ++rectangle_count;
  }
  // Lower part after cutout.
  if (cutout_bottom < rectangle_bottom) {
    assert_true(cutout_bottom > rectangle.y_pixels);
    if (rectangles_out) {
      Rectangle& rectangle_upper = rectangles_out[rectangle_count];
      rectangle_upper.x_pixels = rectangle.x_pixels;
      rectangle_upper.y_pixels = cutout_bottom;
      rectangle_upper.width_pixels = rectangle.width_pixels;
      rectangle_upper.height_pixels = rectangle_bottom - cutout_bottom;
    }
    ++rectangle_count;
  }
  assert_true(rectangle_count <= kMaxCutoutBorderRectangles);
  return rectangle_count;
}

RenderTargetCache::~RenderTargetCache() { ShutdownCommon(); }

void RenderTargetCache::InitializeCommon() {
  assert_true(ownership_ranges_.empty());
  ownership_ranges_.emplace(
      std::piecewise_construct, std::forward_as_tuple(uint32_t(0)),
      std::forward_as_tuple(xenos::kEdramTileCount, RenderTargetKey(),
                            RenderTargetKey(), RenderTargetKey()));
}

void RenderTargetCache::DestroyAllRenderTargets(bool shutting_down) {
  ownership_ranges_.clear();
  if (!shutting_down) {
    ownership_ranges_.emplace(
        std::piecewise_construct, std::forward_as_tuple(uint32_t(0)),
        std::forward_as_tuple(xenos::kEdramTileCount, RenderTargetKey(),
                              RenderTargetKey(), RenderTargetKey()));
  }

  for (const auto& render_target_pair : render_targets_) {
    if (render_target_pair.second) {
      delete render_target_pair.second;
    }
  }
  render_targets_.clear();
}

void RenderTargetCache::ShutdownCommon() { DestroyAllRenderTargets(true); }

void RenderTargetCache::ClearCache() {
  depth_clear_retarget_last_ = RenderTargetKey();
  depth_clear_retarget_stay_.clear();
  // Keep only render targets currently owning any EDRAM data.
  if (!render_targets_.empty()) {
    std::unordered_set<RenderTargetKey, RenderTargetKey::Hasher>
        used_render_targets;
    for (const auto& ownership_range_pair : ownership_ranges_) {
      const OwnershipRange& ownership_range = ownership_range_pair.second;
      if (!ownership_range.render_target.IsEmpty()) {
        used_render_targets.emplace(ownership_range.render_target);
      }
      if (!ownership_range.host_depth_render_target_unorm24.IsEmpty()) {
        used_render_targets.emplace(
            ownership_range.host_depth_render_target_unorm24);
      }
      if (!ownership_range.host_depth_render_target_float24.IsEmpty()) {
        used_render_targets.emplace(
            ownership_range.host_depth_render_target_float24);
      }
      // Its contents are what makes a later transfer back unneeded.
      if (!ownership_range.intact_render_target.IsEmpty()) {
        used_render_targets.emplace(ownership_range.intact_render_target);
      }
    }
    if (render_targets_.size() != used_render_targets.size()) {
      typename decltype(render_targets_)::iterator it_next;
      for (auto it = render_targets_.begin(); it != render_targets_.end();
           it = it_next) {
        it_next = std::next(it);
        if (!it->second) {
          render_targets_.erase(it);
          continue;
        }
        if (used_render_targets.find(it->second->key()) ==
            used_render_targets.end()) {
          delete it->second;
          render_targets_.erase(it);
        }
      }
    }
  }
}

void RenderTargetCache::BeginFrame() { ResetAccumulatedRenderTargets(); }

bool RenderTargetCache::IsEdramTileOwnedBy(uint32_t tile,
                                           RenderTargetKey key) const {
  auto it = ownership_ranges_.upper_bound(tile);
  if (it == ownership_ranges_.cbegin()) {
    return false;
  }
  --it;
  return it->second.end_tiles > tile && it->second.render_target == key;
}

bool RenderTargetCache::Update(bool is_rasterization_done,
                               reg::RB_DEPTHCONTROL normalized_depth_control,
                               uint32_t normalized_color_mask,
                               const Shader& vertex_shader) {
  const RegisterFile& regs = register_file();
  bool interlock_barrier_only = GetPath() == Path::kPixelShaderInterlock;
  last_update_height_used_ = 0;
  last_update_samples_as_pixels_ = xenos::MsaaSamples::k1X;
  const bool depth_clear_retarget_candidate = depth_clear_retarget_candidate_;
  depth_clear_retarget_candidate_ = false;
  last_update_depth_clear_retargeted_ = false;

  auto rb_surface_info = regs.Get<reg::RB_SURFACE_INFO>();
  xenos::MsaaSamples msaa_samples = rb_surface_info.msaa_samples;
  assert_true(msaa_samples <= xenos::MsaaSamples::k4X);
  if (msaa_samples > xenos::MsaaSamples::k4X) {
    // Safety check because a lot of code assumes up to 4x.
    assert_always();
    XELOGE("{}x MSAA requested by the guest, Xenos only supports up to 4x",
           uint32_t(1) << uint32_t(msaa_samples));
    return false;
  }
  uint32_t msaa_samples_x_log2 =
      uint32_t(msaa_samples >= xenos::MsaaSamples::k4X);
  uint32_t pitch_pixels = rb_surface_info.surface_pitch;
  // surface_pitch 0 should be handled in disabling rasterization (hopefully
  // it's safe to assume that).
  assert_true(pitch_pixels || !is_rasterization_done);
  if (!pitch_pixels) {
    is_rasterization_done = false;
  } else if (pitch_pixels > xenos::kTexture2DCubeMaxWidthHeight) {
    XELOGE(
        "Surface pitch {} larger than the maximum texture width {} specified "
        "by the guest",
        pitch_pixels, xenos::kTexture2DCubeMaxWidthHeight);
    return false;
  }
  uint32_t pitch_tiles_at_32bpp = ((pitch_pixels << msaa_samples_x_log2) +
                                   (xenos::kEdramTileWidthSamples - 1)) /
                                  xenos::kEdramTileWidthSamples;
  if (!interlock_barrier_only) {
    uint32_t pitch_pixels_tile_aligned_scaled =
        pitch_tiles_at_32bpp *
        (xenos::kEdramTileWidthSamples >> msaa_samples_x_log2) *
        draw_resolution_scale_x();
    uint32_t max_render_target_width = GetMaxRenderTargetWidth();
    if (pitch_pixels_tile_aligned_scaled > max_render_target_width) {
      // TODO(Triang3l): If really needed for some game on some device, clamp
      // the pitch and generate multiple ranges (each for every row of tiles)
      // with gaps for padding. Very few PowerVR GPUs have 4096, not 8192, as
      // the limit, though with 8192 (on Mali) the actual limit for Xenia is
      // 8160 because tile padding is stored - but 8192 should be extremely rare
      // anyway.
      XELOGE(
          "Surface pitch aligned to EDRAM tiles and resolution-scaled {} "
          "larger than the maximum host render target width {}",
          pitch_pixels_tile_aligned_scaled, max_render_target_width);
      return false;
    }
  }

  // Get used render targets.
  // [0] is depth / stencil where relevant, [1...4] is color.
  // Depth / stencil testing / writing is before color in the pipeline.
  uint32_t depth_and_color_rts_used_bits = 0;
  // depth_and_color_rts_used_bits -> EDRAM base.
  uint32_t edram_bases[1 + xenos::kMaxColorRenderTargets];
  uint32_t resource_formats[1 + xenos::kMaxColorRenderTargets];
  uint32_t rts_are_64bpp = 0;
  if (is_rasterization_done) {
    if (normalized_depth_control.z_enable ||
        normalized_depth_control.stencil_enable) {
      depth_and_color_rts_used_bits |= 1;
      auto rb_depth_info = regs.Get<reg::RB_DEPTH_INFO>();
      edram_bases[0] = rb_depth_info.depth_base;
      // With pixel shader interlock, always the same addressing disregarding
      // the format.
      resource_formats[0] =
          interlock_barrier_only ? 0 : uint32_t(rb_depth_info.depth_format);
    }
    for (uint32_t i = 0; i < xenos::kMaxColorRenderTargets; ++i) {
      if (!(normalized_color_mask & (uint32_t(0b1111) << (4 * i)))) {
        continue;
      }
      auto color_info = regs.Get<reg::RB_COLOR_INFO>(
          reg::RB_COLOR_INFO::rt_register_indices[i]);
      uint32_t rt_bit_index = 1 + i;
      depth_and_color_rts_used_bits |= uint32_t(1) << rt_bit_index;
      edram_bases[rt_bit_index] = color_info.color_base;
      xenos::ColorRenderTargetFormat color_format =
          regs.Get<reg::RB_COLOR_INFO>(
                  reg::RB_COLOR_INFO::rt_register_indices[i])
              .color_format;
      bool is_64bpp = xenos::IsColorRenderTargetFormat64bpp(color_format);
      if (is_64bpp) {
        rts_are_64bpp |= uint32_t(1) << rt_bit_index;
      }
      xenos::ColorRenderTargetFormat color_resource_format;
      if (interlock_barrier_only) {
        // Only changes in mapping between coordinates and addresses are
        // interesting (along with access overlap between draw calls), thus only
        // pixel size is relevant.
        color_resource_format =
            is_64bpp ? xenos::ColorRenderTargetFormat::k_16_16_16_16
                     : xenos::ColorRenderTargetFormat::k_8_8_8_8;
      } else {
        color_resource_format =
            GetColorResourceFormat(xenos::GetStorageColorFormat(color_format));
      }
      resource_formats[rt_bit_index] = uint32_t(color_resource_format);
    }
  }

  uint32_t rts_remaining;
  uint32_t rt_index;

  // Eliminate other bound render targets if their EDRAM base conflicts with
  // another render target - it's an error in most host implementations to bind
  // the same render target into multiple slots, also the behavior would be
  // unpredictable if that happens.
  // Depth is considered the least important as it's earlier in the pipeline
  // (issues caused by color and depth render target collisions haven't been
  // found yet), but render targets with smaller index are considered more
  // important - specifically, because of the usage in the lighting pass of
  // 4D5307E6, which can be checked in the vertical look calibration sequence in
  // the beginning of the game: if render target 0 is removed in favor of 1, the
  // characters and the world will be too dark, like fully in shadow -
  // especially prominent on the helmet. This happens because the shader picks
  // between two render targets to write dynamically (though with a static, bool
  // constant condition), but all other state is set up in a way that implies
  // the same render target being bound twice. On Direct3D 9, if you don't write
  // to a color pixel shader output on the control flow that was taken, the
  // render target will not be written to. However, this has been relaxed in
  // Direct3D 10, where if the shader declares an output, it's assumed to be
  // always written (or with an undefined value otherwise).
  rts_remaining = depth_and_color_rts_used_bits & ~(uint32_t(1));
  while (xe::bit_scan_forward(rts_remaining, &rt_index)) {
    rts_remaining &= ~(uint32_t(1) << rt_index);
    uint32_t edram_base = edram_bases[rt_index];
    uint32_t rts_other_remaining =
        depth_and_color_rts_used_bits &
        (~((uint32_t(1) << (rt_index + 1)) - 1) | uint32_t(1));
    uint32_t rt_other_index;
    while (xe::bit_scan_forward(rts_other_remaining, &rt_other_index)) {
      rts_other_remaining &= ~(uint32_t(1) << rt_other_index);
      if (edram_bases[rt_other_index] == edram_base) {
        depth_and_color_rts_used_bits &= ~(uint32_t(1) << rt_other_index);
      }
    }
  }

  // msaa_samples_as_pixels: for a rectangle list (clears and screen-space
  // passes: whole pixels are covered alike wherever the samples are), the
  // render targets of an MSAA surface are the 1x render targets of the same
  // EDRAM, whose pixels are its samples - unless the host MSAA render target
  // holds that EDRAM now, which then keeps it (no transfer either way).
  // Other draws keep host MSAA and its sample positions, which games may
  // depend on: UE3 shifts its 2x passes so that Xenos sample 0 is at the 1x
  // pixel centre, and depth restored from it must match the 1x passes
  // (Batman: Arkham Asylum's shading broke when its 2x passes were drawn to
  // samples as pixels).
  xenos::MsaaSamples storage_msaa_samples = msaa_samples;
  uint32_t first_rt_index;
  if (msaa_samples != xenos::MsaaSamples::k1X && !interlock_barrier_only &&
      cvars::msaa_samples_as_pixels && SupportsMsaaSamplesAsPixels() &&
      regs.Get<reg::VGT_DRAW_INITIATOR>().prim_type ==
          xenos::PrimitiveType::kRectangleList &&
      xe::bit_scan_forward(depth_and_color_rts_used_bits, &first_rt_index)) {
    RenderTargetKey msaa_key;
    msaa_key.base_tiles = edram_bases[first_rt_index];
    msaa_key.pitch_tiles_at_32bpp = pitch_tiles_at_32bpp;
    msaa_key.msaa_samples = msaa_samples;
    msaa_key.is_depth = first_rt_index == 0;
    msaa_key.resource_format = resource_formats[first_rt_index];
    if (!IsEdramTileOwnedBy(msaa_key.base_tiles, msaa_key)) {
      storage_msaa_samples = xenos::MsaaSamples::k1X;
      last_update_samples_as_pixels_ = msaa_samples;
    }
  }

  // Clear ownership transfers before adding any.
  if (!interlock_barrier_only) {
    for (size_t i = 0; i < xe::countof(last_update_transfers_); ++i) {
      last_update_transfers_[i].clear();
    }
  }

  if (!depth_and_color_rts_used_bits) {
    // Nothing to bind, don't waste time on things like memexport-only draws -
    // just check if old bindings can still be used.
    std::memset(last_update_used_render_targets_, 0,
                sizeof(last_update_used_render_targets_));
    if (are_accumulated_render_targets_valid_) {
      for (size_t i = 0;
           i < xe::countof(last_update_accumulated_render_targets_); ++i) {
        const RenderTarget* render_target =
            last_update_accumulated_render_targets_[i];
        if (!render_target) {
          continue;
        }
        RenderTargetKey rt_key = render_target->key();
        if (rt_key.pitch_tiles_at_32bpp != pitch_tiles_at_32bpp ||
            rt_key.msaa_samples != storage_msaa_samples) {
          are_accumulated_render_targets_valid_ = false;
          break;
        }
      }
    }
    if (!are_accumulated_render_targets_valid_) {
      std::memset(last_update_accumulated_render_targets_, 0,
                  sizeof(last_update_accumulated_render_targets_));
    }
    return true;
  }

  // Estimate height used by render targets (for color for writes, for depth /
  // stencil for both reads and writes) from various sources.
  uint32_t height_used = std::min(
      GetRenderTargetHeight(pitch_tiles_at_32bpp, msaa_samples),
      draw_extent_estimator_.EstimateMaxY(
          interlock_barrier_only
              ? cvars::execute_unclipped_draw_vs_on_cpu_for_psi_render_backend
              : true,
          vertex_shader));
  last_update_height_used_ = height_used;

  // A depth clear through a 4x view of tiles a 1x or 2x depth render target
  // owns goes to the owner, before anything of this update exists: no render
  // target is created, no ownership changes, and the render targets stay as
  // the draw before left them.
  if (!depth_clear_retarget_last_.IsEmpty() &&
      (depth_and_color_rts_used_bits & 1) && !interlock_barrier_only) {
    // The first draw with depth after a retargeted clear: if it is on the 4x
    // view, that view takes the tiles anyway and the retarget saved nothing,
    // so clears through it are drawn as usual from now on. Either way is
    // exact; this only picks the one with less work.
    RenderTargetKey depth_key;
    depth_key.base_tiles = edram_bases[0];
    depth_key.pitch_tiles_at_32bpp = pitch_tiles_at_32bpp;
    depth_key.msaa_samples = msaa_samples;
    depth_key.is_depth = 1;
    depth_key.resource_format = resource_formats[0];
    if (depth_key == depth_clear_retarget_last_ &&
        std::find(depth_clear_retarget_stay_.cbegin(),
                  depth_clear_retarget_stay_.cend(),
                  depth_key) == depth_clear_retarget_stay_.cend()) {
      depth_clear_retarget_stay_.push_back(depth_key);
    }
    depth_clear_retarget_last_ = RenderTargetKey();
  }
  if (cvars::depth_clear_retarget_4x && depth_clear_retarget_candidate &&
      !interlock_barrier_only && depth_and_color_rts_used_bits == 1 &&
      msaa_samples == xenos::MsaaSamples::k4X) {
    RenderTargetKey draw_key;
    draw_key.base_tiles = edram_bases[0];
    draw_key.pitch_tiles_at_32bpp = pitch_tiles_at_32bpp;
    draw_key.msaa_samples = msaa_samples;
    draw_key.is_depth = 1;
    draw_key.resource_format = resource_formats[0];
    const uint32_t length_tiles =
        ((height_used << 1) + (xenos::kEdramTileHeightSamples - 1)) /
        xenos::kEdramTileHeightSamples * pitch_tiles_at_32bpp;
    DepthClearRetarget retarget;
    if (std::find(depth_clear_retarget_stay_.cbegin(),
                  depth_clear_retarget_stay_.cend(),
                  draw_key) == depth_clear_retarget_stay_.cend() &&
        GetDepthClearRetarget(normalized_depth_control, draw_key, length_tiles,
                              height_used, retarget) &&
        PerformDepthClearRetarget(retarget)) {
      // The owner's contents changed, so the copies other render targets
      // hold of the tiles are stale (all of the view's tiles, more than the
      // rectangle touches, which only costs a transfer that could have been
      // skipped).
      const uint32_t stale_end = draw_key.base_tiles + length_tiles;
      auto it = ownership_ranges_.upper_bound(draw_key.base_tiles);
      if (it != ownership_ranges_.begin()) {
        --it;
      }
      while (it != ownership_ranges_.end() && it->first < stale_end) {
        if (it->second.end_tiles > draw_key.base_tiles) {
          it->second.intact_render_target = RenderTargetKey();
        }
        // Keep adjacent ranges distinct.
        if (it != ownership_ranges_.begin()) {
          auto it_prev = std::prev(it);
          if (it_prev->second.end_tiles == it->first &&
              it_prev->second.AreOwnersSame(it->second)) {
            it_prev->second.end_tiles = it->second.end_tiles;
            it = ownership_ranges_.erase(it);
            continue;
          }
        }
        ++it;
      }
      last_update_samples_as_pixels_ = xenos::MsaaSamples::k1X;
      last_update_depth_clear_retargeted_ = true;
      depth_clear_retarget_last_ = draw_key;
      return true;
    }
  }

  // Sorted by EDRAM base and then by index in the pipeline - for simplicity,
  // treat render targets placed closer to the end of the EDRAM as truncating
  // the previous one (and in case multiple render targets are placed at the
  // same EDRAM base, though normally this shouldn't happen, treat the color
  // ones as more important than the depth one, which may be not needed and just
  // a leftover if the draw, for instance, has depth / stencil happening to be
  // always passing and never writing with the current state, and also because
  // depth testing has to happen before the color is written). Overall it's
  // normal for estimated EDRAM ranges of render targets to intersect if drawing
  // without a viewport (as there's nothing to clamp the estimated height) and
  // multiple render targets are bound.
  std::pair<uint32_t, uint32_t>
      edram_bases_sorted[1 + xenos::kMaxColorRenderTargets];
  uint32_t edram_bases_sorted_count = 0;
  rts_remaining = depth_and_color_rts_used_bits;
  while (xe::bit_scan_forward(rts_remaining, &rt_index)) {
    rts_remaining &= ~(uint32_t(1) << rt_index);
    edram_bases_sorted[edram_bases_sorted_count++] =
        std::make_pair(edram_bases[rt_index], rt_index);
  }
  std::sort(edram_bases_sorted, edram_bases_sorted + edram_bases_sorted_count);
  // "As if it was 64bpp" (contribution of 32bpp render targets multiplied by 2,
  // and clamping for 32bpp render targets divides this by 2) because 32bpp
  // render targets can be combined with twice as long 64bpp render targets. An
  // example is the 4541099D menu background (1-sample 1152x720, or 1200x720
  // after rounding to tiles, with a 32bpp depth buffer at 0 requiring 675
  // tiles, and a 64bpp color buffer at 675 requiring 1350 tiles, but the
  // smallest distance between two render target bases is 675 tiles).
  uint32_t rt_max_distance_tiles_at_64bpp = xenos::kEdramTileCount * 2;
  if (cvars::mrt_edram_used_range_clamp_to_min &&
      edram_bases_sorted_count >= 2) {
    for (uint32_t i = 1; i < edram_bases_sorted_count; ++i) {
      const std::pair<uint32_t, uint32_t>& rt_base_prev =
          edram_bases_sorted[i - 1];
      rt_max_distance_tiles_at_64bpp =
          std::min(rt_max_distance_tiles_at_64bpp,
                   (edram_bases_sorted[i].first - rt_base_prev.first)
                       << (((rts_are_64bpp >> rt_base_prev.second) & 1) ^ 1));
    }
    // Clamp to the distance from the last render target to the first with
    // EDRAM addressing wrapping.
    const std::pair<uint32_t, uint32_t>& rt_base_last =
        edram_bases_sorted[edram_bases_sorted_count - 1];
    rt_max_distance_tiles_at_64bpp =
        std::min(rt_max_distance_tiles_at_64bpp,
                 (xenos::kEdramTileCount + edram_bases_sorted[0].first -
                  rt_base_last.first)
                     << (((rts_are_64bpp >> rt_base_last.second) & 1) ^ 1));
  }

  // Make sure all the needed render targets are created, and gather lengths of
  // ranges used by each render target.
  RenderTargetKey rt_keys[1 + xenos::kMaxColorRenderTargets];
  RenderTarget* rts[1 + xenos::kMaxColorRenderTargets];
  uint32_t rt_lengths_tiles[1 + xenos::kMaxColorRenderTargets];
  uint32_t length_used_tiles_at_32bpp =
      ((height_used << uint32_t(msaa_samples >= xenos::MsaaSamples::k2X)) +
       (xenos::kEdramTileHeightSamples - 1)) /
      xenos::kEdramTileHeightSamples * pitch_tiles_at_32bpp;
  for (uint32_t i = 0; i < edram_bases_sorted_count; ++i) {
    const std::pair<uint32_t, uint32_t>& rt_base_index = edram_bases_sorted[i];
    uint32_t rt_base = rt_base_index.first;
    uint32_t rt_bit_index = rt_base_index.second;
    RenderTargetKey& rt_key = rt_keys[rt_bit_index];
    rt_key.base_tiles = rt_base;
    rt_key.pitch_tiles_at_32bpp = pitch_tiles_at_32bpp;
    rt_key.msaa_samples = storage_msaa_samples;
    rt_key.is_depth = rt_bit_index == 0;
    rt_key.resource_format = resource_formats[rt_bit_index];
    if (!interlock_barrier_only) {
      RenderTarget* render_target = GetOrCreateRenderTarget(rt_key);
      if (!render_target) {
        return false;
      }
      rts[rt_bit_index] = render_target;
    }
    uint32_t rt_is_64bpp = (rts_are_64bpp >> rt_bit_index) & 1;
    // The last render target can occupy the EDRAM until the base of the first
    // render target (itself in case of 1 render target) with EDRAM addressing
    // wrapping.
    rt_lengths_tiles[i] = std::min(
        std::min(length_used_tiles_at_32bpp << rt_is_64bpp,
                 rt_max_distance_tiles_at_64bpp >> (rt_is_64bpp ^ 1)),
        ((i + 1 < edram_bases_sorted_count)
             ? edram_bases_sorted[i + 1].first
             : (xenos::kEdramTileCount + edram_bases_sorted[0].first)) -
            rt_base);
  }

  if (interlock_barrier_only) {
    // Because a full pixel shader interlock barrier may clear the ownership map
    // (since it flushes all previous writes, and there's no need for another
    // barrier if an overlap is encountered later between pre-barrier and
    // post-barrier usages), check if any overlap requiring a barrier happens,
    // and then insert the barrier if needed.
    bool interlock_barrier_needed = false;
    for (uint32_t i = 0; i < edram_bases_sorted_count; ++i) {
      const std::pair<uint32_t, uint32_t>& rt_base_index =
          edram_bases_sorted[i];
      if (WouldOwnershipChangeRequireTransfers(rt_keys[rt_base_index.second], 0,
                                               rt_lengths_tiles[i])) {
        interlock_barrier_needed = true;
        break;
      }
    }
    if (interlock_barrier_needed) {
      RequestPixelShaderInterlockBarrier();
    }
  }

  // From now on ownership transfers should succeed for simplicity and
  // consistency, even if they fail in the implementation (just ignore that and
  // draw with whatever contents currently are in the render target in this
  // case).

  // A bound color render target has a non-zero write mask, so it is written.
  // Depth / stencil may be bound only for testing.
  bool depth_written = normalized_depth_control.z_enable &&
                       normalized_depth_control.z_write_enable;
  if (normalized_depth_control.stencil_enable && !depth_written) {
    auto writes_stencil = [](uint32_t write_mask, xenos::StencilOp fail,
                             xenos::StencilOp zpass, xenos::StencilOp zfail) {
      return write_mask && (fail != xenos::StencilOp::kKeep ||
                            zpass != xenos::StencilOp::kKeep ||
                            zfail != xenos::StencilOp::kKeep);
    };
    depth_written =
        writes_stencil(regs.Get<reg::RB_STENCILREFMASK>().stencilwritemask,
                       normalized_depth_control.stencilfail,
                       normalized_depth_control.stencilzpass,
                       normalized_depth_control.stencilzfail) ||
        (normalized_depth_control.backface_enable &&
         writes_stencil(
             regs.Get<reg::RB_STENCILREFMASK>(XE_GPU_REG_RB_STENCILREFMASK_BF)
                 .stencilwritemask,
             normalized_depth_control.stencilfail_bf,
             normalized_depth_control.stencilzpass_bf,
             normalized_depth_control.stencilzfail_bf));
  }

  for (uint32_t i = 0; i < edram_bases_sorted_count; ++i) {
    const std::pair<uint32_t, uint32_t>& rt_base_index = edram_bases_sorted[i];
    uint32_t rt_bit_index = rt_base_index.second;
    ChangeOwnership(rt_keys[rt_bit_index], 0, rt_lengths_tiles[i],
                    interlock_barrier_only
                        ? nullptr
                        : &last_update_transfers_[rt_bit_index],
                    nullptr, rt_bit_index != 0 || depth_written);
  }

  if (interlock_barrier_only) {
    // No copying transfers or render target bindings - only needed the barrier.
    return true;
  }

  // If everything succeeded, update the used render targets.
  for (uint32_t i = 0; i < 1 + xenos::kMaxColorRenderTargets; ++i) {
    last_update_used_render_targets_[i] =
        (depth_and_color_rts_used_bits & (uint32_t(1) << i)) ? rts[i] : nullptr;
  }
  if (are_accumulated_render_targets_valid_) {
    // Check if the only re-enabling a previously bound render target.
    for (uint32_t i = 0; i < 1 + xenos::kMaxColorRenderTargets; ++i) {
      RenderTarget* current_rt =
          (depth_and_color_rts_used_bits & (uint32_t(1) << i)) ? rts[i]
                                                               : nullptr;
      const RenderTarget* accumulated_rt =
          last_update_accumulated_render_targets_[i];
      if (!accumulated_rt) {
        if (current_rt) {
          // Binding a totally new render target - won't keep the existing
          // render pass anyway, no much need to try to re-enable previously
          // disabled render targets in other slots as well, even though that
          // would be valid.
          are_accumulated_render_targets_valid_ = false;
          break;
        }
        // Append the new render target.
        last_update_accumulated_render_targets_[i] = current_rt;
        continue;
      }
      if (current_rt) {
        if (current_rt != accumulated_rt) {
          // Changing a render target in a slot.
          are_accumulated_render_targets_valid_ = false;
          break;
        }
      } else {
        RenderTargetKey accumulated_rt_key = accumulated_rt->key();
        if (accumulated_rt_key.pitch_tiles_at_32bpp != pitch_tiles_at_32bpp ||
            accumulated_rt_key.msaa_samples != storage_msaa_samples) {
          // The previously bound render target is incompatible with the
          // current surface info.
          are_accumulated_render_targets_valid_ = false;
          break;
        }
      }
    }
    // Make sure the same render target isn't bound into two different slots
    // over time.
    // chrispy: this needs optimization!
    if (are_accumulated_render_targets_valid_) {
      for (uint32_t i = 1; i < 1 + xenos::kMaxColorRenderTargets; ++i) {
        const RenderTarget* render_target =
            last_update_accumulated_render_targets_[i];
        if (!render_target) {
          continue;
        }
        for (uint32_t j = 0; j < i; ++j) {
          if (last_update_accumulated_render_targets_[j] == render_target) {
            are_accumulated_render_targets_valid_ = false;
            goto exit_slot_check_loop;
          }
        }
      }
    exit_slot_check_loop:;
    }
  }
  if (!are_accumulated_render_targets_valid_) {
    std::memcpy(last_update_accumulated_render_targets_,
                last_update_used_render_targets_,
                sizeof(last_update_accumulated_render_targets_));
    are_accumulated_render_targets_valid_ = true;
  }

  return true;
}

bool RenderTargetCache::GetDepthClearRetarget(
    reg::RB_DEPTHCONTROL normalized_depth_control, RenderTargetKey draw_key,
    uint32_t length_tiles, uint32_t height_used,
    DepthClearRetarget& retarget_out) {
  // What makes the retarget exact (as XeniOS 38f08b521):
  // - The draw writes one depth value, one stencil value under the write
  //   mask, or both, and nothing else, to every sample of whole pixels of the
  //   4x view: a rectangle is the same set of samples as the rectangle of
  //   doubled coordinates in a 1x render target of the same base and pitch
  //   (doubled in X only, both samples of each pixel, in a 2x one), whatever
  //   the order of the samples in a pixel.
  // - The owner holds all the tiles the 4x view would take, as their render
  //   target and host depth, so the transfer into the 4x view and the one
  //   back out (copies of host depth and stencil between depth render targets
  //   of one format) would leave every other sample, and the ownership once
  //   the owner is drawn to again, as they are.
  const RegisterFile& regs = register_file();
  const reg::RB_DEPTHCONTROL dc = normalized_depth_control;
  if (GetPath() != Path::kHostRenderTargets || IsDrawResolutionScaled()) {
    return false;
  }
  // Nothing fails a test, depth is written or left alone, and stencil is
  // replaced under its write mask or left alone, the same for both faces.
  const bool write_depth = dc.z_enable && dc.z_write_enable;
  if (dc.z_enable && dc.zfunc != xenos::CompareFunction::kAlways) {
    return false;
  }
  auto stencil_ref_mask = regs.Get<reg::RB_STENCILREFMASK>();
  bool write_stencil = false;
  uint32_t stencil_value = 0;
  if (dc.stencil_enable) {
    if (dc.stencilfunc != xenos::CompareFunction::kAlways ||
        (dc.stencilzpass != xenos::StencilOp::kKeep &&
         dc.stencilzpass != xenos::StencilOp::kReplace &&
         dc.stencilzpass != xenos::StencilOp::kZero)) {
      return false;
    }
    if (dc.backface_enable) {
      auto stencil_ref_mask_bf =
          regs.Get<reg::RB_STENCILREFMASK>(XE_GPU_REG_RB_STENCILREFMASK_BF);
      if (dc.stencilfunc_bf != dc.stencilfunc ||
          dc.stencilzpass_bf != dc.stencilzpass ||
          stencil_ref_mask_bf.stencilref != stencil_ref_mask.stencilref ||
          stencil_ref_mask_bf.stencilwritemask !=
              stencil_ref_mask.stencilwritemask) {
        return false;
      }
    }
    write_stencil = dc.stencilzpass != xenos::StencilOp::kKeep &&
                    stencil_ref_mask.stencilwritemask != 0;
    stencil_value = dc.stencilzpass == xenos::StencilOp::kReplace
                        ? stencil_ref_mask.stencilref
                        : 0;
  }
  if (!write_depth && !write_stencil) {
    return false;
  }
  auto draw_initiator = regs.Get<reg::VGT_DRAW_INITIATOR>();
  if (draw_initiator.prim_type != xenos::PrimitiveType::kRectangleList ||
      draw_initiator.num_indices != 3) {
    return false;
  }
  // Window-space positions: no viewport, clipping or perspective divide.
  auto clip_cntl = regs.Get<reg::PA_CL_CLIP_CNTL>();
  auto vte = regs.Get<reg::PA_CL_VTE_CNTL>();
  if (!clip_cntl.clip_disable || vte.vport_x_scale_ena ||
      vte.vport_x_offset_ena || vte.vport_y_scale_ena ||
      vte.vport_y_offset_ena || vte.vport_z_scale_ena ||
      vte.vport_z_offset_ena || !vte.vtx_xy_fmt || !vte.vtx_z_fmt) {
    return false;
  }
  auto sc_mode = regs.Get<reg::PA_SU_SC_MODE_CNTL>();
  if (sc_mode.poly_offset_front_enable || sc_mode.poly_offset_back_enable ||
      sc_mode.poly_offset_para_enable || sc_mode.cull_front ||
      sc_mode.cull_back ||
      sc_mode.poly_mode == xenos::PolygonModeEnable::kDualMode) {
    // Filled only: a quad drawn as lines or points doesn't cover the
    // rectangle.
    return false;
  }
  // Every sample of a covered pixel.
  auto color_control = regs.Get<reg::RB_COLORCONTROL>();
  if ((regs[XE_GPU_REG_PA_SC_AA_MASK] & 0xF) != 0xF ||
      color_control.alpha_to_mask_enable) {
    return false;
  }
  const DrawExtentEstimator::LastVertexPositions& positions =
      draw_extent_estimator_.last_vertex_positions();
  if (!positions.valid || positions.count != 3) {
    return false;
  }
  // Depth 0 and 1 are exact in both depth formats on the host and the guest.
  // Window-space Z is clamped, not clipped (clip_disable), to the viewport's
  // 0...1 (no viewport Z scale or offset): a constant below 0 is stored as 0
  // and one above 1 as 1. The same Z at the three vertices is the Z of the
  // fourth.
  float z = positions.z[0];
  if (write_depth) {
    if (positions.z[1] != z || positions.z[2] != z || !std::isfinite(z) ||
        (z > 0.0f && z < 1.0f)) {
      return false;
    }
    z = z >= 1.0f ? 1.0f : 0.0f;
  }
  // Three distinct corners of an axis-aligned box.
  const float x_lo = std::min({positions.x[0], positions.x[1], positions.x[2]});
  const float x_hi = std::max({positions.x[0], positions.x[1], positions.x[2]});
  const float y_lo = std::min({positions.y[0], positions.y[1], positions.y[2]});
  const float y_hi = std::max({positions.y[0], positions.y[1], positions.y[2]});
  if (!(x_lo < x_hi) || !(y_lo < y_hi)) {
    return false;
  }
  bool corners[4] = {};
  for (uint32_t i = 0; i < 3; ++i) {
    const float x = positions.x[i], y = positions.y[i];
    if ((x != x_lo && x != x_hi) || (y != y_lo && y != y_hi)) {
      return false;
    }
    const uint32_t corner = uint32_t(x == x_hi) | (uint32_t(y == y_hi) << 1);
    if (corners[corner]) {
      return false;
    }
    corners[corner] = true;
  }
  float offset_x = 0.0f, offset_y = 0.0f;
  if (sc_mode.vtx_window_offset_enable) {
    auto window_offset = regs.Get<reg::PA_SC_WINDOW_OFFSET>();
    offset_x += float(window_offset.window_x_offset);
    offset_y += float(window_offset.window_y_offset);
  }
  if (cvars::half_pixel_offset && regs.Get<reg::PA_SU_VTX_CNTL>().pix_center ==
                                      xenos::PixelCenter::kD3DZero) {
    offset_x += 0.5f;
    offset_y += 0.5f;
  }
  // Whole pixels only: each edge of the box is on a pixel boundary or outside
  // the scissor, so no pixel has some of its samples covered and others not.
  draw_util::Scissor scissor;
  draw_util::GetScissor(regs, scissor, true);
  const float scissor_left = float(scissor.offset[0]);
  const float scissor_top = float(scissor.offset[1]);
  const float scissor_right = float(scissor.offset[0] + scissor.extent[0]);
  const float scissor_bottom = float(scissor.offset[1] + scissor.extent[1]);
  const float box_left = x_lo + offset_x, box_right = x_hi + offset_x;
  const float box_top = y_lo + offset_y, box_bottom = y_hi + offset_y;
  auto is_edge_whole = [](float edge) {
    return edge >= 0.0f && edge <= 16384.0f && edge == std::floor(edge);
  };
  if (!(box_left <= scissor_left || is_edge_whole(box_left)) ||
      !(box_top <= scissor_top || is_edge_whole(box_top)) ||
      !(box_right >= scissor_right || is_edge_whole(box_right)) ||
      !(box_bottom >= scissor_bottom || is_edge_whole(box_bottom))) {
    return false;
  }
  const float left = std::max(box_left, scissor_left);
  const float top = std::max(box_top, scissor_top);
  const float right = std::min(box_right, scissor_right);
  const float bottom = std::min(box_bottom, scissor_bottom);
  const uint32_t view_width =
      draw_key.pitch_tiles_at_32bpp * (xenos::kEdramTileWidthSamples >> 1);
  if (!(left < right) || !(top < bottom) || right > float(view_width) ||
      bottom > float(height_used)) {
    return false;
  }
  // The tiles the 4x view would take, all of them the owner's.
  if (!length_tiles ||
      draw_key.base_tiles + length_tiles > xenos::kEdramTileCount) {
    return false;
  }
  // The tiles the rectangle touches through the 4x view (2x2 samples in a
  // pixel, 80x16 samples in a tile): a 1x or 2x view of the same tiles,
  // whichever holds the first of them, has to hold them all. The other tiles
  // of the view's range are not touched and may be anyone's.
  const uint32_t pitch_tiles = draw_key.pitch_tiles_at_32bpp;
  const uint32_t column_first =
      uint32_t(left) * 2 / xenos::kEdramTileWidthSamples;
  const uint32_t column_end =
      std::min((uint32_t(right) * 2 + (xenos::kEdramTileWidthSamples - 1)) /
                   xenos::kEdramTileWidthSamples,
               pitch_tiles);
  const uint32_t row_first = uint32_t(top) * 2 / xenos::kEdramTileHeightSamples;
  const uint32_t row_end =
      (uint32_t(bottom) * 2 + (xenos::kEdramTileHeightSamples - 1)) /
      xenos::kEdramTileHeightSamples;
  if (column_first >= column_end || row_first >= row_end ||
      row_end * pitch_tiles > length_tiles) {
    return false;
  }
  RenderTargetKey owner_key = draw_key;
  {
    auto first_it = ownership_ranges_.upper_bound(
        draw_key.base_tiles + row_first * pitch_tiles + column_first);
    if (first_it == ownership_ranges_.begin()) {
      return false;
    }
    owner_key.msaa_samples =
        std::prev(first_it)->second.render_target.msaa_samples;
    if (owner_key.msaa_samples != xenos::MsaaSamples::k1X &&
        owner_key.msaa_samples != xenos::MsaaSamples::k2X) {
      return false;
    }
  }
  const bool host_depth_encoding_different =
      IsHostDepthEncodingDifferent(draw_key.GetDepthFormat());
  auto is_owned = [&](uint32_t start_tiles, uint32_t end_tiles) -> bool {
    uint32_t owned_until = start_tiles;
    auto it = ownership_ranges_.upper_bound(start_tiles);
    if (it != ownership_ranges_.begin()) {
      --it;
    }
    for (; it != ownership_ranges_.end() && it->first < end_tiles; ++it) {
      const OwnershipRange& range = it->second;
      if (range.end_tiles <= start_tiles) {
        continue;
      }
      if (it->first > owned_until || range.render_target != owner_key) {
        return false;
      }
      if (host_depth_encoding_different &&
          range.GetHostDepthRenderTarget(draw_key.GetDepthFormat()) !=
              owner_key) {
        return false;
      }
      owned_until = range.end_tiles;
    }
    return owned_until >= end_tiles;
  };
  if (!column_first && column_end == pitch_tiles) {
    if (!is_owned(draw_key.base_tiles + row_first * pitch_tiles,
                  draw_key.base_tiles + row_end * pitch_tiles)) {
      return false;
    }
  } else {
    for (uint32_t row = row_first; row < row_end; ++row) {
      const uint32_t row_tiles = draw_key.base_tiles + row * pitch_tiles;
      if (!is_owned(row_tiles + column_first, row_tiles + column_end)) {
        return false;
      }
    }
  }
  auto owner_it = render_targets_.find(owner_key);
  if (owner_it == render_targets_.end() || !owner_it->second) {
    return false;
  }
  retarget_out.owner = owner_it->second;
  // 4x has 2x2 samples in a pixel; 2x has them one above the other, so its
  // pixels are the 4x view's columns of samples.
  const uint32_t owner_y_shift =
      uint32_t(owner_key.msaa_samples == xenos::MsaaSamples::k1X);
  retarget_out.rectangle.x_pixels = uint32_t(left) * 2;
  retarget_out.rectangle.y_pixels = uint32_t(top) << owner_y_shift;
  retarget_out.rectangle.width_pixels = uint32_t(right - left) * 2;
  retarget_out.rectangle.height_pixels = uint32_t(bottom - top)
                                         << owner_y_shift;
  retarget_out.write_depth = write_depth;
  retarget_out.write_stencil = write_stencil;
  retarget_out.stencil_value = stencil_value & 0xFF;
  retarget_out.stencil_write_mask = stencil_ref_mask.stencilwritemask & 0xFF;
  retarget_out.guest_depth =
      !write_depth || z == 0.0f
          ? 0
          : (draw_key.GetDepthFormat() == xenos::DepthRenderTargetFormat::kD24S8
                 ? 0xFFFFFF
                 : xenos::Float32To20e4(1.0f, true));
  return true;
}

uint32_t RenderTargetCache::GetLastUpdateBoundRenderTargets(
    uint32_t* depth_and_color_formats_out) const {
  if (GetPath() != Path::kHostRenderTargets) {
    if (depth_and_color_formats_out) {
      std::memset(depth_and_color_formats_out, 0,
                  sizeof(uint32_t) * (1 + xenos::kMaxColorRenderTargets));
    }
    return 0;
  }
  uint32_t rts_used = 0;
  for (uint32_t i = 0; i < 1 + xenos::kMaxColorRenderTargets; ++i) {
    const RenderTarget* render_target =
        last_update_accumulated_render_targets_[i];
    if (!render_target) {
      if (depth_and_color_formats_out) {
        depth_and_color_formats_out[i] = 0;
      }
      continue;
    }
    rts_used |= uint32_t(1) << i;
    if (depth_and_color_formats_out) {
      depth_and_color_formats_out[i] = render_target->key().resource_format;
    }
  }
  return rts_used;
}

uint32_t RenderTargetCache::GetRenderTargetHeight(
    uint32_t pitch_tiles_at_32bpp, xenos::MsaaSamples msaa_samples) const {
  if (!pitch_tiles_at_32bpp) {
    return 0;
  }
  // Down to the beginning of the render target in the next 11-bit EDRAM
  // addressing period.
  uint32_t tile_rows = (xenos::kEdramTileCount + (pitch_tiles_at_32bpp - 1)) /
                       pitch_tiles_at_32bpp;
  // Clamp to the guest limit (tile padding should exceed it) and to the host
  // limit (tile padding mustn't exceed it).
  static_assert(
      !(xenos::kTexture2DCubeMaxWidthHeight % xenos::kEdramTileHeightSamples),
      "Maximum guest render target height is assumed to always be a multiple "
      "of an EDRAM tile height");
  uint32_t max_height_scaled =
      std::min(xenos::kTexture2DCubeMaxWidthHeight * draw_resolution_scale_y(),
               GetMaxRenderTargetHeight());
  uint32_t msaa_samples_y_log2 =
      uint32_t(msaa_samples >= xenos::MsaaSamples::k2X);
  uint32_t tile_height_samples_scaled =
      xenos::kEdramTileHeightSamples * draw_resolution_scale_y();
  tile_rows = std::min(tile_rows, (max_height_scaled << msaa_samples_y_log2) /
                                      tile_height_samples_scaled);
  assert_not_zero(tile_rows);
  return tile_rows * (xenos::kEdramTileHeightSamples >> msaa_samples_y_log2);
}

void RenderTargetCache::GetHostDepthStoreRectangleInfo(
    const Transfer::Rectangle& transfer_rectangle,
    xenos::MsaaSamples msaa_samples,
    HostDepthStoreRectangleConstant& rectangle_constant_out,
    uint32_t& group_count_x_out, uint32_t& group_count_y_out) const {
  // Initialize to all bits zeroed.
  HostDepthStoreRectangleConstant rectangle_constant;
  // 8 pixels is the resolve granularity, both clearing and tile size are
  // aligned to 8.
  assert_zero(transfer_rectangle.x_pixels & 7);
  assert_zero(transfer_rectangle.y_pixels & 7);
  assert_zero(transfer_rectangle.width_pixels & 7);
  assert_zero(transfer_rectangle.height_pixels & 7);
  assert_not_zero(transfer_rectangle.width_pixels);
  rectangle_constant.x_pixels_div_8 = transfer_rectangle.x_pixels >> 3;
  rectangle_constant.y_pixels_div_8 = transfer_rectangle.y_pixels >> 3;
  rectangle_constant.width_pixels_div_8_minus_1 =
      (transfer_rectangle.width_pixels >> 3) - 1;
  rectangle_constant_out = rectangle_constant;
  // 1 thread group = 64x8 host samples.
  uint32_t pixel_size_x = draw_resolution_scale_x()
                          << uint32_t(msaa_samples >= xenos::MsaaSamples::k4X);
  uint32_t pixel_size_y = draw_resolution_scale_y()
                          << uint32_t(msaa_samples >= xenos::MsaaSamples::k2X);
  group_count_x_out =
      (transfer_rectangle.width_pixels * pixel_size_x + 63) >> 6;
  group_count_y_out = (transfer_rectangle.height_pixels * pixel_size_y) >> 3;
}

void RenderTargetCache::GetResolveCopyRectanglesToDump(
    uint32_t base, uint32_t row_length, uint32_t rows, uint32_t pitch,
    std::vector<ResolveCopyDumpRectangle>& rectangles_out) const {
  rectangles_out.clear();
  assert_true(row_length <= pitch);
  row_length = std::min(row_length, pitch);
  if (!row_length || !rows) {
    return;
  }
  auto get_rectangles_in_extent = [&](uint32_t extent_start,
                                      uint32_t extent_end,
                                      uint32_t range_local_offset) {
    // Collect render targets owning ranges within the specified rectangle. The
    // first render target in the range may be before the lower_bound, only
    // being in the range with its tail.
    auto it = ownership_ranges_.lower_bound(extent_start);
    if (it != ownership_ranges_.cbegin()) {
      auto it_pre = std::prev(it);
      if (it_pre->second.end_tiles > extent_start) {
        it = it_pre;
      }
    }
    for (; it != ownership_ranges_.cend(); ++it) {
      uint32_t range_global_start = std::max(it->first, extent_start);
      if (range_global_start >= extent_end) {
        break;
      }
      RenderTargetKey rt_key = it->second.render_target;
      if (rt_key.IsEmpty()) {
        continue;
      }
      // Merge with other render target ranges with the same current ownership,
      // but different depth ownership, since it's not relevant to resolving.
      while (it != ownership_ranges_.cend()) {
        auto it_next = std::next(it);
        if (it_next == ownership_ranges_.cend() ||
            it_next->first >= extent_end ||
            it_next->second.render_target != rt_key) {
          break;
        }
        it = it_next;
      }

      uint32_t range_local_start = range_local_offset +
                                   std::max(range_global_start, extent_start) -
                                   base;
      uint32_t range_local_end = range_local_offset +
                                 std::min(it->second.end_tiles, extent_end) -
                                 base;
      assert_true(range_local_start < range_local_end);

      uint32_t rows_start = range_local_start / pitch;
      uint32_t rows_end = (range_local_end + (pitch - 1)) / pitch;
      uint32_t row_first_start = range_local_start - rows_start * pitch;
      if (row_first_start >= row_length) {
        // The first row starts within the pitch padding.
        if (rows_start + 1 < rows_end) {
          // Multiple rows - start at the second.
          ++rows_start;
          row_first_start = 0;
        } else {
          // Single row - nothing to dump.
          continue;
        }
      }

      auto it_rt = render_targets_.find(rt_key);
      assert_true(it_rt != render_targets_.cend());
      assert_not_null(it_rt->second);
      // Don't include pitch padding in the last row.
      rectangles_out.emplace_back(
          it_rt->second, rows_start, rows_end - rows_start, row_first_start,
          std::min(pitch - (rows_end * pitch - range_local_end), row_length));
    }
  };
  uint32_t resolve_area_end = base + (rows - 1) * pitch + row_length;
  get_rectangles_in_extent(
      base, std::min(resolve_area_end, xenos::kEdramTileCount), 0);
  if (resolve_area_end > xenos::kEdramTileCount) {
    // The resolve area goes to the next EDRAM addressing period.
    get_rectangles_in_extent(
        0, std::min(resolve_area_end & (xenos::kEdramTileCount - 1), base),
        xenos::kEdramTileCount);
  }
}

bool RenderTargetCache::PrepareHostRenderTargetsResolveClear(
    const draw_util::ResolveInfo& resolve_info,
    Transfer::Rectangle& clear_rectangle_out,
    RenderTarget*& depth_render_target_out,
    std::vector<Transfer>& depth_transfers_out,
    RenderTarget*& color_render_target_out,
    std::vector<Transfer>& color_transfers_out) {
  assert_true(GetPath() == Path::kHostRenderTargets);

  uint32_t pitch_tiles_at_32bpp;
  uint32_t base_offset_tiles_at_32bpp;
  xenos::MsaaSamples msaa_samples;
  if (resolve_info.IsClearingDepth()) {
    pitch_tiles_at_32bpp = resolve_info.depth_edram_info.pitch_tiles;
    base_offset_tiles_at_32bpp = resolve_info.depth_edram_info.base_tiles -
                                 resolve_info.depth_original_base;
    msaa_samples = resolve_info.depth_edram_info.msaa_samples;
  } else if (resolve_info.IsClearingColor()) {
    pitch_tiles_at_32bpp = resolve_info.color_edram_info.pitch_tiles;
    base_offset_tiles_at_32bpp = resolve_info.color_edram_info.base_tiles -
                                 resolve_info.color_original_base;
    if (resolve_info.color_edram_info.format_is_64bpp) {
      assert_zero(pitch_tiles_at_32bpp & 1);
      pitch_tiles_at_32bpp >>= 1;
      assert_zero(base_offset_tiles_at_32bpp & 1);
      base_offset_tiles_at_32bpp >>= 1;
    }
    msaa_samples = resolve_info.color_edram_info.msaa_samples;
  } else {
    return false;
  }
  assert_true(msaa_samples <= xenos::MsaaSamples::k4X);
  if (!pitch_tiles_at_32bpp) {
    return false;
  }
  uint32_t msaa_samples_x_log2 =
      uint32_t(msaa_samples >= xenos::MsaaSamples::k4X);
  uint32_t msaa_samples_y_log2 =
      uint32_t(msaa_samples >= xenos::MsaaSamples::k2X);
  if (pitch_tiles_at_32bpp >
      ((xenos::kTexture2DCubeMaxWidthHeight << msaa_samples_x_log2) +
       (xenos::kEdramTileWidthSamples - 1)) /
          xenos::kEdramTileWidthSamples) {
    XELOGE(
        "Surface pitch in 80-sample groups {} at {}x MSAA larger than the "
        "maximum texture width {} specified by the guest in a resolve",
        pitch_tiles_at_32bpp, uint32_t(1) << uint32_t(msaa_samples),
        xenos::kTexture2DCubeMaxWidthHeight);
    return false;
  }
  uint32_t pitch_pixels =
      pitch_tiles_at_32bpp *
      (xenos::kEdramTileWidthSamples >> msaa_samples_x_log2);
  uint32_t pitch_pixels_scaled = pitch_pixels * draw_resolution_scale_x();
  uint32_t max_render_target_width = GetMaxRenderTargetWidth();
  if (pitch_pixels_scaled > max_render_target_width) {
    // TODO(Triang3l): If really needed for some game on some device, clamp the
    // pitch the same way as explained in the comment in Update.
    XELOGE(
        "Surface pitch aligned to EDRAM tiles and resolution-scaled {} larger "
        "than the maximum host render target width {} in a resolve",
        pitch_pixels_scaled, max_render_target_width);
    return false;
  }

  uint32_t render_target_height_pixels =
      GetRenderTargetHeight(pitch_tiles_at_32bpp, msaa_samples);
  uint32_t base_offset_rows_at_32bpp =
      base_offset_tiles_at_32bpp / pitch_tiles_at_32bpp;
  Transfer::Rectangle clear_rectangle;
  clear_rectangle.x_pixels = std::min(
      (base_offset_tiles_at_32bpp -
       base_offset_rows_at_32bpp * pitch_tiles_at_32bpp) *
              (xenos::kEdramTileWidthSamples >> msaa_samples_x_log2) +
          (uint32_t(resolve_info.coordinate_info.edram_offset_x_div_8) << 3),
      pitch_pixels);
  clear_rectangle.y_pixels = std::min(
      base_offset_rows_at_32bpp *
              (xenos::kEdramTileHeightSamples >> msaa_samples_y_log2) +
          (uint32_t(resolve_info.coordinate_info.edram_offset_y_div_8) << 3),
      render_target_height_pixels);
  clear_rectangle.width_pixels =
      std::min(uint32_t(resolve_info.coordinate_info.width_div_8) << 3,
               pitch_pixels - clear_rectangle.x_pixels);
  clear_rectangle.height_pixels =
      std::min(uint32_t(resolve_info.height_div_8) << 3,
               render_target_height_pixels - clear_rectangle.y_pixels);
  if (!clear_rectangle.width_pixels || !clear_rectangle.height_pixels) {
    // Outside the pitch / height (or initially specified as 0).
    return false;
  }

  // Change ownership of the tiles containing the area to be cleared, so the
  // up-to-date host render target for the cleared range will be the cleared
  // one.
  uint32_t clear_start_tiles_at_32bpp =
      ((clear_rectangle.y_pixels << msaa_samples_y_log2) /
       xenos::kEdramTileHeightSamples) *
          pitch_tiles_at_32bpp +
      (clear_rectangle.x_pixels << msaa_samples_x_log2) /
          xenos::kEdramTileWidthSamples;
  uint32_t clear_length_tiles_at_32bpp =
      (((clear_rectangle.y_pixels + clear_rectangle.height_pixels - 1)
        << msaa_samples_y_log2) /
       xenos::kEdramTileHeightSamples) *
          pitch_tiles_at_32bpp +
      ((clear_rectangle.x_pixels + clear_rectangle.width_pixels - 1)
       << msaa_samples_x_log2) /
          xenos::kEdramTileWidthSamples +
      1 - clear_start_tiles_at_32bpp;
  // Up to the range from the base in the current 11 tile index bits to the base
  // in the next 11 tile index bits after wrapping can be cleared.
  uint32_t depth_clear_start_tiles_base_relative = 0;
  uint32_t depth_clear_length_tiles = 0;
  if (resolve_info.IsClearingDepth()) {
    depth_clear_start_tiles_base_relative =
        std::min(clear_start_tiles_at_32bpp, xenos::kEdramTileCount);
    depth_clear_length_tiles =
        std::min(clear_start_tiles_at_32bpp + clear_length_tiles_at_32bpp,
                 xenos::kEdramTileCount) -
        depth_clear_start_tiles_base_relative;
  }
  uint32_t color_clear_start_tiles_base_relative = 0;
  uint32_t color_clear_length_tiles = 0;
  if (resolve_info.IsClearingColor()) {
    color_clear_start_tiles_base_relative =
        std::min(clear_start_tiles_at_32bpp
                     << resolve_info.color_edram_info.format_is_64bpp,
                 xenos::kEdramTileCount);
    color_clear_length_tiles =
        std::min((clear_start_tiles_at_32bpp + clear_length_tiles_at_32bpp)
                     << resolve_info.color_edram_info.format_is_64bpp,
                 xenos::kEdramTileCount) -
        color_clear_start_tiles_base_relative;
  }
  if (depth_clear_length_tiles && color_clear_length_tiles) {
    // Prevent overlap - clear the depth only until the color, the color only
    // until the depth, in the current or the next 11 bits of the tile index.
    uint32_t depth_clear_start_tiles_wrapped =
        (resolve_info.depth_original_base +
         depth_clear_start_tiles_base_relative) &
        (xenos::kEdramTileCount - 1);
    uint32_t color_clear_start_tiles_wrapped =
        (resolve_info.color_original_base +
         color_clear_start_tiles_base_relative) &
        (xenos::kEdramTileCount - 1);
    depth_clear_length_tiles = std::min(
        depth_clear_length_tiles,
        ((color_clear_start_tiles_wrapped < depth_clear_start_tiles_wrapped)
             ? xenos::kEdramTileCount
             : 0) +
            color_clear_start_tiles_wrapped - depth_clear_start_tiles_wrapped);
    color_clear_length_tiles = std::min(
        color_clear_length_tiles,
        ((depth_clear_start_tiles_wrapped < color_clear_start_tiles_wrapped)
             ? xenos::kEdramTileCount
             : 0) +
            depth_clear_start_tiles_wrapped - color_clear_start_tiles_wrapped);
  }

  // msaa_samples_as_pixels: a clear covers whole pixels wherever the samples
  // are, so as for a rectangle list in Update, the 1x render targets of the
  // samples, unless the host MSAA render target holds the EDRAM now.
  xenos::MsaaSamples storage_msaa_samples = msaa_samples;
  if (msaa_samples != xenos::MsaaSamples::k1X &&
      cvars::msaa_samples_as_pixels && SupportsMsaaSamplesAsPixels() &&
      (depth_clear_length_tiles || color_clear_length_tiles)) {
    RenderTargetKey msaa_key;
    msaa_key.pitch_tiles_at_32bpp = pitch_tiles_at_32bpp;
    msaa_key.msaa_samples = msaa_samples;
    uint32_t clear_first_tile;
    if (depth_clear_length_tiles) {
      msaa_key.base_tiles = resolve_info.depth_original_base;
      msaa_key.is_depth = 1;
      msaa_key.resource_format = resolve_info.depth_edram_info.format;
      clear_first_tile = resolve_info.depth_original_base +
                         depth_clear_start_tiles_base_relative;
    } else {
      msaa_key.base_tiles = resolve_info.color_original_base;
      msaa_key.resource_format =
          uint32_t(GetColorResourceFormat(xenos::ColorRenderTargetFormat(
              resolve_info.color_edram_info.format)));
      clear_first_tile = resolve_info.color_original_base +
                         color_clear_start_tiles_base_relative;
    }
    if (!IsEdramTileOwnedBy(clear_first_tile & (xenos::kEdramTileCount - 1),
                            msaa_key)) {
      storage_msaa_samples = xenos::MsaaSamples::k1X;
    }
  }

  RenderTargetKey depth_render_target_key;
  RenderTarget* depth_render_target = nullptr;
  if (depth_clear_length_tiles) {
    depth_render_target_key.base_tiles = resolve_info.depth_original_base;
    depth_render_target_key.pitch_tiles_at_32bpp = pitch_tiles_at_32bpp;
    depth_render_target_key.msaa_samples = storage_msaa_samples;
    depth_render_target_key.is_depth = 1;
    depth_render_target_key.resource_format =
        resolve_info.depth_edram_info.format;
    depth_render_target = GetOrCreateRenderTarget(depth_render_target_key);
    if (!depth_render_target) {
      // Failed to create the depth render target, don't clear it.
      depth_render_target_key = RenderTargetKey();
      depth_clear_length_tiles = 0;
    }
  }
  RenderTargetKey color_render_target_key;
  RenderTarget* color_render_target = nullptr;
  if (color_clear_length_tiles) {
    color_render_target_key.base_tiles = resolve_info.color_original_base;
    color_render_target_key.pitch_tiles_at_32bpp = pitch_tiles_at_32bpp;
    color_render_target_key.msaa_samples = storage_msaa_samples;
    color_render_target_key.is_depth = 0;
    color_render_target_key.resource_format = uint32_t(GetColorResourceFormat(
        xenos::ColorRenderTargetFormat(resolve_info.color_edram_info.format)));
    color_render_target = GetOrCreateRenderTarget(color_render_target_key);
    if (!color_render_target) {
      // Failed to create the color render target, don't clear it.
      color_render_target_key = RenderTargetKey();
      color_clear_length_tiles = 0;
    }
  }
  if (!depth_clear_length_tiles && !color_clear_length_tiles) {
    // Complete overlap, or failed to create all the render targets.
    return false;
  }

  if (storage_msaa_samples != msaa_samples) {
    // The render targets' pixels are the samples.
    clear_rectangle.x_pixels <<= msaa_samples_x_log2;
    clear_rectangle.y_pixels <<= msaa_samples_y_log2;
    clear_rectangle.width_pixels <<= msaa_samples_x_log2;
    clear_rectangle.height_pixels <<= msaa_samples_y_log2;
  }
  clear_rectangle_out = clear_rectangle;
  depth_render_target_out = depth_render_target;
  depth_transfers_out.clear();
  if (depth_render_target) {
    ChangeOwnership(
        depth_render_target_key, depth_clear_start_tiles_base_relative,
        depth_clear_length_tiles, &depth_transfers_out, &clear_rectangle);
  }
  color_render_target_out = color_render_target;
  color_transfers_out.clear();
  if (color_render_target) {
    ChangeOwnership(
        color_render_target_key, color_clear_start_tiles_base_relative,
        color_clear_length_tiles, &color_transfers_out, &clear_rectangle);
  }
  return true;
}

RenderTargetCache::RenderTarget*
RenderTargetCache::PrepareFullEdram1280xRenderTargetForSnapshotRestoration(
    xenos::ColorRenderTargetFormat color_format) {
  assert_true(GetPath() == Path::kHostRenderTargets);
  constexpr uint32_t kPitchTilesAt32bpp = 16;
  constexpr uint32_t kWidth =
      kPitchTilesAt32bpp * xenos::kEdramTileWidthSamples;
  if (kWidth * draw_resolution_scale_x() > GetMaxRenderTargetWidth()) {
    return nullptr;
  }
  // Same render target height is used for 32bpp and 64bpp to allow mixing them.
  constexpr uint32_t kHeightTileRows =
      xenos::kEdramTileCount / kPitchTilesAt32bpp;
  static_assert(
      kPitchTilesAt32bpp * kHeightTileRows == xenos::kEdramTileCount,
      "Using width of the render target for EDRAM snapshot restoration that is "
      "expected to result in the last row being fully utilized.");
  constexpr uint32_t kHeight = kHeightTileRows * xenos::kEdramTileHeightSamples;
  static_assert(
      kHeight <= xenos::kTexture2DCubeMaxWidthHeight,
      "Using width of the render target for EDRAM snapshot restoration that is "
      "expect to fully cover the EDRAM without exceeding the maximum guest "
      "render target height.");
  if (kHeight * draw_resolution_scale_y() > GetMaxRenderTargetHeight()) {
    return nullptr;
  }
  RenderTargetKey render_target_key;
  render_target_key.pitch_tiles_at_32bpp = kPitchTilesAt32bpp;
  render_target_key.resource_format =
      uint32_t(GetColorResourceFormat(color_format));
  RenderTarget* render_target = GetOrCreateRenderTarget(render_target_key);
  if (!render_target) {
    return nullptr;
  }
  // Change ownership, but don't transfer the contents - they will be replaced
  // anyway.
  ownership_ranges_.clear();
  ownership_ranges_.emplace(
      std::piecewise_construct, std::forward_as_tuple(uint32_t(0)),
      std::forward_as_tuple(xenos::kEdramTileCount, render_target_key,
                            RenderTargetKey(), RenderTargetKey()));
  return render_target;
}

void RenderTargetCache::PixelShaderInterlockFullEdramBarrierPlaced() {
  assert_true(GetPath() == Path::kPixelShaderInterlock);
  // Clear ownership - any overlap of data written before the barrier is safe.
  OwnershipRange empty_range(xenos::kEdramTileCount, RenderTargetKey(),
                             RenderTargetKey(), RenderTargetKey());
  if (ownership_ranges_.size() == 1) {
    // Do not reallocate map elements if not needed (either nothing drawn since
    // the last barrier, or all of the EDRAM is owned by one render target).
    // The ownership map contains no gaps - the first element should always be
    // at 0.
    assert_true(!ownership_ranges_.begin()->first);
    OwnershipRange& all_edram_range = ownership_ranges_.begin()->second;
    assert_true(all_edram_range.end_tiles == xenos::kEdramTileCount);
    all_edram_range = empty_range;
    return;
  }
  ownership_ranges_.clear();
  ownership_ranges_.emplace(0, empty_range);
}

RenderTargetCache::RenderTarget* RenderTargetCache::GetOrCreateRenderTarget(
    RenderTargetKey key) {
  assert_true(GetPath() == Path::kHostRenderTargets);
  auto it_rt = render_targets_.find(key);
  RenderTarget* render_target;
  if (it_rt != render_targets_.end()) {
    render_target = it_rt->second;
  } else {
    render_target = CreateRenderTarget(key);
    uint32_t width = key.GetWidth();
    uint32_t height =
        GetRenderTargetHeight(key.pitch_tiles_at_32bpp, key.msaa_samples);
    if (render_target) {
      XELOGGPU(
          "Created a {}x{} {}xMSAA {} render target with guest format {} at "
          "EDRAM base {}",
          width, height, uint32_t(1) << uint32_t(key.msaa_samples),
          key.is_depth ? "depth" : "color", key.resource_format,
          key.base_tiles);
    } else {
      XELOGE(
          "Failed to create a {}x{} {}xMSAA {} render target with guest format "
          "{} at EDRAM base {}",
          width, height, uint32_t(1) << uint32_t(key.msaa_samples),
          key.is_depth ? "depth" : "color", key.resource_format,
          key.base_tiles);
    }
    // Insert even if failed to create, not to try to create again.
    render_targets_.emplace(key, render_target);
  }
  return render_target;
}

bool RenderTargetCache::WouldOwnershipChangeRequireTransfers(
    RenderTargetKey dest, uint32_t start_tiles_base_relative,
    uint32_t length_tiles) const {
  // xenos::kEdramTileCount with length 0 is fine if both the start and the end
  // are clamped to xenos::kEdramTileCount.
  assert_true(start_tiles_base_relative <=
              (xenos::kEdramTileCount - uint32_t(length_tiles != 0)));
  assert_true(length_tiles <= xenos::kEdramTileCount);
  if (length_tiles == 0) {
    return false;
  }
  bool host_depth_encoding_different =
      dest.is_depth && GetPath() == Path::kHostRenderTargets &&
      IsHostDepthEncodingDifferent(dest.GetDepthFormat());
  auto would_require_transfers_in_extent = [&](uint32_t extent_start,
                                               uint32_t extent_end) -> bool {
    // The map contains consecutive ranges, merged if the adjacent ones are the
    // same. Find the range starting at >= the start. A portion of the range
    // preceding it may be intersecting the render target's range (or even fully
    // contain it).
    auto it = ownership_ranges_.lower_bound(extent_start);
    if (it != ownership_ranges_.begin()) {
      auto it_pre = std::prev(it);
      if (it_pre->second.end_tiles > extent_start) {
        it = it_pre;
      }
    }
    for (; it != ownership_ranges_.end(); ++it) {
      if (it->first >= extent_end) {
        // Outside the touched extent already.
        break;
      }
      if (it->second.IsOwnedBy(dest, host_depth_encoding_different)) {
        // Already owned by the needed render target - no need to transfer
        // anything.
        continue;
      }
      RenderTargetKey transfer_source = it->second.render_target;
      // Only perform the transfer when actually changing the latest owner, not
      // just the latest host depth owner - the transfer source is expected to
      // be different than the destination.
      if (!transfer_source.IsEmpty() && transfer_source != dest) {
        return true;
      }
    }
    return false;
  };
  // start_tiles_base_relative may already be in the next 11 bits - wrap the
  // start tile index to use the same code as if that was not the case.
  uint32_t start_tiles = (dest.base_tiles + start_tiles_base_relative) &
                         (xenos::kEdramTileCount - 1);
  uint32_t end_tiles = start_tiles + length_tiles;
  if (would_require_transfers_in_extent(
          start_tiles, std::min(end_tiles, xenos::kEdramTileCount))) {
    return true;
  }
  if (end_tiles > xenos::kEdramTileCount) {
    // The check extent goes to the next EDRAM addressing period.
    if (would_require_transfers_in_extent(
            0,
            std::min(end_tiles & (xenos::kEdramTileCount - 1), start_tiles))) {
      return true;
    }
  }
  return false;
}

void RenderTargetCache::ChangeOwnership(
    RenderTargetKey dest, uint32_t start_tiles_base_relative,
    uint32_t length_tiles, std::vector<Transfer>* transfers_append_out,
    const Transfer::Rectangle* resolve_clear_cutout, bool dest_written) {
  // xenos::kEdramTileCount with length 0 is fine if both the start and the end
  // are clamped to xenos::kEdramTileCount.
  assert_true(start_tiles_base_relative <=
              (xenos::kEdramTileCount - uint32_t(length_tiles != 0)));
  assert_true(length_tiles <= xenos::kEdramTileCount);
  if (length_tiles == 0) {
    return;
  }
  uint32_t dest_pitch_tiles = dest.GetPitchTiles();
  bool dest_is_64bpp = dest.Is64bpp();
  bool host_depth_encoding_different =
      dest.is_depth && GetPath() == Path::kHostRenderTargets &&
      IsHostDepthEncodingDifferent(dest.GetDepthFormat());
  auto change_ownership_in_extent = [&](uint32_t extent_start,
                                        uint32_t extent_end) {
    // The map contains consecutive ranges, merged if the adjacent ones are the
    // same. Find the range starting at >= the start. A portion of the range
    // preceding it may be intersecting the render target's range (or even fully
    // contain it) - split it into the untouched head and the claimed tail if
    // needed.
    auto it = ownership_ranges_.lower_bound(extent_start);
    if (it != ownership_ranges_.begin()) {
      auto it_pre = std::prev(it);
      if (it_pre->second.end_tiles > extent_start) {
        if (!it_pre->second.IsOwnedBy(dest, host_depth_encoding_different)) {
          // Different render target overlapping the range - split the head.
          ownership_ranges_.emplace(extent_start, it_pre->second);
          it_pre->second.end_tiles = extent_start;
          // Let the next loop do the transfer and needed merging and
          // splitting starting from the added tail.
          it = std::next(it_pre);
        } else if (dest_written) {
          // Already owned, and about to be written: other copies go stale
          // (also in the head before the extent, which is just conservative).
          it_pre->second.intact_render_target = RenderTargetKey();
        }
      }
    }
    while (it != ownership_ranges_.end()) {
      if (it->first >= extent_end) {
        // Outside the touched extent already.
        break;
      }
      if (it->second.IsOwnedBy(dest, host_depth_encoding_different)) {
        // Already owned by the needed render target - no need to transfer
        // anything.
        if (dest_written && !it->second.intact_render_target.IsEmpty()) {
          it->second.intact_render_target = RenderTargetKey();
          // Keep adjacent ranges distinct.
          if (it != ownership_ranges_.begin()) {
            auto it_prev = std::prev(it);
            if (it_prev->second.AreOwnersSame(it->second)) {
              it_prev->second.end_tiles = it->second.end_tiles;
              ownership_ranges_.erase(it);
              it = it_prev;
            }
          }
        }
        ++it;
        continue;
      }
      // Take over the current range. Handle the tail - may be outside the range
      // (split in this case) or within it.
      if (it->second.end_tiles > extent_end) {
        // Split the tail.
        ownership_ranges_.emplace(extent_end, it->second);
        it->second.end_tiles = extent_end;
      }
      // Whether the previous owner's copy will match dest's after this.
      bool previous_owner_matches = false;
      if (transfers_append_out) {
        RenderTargetKey transfer_source = it->second.render_target;
        // Only perform the copying when actually changing the latest owner, not
        // just the latest host depth owner - the transfer source is expected to
        // be different than the destination.
        bool dest_intact = !transfer_source.IsEmpty() &&
                           transfer_source != dest &&
                           it->second.intact_render_target == dest;
        if (dest_intact) {
          ++unchanged_transfers_;
          if (cvars::skip_unchanged_render_target_transfers) {
            ++skipped_transfers_;
            previous_owner_matches = true;
          }
        }
        if (!transfer_source.IsEmpty() && transfer_source != dest &&
            !previous_owner_matches) {
          uint32_t transfer_end_tiles =
              std::min(it->second.end_tiles, extent_end);
          if (!resolve_clear_cutout ||
              Transfer::GetRangeRectangles(it->first, transfer_end_tiles,
                                           dest.base_tiles, dest_pitch_tiles,
                                           dest.msaa_samples, dest_is_64bpp,
                                           nullptr, resolve_clear_cutout)) {
            RenderTargetKey transfer_host_depth_source =
                host_depth_encoding_different
                    ? it->second.GetHostDepthRenderTarget(dest.GetDepthFormat())
                    : RenderTargetKey();
            if (transfer_host_depth_source == transfer_source) {
              // Same render target, don't provide a separate host depth source.
              transfer_host_depth_source = RenderTargetKey();
            }
            if (!transfers_append_out->empty() &&
                transfers_append_out->back().end_tiles == it->first &&
                transfers_append_out->back().source->key() == transfer_source &&
                ((transfers_append_out->back().host_depth_source == nullptr) ==
                 transfer_host_depth_source.IsEmpty()) &&
                (transfer_host_depth_source.IsEmpty() ||
                 transfers_append_out->back().host_depth_source->key() ==
                     transfer_host_depth_source)) {
              // Extend the last transfer if, for example, transferring color,
              // but host depth is different.
              transfers_append_out->back().end_tiles = transfer_end_tiles;
              previous_owner_matches = true;
            } else {
              auto transfer_source_rt_it =
                  render_targets_.find(transfer_source);
              if (transfer_source_rt_it != render_targets_.end()) {
                assert_not_null(transfer_source_rt_it->second);
                auto transfer_host_depth_source_rt_it =
                    !transfer_host_depth_source.IsEmpty()
                        ? render_targets_.find(transfer_host_depth_source)
                        : render_targets_.end();
                if (transfer_host_depth_source.IsEmpty() ||
                    transfer_host_depth_source_rt_it != render_targets_.end()) {
                  assert_false(transfer_host_depth_source_rt_it !=
                                   render_targets_.end() &&
                               !transfer_host_depth_source_rt_it->second);
                  transfers_append_out->emplace_back(
                      it->first, transfer_end_tiles,
                      transfer_source_rt_it->second,
                      transfer_host_depth_source_rt_it != render_targets_.end()
                          ? transfer_host_depth_source_rt_it->second
                          : nullptr);
                  previous_owner_matches = true;
                }
              }
            }
          }
        }
      }
      // Claim the current range.
      RenderTargetKey previous_owner = it->second.render_target;
      it->second.render_target = dest;
      if (host_depth_encoding_different) {
        it->second.GetHostDepthRenderTarget(dest.GetDepthFormat()) = dest;
      }
      it->second.intact_render_target = previous_owner_matches && !dest_written
                                            ? previous_owner
                                            : RenderTargetKey();
      // Check if can merge with the next range after claiming.
      std::map<uint32_t, OwnershipRange>::iterator it_next;
      if (it != ownership_ranges_.end()) {
        it_next = std::next(it);
        if (it_next != ownership_ranges_.end() &&
            it_next->second.AreOwnersSame(it->second)) {
          // Merge with the next range.
          it->second.end_tiles = it_next->second.end_tiles;
          auto it_after = std::next(it_next);
          ownership_ranges_.erase(it_next);
          it_next = it_after;
        }
      } else {
        it_next = ownership_ranges_.end();
      }
      // Check if can merge with the previous range after claiming and merging
      // with the next (thus obtaining the correct end pointer).
      if (it != ownership_ranges_.begin()) {
        auto it_prev = std::prev(it);
        if (it_prev->second.AreOwnersSame(it->second)) {
          it_prev->second.end_tiles = it->second.end_tiles;
          ownership_ranges_.erase(it);
        }
      }
      it = it_next;
    }
  };
  // start_tiles_base_relative may already be in the next 11 bits - wrap the
  // start tile index to use the same code as if that was not the case.
  uint32_t start_tiles = (dest.base_tiles + start_tiles_base_relative) &
                         (xenos::kEdramTileCount - 1);
  uint32_t end_tiles = start_tiles + length_tiles;
  change_ownership_in_extent(start_tiles,
                             std::min(end_tiles, xenos::kEdramTileCount));
  if (end_tiles > xenos::kEdramTileCount) {
    // The ownership change extent goes to the next EDRAM addressing period.
    change_ownership_in_extent(
        0, std::min(end_tiles & (xenos::kEdramTileCount - 1), start_tiles));
  }
}

}  // namespace gpu
}  // namespace xe
