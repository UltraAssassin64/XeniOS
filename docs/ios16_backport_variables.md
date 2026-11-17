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
| `DrawStats` | struct in MetalCommandProcessor | Counters describing what happened to guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 542, 555 | 2 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.h` | 555 | 1 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2529, 2536, 2537, 2538, 2539, 2540, 2541, 2542, 2543, 2807, 2809, 2845, 2870, 4235, 4259, 4294, 5546, 5547, 5553, 5554, 5566, 5640, 5641, 5645, 5654, 5673, 6009 | 36 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.h` | 539, 541 | 2 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2529 | 1 |
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
| `probed` | field of `DrawStats` | Number of scene draws logged by the scene draw probe in the current statistics window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5547, 5554, 5566, 5645, 5654, 5673 | 6 |
| `nan_constants` | field of `DrawStats` | Number of NaN bit patterns the game wrote to vertex float constants in the current statistics window. | `src/xenia/gpu/metal/metal_command_processor.h` | 553 | 1 |
| `nan_constants` | field of `DrawStats` | Number of NaN bit patterns the game wrote to vertex float constants in the current statistics window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2542, 6009 | 2 |
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
| `visibility_probe_buffer_` | member of MetalCommandProcessor | Buffer Metal writes the passed-sample counts of probed scene draws into; attached to every render pass. | `src/xenia/gpu/metal/metal_command_processor.h` | 557 | 1 |
| `visibility_probe_buffer_` | member of MetalCommandProcessor | Buffer Metal writes the passed-sample counts of probed scene draws into; attached to every render pass. | `src/xenia/gpu/metal/metal_command_processor.cc` | 586, 587, 588, 5546, 5553, 5652, 6318, 6319, 6322 | 9 |
| `probe_buffer` | local, scene pixel probe in IssueDrawMsl | Copy of `visibility_probe_buffer_` kept alive for the completion handler. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5652, 5653, 5660, 5670 | 4 |
| `probe_slot` | local, scene pixel probe in IssueDrawMsl | Index of the 8-byte counter this draw used. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5654, 5661, 5667 | 3 |
| `probe_vertices` | local, scene pixel probe in IssueDrawMsl | Vertex count of the probed draw, to pair the line with the scene draw probe line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5655, 5667 | 2 |
| `probe_samples` | local, scene pixel probe in IssueDrawMsl | Samples the draw passed in each of its six probe passes, read back from the buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5658, 5659, 5662, 5667, 5668, 5669 | 9 |
| `probe_pass` | local, scene pixel probe in IssueDrawMsl | Loop counter over the encodings of a probed draw: 0 as issued, 1 depth and stencil tests off, 2 also culling off, 3 also depth clipping off, 4 vertices drawn as lines, 5 as points. Unprobed draws run the loop once. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5545, 5546, 5552, 5557, 5559, 5561, 5566, 5572, 5573, 5632, 5633 | 11 |
| `probe_memory` | local, scene data probe in IssueDrawMsl | CPU pointer to the contents of the shared memory Metal buffer, the memory the vertex shader fetches from. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5712, 5723, 5739, 5742, 5747, 5750, 5790, 5802 | 8 |
| `probe_text` | local, scene data probe in IssueDrawMsl | Text of the index bytes and vertex streams appended to the scene data probe line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5714, 5719, 5721, 5753, 5762, 5770, 5781, 5800, 5806, 5819, 5840 | 11 |
| `probe_byte` | local, scene data probe in IssueDrawMsl | Loop counter over the bytes dumped from the index data and from each vertex stream. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5720, 5724, 5730, 5731, 5733, 5735, 5740, 5743, 5748, 5751, 5794, 5795, 5796, 5797, 5799, 5802 | 18 |
| `probe_binding` | local, scene data probe in IssueDrawMsl | Vertex binding of the probed draw's vertex shader being described. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5777, 5780, 5784, 5786, 5805 | 5 |
| `probe_fetch` | local, scene data probe in IssueDrawMsl | Vertex fetch constant of `probe_binding` (address, size, endianness). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5779, 5784, 5785, 5787, 5790, 5791, 5796, 5797, 5802 | 10 |
| `probe_attribute` | local, scene data probe in IssueDrawMsl | Attribute of `probe_binding` being described (format, offset). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5804, 5808, 5809, 5810, 5811 | 5 |
| `probe_constant` | local, scene data probe in IssueDrawMsl | Loop counter over vertex float constants c0 to c63; those the shader uses are logged as raw bits. Replaces `probe_constants`. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5816, 5818, 5820, 5821, 5822, 5823, 5824 | 9 |
| `probe_index_min` | local, scene data probe in IssueDrawMsl | Smallest index of the probed draw, read from the shared memory buffer as big-endian 16-bit. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5728, 5736, 5737, 5754 | 4 |
| `probe_index_max` | local, scene data probe in IssueDrawMsl | Largest index of the probed draw, read the same way. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5729, 5744, 5745, 5754 | 4 |
| `probe_target` | local, scene data probe in IssueDrawMsl | Colour or depth texture of the current render pass, to log its size, sample count and format (declared twice, in two adjacent `if` scopes). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5758, 5763, 5764, 5765, 5766, 5768, 5771, 5772, 5773, 5774 | 10 |
| `touch` | namespace `xe::hid::touch` | Holds the on-screen controller driver and its state. | `src/xenia/hid/touch_input_driver_ios.h` | 21, 37 | 2 |
| `touch` | namespace `xe::hid::touch` | Holds the on-screen controller driver and its state. | `src/xenia/hid/touch_input_driver_ios.cc` | 20, 185 | 2 |
| `SetTouchState` | function | Stores the on-screen controller state; called by the UI, read by the driver. | `src/xenia/hid/touch_input_driver_ios.h` | 27 | 1 |
| `SetTouchState` | function | Stores the on-screen controller state; called by the UI, read by the driver. | `src/xenia/hid/touch_input_driver_ios.cc` | 159 | 1 |
| `SetTouchState` | function | Stores the on-screen controller state; called by the UI, read by the driver. | `src/xenia/ui/windowed_app_main_ios.mm` | 8201, 8357 | 2 |
| `TouchInputDriver` | class (anonymous namespace) | Input driver presenting the on-screen controller as a fallback gamepad. | `src/xenia/hid/touch_input_driver_ios.cc` | 29, 31, 182 | 3 |
| `touch_state_mutex_` | file-scope variable | Guards `touch_state_`. | `src/xenia/hid/touch_input_driver_ios.cc` | 24, 25, 66, 107, 162 | 5 |
| `touch_state_` | file-scope variable | Latest on-screen controller state and packet number. | `src/xenia/hid/touch_input_driver_ios.cc` | 27, 67, 108, 109, 113, 163, 177 | 8 |
| `kKeystrokeKeys` | local constant in GetKeystroke | Virtual key for each button bit. | `src/xenia/hid/touch_input_driver_ios.cc` | 85, 120, 123, 129, 153 | 5 |
| `keystroke_buttons` | local in GetKeystroke | Current buttons and triggers as a bit field. | `src/xenia/hid/touch_input_driver_ios.cc` | 105, 108, 111, 115, 122, 124 | 6 |
| `release_pass` | local in GetKeystroke | Loop counter: releases are reported before presses. | `src/xenia/hid/touch_input_driver_ios.cc` | 119, 125 | 3 |
| `keystroke_bit` | local in GetKeystroke | Bit of the button being examined. | `src/xenia/hid/touch_input_driver_ios.cc` | 121, 122, 124, 128 | 4 |
| `pressed` | local in GetKeystroke | Whether that button is currently down. | `src/xenia/hid/touch_input_driver_ios.cc` | 122, 125, 132 | 3 |
| `last_keystroke_buttons_` | member of TouchInputDriver | Buttons already reported through GetKeystroke. | `src/xenia/hid/touch_input_driver_ios.cc` | 124, 128, 154 | 3 |
| `XeniaTouchPadView` | Objective-C class | The on-screen controller view. | `src/xenia/ui/windowed_app_main_ios.mm` | 8199, 8221, 8228, 8468, 8567 | 5 |
| `XeniaTouchPadControl` | struct | One control of the on-screen controller: kind, button bit, label, centre, radius. | `src/xenia/ui/windowed_app_main_ios.mm` | 8211, 8229, 8262, 8407 | 4 |
| `XeniaTouchPadControlKind` | enum | Kind of control. | `src/xenia/ui/windowed_app_main_ios.mm` | 8203, 8212 | 2 |
| `XeniaTouchPadControlKindButton` | enumerator | A digital button. | `src/xenia/ui/windowed_app_main_ios.mm` | 8204, 8267, 8269, 8271, 8273, 8274, 8276, 8278, 8280, 8282, 8284, 8286, 8288, 8290, 8292, 8294 | 16 |
| `XeniaTouchPadControlKindLeftTrigger` | enumerator | Left trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8205, 8265, 8349 | 3 |
| `XeniaTouchPadControlKindRightTrigger` | enumerator | Right trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8206, 8266, 8351 | 3 |
| `XeniaTouchPadControlKindLeftStick` | enumerator | Left thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8207, 8263, 8328, 8339, 8412, 8440 | 6 |
| `XeniaTouchPadControlKindRightStick` | enumerator | Right thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8208, 8264, 8329, 8413, 8441 | 5 |
| `kXeniaTouchPadControlCount` | file-scope constant | Number of controls (19). | `src/xenia/ui/windowed_app_main_ios.mm` | 8219, 8229, 8262, 8297, 8304, 8406 | 6 |
| `controls_` | instance variable of XeniaTouchPadView | The controls with their current positions. | `src/xenia/ui/windowed_app_main_ios.mm` | 8229, 8298, 8305, 8306, 8308, 8328, 8329, 8331, 8332, 8339, 8349, 8351, 8354, 8407 | 16 |
| `pad_touches_` | instance variable of XeniaTouchPadView | Touches currently on the pad. | `src/xenia/ui/windowed_app_main_ios.mm` | 8231, 8242, 8249, 8325, 8326, 8363, 8366, 8375, 8388, 8390, 8410, 8411 | 12 |
| `pad_touch_controls_` | instance variable of XeniaTouchPadView | Control each of those touches started on. | `src/xenia/ui/windowed_app_main_ios.mm` | 8232, 8243, 8250, 8327, 8367, 8376, 8391, 8414 | 8 |
| `controlIndexAtPoint` | method of XeniaTouchPadView | Index of the control at a point, or -1. | `src/xenia/ui/windowed_app_main_ios.mm` | 8223, 8303, 8318, 8345, 8373, 8415, 9977 | 7 |
| `resetTouches` | method of XeniaTouchPadView | Releases every control and reports the idle state. | `src/xenia/ui/windowed_app_main_ios.mm` | 8225, 8362, 9365 | 3 |
| `reportState` | method of XeniaTouchPadView | Computes the gamepad state from the touches and passes it to SetTouchState. | `src/xenia/ui/windowed_app_main_ios.mm` | 8321, 8368, 8379, 8383, 8394 | 5 |
| `touchPadView` | property of XeniaViewController | The on-screen controller view instance. | `src/xenia/ui/windowed_app_main_ios.mm` | 8468, 8567, 8568, 8570, 8571, 9362, 9363, 9365, 9976, 9977 | 11 |
| `hide_touch_pad` | local in pollControllerNavigation | Whether the on-screen controller should be hidden now. | `src/xenia/ui/windowed_app_main_ios.mm` | 9359, 9362, 9363, 9364 | 4 |
| `width` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8256, 8260, 8261, 8452 | 5 |
| `height` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8257, 8263, 8264, 8277, 8279, 8281, 8283, 8285, 8287, 8289, 8291, 8293, 8295, 8453 | 15 |
| `insets` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8258, 8259, 8260 | 3 |
| `left` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8259, 8263, 8265, 8268, 8277, 8279, 8281, 8283, 8293 | 10 |
| `right` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8260, 8264, 8266, 8270, 8285, 8287, 8289, 8291, 8295 | 10 |
| `middle` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8261, 8272, 8273, 8275 | 4 |
| `controls` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8262, 8298, 8316 | 3 |
| `reach` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8308, 8309 | 3 |
| `dx` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8305, 8309 | 3 |
| `dy` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8306, 8309 | 3 |
| `buttons` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8213, 8322, 8354, 8357 | 4 |
| `triggers` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8323, 8344, 8350, 8352, 8357 | 6 |
| `axes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8324, 8340, 8341, 8357, 8358 | 7 |
| `point` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8222, 8223, 8303, 8305, 8306, 8317, 8318, 8326, 8331, 8332, 8345, 8411, 8415, 8418, 8419 | 15 |
| `index` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8327, 8328, 8329, 8331, 8332, 8339, 8345, 8346, 8349, 8351, 8354, 8373, 8374, 8376 | 16 |
| `x` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8305, 8331, 8333, 8335, 8340, 8418, 8420, 8422, 8425, 8452 | 18 |
| `y` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8306, 8332, 8333, 8336, 8341, 8419, 8420, 8423, 8425, 8453 | 18 |
| `length` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8333, 8334, 8335, 8336, 8420, 8421, 8422, 8423 | 8 |
| `axis` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8338, 8340, 8341 | 3 |
| `position` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8388, 8389, 8390, 8391 | 4 |
| `attributes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8402, 8451, 8454 | 3 |
| `control` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8222, 8224, 8230, 8307, 8407, 8409, 8412, 8413, 8418, 8419, 8421, 8422, 8423, 8425, 8430, 8431, 8440, 8441, 8444, 8451, 8452 | 22 |
| `active` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8408, 8416, 8435 | 4 |
| `knob` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8409, 8425, 8443, 8452, 8453 | 5 |
| `is_stick` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8412, 8414, 8417 | 3 |
| `outline` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8429, 8436, 8438, 8439 | 4 |
| `knob_path` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8442, 8449 | 2 |
| `label_size` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8451, 8452, 8453 | 3 |
| `ppc-test-binaries` | job id and artifact name, CPU tests workflow | Assembles the PowerPC instruction tests on Linux and hands the binaries to the macOS job. | `.github/workflows/cpu-tests.yml` | 18, 54, 63, 204 | 4 |
| `cpu-tests-macos-arm64` | job id, CPU tests workflow | Builds and runs the CPU test programs on Apple Silicon. | `.github/workflows/cpu-tests.yml` | 61 | 1 |
| `cache-binutils` | step id, CPU tests workflow | Cache of the patched PowerPC binutils build. | `.github/workflows/cpu-tests.yml` | 25, 34 | 2 |
| `cache-build-cpu-tests` | step id, CPU tests workflow | Cache of the Xcode build output of the test programs. | `.github/workflows/cpu-tests.yml` | 78 | 1 |
| `cache-submodules-cpu-tests` | step id, CPU tests workflow | Cache of the submodules the desktop test build needs. | `.github/workflows/cpu-tests.yml` | 96 | 1 |
| `build_hir_tests` | step id, CPU tests workflow | Build of xenia-cpu-tests; its outcome decides the final step. | `.github/workflows/cpu-tests.yml` | 176, 226 | 2 |
| `build_ppc_tests` | step id, CPU tests workflow | Build of xenia-cpu-ppc-tests; its outcome decides the final step. | `.github/workflows/cpu-tests.yml` | 189, 226 | 2 |
| `bin_count` | shell variable, CPU tests workflow | Number of assembled test binaries, checked against a minimum. | `.github/workflows/cpu-tests.yml` | 44, 45, 46 | 3 |
| `run` | function, CPU test runner script | Runs one command with a timeout and classifies it as pass, fail, crash or timeout. | `tools/ci/run_cpu_tests.py` | 6, 20, 23, 50, 56, 64 | 6 |
| `tail` | function, CPU test runner script | Last non-empty lines of a test's output, for the report. | `tools/ci/run_cpu_tests.py` | 39, 58, 77 | 3 |
| `escape_catch_name` | function, CPU test runner script | Escapes a test name so Catch matches it exactly. | `tools/ci/run_cpu_tests.py` | 44, 56 | 2 |
| `run_hir_tests` | function, CPU test runner script | Lists the HIR test cases and runs each in its own process. | `tools/ci/run_cpu_tests.py` | 49, 157 | 2 |
| `run_ppc_suite` | function, CPU test runner script | Runs one instruction test file and extracts its pass and fail counts. | `tools/ci/run_cpu_tests.py` | 63, 87 | 2 |
| `run_ppc_tests` | function, CPU test runner script | Runs every instruction test file, several at a time. | `tools/ci/run_cpu_tests.py` | 81, 159 | 2 |
| `write_report` | function, CPU test runner script | Writes report.md from the collected results. | `tools/ci/run_cpu_tests.py` | 95, 164 | 2 |
| `main` | function, CPU test runner script | Parses arguments, runs both test programs, writes the results. | `tools/ci/run_cpu_tests.py` | 142, 178 | 2 |

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
| `draw_stats_` | 37 |
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
| `nan_constants` | 3 |
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
| `visibility_probe_buffer_` | 10 |
| `probe_buffer` | 4 |
| `probe_slot` | 3 |
| `probe_vertices` | 2 |
| `probe_samples` | 9 |
| `probe_pass` | 11 |
| `probe_memory` | 8 |
| `probe_text` | 11 |
| `probe_byte` | 18 |
| `probe_binding` | 5 |
| `probe_fetch` | 10 |
| `probe_attribute` | 5 |
| `probe_constant` | 9 |
| `probe_index_min` | 4 |
| `probe_index_max` | 4 |
| `probe_target` | 10 |
| `touch` | 4 |
| `SetTouchState` | 4 |
| `TouchInputDriver` | 3 |
| `touch_state_mutex_` | 5 |
| `touch_state_` | 8 |
| `kKeystrokeKeys` | 5 |
| `keystroke_buttons` | 6 |
| `release_pass` | 3 |
| `keystroke_bit` | 4 |
| `pressed` | 3 |
| `last_keystroke_buttons_` | 3 |
| `XeniaTouchPadView` | 5 |
| `XeniaTouchPadControl` | 4 |
| `XeniaTouchPadControlKind` | 2 |
| `XeniaTouchPadControlKindButton` | 16 |
| `XeniaTouchPadControlKindLeftTrigger` | 3 |
| `XeniaTouchPadControlKindRightTrigger` | 3 |
| `XeniaTouchPadControlKindLeftStick` | 6 |
| `XeniaTouchPadControlKindRightStick` | 5 |
| `kXeniaTouchPadControlCount` | 6 |
| `controls_` | 16 |
| `pad_touches_` | 12 |
| `pad_touch_controls_` | 8 |
| `controlIndexAtPoint` | 7 |
| `resetTouches` | 3 |
| `reportState` | 5 |
| `touchPadView` | 11 |
| `hide_touch_pad` | 4 |
| `width` (XeniaTouchPadView locals) | 5 |
| `height` (XeniaTouchPadView locals) | 15 |
| `insets` (XeniaTouchPadView locals) | 3 |
| `left` (XeniaTouchPadView locals) | 10 |
| `right` (XeniaTouchPadView locals) | 10 |
| `middle` (XeniaTouchPadView locals) | 4 |
| `controls` (XeniaTouchPadView locals) | 3 |
| `reach` (XeniaTouchPadView locals) | 3 |
| `dx` (XeniaTouchPadView locals) | 3 |
| `dy` (XeniaTouchPadView locals) | 3 |
| `buttons` (XeniaTouchPadView locals) | 4 |
| `triggers` (XeniaTouchPadView locals) | 6 |
| `axes` (XeniaTouchPadView locals) | 7 |
| `point` (XeniaTouchPadView locals) | 15 |
| `index` (XeniaTouchPadView locals) | 16 |
| `x` (XeniaTouchPadView locals) | 18 |
| `y` (XeniaTouchPadView locals) | 18 |
| `length` (XeniaTouchPadView locals) | 8 |
| `axis` (XeniaTouchPadView locals) | 3 |
| `position` (XeniaTouchPadView locals) | 4 |
| `attributes` (XeniaTouchPadView locals) | 3 |
| `control` (XeniaTouchPadView locals) | 22 |
| `active` (XeniaTouchPadView locals) | 4 |
| `knob` (XeniaTouchPadView locals) | 5 |
| `is_stick` (XeniaTouchPadView locals) | 3 |
| `outline` (XeniaTouchPadView locals) | 4 |
| `knob_path` (XeniaTouchPadView locals) | 2 |
| `label_size` (XeniaTouchPadView locals) | 3 |
| `ppc-test-binaries` | 4 |
| `cpu-tests-macos-arm64` | 1 |
| `cache-binutils` | 2 |
| `cache-build-cpu-tests` | 1 |
| `cache-submodules-cpu-tests` | 1 |
| `build_hir_tests` | 2 |
| `build_ppc_tests` | 2 |
| `bin_count` | 3 |
| `run` (run_cpu_tests.py) | 6 |
| `tail` (run_cpu_tests.py) | 3 |
| `escape_catch_name` | 2 |
| `run_hir_tests` | 2 |
| `run_ppc_suite` | 2 |
| `run_ppc_tests` | 2 |
| `write_report` | 2 |
| `main` (run_cpu_tests.py) | 2 |

## Checks

- None of these names is defined anywhere else in this repository, so there are
  no conflicting or circular definitions. `probe_count`, `probe_index` and
  `probe_data` are each declared in two places (the texture cache probe and the
  resolve probe); they are locals of separate functions in separate files and
  do not refer to each other.
- The `XeniaTouchPadView` locals (`width`, `x`, `index` and so on) are ordinary
  short names that also occur elsewhere in the same file; their line lists and
  counts cover only the `XeniaTouchPadView` block.
- `touch_input_driver_ios.{h,cc}` are only compiled for iOS (the build's
  `_ios` file filter), and `xe::hid::touch::SetTouchState` is called only from
  the iOS UI file.
- `shared_drivers_mutex_`, `shared_drivers_`, `shared_lock`, `mix_frame` and
  `mixed` only exist inside `#if XE_PLATFORM_IOS` blocks.
- `artifact_suffix` is written to `$GITHUB_OUTPUT` in the `prepare_artifacts`
  step and read by the following upload step, which only runs when that step
  succeeded.
- The CPU test sources added under `src/xenia/cpu/testing` (and the
  `skip_cf_simplification` parameter of `TestModule`, `CreateBackend` in
  `util.h`, `test_backend_cvars.cc`) are copied from upstream
  `release-v2.0.1-build-9730`, so their identifiers are not listed here.
- The names in `tools/ci/run_cpu_tests.py` are local to that script; `run`,
  `tail` and `main` are common names elsewhere in the tree but are not
  shared with any other file.
