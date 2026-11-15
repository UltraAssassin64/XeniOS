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
| `probe_count` | 2 |
| `probe_index` | 4 |
| `probe_format` | 2 |
| `probe_base` | 2 |
| `probe_size` | 5 |
| `probe_data` | 4 |
| `probe_nonzero` | 3 |
| `probe_texels` | 5 |
| `probe_row` | 3 |

## Checks

- None of these names is defined anywhere else in this repository, so there are
  no conflicting or circular definitions.
- `shared_drivers_mutex_`, `shared_drivers_`, `shared_lock`, `mix_frame` and
  `mixed` only exist inside `#if XE_PLATFORM_IOS` blocks.
- `artifact_suffix` is written to `$GITHUB_OUTPUT` in the `prepare_artifacts`
  step and read by the following upload step, which only runs when that step
  succeeded.
