# iOS 16 backport: names added to this branch

Every identifier introduced by the backport work that does not already exist in
`xenios-jp/XeniOS` (upstream) under the same name and role. Identifiers copied
from upstream code (for example the locals in `SDLInputDriver::EnumerateDevices`)
are not listed. The fields of `DrawStats` (`frames`, `issued`, `copies`,
`no_rasterization`, `no_vertices`, `compile_pending`, `encoded`,
`encoded_vertices`) are declared in `metal_command_processor.h` and used only
through `draw_stats_`.

Line numbers are as of the commit that last touched this file; regenerate after
further edits.

| Name | Kind | Purpose | File | Lines | Uses in file |
|---|---|---|---|---|---|
| `shared_drivers_mutex_` | static member of SDLAudioDriver (iOS only) | Guards shared_drivers_ and the shared SDL device. | `src/xenia/apu/sdl/sdl_audio_driver.h` | 50, 51 | 2 |
| `shared_drivers_mutex_` | static member of SDLAudioDriver (iOS only) | Guards shared_drivers_ and the shared SDL device. | `src/xenia/apu/sdl/sdl_audio_driver.cc` | 26, 77, 171, 210 | 4 |
| `shared_drivers_` | static member of SDLAudioDriver (iOS only) | Every driver attached to the one iOS output device; the callback mixes them. | `src/xenia/apu/sdl/sdl_audio_driver.h` | 52 | 1 |
| `shared_drivers_` | static member of SDLAudioDriver (iOS only) | Every driver attached to the one iOS output device; the callback mixes them. | `src/xenia/apu/sdl/sdl_audio_driver.cc` | 27, 78, 81, 82, 84, 88, 89, 90, 133, 172, 174, 178, 217 | 14 |
| `RenderFrame` | member function of SDLAudioDriver | The previous body of SDLCallback, split out so several drivers can be rendered into one stream. | `src/xenia/apu/sdl/sdl_audio_driver.h` | 45 | 1 |
| `RenderFrame` | member function of SDLAudioDriver | The previous body of SDLCallback, split out so several drivers can be rendered into one stream. | `src/xenia/apu/sdl/sdl_audio_driver.cc` | 218, 224, 228 | 3 |
| `shared_lock` | local (lock guard) | Holds shared_drivers_mutex_. | `src/xenia/apu/sdl/sdl_audio_driver.cc` | 77, 171, 210 | 3 |
| `mix_frame` | local in SDLCallback (iOS only) | Scratch buffer one driver is rendered into before being summed. | `src/xenia/apu/sdl/sdl_audio_driver.cc` | 212, 213, 218, 220 | 4 |
| `mixed` | local in SDLCallback (iOS only) | The output stream viewed as float samples. | `src/xenia/apu/sdl/sdl_audio_driver.cc` | 216, 220 | 2 |
| `session_log_path` | function-local static in GetLogFilePath | The log file chosen for this launch; read by the logger setup and the in-app log viewer. | `src/xenia/ui/windowed_app_main_ios.mm` | 1256, 1269 | 2 |
| `now` | local in GetLogFilePath | Current time used to name the log file. | `src/xenia/ui/windowed_app_main_ios.mm` | 1261, 1263 | 2 |
| `local_time` | local in GetLogFilePath | now broken down in the device's time zone. | `src/xenia/ui/windowed_app_main_ios.mm` | 1262, 1263, 1266 | 3 |
| `log_name` | local in GetLogFilePath | File name xenia_<date>_<time>.log. | `src/xenia/ui/windowed_app_main_ios.mm` | 1264, 1265, 1267 | 4 |
| `kText` | enumerator of IOSConfigControlType | Settings row whose value is edited as text. | `src/xenia/ui/windowed_app_main_ios.mm` | 542, 1227, 3701, 8043, 8096 | 5 |
| `AddAllConfigVarSettings` | function | Adds a settings section per config category with every variable not already shown. | `src/xenia/ui/windowed_app_main_ios.mm` | 1201, 3652 | 2 |
| `existing_keys` | local in AddAllConfigVarSettings | Config keys the curated sections already expose, so they are not listed twice. | `src/xenia/ui/windowed_app_main_ios.mm` | 1205, 1209, 1215 | 3 |
| `by_category` | local in AddAllConfigVarSettings | Generated sections keyed by config category. | `src/xenia/ui/windowed_app_main_ios.mm` | 1213, 1231, 1235 | 3 |
| `parsed` | local in ApplyIOSConfigSections | The edited text parsed as a TOML value. | `src/xenia/ui/windowed_app_main_ios.mm` | 3715, 3717, 3720, 3727 | 4 |
| `editor` | local in tableView:didSelectRowAtIndexPath: | Alert with a text field used to edit a kText row. | `src/xenia/ui/windowed_app_main_ios.mm` | 8100, 8104, 8110, 8113, 8116, 8126 | 6 |
| `DrawStats` | struct in MetalCommandProcessor | Counters describing what happened to guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 542, 552 | 2 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.h` | 552 | 1 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2525, 2532, 2533, 2534, 2535, 2536, 2537, 2538, 2539, 2803, 2805, 2841, 2866, 4231, 4255, 4290, 5596, 5597 | 25 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.h` | 539, 541 | 2 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2525 | 1 |
| `app_version` | shell variable, CI Prepare artifacts step | CFBundleShortVersionString of the built app. | `.github/workflows/CI.yml` | 662, 664 | 2 |
| `app_build` | shell variable, CI Prepare artifacts step | CFBundleVersion of the built app. | `.github/workflows/CI.yml` | 663, 664 | 2 |
| `artifact_suffix` | shell variable and step output, CI | <version>-<build>, appended to the IPA and artifact names. | `.github/workflows/CI.yml` | 664, 665, 668, 682 | 5 |
| `boost_context` | premake project name | Builds the fcontext assembly used by threading_fiber.cc. Same name as upstream's CMake target. | `third_party/boost_context.lua` | 2, 10, 11, 12, 16, 17, 18, 22, 23, 24, 28, 29, 30 | 13 |
| `boost_context` | premake project name | Builds the fcontext assembly used by threading_fiber.cc. Same name as upstream's CMake target. | `premake5.lua` | 762 | 1 |
| `boost_context` | premake project name | Builds the fcontext assembly used by threading_fiber.cc. Same name as upstream's CMake target. | `src/xenia/app/premake5.lua` | 39 | 1 |
| `probe_count` | function-local static in LoadTextureDataFromResidentMemoryImpl | Counts 1280x720 uncompressed texture loads so the frame texture probe samples every 512th. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1368, 1370 | 2 |
| `probe_index` | local, frame texture probe | Value of `probe_count` for this load; selects which loads are probed. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1370, 1371, 1399 | 4 |
| `probe_format` | local, frame texture probe | Guest texture format of the probed texture. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1373, 1399 | 2 |
| `probe_base` | local, frame texture probe | Guest address of the probed texture. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1374, 1399 | 2 |
| `probe_size` | local, frame texture probe | Size in bytes of the converted texture data. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1375, 1381, 1382, 1387, 1399 | 5 |
| `probe_data` | local, frame texture probe | Pointer to the converted bytes. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1377, 1381, 1383, 1391 | 4 |
| `probe_nonzero` | local, frame texture probe | How many of the bytes are non-zero. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1379, 1383, 1399 | 3 |
| `probe_texels` | local, frame texture probe | Eight bytes from the middle column at 1/4, 1/2 and 3/4 of the image height. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1380, 1390, 1400 | 5 |
| `probe_row` | local, frame texture probe | Bytes per image row, used to locate the sampled texels. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1387, 1391 | 3 |
| `probe_count` | function-local static in MetalRenderTargetCache::Resolve (separate from the one in the texture cache) | Counts resolves so the resolve probe samples every 97th. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4948, 4949 | 2 |
| `probe_index` | local, resolve probe in MetalRenderTargetCache::Resolve | Value of `probe_count` for this resolve. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4949, 4952, 4953, 5000 | 4 |
| `probe_edram_size` | local, resolve probe in MetalRenderTargetCache::Resolve | Size in bytes of the EDRAM buffer. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4950, 4957, 4963, 4966, 4973, 4975, 4978, 4988 | 8 |
| `probe_dest_size` | local, resolve probe in MetalRenderTargetCache::Resolve | Size in bytes of the range the resolve wrote. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4951, 4954, 4957, 4966, 4986, 5007 | 6 |
| `probe_buffer` | local, resolve probe in MetalRenderTargetCache::Resolve | Temporary CPU-readable buffer holding the snapshot of EDRAM and of the written range. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4956, 4960, 4962, 4966, 4970, 5008, 5010, 5011 | 8 |
| `probe_blit` | local, resolve probe in MetalRenderTargetCache::Resolve | Blit encoder that copies the snapshot. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4959, 4961, 4962, 4964, 4967 | 5 |
| `probe_data` | local, resolve probe in MetalRenderTargetCache::Resolve | Pointer to the snapshot bytes. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4969, 4981, 4984, 4988 | 4 |
| `probe_tile_size` | local, resolve probe in MetalRenderTargetCache::Resolve | Bytes per EDRAM tile. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4972, 4975, 4977 | 3 |
| `probe_span_start` | local, resolve probe in MetalRenderTargetCache::Resolve | Byte offset in EDRAM of the first dumped tile. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4974, 4978, 4984 | 3 |
| `probe_span_size` | local, resolve probe in MetalRenderTargetCache::Resolve | Byte length of the dumped tile span. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4976, 4982, 5006 | 3 |
| `probe_edram_nonzero` | local, resolve probe in MetalRenderTargetCache::Resolve | Non-zero bytes in the dumped EDRAM span. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4979, 4983, 5006 | 3 |
| `probe_dest_nonzero` | local, resolve probe in MetalRenderTargetCache::Resolve | Non-zero bytes in the written range. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4980, 4987, 5006 | 3 |
| `probe_source` | local, resolve probe in MetalRenderTargetCache::Resolve | EDRAM info (format, MSAA, depth flag) of the resolve source. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4991, 5001, 5002, 5003 | 4 |
| `probed` | field of `DrawStats` | Number of scene draws logged by the scene draw probe in the current statistics window. | `src/xenia/gpu/metal/metal_command_processor.h` | 551 | 1 |
| `probed` | field of `DrawStats` | Number of scene draws logged by the scene draw probe in the current statistics window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5541, 5545, 5614, 5621, 5635, 6119 | 6 |
| `msaa_probe_count` | function-local static in MetalRenderTargetCache::DumpRenderTargets | Counts dumps of multisampled RGBA16Float targets so the multisample probe samples every 61st. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4121, 4131 | 2 |
| `msaa_probe_rt` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Render target of the dump rectangle being considered. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4123, 4125, 4168 | 4 |
| `msaa_probe_source` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Multisampled texture of that render target. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4124, 4126, 4127, 4128, 4137, 4138, 4156 | 7 |
| `msaa_probe_index` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Value of `msaa_probe_count` for this dump. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4131, 4132, 4184 | 4 |
| `msaa_probe_desc` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Descriptor of the temporary single-sample texture. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4135, 4137, 4138, 4139, 4140, 4141, 4142, 4143, 4144, 4145 | 10 |
| `msaa_probe_texture` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Temporary CPU-readable texture Metal resolves the target into. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4144, 4148, 4149, 4150, 4157, 4164, 4170, 4171, 4174, 4187 | 10 |
| `msaa_probe_pass` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Render pass descriptor that performs the resolve. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4146, 4148, 4155, 4162 | 4 |
| `msaa_probe_attachment` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Colour attachment of that pass. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4154, 4156, 4157, 4158, 4159 | 5 |
| `msaa_probe_encoder` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Encoder of the (empty) resolve pass. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4161, 4163, 4167 | 3 |
| `msaa_probe_key` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Render target key, for the log line. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4168, 4184 | 2 |
| `msaa_probe_width` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Width of the resolved texture. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4170, 4172, 4175, 4176, 4184 | 5 |
| `msaa_probe_height` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Height of the resolved texture. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4171, 4173, 4176, 4184 | 4 |
| `msaa_probe_bytes` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Resolved pixels copied to the CPU. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4172, 4175, 4178, 4186 | 4 |
| `msaa_probe_nonzero` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Number of non-zero bytes in the resolved pixels. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4177, 4179, 4185 | 3 |
| `msaa_probe_byte` | local, multisample probe in MetalRenderTargetCache::DumpRenderTargets | Loop variable over the resolved bytes. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4178, 4179 | 2 |
| `visibility_probe_buffer_` | member of MetalCommandProcessor | Buffer Metal writes the passed-sample counts of probed scene draws into; attached to every render pass. | `src/xenia/gpu/metal/metal_command_processor.h` | 555 | 1 |
| `visibility_probe_buffer_` | member of MetalCommandProcessor | Buffer Metal writes the passed-sample counts of probed scene draws into; attached to every render pass. | `src/xenia/gpu/metal/metal_command_processor.cc` | 586, 587, 588, 5540, 5619, 6120, 6121, 6124 | 8 |
| `probe_buffer` | local, scene pixel probe in IssueDrawMsl | Copy of `visibility_probe_buffer_` kept alive for the completion handler. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5619, 5620, 5627, 5632 | 4 |
| `probe_slot` | local, scene pixel probe in IssueDrawMsl | Index of the 8-byte counter this draw used. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5621, 5628, 5631 | 3 |
| `probe_vertices` | local, scene pixel probe in IssueDrawMsl | Vertex count of the probed draw, to pair the line with the scene draw probe line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5622, 5631 | 2 |
| `probe_samples` | local, scene pixel probe in IssueDrawMsl | Number of samples the draw passed, read back from the buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5625, 5626, 5629, 5631 | 4 |

## Total references per name

| Name | Total uses in project |
|---|---|
| `shared_drivers_mutex_` | 6 |
| `shared_drivers_` | 15 |
| `RenderFrame` | 4 |
| `shared_lock` | 3 |
| `mix_frame` | 4 |
| `mixed` | 2 |
| `session_log_path` | 2 |
| `now` | 2 |
| `local_time` | 3 |
| `log_name` | 4 |
| `kText` | 5 |
| `AddAllConfigVarSettings` | 2 |
| `existing_keys` | 3 |
| `by_category` | 3 |
| `parsed` | 4 |
| `editor` | 6 |
| `DrawStats` | 2 |
| `draw_stats_` | 26 |
| `kDrawStatsFrames` | 3 |
| `app_version` | 2 |
| `app_build` | 2 |
| `artifact_suffix` | 5 |
| `boost_context` | 15 |
| `probe_count` | 4 |
| `probe_index` | 8 |
| `probe_format` | 2 |
| `probe_base` | 2 |
| `probe_size` | 5 |
| `probe_data` | 8 |
| `probe_nonzero` | 3 |
| `probe_texels` | 5 |
| `probe_row` | 3 |
| `probe_edram_size` | 8 |
| `probe_dest_size` | 6 |
| `probe_buffer` | 8 |
| `probe_blit` | 5 |
| `probe_tile_size` | 3 |
| `probe_span_start` | 3 |
| `probe_span_size` | 3 |
| `probe_edram_nonzero` | 3 |
| `probe_dest_nonzero` | 3 |
| `probe_source` | 4 |
| `probed` | 7 |
| `msaa_probe_count` | 2 |
| `msaa_probe_rt` | 4 |
| `msaa_probe_source` | 7 |
| `msaa_probe_index` | 4 |
| `msaa_probe_desc` | 10 |
| `msaa_probe_texture` | 10 |
| `msaa_probe_pass` | 4 |
| `msaa_probe_attachment` | 5 |
| `msaa_probe_encoder` | 3 |
| `msaa_probe_key` | 2 |
| `msaa_probe_width` | 5 |
| `msaa_probe_height` | 4 |
| `msaa_probe_bytes` | 4 |
| `msaa_probe_nonzero` | 3 |
| `msaa_probe_byte` | 2 |
| `visibility_probe_buffer_` | 9 |
| `probe_buffer` | 4 |
| `probe_slot` | 3 |
| `probe_vertices` | 2 |
| `probe_samples` | 4 |

## Checks

- None of these names is defined anywhere else in this repository, so there are
  no conflicting or circular definitions. `probe_count`, `probe_index` and
  `probe_data` are each declared in two places (the texture cache probe and the
  resolve probe); they are locals of separate functions in separate files and
  do not refer to each other.
- `shared_drivers_mutex_`, `shared_drivers_`, `shared_lock`, `mix_frame` and
  `mixed` only exist inside `#if XE_PLATFORM_IOS` blocks.
- `artifact_suffix` is written to `$GITHUB_OUTPUT` in the `prepare_artifacts`
  step and read by the following upload step, which only runs when that step
  succeeded.
