/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2026 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/gpu/metal/metal_gpu_pass_timing.h"

#include <algorithm>
#include <memory>
#include <thread>

#include "xenia/base/cvar.h"
#include "xenia/base/logging.h"

DEFINE_bool(metal_gpu_pass_timing, true,
            "Diagnostics: measure the GPU time of every render, compute and "
            "blit pass with Metal's stage boundary timestamps, estimate the "
            "attachment memory traffic, share the time out per pipeline, and "
            "track draws skipped for compiles; logged with the Metal perf "
            "lines.",
            "Metal");

namespace xe {
namespace gpu {
namespace metal {

namespace {

// Bytes per sample of a render target format, for the traffic estimate
// (stencil: of the stencil plane alone). 0 for formats not listed.
uint32_t MetalGpuBytesPerPixel(MTL::PixelFormat format, bool stencil) {
  if (stencil) {
    return 1;
  }
  switch (format) {
    case MTL::PixelFormatR8Unorm:
    case MTL::PixelFormatR8Snorm:
    case MTL::PixelFormatR8Uint:
    case MTL::PixelFormatR8Sint:
    case MTL::PixelFormatStencil8:
      return 1;
    case MTL::PixelFormatRG8Unorm:
    case MTL::PixelFormatRG8Snorm:
    case MTL::PixelFormatR16Unorm:
    case MTL::PixelFormatR16Snorm:
    case MTL::PixelFormatR16Float:
    case MTL::PixelFormatR16Uint:
    case MTL::PixelFormatB5G6R5Unorm:
    case MTL::PixelFormatDepth16Unorm:
      return 2;
    case MTL::PixelFormatRGBA8Unorm:
    case MTL::PixelFormatRGBA8Unorm_sRGB:
    case MTL::PixelFormatRGBA8Snorm:
    case MTL::PixelFormatRGBA8Uint:
    case MTL::PixelFormatBGRA8Unorm:
    case MTL::PixelFormatBGRA8Unorm_sRGB:
    case MTL::PixelFormatRGB10A2Unorm:
    case MTL::PixelFormatBGR10A2Unorm:
    case MTL::PixelFormatRG11B10Float:
    case MTL::PixelFormatRGB9E5Float:
    case MTL::PixelFormatRG16Unorm:
    case MTL::PixelFormatRG16Snorm:
    case MTL::PixelFormatRG16Float:
    case MTL::PixelFormatRG16Uint:
    case MTL::PixelFormatR32Float:
    case MTL::PixelFormatR32Uint:
    case MTL::PixelFormatR32Sint:
    case MTL::PixelFormatDepth32Float:
    // The depth plane; the stencil plane is counted with the stencil
    // attachment.
    case MTL::PixelFormatDepth32Float_Stencil8:
      return 4;
    case MTL::PixelFormatRGBA16Unorm:
    case MTL::PixelFormatRGBA16Snorm:
    case MTL::PixelFormatRGBA16Float:
    case MTL::PixelFormatRGBA16Uint:
    case MTL::PixelFormatRG32Float:
    case MTL::PixelFormatRG32Uint:
      return 8;
    case MTL::PixelFormatRGBA32Float:
    case MTL::PixelFormatRGBA32Uint:
      return 16;
    default:
      return 0;
  }
}

}  // namespace

void MetalGpuPassTimer::Initialize(MTL::Device* device) {
  std::lock_guard<std::mutex> lock(mutex_);
  // Shaders of an earlier title are gone, and their addresses may be reused.
  pixel_shader_features_.clear();
  stats_by_pipeline_.clear();
  if (device == device_) {
    return;
  }
  // From an earlier device (an earlier title): release the sample buffers no
  // command buffer still writes to; any other is left alive.
  for (size_t i = 0; i < sample_buffers_.size(); ++i) {
    if (!sample_buffer_outstanding_[i]) {
      sample_buffers_[i]->release();
    }
  }
  sample_buffers_.clear();
  sample_buffer_outstanding_.clear();
  stats_by_label_.clear();
  stats_by_pass_.clear();
  untimed_passes_ = 0;
  invalid_passes_ = 0;
  current_sample_buffer_ = 0;
  next_sample_ = 0;
  supported_ = false;
  device_ = device;
  if (!device) {
    return;
  }
  if (!device->supportsCounterSampling(
          MTL::CounterSamplingPointAtStageBoundary)) {
    XELOGW(
        "Metal GPU pass timing: the GPU can't take stage boundary "
        "timestamps, so pass timing is off");
    return;
  }
  MTL::CounterSet* timestamp_set = nullptr;
  // Every counter the GPU offers (Apple GPUs are documented to offer only
  // timestamps; memory traffic counters would be listed here).
  std::string counter_text;
  if (NS::Array* counter_sets = device->counterSets()) {
    for (NS::UInteger i = 0; i < counter_sets->count(); ++i) {
      auto* counter_set = counter_sets->object<MTL::CounterSet>(i);
      if (!counter_set || !counter_set->name()) {
        continue;
      }
      counter_text += fmt::format(" {}:", counter_set->name()->utf8String());
      if (NS::Array* counters = counter_set->counters()) {
        for (NS::UInteger j = 0; j < counters->count(); ++j) {
          auto* counter = counters->object<MTL::Counter>(j);
          if (counter && counter->name()) {
            counter_text += fmt::format(" {}", counter->name()->utf8String());
          }
        }
      }
      if (!timestamp_set && counter_set->name()->isEqualToString(
                                MTL::CommonCounterSetTimestamp)) {
        timestamp_set = counter_set;
      }
    }
  }
  XELOGI("Metal GPU counter sets:{}",
         counter_text.empty() ? std::string(" none") : counter_text);
  if (!timestamp_set) {
    XELOGW(
        "Metal GPU pass timing: the GPU has no timestamp counter set, so pass "
        "timing is off");
    return;
  }
  MTL::CounterSampleBufferDescriptor* descriptor =
      MTL::CounterSampleBufferDescriptor::alloc()->init();
  descriptor->setCounterSet(timestamp_set);
  descriptor->setStorageMode(MTL::StorageModeShared);
  // Each sample buffer is reused once every pass written to it has
  // completed; 16 of them cover several frames in flight.
  for (uint32_t size : {4096u, 2048u, 1024u}) {
    descriptor->setSampleCount(size);
    for (uint32_t i = 0; i < 16; ++i) {
      NS::Error* error = nullptr;
      MTL::CounterSampleBuffer* sample_buffer =
          device->newCounterSampleBuffer(descriptor, &error);
      if (!sample_buffer) {
        break;
      }
      sample_buffers_.push_back(sample_buffer);
      sample_buffer_outstanding_.push_back(0);
    }
    if (sample_buffers_.size() == 16) {
      sample_buffer_size_ = size;
      break;
    }
    for (MTL::CounterSampleBuffer* sample_buffer : sample_buffers_) {
      sample_buffer->release();
    }
    sample_buffers_.clear();
    sample_buffer_outstanding_.clear();
  }
  descriptor->release();
  if (sample_buffers_.empty()) {
    XELOGW(
        "Metal GPU pass timing: couldn't create counter sample buffers, so "
        "pass timing is off");
    return;
  }
  // GPU timestamps are in GPU ticks: calibrate them against the steady
  // clock, here over 20 ms, and then over the whole run at each log.
  MTL::Timestamp cpu_timestamp = 0;
  MTL::Timestamp gpu_timestamp = 0;
  device->sampleTimestamps(&cpu_timestamp, &calibration_gpu_);
  calibration_time_ = std::chrono::steady_clock::now();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  device->sampleTimestamps(&cpu_timestamp, &gpu_timestamp);
  if (gpu_timestamp > calibration_gpu_) {
    ns_per_tick_ =
        double(std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::steady_clock::now() - calibration_time_)
                   .count()) /
        double(gpu_timestamp - calibration_gpu_);
  }
  supported_ = true;
  XELOGI(
      "Metal GPU pass timing: {} sample buffers of {} timestamps, 1 GPU tick "
      "= {:.3f} ns; {}",
      sample_buffers_.size(), sample_buffer_size_, ns_per_tick_,
      cvars::metal_gpu_pass_timing ? "on" : "off (metal_gpu_pass_timing)");
}

bool MetalGpuPassTimer::Enabled() {
  return supported_ && cvars::metal_gpu_pass_timing;
}

MetalGpuPassRecord* MetalGpuPassTimer::AllocateSamples(
    MTL::CommandBuffer* command_buffer, uint32_t sample_count,
    const char* label) {
  MetalGpuPassRecord* record = nullptr;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sample_buffers_.empty()) {
      return nullptr;
    }
    if (next_sample_ + sample_count > sample_buffer_size_) {
      // Move on to a sample buffer no pending pass writes to.
      bool found = false;
      for (size_t i = 1; i <= sample_buffers_.size(); ++i) {
        size_t candidate =
            (current_sample_buffer_ + i) % sample_buffers_.size();
        if (!sample_buffer_outstanding_[candidate]) {
          current_sample_buffer_ = uint32_t(candidate);
          next_sample_ = 0;
          found = true;
          break;
        }
      }
      if (!found) {
        ++untimed_passes_;
        return nullptr;
      }
    }
    record = new MetalGpuPassRecord;
    record->sample_buffer = sample_buffers_[current_sample_buffer_];
    record->sample_buffer_index = current_sample_buffer_;
    record->first_sample = next_sample_;
    record->sample_count = sample_count;
    record->label = label;
    next_sample_ += sample_count;
    ++sample_buffer_outstanding_[current_sample_buffer_];
  }
  // The handler owns the record; its sample buffer slots are freed when the
  // handler is destroyed, which also happens to a command buffer released
  // without being committed.
  std::shared_ptr<MetalGpuPassRecord> holder(
      record, [this](MetalGpuPassRecord* released) { Release(released); });
  command_buffer->addCompletedHandler(
      [this, holder](MTL::CommandBuffer*) { Complete(holder.get()); });
  return record;
}

MTL::RenderCommandEncoder* MetalGpuPassTimer::RenderEncoder(
    MTL::CommandBuffer* command_buffer, MTL::RenderPassDescriptor* descriptor,
    const char* label, MetalGpuPassRecord** record_out) {
  if (record_out) {
    *record_out = nullptr;
  }
  MetalGpuPassRecord* record = Enabled() && descriptor
                                   ? AllocateSamples(command_buffer, 4, label)
                                   : nullptr;
  if (!record) {
    return command_buffer->renderCommandEncoder(descriptor);
  }
  auto describe_attachment =
      [&](MTL::RenderPassAttachmentDescriptor* attachment,
          uint32_t index) -> uint32_t {
    MTL::Texture* texture = attachment ? attachment->texture() : nullptr;
    if (!texture) {
      return 0;
    }
    if (!record->width) {
      record->width = uint32_t(texture->width());
      record->height = uint32_t(texture->height());
      record->samples = uint32_t(texture->sampleCount());
    }
    MTL::LoadAction load_action = attachment->loadAction();
    MTL::StoreAction store_action = attachment->storeAction();
    record->load_actions |= (uint32_t(load_action) & 3) << (index * 2);
    record->store_actions |= (uint32_t(store_action) & 3) << (index * 2);
    if (texture->storageMode() != MTL::StorageModeMemoryless) {
      const uint64_t pixel_bytes =
          MetalGpuBytesPerPixel(texture->pixelFormat(), index == 5);
      const uint64_t attachment_bytes = pixel_bytes * texture->width() *
                                        texture->height() *
                                        texture->sampleCount();
      if (load_action == MTL::LoadActionLoad) {
        record->load_bytes += attachment_bytes;
      }
      if (store_action == MTL::StoreActionStore ||
          store_action == MTL::StoreActionStoreAndMultisampleResolve) {
        record->store_bytes += attachment_bytes;
      }
      if (store_action == MTL::StoreActionMultisampleResolve ||
          store_action == MTL::StoreActionStoreAndMultisampleResolve) {
        MTL::Texture* resolve_texture = attachment->resolveTexture();
        if (resolve_texture) {
          record->store_bytes += pixel_bytes * resolve_texture->width() *
                                 resolve_texture->height();
        }
      }
    }
    return uint32_t(texture->pixelFormat());
  };
  for (uint32_t i = 0; i < 4; ++i) {
    record->color_formats[i] =
        describe_attachment(descriptor->colorAttachments()->object(i), i);
  }
  record->depth_format = describe_attachment(descriptor->depthAttachment(), 4);
  describe_attachment(descriptor->stencilAttachment(), 5);

  MTL::RenderPassSampleBufferAttachmentDescriptor* sample_attachment =
      descriptor->sampleBufferAttachments()->object(0);
  sample_attachment->setSampleBuffer(record->sample_buffer);
  sample_attachment->setStartOfVertexSampleIndex(record->first_sample);
  sample_attachment->setEndOfVertexSampleIndex(record->first_sample + 1);
  sample_attachment->setStartOfFragmentSampleIndex(record->first_sample + 2);
  sample_attachment->setEndOfFragmentSampleIndex(record->first_sample + 3);
  MTL::RenderCommandEncoder* encoder =
      command_buffer->renderCommandEncoder(descriptor);
  // The descriptor is reused for passes created elsewhere.
  sample_attachment->setSampleBuffer(nullptr);
  sample_attachment->setStartOfVertexSampleIndex(MTL::CounterDontSample);
  sample_attachment->setEndOfVertexSampleIndex(MTL::CounterDontSample);
  sample_attachment->setStartOfFragmentSampleIndex(MTL::CounterDontSample);
  sample_attachment->setEndOfFragmentSampleIndex(MTL::CounterDontSample);
  if (!encoder) {
    // Not written to: Complete discards it.
    record->label = nullptr;
    return nullptr;
  }
  if (record_out) {
    *record_out = record;
  }
  return encoder;
}

MTL::ComputeCommandEncoder* MetalGpuPassTimer::ComputeEncoder(
    MTL::CommandBuffer* command_buffer, const char* label) {
  MetalGpuPassRecord* record =
      Enabled() ? AllocateSamples(command_buffer, 2, label) : nullptr;
  if (!record) {
    return command_buffer->computeCommandEncoder();
  }
  MTL::ComputePassDescriptor* descriptor =
      MTL::ComputePassDescriptor::alloc()->init();
  MTL::ComputePassSampleBufferAttachmentDescriptor* sample_attachment =
      descriptor->sampleBufferAttachments()->object(0);
  sample_attachment->setSampleBuffer(record->sample_buffer);
  sample_attachment->setStartOfEncoderSampleIndex(record->first_sample);
  sample_attachment->setEndOfEncoderSampleIndex(record->first_sample + 1);
  MTL::ComputeCommandEncoder* encoder =
      command_buffer->computeCommandEncoder(descriptor);
  descriptor->release();
  if (!encoder) {
    record->label = nullptr;
  }
  return encoder;
}

MTL::BlitCommandEncoder* MetalGpuPassTimer::BlitEncoder(
    MTL::CommandBuffer* command_buffer, const char* label) {
  MetalGpuPassRecord* record =
      Enabled() ? AllocateSamples(command_buffer, 2, label) : nullptr;
  if (!record) {
    return command_buffer->blitCommandEncoder();
  }
  MTL::BlitPassDescriptor* descriptor =
      MTL::BlitPassDescriptor::alloc()->init();
  MTL::BlitPassSampleBufferAttachmentDescriptor* sample_attachment =
      descriptor->sampleBufferAttachments()->object(0);
  sample_attachment->setSampleBuffer(record->sample_buffer);
  sample_attachment->setStartOfEncoderSampleIndex(record->first_sample);
  sample_attachment->setEndOfEncoderSampleIndex(record->first_sample + 1);
  MTL::BlitCommandEncoder* encoder =
      command_buffer->blitCommandEncoder(descriptor);
  descriptor->release();
  if (!encoder) {
    record->label = nullptr;
  }
  return encoder;
}

void MetalGpuPassTimer::Complete(MetalGpuPassRecord* record) {
  // Metal's completion queue may have no autorelease pool of its own.
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  uint64_t timestamps[4] = {};
  bool resolved = false;
  if (record->label) {
    NS::Data* data = record->sample_buffer->resolveCounterRange(
        NS::Range::Make(record->first_sample, record->sample_count));
    if (data && data->length() >= record->sample_count *
                                      sizeof(MTL::CounterResultTimestamp)) {
      auto* results =
          static_cast<const MTL::CounterResultTimestamp*>(data->mutableBytes());
      for (uint32_t i = 0; i < record->sample_count; ++i) {
        timestamps[i] = results[i].timestamp;
      }
      resolved = true;
    }
  }
  pool->release();
  if (!record->label) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  auto valid = [](uint64_t timestamp) {
    return timestamp != 0 && timestamp != MTL::CounterErrorValue;
  };
  uint64_t vertex_ticks = 0;
  uint64_t fragment_ticks = 0;
  uint64_t total_ticks = 0;
  bool usable = resolved;
  if (usable && record->sample_count == 4) {
    bool vertex_ok = valid(timestamps[0]) && valid(timestamps[1]) &&
                     timestamps[1] >= timestamps[0];
    bool fragment_ok = valid(timestamps[2]) && valid(timestamps[3]) &&
                       timestamps[3] >= timestamps[2];
    // Sample buffers aren't cleared between uses, and a stage that didn't
    // run may leave an earlier pass's timestamps: drop a stage that ended
    // over a second before the other one.
    const uint64_t stale_ticks = uint64_t(1e9 / ns_per_tick_);
    if (vertex_ok && fragment_ok) {
      if (timestamps[1] + stale_ticks < timestamps[3]) {
        vertex_ok = false;
      } else if (timestamps[3] + stale_ticks < timestamps[1]) {
        fragment_ok = false;
      }
    }
    if (vertex_ok) {
      vertex_ticks = timestamps[1] - timestamps[0];
    }
    if (fragment_ok) {
      fragment_ticks = timestamps[3] - timestamps[2];
    }
    if (vertex_ok && fragment_ok) {
      total_ticks = std::max(timestamps[1], timestamps[3]) -
                    std::min(timestamps[0], timestamps[2]);
    } else {
      total_ticks = vertex_ticks + fragment_ticks;
    }
    usable = vertex_ok || fragment_ok;
  } else if (usable) {
    usable = valid(timestamps[0]) && valid(timestamps[1]) &&
             timestamps[1] >= timestamps[0];
    if (usable) {
      total_ticks = timestamps[1] - timestamps[0];
    }
  }
  if (!usable) {
    ++invalid_passes_;
    return;
  }
  auto add = [&](MetalGpuPassStats& stats) {
    ++stats.passes;
    stats.draws += record->draws;
    stats.vertices += record->vertices;
    stats.total_ticks += total_ticks;
    stats.vertex_ticks += vertex_ticks;
    stats.fragment_ticks += fragment_ticks;
    stats.load_bytes += record->load_bytes;
    stats.store_bytes += record->store_bytes;
  };
  add(stats_by_label_[record->label]);
  // Each pipeline gets the fragment time by its share of the draws and the
  // vertex time by its share of the vertices.
  for (const MetalGpuPassRecord::MetalGpuPassPipeline& pipeline :
       record->pipelines) {
    MetalGpuPassStats& stats = stats_by_pipeline_[pipeline.shaders];
    ++stats.passes;
    stats.draws += pipeline.draws;
    stats.vertices += pipeline.vertices;
    const uint64_t pipeline_vertex_ticks =
        record->vertices
            ? uint64_t(double(vertex_ticks) * pipeline.vertices /
                       record->vertices)
            : uint64_t(double(vertex_ticks) * pipeline.draws / record->draws);
    const uint64_t pipeline_fragment_ticks =
        uint64_t(double(fragment_ticks) * pipeline.draws / record->draws);
    stats.vertex_ticks += pipeline_vertex_ticks;
    stats.fragment_ticks += pipeline_fragment_ticks;
    stats.total_ticks += pipeline_vertex_ticks + pipeline_fragment_ticks;
    if (pipeline.pixel_translation) {
      auto features = pixel_shader_features_.find(pipeline.pixel_translation);
      if (features != pixel_shader_features_.end()) {
        stats.features = features->second.first;
        stats.texture_samples = features->second.second;
      }
    }
  }
  if (record->sample_count == 4) {
    // Formats are MTL::PixelFormat values; load and store actions are 2 bits
    // per attachment, colors 0-3, depth, then stencil, lowest first.
    add(stats_by_pass_[fmt::format(
        "{} {}x{} x{} color {}/{}/{}/{} depth {} load {:03x} store {:03x}",
        record->label, record->width, record->height, record->samples,
        record->color_formats[0], record->color_formats[1],
        record->color_formats[2], record->color_formats[3],
        record->depth_format, record->load_actions, record->store_actions)]);
  } else {
    add(stats_by_pass_[record->label]);
  }
}

void MetalGpuPassTimer::Release(MetalGpuPassRecord* record) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (record->sample_buffer_index < sample_buffers_.size() &&
        sample_buffers_[record->sample_buffer_index] == record->sample_buffer) {
      --sample_buffer_outstanding_[record->sample_buffer_index];
    }
  }
  delete record;
}

std::pair<uint32_t, uint32_t> MetalGpuPassTimer::MetalGpuPixelShaderFeatures(
    MslShader::MslTranslation* pixel_translation) {
  const std::string& source = pixel_translation->msl_source();
  uint32_t features = 0;
  if (source.find("discard_fragment") != std::string::npos) {
    features |= 1;
  }
  if (source.find("[[depth(") != std::string::npos) {
    features |= 2;
  }
  if (source.find("[[raster_order_group(") != std::string::npos) {
    features |= 4;
  }
  uint32_t texture_samples = 0;
  for (const char* call :
       {".sample(", ".sample_compare(", ".gather(", ".read("}) {
    for (size_t position = source.find(call); position != std::string::npos;
         position = source.find(call, position + 1)) {
      ++texture_samples;
    }
  }
  return {features, texture_samples};
}

void MetalGpuPassTimer::NoteDraw(MetalGpuPassRecord* record,
                                 MslShader::MslTranslation* vertex_translation,
                                 MslShader::MslTranslation* pixel_translation,
                                 uint64_t vertices) {
  if (!record) {
    return;
  }
  ++record->draws;
  record->vertices += vertices;
  if (record->pipelines.empty() ||
      record->pipelines.back().vertex_translation != vertex_translation ||
      record->pipelines.back().pixel_translation != pixel_translation) {
    MetalGpuPassRecord::MetalGpuPassPipeline pipeline;
    pipeline.vertex_translation = vertex_translation;
    pipeline.pixel_translation = pixel_translation;
    if (vertex_translation) {
      pipeline.shaders[0] = vertex_translation->shader().ucode_data_hash();
      pipeline.shaders[1] = vertex_translation->modification();
    }
    if (pixel_translation) {
      pipeline.shaders[2] = pixel_translation->shader().ucode_data_hash();
      pipeline.shaders[3] = pixel_translation->modification();
      std::lock_guard<std::mutex> lock(mutex_);
      if (pixel_shader_features_.find(pixel_translation) ==
          pixel_shader_features_.end()) {
        pixel_shader_features_.emplace(
            pixel_translation, MetalGpuPixelShaderFeatures(pixel_translation));
      }
    }
    record->pipelines.push_back(pipeline);
  }
  ++record->pipelines.back().draws;
  record->pipelines.back().vertices += vertices;
}

void MetalGpuPassTimer::Log(uint32_t frames, uint64_t window_us) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!supported_ || !frames ||
      (stats_by_label_.empty() && !untimed_passes_ && !invalid_passes_)) {
    return;
  }
  MTL::Timestamp cpu_timestamp = 0;
  MTL::Timestamp gpu_timestamp = 0;
  device_->sampleTimestamps(&cpu_timestamp, &gpu_timestamp);
  if (gpu_timestamp > calibration_gpu_) {
    ns_per_tick_ =
        double(std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::steady_clock::now() - calibration_time_)
                   .count()) /
        double(gpu_timestamp - calibration_gpu_);
  }
  auto ms_per_frame = [&](uint64_t ticks) {
    return double(ticks) * ns_per_tick_ / 1e6 / frames;
  };
  auto describe = [&](const MetalGpuPassStats& stats) {
    std::string text = fmt::format(
        "{:.2f} ms, {:.1f} passes, {:.0f} draws, {:.0f} vertices",
        ms_per_frame(stats.total_ticks), double(stats.passes) / frames,
        double(stats.draws) / frames, double(stats.vertices) / frames);
    if (stats.vertex_ticks || stats.fragment_ticks) {
      text += fmt::format(" (vertex {:.2f} ms, fragment {:.2f} ms)",
                          ms_per_frame(stats.vertex_ticks),
                          ms_per_frame(stats.fragment_ticks));
    }
    if (stats.load_bytes || stats.store_bytes) {
      text += fmt::format(", attachments {:.1f} MB loaded, {:.1f} MB stored",
                          stats.load_bytes / 1e6 / frames,
                          stats.store_bytes / 1e6 / frames);
    }
    return text;
  };
  auto by_time = [](const auto& stats) {
    std::vector<
        std::pair<decltype(&stats.begin()->first), const MetalGpuPassStats*>>
        sorted;
    for (const auto& [key, value] : stats) {
      sorted.emplace_back(&key, &value);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& metal_perf_left, const auto& metal_perf_right) {
                return metal_perf_left.second->total_ticks >
                       metal_perf_right.second->total_ticks;
              });
    return sorted;
  };
  // Passes can overlap on the GPU, so the sum can exceed the GPU busy time.
  std::string text;
  for (const auto& [key, value] : by_time(stats_by_label_)) {
    text += fmt::format("; {}: {}", *key, describe(*value));
  }
  XELOGI(
      "Metal GPU passes per frame over {} frames (1 GPU tick = {:.3f} ns, "
      "untimed {}, unusable {}){}",
      frames, ns_per_tick_, untimed_passes_, invalid_passes_, text);
  text.clear();
  size_t listed = 0;
  for (const auto& [key, value] : by_time(stats_by_pass_)) {
    if (++listed > 12) {
      break;
    }
    text += fmt::format("; {}: {}", *key, describe(*value));
  }
  XELOGI("Metal GPU top passes per frame{}", text);
  // Main memory traffic estimate from the render pass attachments alone
  // (texture sampling, vertex fetch, compute and blit traffic aren't known).
  uint64_t load_bytes = 0;
  uint64_t store_bytes = 0;
  for (const auto& [key, value] : stats_by_label_) {
    load_bytes += value.load_bytes;
    store_bytes += value.store_bytes;
  }
  const double window_seconds = std::max<uint64_t>(window_us, 1) / 1e6;
  XELOGI(
      "Metal GPU estimated attachment traffic: {:.1f} MB per frame ({:.1f} "
      "MB loaded, {:.1f} MB stored), {:.2f} GB/s over the window",
      (load_bytes + store_bytes) / 1e6 / frames, load_bytes / 1e6 / frames,
      store_bytes / 1e6 / frames,
      (load_bytes + store_bytes) / 1e9 / window_seconds);
  text.clear();
  listed = 0;
  for (const auto& [key, value] : by_time(stats_by_pipeline_)) {
    if (++listed > 12) {
      break;
    }
    std::string features;
    if (value->features & 1) {
      features += " discard";
    }
    if (value->features & 2) {
      features += " depth-output";
    }
    if (value->features & 4) {
      features += " raster-order";
    }
    text += fmt::format(
        "; VS {:016X} mod {:016X} PS {:016X} mod {:016X}: ~{:.2f} ms "
        "(vertex {:.2f}, fragment {:.2f}), {:.0f} draws, {:.0f} vertices, "
        "{:.1f} passes, {} texture samples{}{}",
        (*key)[0], (*key)[1], (*key)[2], (*key)[3],
        ms_per_frame(value->total_ticks), ms_per_frame(value->vertex_ticks),
        ms_per_frame(value->fragment_ticks), double(value->draws) / frames,
        double(value->vertices) / frames, double(value->passes) / frames,
        value->texture_samples, features.empty() ? "" : ",", features);
  }
  XELOGI(
      "Metal GPU top pipelines per frame (pass time shared by draws and "
      "vertices, an estimate){}",
      text);
  stats_by_label_.clear();
  stats_by_pass_.clear();
  stats_by_pipeline_.clear();
  untimed_passes_ = 0;
  invalid_passes_ = 0;
}

void MetalCompileStallTracker::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  pending_.clear();
  finished_.clear();
  for (size_t i = 0; i < 8; ++i) {
    bucket_compiles_[i] = 0;
    bucket_skipped_[i] = 0;
  }
  queue_us_ = 0;
  compile_us_ = 0;
}

void MetalCompileStallTracker::Queued(const MetalCompileKey& key) {
  if (!cvars::metal_gpu_pass_timing) {
    return;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  if (pending_.find(key) == pending_.end()) {
    MetalCompileStall stall;
    stall.key = key;
    stall.queued_at = std::chrono::steady_clock::now();
    pending_.emplace(key, stall);
  }
}

void MetalCompileStallTracker::Skipped(const MetalCompileKey& key) {
  if (!cvars::metal_gpu_pass_timing) {
    return;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  // Unknown: queued while tracking was off, or a draw that saw the compile
  // pending just before it finished.
  auto stall = pending_.find(key);
  if (stall != pending_.end()) {
    ++stall->second.skipped;
  }
}

void MetalCompileStallTracker::Finished(const MetalCompileKey& key,
                                        uint64_t compile_us) {
  std::string note;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto pending = pending_.find(key);
    if (pending == pending_.end()) {
      return;
    }
    MetalCompileStall stall = pending->second;
    pending_.erase(pending);
    stall.pending_us =
        uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(
                     std::chrono::steady_clock::now() - stall.queued_at)
                     .count());
    stall.compile_us = std::min(compile_us, stall.pending_us);
    static const uint64_t bucket_limits_ms[7] = {16,  33,  50,  100,
                                                 250, 500, 1000};
    size_t bucket = 0;
    while (bucket < 7 && stall.pending_us > bucket_limits_ms[bucket] * 1000) {
      ++bucket;
    }
    ++bucket_compiles_[bucket];
    bucket_skipped_[bucket] += stall.skipped;
    queue_us_ += stall.pending_us - stall.compile_us;
    compile_us_ += stall.compile_us;
    if (stall.skipped) {
      finished_.push_back(stall);
      // A note of every compile draws waited 100 ms or more for.
      if (stall.pending_us >= 100000) {
        note = fmt::format(
            "Metal compile stall: {} pending {:.0f} ms (queue {:.0f} ms, "
            "compile {:.0f} ms), {} draws skipped",
            Describe(stall.key), stall.pending_us / 1000.0,
            (stall.pending_us - stall.compile_us) / 1000.0,
            stall.compile_us / 1000.0, stall.skipped);
      }
    }
  }
  if (!note.empty()) {
    XELOGI("{}", note);
  }
}

std::string MetalCompileStallTracker::Describe(const MetalCompileKey& key) {
  switch (key[0]) {
    case 1:
      return fmt::format("VS {:016X} mod {:016X}", key[1], key[2]);
    case 2:
      return fmt::format("PS {:016X} mod {:016X}", key[1], key[2]);
    default:
      return fmt::format(
          "pipeline VS {:016X} mod {:016X} PS {:016X} mod {:016X}", key[1],
          key[2], key[3], key[4]);
  }
}

void MetalCompileStallTracker::Log(uint32_t frames) {
  std::lock_guard<std::mutex> lock(mutex_);
  uint64_t compiles = 0;
  uint64_t skipped = 0;
  for (size_t i = 0; i < 8; ++i) {
    compiles += bucket_compiles_[i];
    skipped += bucket_skipped_[i];
  }
  uint64_t waiting = 0;
  uint64_t waiting_skipped = 0;
  uint64_t oldest_us = 0;
  const auto now = std::chrono::steady_clock::now();
  for (const auto& [key, stall] : pending_) {
    if (!stall.skipped) {
      continue;
    }
    ++waiting;
    waiting_skipped += stall.skipped;
    oldest_us = std::max<uint64_t>(
        oldest_us,
        uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(
                     now - stall.queued_at)
                     .count()));
  }
  if (!compiles && !waiting) {
    return;
  }
  // Draws skipped per bucket: a bounded wait as long as a bucket's limit
  // would have drawn those (stalling the frame instead).
  static const char* const bucket_names[8] = {
      "<=16 ms",  "<=33 ms",  "<=50 ms", "<=100 ms",
      "<=250 ms", "<=500 ms", "<=1 s",   ">1 s"};
  std::string text;
  for (size_t i = 0; i < 8; ++i) {
    text += fmt::format("{}{} {}/{}", i ? ", " : "", bucket_names[i],
                        bucket_compiles_[i], bucket_skipped_[i]);
  }
  std::sort(finished_.begin(), finished_.end(),
            [](const MetalCompileStall& metal_perf_left,
               const MetalCompileStall& metal_perf_right) {
              return metal_perf_left.skipped > metal_perf_right.skipped;
            });
  std::string worst;
  for (size_t i = 0; i < std::min<size_t>(finished_.size(), 8); ++i) {
    worst += fmt::format(
        "; {}: {} draws skipped, pending {:.0f} ms (compile "
        "{:.0f} ms)",
        Describe(finished_[i].key), finished_[i].skipped,
        finished_[i].pending_us / 1000.0, finished_[i].compile_us / 1000.0);
  }
  XELOGI(
      "Metal compile stalls over {} frames: {} compiles finished, {} draws "
      "skipped for them, queue wait {:.0f} ms and compiling {:.0f} ms in "
      "total; by time pending (compiles/draws skipped): {}; still pending "
      "with draws skipped: {} (oldest {:.0f} ms, {} draws skipped so far){}",
      frames, compiles, skipped, queue_us_ / 1000.0, compile_us_ / 1000.0, text,
      waiting, oldest_us / 1000.0, waiting_skipped, worst);
  for (size_t i = 0; i < 8; ++i) {
    bucket_compiles_[i] = 0;
    bucket_skipped_[i] = 0;
  }
  queue_us_ = 0;
  compile_us_ = 0;
  finished_.clear();
}

}  // namespace metal
}  // namespace gpu
}  // namespace xe
