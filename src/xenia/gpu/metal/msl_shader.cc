/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/gpu/metal/msl_shader.h"

#include <dispatch/dispatch.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

#include "spirv2air.h"
#include "spirv_msl.hpp"
#include "third_party/fmt/include/fmt/format.h"
#include "third_party/xxhash/xxhash.h"

#include "xenia/base/assert.h"
#include "xenia/base/filesystem.h"
#include "xenia/base/logging.h"
#include "xenia/gpu/gpu_flags.h"
#include "xenia/gpu/metal/metal_perf_counters.h"
#include "xenia/gpu/metal/metal_shader_cache.h"
#include "xenia/gpu/metal/msl_bindings.h"
#include "xenia/gpu/metal/spirv2air_self_test_spirv.h"

DEFINE_bool(
    metal_spirvcross_strip_nocontract_optnone, true,
    "When using the Metal SPIRV-Cross path, strip [[clang::optnone]] from "
    "NoContraction float helper wrappers (spvFAdd/spvFSub/spvFMul and matrix "
    "variants). This avoids severe strict-math performance regressions on some "
    "Apple GPUs while keeping NoContraction semantics enabled.",
    "GPU");
DEFINE_bool(
    metal_shader_preserve_invariance, true,
    "Compile Metal shaders with preserveInvariance, so vertex positions the "
    "guest marks invariant come out bit-identical from different vertex "
    "shaders. Without it, fast math may compute the same position slightly "
    "differently in a depth pre-pass and the pass drawing over it, and "
    "equal-depth tests and decals flicker (z-fighting).",
    "GPU");
DEFINE_bool(
    metal_vertex_shader_fast_math, false,
    "Compile vertex shaders with fast math too (with metal_shader_fast_math). "
    "Off keeps positions bit-identical between shaders that compute them with "
    "different but equivalent instructions: games draw a depth pass and then "
    "the same geometry with an equal-depth test (Spider-Man: Edge of Time's "
    "hypersense copies), and fast math lets the compiler fold the helpers that "
    "keep a multiply and an add separately rounded (see "
    "metal_spirvcross_strip_nocontract_optnone) and fuse them differently in "
    "each shader, so the second pass fails the depth test.",
    "GPU");
DEFINE_bool(
    metal_shader_fast_math, true,
    "Compile Metal shaders with fast math, which lets the compiler assume no "
    "value is NaN or infinite. Xbox 360 shaders use infinities (1/0, log of "
    "0) and rely on 0 times anything being 0, and the checks for that may be "
    "optimized away under fast math. Off: IEEE math, exact with infinities "
    "but slower shaders; try it per game if geometry stretches out to "
    "infinity or glow and bloom effects are missing. Applies to shaders "
    "compiled afterwards.",
    "GPU");
DEFINE_string(
    metal_log_shader_hashes, "",
    "Comma-separated vertex or pixel shader ucode hashes (hex, as logged, for "
    "example 6C76CC934779C668) whose Xbox 360 microcode disassembly and "
    "generated Metal source are written to the log when they are translated "
    "(\"Shader log\" lines).",
    "Metal");
DEFINE_bool(
    metal_share_argument_encoders, true,
    "Share one texture/sampler argument encoder between Metal shaders whose "
    "argument buffer layout is the same, instead of asking Metal for one per "
    "shader. Metal reflects the compiled function for that, which on a cold "
    "shader cache (first run, app update, reboot) adds tens to hundreds of ms "
    "to most pixel shader compiles. Off: one per shader, as before.",
    "Metal");

DEFINE_bool(
    metal_spirv2air_probe, false,
    "Test spirv2air (third_party/spirv2air, made to target iOS 16): also "
    "compile each guest shader's SPIR-V to a Metal library with it, without "
    "drawing with it, and log whether spirv2air translates it, whether Metal "
    "accepts the library and its function, and the time taken against the "
    "Metal Shading Language compile of the same shader (\"spirv2air probe\" "
    "lines). The first modules it rejects or Metal refuses are written to "
    "spirv2air_probe in the title's shader storage.",
    "Metal");
DEFINE_bool(
    metal_spirv2air_self_test, false,
    "Before a title starts, build Metal pipelines one at a time from small "
    "shaders compiled with spirv2air, with each "
    "metal_spirv2air_declaration_attributes mode from 3 to 1, and from the "
    "shaders of pipelines Metal refused that earlier runs wrote to "
    "spirv2air_probe, and log which ones Metal builds (\"spirv2air "
    "self-test\" lines). The title waits for it; a pipeline Metal can't "
    "build can take 20 s or more to fail, and it stops after 10 minutes.",
    "Metal");
DEFINE_int32(
    metal_spirv2air_options, 3,
    "With metal_shader_compiler \"spirv2air\", the sum of:\n"
    " 1: shared memory as a buffer binding instead of a pointer in the "
    "argument buffer.\n"
    " 2: textures and samplers as texture and sampler bindings instead of "
    "handles in heaps.\n"
    " 4: the heaps' index records read through a constant pointer instead of "
    "a device one (without 2).\n"
    " 8: precise math functions instead of the fast ones.\n"
    "1 and 2 bind as Metal Shading Language shaders do.",
    "Metal");
DEFINE_int32(
    metal_spirv2air_declaration_attributes, 2,
    "With metal_shader_compiler \"spirv2air\", the attributes on the "
    "declarations of the Metal intrinsics a shader calls:\n"
    " 0: the ones Apple's current compiler writes.\n"
    " 1: without mustprogress.\n"
    " 2: without mustprogress, willreturn, nofree and nosync.\n"
    " 3: none.\n"
    "iOS 16's Metal compiler failed every spirv2air shader with a call with "
    "0 or 1. 2 keeps convergent, which stops the compiler from moving "
    "derivatives and implicit-LOD samples into divergent branches.",
    "Metal");
DEFINE_string(
    metal_shader_compiler, "msl",
    "Guest shader compiler.\n"
    " msl: SPIR-V -> Metal Shading Language with SPIRV-Cross, compiled by "
    "Metal.\n"
    " spirv2air: SPIR-V -> AIR directly (third_party/spirv2air), falling back "
    "to msl for shaders it can't translate or Metal refuses, and for "
    "tessellated draws. Its libraries are kept with metal_shader_disk_cache.",
    "Metal");

namespace xe {
namespace gpu {
namespace metal {

namespace {

constexpr uint32_t kMslSourceCacheMagic = 0x5843534D;  // 'MSCX'
constexpr uint32_t kMslSourceCacheVersion = 1;
constexpr uint32_t kMslSourceCacheMaxBytes = 16 * 1024 * 1024;
// Bump when SPIRV-Cross -> MSL resource remapping or codegen-affecting options
// change so stale on-disk MSL source can't be reused with incompatible runtime
// bindings.
constexpr uint32_t kMslSourceCacheSchemaVersion = 5;

std::mutex g_msl_source_cache_mutex;
std::filesystem::path g_msl_source_cache_directory;

std::filesystem::path GetMslSourceCachePathLocked(uint64_t cache_key) {
  char name[32];
  std::snprintf(name, sizeof(name), "%016llX.mslsrcache",
                static_cast<unsigned long long>(cache_key));
  return g_msl_source_cache_directory / name;
}

// spirv2air totals (metal_spirv2air_probe or metal_shader_compiler
// "spirv2air"), under g_spirv2air_probe_mutex.
struct Spirv2airProbeStats {
  uint32_t shaders = 0;
  uint32_t translated = 0;
  uint32_t libraries = 0;
  uint32_t functions = 0;
  // Of the shaders whose library came from metal_shader_disk_cache.
  uint32_t cached = 0;
  // Of the shaders Metal accepted the function of.
  double translate_ms = 0.0;
  double library_ms = 0.0;
  double msl_library_ms = 0.0;
  std::map<std::string, uint32_t> rejections;
  std::map<std::string, uint32_t> library_errors;
  uint32_t dumped = 0;
  // Pipelines DumpSpirv2airPipelineShaders wrote the shaders of, and their
  // shaders' hashes and modifications.
  uint32_t pipelines_dumped = 0;
  std::set<std::array<uint64_t, 4>> dumped_pipeline_shaders;
};
std::mutex g_spirv2air_probe_mutex;
Spirv2airProbeStats g_spirv2air_probe_stats;

// Bump when GetSpirv2airOptions changes, for the cached libraries (the
// metal_spirv2air_options variant is part of the key too).
constexpr uint64_t kSpirv2airOptionsRevision = 2;

// spirv2air options for this backend's SPIR-V and draw path
// (Spirv2airLayout): seven constant buffers, so the slots after them move up
// by 8 bytes from spirv2air's defaults, and the vertex index and position as
// SPIRV-Cross gives them to the Metal Shading Language shaders. variant: the
// metal_spirv2air_options bits (Spirv2airLayout).
spirv2air::Options GetSpirv2airOptions(bool is_ios, uint32_t variant) {
  spirv2air::Options options;
  options.platform =
      is_ios ? spirv2air::Platform::IOS : spirv2air::Platform::MacOS;
  options.minOsMajor = is_ios ? 16 : 14;
  options.vertexIndexRelativeToBase = false;
  options.positionInfToNan = false;
  spirv2air::GraphicsBindings& bindings = options.graphics;
  bindings.resourceHeapBuffer = Spirv2airLayout::kResourceHeapBuffer;
  bindings.samplerHeapBuffer = Spirv2airLayout::kSamplerHeapBuffer;
  bindings.argumentBuffer = Spirv2airLayout::kArgumentBuffer;
  // Tessellation stays on the Metal Shading Language path, and the EDRAM
  // binding (set 0 binding 1, the counter's by default) is only declared with
  // fragment shader interlock, which this path doesn't use.
  bindings.controlPointBuffer = -1;
  bindings.occlusionCounterBuffer = -1;
  bindings.constantBufferCount = Spirv2airLayout::kConstantBufferCount;
  bindings.sharedMemoryReadOffset = Spirv2airLayout::kSharedMemoryReadOffset;
  bindings.sharedMemoryWriteOffset = Spirv2airLayout::kSharedMemoryWriteOffset;
  bindings.vertexTextureIndicesOffset =
      Spirv2airLayout::kVertexTextureIndicesOffset;
  bindings.fragmentTextureIndicesOffset =
      Spirv2airLayout::kFragmentTextureIndicesOffset;
  bindings.descriptorStride = Spirv2airLayout::kHeapRecordBytes;
  bindings.textureHandleOffset = Spirv2airLayout::kHeapTextureHandleOffset;
  bindings.samplerHandleOffset = 0;
  if (variant & Spirv2airLayout::kDirectSharedMemory) {
    bindings.sharedMemoryBuffer = int(MslBufferIndex::kSharedMemory);
  }
  bindings.directTextures = (variant & Spirv2airLayout::kDirectTextures) != 0;
  bindings.constantTextureIndices =
      (variant & Spirv2airLayout::kConstantTextureIndices) != 0;
  options.preciseMath = (variant & Spirv2airLayout::kPreciseMath) != 0;
  options.declarationAttributes =
      int((variant >> Spirv2airLayout::kDeclarationAttributesShift) & 3);
  return options;
}

// The variant guest shaders are compiled with: metal_spirv2air_options and
// metal_spirv2air_declaration_attributes.
uint32_t GetSpirv2airVariant() {
  return (uint32_t(cvars::metal_spirv2air_options) &
          Spirv2airLayout::kOptionMask) |
         ((uint32_t(cvars::metal_spirv2air_declaration_attributes) & 3)
          << Spirv2airLayout::kDeclarationAttributesShift);
}

// The cache key seed of a variant's libraries.
uint64_t GetSpirv2airCacheSeed(bool is_ios, uint32_t variant) {
  return XXH3_64bits_withSeed(
      spirv2air::kVersion, std::strlen(spirv2air::kVersion),
      ((kSpirv2airOptionsRevision << 8) | variant) * 2 + (is_ios ? 1 : 0));
}

// The execution model of the module's first entry point.
spv::ExecutionModel GetSpirvExecutionModel(const uint32_t* words,
                                           size_t word_count) {
  for (size_t i = 5; i < word_count;) {
    const uint32_t word_count_op = words[i] >> 16;
    if (!word_count_op) {
      break;
    }
    if ((words[i] & 0xFFFF) == uint32_t(spv::OpEntryPoint) &&
        i + 1 < word_count) {
      return spv::ExecutionModel(words[i + 1]);
    }
    i += word_count_op;
  }
  return spv::ExecutionModelMax;
}

// Compiles a shader's SPIR-V with spirv2air (GetSpirv2airOptions with the
// metal_spirv2air_options bits in variant) and loads
// the library and its function "main", recording in the spirv2air totals
// whether spirv2air translates it and Metal accepts it, and the time taken
// (against the Metal Shading Language compile of the same shader for the
// probe, msl_library_ms). With cache_key, the library is looked up in and
// stored to metal_shader_disk_cache. The first modules it rejects or Metal
// refuses are written to spirv2air_probe in the title's shader storage.
// Returns false, with nothing to release, where it fails.
bool CompileSpirv2air(MTL::Device* device, bool is_ios, uint32_t variant,
                      const uint32_t* words, size_t word_count, bool is_vertex,
                      uint64_t shader_hash, uint64_t modification,
                      double msl_library_ms, uint64_t cache_key,
                      MTL::Library** library_out,
                      MTL::Function** function_out) {
  *library_out = nullptr;
  *function_out = nullptr;
  auto to_ms = [](std::chrono::steady_clock::duration duration) {
    return std::chrono::duration<double, std::milli>(duration).count();
  };

  const auto translate_begin = std::chrono::steady_clock::now();
  spirv2air::Result result;
  bool cached = false;
  if (cache_key) {
    MetalShaderCache::CachedMetallib cached_metallib;
    if (g_metal_shader_cache->Load(cache_key, &cached_metallib) &&
        cached_metallib.function_name == "main" &&
        !cached_metallib.metallib_data.empty()) {
      result.metallib = std::move(cached_metallib.metallib_data);
      cached = true;
    }
  }
  if (!cached) {
    result = spirv2air::translate(words, word_count,
                                  GetSpirv2airOptions(is_ios, variant));
  }
  const double translate_ms =
      to_ms(std::chrono::steady_clock::now() - translate_begin);

  std::string library_error;
  MTL::Library* library = nullptr;
  MTL::Function* function = nullptr;
  double library_ms = 0.0;
  if (result.ok()) {
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    const auto library_begin = std::chrono::steady_clock::now();
    dispatch_data_t data =
        dispatch_data_create(result.metallib.data(), result.metallib.size(),
                             nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    NS::Error* error = nullptr;
    library = device->newLibrary(data, &error);
    dispatch_release(data);
    if (library) {
      function = library->newFunction(
          NS::String::string("main", NS::UTF8StringEncoding));
      if (!function) {
        library_error = "no function main";
        library->release();
        library = nullptr;
      }
    } else {
      library_error = error && error->localizedDescription()
                          ? error->localizedDescription()->utf8String()
                          : "unknown error";
    }
    library_ms = to_ms(std::chrono::steady_clock::now() - library_begin);
    pool->release();
    if (function && cache_key && !cached) {
      g_metal_shader_cache->Store(cache_key, "main", result.metallib.data(),
                                  result.metallib.size());
    }
  }

  const char* stage = is_vertex ? "VS" : "PS";
  std::lock_guard<std::mutex> lock(g_spirv2air_probe_mutex);
  Spirv2airProbeStats& stats = g_spirv2air_probe_stats;
  ++stats.shaders;
  bool dump = false;
  if (!result.ok()) {
    std::string reason = result.error.substr(0, 120);
    if (stats.rejections[reason]++ == 0) {
      XELOGW("spirv2air: rejected {} {:016X} mod {:016X}: {}", stage,
             shader_hash, modification, result.error);
    }
    dump = true;
  } else {
    ++stats.translated;
    if (library) {
      ++stats.libraries;
    }
    if (function) {
      ++stats.functions;
      if (cached) {
        ++stats.cached;
      }
      stats.translate_ms += translate_ms;
      stats.library_ms += library_ms;
      stats.msl_library_ms += msl_library_ms;
    } else {
      std::string reason = library_error.substr(0, 200);
      if (stats.library_errors[reason]++ == 0) {
        XELOGW(
            "spirv2air: Metal refused the library of {} {:016X} mod {:016X}: "
            "{}",
            stage, shader_hash, modification, library_error);
      }
      dump = true;
    }
  }
  if (dump && stats.dumped < 16 && !g_msl_source_cache_directory.empty()) {
    ++stats.dumped;
    std::filesystem::path dump_dir =
        g_msl_source_cache_directory.parent_path() / "spirv2air_probe";
    std::error_code ec;
    std::filesystem::create_directories(dump_dir, ec);
    std::string base =
        fmt::format("{}_{:016X}_{:016X}", stage, shader_hash, modification);
    std::ofstream spv(dump_dir / (base + ".spv"), std::ios::binary);
    spv.write(reinterpret_cast<const char*>(words),
              std::streamsize(word_count * sizeof(uint32_t)));
    if (!result.metallib.empty()) {
      std::ofstream metallib(dump_dir / (base + ".metallib"), std::ios::binary);
      metallib.write(reinterpret_cast<const char*>(result.metallib.data()),
                     std::streamsize(result.metallib.size()));
    }
  }
  if (stats.shaders == 1 || stats.shaders % 25 == 0) {
    std::string rejections;
    for (const auto& [reason, count] : stats.rejections) {
      rejections += fmt::format("; {}x {}", count, reason);
    }
    double accepted = std::max(stats.functions, 1u);
    std::string msl_compile;
    if (stats.msl_library_ms > 0.0) {
      msl_compile = fmt::format(" vs Metal Shading Language compile {:.2f} ms",
                                stats.msl_library_ms / accepted);
    }
    XELOGI(
        "spirv2air {} ({}, iOS 16 target): {} shaders, {} translated, {} "
        "libraries and {} functions accepted by Metal ({} from the disk "
        "cache); per accepted shader spirv2air {:.2f} ms + library {:.2f} "
        "ms{}; {} rejected{}",
        cvars::metal_shader_compiler == "spirv2air" ? "compiler" : "probe",
        spirv2air::kVersion, stats.shaders, stats.translated, stats.libraries,
        stats.functions, stats.cached, stats.translate_ms / accepted,
        stats.library_ms / accepted, msl_compile,
        stats.shaders - stats.translated, rejections);
  }
  *library_out = library;
  *function_out = function;
  return function != nullptr;
}

// metal_spirv2air_probe: compiles a shader's SPIR-V with spirv2air as well
// and loads the library, without drawing with it.
void ProbeSpirv2air(MTL::Device* device, bool is_ios, const uint32_t* words,
                    size_t word_count, bool is_vertex, uint64_t shader_hash,
                    uint64_t modification, double msl_library_ms) {
  MTL::Library* library = nullptr;
  MTL::Function* function = nullptr;
  if (CompileSpirv2air(device, is_ios, GetSpirv2airVariant(), words, word_count,
                       is_vertex, shader_hash, modification, msl_library_ms, 0,
                       &library, &function)) {
    function->release();
    library->release();
  }
}

bool LoadCachedMslSource(uint64_t cache_key, std::string* source_out) {
  if (!source_out) {
    return false;
  }
  std::filesystem::path path;
  {
    std::lock_guard<std::mutex> lock(g_msl_source_cache_mutex);
    if (g_msl_source_cache_directory.empty()) {
      return false;
    }
    path = GetMslSourceCachePathLocked(cache_key);
  }

  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return false;
  }

  uint32_t magic = 0;
  uint32_t version = 0;
  uint64_t key = 0;
  uint32_t source_size = 0;
  file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
  file.read(reinterpret_cast<char*>(&version), sizeof(version));
  file.read(reinterpret_cast<char*>(&key), sizeof(key));
  file.read(reinterpret_cast<char*>(&source_size), sizeof(source_size));
  if (!file || magic != kMslSourceCacheMagic ||
      version != kMslSourceCacheVersion || key != cache_key ||
      source_size > kMslSourceCacheMaxBytes) {
    return false;
  }

  source_out->resize(source_size);
  file.read(source_out->data(), source_size);
  return bool(file);
}

bool StoreCachedMslSource(uint64_t cache_key, std::string_view source) {
  if (source.empty() || source.size() > kMslSourceCacheMaxBytes) {
    return false;
  }

  std::filesystem::path path;
  {
    std::lock_guard<std::mutex> lock(g_msl_source_cache_mutex);
    if (g_msl_source_cache_directory.empty()) {
      return false;
    }
    path = GetMslSourceCachePathLocked(cache_key);
  }
  std::filesystem::path tmp_path = path;
  tmp_path += ".tmp";

  std::ofstream file(tmp_path, std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    return false;
  }
  uint32_t magic = kMslSourceCacheMagic;
  uint32_t version = kMslSourceCacheVersion;
  uint64_t key = cache_key;
  uint32_t source_size = static_cast<uint32_t>(source.size());
  file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
  file.write(reinterpret_cast<const char*>(&version), sizeof(version));
  file.write(reinterpret_cast<const char*>(&key), sizeof(key));
  file.write(reinterpret_cast<const char*>(&source_size), sizeof(source_size));
  file.write(source.data(), source.size());
  file.close();

  std::error_code ec;
  std::filesystem::rename(tmp_path, path, ec);
  if (ec) {
    std::filesystem::remove(tmp_path, ec);
    return false;
  }
  return true;
}

size_t ReplaceAllInPlace(std::string& text, std::string_view from,
                         std::string_view to) {
  if (from.empty()) {
    return 0;
  }
  size_t replacement_count = 0;
  size_t position = 0;
  while ((position = text.find(from, position)) != std::string::npos) {
    text.replace(position, from.size(), to);
    position += to.size();
    ++replacement_count;
  }
  return replacement_count;
}

size_t StripNoContractionOptnoneWrappers(std::string& msl_source) {
  // Keep this narrowly scoped to the NoContraction helper wrappers that are
  // known to cause significant slowdowns with strict math.
  static constexpr std::pair<std::string_view, std::string_view> kPatterns[] = {
      {"[[clang::optnone]] T spvFAdd(T l, T r)", "T spvFAdd(T l, T r)"},
      {"[[clang::optnone]] T spvFSub(T l, T r)", "T spvFSub(T l, T r)"},
      {"[[clang::optnone]] T spvFMul(T l, T r)", "T spvFMul(T l, T r)"},
      {"[[clang::optnone]] vec<T, Cols> spvFMulVectorMatrix(vec<T, Rows> v, "
       "matrix<T, Cols, Rows> m)",
       "vec<T, Cols> spvFMulVectorMatrix(vec<T, Rows> v, matrix<T, Cols, Rows> "
       "m)"},
      {"[[clang::optnone]] vec<T, Rows> spvFMulMatrixVector(matrix<T, Cols, "
       "Rows> m, vec<T, Cols> v)",
       "vec<T, Rows> spvFMulMatrixVector(matrix<T, Cols, Rows> m, vec<T, Cols> "
       "v)"},
      {"[[clang::optnone]] matrix<T, RCols, LRows> spvFMulMatrixMatrix("
       "matrix<T, LCols, LRows> l, matrix<T, RCols, RRows> r)",
       "matrix<T, RCols, LRows> spvFMulMatrixMatrix(matrix<T, LCols, LRows> l, "
       "matrix<T, RCols, RRows> r)"},
  };
  size_t replacements = 0;
  for (const auto& pattern : kPatterns) {
    replacements +=
        ReplaceAllInPlace(msl_source, pattern.first, pattern.second);
  }
  return replacements;
}

uint64_t GetMslSourceCacheKey(const MslShader::MslTranslation& translation,
                              bool is_ios, uint32_t msl_major,
                              uint32_t msl_minor,
                              bool ios_support_base_vertex_instance,
                              bool emulate_cube_array,
                              bool ios_use_simdgroup_functions,
                              bool use_argument_buffers,
                              uint8_t argument_buffers_tier) {
  uint64_t translated_spirv_hash = 0;
  const auto& translated_spirv = translation.translated_binary();
  if (!translated_spirv.empty()) {
    translated_spirv_hash =
        XXH3_64bits(translated_spirv.data(), translated_spirv.size());
  }

  struct KeyData {
    uint64_t shader_hash;
    uint64_t modification;
    uint64_t translated_spirv_hash;
    uint32_t stage;
    uint8_t is_ios;
    uint8_t msl_major;
    uint8_t msl_minor;
    uint8_t ios_support_base_vertex_instance;
    uint8_t emulate_cube_array;
    uint8_t ios_use_simdgroup_functions;
    uint8_t use_argument_buffers;
    uint8_t argument_buffers_tier;
    uint16_t reserved0;
    uint32_t cache_schema_version;
  } key_data = {};
  key_data.shader_hash = translation.shader().ucode_data_hash();
  key_data.modification = translation.modification();
  key_data.translated_spirv_hash = translated_spirv_hash;
  key_data.stage = static_cast<uint32_t>(translation.shader().type());
  key_data.is_ios = is_ios ? 1 : 0;
  key_data.msl_major = static_cast<uint8_t>(msl_major);
  key_data.msl_minor = static_cast<uint8_t>(msl_minor);
  key_data.ios_support_base_vertex_instance =
      ios_support_base_vertex_instance ? 1 : 0;
  key_data.emulate_cube_array = emulate_cube_array ? 1 : 0;
  key_data.ios_use_simdgroup_functions = ios_use_simdgroup_functions ? 1 : 0;
  key_data.use_argument_buffers = use_argument_buffers ? 1 : 0;
  key_data.argument_buffers_tier = argument_buffers_tier;
  key_data.cache_schema_version = kMslSourceCacheSchemaVersion;
  return XXH3_64bits(&key_data, sizeof(key_data));
}

}  // namespace

bool ShouldLogMslShader(uint64_t shader_hash) {
  const std::string& list = cvars::metal_log_shader_hashes;
  size_t begin = 0;
  while (begin < list.size()) {
    size_t end = list.find(',', begin);
    if (end == std::string::npos) {
      end = list.size();
    }
    std::string entry = list.substr(begin, end - begin);
    entry.erase(std::remove_if(entry.begin(), entry.end(),
                               [](char c) { return std::isspace(uint8_t(c)); }),
                entry.end());
    if (!entry.empty() &&
        std::strtoull(entry.c_str(), nullptr, 16) == shader_hash) {
      return true;
    }
    begin = end + 1;
  }
  return false;
}

void SetMslShaderSourceCacheDirectory(const std::filesystem::path& cache_dir) {
  std::lock_guard<std::mutex> lock(g_msl_source_cache_mutex);
  g_msl_source_cache_directory = cache_dir;
  if (g_msl_source_cache_directory.empty()) {
    return;
  }
  std::error_code ec;
  std::filesystem::create_directories(g_msl_source_cache_directory, ec);
  if (ec) {
    XELOGW("MslShader: Failed to create source cache directory {}: {}",
           g_msl_source_cache_directory.string(), ec.message());
    g_msl_source_cache_directory.clear();
  }
}

void ClearMslShaderSourceCacheDirectory() {
  std::lock_guard<std::mutex> lock(g_msl_source_cache_mutex);
  g_msl_source_cache_directory.clear();
}

void RunSpirv2airSelfTest(MTL::Device* device, bool is_ios,
                          const std::filesystem::path& probe_dir) {
  namespace test = spirv2air_self_test;
  struct Module {
    std::string name;
    std::vector<uint32_t> words;
  };
  auto module_of = [](const char* name, const uint32_t* words,
                      size_t word_count) {
    return Module{name, std::vector<uint32_t>(words, words + word_count)};
  };
  // 0-2 vertex shaders, 3-13 pixel shaders, then the shaders from probe_dir.
  std::vector<Module> modules = {
      module_of("vertex shader writing only the position",
                test::kVertexPosition, std::size(test::kVertexPosition)),
      module_of("vertex shader with two outputs", test::kVertexOutputs,
                std::size(test::kVertexOutputs)),
      module_of("vertex shader reading shared memory",
                test::kVertexSharedMemory,
                std::size(test::kVertexSharedMemory)),
      module_of("pixel shader writing a constant", test::kPixelConstant,
                std::size(test::kPixelConstant)),
      module_of("pixel shader with an input", test::kPixelInput,
                std::size(test::kPixelInput)),
      module_of("pixel shader with a centroid input", test::kPixelCentroidInput,
                std::size(test::kPixelCentroidInput)),
      module_of("pixel shader reading a constant buffer",
                test::kPixelConstantBuffer,
                std::size(test::kPixelConstantBuffer)),
      module_of("pixel shader indexing a constant buffer",
                test::kPixelConstantBufferIndexed,
                std::size(test::kPixelConstantBufferIndexed)),
      module_of("pixel shader reading shared memory", test::kPixelSharedMemory,
                std::size(test::kPixelSharedMemory)),
      module_of("pixel shader with math functions", test::kPixelMath,
                std::size(test::kPixelMath)),
      module_of("pixel shader sampling a texture", test::kPixelTexture,
                std::size(test::kPixelTexture)),
      module_of("pixel shader sampling a texture array with gradients",
                test::kPixelTextureArrayGradients,
                std::size(test::kPixelTextureArrayGradients)),
      module_of("pixel shader with discard", test::kPixelDiscard,
                std::size(test::kPixelDiscard)),
      module_of("pixel shader with early fragment tests",
                test::kPixelEarlyFragmentTests,
                std::size(test::kPixelEarlyFragmentTests)),
  };
  // A pipeline: its vertex and pixel shader (-1: none) as modules indices, and
  // the variant (Spirv2airLayout bits) both are translated with.
  struct Test {
    std::string name;
    int vertex;
    int pixel;
    uint32_t variant;
  };
  constexpr uint32_t kSharedMemory = Spirv2airLayout::kDirectSharedMemory;
  constexpr uint32_t kTextures = Spirv2airLayout::kDirectTextures;
  constexpr uint32_t kAttributesShift =
      Spirv2airLayout::kDeclarationAttributesShift;
  // Shaders with no calls, which built on builds 9697 and 9698.
  const std::vector<Test> baseline = {
      {"vertex shader writing only the position, no pixel shader", 0, -1, 0},
      {"vertex shader reading shared memory, no pixel shader", 2, -1,
       kSharedMemory},
      {"pixel shader with an input", 1, 4, 0},
      {"pixel shader reading a constant buffer", 0, 6, 0},
  };
  // Shaders calling Metal intrinsics, which all failed on build 9698 with the
  // declaration attributes Apple's current compiler writes.
  const std::vector<Test> with_calls = {
      {"pixel shader indexing a constant buffer", 1, 7, 0},
      {"pixel shader with discard", 1, 12, 0},
      {"pixel shader with math functions", 1, 9, 0},
      {"pixel shader reading shared memory", 1, 8, kSharedMemory},
      {"pixel shader sampling a texture", 1, 10, kTextures},
      {"pixel shader sampling a texture array with gradients", 1, 11,
       kTextures},
  };
  // The pairs of shaders of pipelines Metal refused in games
  // (DumpSpirv2airPipelineShaders), by pair: the vertex and pixel shader file.
  std::map<std::string, std::array<std::filesystem::path, 2>> dumps;
  std::error_code ec;
  for (const auto& entry : std::filesystem::directory_iterator(probe_dir, ec)) {
    // pair_<16 digits>_<VS or PS>_...
    const std::string file_name = entry.path().filename().string();
    if (file_name.rfind("pair_", 0) != 0 || file_name.size() < 24 ||
        entry.path().extension() != ".spv") {
      continue;
    }
    const std::string stage = file_name.substr(22, 2);
    if (stage == "VS" || stage == "PS") {
      dumps[file_name.substr(0, 21)][stage == "PS"] = entry.path();
    }
  }
  auto read_words = [](const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::vector<char> bytes((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
    std::vector<uint32_t> words(bytes.size() / sizeof(uint32_t));
    std::memcpy(words.data(), bytes.data(), words.size() * sizeof(uint32_t));
    return words;
  };
  // The vertex and pixel shader (-1: none) modules index of each pair.
  std::vector<std::pair<int, int>> game_pairs;
  for (const auto& [pair_name, paths] : dumps) {
    if (paths[0].empty()) {
      continue;
    }
    const int vertex_index = int(modules.size());
    modules.push_back({paths[0].stem().string(), read_words(paths[0])});
    int pixel_index = -1;
    if (!paths[1].empty()) {
      pixel_index = int(modules.size());
      modules.push_back({paths[1].stem().string(), read_words(paths[1])});
    }
    game_pairs.push_back({vertex_index, pixel_index});
  }

  auto to_ms = [](std::chrono::steady_clock::duration duration) {
    return std::chrono::duration<double, std::milli>(duration).count();
  };
  auto error_text = [](NS::Error* error) -> std::string {
    return error && error->localizedDescription()
               ? error->localizedDescription()->utf8String()
               : "unknown error";
  };
  std::vector<MTL::Library*> libraries;
  // By module index and variant; null where it didn't load.
  std::map<std::pair<int, uint32_t>, MTL::Function*> functions;
  auto function_of = [&](int index, uint32_t variant) -> MTL::Function* {
    auto [it, inserted] = functions.try_emplace({index, variant}, nullptr);
    if (!inserted) {
      return it->second;
    }
    const Module& module = modules[index];
    spirv2air::Result result =
        spirv2air::translate(module.words.data(), module.words.size(),
                             GetSpirv2airOptions(is_ios, variant));
    if (!result.ok()) {
      XELOGW(
          "spirv2air self-test: spirv2air can't translate the {} (variant "
          "{}): {}",
          module.name, variant, result.error);
      return nullptr;
    }
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    dispatch_data_t data =
        dispatch_data_create(result.metallib.data(), result.metallib.size(),
                             nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    NS::Error* error = nullptr;
    MTL::Library* library = device->newLibrary(data, &error);
    dispatch_release(data);
    if (library) {
      libraries.push_back(library);
      it->second = library->newFunction(
          NS::String::string("main", NS::UTF8StringEncoding));
    } else {
      XELOGW(
          "spirv2air self-test: Metal refused the library of the {} "
          "(variant {}): {}",
          module.name, variant, error_text(error));
    }
    pool->release();
    return it->second;
  };

  // Metal's compiler service hung for up to an hour on a pipeline on build
  // 9698, so a pipeline gets kAnswerTimeout, and the self-test stops once one
  // doesn't answer or kTimeBudget is spent.
  constexpr auto kAnswerTimeout = std::chrono::seconds(40);
  constexpr auto kTimeBudget = std::chrono::minutes(10);
  const auto self_test_begin = std::chrono::steady_clock::now();
  bool stopped = false;
  struct Answer {
    std::mutex mutex;
    std::condition_variable cv;
    bool answered = false;
    bool built = false;
    std::string error;
  };
  // Builds a pipeline from the functions and logs the result (a success only
  // with log_success). Returns whether it built.
  auto build = [&](const std::string& name, uint32_t variant,
                   MTL::Function* vertex_function,
                   MTL::Function* fragment_function, bool log_success) {
    if (stopped) {
      return false;
    }
    if (std::chrono::steady_clock::now() - self_test_begin > kTimeBudget) {
      XELOGW("spirv2air self-test: stopping, 10 minutes spent");
      stopped = true;
      return false;
    }
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    MTL::RenderPipelineDescriptor* desc =
        MTL::RenderPipelineDescriptor::alloc()->init();
    desc->setVertexFunction(vertex_function);
    if (fragment_function) {
      desc->setFragmentFunction(fragment_function);
      desc->colorAttachments()->object(0)->setPixelFormat(
          MTL::PixelFormatRGBA8Unorm);
    }
    desc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
    auto answer = std::make_shared<Answer>();
    const auto begin = std::chrono::steady_clock::now();
    device->newRenderPipelineState(
        desc, [answer, error_text](MTL::RenderPipelineState* pipeline,
                                   NS::Error* error) {
          std::lock_guard<std::mutex> lock(answer->mutex);
          answer->answered = true;
          answer->built = pipeline != nullptr;
          if (!pipeline) {
            answer->error = error_text(error);
          }
          answer->cv.notify_all();
        });
    desc->release();
    pool->release();
    std::unique_lock<std::mutex> lock(answer->mutex);
    if (!answer->cv.wait_for(lock, kAnswerTimeout,
                             [&]() { return answer->answered; })) {
      XELOGW(
          "spirv2air self-test, {} (variant {}): no answer in 40 s; stopping, "
          "as Metal's compiler service is stuck",
          name, variant);
      stopped = true;
      return false;
    }
    const double pipeline_ms = to_ms(std::chrono::steady_clock::now() - begin);
    if (!answer->built) {
      XELOGW("spirv2air self-test, {} (variant {}): failed after {:.1f} ms: {}",
             name, variant, pipeline_ms, answer->error);
    } else if (log_success) {
      XELOGI("spirv2air self-test, {} (variant {}): built in {:.1f} ms", name,
             variant, pipeline_ms);
    }
    return answer->built;
  };
  auto run = [&](const Test& t) {
    MTL::Function* vertex_function = function_of(t.vertex, t.variant);
    MTL::Function* fragment_function =
        t.pixel >= 0 ? function_of(t.pixel, t.variant) : nullptr;
    if (!vertex_function || (t.pixel >= 0 && !fragment_function)) {
      XELOGW(
          "spirv2air self-test, {} (variant {}): skipped, a shader didn't "
          "load",
          t.name, t.variant);
      return false;
    }
    return build(t.name, t.variant, vertex_function, fragment_function, true);
  };
  // A Metal Shading Language pipeline Metal hasn't seen yet (the constant
  // differs each time and each run), so it is compiled, not taken from
  // Metal's caches.
  uint32_t controls = 0;
  const uint32_t control_salt = uint32_t(
      std::chrono::system_clock::now().time_since_epoch().count() & 0xFFFFFF);
  auto control = [&]() {
    static const char kMslSource[] =
        "#include <metal_stdlib>\n"
        "using namespace metal;\n"
        "struct Varyings {\n"
        "  float4 position [[position]];\n"
        "  float4 texcoord0 [[user(texcoord0)]];\n"
        "};\n"
        "vertex Varyings vs(uint i [[vertex_id]]) {\n"
        "  float2 p = float2(float(i & 1) * 4.0 - 1.0, float(i & 2) * 2.0 - "
        "1.0);\n"
        "  Varyings v;\n"
        "  v.position = float4(p, 0.0, 1.0);\n"
        "  v.texcoord0 = float4(p * 0.5 + 0.5, 0.0, 1.0);\n"
        "  return v;\n"
        "}\n"
        "struct Inputs {\n"
        "  float4 texcoord0 [[user(texcoord0)]];\n"
        "};\n"
        "fragment float4 fs(Inputs in [[stage_in]]) {\n"
        "  return in.texcoord0 * float(CONTROL);\n"
        "}\n";
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    const std::string source = fmt::format(
        "#define CONTROL {}\n{}", control_salt + ++controls, kMslSource);
    MTL::CompileOptions* options = MTL::CompileOptions::alloc()->init();
    NS::Error* error = nullptr;
    MTL::Library* library = device->newLibrary(
        NS::String::string(source.c_str(), NS::UTF8StringEncoding), options,
        &error);
    options->release();
    bool built = false;
    if (library) {
      MTL::Function* vertex_function = library->newFunction(
          NS::String::string("vs", NS::UTF8StringEncoding));
      MTL::Function* fragment_function = library->newFunction(
          NS::String::string("fs", NS::UTF8StringEncoding));
      if (vertex_function && fragment_function) {
        built = build("Metal Shading Language control", 0, vertex_function,
                      fragment_function, controls == 1);
      }
      if (vertex_function) {
        vertex_function->release();
      }
      if (fragment_function) {
        fragment_function->release();
      }
      library->release();
    } else {
      XELOGW(
          "spirv2air self-test: the Metal Shading Language control didn't "
          "compile: {}",
          error_text(error));
    }
    pool->release();
    return built;
  };

  XELOGI(
      "spirv2air self-test ({}): {} pipelines without calls, {} with calls "
      "for each declaration attributes mode, {} pairs of game shaders",
      spirv2air::kVersion, baseline.size(), with_calls.size(),
      game_pairs.size());
  uint32_t built = 0;
  uint32_t failed = 0;
  auto count = [&](bool result) {
    ++(result ? built : failed);
    // A failure can leave Metal's compiler service unable to answer.
    if (!result && !stopped) {
      const auto begin = std::chrono::steady_clock::now();
      if (control()) {
        XELOGI(
            "spirv2air self-test: the compiler builds a new control "
            "afterwards, in {:.1f} ms",
            to_ms(std::chrono::steady_clock::now() - begin));
      }
    }
    return result;
  };
  count(control());
  for (const Test& t : baseline) {
    count(run(t));
  }
  // Most faithful first: 3 writes no attributes, 1 all but mustprogress.
  uint32_t best_mode = 0;
  uint32_t best_built = 0;
  for (uint32_t mode : {3u, 2u, 1u}) {
    uint32_t mode_built = 0;
    for (const Test& t : with_calls) {
      Test variant_test = t;
      variant_test.variant |= mode << kAttributesShift;
      mode_built += count(run(variant_test)) ? 1 : 0;
    }
    XELOGI(
        "spirv2air self-test: declaration attributes {}: {} of {} pipelines "
        "with calls built",
        mode, mode_built, with_calls.size());
    if (mode_built >= best_built && mode_built) {
      best_mode = mode;
      best_built = mode_built;
    }
  }
  if (best_built) {
    const uint32_t game_variant =
        kSharedMemory | kTextures | (best_mode << kAttributesShift);
    for (const auto& [vertex_index, pixel_index] : game_pairs) {
      const std::string& vertex_name = modules[vertex_index].name;
      count(run(
          {"game " + vertex_name + " alone", vertex_index, -1, game_variant}));
      if (pixel_index >= 0) {
        count(run({"game " + vertex_name + " and " + modules[pixel_index].name,
                   vertex_index, pixel_index, game_variant}));
      }
    }
  }
  XELOGI(
      "spirv2air self-test ({}): {} pipelines built, {} failed{}; "
      "declaration attributes {} built the most with calls ({} of {})",
      spirv2air::kVersion, built, failed, stopped ? ", stopped early" : "",
      best_mode, best_built, with_calls.size());
  for (const auto& [key, function] : functions) {
    if (function) {
      function->release();
    }
  }
  for (MTL::Library* library : libraries) {
    library->release();
  }
}

void DumpSpirv2airPipelineShaders(const MslShader::MslTranslation& vertex,
                                  const MslShader::MslTranslation* pixel) {
  std::filesystem::path dump_dir;
  {
    std::lock_guard<std::mutex> lock(g_msl_source_cache_mutex);
    if (g_msl_source_cache_directory.empty()) {
      return;
    }
    dump_dir = g_msl_source_cache_directory.parent_path() / "spirv2air_probe";
  }
  // Pipelines of one pair of shaders differ only in render state. The files
  // are named after the pair, so runs don't mix theirs up.
  const std::array<uint64_t, 4> pair = {
      vertex.shader().ucode_data_hash(), vertex.modification(),
      pixel ? pixel->shader().ucode_data_hash() : 0,
      pixel ? pixel->modification() : 0};
  const uint64_t pair_id = XXH3_64bits(pair.data(), sizeof(pair));
  {
    std::lock_guard<std::mutex> lock(g_spirv2air_probe_mutex);
    Spirv2airProbeStats& stats = g_spirv2air_probe_stats;
    if (stats.pipelines_dumped >= 8 ||
        !stats.dumped_pipeline_shaders.insert(pair).second) {
      return;
    }
    ++stats.pipelines_dumped;
  }
  std::error_code ec;
  std::filesystem::create_directories(dump_dir, ec);
  for (const MslShader::MslTranslation* translation : {&vertex, pixel}) {
    if (!translation) {
      continue;
    }
    std::string base = fmt::format(
        "pair_{:016X}_{}_{:016X}_{:016X}", pair_id,
        translation->shader().type() == xenos::ShaderType::kVertex ? "VS"
                                                                   : "PS",
        translation->shader().ucode_data_hash(), translation->modification());
    const std::vector<uint8_t>& spirv = translation->translated_binary();
    std::ofstream spv(dump_dir / (base + ".spv"), std::ios::binary);
    spv.write(reinterpret_cast<const char*>(spirv.data()),
              std::streamsize(spirv.size()));
  }
  XELOGI(
      "spirv2air: the SPIR-V of the shaders of a pipeline Metal refused is in "
      "spirv2air_probe (pair_{:016X}_*.spv)",
      pair_id);
}

namespace {

// metal_share_argument_encoders: encoders by GetArgumentBufferLayoutKey. Each
// holds a reference of its own, kept for the life of the process (one per
// distinct layout, a few dozen at most).
std::mutex g_argument_encoder_cache_mutex;
std::unordered_map<uint64_t, MTL::ArgumentEncoder*> g_argument_encoder_cache;

// A hash of the argument buffer structs (spvDescriptorSetBuffer<set>) that
// SPIRV-Cross declared in the MSL source - each member's type and [[id]],
// without its name, which only says which fetch constant it is - and of the
// device. The encoder's layout follows from those alone. 0 if the source has
// no such struct or one in an unexpected form (the caller then reflects).
uint64_t GetArgumentBufferLayoutKey(const std::string& msl_source,
                                    const MTL::Device* device) {
  static constexpr std::string_view kStructPrefix =
      "struct spvDescriptorSetBuffer";
  std::string layout;
  size_t struct_begin = 0;
  while ((struct_begin = msl_source.find(kStructPrefix, struct_begin)) !=
         std::string::npos) {
    size_t struct_end = msl_source.find("};", struct_begin);
    size_t line_begin = msl_source.find('\n', struct_begin);
    if (struct_end == std::string::npos || line_begin == std::string::npos ||
        line_begin > struct_end) {
      return 0;
    }
    // The struct's name, with its descriptor set.
    layout.append(msl_source, struct_begin, line_begin - struct_begin);
    layout.push_back('\n');
    ++line_begin;
    while (line_begin < struct_end) {
      size_t line_end = std::min(msl_source.find('\n', line_begin), struct_end);
      std::string_view line(msl_source.data() + line_begin,
                            line_end - line_begin);
      line_begin = line_end + 1;
      size_t id_pos = line.find("[[id(");
      if (id_pos == std::string_view::npos) {
        if (line.find_first_not_of(" \t{\r") != std::string_view::npos) {
          return 0;
        }
        continue;
      }
      size_t name_end = id_pos;
      while (name_end && line[name_end - 1] == ' ') {
        --name_end;
      }
      size_t name_begin = name_end;
      while (name_begin &&
             (std::isalnum(static_cast<unsigned char>(line[name_begin - 1])) ||
              line[name_begin - 1] == '_')) {
        --name_begin;
      }
      if (name_begin == name_end) {
        return 0;
      }
      layout.append(line.substr(0, name_begin));
      layout.append(line.substr(id_pos));
      layout.push_back('\n');
    }
    struct_begin = struct_end;
  }
  if (layout.empty()) {
    return 0;
  }
  const uintptr_t device_address = reinterpret_cast<uintptr_t>(device);
  layout.append(reinterpret_cast<const char*>(&device_address),
                sizeof(device_address));
  return XXH3_64bits(layout.data(), layout.size());
}

}  // namespace

// SPIR-V descriptor set indices matching SpirvShaderTranslator's layout.
// These values must stay in sync with the enum in spirv_shader_translator.h.
namespace SpirvSets {
constexpr uint32_t kSharedMemoryAndEdram = 0;
constexpr uint32_t kConstants = 1;
constexpr uint32_t kTexturesVertex = 2;
constexpr uint32_t kTexturesPixel = 3;
}  // namespace SpirvSets

// SPIR-V constant buffer binding indices within descriptor set 1.
// Must match SpirvShaderTranslator::ConstantBuffer enum.
namespace SpirvCbv {
constexpr uint32_t kSystem = 0;
constexpr uint32_t kFloatVertex = 1;
constexpr uint32_t kFloatPixel = 2;
constexpr uint32_t kBoolLoop = 3;
constexpr uint32_t kFetch = 4;
constexpr uint32_t kClipPlanes = 5;
constexpr uint32_t kTessellation = 6;
}  // namespace SpirvCbv

MslShader::MslShader(xenos::ShaderType shader_type, uint64_t ucode_data_hash,
                     const uint32_t* ucode_dwords, size_t ucode_dword_count,
                     std::endian ucode_source_endian)
    : SpirvShader(shader_type, ucode_data_hash, ucode_dwords, ucode_dword_count,
                  ucode_source_endian) {}

Shader::Translation* MslShader::CreateTranslationInstance(
    uint64_t modification) {
  return new MslTranslation(*this, modification);
}

MslShader::MslTranslation::~MslTranslation() {
  if (argument_encoder_) {
    argument_encoder_->release();
    argument_encoder_ = nullptr;
  }
  if (metal_function_) {
    metal_function_->release();
    metal_function_ = nullptr;
  }
  if (metal_library_) {
    metal_library_->release();
    metal_library_ = nullptr;
  }
  if (spirv2air_function_) {
    spirv2air_function_->release();
    spirv2air_function_ = nullptr;
  }
  if (spirv2air_library_) {
    spirv2air_library_->release();
    spirv2air_library_ = nullptr;
  }
}

// Metal buffer binding indices for resources.
// These must match the SPIRV-Cross remapping done below.
namespace MslBindings {
// Descriptor Set 0: Shared memory and EDRAM.
constexpr uint32_t kSharedMemory = 0;
// Descriptor Set 1: Constant buffers.
constexpr uint32_t kSystemConstants = 1;
constexpr uint32_t kFloatConstantsVertex = 2;
constexpr uint32_t kFloatConstantsPixel = 3;
constexpr uint32_t kBoolLoopConstants = 4;
constexpr uint32_t kFetchConstants = 5;
constexpr uint32_t kClipPlaneConstants = 6;
constexpr uint32_t kTessellationConstants = 7;
// Descriptor Set 2/3: Textures start at this index.
constexpr uint32_t kTextureBase = 0;
// Samplers start at this index.
constexpr uint32_t kSamplerBase = 0;
}  // namespace MslBindings

static const char* GetExecutionModelName(spv::ExecutionModel stage) {
  switch (stage) {
    case spv::ExecutionModelVertex:
      return "vertex";
    case spv::ExecutionModelFragment:
      return "fragment";
    case spv::ExecutionModelTessellationControl:
      return "tess_control";
    case spv::ExecutionModelTessellationEvaluation:
      return "tess_evaluation";
    case spv::ExecutionModelGLCompute:
      return "compute";
    default:
      return "other";
  }
}

static std::string JoinStringList(const std::vector<std::string>& values) {
  std::string joined;
  for (size_t i = 0; i < values.size(); ++i) {
    if (i) {
      joined += ", ";
    }
    joined += values[i];
  }
  return joined;
}

static void AddEntryPointCandidatesFromMslSource(
    const std::string& msl_source, xenos::ShaderType shader_type,
    std::vector<std::string>* candidates) {
  if (!candidates || msl_source.empty()) {
    return;
  }
  const char* expected_stage =
      shader_type == xenos::ShaderType::kVertex ? "vertex" : "fragment";
  // Match typical MSL stage entry declarations generated by SPIRV-Cross:
  //   vertex <ret> <name>(...)
  //   fragment <ret> <name>(...)
  static const std::regex kStageEntryRegex(
      R"((?:^|\n)\s*(vertex|fragment)\s+[^\n\(\{;]*?\b([A-Za-z_][A-Za-z0-9_]*)\s*\()",
      std::regex::optimize);
  for (std::sregex_iterator
           it(msl_source.begin(), msl_source.end(), kStageEntryRegex),
       end;
       it != end; ++it) {
    if (it->size() < 3) {
      continue;
    }
    const std::string stage = (*it)[1].str();
    if (stage != expected_stage) {
      continue;
    }
    const std::string name = (*it)[2].str();
    if (name.empty()) {
      continue;
    }
    if (std::find(candidates->begin(), candidates->end(), name) ==
        candidates->end()) {
      candidates->push_back(name);
    }
  }
  // Match alternate style:
  //   <ret> <name>(...) [[vertex]]
  //   <ret> <name>(...) [[fragment]]
  static const std::regex kAttributedEntryRegex(
      R"((?:^|\n)\s*[^\n\(\{;]*?\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;\{\)]*\)\s*\[\[\s*(vertex|fragment)\s*\]\])",
      std::regex::optimize);
  for (std::sregex_iterator
           it(msl_source.begin(), msl_source.end(), kAttributedEntryRegex),
       end;
       it != end; ++it) {
    if (it->size() < 3) {
      continue;
    }
    const std::string name = (*it)[1].str();
    const std::string stage = (*it)[2].str();
    if (name.empty() || stage != expected_stage) {
      continue;
    }
    if (std::find(candidates->begin(), candidates->end(), name) ==
        candidates->end()) {
      candidates->push_back(name);
    }
  }
}

static void DumpMslResolveFailureSource(const std::string& msl_source,
                                        uint64_t shader_hash,
                                        uint64_t modification,
                                        xenos::ShaderType shader_type) {
  if (msl_source.empty()) {
    return;
  }
  const char* stage_tag =
      shader_type == xenos::ShaderType::kVertex ? "vs" : "ps";
  std::filesystem::path dump_dir;
  if (!cvars::dump_shaders.empty()) {
    dump_dir = std::filesystem::absolute(cvars::dump_shaders) / "msl_shaders" /
               "failures";
  } else {
    dump_dir = std::filesystem::temp_directory_path() / "xenia_msl_failures";
  }
  std::error_code ec;
  std::filesystem::create_directories(dump_dir, ec);
  if (ec) {
    XELOGW("MslShader: Failed to create unresolved-entry dump directory {}: {}",
           dump_dir.string(), ec.message());
    return;
  }
  std::filesystem::path dump_path =
      dump_dir / fmt::format("{}_{:016X}_mod{:016X}_entry_resolve_fail.metal",
                             stage_tag, shader_hash, modification);
  FILE* f = xe::filesystem::OpenFile(dump_path, "w");
  if (!f) {
    XELOGW("MslShader: Failed to open unresolved-entry dump file {}",
           dump_path.string());
    return;
  }
  fwrite(msl_source.data(), 1, msl_source.size(), f);
  fclose(f);
  XELOGW("MslShader: Dumped unresolved-entry MSL source to {}",
         dump_path.string());
}

static void AddResourceBindings(
    spirv_cross::CompilerMSL& compiler, spv::ExecutionModel stage,
    uint32_t texture_descriptor_set, uint64_t shader_hash,
    bool use_argument_buffers,
    std::vector<uint32_t>* sampler_spv_bindings_msl_order) {
  using MSLBinding = spirv_cross::MSLResourceBinding;

  // Set 0: Shared memory (storage buffer at binding 0).
  {
    MSLBinding binding;
    binding.stage = stage;
    binding.desc_set = SpirvSets::kSharedMemoryAndEdram;
    binding.binding = 0;
    binding.msl_buffer = MslBindings::kSharedMemory;
    binding.msl_texture = 0;
    binding.msl_sampler = 0;
    compiler.add_msl_resource_binding(binding);
  }

  // Set 0 binding 1: EDRAM (only used in FSI mode — skip for host RT path).
  // We still add a dummy mapping so SPIRV-Cross doesn't complain if the
  // shader happens to reference it (it shouldn't for kHostRenderTargets).
  {
    MSLBinding binding;
    binding.stage = stage;
    binding.desc_set = SpirvSets::kSharedMemoryAndEdram;
    binding.binding = 1;
    binding.msl_buffer = 30;  // High index, unused.
    binding.msl_texture = 0;
    binding.msl_sampler = 0;
    compiler.add_msl_resource_binding(binding);
  }

  // Set 1: Constant buffers.
  struct CbvMapping {
    uint32_t spirv_binding;
    uint32_t msl_buffer;
  };
  static const CbvMapping cbv_mappings[] = {
      {SpirvCbv::kSystem, MslBindings::kSystemConstants},
      {SpirvCbv::kFloatVertex, MslBindings::kFloatConstantsVertex},
      {SpirvCbv::kFloatPixel, MslBindings::kFloatConstantsPixel},
      {SpirvCbv::kBoolLoop, MslBindings::kBoolLoopConstants},
      {SpirvCbv::kFetch, MslBindings::kFetchConstants},
      {SpirvCbv::kClipPlanes, MslBindings::kClipPlaneConstants},
      {SpirvCbv::kTessellation, MslBindings::kTessellationConstants},
  };
  for (const auto& cbv : cbv_mappings) {
    MSLBinding binding;
    binding.stage = stage;
    binding.desc_set = SpirvSets::kConstants;
    binding.binding = cbv.spirv_binding;
    binding.msl_buffer = cbv.msl_buffer;
    binding.msl_texture = 0;
    binding.msl_sampler = 0;
    compiler.add_msl_resource_binding(binding);
  }

  // Set 2 (vertex textures) and Set 3 (pixel textures): texture bindings.
  //
  // The SPIR-V translator places sampler bindings AFTER texture bindings in
  // the same descriptor set (sampler i gets SPIR-V binding texture_count + i).
  // Metal only supports 16 samplers per stage, so we must remap sampler
  // bindings to compact indices 0..M-1 regardless of their SPIR-V binding.
  //
  // Use a single MSLResourceBinding entry per SPIR-V binding to avoid
  // overwrite issues (SPIRV-Cross keys entries by {stage, set, binding}).
  //
  // Step 1: Map all 32 possible texture bindings with identity mapping.
  //         Set msl_sampler = 0 as a safe default (overridden for real
  //         samplers in step 2).
  for (uint32_t set = SpirvSets::kTexturesVertex;
       set <= SpirvSets::kTexturesPixel; ++set) {
    if (use_argument_buffers) {
      // Map the argument buffer pointer itself to a Metal buffer slot.
      MSLBinding argbuf_binding;
      argbuf_binding.stage = stage;
      argbuf_binding.desc_set = set;
      argbuf_binding.binding = spirv_cross::kArgumentBufferBinding;
      argbuf_binding.msl_buffer =
          MslBufferIndex::kArgumentBufferTexturesSamplers;
      argbuf_binding.msl_texture = 0;
      argbuf_binding.msl_sampler = 0;
      compiler.add_msl_resource_binding(argbuf_binding);
    }
    for (uint32_t i = 0; i < MslTextureIndex::kMaxPerStage; ++i) {
      MSLBinding binding;
      binding.stage = stage;
      binding.desc_set = set;
      binding.binding = i;
      binding.msl_buffer = 0;
      binding.msl_texture = MslBindings::kTextureBase + i;
      binding.msl_sampler = 0;
      compiler.add_msl_resource_binding(binding);
    }
  }

  // Step 2: Query the actual SPIR-V resources to find sampler bindings
  //         and remap them to compact Metal sampler indices 0..M-1.
  //         This handles the case where SPIR-V sampler bindings exceed 15
  //         (which would violate Metal's 16-sampler-per-stage limit).
  auto resources = compiler.get_shader_resources();
  struct SamplerSpvBinding {
    uint32_t set;
    uint32_t binding;
  };
  std::vector<SamplerSpvBinding> sampler_spv_bindings;
  sampler_spv_bindings.reserve(resources.separate_samplers.size());
  for (const auto& samp : resources.separate_samplers) {
    uint32_t set =
        compiler.get_decoration(samp.id, spv::DecorationDescriptorSet);
    if (set != texture_descriptor_set) {
      continue;
    }
    uint32_t spv_binding =
        compiler.get_decoration(samp.id, spv::DecorationBinding);
    sampler_spv_bindings.push_back({set, spv_binding});
  }
  bool sampler_order_was_unsorted = false;
  for (size_t i = 1; i < sampler_spv_bindings.size(); ++i) {
    const auto& prev = sampler_spv_bindings[i - 1];
    const auto& curr = sampler_spv_bindings[i];
    if (curr.set < prev.set ||
        (curr.set == prev.set && curr.binding < prev.binding)) {
      sampler_order_was_unsorted = true;
      break;
    }
  }
  std::sort(sampler_spv_bindings.begin(), sampler_spv_bindings.end(),
            [](const SamplerSpvBinding& a, const SamplerSpvBinding& b) {
              if (a.set != b.set) {
                return a.set < b.set;
              }
              return a.binding < b.binding;
            });
  if (sampler_order_was_unsorted) {
    XELOGD(
        "MslShader: Reordered sampler SPIR-V bindings by set/binding for "
        "stable Metal remap shader={:016X} stage={}",
        shader_hash, GetExecutionModelName(stage));
  }
  uint32_t sampler_msl_index = 0;
  std::ostringstream sampler_remap_log;
  uint32_t sampler_remap_count = 0;
  for (size_t i = 0; i < sampler_spv_bindings.size(); ++i) {
    uint32_t set = sampler_spv_bindings[i].set;
    uint32_t spv_binding = sampler_spv_bindings[i].binding;
    if (i > 0 && set == sampler_spv_bindings[i - 1].set &&
        spv_binding == sampler_spv_bindings[i - 1].binding) {
      XELOGW(
          "MslShader: Duplicate sampler SPIR-V binding at set={} binding={} "
          "shader={:016X}",
          set, spv_binding, shader_hash);
      continue;
    }
    if (sampler_msl_index >= 16) {
      XELOGW(
          "MslShader: Too many samplers ({} >= 16), sampler at set={} "
          "binding={} will not be bound",
          sampler_msl_index, set, spv_binding);
      break;
    }
    // Overwrite the entry for this binding with the compact sampler index.
    // For standalone samplers, don't propagate the SPIR-V binding to
    // msl_texture; sampler SPIR-V bindings are after image bindings and may
    // exceed the per-stage Metal texture slot limit.
    MSLBinding binding;
    binding.stage = stage;
    binding.desc_set = set;
    binding.binding = spv_binding;
    binding.msl_buffer = 0;
    binding.msl_texture = MslTextureIndex::kBase;
    binding.msl_sampler =
        (use_argument_buffers ? MslArgumentBufferId::kSamplerBase
                              : MslBindings::kSamplerBase) +
        sampler_msl_index;
    compiler.add_msl_resource_binding(binding);
    if (sampler_spv_bindings_msl_order) {
      sampler_spv_bindings_msl_order->push_back(spv_binding);
    }
    sampler_remap_log << "[set=" << set << ",binding=" << spv_binding
                      << "->msl_sampler=" << sampler_msl_index << "] ";
    sampler_msl_index++;
    sampler_remap_count++;
  }
  if (sampler_remap_count) {
    XELOGD("MslShader: Sampler remap table shader={:016X} stage={} count={} {}",
           shader_hash, GetExecutionModelName(stage), sampler_remap_count,
           sampler_remap_log.str());
  } else {
    XELOGD("MslShader: Sampler remap table shader={:016X} stage={} count=0",
           shader_hash, GetExecutionModelName(stage));
  }
}

bool MslShader::MslTranslation::CompileToMsl(MTL::Device* device, bool is_ios) {
  if (!device) {
    XELOGE("MslShader: No Metal device provided");
    return false;
  }
  if (argument_encoder_) {
    argument_encoder_->release();
    argument_encoder_ = nullptr;
  }
  uses_argument_buffers_ = false;
  argument_encoder_alignment_ = 0;
  argument_encoder_encoded_length_ = 0;

  const std::vector<uint8_t>& spirv_data = translated_binary();
  if (spirv_data.empty()) {
    XELOGE("MslShader: No translated SPIR-V data available");
    return false;
  }
  const size_t spirv_size_bytes = spirv_data.size();
  if (spirv_size_bytes >= 64 * 1024) {
    XELOGW(
        "MslShader: Large SPIR-V module detected ({:016X}, {} bytes, stage={})",
        shader().ucode_data_hash(), spirv_size_bytes,
        shader().type() == xenos::ShaderType::kVertex ? "vertex" : "fragment");
  }
  const auto compile_begin = std::chrono::steady_clock::now();
  auto to_ms =
      [](const std::chrono::steady_clock::duration& duration) -> double {
    return std::chrono::duration<double, std::milli>(duration).count();
  };
  double spirv_cross_ms = 0.0;
  double metal_library_ms = 0.0;
  double function_resolve_ms = 0.0;

  // SPIR-V data is a vector of bytes, but SPIRV-Cross expects uint32_t words.
  if (spirv_data.size() % sizeof(uint32_t) != 0) {
    XELOGE("MslShader: SPIR-V data size is not aligned to 4 bytes");
    return false;
  }
  const uint32_t* spirv_words =
      reinterpret_cast<const uint32_t*>(spirv_data.data());
  size_t spirv_word_count = spirv_data.size() / sizeof(uint32_t);

  // metal_shader_compiler "spirv2air": a vertex or fragment shader is
  // compiled with spirv2air first, and with SPIRV-Cross where that fails. A
  // second call, for a pipeline whose other shader has no spirv2air function,
  // compiles the Metal Shading Language version.
  if (cvars::metal_shader_compiler == "spirv2air" && !spirv2air_function_ &&
      !metal_function_) {
    const spv::ExecutionModel spirv2air_model =
        GetSpirvExecutionModel(spirv_words, spirv_word_count);
    if (spirv2air_model == spv::ExecutionModelVertex ||
        spirv2air_model == spv::ExecutionModelFragment) {
      const uint32_t spirv2air_variant = GetSpirv2airVariant();
      uint64_t spirv2air_cache_key = 0;
      if (cvars::metal_shader_disk_cache && g_metal_shader_cache &&
          g_metal_shader_cache->IsInitialized()) {
        spirv2air_cache_key = XXH3_64bits_withSeed(
            spirv_words, spirv_word_count * sizeof(uint32_t),
            GetSpirv2airCacheSeed(is_ios, spirv2air_variant));
      }
      if (CompileSpirv2air(
              device, is_ios, spirv2air_variant, spirv_words, spirv_word_count,
              shader().type() == xenos::ShaderType::kVertex,
              shader().ucode_data_hash(), modification(), 0.0,
              spirv2air_cache_key, &spirv2air_library_, &spirv2air_function_)) {
        spirv2air_options_ = spirv2air_variant;
        const double total_compile_ms =
            to_ms(std::chrono::steady_clock::now() - compile_begin);
        metal_perf_counters.shader_compiles.fetch_add(
            1, std::memory_order_relaxed);
        metal_perf_counters.shader_compile_us.fetch_add(
            uint64_t(total_compile_ms * 1000.0), std::memory_order_relaxed);
        return true;
      }
    }
  }

  // SPIRV-Cross throws exceptions on translation errors.  We catch them
  // below and return false so the draw can be skipped gracefully instead
  // of crashing the process.
  spirv_cross::CompilerMSL compiler(spirv_words, spirv_word_count);

  // Configure MSL options.
  auto opts = compiler.get_msl_options();
  if (is_ios) {
    opts.platform = spirv_cross::CompilerMSL::Options::iOS;
  } else {
    opts.platform = spirv_cross::CompilerMSL::Options::macOS;
  }
  bool ios_supports_base_vertex_instance = false;
  bool ios_supports_cube_array = true;
  bool ios_supports_simdgroup_scope_ops = false;
  bool ios_supports_metal3 = false;
  bool ios_supports_apple9 = false;
  bool ios_supports_apple10 = false;
  if (is_ios) {
    // Apple docs: base vertex/instance drawing is Apple3+, cube array textures
    // are Apple4+, SIMD-scoped permute is Apple6+.
    ios_supports_base_vertex_instance =
        device->supportsFamily(MTL::GPUFamilyApple3);
    ios_supports_cube_array = device->supportsFamily(MTL::GPUFamilyApple4);
    ios_supports_simdgroup_scope_ops =
        device->supportsFamily(MTL::GPUFamilyApple6);
    // Apple Metal Feature Set Tables (2025-10-20): Metal 3+ starts at Apple7,
    // Apple9/Apple10 map to newer iPhone GPU families (A17/A18 and A19).
    ios_supports_metal3 = device->supportsFamily(MTL::GPUFamilyApple7);
    ios_supports_apple9 = device->supportsFamily(MTL::GPUFamilyApple9);
    ios_supports_apple10 = device->supportsFamily(MTL::GPUFamilyApple10);
  }
  // iOS: prefer Metal 3 language levels on Apple7+ devices.
  // Keep legacy fallback for older iOS GPUs.
  uint32_t msl_major = 2;
  uint32_t msl_minor = is_ios ? 3u : 4u;
  MTL::LanguageVersion language_version =
      is_ios ? MTL::LanguageVersion2_3 : MTL::LanguageVersion2_4;
  if (is_ios && ios_supports_metal3) {
    msl_major = 3;
    if (ios_supports_apple10) {
      msl_minor = 2;
      language_version = MTL::LanguageVersion3_2;
    } else if (ios_supports_apple9) {
      msl_minor = 1;
      language_version = MTL::LanguageVersion3_1;
    } else {
      msl_minor = 0;
      language_version = MTL::LanguageVersion3_0;
    }
  }
  opts.msl_version =
      spirv_cross::CompilerMSL::Options::make_msl_version(msl_major, msl_minor);
  // Use argument buffers for SPIRV-Cross texture/sampler bindings to reduce
  // per-draw CPU binding overhead.
  uses_argument_buffers_ = true;
  opts.argument_buffers = uses_argument_buffers_;
  uint8_t argument_buffers_tier_key = 0;
  if (uses_argument_buffers_) {
    if (device->argumentBuffersSupport() >= MTL::ArgumentBuffersTier2) {
      opts.argument_buffers_tier =
          spirv_cross::CompilerMSL::Options::ArgumentBuffersTier::Tier2;
      argument_buffers_tier_key = 1;
    } else {
      opts.argument_buffers_tier =
          spirv_cross::CompilerMSL::Options::ArgumentBuffersTier::Tier1;
      argument_buffers_tier_key = 0;
    }
    // Keep argument-buffer layouts stable even if SPIRV-Cross marks specific
    // resources inactive in some variants.
    opts.force_active_argument_buffer_resources = true;
  }
  // Match iOS feature toggles to documented GPU-family availability.
  opts.ios_support_base_vertex_instance = ios_supports_base_vertex_instance;
  const bool emulate_cube_array = is_ios && !ios_supports_cube_array;
  opts.emulate_cube_array = emulate_cube_array;
  // Keep iOS aligned with the conservative cross-platform path by default.
  // Enabling simdgroup translation on some iOS shader/compiler combinations can
  // yield libraries with no exported stage entry points (`functionNames=[]`).
  const bool ios_use_simdgroup_functions =
      is_ios && ios_supports_simdgroup_scope_ops;
  opts.ios_use_simdgroup_functions = ios_use_simdgroup_functions;
  // Force sample rate shading to be available if needed.
  opts.force_sample_rate_shading = false;
  // Keep native component counts for fragment outputs to preserve shader-
  // defined alpha semantics and avoid over-constraining RT writes.
  opts.pad_fragment_output_components = false;
  const uint64_t msl_source_cache_key = GetMslSourceCacheKey(
      *this, is_ios, msl_major, msl_minor, ios_supports_base_vertex_instance,
      emulate_cube_array, ios_use_simdgroup_functions, uses_argument_buffers_,
      argument_buffers_tier_key);
  compiler.set_msl_options(opts);
  if (uses_argument_buffers_) {
    // Keep shared memory/storage buffers and constant buffers as discrete
    // bindings; put textures + samplers in argument buffers.
    compiler.add_discrete_descriptor_set(SpirvSets::kSharedMemoryAndEdram);
    compiler.add_discrete_descriptor_set(SpirvSets::kConstants);
  }

  // Remap SPIR-V descriptor sets/bindings to Metal buffer/texture/sampler
  // indices.
  // Query the actual execution model from the SPIR-V binary rather than
  // assuming kVertex always maps to spv::ExecutionModelVertex.  The
  // SpirvShaderTranslator already sets the correct model — domain shaders
  // (tessellation evaluation) are emitted as TessellationEvaluation, not
  // Vertex — so we must honour that for SPIRV-Cross resource remapping and
  // entry-point lookup.
  std::string spirv_entry_point_name = "main";
  std::vector<std::string> spirv_entry_point_descriptions;
  spv::ExecutionModel execution_model;
  {
    auto entry_points = compiler.get_entry_points_and_stages();
    if (!entry_points.empty()) {
      spirv_entry_point_descriptions.reserve(entry_points.size());
      for (const auto& entry_point : entry_points) {
        spirv_entry_point_descriptions.push_back(
            fmt::format("{}:{}", entry_point.name,
                        GetExecutionModelName(entry_point.execution_model)));
      }
      spv::ExecutionModel preferred_model =
          shader().type() == xenos::ShaderType::kVertex
              ? spv::ExecutionModelVertex
              : spv::ExecutionModelFragment;
      const spirv_cross::EntryPoint* selected_entry = &entry_points.front();
      for (const auto& entry_point : entry_points) {
        if (entry_point.execution_model == preferred_model) {
          selected_entry = &entry_point;
          break;
        }
      }
      execution_model = selected_entry->execution_model;
      if (!selected_entry->name.empty()) {
        spirv_entry_point_name = selected_entry->name;
      }
    } else {
      // Fallback: infer from shader type (pre-tessellation behavior).
      execution_model = shader().type() == xenos::ShaderType::kVertex
                            ? spv::ExecutionModelVertex
                            : spv::ExecutionModelFragment;
      XELOGW(
          "MslShader: SPIR-V module has no declared entry points "
          "(shader={:016X}, expected_stage={})",
          shader().ucode_data_hash(), GetExecutionModelName(execution_model));
    }
  }
  try {
    compiler.set_entry_point(spirv_entry_point_name, execution_model);
  } catch (const std::exception& e) {
    XELOGW(
        "MslShader: Failed to set SPIR-V entry point '{}' (stage={}) "
        "shader={:016X}: {}",
        spirv_entry_point_name, GetExecutionModelName(execution_model),
        shader().ucode_data_hash(), e.what());
  }
  sampler_binding_indices_for_msl_slots_.clear();
  texture_binding_indices_for_msl_slots_.clear();
  // Domain shaders use tessellation-evaluation execution model, but still use
  // the vertex texture descriptor set in SpirvShaderTranslator.
  uint32_t texture_descriptor_set =
      shader().type() == xenos::ShaderType::kVertex ? SpirvSets::kTexturesVertex
                                                    : SpirvSets::kTexturesPixel;
  std::vector<uint32_t> sampler_spv_bindings_msl_order;
  AddResourceBindings(compiler, execution_model, texture_descriptor_set,
                      shader().ucode_data_hash(), uses_argument_buffers_,
                      &sampler_spv_bindings_msl_order);
  auto resources = compiler.get_shader_resources();

  // Build the runtime texture binding lookup for Metal slots.
  // Slots are direct (msl_texture = SPIR-V binding). Seed with translator
  // order, then refine with reflection where possible.
  const MslShader& msl_shader = static_cast<const MslShader&>(shader());
  const auto& shader_texture_bindings =
      msl_shader.GetTextureBindingsAfterTranslation();
  auto find_runtime_texture_binding_index =
      [&](uint32_t fetch_constant, xenos::FetchOpDimension dimension,
          bool is_signed) -> int32_t {
    for (size_t i = 0; i < shader_texture_bindings.size(); ++i) {
      const auto& binding = shader_texture_bindings[i];
      if (binding.fetch_constant == fetch_constant &&
          binding.dimension == dimension && binding.is_signed == is_signed) {
        return int32_t(i);
      }
    }
    return -1;
  };
  auto parse_texture_resource_name =
      [](const std::string& name, uint32_t* fetch_constant_out,
         xenos::FetchOpDimension* dimension_out, bool* is_signed_out) -> bool {
    static constexpr const char kPrefix[] = "xe_texture";
    if (!fetch_constant_out || !dimension_out || !is_signed_out ||
        name.size() <= sizeof(kPrefix) - 1 ||
        name.compare(0, sizeof(kPrefix) - 1, kPrefix) != 0) {
      return false;
    }
    size_t offset = sizeof(kPrefix) - 1;
    uint32_t fetch_constant = 0;
    bool has_digit = false;
    while (offset < name.size() &&
           std::isdigit(static_cast<unsigned char>(name[offset]))) {
      has_digit = true;
      fetch_constant = fetch_constant * 10 + uint32_t(name[offset] - '0');
      ++offset;
    }
    if (!has_digit || offset >= name.size() || name[offset] != '_') {
      return false;
    }
    ++offset;

    size_t dim_end = name.find('_', offset);
    if (dim_end == std::string::npos || dim_end <= offset) {
      return false;
    }
    std::string dim_token = name.substr(offset, dim_end - offset);
    xenos::FetchOpDimension dimension = xenos::FetchOpDimension::k2D;
    if (dim_token == "2d") {
      dimension = xenos::FetchOpDimension::k2D;
    } else if (dim_token == "3d") {
      dimension = xenos::FetchOpDimension::k3DOrStacked;
    } else if (dim_token == "cube") {
      dimension = xenos::FetchOpDimension::kCube;
    } else {
      return false;
    }

    offset = dim_end + 1;
    if (offset >= name.size()) {
      return false;
    }
    char sign_char = name[offset];
    if (sign_char != 's' && sign_char != 'u') {
      return false;
    }

    *fetch_constant_out = fetch_constant;
    *dimension_out = dimension;
    *is_signed_out = sign_char == 's';
    return true;
  };
  texture_binding_indices_for_msl_slots_.reserve(std::min<size_t>(
      shader_texture_bindings.size(), MslTextureIndex::kMaxPerStage));
  for (uint32_t i = 0;
       i < std::min<uint32_t>(uint32_t(shader_texture_bindings.size()),
                              MslTextureIndex::kMaxPerStage);
       ++i) {
    texture_binding_indices_for_msl_slots_.push_back(int32_t(i));
  }
  bool has_reflected_texture_bindings = false;
  bool texture_reflection_incomplete = false;
  auto register_texture_binding =
      [&](const spirv_cross::Resource& resource) -> void {
    uint32_t set =
        compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    if (set != texture_descriptor_set) {
      return;
    }
    has_reflected_texture_bindings = true;
    uint32_t spv_binding =
        compiler.get_decoration(resource.id, spv::DecorationBinding);
    if (spv_binding >= MslTextureIndex::kMaxPerStage) {
      XELOGW(
          "MslShader: Reflected texture binding {} exceeds Metal slot limit {} "
          "shader={:016X}",
          spv_binding, MslTextureIndex::kMaxPerStage,
          shader().ucode_data_hash());
      return;
    }
    int32_t runtime_binding_index = -1;
    uint32_t parsed_fetch_constant = 0;
    xenos::FetchOpDimension parsed_dimension = xenos::FetchOpDimension::k2D;
    bool parsed_is_signed = false;
    if (parse_texture_resource_name(resource.name, &parsed_fetch_constant,
                                    &parsed_dimension, &parsed_is_signed)) {
      runtime_binding_index = find_runtime_texture_binding_index(
          parsed_fetch_constant, parsed_dimension, parsed_is_signed);
      if (runtime_binding_index < 0) {
        XELOGW(
            "MslShader: Unable to resolve texture resource '{}' in runtime "
            "bindings shader={:016X}",
            resource.name, shader().ucode_data_hash());
      }
    }
    if (runtime_binding_index < 0) {
      // Fall back to SPIR-V decoration binding index when reflection names are
      // unavailable or don't match translator naming.
      if (spv_binding >= shader_texture_bindings.size()) {
        XELOGW(
            "MslShader: Reflected texture binding {} is out of runtime range "
            "{} "
            "shader={:016X}",
            spv_binding, shader_texture_bindings.size(),
            shader().ucode_data_hash());
        texture_reflection_incomplete = true;
        return;
      }
      runtime_binding_index = int32_t(spv_binding);
    }

    if (texture_binding_indices_for_msl_slots_.size() <= spv_binding) {
      texture_binding_indices_for_msl_slots_.resize(spv_binding + 1, -1);
    }
    int32_t& slot_binding = texture_binding_indices_for_msl_slots_[spv_binding];
    if (slot_binding == -1) {
      slot_binding = runtime_binding_index;
    } else if (slot_binding != runtime_binding_index) {
      XELOGW(
          "MslShader: Conflicting texture remap at slot {} ({} vs {}) "
          "shader={:016X}",
          spv_binding, slot_binding, runtime_binding_index,
          shader().ucode_data_hash());
      texture_reflection_incomplete = true;
    }
  };
  for (const auto& image : resources.separate_images) {
    register_texture_binding(image);
  }
  for (const auto& image : resources.sampled_images) {
    register_texture_binding(image);
  }
  if (!has_reflected_texture_bindings && !shader_texture_bindings.empty()) {
    XELOGW(
        "MslShader: No reflected texture bindings for shader={:016X} stage={} "
        "(runtime={}), using translator-order texture remap",
        shader().ucode_data_hash(), GetExecutionModelName(execution_model),
        shader_texture_bindings.size());
  } else if (texture_reflection_incomplete) {
    XELOGW(
        "MslShader: Incomplete texture reflection remap for shader={:016X} "
        "stage={}, keeping translator-order mappings for unresolved slots",
        shader().ucode_data_hash(), GetExecutionModelName(execution_model));
  }

  // Build the runtime sampler binding lookup for Metal slots.
  // This is needed because SPIRV-Cross may drop / reorder separate samplers,
  // while runtime sampler parameters are stored in translator binding order.
  uint32_t shader_texture_binding_count = static_cast<uint32_t>(
      msl_shader.GetTextureBindingsAfterTranslation().size());
  const auto& shader_sampler_bindings =
      msl_shader.GetSamplerBindingsAfterTranslation();
  std::vector<bool> sampler_binding_mapped(shader_sampler_bindings.size(),
                                           false);
  sampler_binding_indices_for_msl_slots_.reserve(
      sampler_spv_bindings_msl_order.size());
  for (uint32_t spv_binding : sampler_spv_bindings_msl_order) {
    if (spv_binding < shader_texture_binding_count) {
      XELOGW(
          "MslShader: Reflected sampler binding {} is below texture binding "
          "base {} shader={:016X}",
          spv_binding, shader_texture_binding_count,
          shader().ucode_data_hash());
      continue;
    }
    uint32_t sampler_binding_index = spv_binding - shader_texture_binding_count;
    if (sampler_binding_index >= shader_sampler_bindings.size()) {
      XELOGW(
          "MslShader: Reflected sampler binding {} maps to out-of-range "
          "sampler index {} (available {}) shader={:016X}",
          spv_binding, sampler_binding_index, shader_sampler_bindings.size(),
          shader().ucode_data_hash());
      continue;
    }
    if (sampler_binding_index < sampler_binding_mapped.size() &&
        sampler_binding_mapped[sampler_binding_index]) {
      continue;
    }
    sampler_binding_indices_for_msl_slots_.push_back(sampler_binding_index);
    if (sampler_binding_index < sampler_binding_mapped.size()) {
      sampler_binding_mapped[sampler_binding_index] = true;
    }
  }
  size_t sampler_mapped_count = sampler_binding_indices_for_msl_slots_.size();
  if (!shader_sampler_bindings.empty() &&
      sampler_mapped_count < shader_sampler_bindings.size()) {
    for (uint32_t i = 0; i < shader_sampler_bindings.size(); ++i) {
      if (!sampler_binding_mapped[i]) {
        sampler_binding_indices_for_msl_slots_.push_back(i);
      }
    }
  }
  bool sampler_reflection_incomplete =
      sampler_spv_bindings_msl_order.size() > sampler_mapped_count;
  if (!shader_sampler_bindings.empty() && sampler_reflection_incomplete) {
    XELOGW(
        "MslShader: Sampler reflection remap incomplete; preserving mapped "
        "samplers and appending unmapped samplers in translator order "
        "shader={:016X} stage={} (reflected={} mapped={} runtime={})",
        shader().ucode_data_hash(), GetExecutionModelName(execution_model),
        sampler_spv_bindings_msl_order.size(), sampler_mapped_count,
        shader_sampler_bindings.size());
  } else if (sampler_spv_bindings_msl_order.empty() &&
             !shader_sampler_bindings.empty()) {
    XELOGW(
        "MslShader: No reflected sampler bindings for shader={:016X} stage={}, "
        "using translator-order sampler remap",
        shader().ucode_data_hash(), GetExecutionModelName(execution_model));
  }
  if (sampler_binding_indices_for_msl_slots_.size() >
      MslSamplerIndex::kMaxPerStage) {
    sampler_binding_indices_for_msl_slots_.resize(
        MslSamplerIndex::kMaxPerStage);
  }

  // Validate resource counts against Metal limits before compilation.
  bool has_argument_buffer_resources = false;
  {
    bool expects_tessellation_constants = false;
    for (const auto& uniform_buffer : resources.uniform_buffers) {
      uint32_t set = compiler.get_decoration(uniform_buffer.id,
                                             spv::DecorationDescriptorSet);
      uint32_t binding =
          compiler.get_decoration(uniform_buffer.id, spv::DecorationBinding);
      if (set == SpirvSets::kConstants && binding == SpirvCbv::kTessellation) {
        expects_tessellation_constants = true;
        break;
      }
    }
    size_t texture_count =
        resources.sampled_images.size() + resources.separate_images.size();
    size_t sampler_count = resources.separate_samplers.size();
    has_argument_buffer_resources = (texture_count + sampler_count) != 0;
    XELOGD(
        "MslShader: Resource summary shader={:016X} stage={} textures={} "
        "samplers={} uniform_buffers={} tessellation_cbv_expected={}",
        shader().ucode_data_hash(), GetExecutionModelName(execution_model),
        texture_count, sampler_count, resources.uniform_buffers.size(),
        expects_tessellation_constants ? 1 : 0);
    if (expects_tessellation_constants) {
      XELOGW(
          "MslShader: Shader {:016X} stage={} declares set={},binding={} "
          "tessellation constants (msl_buffer={})",
          shader().ucode_data_hash(), GetExecutionModelName(execution_model),
          SpirvSets::kConstants, SpirvCbv::kTessellation,
          MslBindings::kTessellationConstants);
    }
    if (texture_count > MslTextureIndex::kMaxPerStage) {
      XELOGE(
          "MslShader: Shader uses {} textures, exceeding Metal's {}-per-stage "
          "limit in this backend",
          texture_count, MslTextureIndex::kMaxPerStage);
      return false;
    }
    if (sampler_count > 16) {
      XELOGE(
          "MslShader: Shader uses {} samplers, exceeding Metal's 16-per-stage "
          "limit — translation should be considered failed",
          sampler_count);
      return false;
    }
  }

  // Compile to MSL.  SPIRV-Cross throws on translation errors.
  const auto spirv_cross_begin = std::chrono::steady_clock::now();
  bool loaded_source_from_cache = false;
  if (cvars::metal_shader_disk_cache) {
    loaded_source_from_cache =
        LoadCachedMslSource(msl_source_cache_key, &msl_source_);
    if (loaded_source_from_cache) {
      XELOGD("MslShader: Loaded cached MSL source ({:016X})",
             shader().ucode_data_hash());
    }
  }
  if (!loaded_source_from_cache) {
    try {
      msl_source_ = compiler.compile();
    } catch (const std::exception& e) {
      XELOGE("MslShader: SPIRV-Cross compilation failed: {}", e.what());
      return false;
    }
    if (cvars::metal_shader_disk_cache) {
      StoreCachedMslSource(msl_source_cache_key, msl_source_);
    }
  }
  spirv_cross_ms = to_ms(std::chrono::steady_clock::now() - spirv_cross_begin);
  if (msl_source_.empty()) {
    XELOGE("MslShader: SPIRV-Cross compilation produced empty output");
    return false;
  }
  if (cvars::metal_spirvcross_strip_nocontract_optnone) {
    size_t stripped = StripNoContractionOptnoneWrappers(msl_source_);
    if (stripped != 0) {
      XELOGD(
          "MslShader: Stripped {} NoContraction optnone wrapper signature(s) "
          "for shader {:016X}",
          stripped, shader().ucode_data_hash());
    }
  }

  if (ShouldLogMslShader(shader().ucode_data_hash())) {
    const char* stage_name =
        shader().type() == xenos::ShaderType::kVertex ? "VS" : "PS";
    auto log_shader_lines = [&](const char* what, const std::string& text) {
      size_t line_begin = 0;
      while (line_begin < text.size()) {
        size_t line_end = text.find('\n', line_begin);
        if (line_end == std::string::npos) {
          line_end = text.size();
        }
        XELOGI(
            "Shader log {} {:016X} mod {:016X} {}: {}", stage_name,
            shader().ucode_data_hash(), modification(), what,
            std::string_view(text).substr(line_begin, line_end - line_begin));
        line_begin = line_end + 1;
      }
    };
    log_shader_lines("ucode", shader().ucode_disassembly());
    log_shader_lines("msl", msl_source_);
  }

  // Get the entry point name that SPIRV-Cross chose.
  std::vector<std::string> entry_point_candidates;
  auto add_entry_point_candidate = [&](const std::string& name) {
    if (name.empty()) {
      return;
    }
    if (std::find(entry_point_candidates.begin(), entry_point_candidates.end(),
                  name) == entry_point_candidates.end()) {
      entry_point_candidates.push_back(name);
    }
  };
  add_entry_point_candidate(spirv_entry_point_name);
  try {
    add_entry_point_candidate(compiler.get_cleansed_entry_point_name(
        spirv_entry_point_name, execution_model));
  } catch (const std::exception& e) {
    XELOGW(
        "MslShader: SPIRV-Cross entry point lookup failed for '{}' "
        "(stage={}): {}",
        spirv_entry_point_name, GetExecutionModelName(execution_model),
        e.what());
  }
  if (spirv_entry_point_name != "main") {
    try {
      add_entry_point_candidate(
          compiler.get_cleansed_entry_point_name("main", execution_model));
    } catch (const std::exception&) {
    }
  }
  add_entry_point_candidate("main0");
  add_entry_point_candidate("main");
  add_entry_point_candidate("vertexMain");
  add_entry_point_candidate("fragmentMain");
  if (entry_point_candidates.empty()) {
    entry_point_candidates.push_back("main0");
  }
  // Entry points named in the MSL source are only searched for if none of
  // these resolves (below): the regular expression search took hundreds of
  // milliseconds per shader on iOS (libc++ std::regex over ~60 KB of MSL),
  // most of every compile, for candidates that come after these anyway.
  entry_point_name_ = entry_point_candidates.front();
  XELOGD("MslShader: Compiled SPIR-V to MSL ({} bytes, entries: {})",
         msl_source_.size(), JoinStringList(entry_point_candidates));

  // Dump MSL source when --dump_shaders is set (matching Vulkan dump pattern).
  if (!cvars::dump_shaders.empty()) {
    auto base_path = std::filesystem::absolute(cvars::dump_shaders);
    auto msl_dir = base_path / "msl_shaders";
    std::filesystem::create_directories(msl_dir);
    const char* type_str =
        (shader().type() == xenos::ShaderType::kVertex) ? "vs" : "ps";
    auto msl_path =
        msl_dir / fmt::format("{}_{:016X}_mod{:016X}.metal", type_str,
                              shader().ucode_data_hash(), modification());
    FILE* f = xe::filesystem::OpenFile(msl_path, "w");
    if (f) {
      fwrite(msl_source_.data(), 1, msl_source_.size(), f);
      fclose(f);
      XELOGD("MslShader: Dumped MSL to {}", msl_path.string());
    }
  }

  // Log SPIRV-Cross resolved SystemConstants member offsets for diagnostic
  // comparison with the C++ struct layout.
  try {
    auto resources = compiler.get_shader_resources();
    for (const auto& ubo : resources.uniform_buffers) {
      uint32_t set =
          compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
      uint32_t binding =
          compiler.get_decoration(ubo.id, spv::DecorationBinding);
      if (set == SpirvSets::kConstants && binding == SpirvCbv::kSystem) {
        auto& base_type = compiler.get_type(ubo.base_type_id);
        uint32_t member_count =
            static_cast<uint32_t>(base_type.member_types.size());
        XELOGD(
            "MslShader: SystemConstants reflection shader={:016X} "
            "members={}",
            shader().ucode_data_hash(), member_count);
        for (uint32_t m = 0; m < member_count; ++m) {
          uint32_t offset = compiler.get_member_decoration(
              base_type.self, m, spv::DecorationOffset);
          const std::string& name = compiler.get_member_name(base_type.self, m);
          XELOGD("  [{}] '{}' offset={}", m, name, offset);
        }
        break;
      }
    }
  } catch (const std::exception& e) {
    XELOGW("MslShader: SystemConstants reflection failed: {}", e.what());
  }

  // Compile MSL source to a Metal library.
  // Use a local autorelease pool to ensure NS::String factory objects and any
  // NS::Error autoreleased by the Metal driver are cleaned up promptly.
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  NS::Error* error = nullptr;
  auto* source_str =
      NS::String::string(msl_source_.c_str(), NS::UTF8StringEncoding);
  auto* compile_options = MTL::CompileOptions::alloc()->init();
  // Fast math is faster but assumes no NaN or infinity
  // (metal_shader_fast_math); vertex shaders only with
  // metal_vertex_shader_fast_math, for exact positions.
  compile_options->setFastMathEnabled(
      cvars::metal_shader_fast_math &&
      (shader().type() != xenos::ShaderType::kVertex ||
       cvars::metal_vertex_shader_fast_math));
  // SPIRV-Cross marks the position [[invariant]]; Metal only honours that
  // with invariance preserved (iOS 14+).
  compile_options->setPreserveInvariance(
      cvars::metal_shader_preserve_invariance);
  // Match the Metal compiler language version to SPIRV-Cross output.
  compile_options->setLanguageVersion(language_version);

  const auto metal_library_begin = std::chrono::steady_clock::now();
  metal_library_ = device->newLibrary(source_str, compile_options, &error);
  metal_library_ms =
      to_ms(std::chrono::steady_clock::now() - metal_library_begin);
  compile_options->release();

  if (error && error->localizedDescription() &&
      error->localizedDescription()->utf8String()) {
    // Metal may return warnings alongside a non-null library.
    XELOGW("MslShader: Metal compile diagnostics: {}",
           error->localizedDescription()->utf8String());
  }

  if (!metal_library_) {
    if (error) {
      XELOGE("MslShader: Metal library compilation failed: {}",
             error->localizedDescription()->utf8String());
    } else {
      XELOGE("MslShader: Metal library compilation failed (unknown error)");
    }
    pool->release();
    return false;
  }

  // Get the entry point function.
  std::vector<std::string> attempted_function_names;
  std::vector<std::string> available_function_names;
  const auto function_resolve_begin = std::chrono::steady_clock::now();
  auto try_bind_function = [&](const std::string& function_name) -> bool {
    if (function_name.empty()) {
      return false;
    }
    if (std::find(attempted_function_names.begin(),
                  attempted_function_names.end(),
                  function_name) != attempted_function_names.end()) {
      return false;
    }
    attempted_function_names.push_back(function_name);
    auto* fn_name =
        NS::String::string(function_name.c_str(), NS::UTF8StringEncoding);
    MTL::Function* function = metal_library_->newFunction(fn_name);
    if (!function) {
      return false;
    }
    metal_function_ = function;
    entry_point_name_ = function_name;
    return true;
  };
  for (const auto& candidate : entry_point_candidates) {
    if (try_bind_function(candidate)) {
      break;
    }
  }
  if (!metal_function_) {
    const size_t named_candidate_count = entry_point_candidates.size();
    AddEntryPointCandidatesFromMslSource(msl_source_, shader().type(),
                                         &entry_point_candidates);
    for (size_t i = named_candidate_count; i < entry_point_candidates.size();
         ++i) {
      if (try_bind_function(entry_point_candidates[i])) {
        break;
      }
    }
  }
  if (!metal_function_) {
    NS::Array* function_names = metal_library_->functionNames();
    if (function_names) {
      for (NS::UInteger i = 0; i < function_names->count(); ++i) {
        auto* name = static_cast<NS::String*>(function_names->object(i));
        if (!name || !name->utf8String()) {
          continue;
        }
        available_function_names.emplace_back(name->utf8String());
      }
    }
    if (available_function_names.size() == 1) {
      try_bind_function(available_function_names.front());
    }
    if (!metal_function_) {
      for (const auto& available_name : available_function_names) {
        if (available_name.find("main") != std::string::npos &&
            try_bind_function(available_name)) {
          break;
        }
      }
    }
    if (!metal_function_) {
      const char* stage_hint =
          shader().type() == xenos::ShaderType::kVertex ? "vertex" : "fragment";
      for (const auto& available_name : available_function_names) {
        if (available_name.find(stage_hint) != std::string::npos &&
            try_bind_function(available_name)) {
          break;
        }
      }
    }
  }
  if (!metal_function_) {
    DumpMslResolveFailureSource(msl_source_, shader().ucode_data_hash(),
                                modification(), shader().type());
    XELOGE(
        "MslShader: Could not resolve function in compiled library "
        "(shader={:016X}, stage={}, spirv_entry='{}', candidates=[{}], "
        "attempted=[{}], available=[{}], spirv_entries=[{}])",
        shader().ucode_data_hash(), GetExecutionModelName(execution_model),
        spirv_entry_point_name, JoinStringList(entry_point_candidates),
        JoinStringList(attempted_function_names),
        JoinStringList(available_function_names),
        JoinStringList(spirv_entry_point_descriptions));
    metal_library_->release();
    metal_library_ = nullptr;
    pool->release();
    return false;
  }
  function_resolve_ms =
      to_ms(std::chrono::steady_clock::now() - function_resolve_begin);

  const auto argument_encoder_begin = std::chrono::steady_clock::now();
  if (uses_argument_buffers_ && has_argument_buffer_resources) {
    // newArgumentEncoder reflects the function, which compiles it; the
    // encoder depends only on the argument buffer struct, so with
    // metal_share_argument_encoders one is made per layout.
    const uint64_t argument_buffer_layout_key =
        cvars::metal_share_argument_encoders
            ? GetArgumentBufferLayoutKey(msl_source_, device)
            : 0;
    if (argument_buffer_layout_key) {
      std::lock_guard<std::mutex> lock(g_argument_encoder_cache_mutex);
      auto it = g_argument_encoder_cache.find(argument_buffer_layout_key);
      if (it != g_argument_encoder_cache.end()) {
        argument_encoder_ = it->second;
        argument_encoder_->retain();
      }
    }
    if (!argument_encoder_) {
      argument_encoder_ = metal_function_->newArgumentEncoder(
          MslBufferIndex::kArgumentBufferTexturesSamplers);
      if (argument_encoder_ && argument_buffer_layout_key) {
        std::lock_guard<std::mutex> lock(g_argument_encoder_cache_mutex);
        if (g_argument_encoder_cache
                .emplace(argument_buffer_layout_key, argument_encoder_)
                .second) {
          argument_encoder_->retain();
          XELOGI(
              "MslShader: argument encoder for layout {:016X} from shader "
              "{:016X} ({} bytes; {} layouts)",
              argument_buffer_layout_key, shader().ucode_data_hash(),
              argument_encoder_->encodedLength(),
              g_argument_encoder_cache.size());
        }
      }
    }
    if (argument_encoder_) {
      argument_encoder_alignment_ =
          static_cast<uint32_t>(argument_encoder_->alignment());
      argument_encoder_encoded_length_ =
          static_cast<uint32_t>(argument_encoder_->encodedLength());
    } else {
      XELOGE("MslShader: Failed to create argument encoder for shader {:016X}",
             shader().ucode_data_hash());
      metal_function_->release();
      metal_function_ = nullptr;
      metal_library_->release();
      metal_library_ = nullptr;
      pool->release();
      return false;
    }
  } else {
    argument_encoder_alignment_ = 0;
    argument_encoder_encoded_length_ = 0;
  }

  const double argument_encoder_ms =
      to_ms(std::chrono::steady_clock::now() - argument_encoder_begin);

  pool->release();

  const double total_compile_ms =
      to_ms(std::chrono::steady_clock::now() - compile_begin);
  metal_perf_counters.shader_compiles.fetch_add(1, std::memory_order_relaxed);
  metal_perf_counters.shader_compile_us.fetch_add(
      uint64_t(total_compile_ms * 1000.0), std::memory_order_relaxed);
  if (total_compile_ms >= 100.0 || spirv_size_bytes >= 64 * 1024) {
    XELOGW(
        "MslShader: Compile timings shader={:016X} stage={} spirv={}B "
        "msl={}B total={:.2f}ms (spirv-cross={:.2f}ms, metal-library={:.2f}ms, "
        "entry-resolve={:.2f}ms, argument-encoder={:.2f}ms)",
        shader().ucode_data_hash(),
        shader().type() == xenos::ShaderType::kVertex ? "vertex" : "fragment",
        spirv_size_bytes, msl_source_.size(), total_compile_ms, spirv_cross_ms,
        metal_library_ms, function_resolve_ms, argument_encoder_ms);
  } else {
    XELOGD(
        "MslShader: Compile timings shader={:016X} stage={} total={:.2f}ms "
        "(spirv-cross={:.2f}ms, metal-library={:.2f}ms, "
        "entry-resolve={:.2f}ms, argument-encoder={:.2f}ms)",
        shader().ucode_data_hash(),
        shader().type() == xenos::ShaderType::kVertex ? "vertex" : "fragment",
        total_compile_ms, spirv_cross_ms, metal_library_ms, function_resolve_ms,
        argument_encoder_ms);
  }

  XELOGD("MslShader: Successfully created Metal function '{}'",
         entry_point_name_);
  if (cvars::metal_spirv2air_probe &&
      cvars::metal_shader_compiler != "spirv2air") {
    ProbeSpirv2air(device, is_ios, spirv_words, spirv_word_count,
                   shader().type() == xenos::ShaderType::kVertex,
                   shader().ucode_data_hash(), modification(),
                   metal_library_ms);
  }
  return true;
}

}  // namespace metal
}  // namespace gpu
}  // namespace xe
