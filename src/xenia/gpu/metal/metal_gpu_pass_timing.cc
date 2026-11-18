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
            "blit pass with Metal's stage boundary timestamps, and log where "
            "the GPU time goes with the Metal perf lines.",
            "Metal");

namespace xe {
namespace gpu {
namespace metal {

void MetalGpuPassTimer::Initialize(MTL::Device* device) {
  std::lock_guard<std::mutex> lock(mutex_);
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
  if (NS::Array* counter_sets = device->counterSets()) {
    for (NS::UInteger i = 0; i < counter_sets->count(); ++i) {
      auto* counter_set = counter_sets->object<MTL::CounterSet>(i);
      if (counter_set && counter_set->name() &&
          counter_set->name()->isEqualToString(
              MTL::CommonCounterSetTimestamp)) {
        timestamp_set = counter_set;
        break;
      }
    }
  }
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
    record->load_actions |= (uint32_t(attachment->loadAction()) & 3)
                            << (index * 2);
    record->store_actions |= (uint32_t(attachment->storeAction()) & 3)
                             << (index * 2);
    return uint32_t(texture->pixelFormat());
  };
  for (uint32_t i = 0; i < 4; ++i) {
    record->color_formats[i] =
        describe_attachment(descriptor->colorAttachments()->object(i), i);
  }
  record->depth_format = describe_attachment(descriptor->depthAttachment(), 4);

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
  };
  add(stats_by_label_[record->label]);
  if (record->sample_count == 4) {
    // Formats are MTL::PixelFormat values; load and store actions are 2 bits
    // per attachment, colors 0-3 then depth, lowest first.
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

void MetalGpuPassTimer::Log(uint32_t frames) {
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
    return text;
  };
  auto by_time = [](const std::map<std::string, MetalGpuPassStats>& stats) {
    std::vector<std::pair<const std::string*, const MetalGpuPassStats*>> sorted;
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
  stats_by_label_.clear();
  stats_by_pass_.clear();
  untimed_passes_ = 0;
  invalid_passes_ = 0;
}

}  // namespace metal
}  // namespace gpu
}  // namespace xe
