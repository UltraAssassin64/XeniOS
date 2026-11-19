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
| `session_log_path` | function-local static in GetLogFilePath | The log file chosen for this launch; read by the logger setup and the in-app log viewer. | `src/xenia/ui/windowed_app_main_ios.mm` | 1259, 1272 | 2 |
| `now` | local in GetLogFilePath | Current time used to name the log file. | `src/xenia/ui/windowed_app_main_ios.mm` | 1264, 1266 | 2 |
| `local_time` | local in GetLogFilePath | now broken down in the device's time zone. | `src/xenia/ui/windowed_app_main_ios.mm` | 1265, 1266, 1269 | 3 |
| `log_name` | local in GetLogFilePath | File name xenia_<date>_<time>.log. | `src/xenia/ui/windowed_app_main_ios.mm` | 1267, 1268, 1270 | 4 |
| `kText` | enumerator of IOSConfigControlType | Settings row whose value is edited as text. | `src/xenia/ui/windowed_app_main_ios.mm` | 543, 1230, 3886, 8295, 8348 | 5 |
| `AddAllConfigVarSettings` | function | Adds a settings section per config category with every variable not already shown. | `src/xenia/ui/windowed_app_main_ios.mm` | 1204, 3802 | 2 |
| `existing_keys` | local in AddAllConfigVarSettings | Config keys the curated sections already expose, so they are not listed twice. | `src/xenia/ui/windowed_app_main_ios.mm` | 1208, 1212, 1218 | 3 |
| `by_category` | local in AddAllConfigVarSettings | Generated sections keyed by config category. | `src/xenia/ui/windowed_app_main_ios.mm` | 1216, 1234, 1238 | 3 |
| `parsed` | local in ApplyIOSConfigSections | The edited text parsed as a TOML value. | `src/xenia/ui/windowed_app_main_ios.mm` | 3900, 3902, 3905, 3912 | 4 |
| `editor` | local in tableView:didSelectRowAtIndexPath: | Alert with a text field used to edit a kText row. | `src/xenia/ui/windowed_app_main_ios.mm` | 8352, 8356, 8362, 8365, 8368, 8378 | 6 |
| `DrawStats` | struct in MetalCommandProcessor | Counters describing what happened to guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 644, 659, 669 | 3 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.h` | 659, 661 | 2 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2773, 2778, 2785, 2786, 2787, 2788, 2789, 2790, 2841, 2849, 2850, 2851, 2852, 2853, 2856, 2863, 2864, 2865, 2866, 2867, 2868, 2869, 2886, 2897, 2898, 2899, 2900, 2962, 2969, 2989, 2999, 3000, 3002, 3275, 3277, 3313, 3338, 4752, 4783, 4831, 6383, 6384, 7059, 7072 | 53 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.h` | 641, 643 | 2 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2856 | 1 |
| `app_version` | shell variable, CI Prepare artifacts step | CFBundleShortVersionString of the built app. | `.github/workflows/CI.yml` | 662, 664 | 2 |
| `app_build` | shell variable, CI Prepare artifacts step | CFBundleVersion of the built app. | `.github/workflows/CI.yml` | 663, 664 | 2 |
| `artifact_suffix` | shell variable and step output, CI | <version>-<build>, appended to the IPA and artifact names. | `.github/workflows/CI.yml` | 664, 665, 668, 682 | 5 |
| `boost_context` | premake project name | Builds the fcontext assembly used by threading_fiber.cc. Same name as upstream's CMake target. | `third_party/boost_context.lua` | 2, 10, 11, 12, 16, 17, 18, 22, 23, 24, 28, 29, 30 | 13 |
| `boost_context` | premake project name | Builds the fcontext assembly used by threading_fiber.cc. Same name as upstream's CMake target. | `premake5.lua` | 762 | 1 |
| `boost_context` | premake project name | Builds the fcontext assembly used by threading_fiber.cc. Same name as upstream's CMake target. | `src/xenia/app/premake5.lua` | 39 | 1 |
| `touch` | namespace `xe::hid::touch` | Holds the on-screen controller driver and its state. | `src/xenia/hid/touch_input_driver_ios.h` | 21, 37 | 2 |
| `touch` | namespace `xe::hid::touch` | Holds the on-screen controller driver and its state. | `src/xenia/hid/touch_input_driver_ios.cc` | 20, 185 | 2 |
| `SetTouchState` | function | Stores the on-screen controller state; called by the UI, read by the driver. | `src/xenia/hid/touch_input_driver_ios.h` | 27 | 1 |
| `SetTouchState` | function | Stores the on-screen controller state; called by the UI, read by the driver. | `src/xenia/hid/touch_input_driver_ios.cc` | 159 | 1 |
| `SetTouchState` | function | Stores the on-screen controller state; called by the UI, read by the driver. | `src/xenia/ui/windowed_app_main_ios.mm` | 8457, 8613 | 2 |
| `TouchInputDriver` | class (anonymous namespace) | Input driver presenting the on-screen controller as a fallback gamepad. | `src/xenia/hid/touch_input_driver_ios.cc` | 29, 31, 182 | 3 |
| `touch_state_mutex_` | file-scope variable | Guards `touch_state_`. | `src/xenia/hid/touch_input_driver_ios.cc` | 24, 25, 66, 107, 162 | 5 |
| `touch_state_` | file-scope variable | Latest on-screen controller state and packet number. | `src/xenia/hid/touch_input_driver_ios.cc` | 27, 67, 108, 109, 113, 163, 177 | 8 |
| `kKeystrokeKeys` | local constant in GetKeystroke | Virtual key for each button bit. | `src/xenia/hid/touch_input_driver_ios.cc` | 85, 120, 123, 129, 153 | 5 |
| `keystroke_buttons` | local in GetKeystroke | Current buttons and triggers as a bit field. | `src/xenia/hid/touch_input_driver_ios.cc` | 105, 108, 111, 115, 122, 124 | 6 |
| `release_pass` | local in GetKeystroke | Loop counter: releases are reported before presses. | `src/xenia/hid/touch_input_driver_ios.cc` | 119, 125 | 3 |
| `keystroke_bit` | local in GetKeystroke | Bit of the button being examined. | `src/xenia/hid/touch_input_driver_ios.cc` | 121, 122, 124, 128 | 4 |
| `pressed` | local in GetKeystroke | Whether that button is currently down. | `src/xenia/hid/touch_input_driver_ios.cc` | 122, 125, 132 | 3 |
| `last_keystroke_buttons_` | member of TouchInputDriver | Buttons already reported through GetKeystroke. | `src/xenia/hid/touch_input_driver_ios.cc` | 124, 128, 154 | 3 |
| `XeniaTouchPadView` | Objective-C class | The on-screen controller view. | `src/xenia/ui/windowed_app_main_ios.mm` | 8455, 8477, 8484, 8724, 8823 | 5 |
| `XeniaTouchPadControl` | struct | One control of the on-screen controller: kind, button bit, label, centre, radius. | `src/xenia/ui/windowed_app_main_ios.mm` | 8467, 8485, 8518, 8663 | 4 |
| `XeniaTouchPadControlKind` | enum | Kind of control. | `src/xenia/ui/windowed_app_main_ios.mm` | 8459, 8468 | 2 |
| `XeniaTouchPadControlKindButton` | enumerator | A digital button. | `src/xenia/ui/windowed_app_main_ios.mm` | 8460, 8523, 8525, 8527, 8529, 8530, 8532, 8534, 8536, 8538, 8540, 8542, 8544, 8546, 8548, 8550 | 16 |
| `XeniaTouchPadControlKindLeftTrigger` | enumerator | Left trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8461, 8521, 8605 | 3 |
| `XeniaTouchPadControlKindRightTrigger` | enumerator | Right trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8462, 8522, 8607 | 3 |
| `XeniaTouchPadControlKindLeftStick` | enumerator | Left thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8463, 8519, 8584, 8595, 8668, 8696 | 6 |
| `XeniaTouchPadControlKindRightStick` | enumerator | Right thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8464, 8520, 8585, 8669, 8697 | 5 |
| `kXeniaTouchPadControlCount` | file-scope constant | Number of controls (19). | `src/xenia/ui/windowed_app_main_ios.mm` | 8475, 8485, 8518, 8553, 8560, 8662 | 6 |
| `controls_` | instance variable of XeniaTouchPadView | The controls with their current positions. | `src/xenia/ui/windowed_app_main_ios.mm` | 8485, 8554, 8561, 8562, 8564, 8584, 8585, 8587, 8588, 8595, 8605, 8607, 8610, 8663 | 16 |
| `pad_touches_` | instance variable of XeniaTouchPadView | Touches currently on the pad. | `src/xenia/ui/windowed_app_main_ios.mm` | 8487, 8498, 8505, 8581, 8582, 8619, 8622, 8631, 8644, 8646, 8666, 8667 | 12 |
| `pad_touch_controls_` | instance variable of XeniaTouchPadView | Control each of those touches started on. | `src/xenia/ui/windowed_app_main_ios.mm` | 8488, 8499, 8506, 8583, 8623, 8632, 8647, 8670 | 8 |
| `controlIndexAtPoint` | method of XeniaTouchPadView | Index of the control at a point, or -1. | `src/xenia/ui/windowed_app_main_ios.mm` | 8479, 8559, 8574, 8601, 8629, 8671, 10233 | 7 |
| `resetTouches` | method of XeniaTouchPadView | Releases every control and reports the idle state. | `src/xenia/ui/windowed_app_main_ios.mm` | 8481, 8618, 9621 | 3 |
| `reportState` | method of XeniaTouchPadView | Computes the gamepad state from the touches and passes it to SetTouchState. | `src/xenia/ui/windowed_app_main_ios.mm` | 8577, 8624, 8635, 8639, 8650 | 5 |
| `touchPadView` | property of XeniaViewController | The on-screen controller view instance. | `src/xenia/ui/windowed_app_main_ios.mm` | 8724, 8823, 8824, 8826, 8827, 9618, 9619, 9621, 10232, 10233 | 11 |
| `hide_touch_pad` | local in pollControllerNavigation | Whether the on-screen controller should be hidden now. | `src/xenia/ui/windowed_app_main_ios.mm` | 9615, 9618, 9619, 9620 | 4 |
| `width` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8512, 8516, 8517, 8708 | 5 |
| `height` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8513, 8519, 8520, 8533, 8535, 8537, 8539, 8541, 8543, 8545, 8547, 8549, 8551, 8709 | 15 |
| `insets` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8514, 8515, 8516 | 3 |
| `left` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8515, 8519, 8521, 8524, 8533, 8535, 8537, 8539, 8549 | 10 |
| `right` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8516, 8520, 8522, 8526, 8541, 8543, 8545, 8547, 8551 | 10 |
| `middle` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8517, 8528, 8529, 8531 | 4 |
| `controls` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8518, 8554, 8572 | 3 |
| `reach` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8564, 8565 | 3 |
| `dx` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8561, 8565 | 3 |
| `dy` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8562, 8565 | 3 |
| `buttons` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8469, 8578, 8610, 8613 | 4 |
| `triggers` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8579, 8600, 8606, 8608, 8613 | 6 |
| `axes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8580, 8596, 8597, 8613, 8614 | 7 |
| `point` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8478, 8479, 8559, 8561, 8562, 8573, 8574, 8582, 8587, 8588, 8601, 8667, 8671, 8674, 8675 | 15 |
| `index` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8583, 8584, 8585, 8587, 8588, 8595, 8601, 8602, 8605, 8607, 8610, 8629, 8630, 8632 | 16 |
| `x` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8561, 8587, 8589, 8591, 8596, 8674, 8676, 8678, 8681, 8708 | 18 |
| `y` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8562, 8588, 8589, 8592, 8597, 8675, 8676, 8679, 8681, 8709 | 18 |
| `length` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8589, 8590, 8591, 8592, 8676, 8677, 8678, 8679 | 8 |
| `axis` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8594, 8596, 8597 | 3 |
| `position` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8644, 8645, 8646, 8647 | 4 |
| `attributes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8658, 8707, 8710 | 3 |
| `control` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8478, 8480, 8486, 8563, 8663, 8665, 8668, 8669, 8674, 8675, 8677, 8678, 8679, 8681, 8686, 8687, 8696, 8697, 8700, 8707, 8708 | 22 |
| `active` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8664, 8672, 8691 | 4 |
| `knob` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8665, 8681, 8699, 8708, 8709 | 5 |
| `is_stick` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8668, 8670, 8673 | 3 |
| `outline` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8685, 8692, 8694, 8695 | 4 |
| `knob_path` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8698, 8705 | 2 |
| `label_size` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8707, 8708, 8709 | 3 |
| `ppc-test-binaries` | job id and artifact name, CPU tests workflow | Assembles the PowerPC instruction tests on Linux and hands the binaries to the macOS job. | `.github/workflows/cpu-tests.yml` | 19, 55, 64, 128 | 4 |
| `cpu-tests-macos-arm64` | job id, CPU tests workflow | Builds and runs the CPU test programs on Apple Silicon. | `.github/workflows/cpu-tests.yml` | 62 | 1 |
| `cache-binutils` | step id, CPU tests workflow | Cache of the patched PowerPC binutils build. | `.github/workflows/cpu-tests.yml` | 26, 35 | 2 |
| `cache-submodules-cpu-tests` | step id, CPU tests workflow | Cache of the submodules the CPU test build needs. | `.github/workflows/cpu-tests.yml` | 72 | 1 |
| `bin_count` | shell variable, CPU tests workflow | Number of assembled test binaries, checked against a minimum. | `.github/workflows/cpu-tests.yml` | 45, 46, 47 | 3 |
| `build_cpu_tests` | step id, CPU tests workflow | Build of both CPU test programs with tools/ci/build_cpu_tests.py; its outcome decides the final step. | `.github/workflows/cpu-tests.yml` | 119, 148 | 2 |
| `run` | function, CPU test runner script | Runs one command with a timeout and classifies it as pass, fail, crash or timeout. | `tools/ci/run_cpu_tests.py` | 6, 20, 23, 50, 56, 64, 87, 156, 161, 170 | 10 |
| `tail` | function, CPU test runner script | Last non-empty lines of a test's output, for the report. | `tools/ci/run_cpu_tests.py` | 39, 58, 82 | 3 |
| `escape_catch_name` | function, CPU test runner script | Escapes a test name so Catch matches it exactly. | `tools/ci/run_cpu_tests.py` | 44, 56 | 2 |
| `run_hir_tests` | function, CPU test runner script | Lists the HIR test cases and runs each in its own process. | `tools/ci/run_cpu_tests.py` | 49, 198 | 2 |
| `run_ppc_suite` | function, CPU test runner script | Runs one instruction test file, and twice more if any case failed, to tell cases that fail every time from those that fail now and then. | `tools/ci/run_cpu_tests.py` | 86, 118 | 2 |
| `run_ppc_suite_once` | function, CPU test runner script | Runs one instruction test file once and extracts its pass and fail counts and the reason printed for each failed case. | `tools/ci/run_cpu_tests.py` | 63, 89, 98 | 3 |
| `run_ppc_tests` | function, CPU test runner script | Runs every instruction test file, several at a time. | `tools/ci/run_cpu_tests.py` | 112, 200 | 2 |
| `write_report` | function, CPU test runner script | Writes report.md from the collected results. | `tools/ci/run_cpu_tests.py` | 126, 205 | 2 |
| `main` | function, CPU test runner script | Parses arguments, runs both test programs, writes the results. | `tools/ci/run_cpu_tests.py` | 183, 219 | 2 |
| `ROOT` | module constant, CPU test build script | Repository root, found from the script location. | `tools/ci/build_cpu_tests.py` | 20, 57, 58, 104, 139, 145, 152, 187, 204, 229 | 10 |
| `host` | function, CPU test build script | Host system (mac or linux) and backend architecture (a64 or x64). | `tools/ci/build_cpu_tests.py` | 6, 24, 32, 193 | 4 |
| `wanted` | function, CPU test build script | Whether a source file belongs in the build for this host (no tests, demos, mains or other platforms). | `tools/ci/build_cpu_tests.py` | 31, 62, 74 | 3 |
| `collect_sources` | function, CPU test build script | Lists the shared sources and those of each test program. | `tools/ci/build_cpu_tests.py` | 55, 195 | 2 |
| `rel` | nested function in collect_sources | Repository-relative paths matching a glob, sorted. | `tools/ci/build_cpu_tests.py` | 56, 61, 64, 65, 88, 90, 91, 93, 105 | 9 |
| `flags` | function, CPU test build script | Compiler flags: defines, include directories, optimization and host CPU features. | `tools/ci/build_cpu_tests.py` | 111, 196 | 2 |
| `ensure_version_header` | function, CPU test build script | Writes build/version.h, which the regular build generates. | `tools/ci/build_cpu_tests.py` | 143, 194 | 2 |
| `git` | nested function in ensure_version_header | Output of a git command in the repository, or "unknown". | `tools/ci/build_cpu_tests.py` | 149, 152, 157, 169 | 4 |
| `compile_one` | function, CPU test build script | Compiles one source unless its object is newer. | `tools/ci/build_cpu_tests.py` | 173, 226 | 2 |
| `main` | function, CPU test build script | Parses arguments, compiles everything, archives the shared objects and links both test programs. | `tools/ci/build_cpu_tests.py` | 184, 267 | 2 |
| `cpu_tests_duplicate_record_mmio_access_exceptions` | macro name given on the command line by the CPU test build script | Renames the copy of the record_mmio_access_exceptions cvar in cpu/backend/backend.cc, which memory.cc also defines on this branch. | `tools/ci/build_cpu_tests.py` | 217 | 1 |
| `cpu_tests_duplicate_emit_mmio_aware_stores` | macro name given on the command line by the CPU test build script | Renames the copy of the emit_mmio_aware_stores_for_recorded_exception_addresses cvar in cpu/backend/backend.cc. | `tools/ci/build_cpu_tests.py` | 219 | 1 |
| `mmio_repeat_address` | file-static atomic, MMIOHandler hang diagnostics | Guest address of the last MMIO store handled. | `src/xenia/cpu/mmio_handler.cc` | 37, 595 | 2 |
| `mmio_repeat_count` | file-static atomic, MMIOHandler hang diagnostics | How many MMIO stores in a row went to mmio_repeat_address. | `src/xenia/cpu/mmio_handler.cc` | 38, 597, 598, 606, 622 | 6 |
| `mmio_guest_context` | local in MMIOHandler::ExceptionCallback (ARM64) | Guest register context of the faulting thread, read through host register x20 for the repeated-store report. | `src/xenia/cpu/mmio_handler.cc` | 599, 610, 611, 612, 613, 614, 615, 616, 617, 618, 636, 653, 670, 675, 680 | 16 |
| `shm_attempt` | local in CreateFileMappingHandle (macOS only) | Attempt number when creating a shared memory object exclusively; attempts after the first add the process ID to the name. | `src/xenia/base/memory_posix.cc` | 486, 488, 490 | 5 |
| `mmio_thread_state` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest thread state of the faulting thread, to reach the processor. | `src/xenia/cpu/mmio_handler.cc` | 621, 622, 623, 624 | 4 |
| `mmio_function` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest function whose generated code holds the repeated store. | `src/xenia/cpu/mmio_handler.cc` | 624, 643, 644, 645, 646, 647, 649, 651 | 8 |
| `mmio_dump` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Lambda that logs guest code words, eight per line. | `src/xenia/cpu/mmio_handler.cc` | 628, 649, 653, 685 | 4 |
| `mmio_start` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest address the dump starts at (lambda parameter). | `src/xenia/cpu/mmio_handler.cc` | 628, 637, 639 | 3 |
| `mmio_words` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Number of guest words to dump (lambda parameter). | `src/xenia/cpu/mmio_handler.cc` | 628, 629, 632 | 3 |
| `mmio_line` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Index of the first word on the current dump line. | `src/xenia/cpu/mmio_handler.cc` | 629, 631, 632, 639, 656, 658, 665 | 12 |
| `mmio_text` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Text of one dump line. | `src/xenia/cpu/mmio_handler.cc` | 630, 634, 640, 657, 660, 665 | 6 |
| `mmio_word` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Index of the word being added to a dump line. | `src/xenia/cpu/mmio_handler.cc` | 631, 632, 633, 637, 658, 659, 662 | 9 |
| `mmio_end` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | End address of the guest function, or start + 0x200 if unknown. | `src/xenia/cpu/mmio_handler.cc` | 644, 648, 651 | 3 |
| `kXeniaAutoEnableTrollStoreJITOnLaunchPreferenceKey` | static NSString constant, iOS UI | User-defaults key of the "Auto-Enable JIT via TrollStore" toggle (stored as ios_auto_enable_jit_via_trollstore). | `src/xenia/ui/windowed_app_main_ios.mm` | 594, 3765, 10978, 11008 | 4 |
| `jit_provider_name` | local in requestAutomaticStikDebugJITHandoffForPendingLaunchPath: | "TrollStore" or "StikDebug", for the status label and log lines. | `src/xenia/ui/windowed_app_main_ios.mm` | 11006, 11013, 11045, 11046, 11056, 11058 | 6 |
| `trollstore_url` | local in requestAutomaticStikDebugJITHandoffForPendingLaunchPath: | apple-magnifier://enable-jit?bundle-id=... request for TrollStore. | `src/xenia/ui/windowed_app_main_ios.mm` | 11009, 11011, 11012 | 4 |
| `mmio_frame` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest stack frame being walked (starts at r1). | `src/xenia/cpu/mmio_handler.cc` | 670, 672, 675, 676, 687 | 6 |
| `mmio_depth` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Depth of the frame in the guest stack walk. | `src/xenia/cpu/mmio_handler.cc` | 671, 672, 673, 681, 683 | 5 |
| `mmio_caller_frame` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Back-chain pointer: the caller's frame. | `src/xenia/cpu/mmio_handler.cc` | 674, 676, 680, 682, 687 | 6 |
| `mmio_return` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest return address saved at -8 from the caller's frame. | `src/xenia/cpu/mmio_handler.cc` | 679, 682, 683, 684, 685 | 5 |
| `LogFrequentXmaContextCreation` | static function, XMA hang diagnostics | Logs the caller, guest stack and code when XMACreateContext is called 1000 times (and every 200,000 after). | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 67, 128 | 2 |
| `xma_create_count` | function-static atomic, XMA hang diagnostics | Number of XMACreateContext calls so far. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 68, 69 | 2 |
| `xma_count` | local, XMA hang diagnostics | This call's number. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 69, 70, 80, 81 | 5 |
| `xma_thread_state` | local, XMA hang diagnostics | Guest thread state of the caller. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 73, 74, 77 | 3 |
| `xma_context` | local, XMA hang diagnostics | Guest register context of the caller. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 77, 80, 96, 99 | 5 |
| `xma_memory` | local, XMA hang diagnostics | Guest memory, for reading the stack and code. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 78, 90, 105, 111 | 4 |
| `xma_dump` | local lambda, XMA hang diagnostics | Logs guest code words, eight per line. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 84, 96, 118, 120 | 4 |
| `xma_start` | lambda parameter, XMA hang diagnostics | Guest address the dump starts at. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 84, 91, 93 | 3 |
| `xma_words` | lambda parameter, XMA hang diagnostics | Number of words to dump. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 84, 85, 88 | 3 |
| `xma_line` | local, XMA hang diagnostics | Index of the first word on a dump line. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 85, 87, 88, 93 | 6 |
| `xma_text` | local, XMA hang diagnostics | Text of one dump line. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 86, 89, 93 | 3 |
| `xma_word` | local, XMA hang diagnostics | Index of the word being added. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 87, 88, 91 | 5 |
| `xma_frame` | local, XMA hang diagnostics | Guest stack frame being walked. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 99, 101, 105, 106, 123 | 6 |
| `xma_depth` | local, XMA hang diagnostics | Depth in the stack walk. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 100, 112, 114, 117 | 6 |
| `xma_caller_frame` | local, XMA hang diagnostics | Back-chain pointer to the caller's frame. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 104, 106, 107, 111, 113, 123 | 7 |
| `xma_return` | local, XMA hang diagnostics | Return address saved at -8 from the caller's frame. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 110, 113, 114, 115, 118, 120 | 6 |
| `xma_repeat_key` | function-static atomic in XmaDecoder::WriteRegister, hang diagnostics | Register number and value of the last XMA register write. | `src/xenia/apu/xma_decoder.cc` | 367, 370 | 2 |
| `xma_repeat_count` | function-static atomic in XmaDecoder::WriteRegister, hang diagnostics | How many times in a row that pair was written. | `src/xenia/apu/xma_decoder.cc` | 368, 371, 372, 376, 379 | 6 |
| `xma_key` | local in XmaDecoder::WriteRegister, hang diagnostics | This write's register/value pair. | `src/xenia/apu/xma_decoder.cc` | 369, 370 | 3 |
| `xma_thread_state` | local in XmaDecoder::WriteRegister, hang diagnostics | Guest thread state of the writer, for its link register. | `src/xenia/apu/xma_decoder.cc` | 373, 377 | 3 |
| `xma_frames` | local array in XmaDecoder::WriteRegister, hang diagnostics | Native return addresses from backtrace(). | `src/xenia/apu/xma_decoder.cc` | 380, 381, 384, 388, 394 | 5 |
| `xma_frame_count` | local in XmaDecoder::WriteRegister, hang diagnostics | Number of native frames captured. | `src/xenia/apu/xma_decoder.cc` | 381, 382 | 2 |
| `xma_frame` | local in XmaDecoder::WriteRegister, hang diagnostics | Index of the frame being logged. | `src/xenia/apu/xma_decoder.cc` | 382, 384, 386, 388, 393, 394 | 8 |
| `xma_symbol` | local in XmaDecoder::WriteRegister, hang diagnostics | dladdr() result for one frame. | `src/xenia/apu/xma_decoder.cc` | 383, 384, 385, 387, 389, 390, 391 | 10 |
| `kEmitInlineMmioChecksDefault` | file-scope constexpr | Default of the `emit_inline_mmio_checks` cvar: true on iOS, false elsewhere. | `src/xenia/memory.cc` | 42, 44, 46 | 3 |
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/kernel_flags.cc` | 18 | 1 |
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/kernel_flags.h` | 16 | 1 |
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/util/shim_utils.h` | 547, 592 | 2 |
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/util/shim_utils.cc` | 589 | 1 |
| `NoteKernelCall` | function, kernel shim | Called at the start of every kernel export trampoline; detects repeating calls per guest thread. | `src/xenia/kernel/util/shim_utils.h` | 548, 549, 594 | 3 |
| `NoteKernelCall` | function, kernel shim | Called at the start of every kernel export trampoline; detects repeating calls per guest thread. | `src/xenia/kernel/util/shim_utils.cc` | 545 | 1 |
| `kernel_call_history` | thread_local array | The thread's last 32 kernel exports, newest first. | `src/xenia/kernel/util/shim_utils.cc` | 568, 573, 581, 582, 586, 606, 610, 649, 663 | 9 |
| `kernel_call_runs` | thread_local array | For cycle lengths 1-8, how many calls in a row matched the call that many places earlier; only the shortest cycle at the threshold is reported. | `src/xenia/kernel/util/shim_utils.cc` | 570, 574, 576, 633 | 4 |
| `kernel_call_period` | local | Cycle length being checked (1-8). | `src/xenia/kernel/util/shim_utils.cc` | 571, 572, 573, 574, 576, 631, 632, 633, 647, 658 | 12 |
| `kernel_call_index` | local | Index into the history. | `src/xenia/kernel/util/shim_utils.cc` | 579, 580, 581, 582, 583, 584, 604, 605, 606, 610, 611, 647, 648, 649, 658, 659, 663 | 21 |
| `kernel_call_threshold` | local | Copy of the cvar for this call. | `src/xenia/kernel/util/shim_utils.cc` | 589, 636, 641, 642, 672, 696, 697 | 7 |
| `kernel_call_run` | local | Run length for the cycle length being checked. | `src/xenia/kernel/util/shim_utils.cc` | 633, 636, 641, 642, 668, 672, 696, 697 | 8 |
| `kernel_call_names` | local | Text of the repeating cycle ("A -> B"), of the last 32 calls in a storm report, or of a listed thread's suspend count, priority, CPU and start address. | `src/xenia/kernel/util/shim_utils.cc` | 386, 391, 400, 411, 603, 609, 620, 657, 660, 661, 663, 669 | 12 |
| `kernel_call_thread` | local | Current guest XThread, for its handle. | `src/xenia/kernel/util/shim_utils.cc` | 590, 616, 668 | 5 |
| `kernel_call_memory` | local | Guest memory, for the stack walk and the code dumps. | `src/xenia/kernel/util/shim_utils.cc` | 95, 114, 122, 418, 428, 676, 686 | 7 |
| `kernel_call_frame` | parameter of LogKernelCallStack | Guest stack frame being walked. | `src/xenia/kernel/util/shim_utils.cc` | 94, 105, 108, 110, 114, 115, 124 | 8 |
| `kernel_call_stack` | local | Text of the guest return addresses. | `src/xenia/kernel/util/shim_utils.cc` | 96, 109, 120, 127 | 4 |
| `kernel_call_depth` | local | Depth in the stack walk. | `src/xenia/kernel/util/shim_utils.cc` | 103, 104 | 3 |
| `kernel_call_caller_frame` | local | Back-chain pointer to the caller's frame. | `src/xenia/kernel/util/shim_utils.cc` | 113, 115, 116, 117, 123, 124 | 6 |
| `NoteKernelCallDone` | function, kernel shim | Clears the thread's in-call mark after every kernel export returns. | `src/xenia/kernel/util/shim_utils.h` | 550, 624 | 2 |
| `NoteKernelCallDone` | function, kernel shim | Clears the thread's in-call mark after every kernel export returns. | `src/xenia/kernel/util/shim_utils.cc` | 538 | 1 |
| `kernel_call_noted` | local in the export trampoline | Whether this call was reported to NoteKernelCall (so NoteKernelCallDone is called). | `src/xenia/kernel/util/shim_utils.h` | 592, 593, 623 | 3 |
| `KernelCallThreadState` | struct (anonymous namespace) | One guest thread's kernel-call state, readable from any thread. | `src/xenia/kernel/util/shim_utils.cc` | 59, 74, 75, 219, 547 | 5 |
| `kernel_call_threads_mutex` | file-static mutex | Guards kernel_call_threads. | `src/xenia/kernel/util/shim_utils.cc` | 73, 218, 552 | 3 |
| `kernel_call_threads` | file-static vector | Every thread's KernelCallThreadState; entries are never freed. | `src/xenia/kernel/util/shim_utils.cc` | 74, 219, 553 | 3 |
| `kernel_call_this_thread` | thread_local pointer | This thread's KernelCallThreadState. | `src/xenia/kernel/util/shim_utils.cc` | 75, 539, 540, 541, 546, 547, 549, 550, 553, 555, 556, 557, 558, 559, 561, 562, 563 | 17 |
| `LogKernelCallThreads` | function, kernel shim | Logs every guest thread: inside or last kernel call, for how long, guest lr, call count; for a thread blocked a second or more, its wait object and guest stack. Also called when the iOS app stops a game. | `src/xenia/kernel/util/shim_utils.cc` | 216, 626, 698 | 3 |
| `kernel_call_now` | local | Current host uptime in ms (in LogKernelCallThreads and in NoteKernelCall). | `src/xenia/kernel/util/shim_utils.cc` | 217, 228, 560, 561, 599, 600 | 6 |
| `kernel_call_lock` | local | Lock on kernel_call_threads_mutex. | `src/xenia/kernel/util/shim_utils.cc` | 218, 552 | 2 |
| `kernel_call_state` | local | Thread state being logged. | `src/xenia/kernel/util/shim_utils.cc` | 219, 220, 221, 228, 232, 236, 242, 266, 267, 268 | 12 |
| `kernel_call_last` | local | That thread's last export. | `src/xenia/kernel/util/shim_utils.cc` | 220, 223, 224, 234 | 5 |
| `kernel_call_inside` | local | Whether that thread is inside a kernel call now. | `src/xenia/kernel/util/shim_utils.cc` | 221, 223, 233, 235, 241 | 5 |
| `kernel_call_owner` | local | The new thread's XThread, for handle and name. | `src/xenia/kernel/util/shim_utils.cc` | 548, 549, 550 | 3 |
| `kernel_call_only_sleeps` | local | True when the repeating cycle is only KeDelayExecutionThread (idle, not reported). | `src/xenia/kernel/util/shim_utils.cc` | 646, 651, 654 | 3 |
| `kernel_call_code` | local | Guest address a code dump starts at (lr - 0x80 in a repeat report; sampled guest PC - 0x40 in the guest-thread list). | `src/xenia/kernel/util/shim_utils.cc` | 419, 420, 429, 433, 677, 687, 690 | 7 |
| `kernel_call_line` | local | First word of a code dump line. | `src/xenia/kernel/util/shim_utils.cc` | 421, 422, 424, 425, 433, 679, 680, 682, 683, 690 | 12 |
| `kernel_call_words` | local | Text of one code dump line. | `src/xenia/kernel/util/shim_utils.cc` | 423, 426, 433, 681, 684, 690 | 6 |
| `kernel_call_word` | local | Word being added to a code dump line. | `src/xenia/kernel/util/shim_utils.cc` | 424, 425, 429, 682, 683, 687 | 8 |
| `LogKernelCallStack` | function in xe::kernel::shim | Returns the guest return addresses up the stack from a frame (back chain at 0(r1), saved LR at caller frame - 8); also used by the guest crash dump. | `src/xenia/kernel/util/shim_utils.cc` | 94, 268, 414, 622, 674 | 5 |
| `guest_stack_pointer` | KernelCallThreadState field | Guest r1 at the thread's last kernel call, for its stack when it is blocked. | `src/xenia/kernel/util/shim_utils.cc` | 68, 268, 557 | 3 |
| `guest_arg0` | KernelCallThreadState field | Guest r3 at the thread's last kernel call (the handle for NtWaitForSingleObjectEx). | `src/xenia/kernel/util/shim_utils.cc` | 69, 242, 558 | 3 |
| `guest_arg1` | KernelCallThreadState field | Guest r4 at the thread's last kernel call. | `src/xenia/kernel/util/shim_utils.cc` | 70, 267, 559 | 3 |
| `kernel_call_ms` | local | How long the logged thread has been inside, or out of, its last call. | `src/xenia/kernel/util/shim_utils.cc` | 227, 235, 241 | 3 |
| `kernel_call_handle` | local | The blocked thread's r3, looked up in the object table. | `src/xenia/kernel/util/shim_utils.cc` | 242, 250, 266 | 3 |
| `kernel_call_object` | local | Text naming a wait object, or the guest function / host symbol a sampled host PC is in. | `src/xenia/kernel/util/shim_utils.cc` | 243, 251, 256, 263, 267, 336, 347, 352, 362, 378, 411 | 11 |
| `kernel_call_xobject` | local | Object-table lookup result for that handle. | `src/xenia/kernel/util/shim_utils.cc` | 248, 253, 254, 255, 257 | 5 |
| `kernel_call_total` | local | This thread's call count including this call. | `src/xenia/kernel/util/shim_utils.cc` | 563, 597, 617 | 3 |
| `kernel_call_history_lr` | thread_local array | Guest return address of each call in kernel_call_history. | `src/xenia/kernel/util/shim_utils.cc` | 569, 583, 584, 587, 611 | 5 |
| `kernel_call_storm_ms` | thread_local | Uptime at the last millionth call on this thread. | `src/xenia/kernel/util/shim_utils.cc` | 595, 599, 600 | 3 |
| `kernel_call_storm_reports` | thread_local | How many million-call bursts under a second this thread has had (every tenth is logged). | `src/xenia/kernel/util/shim_utils.cc` | 596, 602, 623 | 3 |
| `kernel_call_storm_elapsed` | local | Milliseconds this thread took for its last million calls. | `src/xenia/kernel/util/shim_utils.cc` | 598, 601, 617 | 3 |
| `kernel_call_global` | local | The global critical region mutex, only try-locked so a report never waits on a stuck thread (wait-object lookup, guest-thread list, stack walk). | `src/xenia/kernel/util/shim_utils.cc` | 99, 100, 126, 246, 247, 261, 316, 317, 323, 387, 398, 417, 436 | 13 |
| `kernel_call_xthreads` | local | Every XThread in the object table. | `src/xenia/kernel/util/shim_utils.cc` | 321, 324 | 2 |
| `kernel_call_xthread` | local | The XThread being listed. | `src/xenia/kernel/util/shim_utils.cc` | 324, 325, 330, 388, 390, 393, 394, 395, 396, 403, 404, 409, 410, 432 | 15 |
| `kernel_call_host_pc` | local / parameter | Host program counter of a thread (in SampleKernelCallHostPc and the guest-thread list). | `src/xenia/kernel/util/shim_utils.cc` | 192, 205, 211, 332, 340, 341, 346, 372, 375, 380, 411 | 11 |
| `kernel_call_host_thread` | local / parameter | A running guest thread's host thread, to sample. | `src/xenia/kernel/util/shim_utils.cc` | 191, 195, 329, 333, 334, 357 | 6 |
| `kernel_call_port` | local | Mach thread port of the host thread (in SampleKernelCallHostPc). | `src/xenia/kernel/util/shim_utils.cc` | 194, 196, 197, 198, 202, 208 | 6 |
| `kernel_call_arm_state` | local | ARM64 register state read with thread_get_state (in SampleKernelCallHostPc). | `src/xenia/kernel/util/shim_utils.cc` | 199, 203, 206 | 3 |
| `kernel_call_state_count` | local | Size argument for thread_get_state (in SampleKernelCallHostPc). | `src/xenia/kernel/util/shim_utils.cc` | 200, 204 | 2 |
| `kernel_call_function` | local | JIT guest function containing a sampled host PC, from the code cache. | `src/xenia/kernel/util/shim_utils.cc` | 339, 344, 345, 349, 358, 363, 366 | 7 |
| `kernel_call_context` | local | That thread's guest PPC context, for lr and r1. | `src/xenia/kernel/util/shim_utils.cc` | 402, 412, 413, 414 | 5 |
| `KernelCallReadable` | function in xe::kernel::shim | Whether a guest memory range is committed and readable, so the reports and the crash dump never read a page an exited thread released. | `src/xenia/kernel/util/shim_utils.cc` | 85, 108, 117, 276, 388, 390, 420 | 7 |
| `kernel_call_heap` | local | Guest heap containing the address being checked. | `src/xenia/kernel/util/shim_utils.cc` | 86, 87, 90 | 3 |
| `kernel_call_symbol` | local | dladdr() result naming the host function a sampled PC is in. | `src/xenia/kernel/util/shim_utils.cc` | 374, 376, 377, 379, 380 | 5 |
| `SampleKernelCallHostPc` | function (anonymous namespace) | Reads another thread's host PC while it is suspended for a moment (Apple ARM64; 0 elsewhere). | `src/xenia/kernel/util/shim_utils.cc` | 190, 334, 357 | 3 |
| `kernel_call_code_cache` | local | The JIT code cache, to map sampled host PCs to guest functions. | `src/xenia/kernel/util/shim_utils.cc` | 337, 341, 360 | 3 |
| `kernel_call_guest_pc` | local | Guest address of the first sample for a thread running game code; 0 if it is not in game code. | `src/xenia/kernel/util/shim_utils.cc` | 343, 345, 349, 417, 419 | 5 |
| `kernel_call_sample` | local | Index of the extra samples (7, 1 ms apart). | `src/xenia/kernel/util/shim_utils.cc` | 353, 354 | 3 |
| `kernel_call_sample_pc` | local | Host PC of one extra sample. | `src/xenia/kernel/util/shim_utils.cc` | 356, 359, 360, 367 | 4 |
| `LogKernelCallThreads` | function, kernel shim | Logs every guest thread: inside or last kernel call, for how long, guest lr, call count; for a thread blocked a second or more, its wait object and guest stack. Also called when the iOS app stops a game. | `src/xenia/kernel/util/shim_utils.h` | 554 | 1 |
| `LogKernelCallThreads` | function, kernel shim | Logs every guest thread: inside or last kernel call, for how long, guest lr, call count; for a thread blocked a second or more, its wait object and guest stack. Also called when the iOS app stops a game. | `src/xenia/app/xenia_main_ios.mm` | 586 | 1 |

| `MetalPerfCounters` | struct | Performance counters filled in across the Metal backend, logged and reset with the draw stats. | `src/xenia/gpu/metal/metal_perf_counters.h` | 26, 76 | 2 |
| `gpu_us` | MetalPerfCounters field | GPU execution time of completed submission command buffers (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 28 | 1 |
| `gpu_us` | MetalPerfCounters field | GPU execution time of completed submission command buffers (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2888, 6709 | 2 |
| `gpu_command_buffers` | MetalPerfCounters field | Number of those command buffers. | `src/xenia/gpu/metal/metal_perf_counters.h` | 29 | 1 |
| `gpu_command_buffers` | MetalPerfCounters field | Number of those command buffers. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2901, 6713 | 2 |
| `cpu_wait_us` | MetalPerfCounters field | Time the CPU spent blocked in waitUntilCompleted (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 31, 90 | 2 |
| `cpu_wait_us` | MetalPerfCounters field | Time the CPU spent blocked in waitUntilCompleted (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2905 | 1 |
| `cpu_waits` | MetalPerfCounters field | Number of those waits. | `src/xenia/gpu/metal/metal_perf_counters.h` | 32, 92 | 2 |
| `cpu_waits` | MetalPerfCounters field | Number of those waits. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2904 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/metal_perf_counters.h` | 34 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/msl_shader.cc` | 1490 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2907 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 35 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/msl_shader.cc` | 1491 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2909 | 1 |
| `pipeline_compiles` | MetalPerfCounters field | Render pipeline states created. | `src/xenia/gpu/metal/metal_perf_counters.h` | 37 | 1 |
| `pipeline_compiles` | MetalPerfCounters field | Render pipeline states created. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1118, 2912, 8789 | 3 |
| `pipeline_compile_us` | MetalPerfCounters field | Time spent creating them (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 38 | 1 |
| `pipeline_compile_us` | MetalPerfCounters field | Time spent creating them (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 1119, 2914, 8790 | 3 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_perf_counters.h` | 70 | 1 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_shared_memory.cc` | 141 | 1 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2922 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_perf_counters.h` | 76, 90, 92 | 3 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 14 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 32, 5075 | 2 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/msl_shader.cc` | 34, 1490, 1491 | 3 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_shared_memory.cc` | 11, 141 | 2 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_command_processor.cc` | 50, 941, 942, 1118, 1119, 2775, 2791, 2798, 2888, 2901, 2904, 2905, 2907, 2909, 2912, 2914, 2917, 2919, 2922, 2946, 2948, 2950, 2952, 2954, 2956, 2964, 2968, 2971, 2989, 2990, 2991, 2992, 2993, 5881, 6425, 6709, 6713, 7014, 7017, 7042, 7060, 7098, 7100, 7126, 7163, 7171, 7174, 7208, 7211, 7440, 7449, 7599, 7602, 7604, 7618, 8789, 8790 | 57 |
| `MetalPerfMicrosecondsSince` | inline function | Microseconds since a steady_clock time point. | `src/xenia/gpu/metal/metal_perf_counters.h` | 78, 91 | 2 |
| `MetalPerfMicrosecondsSince` | inline function | Microseconds since a steady_clock time point. | `src/xenia/gpu/metal/metal_command_processor.cc` | 939, 1120, 1168, 1233, 8791 | 5 |
| `metal_perf_begin` | parameter / local | Start time of a timed operation. | `src/xenia/gpu/metal/metal_perf_counters.h` | 79, 81, 88, 91 | 4 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_perf_counters.h` | 86 | 1 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1379, 1538, 3127 | 3 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3360, 4422, 4978 | 3 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2065, 2097 | 2 |
| `metal_perf_command_buffer` | parameter | Command buffer waited on. | `src/xenia/gpu/metal/metal_perf_counters.h` | 87, 89 | 2 |
| `MetalPerfThreadCpu` | function (anonymous namespace) | Host CPU share of every thread since the previous call, busiest first, with name, ID and host priority (Apple only). | `src/xenia/gpu/metal/metal_command_processor.cc` | 595, 2927 | 2 |
| `metal_perf_text` | local | Text of the thread CPU report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 596, 656, 662, 668, 670 | 5 |
| `metal_perf_previous_ns` | function static map | CPU time per system thread ID at the previous report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 598, 625, 627, 649 | 4 |
| `metal_perf_current_ns` | local map | CPU time per system thread ID now. | `src/xenia/gpu/metal/metal_command_processor.cc` | 599, 623, 649 | 3 |
| `metal_perf_threads` | local vector | (CPU time in the window, label) per thread. | `src/xenia/gpu/metal/metal_command_processor.cc` | 600, 635, 650, 660, 663, 664 | 7 |
| `metal_perf_list` | local | Thread ports from task_threads. | `src/xenia/gpu/metal/metal_command_processor.cc` | 601, 603, 615, 618, 645, 647 | 6 |
| `metal_perf_count` | local | Number of threads in the process. | `src/xenia/gpu/metal/metal_command_processor.cc` | 602, 603, 609, 648, 658 | 5 |
| `metal_perf_total_ns` | local | CPU time of all threads in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 607, 631, 658 | 3 |
| `metal_perf_index` | local | Loop index over threads. | `src/xenia/gpu/metal/metal_command_processor.cc` | 608, 609, 615, 618, 645, 659, 660, 661, 663, 664 | 11 |
| `metal_perf_identifier` | local | thread_identifier_info (system thread ID). | `src/xenia/gpu/metal/metal_command_processor.cc` | 610, 616, 623, 625, 641 | 5 |
| `metal_perf_info_count` | local | Size argument for THREAD_IDENTIFIER_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 611, 617 | 2 |
| `metal_perf_extended` | local | thread_extended_info (CPU time, name, priority). | `src/xenia/gpu/metal/metal_command_processor.cc` | 612, 619, 621, 622, 633, 638, 639, 642 | 9 |
| `metal_perf_extended_count` | local | Size argument for THREAD_EXTENDED_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 613, 620 | 2 |
| `metal_perf_ns` | local | A thread's total CPU time. | `src/xenia/gpu/metal/metal_command_processor.cc` | 621, 623, 628, 629, 630 | 5 |
| `metal_perf_previous` | local iterator | Its entry from the previous report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 624, 627, 629 | 3 |
| `metal_perf_delta` | local | Its CPU time in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 626, 631, 632, 636 | 4 |
| `metal_perf_window_ns` | local | Window length in ns. | `src/xenia/gpu/metal/metal_command_processor.cc` | 654, 658, 665 | 3 |
| `metal_perf_left` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_command_processor.cc` | 651, 652 | 2 |
| `metal_perf_left` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 648, 649, 857, 859 | 4 |
| `metal_perf_right` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_command_processor.cc` | 651, 652 | 2 |
| `metal_perf_right` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 648, 650, 858, 859 | 4 |
| `metal_perf_footprint_mb` | local | Process physical footprint (task_info TASK_VM_INFO). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2871, 2878, 2925 | 3 |
| `metal_perf_vm_info` | local | TASK_VM_INFO result. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2873, 2876, 2878 | 3 |
| `metal_perf_vm_count` | local | Size argument for TASK_VM_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2874, 2877 | 2 |
| `metal_perf_available_mb` | local | os_proc_available_memory() before iOS ends the app (iOS only). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2881, 2883, 2925 | 3 |
| `metal_perf_window_us` | local | Window length: host time between the window's swaps. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2885, 2903, 2927, 2999 | 4 |
| `metal_perf_gpu_us` | local | GPU time in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2887, 2900, 2903 | 3 |
| `metal_perf_swap_time` | local | Host time of this swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2843, 2847, 2855 | 3 |
| `metal_perf_frame_us` | local | Host time since the previous swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2845, 2849, 2851, 2852, 2853 | 5 |
| `pipeline_compile_begin` | local (two functions) | Start time of a newRenderPipelineState call. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1115, 1120, 8787, 8791 | 4 |
| `completed_buffer` | lambda parameter | Completed submission command buffer, for its GPU times. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6704, 6707 | 3 |
| `metal_perf_gpu_seconds` | local | Its GPUEndTime - GPUStartTime. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6706, 6708, 6710 | 3 |
| `frame_us_total` | DrawStats field | Host time between swaps in the window (us). | `src/xenia/gpu/metal/metal_command_processor.h` | 654 | 1 |
| `frame_us_total` | DrawStats field | Host time between swaps in the window (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2849, 2886, 2898 | 3 |
| `frame_us_max` | DrawStats field | Longest frame in the window (us). | `src/xenia/gpu/metal/metal_command_processor.h` | 655 | 1 |
| `frame_us_max` | DrawStats field | Longest frame in the window (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2850, 2851, 2899 | 3 |
| `frames_over_33ms` | DrawStats field | Frames longer than 33 ms. | `src/xenia/gpu/metal/metal_command_processor.h` | 656 | 1 |
| `frames_over_33ms` | DrawStats field | Frames longer than 33 ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2852, 2899 | 2 |
| `frames_over_50ms` | DrawStats field | Frames longer than 50 ms. | `src/xenia/gpu/metal/metal_command_processor.h` | 657 | 1 |
| `frames_over_50ms` | DrawStats field | Frames longer than 50 ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2853, 2900 | 2 |
| `metal_perf_last_swap_` | MetalCommandProcessor member | Host time of the previous swap. | `src/xenia/gpu/metal/metal_command_processor.h` | 697 | 1 |
| `metal_perf_last_swap_` | MetalCommandProcessor member | Host time of the previous swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2844, 2847, 2855 | 3 |
| `unhandled_host_exception_count` | function static atomic | Unhandled host faults so far, to log only the first ten and then one in a million. | `src/xenia/base/exception_handler_mac.cc` | 224, 226 | 2 |
| `unhandled_count` | local | This fault's number. | `src/xenia/base/exception_handler_mac.cc` | 225, 228, 233 | 4 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading.h` | 130 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading_posix.cc` | 200 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading_win.cc` | 164 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/gpu/command_processor.cc` | 412 | 1 |
| `fifo_min` | function static in PosixCondition<Thread>::set_priority | Darwin SCHED_FIFO minimum priority, for the 1..32 remap. Ported from XeniOS (same name). | `src/xenia/base/threading_posix.cc` | 932, 935 | 3 |
| `fifo_max` | function static in PosixCondition<Thread>::set_priority | Darwin SCHED_FIFO maximum priority, for the 1..32 remap. Ported from XeniOS (same name). | `src/xenia/base/threading_posix.cc` | 933, 935 | 2 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 31, 83, 89, 114, 117, 119 | 6 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 224, 227, 251, 263, 264, 272, 276, 368, 391, 412, 496, 534, 569, 581 | 14 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 65, 455 | 2 |
| `sample_buffer` | MetalGpuPassRecord field; local in Initialize | Counter sample buffer holding the pass timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 32 | 1 |
| `sample_buffer` | MetalGpuPassRecord field; local in Initialize | Counter sample buffer holding the pass timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 172, 174, 177, 184, 185, 252, 342, 377, 400, 418, 538 | 11 |
| `sample_buffer_index` | MetalGpuPassRecord field | Index of that buffer in sample_buffers_, to free its slots. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 33 | 1 |
| `sample_buffer_index` | MetalGpuPassRecord field | Index of that buffer in sample_buffers_, to free its slots. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 253, 537, 538, 539 | 4 |
| `first_sample` | MetalGpuPassRecord field | First of the pass timestamp slots. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 34 | 1 |
| `first_sample` | MetalGpuPassRecord field | First of the pass timestamp slots. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 254, 343, 344, 345, 346, 378, 379, 401, 402, 419 | 10 |
| `sample_count` | MetalGpuPassRecord field; AllocateSamples parameter | Timestamps per pass: 4 for render passes (vertex and fragment start and end), 2 otherwise. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 36, 115 | 2 |
| `sample_count` | MetalGpuPassRecord field; AllocateSamples parameter | Timestamps per pass: 4 for render passes (vertex and fragment start and end), 2 otherwise. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 225, 233, 255, 257, 419, 420, 424, 443, 520 | 10 |
| `label` | MetalGpuPassRecord field; encoder helper parameter | Static string naming what the pass is for; null discards the pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 38, 83, 85, 87, 115 | 5 |
| `label` | MetalGpuPassRecord field; encoder helper parameter | Static string naming what the pass is for; null discards the pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 226, 256, 272, 277, 357, 367, 369, 384, 390, 392, 407, 417, 431, 493, 525, 530 | 17 |
| `width` | MetalGpuPassRecord field | Width of the first attachment (counts include texture->width()). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 40 | 1 |
| `width` | MetalGpuPassRecord field | Width of the first attachment (counts include texture->width()). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 290, 298, 299, 324, 525 | 5 |
| `height` | MetalGpuPassRecord field | Height of the first attachment (counts include texture->height()). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 41 | 1 |
| `height` | MetalGpuPassRecord field | Height of the first attachment (counts include texture->height()). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 291, 300, 326, 525 | 4 |
| `samples` | MetalGpuPassRecord field | Sample count of the first attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 42, 108 | 2 |
| `samples` | MetalGpuPassRecord field | Sample count of the first attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 301, 525, 706 | 3 |
| `color_formats` | MetalGpuPassRecord field | MTL::PixelFormat of color attachments 0-3. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 43 | 1 |
| `color_formats` | MetalGpuPassRecord field | MTL::PixelFormat of color attachments 0-3. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 334, 526, 527 | 5 |
| `depth_format` | MetalGpuPassRecord field | MTL::PixelFormat of the depth attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 44 | 1 |
| `depth_format` | MetalGpuPassRecord field | MTL::PixelFormat of the depth attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 337, 528 | 2 |
| `load_actions` | MetalGpuPassRecord field | Load actions, 2 bits per attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 47 | 1 |
| `load_actions` | MetalGpuPassRecord field | Load actions, 2 bits per attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 305, 528 | 2 |
| `store_actions` | MetalGpuPassRecord field | Store actions, 2 bits per attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 48 | 1 |
| `store_actions` | MetalGpuPassRecord field | Store actions, 2 bits per attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 306, 528 | 2 |
| `draws` | MetalGpuPassRecord, MetalGpuPassPipeline and MetalGpuPassStats field | Guest draws encoded in the pass (in metal_command_processor.cc only at the draw count lines, see Checks). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 55, 56, 58, 65, 100, 143, 152, 153 | 8 |
| `draws` | MetalGpuPassRecord, MetalGpuPassPipeline and MetalGpuPassStats field | Guest draws encoded in the pass (in metal_command_processor.cc only at the draw count lines, see Checks). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 23, 485, 494, 500, 506, 508, 576, 600, 625, 627, 705, 709, 714, 790, 794, 864, 870, 872, 873 | 24 |
| `vertices` | MetalGpuPassRecord, MetalGpuPassPipeline and MetalGpuPassStats field; NoteDraw parameter | Host vertices drawn in the pass (in metal_command_processor.cc only at the draw count lines, see Checks). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 57, 66, 92, 101, 144 | 5 |
| `vertices` | MetalGpuPassRecord, MetalGpuPassPipeline and MetalGpuPassStats field; NoteDraw parameter | Host vertices drawn in the pass (in metal_command_processor.cc only at the draw count lines, see Checks). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 486, 495, 501, 503, 504, 505, 572, 577, 601, 625, 627, 705, 710, 715 | 18 |
| `MetalGpuPassTimer` | class | Per-pass GPU time from stage boundary timestamps; creates the timed encoders and logs the totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 55, 76, 150 | 3 |
| `MetalGpuPassTimer` | class | Per-pass GPU time from stage boundary timestamps; creates the timed encoders and logs the totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 94, 220, 224, 270, 366, 389, 412, 534, 545, 569, 604 | 11 |
| `Initialize` | MetalGpuPassTimer member function | Creates the sample buffers for a device and calibrates GPU ticks. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 78 | 1 |
| `Initialize` | MetalGpuPassTimer member function | Creates the sample buffers for a device and calibrates GPU ticks. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 94 | 1 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 74, 81 | 2 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 270 | 1 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7583 | 1 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4102, 6177 | 2 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 74, 84 | 2 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 366 | 1 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4115, 4223, 4946, 5162 | 4 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1112 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 75, 86 | 2 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 389 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 5693 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1241 | 1 |
| `Log` | MetalGpuPassTimer and MetalCompileStallTracker member function | Logs and resets the window totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 95, 165 | 2 |
| `Log` | MetalGpuPassTimer and MetalCompileStallTracker member function | Logs and resets the window totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 604, 819 | 2 |
| `Enabled` | MetalGpuPassTimer member function | Supported and metal_gpu_pass_timing on. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 113 | 1 |
| `Enabled` | MetalGpuPassTimer member function | Supported and metal_gpu_pass_timing on. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 220, 276, 369, 392 | 4 |
| `AllocateSamples` | MetalGpuPassTimer member function | Takes timestamp slots for a pass and registers its completion handler. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 114 | 1 |
| `AllocateSamples` | MetalGpuPassTimer member function | Takes timestamp slots for a pass and registers its completion handler. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 224, 277, 369, 392 | 4 |
| `Complete` | MetalGpuPassTimer member function | Adds a completed pass GPU time to the totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 117 | 1 |
| `Complete` | MetalGpuPassTimer member function | Adds a completed pass GPU time to the totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 266, 356, 412 | 3 |
| `Release` | MetalGpuPassTimer member function | Frees a pass slots and record when its handler is destroyed. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 119 | 1 |
| `Release` | MetalGpuPassTimer member function | Frees a pass slots and record when its handler is destroyed. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 264, 534 | 2 |
| `MetalGpuPassStats` | struct in MetalGpuPassTimer | Totals for a kind of pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 98, 141, 142, 146 | 4 |
| `MetalGpuPassStats` | struct in MetalGpuPassTimer | Totals for a kind of pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 483, 498, 623, 642 | 4 |
| `passes` | MetalGpuPassStats field | Passes timed. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 35, 39, 72, 73, 74, 99 | 6 |
| `passes` | MetalGpuPassStats field | Passes timed. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 349, 484, 499, 625, 626, 660, 671, 706, 710 | 9 |
| `total_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks from the first stage start to the last stage end. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 102 | 1 |
| `total_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks from the first stage start to the last stage end. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 441, 466, 469, 476, 487, 511, 626, 649, 650, 708 | 11 |
| `vertex_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks of the vertex stage. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 103 | 1 |
| `vertex_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks of the vertex stage. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 439, 460, 469, 488, 504, 506, 509, 628, 630, 708 | 11 |
| `fragment_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks of the fragment stage. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 104 | 1 |
| `fragment_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks of the fragment stage. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 440, 463, 469, 489, 508, 510, 628, 631, 709 | 10 |
| `record_out` | RenderEncoder parameter | Receives the pass record, for the draw count. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 79, 83 | 2 |
| `record_out` | RenderEncoder parameter | Receives the pass record, for the draw count. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 272, 273, 274, 360, 361 | 5 |
| `mutex_` | MetalGpuPassTimer and MetalCompileStallTracker member | Guards each class's state. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 126, 179 | 2 |
| `mutex_` | MetalGpuPassTimer and MetalCompileStallTracker member | Guards each class's state. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 95, 229, 435, 536, 591, 605, 725, 740, 753, 766, 820 | 11 |
| `device_` | MetalGpuPassTimer member | Device the sample buffers belong to. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 127 | 1 |
| `device_` | MetalGpuPassTimer member | Device the sample buffers belong to. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 99, 118, 612 | 3 |
| `supported_` | MetalGpuPassTimer member | The device can take stage boundary timestamps and the buffers exist. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 128 | 1 |
| `supported_` | MetalGpuPassTimer member | The device can take stage boundary timestamps and the buffers exist. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 117, 212, 221, 606 | 4 |
| `sample_buffers_` | MetalGpuPassTimer member | The 16 counter sample buffers. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 129 | 1 |
| `sample_buffers_` | MetalGpuPassTimer member | The 16 counter sample buffers. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 104, 106, 109, 177, 180, 184, 187, 191, 216, 230, 236, 238, 252, 537, 538 | 15 |
| `sample_buffer_outstanding_` | MetalGpuPassTimer member | Pending passes per sample buffer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 131 | 1 |
| `sample_buffer_outstanding_` | MetalGpuPassTimer member | Pending passes per sample buffer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 105, 110, 178, 188, 239, 258, 539 | 7 |
| `sample_buffer_size_` | MetalGpuPassTimer member | Timestamps per sample buffer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 132 | 1 |
| `sample_buffer_size_` | MetalGpuPassTimer member | Timestamps per sample buffer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 181, 216, 233 | 3 |
| `current_sample_buffer_` | MetalGpuPassTimer member | Sample buffer being filled. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 133 | 1 |
| `current_sample_buffer_` | MetalGpuPassTimer member | Sample buffer being filled. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 115, 238, 240, 252, 253, 258 | 6 |
| `next_sample_` | MetalGpuPassTimer member | Next free slot in it. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 134 | 1 |
| `next_sample_` | MetalGpuPassTimer member | Next free slot in it. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 116, 233, 241, 254, 257 | 5 |
| `untimed_passes_` | MetalGpuPassTimer member | Passes not timed because no sample buffer was free. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 135 | 1 |
| `untimed_passes_` | MetalGpuPassTimer member | Passes not timed because no sample buffer was free. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 113, 247, 607, 662, 720 | 5 |
| `invalid_passes_` | MetalGpuPassTimer member | Passes whose timestamps were unusable. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 136 | 1 |
| `invalid_passes_` | MetalGpuPassTimer member | Passes whose timestamps were unusable. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 114, 480, 607, 662, 721 | 5 |
| `calibration_gpu_` | MetalGpuPassTimer member | GPU timestamp at Initialize. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 138 | 1 |
| `calibration_gpu_` | MetalGpuPassTimer member | GPU timestamp at Initialize. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 201, 205, 210, 613, 618 | 5 |
| `calibration_time_` | MetalGpuPassTimer member | Steady clock time at Initialize. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 139 | 1 |
| `calibration_time_` | MetalGpuPassTimer member | Steady clock time at Initialize. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 202, 208, 616 | 3 |
| `ns_per_tick_` | MetalGpuPassTimer member | Nanoseconds per GPU tick, from the calibration. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 140 | 1 |
| `ns_per_tick_` | MetalGpuPassTimer member | Nanoseconds per GPU tick, from the calibration. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 206, 216, 451, 614, 621, 662 | 6 |
| `stats_by_label_` | MetalGpuPassTimer member | Totals per kind of pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 141 | 1 |
| `stats_by_label_` | MetalGpuPassTimer member | Totals per kind of pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 111, 493, 607, 656, 676, 717 | 6 |
| `stats_by_pass_` | MetalGpuPassTimer member | Totals per render pass configuration. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 142 | 1 |
| `stats_by_pass_` | MetalGpuPassTimer member | Totals per render pass configuration. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 112, 523, 530, 665, 718 | 5 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 150 | 1 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1534, 2999, 6386, 7583 | 4 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4102, 4115, 4223, 4946, 5162, 5693, 6177 | 7 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1112, 1241 | 2 |
| `metal_gpu_pass_timing` | cvar (Metal); iOS settings key | Turns pass timing, the traffic estimate, per-pipeline time and compile stall tracking on (default) or off; Diagnostics section toggle. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 10, 19, 217, 221, 737, 750 | 7 |
| `metal_gpu_pass_timing` | cvar (Metal); iOS settings key | Turns pass timing, the traffic estimate, per-pipeline time and compile stall tracking on (default) or off; Diagnostics section toggle. | `src/xenia/ui/windowed_app_main_ios.mm` | 3783 | 1 |
| `metal_gpu_pass_record_` | MetalCommandProcessor member | Record of the current render encoder, for its draw count. | `src/xenia/gpu/metal/metal_command_processor.h` | 455 | 1 |
| `metal_gpu_pass_record_` | MetalCommandProcessor member | Record of the current render encoder, for its draw count. | `src/xenia/gpu/metal/metal_command_processor.cc` | 688, 2035, 2109, 6387, 7313, 7585 | 6 |
| `timestamp_set` | local in Initialize | The timestamp counter set. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 129, 148, 150, 156, 164 | 5 |
| `counter_sets` | local in Initialize | The device counter sets. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 133, 134, 135 | 3 |
| `counter_set` | local in Initialize | One of them. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 135, 136, 139, 140, 148, 150 | 7 |
| `cpu_timestamp` | local | CPU half of sampleTimestamps (unused). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 199, 201, 204, 610, 612 | 5 |
| `gpu_timestamp` | local | GPU half of sampleTimestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 200, 204, 205, 210, 611, 612, 613, 618 | 8 |
| `holder` | local in AllocateSamples | shared_ptr owning the record, captured by the completion handler. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 263, 266 | 3 |
| `released` | lambda parameter | Record to release. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 261, 264 | 3 |
| `describe_attachment` | lambda in RenderEncoder | Records an attachment format and actions. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 282, 335, 337, 338 | 4 |
| `sample_attachment` | local | Sample buffer attachment of the pass descriptor. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 340, 342, 343, 344, 345, 346, 350, 351, 352, 353, 354, 375, 377, 378, 379, 398, 400, 401, 402 | 19 |
| `timestamps` | local in Complete | Resolved timestamps of the pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 21, 126, 131, 197, 214, 415, 425, 444, 445, 446, 447, 449, 453, 455, 460, 463, 466, 467, 473, 474, 476 | 34 |
| `resolved` | local in Complete | The timestamps were resolved. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 416, 427, 442 | 3 |
| `results` | local in Complete | resolveCounterRange data as timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 422, 425 | 2 |
| `valid` | lambda in Complete | A timestamp is neither 0 nor CounterErrorValue. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 436, 444, 446, 473 | 7 |
| `vertex_ok` | local in Complete | The vertex timestamps are usable. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 444, 452, 454, 459, 465, 471 | 6 |
| `fragment_ok` | local in Complete | The fragment timestamps are usable. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 446, 452, 456, 462, 465, 471 | 6 |
| `stale_ticks` | local in Complete | One second in GPU ticks, to spot a stage left with old timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 451, 453, 455 | 3 |
| `usable` | local in Complete | The pass has usable timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 442, 443, 471, 472, 473, 475, 479 | 7 |
| `add` | lambda in Complete | Adds the pass to a MetalGpuPassStats. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 483, 493, 523, 530 | 4 |
| `ms_per_frame` | lambda in Log | GPU ticks to milliseconds per frame. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 620, 626, 630, 631, 708, 709 | 7 |
| `describe` | lambda in Log | Text for a MetalGpuPassStats. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 623, 657, 669 | 3 |
| `by_time` | lambda in Log | Totals sorted by GPU time. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 640, 656, 665, 689 | 4 |
| `listed` | local in Log | Top passes printed so far. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 34, 131, 664, 666, 688, 690 | 6 |
| `stats` | lambda parameter | Totals to add to or describe. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 483, 484, 485, 486, 487, 488, 489, 490, 491, 498, 499, 500, 501, 509, 510, 511, 515, 516, 623, 626, 627, 628, 630, 631, 633, 635, 636, 640, 642, 644 | 34 |
| `ticks` | lambda parameter | GPU ticks to convert. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 197, 620, 621 | 3 |
| `load_bytes` | MetalGpuPassRecord and MetalGpuPassStats field; local in Log | Estimated bytes of whole attachments loaded (LoadActionLoad). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 53, 105 | 2 |
| `load_bytes` | MetalGpuPassRecord and MetalGpuPassStats field; local in Log | Estimated bytes of whole attachments loaded (LoadActionLoad). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 313, 490, 633, 635, 674, 677, 684, 686 | 11 |
| `store_bytes` | MetalGpuPassRecord and MetalGpuPassStats field; local in Log | Estimated bytes of attachments stored, including multisample resolve output. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 54, 106 | 2 |
| `store_bytes` | MetalGpuPassRecord and MetalGpuPassStats field; local in Log | Estimated bytes of attachments stored, including multisample resolve output. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 317, 323, 491, 633, 636, 675, 678, 684, 685, 686 | 12 |
| `pipelines` | MetalGpuPassRecord field | Draws per pipeline in the pass, consecutive draws merged. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 68 | 1 |
| `pipelines` | MetalGpuPassRecord field | Draws per pipeline in the pass, consecutive draws merged. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 497, 578, 579, 580, 598, 600, 601, 714 | 8 |
| `MetalGpuPassPipeline` | struct in MetalGpuPassRecord | One pipeline (shader pair) in a pass with its draws and vertices. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 59, 68 | 2 |
| `MetalGpuPassPipeline` | struct in MetalGpuPassRecord | One pipeline (shader pair) in a pass with its draws and vertices. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 496, 581 | 2 |
| `vertex_translation` | MetalGpuPassPipeline field; NoteDraw parameter | Vertex shader translation, to merge consecutive draws (also an existing local name in metal_command_processor.cc). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 60, 90 | 2 |
| `vertex_translation` | MetalGpuPassPipeline field; NoteDraw parameter | Vertex shader translation, to merge consecutive draws (also an existing local name in metal_command_processor.cc). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 570, 579, 582, 584, 585, 586 | 8 |
| `pixel_translation` | MetalGpuPassPipeline field; NoteDraw and MetalGpuPixelShaderFeatures parameter | Pixel shader translation (also an existing local name in metal_command_processor.cc). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 61, 91, 124 | 3 |
| `pixel_translation` | MetalGpuPassPipeline field; NoteDraw and MetalGpuPixelShaderFeatures parameter | Pixel shader translation (also an existing local name in metal_command_processor.cc). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 512, 513, 546, 547, 571, 580, 583, 588, 589, 590, 592, 595 | 15 |
| `shaders` | MetalGpuPassPipeline field | Vertex shader hash and modification, pixel shader hash and modification. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 64 | 1 |
| `shaders` | MetalGpuPassPipeline field | Vertex shader hash and modification, pixel shader hash and modification. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 498, 585, 586, 589, 590 | 5 |
| `features` | MetalGpuPassStats field; locals in MetalGpuPixelShaderFeatures, Complete and Log | Pixel shader feature bits: 1 discard_fragment, 2 depth output, 4 raster order groups. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 107, 109, 120 | 3 |
| `features` | MetalGpuPassStats field; locals in MetalGpuPixelShaderFeatures, Complete and Log | Pixel shader feature bits: 1 discard_fragment, 2 depth output, 4 raster order groups. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 513, 514, 515, 516, 548, 550, 553, 556, 566, 693, 694, 695, 697, 698, 700, 701, 711 | 19 |
| `texture_samples` | MetalGpuPassStats field; local in MetalGpuPixelShaderFeatures | Texture sample, gather and read calls in the pixel shader source. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 110 | 1 |
| `texture_samples` | MetalGpuPassStats field; local in MetalGpuPixelShaderFeatures | Texture sample, gather and read calls in the pixel shader source. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 516, 558, 563, 566, 711 | 5 |
| `NoteDraw` | MetalGpuPassTimer member function | Counts a guest draw in its pass, per pipeline. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 55, 89 | 2 |
| `NoteDraw` | MetalGpuPassTimer member function | Counts a guest draw in its pass, per pipeline. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 569 | 1 |
| `NoteDraw` | MetalGpuPassTimer member function | Counts a guest draw in its pass, per pipeline. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6386 | 1 |
| `MetalGpuPixelShaderFeatures` | MetalGpuPassTimer member function | Scans a pixel shader MSL source for features slow on Apple GPUs and counts its texture samples. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 107, 123 | 2 |
| `MetalGpuPixelShaderFeatures` | MetalGpuPassTimer member function | Scans a pixel shader MSL source for features slow on Apple GPUs and counts its texture samples. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 545, 595 | 2 |
| `stats_by_pipeline_` | MetalGpuPassTimer member | Totals per pipeline (pass time shared by draws and vertices). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 146 | 1 |
| `stats_by_pipeline_` | MetalGpuPassTimer member | Totals per pipeline (pass time shared by draws and vertices). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 98, 498, 689, 719 | 4 |
| `pixel_shader_features_` | MetalGpuPassTimer member | MetalGpuPixelShaderFeatures result per pixel shader translation. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 147 | 1 |
| `pixel_shader_features_` | MetalGpuPassTimer member | MetalGpuPixelShaderFeatures result per pixel shader translation. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 97, 513, 514, 592, 593, 594 | 6 |
| `window_us` | Log parameter | Host time of the window, for GB/s. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 94, 95 | 2 |
| `window_us` | Log parameter | Host time of the window, for GB/s. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 604, 680 | 2 |
| `MetalGpuBytesPerPixel` | function (anonymous namespace) | Bytes per sample of a render target format, for the traffic estimate. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 35, 309 | 2 |
| `stencil` | MetalGpuBytesPerPixel parameter | The stencil plane alone (1 byte). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 34, 35, 36, 73, 522 | 7 |
| `load_action` | local in RenderEncoder | Attachment load action. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 303, 305, 312 | 3 |
| `store_action` | local in RenderEncoder | Attachment store action. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 304, 306, 315, 316, 319, 320 | 6 |
| `pixel_bytes` | local in RenderEncoder | Bytes per sample of the attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 308, 311, 323 | 3 |
| `attachment_bytes` | local in RenderEncoder | Bytes of the whole attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 310, 313, 317 | 3 |
| `resolve_texture` | local in RenderEncoder | Multisample resolve destination. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 321, 322, 324, 326 | 4 |
| `counter_text` | local in Initialize | Logged list of the GPU counter sets and counters. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 132, 139, 144, 155 | 5 |
| `pipeline_vertex_ticks` | local in Complete | A pipeline share of the vertex time. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 502, 509, 511 | 3 |
| `pipeline_fragment_ticks` | local in Complete | A pipeline share of the fragment time. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 507, 510, 511 | 3 |
| `source` | local in MetalGpuPixelShaderFeatures | The MSL source. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 547, 549, 552, 555, 561, 562 | 6 |
| `call` | local in MetalGpuPixelShaderFeatures | Texture call searched for. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 559, 561, 562 | 3 |
| `position` | local in MetalGpuPixelShaderFeatures | Search position. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 561, 562 | 4 |
| `window_seconds` | local in Log | Window length in seconds. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 680, 686 | 2 |
| `MetalCompileStallTracker` | class | Tracks async shader and pipeline compiles that draws were skipped for. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 155, 190 | 2 |
| `MetalCompileStallTracker` | class | Tracks async shader and pipeline compiles that draws were skipped for. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 724, 736, 749, 762, 806, 819 | 6 |
| `MetalCompileKey` | type alias in MetalCompileStallTracker | {kind, hashes and modifications} of a shader or pipeline compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 160, 161, 162, 164, 171, 177, 180 | 7 |
| `MetalCompileKey` | type alias in MetalCompileStallTracker | {kind, hashes and modifications} of a shader or pipeline compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 736, 749, 762, 806 | 4 |
| `Queued` | MetalCompileStallTracker member function | A compile was queued. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 161 | 1 |
| `Queued` | MetalCompileStallTracker member function | A compile was queued. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 736 | 1 |
| `Queued` | MetalCompileStallTracker member function | A compile was queued. | `src/xenia/gpu/metal/metal_command_processor.cc` | 978, 1016 | 2 |
| `Skipped` | MetalCompileStallTracker member function | A draw was skipped for a pending compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 162 | 1 |
| `Skipped` | MetalCompileStallTracker member function | A draw was skipped for a pending compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 749 | 1 |
| `Skipped` | MetalCompileStallTracker member function | A draw was skipped for a pending compile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 4749, 4780, 4825 | 3 |
| `Finished` | MetalCompileStallTracker member function | A compile finished: buckets it and notes it if draws waited 100 ms or more. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 164, 181 | 2 |
| `Finished` | MetalCompileStallTracker member function | A compile finished: buckets it and notes it if draws waited 100 ms or more. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 762 | 1 |
| `Finished` | MetalCompileStallTracker member function | A compile finished: buckets it and notes it if draws waited 100 ms or more. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1191, 1251 | 2 |
| `Clear` | MetalCompileStallTracker member function | Forgets everything for a new title. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 167 | 1 |
| `Clear` | MetalCompileStallTracker member function | Forgets everything for a new title. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 724 | 1 |
| `Clear` | MetalCompileStallTracker member function | Forgets everything for a new title. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1535 | 1 |
| `Describe` | MetalCompileStallTracker static member function | Text for a MetalCompileKey. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 177 | 1 |
| `Describe` | MetalCompileStallTracker static member function | Text for a MetalCompileKey. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 795, 806, 866 | 3 |
| `MetalCompileStall` | struct in MetalCompileStallTracker | One pending or finished compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 170, 180, 187 | 3 |
| `MetalCompileStall` | struct in MetalCompileStallTracker | One pending or finished compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 742, 771, 857, 858 | 4 |
| `key` | MetalCompileStall field | Its MetalCompileKey (also generic loop names in the .cc). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 161, 162, 164, 171, 177 | 5 |
| `key` | MetalCompileStall field | Its MetalCompileKey (also generic loop names in the .cc). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 644, 645, 656, 657, 665, 669, 676, 689, 707, 736, 741, 743, 745, 749, 756, 762, 767, 795, 806, 807, 809, 811, 814, 815, 831, 866 | 34 |
| `queued_at` | MetalCompileStall field | Time it was queued. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 172 | 1 |
| `queued_at` | MetalCompileStall field | Time it was queued. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 744, 775, 840 | 3 |
| `skipped` | MetalCompileStall field; local in Log | Draws skipped for it. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 152, 154, 173 | 3 |
| `skipped` | MetalCompileStall field; local in Log | Draws skipped for it. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 23, 758, 785, 788, 794, 797, 822, 825, 832, 836, 846, 859, 864, 866, 871, 872, 873, 874 | 20 |
| `pending_us` | MetalCompileStall field | Queued to finished. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 174 | 1 |
| `pending_us` | MetalCompileStall field | Queued to finished. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 773, 777, 781, 786, 791, 795, 796, 867 | 8 |
| `compile_us` | MetalCompileStall field; Finished parameter; locals in MslShaderCompileThread | Time of the compile itself. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 163, 164, 175 | 3 |
| `compile_us` | MetalCompileStall field; Finished parameter; locals in MslShaderCompileThread | Time of the compile itself. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 763, 777, 786, 787, 796, 797, 867 | 8 |
| `compile_us` | MetalCompileStall field; Finished parameter; locals in MslShaderCompileThread | Time of the compile itself. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1168, 1196, 1228, 1233, 1257 | 5 |
| `pending_` | MetalCompileStallTracker member | Compiles queued and not finished. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 180 | 1 |
| `pending_` | MetalCompileStallTracker member | Compiles queued and not finished. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 726, 741, 745, 756, 757, 767, 768, 772, 831 | 10 |
| `bucket_compiles_` | MetalCompileStallTracker member | Finished compiles per pending-time bucket. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 183 | 1 |
| `bucket_compiles_` | MetalCompileStallTracker member | Finished compiles per pending-time bucket. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 729, 784, 824, 854, 877 | 5 |
| `bucket_skipped_` | MetalCompileStallTracker member | Draws skipped per pending-time bucket. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 184 | 1 |
| `bucket_skipped_` | MetalCompileStallTracker member | Draws skipped per pending-time bucket. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 730, 785, 825, 854, 878 | 5 |
| `queue_us_` | MetalCompileStallTracker member | Total queue wait in the window. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 185 | 1 |
| `queue_us_` | MetalCompileStallTracker member | Total queue wait in the window. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 732, 786, 874, 880 | 4 |
| `compile_us_` | MetalCompileStallTracker member | Total compile time in the window. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 186 | 1 |
| `compile_us_` | MetalCompileStallTracker member | Total compile time in the window. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 733, 787, 874, 881 | 4 |
| `finished_` | MetalCompileStallTracker member | Finished compiles with skipped draws in the window. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 187 | 1 |
| `finished_` | MetalCompileStallTracker member | Finished compiles with skipped draws in the window. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 727, 789, 856, 862, 866, 867, 882 | 10 |
| `metal_compile_stall_tracker` | inline global | The one MetalCompileStallTracker. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 190 | 1 |
| `metal_compile_stall_tracker` | inline global | The one MetalCompileStallTracker. | `src/xenia/gpu/metal/metal_command_processor.cc` | 978, 1016, 1191, 1251, 1535, 3000, 4749, 4780, 4825 | 9 |
| `stall` | locals in Skipped and Finished | The tracked compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 742, 743, 744, 745, 756, 757, 758, 771, 773, 775, 777, 781, 785, 786, 787, 788, 789, 791, 793, 795, 796, 797, 831, 832, 836, 840 | 31 |
| `note` | local in Finished | The stall note to log after unlocking. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 764, 790, 792, 801, 802 | 5 |
| `bucket_limits_ms` | function static in Finished | Bucket upper limits. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 778, 781 | 2 |
| `bucket` | local in Finished | Bucket of the compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 780, 781, 782, 784, 785, 846 | 8 |
| `compiles` | local in Log | Compiles finished in the window. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 23, 821, 824, 843, 870, 872, 874 | 7 |
| `waiting` | local in Log | Compiles still pending with draws skipped. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 827, 835, 843, 875 | 4 |
| `waiting_skipped` | local in Log | Draws skipped for those. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 828, 836, 875 | 3 |
| `oldest_us` | local in Log | Age of the oldest of them. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 829, 837, 838, 875 | 4 |
| `bucket_names` | function static in Log | Bucket labels. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 848, 853 | 2 |
| `worst` | local in Log | Text of the 8 compiles with the most skipped draws. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 861, 863, 875 | 3 |
| `XE_TRANSFER_STENCIL_OUTPUT` | MSL preprocessor define (transfer shaders) | 1 when a depth-output transfer shader also writes the guest stencil through [[stencil]]. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 7197, 8371, 8688, 8726, 8784, 8955 | 6 |
| `stencil` | TransferDepthOut member (MSL) | The [[stencil]] output of a depth transfer with native stencil output (only these two lines; the word is common in the file). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 747, 751, 752, 1909, 1970, 2087, 2149, 2266, 2322, 3632, 3633, 4047, 4056, 6115, 6191, 6555, 7127, 8372, 8690, 8691, 8785, 8786, 8956, 9314, 9315, 9317, 9318, 9319, 9320, 9323, 9324, 9325, 9326, 9327, 9329, 9339, 9340, 9351, 9356, 9424, 9425, 9426, 9427, 9428, 9429, 9430, 9431, 9432, 9434, 9447, 9448, 9449, 9450, 9451, 9452, 9453, 9454, 9455, 9457, 9475, 9476, 9477, 9478, 9479, 9480, 9481, 9482, 9483, 9485 | 72 |
| `stencil_packed` | local in transfer_ps (MSL) | Gamma color packed after conversion, as the per-bit stencil draws pack it. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 8692, 8728, 8790 | 3 |
| `stencil_packed_set` | local in transfer_ps (MSL) | stencil_packed was computed (8_8_8_8_GAMMA source). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 8693, 8730, 8790 | 3 |
| `transfer_stencil` | local in transfer_ps (MSL) | Guest stencil written with the depth. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 8789, 8792, 8795, 8956 | 4 |
| `depth_stencil_output_state` | local in PerformTransfersAndResolveClears | GetTransferDepthAndStencilOutputState() for a draw writing stencil with depth. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6557, 6559, 6562 | 3 |
| `named_candidate_count` | local in CompileToMsl | Entry point candidates before those found in the MSL source, which are only searched for when none of them resolves. | `src/xenia/gpu/metal/msl_shader.cc` | 1396, 1399 | 2 |
| `argument_encoder_begin` | local in CompileToMsl | Start of the argument encoder creation, timed for the compile timings line. | `src/xenia/gpu/metal/msl_shader.cc` | 1459, 1484 | 2 |
| `argument_encoder_ms` | local in CompileToMsl | Its time (argument-encoder= in the compile timings line). | `src/xenia/gpu/metal/msl_shader.cc` | 1483, 1501, 1510 | 3 |
| `metal_async_compile_wait_ms` | cvar (Metal); iOS settings key | How long draws may wait per frame for a compile before being skipped; 0 (default) never waits. Settings > Performance > Wait for Shader Compiles. | `src/xenia/gpu/metal/metal_command_processor.cc` | 143, 152, 915 | 3 |
| `metal_async_compile_wait_ms` | cvar (Metal); iOS settings key | How long draws may wait per frame for a compile before being skipped; 0 (default) never waits. Settings > Performance > Wait for Shader Compiles. | `src/xenia/ui/windowed_app_main_ios.mm` | 3581 | 1 |
| `queue_arrival` | MslShaderCompileRequest and MslPipelineCompileRequest field | Queue order within a priority (first in, first out). | `src/xenia/gpu/metal/metal_command_processor.h` | 566, 586, 596, 605, 625 | 7 |
| `queue_arrival` | MslShaderCompileRequest and MslPipelineCompileRequest field | Queue order within a priority (first in, first out). | `src/xenia/gpu/metal/metal_command_processor.cc` | 975, 1013 | 2 |
| `msl_compile_arrivals_` | MetalCommandProcessor member | Compiles queued so far: the next queue_arrival. | `src/xenia/gpu/metal/metal_command_processor.h` | 628 | 1 |
| `msl_compile_arrivals_` | MetalCommandProcessor member | Compiles queued so far: the next queue_arrival. | `src/xenia/gpu/metal/metal_command_processor.cc` | 975, 1013 | 2 |
| `msl_compile_completions_` | MetalCommandProcessor member | Compiles finished so far, for waiting draws to see progress. | `src/xenia/gpu/metal/metal_command_processor.h` | 629 | 1 |
| `msl_compile_completions_` | MetalCommandProcessor member | Compiles finished so far, for waiting draws to see progress. | `src/xenia/gpu/metal/metal_command_processor.cc` | 929, 935, 1187, 1247 | 4 |
| `msl_compile_done_cv_` | MetalCommandProcessor member | Notified when a compile finishes (and at shutdown). | `src/xenia/gpu/metal/metal_command_processor.h` | 626, 630 | 2 |
| `msl_compile_done_cv_` | MetalCommandProcessor member | Notified when a compile finishes (and at shutdown). | `src/xenia/gpu/metal/metal_command_processor.cc` | 859, 930, 1189, 1249 | 4 |
| `msl_compile_wait_used_us_` | MetalCommandProcessor member | Time draws waited for compiles in the current frame; reset at each swap. | `src/xenia/gpu/metal/metal_command_processor.h` | 632 | 1 |
| `msl_compile_wait_used_us_` | MetalCommandProcessor member | Time draws waited for compiles in the current frame; reset at each swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 920, 933, 940, 3005 | 4 |
| `WaitForMslCompileProgress` | MetalCommandProcessor member function | Waits for any compile to finish within the frame's remaining wait budget. | `src/xenia/gpu/metal/metal_command_processor.h` | 635 | 1 |
| `WaitForMslCompileProgress` | MetalCommandProcessor member function | Waits for any compile to finish within the frame's remaining wait budget. | `src/xenia/gpu/metal/metal_command_processor.cc` | 914, 4744, 4775, 4805 | 4 |
| `compile_waits` | MetalPerfCounters field | Draws that waited for a compile. | `src/xenia/gpu/metal/metal_perf_counters.h` | 40 | 1 |
| `compile_waits` | MetalPerfCounters field | Draws that waited for a compile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 941, 2917 | 2 |
| `compile_wait_us` | MetalPerfCounters field | Time they waited. | `src/xenia/gpu/metal/metal_perf_counters.h` | 41 | 1 |
| `compile_wait_us` | MetalPerfCounters field | Time they waited. | `src/xenia/gpu/metal/metal_command_processor.cc` | 942, 2919 | 2 |
| `wait_ms` | local in WaitForMslCompileProgress | metal_async_compile_wait_ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 915, 916, 919 | 3 |
| `budget_us` | local in WaitForMslCompileProgress | The per-frame budget in us. | `src/xenia/gpu/metal/metal_command_processor.cc` | 919, 920, 933 | 3 |
| `wait_begin` | local in WaitForMslCompileProgress | Start of the wait. | `src/xenia/gpu/metal/metal_command_processor.cc` | 923, 932, 939 | 3 |
| `completions` | local in WaitForMslCompileProgress | msl_compile_completions_ when the wait started. | `src/xenia/gpu/metal/metal_command_processor.cc` | 929, 935 | 2 |
| `waited_us` | local in WaitForMslCompileProgress | Time waited. | `src/xenia/gpu/metal/metal_command_processor.cc` | 939, 940, 942 | 3 |
| `metal_render_pass_area_from_scissor` | cvar (Metal); iOS settings key | Limit guest draw render passes to the area their scissors cover, and transfer passes to their rectangles (default off). Settings > Performance > Limit Render Pass Area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 131, 141, 3392, 3396 | 4 |
| `metal_render_pass_area_from_scissor` | cvar (Metal); iOS settings key | Limit guest draw render passes to the area their scissors cover, and transfer passes to their rectangles (default off). Settings > Performance > Limit Render Pass Area. | `src/xenia/ui/windowed_app_main_ios.mm` | 3624 | 1 |
| `render_area_needed_width_` | MetalCommandProcessor member | Right edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 463 | 1 |
| `render_area_needed_width_` | MetalCommandProcessor member | Right edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3394, 3419, 7446, 7461, 7486, 7598 | 6 |
| `render_area_needed_height_` | MetalCommandProcessor member | Bottom edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 464 | 1 |
| `render_area_needed_height_` | MetalCommandProcessor member | Bottom edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3395, 3422, 7447, 7461, 7488 | 5 |
| `current_render_area_width_` | MetalCommandProcessor member | Render area width of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 465 | 1 |
| `current_render_area_width_` | MetalCommandProcessor member | Render area width of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7314, 7445, 7446, 7596, 7667, 7668 | 6 |
| `current_render_area_height_` | MetalCommandProcessor member | Render area height of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 466 | 1 |
| `current_render_area_height_` | MetalCommandProcessor member | Render area height of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7315, 7447, 7597, 7669 | 4 |
| `render_area_by_descriptor_` | MetalCommandProcessor member | Largest render area so far per render pass descriptor, so passes stop growing. | `src/xenia/gpu/metal/metal_command_processor.h` | 468 | 1 |
| `render_area_by_descriptor_` | MetalCommandProcessor member | Largest render area so far per render pass descriptor, so passes stop growing. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3007, 7484 | 2 |
| `area_scissor` | local in IssueDraw | The draw's scissor. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3397, 3398, 3410, 3420 | 6 |
| `area_scale_x` | local in IssueDraw | Draw resolution scale (x), times the sample grid with msaa_samples_as_pixels. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3399, 3405, 3420 | 3 |
| `area_scale_y` | local in IssueDraw | Draw resolution scale (y), times the sample grid with msaa_samples_as_pixels. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3401, 3407, 3423 | 3 |
| `area_width` | local in BeginCommandBuffer | Render area width for the new encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7459, 7489, 7493, 7495, 7500, 7501, 7586, 7596, 7601, 7650 | 10 |
| `area_height` | local in BeginCommandBuffer | Render area height for the new encoder. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7460, 7491, 7494, 7496, 7502, 7597, 7605, 7650 | 8 |
| `attachments_width` | local in BeginCommandBuffer | Smallest attachment width, the most the area may be. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7462, 7472, 7473, 7483, 7490, 7493 | 6 |
| `attachments_height` | local in BeginCommandBuffer | Smallest attachment height. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7463, 7474, 7475, 7492, 7494 | 5 |
| `clears_on_load` | local in BeginCommandBuffer | An attachment is cleared on load: keep the whole area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7464, 7476, 7483 | 3 |
| `limit_to_attachment` | lambda in BeginCommandBuffer | Takes an attachment's size and load action into account. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7465, 7479, 7481, 7482 | 4 |
| `known_area` | local in BeginCommandBuffer | render_area_by_descriptor_ entry. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3007, 3008, 3009, 3010, 3011, 7484, 7485, 7486, 7487, 7488, 7490, 7491 | 16 |
| `pass_width` | local in MetalGpuPassTimer::RenderEncoder | Render area width, or the attachment's, for the traffic estimate. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 290, 293, 294, 299, 311, 325 | 6 |
| `pass_height` | local in MetalGpuPassTimer::RenderEncoder | Render area height, or the attachment's. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 291, 295, 296, 300, 311, 327 | 6 |
| `transfer_area_width` | local in PerformTransfersAndResolveClears | Render area width of the transfer pass (0: whole target), for its full-target scissor. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6086, 6137, 6485, 6488 | 4 |
| `transfer_area_height` | local in PerformTransfersAndResolveClears | Render area height of the transfer pass (0: whole target). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6087, 6138, 6489 | 3 |
| `area_right` | local in ensure_transfer_encoder | Right edge of the rectangles drawn in the transfer pass. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6102, 6111, 6131, 6133 | 5 |
| `area_bottom` | local in ensure_transfer_encoder | Bottom edge of the rectangles drawn in the transfer pass. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6103, 6112, 6131, 6135 | 5 |
| `add_to_area` | lambda in ensure_transfer_encoder | Extends the area by a (scaled) rectangle. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6104, 6125, 6129 | 3 |
| `metal_render_pass_area_from_scissor` | cvar (Metal); iOS settings key | Limit guest draw render passes to the area their scissors cover, and transfer passes to their rectangles (default off). Settings > Performance > Limit Render Pass Area. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 73, 6084, 6100 | 3 |
| `log_guest_thread_hotspots` | cvar (Logging); iOS settings key | Sample running guest threads with the Metal perf lines and log where busy ones spend their time (default on). Settings > Diagnostics > Guest Thread Hotspots. | `src/xenia/kernel/kernel_flags.cc` | 24 | 1 |
| `log_guest_thread_hotspots` | cvar (Logging); iOS settings key | Sample running guest threads with the Metal perf lines and log where busy ones spend their time (default on). Settings > Diagnostics > Guest Thread Hotspots. | `src/xenia/kernel/kernel_flags.h` | 17 | 1 |
| `log_guest_thread_hotspots` | cvar (Logging); iOS settings key | Sample running guest threads with the Metal perf lines and log where busy ones spend their time (default on). Settings > Diagnostics > Guest Thread Hotspots. | `src/xenia/kernel/util/shim_utils.cc` | 442 | 1 |
| `log_guest_thread_hotspots` | cvar (Logging); iOS settings key | Sample running guest threads with the Metal perf lines and log where busy ones spend their time (default on). Settings > Diagnostics > Guest Thread Hotspots. | `src/xenia/ui/windowed_app_main_ios.mm` | 3790 | 1 |
| `LogGuestThreadHotspots` | function, kernel shim | Samples every running guest thread 16 times, 1 ms apart, and logs where each busy one spends its time (Apple ARM64). | `src/xenia/kernel/util/shim_utils.h` | 558 | 1 |
| `LogGuestThreadHotspots` | function, kernel shim | Samples every running guest thread 16 times, 1 ms apart, and logs where each busy one spends its time (Apple ARM64). | `src/xenia/kernel/util/shim_utils.cc` | 441 | 1 |
| `LogGuestThreadHotspots` | function, kernel shim | Samples every running guest thread 16 times, 1 ms apart, and logs where each busy one spends its time (Apple ARM64). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3001 | 1 |
| `HotspotThread` | struct in LogGuestThreadHotspots | A sampled thread and where its samples were. | `src/xenia/kernel/util/shim_utils.cc` | 453, 461, 482, 504 | 4 |
| `xthread` | HotspotThread field | The guest thread (kept referenced). | `src/xenia/kernel/util/shim_utils.cc` | 454, 532 | 2 |
| `host_thread` | HotspotThread field | Its host thread, sampled. | `src/xenia/kernel/util/shim_utils.cc` | 455, 484, 532 | 3 |
| `places` | HotspotThread field | Samples per place: guest function address or host symbol. | `src/xenia/kernel/util/shim_utils.cc` | 457, 501, 507, 520 | 4 |
| `guest_samples` | HotspotThread field | Samples in guest (JIT) code. | `src/xenia/kernel/util/shim_utils.cc` | 458, 493, 506, 533 | 4 |
| `samples` | HotspotThread field | Samples taken. | `src/xenia/kernel/util/shim_utils.cc` | 459, 488, 505, 531, 533 | 5 |
| `kHotspotRounds` | constant in LogGuestThreadHotspots | Sampling rounds (16). | `src/xenia/kernel/util/shim_utils.cc` | 476, 477 | 2 |
| `hotspot_global` | local | The global critical region, try-locked for the object table. | `src/xenia/kernel/util/shim_utils.cc` | 446, 447, 452 | 3 |
| `hotspot_xthreads` | local | Every XThread. | `src/xenia/kernel/util/shim_utils.cc` | 450, 462 | 2 |
| `hotspot_threads` | local | The running guest threads being sampled. | `src/xenia/kernel/util/shim_utils.cc` | 461, 465, 469, 482, 504 | 5 |
| `hotspot_xthread` | loop variable | One XThread. | `src/xenia/kernel/util/shim_utils.cc` | 462, 463, 464, 466 | 6 |
| `hotspot_code_cache` | local | JIT code cache, to map host PCs to guest functions. | `src/xenia/kernel/util/shim_utils.cc` | 472, 491 | 2 |
| `hotspot_round` | loop variable | Sampling round. | `src/xenia/kernel/util/shim_utils.cc` | 477, 478, 479 | 4 |
| `hotspot_thread` | loop variable | A sampled thread. | `src/xenia/kernel/util/shim_utils.cc` | 482, 484, 488, 493, 501, 504, 506, 507, 520, 532, 533 | 13 |
| `hotspot_pc` | local | Its host program counter. | `src/xenia/kernel/util/shim_utils.cc` | 483, 485, 491, 496 | 4 |
| `hotspot_place` | local; structured binding | Guest function address or host symbol of a sample. | `src/xenia/kernel/util/shim_utils.cc` | 489, 492, 498, 501, 507, 508, 509, 510, 511, 512, 520, 521 | 12 |
| `hotspot_function` | local | Guest function at the PC. | `src/xenia/kernel/util/shim_utils.cc` | 490, 492 | 2 |
| `hotspot_symbol` | local | dladdr result for host code. | `src/xenia/kernel/util/shim_utils.cc` | 495, 496, 497, 498 | 4 |
| `hotspot_busy` | local | The thread had samples outside host waits. | `src/xenia/kernel/util/shim_utils.cc` | 506, 513, 516 | 3 |
| `hotspot_count` | structured binding | Samples at a place. | `src/xenia/kernel/util/shim_utils.cc` | 507, 520, 521 | 3 |
| `hotspot_sorted` | local | Places by sample count. | `src/xenia/kernel/util/shim_utils.cc` | 519, 521, 523, 525, 527, 528 | 7 |
| `hotspot_text` | local | The logged list. | `src/xenia/kernel/util/shim_utils.cc` | 524, 526, 533 | 3 |
| `metal_shader_preserve_invariance` | cvar (GPU); iOS settings key | Compile Metal shaders with preserveInvariance so [[invariant]] positions are bit-identical across vertex shaders (default on). Settings > Accuracy > Consistent Vertex Positions. | `src/xenia/gpu/metal/msl_shader.cc` | 45, 1338 | 2 |
| `metal_shader_preserve_invariance` | cvar (GPU); iOS settings key | Compile Metal shaders with preserveInvariance so [[invariant]] positions are bit-identical across vertex shaders (default on). Settings > Accuracy > Consistent Vertex Positions. | `src/xenia/ui/windowed_app_main_ios.mm` | 3645 | 1 |
| `last_update_height_used_` | RenderTargetCache member | Height in guest pixels the last Update estimated the draw uses (draw_extent_estimator_), 0 if not estimated. | `src/xenia/gpu/render_target_cache.h` | 212, 637 | 2 |
| `last_update_height_used_` | RenderTargetCache member | Height in guest pixels the last Update estimated the draw uses (draw_extent_estimator_), 0 if not estimated. | `src/xenia/gpu/render_target_cache.cc` | 599, 793 | 2 |
| `last_update_height_used` | RenderTargetCache member function | Getter for last_update_height_used_. | `src/xenia/gpu/render_target_cache.h` | 212 | 1 |
| `last_update_height_used` | RenderTargetCache member function | Getter for last_update_height_used_. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3415, 3417 | 2 |
| `area_bottom` | local in IssueDraw | Bottom of the draw's needed render area: scissor bottom, clamped to last_update_height_used(). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3410, 3416, 3423 | 4 |
| `render_area_draw_passes` | MetalPerfCounters field | Guest draw render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_perf_counters.h` | 50 | 1 |
| `render_area_draw_passes` | MetalPerfCounters field | Guest draw render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2946, 7599 | 2 |
| `render_area_draw_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_perf_counters.h` | 51 | 1 |
| `render_area_draw_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2948, 7602 | 2 |
| `render_area_draw_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_perf_counters.h` | 52 | 1 |
| `render_area_draw_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2950, 7604 | 2 |
| `render_area_restarts` | MetalPerfCounters field | Draw passes restarted to grow their render area. | `src/xenia/gpu/metal/metal_perf_counters.h` | 53 | 1 |
| `render_area_restarts` | MetalPerfCounters field | Draw passes restarted to grow their render area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2964, 7449 | 2 |
| `render_area_transfer_passes` | MetalPerfCounters field | Transfer render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_perf_counters.h` | 54 | 1 |
| `render_area_transfer_passes` | MetalPerfCounters field | Transfer render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2952 | 1 |
| `render_area_transfer_passes` | MetalPerfCounters field | Transfer render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6097 | 1 |
| `render_area_transfer_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_perf_counters.h` | 55 | 1 |
| `render_area_transfer_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2954 | 1 |
| `render_area_transfer_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6141 | 1 |
| `render_area_transfer_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_perf_counters.h` | 56 | 1 |
| `render_area_transfer_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2956 | 1 |
| `render_area_transfer_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6143 | 1 |
| `occlusion_query_events` | MetalPerfCounters field | Guest EVENT_WRITE_ZPD events (query begins and ends). | `src/xenia/gpu/metal/metal_perf_counters.h` | 59 | 1 |
| `occlusion_query_events` | MetalPerfCounters field | Guest EVENT_WRITE_ZPD events (query begins and ends). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2971, 7014 | 2 |
| `occlusion_queries` | MetalPerfCounters field | Guest occlusion query ends. | `src/xenia/gpu/metal/metal_perf_counters.h` | 60 | 1 |
| `occlusion_queries` | MetalPerfCounters field | Guest occlusion query ends. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2968, 7017 | 2 |
| `take` | lambda in IssueSwap | Reads and resets a counter. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2942, 2946, 2948, 2950, 2952, 2954, 2956, 2964, 2968, 2971, 2989, 2990, 2991, 2992, 2993 | 15 |
| `area_draw_passes` | local in IssueSwap | render_area_draw_passes for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2945, 2962 | 2 |
| `area_draw_limited` | local in IssueSwap | render_area_draw_limited for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2947, 2962, 2963 | 4 |
| `area_draw_rows` | local in IssueSwap | render_area_draw_rows for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2949, 2963 | 2 |
| `area_transfer_passes` | local in IssueSwap | render_area_transfer_passes for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2951, 2965 | 2 |
| `area_transfer_limited` | local in IssueSwap | render_area_transfer_limited for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2953, 2964, 2966 | 4 |
| `area_transfer_rows` | local in IssueSwap | render_area_transfer_rows for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2955, 2966 | 2 |
| `area_descriptor` | structured binding in IssueSwap | Descriptor key while rolling render areas over to the previous frame (unused). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3007 | 1 |
| `accuracy` | local in BuildIOSConfigSections | The Accuracy settings section. | `src/xenia/ui/windowed_app_main_ios.mm` | 3640, 3641, 3642, 3645, 3650, 3654, 3672, 3678, 3679 | 9 |
| `metal_uniform_buffers_max` | cvar (GPU) | Most SPIRV-Cross uniform buffers (8 draws each) in flight before draws wait for the GPU; 0 for the default (96 on iOS, 12 elsewhere). | `src/xenia/gpu/metal/metal_command_processor.cc` | 111, 175, 177, 178 | 4 |
| `MetalUniformBuffersMax` | function (anonymous namespace) | metal_uniform_buffers_max, or its platform default. | `src/xenia/gpu/metal/metal_command_processor.cc` | 176, 2940, 7743, 7972 | 4 |
| `uniform_waits` | MetalPerfCounters field | Draws that waited for a free SPIRV-Cross uniform buffer. | `src/xenia/gpu/metal/metal_perf_counters.h` | 44 | 1 |
| `uniform_waits` | MetalPerfCounters field | Draws that waited for a free SPIRV-Cross uniform buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2933, 7910 | 2 |
| `uniform_wait_us` | MetalPerfCounters field | Microseconds those draws waited. | `src/xenia/gpu/metal/metal_perf_counters.h` | 45 | 1 |
| `uniform_wait_us` | MetalPerfCounters field | Microseconds those draws waited. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2935, 7911 | 2 |
| `uniform_rollovers` | MetalPerfCounters field | Command buffers ended because no uniform buffer was free and the pool was at its cap. | `src/xenia/gpu/metal/metal_perf_counters.h` | 46 | 1 |
| `uniform_rollovers` | MetalPerfCounters field | Command buffers ended because no uniform buffer was free and the pool was at its cap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2938, 8018 | 2 |
| `uniform_wait_begin` | local in EnsureSpirvUniformBuffer | Start time of a wait for a free uniform buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7907, 7912 | 2 |
| `float_constants_vertex_size` | local in IssueDrawMsl | Bytes of packed float constants the vertex shader reads (float_count x 16). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5283, 5287 | 2 |
| `float_constants_pixel_size` | local in IssueDrawMsl | Bytes of packed float constants the pixel shader reads (0 without one). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5289, 5298 | 2 |
| `ProcessCompletedZPDQueries` | MetalCommandProcessor member function | occlusion_query_enable: reads the counts of guest occlusion queries whose command buffers completed, releases their slots and corrects the guess written at their end. | `src/xenia/gpu/metal/metal_command_processor.h` | 197 | 1 |
| `ProcessCompletedZPDQueries` | MetalCommandProcessor member function | occlusion_query_enable: reads the counts of guest occlusion queries whose command buffers completed, releases their slots and corrects the guess written at their end. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6751, 7053, 7135 | 3 |
| `MetalZPDQuery` | struct in MetalCommandProcessor | A guest occlusion query counted with Metal visibility results: report address, slots, guess written at its end. | `src/xenia/gpu/metal/metal_command_processor.h` | 474, 500, 503 | 3 |
| `MetalZPDQuery` | struct in MetalCommandProcessor | A guest occlusion query counted with Metal visibility results: report address, slots, guess written at its end. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7068, 7148 | 2 |
| `address` | MetalZPDQuery field | Guest report address the count goes to, 0 for an abandoned query. | `src/xenia/gpu/metal/metal_command_processor.h` | 476 | 1 |
| `address` | MetalZPDQuery field | Guest report address the count goes to, 0 for an abandoned query. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7035, 7084, 7162, 7165, 7194, 7201, 7216 | 7 |
| `submission` | MetalZPDQuery field | Command buffer of its last segment; read once it completes. | `src/xenia/gpu/metal/metal_command_processor.h` | 478 | 1 |
| `submission` | MetalZPDQuery field | Command buffer of its last segment; read once it completes. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7036, 7113, 7147 | 3 |
| `ticket` | MetalZPDQuery field | zpd_query_ends_ at its end; only the latest query of an address is corrected. | `src/xenia/gpu/metal/metal_command_processor.h` | 480 | 1 |
| `ticket` | MetalZPDQuery field | zpd_query_ends_ at its end; only the latest query of an address is corrected. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7085, 7086, 7202 | 3 |
| `guess` | MetalZPDQuery field | Count written to the guest at its end. | `src/xenia/gpu/metal/metal_command_processor.h` | 482 | 1 |
| `guess` | MetalZPDQuery field | Count written to the guest at its end. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7112, 7203, 7204, 7216 | 5 |
| `inexact` | MetalZPDQuery field | A draw was skipped for a pending compile, or found no slot or pass to count in: the guess stays. | `src/xenia/gpu/metal/metal_command_processor.h` | 490 | 1 |
| `inexact` | MetalZPDQuery field | A draw was skipped for a pending compile, or found no slot or pass to count in: the guess stays. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5998, 6010, 7073, 7076, 7092, 7150, 7435 | 7 |
| `slots` | MetalZPDQuery field | {index, generation} of the pool slot of each render pass it counted in. | `src/xenia/gpu/metal/metal_command_processor.h` | 492 | 1 |
| `slots` | MetalZPDQuery field | {index, generation} of the pool slot of each render pass it counted in. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6007, 7026, 7037, 7092, 7151 | 5 |
| `zpd_query_active_` | MetalCommandProcessor member | A guest occlusion query began and has not ended. | `src/xenia/gpu/metal/metal_command_processor.h` | 498 | 1 |
| `zpd_query_active_` | MetalCommandProcessor member | A guest occlusion query began and has not ended. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2116, 5929, 5998, 7032, 7034, 7058, 7069, 7071, 7435 | 9 |
| `zpd_segment_open_` | MetalCommandProcessor member | A pool slot is counting in the current render encoder for that query. | `src/xenia/gpu/metal/metal_command_processor.h` | 499 | 1 |
| `zpd_segment_open_` | MetalCommandProcessor member | A pool slot is counting in the current render encoder for that query. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2117, 5998, 6008, 7021, 7027, 7310 | 6 |
| `zpd_query_` | MetalCommandProcessor member | The query being counted. | `src/xenia/gpu/metal/metal_command_processor.h` | 500 | 1 |
| `zpd_query_` | MetalCommandProcessor member | The query being counted. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5998, 6007, 6010, 7026, 7035, 7036, 7037, 7038, 7057, 7070, 7435 | 11 |
| `zpd_compile_pending_at_begin_` | MetalCommandProcessor member | draw_stats_.compile_pending at the query begin; a change by its end means draws were skipped. | `src/xenia/gpu/metal/metal_command_processor.h` | 501 | 1 |
| `zpd_compile_pending_at_begin_` | MetalCommandProcessor member | draw_stats_.compile_pending at the query begin; a change by its end means draws were skipped. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7059, 7072 | 2 |
| `zpd_query_ends_` | MetalCommandProcessor member | Query ends so far, giving each its ticket. | `src/xenia/gpu/metal/metal_command_processor.h` | 479, 502 | 2 |
| `zpd_query_ends_` | MetalCommandProcessor member | Query ends so far, giving each its ticket. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7085 | 1 |
| `zpd_queries_in_flight_` | MetalCommandProcessor member | Ended (and abandoned) queries waiting for their command buffers. | `src/xenia/gpu/metal/metal_command_processor.h` | 503 | 1 |
| `zpd_queries_in_flight_` | MetalCommandProcessor member | Ended (and abandoned) queries waiting for their command buffers. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2119, 2994, 7038, 7114, 7136, 7146, 7147, 7148, 7222 | 9 |
| `zpd_results_by_address_` | MetalCommandProcessor member | Per guest report address: last count (the next guess) and the ticket of its latest query. | `src/xenia/gpu/metal/metal_command_processor.h` | 506 | 1 |
| `zpd_results_by_address_` | MetalCommandProcessor member | Per guest report address: last count (the next guess) and the ticket of its latest query. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7078, 7079, 7080, 7083, 7194, 7195 | 6 |
| `zpd_queries_measured` | MetalPerfCounters field | Query ends given a counted result. | `src/xenia/gpu/metal/metal_perf_counters.h` | 64 | 1 |
| `zpd_queries_measured` | MetalPerfCounters field | Query ends given a counted result. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2989, 7098, 7171 | 3 |
| `zpd_queries_hidden` | MetalPerfCounters field | Of those, with no samples. | `src/xenia/gpu/metal/metal_perf_counters.h` | 65 | 1 |
| `zpd_queries_hidden` | MetalPerfCounters field | Of those, with no samples. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2990, 7100, 7174 | 3 |
| `zpd_queries_corrected` | MetalPerfCounters field | Of those, written over the guess. | `src/xenia/gpu/metal/metal_perf_counters.h` | 66 | 1 |
| `zpd_queries_corrected` | MetalPerfCounters field | Of those, written over the guess. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2991, 7208 | 2 |
| `zpd_queries_guessed` | MetalPerfCounters field | Query ends left with the guess. | `src/xenia/gpu/metal/metal_perf_counters.h` | 67 | 1 |
| `zpd_queries_guessed` | MetalPerfCounters field | Query ends left with the guess. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2992, 7163 | 2 |
| `zpd_pass_restarts` | MetalPerfCounters field | Draw passes restarted to count a query in. | `src/xenia/gpu/metal/metal_perf_counters.h` | 68 | 1 |
| `zpd_pass_restarts` | MetalPerfCounters field | Draw passes restarted to count a query in. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2993, 7440 | 2 |
| `kQueryFinished` | local in MetalCommandProcessor::ExecutePacketType3_EVENT_WRITE_ZPD (as in the Vulkan override) | 0xFFFFFEED, byte-swapped: the query end marker D3D writes. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7003, 7010, 7011, 7012, 7013 | 5 |
| `sample_count_addr` | local in ExecutePacketType3_EVENT_WRITE_ZPD (as in the Vulkan override) | RB_SAMPLE_COUNT_ADDR, the guest report of the event. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7004, 7008, 7045, 7063, 7079, 7083, 7084, 7130 | 8 |
| `sample_counts` | local in ExecutePacketType3_EVENT_WRITE_ZPD and ProcessCompletedZPDQueries (as in the Vulkan override) | The guest report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7006, 7010, 7011, 7012, 7013, 7030, 7056, 7116, 7117, 7118, 7119, 7120, 7121, 7122, 7125, 7199, 7203, 7204, 7205, 7207 | 22 |
| `is_end` | local in ExecutePacketType3_EVENT_WRITE_ZPD (as in the Vulkan override) | The event ends a query. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7009, 7016, 7032, 7045, 7054 | 5 |
| `real_queries` | local in ExecutePacketType3_EVENT_WRITE_ZPD | occlusion_query_enable is on, on the SPIRV-Cross path, with the pool ready. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7029, 7032, 7041 | 3 |
| `query` | local in ExecutePacketType3_EVENT_WRITE_ZPD and ProcessCompletedZPDQueries | The query ending, or the oldest in flight. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7068, 7070, 7073, 7076, 7084, 7085, 7086, 7087, 7088, 7089, 7092, 7095, 7096, 7105, 7106, 7107, 7109, 7110, 7111, 7112, 7113, 7114, 7148, 7150, 7151, 7162, 7165, 7177, 7183, 7184, 7191, 7192, 7194, 7201, 7202, 7203, 7204, 7216, 7217 | 46 |
| `address_result` | local in ExecutePacketType3_EVENT_WRITE_ZPD and ProcessCompletedZPDQueries | zpd_results_by_address_ entry of the query address. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7082, 7086, 7094, 7105, 7194, 7195, 7196, 7202 | 8 |
| `samples` | local in ExecutePacketType3_EVENT_WRITE_ZPD and ProcessCompletedZPDQueries | Count written at the end; summed slot results. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7091, 7109, 7112, 7116, 7125, 7130, 7149, 7156, 7166, 7167, 7168, 7170 | 12 |
| `scale_area` | local in ProcessCompletedZPDQueries | Draw resolution scale area the counts are divided by. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7142, 7166, 7168 | 4 |
| `slot_samples` | local in ProcessCompletedZPDQueries | Result of one slot. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7152, 7155, 7156 | 3 |
| `counted` | local in ProcessCompletedZPDQueries | Every slot was read and the query is exact. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7150, 7158, 7162 | 3 |
| `count` | local in ProcessCompletedZPDQueries | The normalized count for the guest. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7170, 7173, 7179, 7180, 7192, 7196, 7203, 7205, 7207, 7216 | 12 |
| `slot_index` | local in IssueDrawMsl; structured binding in ProcessCompletedZPDQueries | Pool slot index. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5999, 6003, 6007, 7151, 7153, 7155, 7160 | 7 |
| `slot_generation` | local in IssueDrawMsl; structured binding in ProcessCompletedZPDQueries | Pool slot generation. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6000, 6003, 6007, 7151, 7154, 7160 | 6 |
| `slot_offset` | local in IssueDrawMsl | Byte offset of the slot in the visibility buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6001, 6004, 6006 | 3 |
| `zpd_pass` | local in BeginCommandBuffer | The draw pass must be created with the visibility buffer (a query is being counted). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7435, 7437, 7504, 7591, 7595 | 5 |
| `metal_frame_detail_log` | cvar (GPU); iOS settings key | Changed to N while a game runs: logs every render pass, draw, texture fetch constant, resolve, render target transfer and occlusion query of the next N frames. Settings > Diagnostics > Frame Detail Log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 117, 1512, 2772, 2794, 2795 | 5 |
| `metal_frame_detail_log` | cvar (GPU); iOS settings key | Changed to N while a game runs: logs every render pass, draw, texture fetch constant, resolve, render target transfer and occlusion query of the next N frames. Settings > Frame Capture > Frame Detail Log (first section). | `src/xenia/ui/windowed_app_main_ios.mm` | 3514, 3832 | 2 |
| `frame_detail_frames` | MetalPerfCounters field | Frames left to log in detail; counted down at swaps, read by the render target cache for transfers. | `src/xenia/gpu/metal/metal_perf_counters.h` | 73 | 1 |
| `frame_detail_frames` | MetalPerfCounters field | Frames left to log in detail; counted down at swaps, read by the render target cache for transfers. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2775, 2791, 2798, 5881, 6425, 7042, 7060, 7126, 7211, 7618 | 10 |
| `frame_detail_frames` | MetalPerfCounters field | Frames left to log in detail; counted down at swaps, read by the render target cache for transfers. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 5075 | 1 |
| `frame_detail_setting_` | MetalCommandProcessor member | metal_frame_detail_log when last seen; a change starts a capture. | `src/xenia/gpu/metal/metal_command_processor.h` | 662 | 1 |
| `frame_detail_setting_` | MetalCommandProcessor member | metal_frame_detail_log when last seen; a change starts a capture. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1512, 2794, 2795, 2796, 2797, 2799 | 6 |
| `frame_detail_passes_` | MetalCommandProcessor member | Draw passes so far in the logged frame (their number in the log). | `src/xenia/gpu/metal/metal_command_processor.h` | 667 | 1 |
| `frame_detail_passes_` | MetalCommandProcessor member | Draw passes so far in the logged frame (their number in the log). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2785, 2802, 5907, 7650 | 4 |
| `frame_detail_draws_` | MetalCommandProcessor member | Draws encoded so far in the logged frame (their number in the log). | `src/xenia/gpu/metal/metal_command_processor.h` | 668 | 1 |
| `frame_detail_draws_` | MetalCommandProcessor member | Draws encoded so far in the logged frame (their number in the log). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2803, 5907, 5991, 6432, 6458, 7045, 7063, 7130 | 8 |
| `frame_detail_stats_` | MetalCommandProcessor member | draw_stats_ at the frame start, for the frame's totals. | `src/xenia/gpu/metal/metal_command_processor.h` | 669 | 1 |
| `frame_detail_stats_` | MetalCommandProcessor member | draw_stats_ at the frame start, for the frame's totals. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2778, 2841 | 3 |
| `frame_start` | local in IssueSwap | frame_detail_stats_, or zeros if draw_stats_ was reset since. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2777, 2785, 2786, 2787, 2788, 2789, 2790 | 7 |
| `frame_detail_attachments` | local in BeginCommandBuffer | Host attachments of the new draw pass, for its log line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7623, 7630, 7655 | 3 |
| `add_attachment` | lambda in BeginCommandBuffer | Appends one attachment's size, format, samples and load/store actions. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7624, 7641, 7644 | 3 |
| `kColorNames` | local static in BeginCommandBuffer | Names c0-c3 of the color attachments. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7639, 7641 | 2 |
| `frame_detail_textures` | local in IssueDrawMsl | The draw's texture fetch constants, for its log line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5884, 5892, 5930 | 3 |
| `fetch_bits` | local in IssueDrawMsl | used_texture_mask fetches not yet logged. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5885, 5887, 5888 | 3 |
| `frame_detail_sources` | local in MetalRenderTargetCache::PerformTransfersAndResolveClears | A render target's transfer sources, for its log line. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 5081, 5083, 5092 | 3 |
| `metal_inline_shader_constants` | cvar (GPU); iOS settings key | SPIRV-Cross path: pass each draw's shader constants with setVertexBytes/setFragmentBytes when they change instead of through the pooled 8-draw uniform buffers (default on). Settings > Performance > Inline Shader Constants. | `src/xenia/gpu/metal/metal_command_processor.cc` | 104, 3448, 5107, 5110, 6683 | 5 |
| `metal_inline_shader_constants` | cvar (GPU); iOS settings key | SPIRV-Cross path: pass each draw's shader constants with setVertexBytes/setFragmentBytes when they change instead of through the pooled 8-draw uniform buffers (default on). Settings > Performance > Inline Shader Constants. | `src/xenia/ui/windowed_app_main_ios.mm` | 3630 | 1 |
| `inline_constants` | local in IssueDrawMsl | metal_inline_shader_constants for this draw. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5110, 5113, 5120, 5173, 5397, 5458 | 6 |
| `write_constant_block` | lambda in IssueDrawMsl | Passes one constant block with set*Bytes if its version is not bound in this encoder, or writes it to the draw's table. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5166, 5195, 5200, 5285, 5296, 5311, 5315, 5331, 5335, 5342, 5350, 5356 | 12 |
| `pixel_stage` | write_constant_block parameter | The block is for the pixel stage. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5167, 5177, 5185 | 3 |
| `slot` | write_constant_block parameter | Index in msl_inline_constants_bound_versions_. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5167, 5174, 5182 | 3 |
| `buffer_index` | write_constant_block parameter | Metal buffer index (MslBufferIndex). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5167, 5178, 5180 | 3 |
| `offset` | write_constant_block parameter | Offset of the block in the stage's table (table path). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5168, 5187, 5190 | 3 |
| `data` | write_constant_block parameter | The block's data. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5168, 5178, 5180, 5187, 5190 | 5 |
| `size` | write_constant_block parameter | The block's size in bytes; 0 skips it. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5168, 5170, 5178, 5180, 5187, 5190 | 6 |
| `version` | write_constant_block parameter | Version of the block's data. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5168, 5174, 5182, 5188 | 4 |
| `table` | local in write_constant_block | The stage's table in the uniform buffer (table path). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5185, 5187, 5190 | 3 |
| `msl_float_constants_version_vertex_` | MetalCommandProcessor member | Version of the cached packed vertex float constants, bumped when rebuilt. | `src/xenia/gpu/metal/metal_command_processor.h` | 916 | 1 |
| `msl_float_constants_version_vertex_` | MetalCommandProcessor member | Version of the cached packed vertex float constants, bumped when rebuilt. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5272, 5288 | 2 |
| `msl_float_constants_version_pixel_` | MetalCommandProcessor member | Version of the cached packed pixel float constants, bumped when rebuilt. | `src/xenia/gpu/metal/metal_command_processor.h` | 917 | 1 |
| `msl_float_constants_version_pixel_` | MetalCommandProcessor member | Version of the cached packed pixel float constants, bumped when rebuilt. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5279, 5299 | 2 |
| `msl_bool_loop_constants_version_` | MetalCommandProcessor member | Version of the cached bool/loop constants, bumped when recopied. | `src/xenia/gpu/metal/metal_command_processor.h` | 918 | 1 |
| `msl_bool_loop_constants_version_` | MetalCommandProcessor member | Version of the cached bool/loop constants, bumped when recopied. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5309, 5314, 5318 | 3 |
| `msl_fetch_constants_version_` | MetalCommandProcessor member | Version of the cached fetch constants, bumped when recopied. | `src/xenia/gpu/metal/metal_command_processor.h` | 919 | 1 |
| `msl_fetch_constants_version_` | MetalCommandProcessor member | Version of the cached fetch constants, bumped when recopied. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5329, 5333, 5337 | 3 |
| `msl_inline_constants_bound_versions_` | MetalCommandProcessor member | Per constant block slot (11), the version passed with set*Bytes in the current render encoder; 0 for none, cleared with the encoder state. | `src/xenia/gpu/metal/metal_command_processor.h` | 920 | 1 |
| `msl_inline_constants_bound_versions_` | MetalCommandProcessor member | Per constant block slot (11), the version passed with set*Bytes in the current render encoder; 0 for none, cleared with the encoder state. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5174, 5182, 5455, 7266 | 4 |
| `metal_log_shader_hashes` | cvar (GPU) | Comma-separated shader ucode hashes whose microcode disassembly and generated MSL are logged when translated ("Shader log" lines), and whose float constants the frame detail log adds per draw. Settings > Metal (generated section). | `src/xenia/gpu/metal/msl_shader.cc` | 53, 255 | 2 |
| `ShouldLogMslShader` | function (xe::gpu::metal) | metal_log_shader_hashes lists the hash. | `src/xenia/gpu/metal/msl_shader.h` | 26 | 1 |
| `ShouldLogMslShader` | function (xe::gpu::metal) | metal_log_shader_hashes lists the hash. | `src/xenia/gpu/metal/msl_shader.cc` | 254, 1210 | 2 |
| `ShouldLogMslShader` | function (xe::gpu::metal) | metal_log_shader_hashes lists the hash. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5971 | 1 |
| `log_shader_lines` | lambda in MslTranslation::CompileToMsl | Logs a text line by line with the shader's stage, hash and modification. | `src/xenia/gpu/metal/msl_shader.cc` | 1213, 1227, 1228 | 3 |
| `line_begin` | local in log_shader_lines | Start of the line being logged. | `src/xenia/gpu/metal/msl_shader.cc` | 1214, 1215, 1216, 1223, 1224 | 6 |
| `line_end` | local in log_shader_lines | End of the line being logged. | `src/xenia/gpu/metal/msl_shader.cc` | 1216, 1217, 1218, 1223, 1224 | 5 |
| `frame_detail_shader` | loop variable in IssueDrawMsl | The draw's vertex or pixel shader, for its float constants in the frame detail log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5968, 5970, 5971, 5974, 5977, 5992 | 6 |
| `is_pixel` | local in IssueDrawMsl | frame_detail_shader is the pixel shader. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5974, 5982, 5991 | 3 |
| `frame_detail_constants` | local in IssueDrawMsl | The shader's float constants for the log line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5975, 5985, 5992 | 3 |
| `MetalResolveProbe` | struct in MetalCommandProcessor | A depth resolve during a frame detail capture, sampled once its command buffer completes. | `src/xenia/gpu/metal/metal_command_processor.h` | 672, 696 | 2 |
| `MetalResolveProbe` | struct in MetalCommandProcessor | A depth resolve during a frame detail capture, sampled once its command buffer completes. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6448, 7524 | 2 |
| `resolve_probes_` | MetalCommandProcessor member | Depth resolves waiting to be sampled. | `src/xenia/gpu/metal/metal_command_processor.h` | 198, 696 | 2 |
| `resolve_probes_` | MetalCommandProcessor member | Depth resolves waiting to be sampled. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6512, 6756, 6765, 6776, 6994, 7575 | 7 |
| `LogResolveProbes` | MetalCommandProcessor member function | Logs a grid of the values of each completed probe's resolve, read from the shared memory buffer (tiled addressing). | `src/xenia/gpu/metal/metal_command_processor.h` | 200, 671 | 2 |
| `LogResolveProbes` | MetalCommandProcessor member function | Logs a grid of the values of each completed probe's resolve, read from the shared memory buffer (tiled addressing). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6752, 6755 | 2 |
| `address` | MetalResolveProbe field | RB_COPY_DEST_BASE. | `src/xenia/gpu/metal/metal_command_processor.h` | 673 | 1 |
| `address` | MetalResolveProbe field | RB_COPY_DEST_BASE. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6449, 6800, 6810, 6826, 6834, 6914, 6950, 6974 | 8 |
| `written_begin` | MetalResolveProbe field | Start of the range the resolve wrote. | `src/xenia/gpu/metal/metal_command_processor.h` | 675 | 1 |
| `written_begin` | MetalResolveProbe field | Start of the range the resolve wrote. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6450, 6785, 6800, 6812, 6836 | 5 |
| `written_end` | MetalResolveProbe field | End of the range the resolve wrote. | `src/xenia/gpu/metal/metal_command_processor.h` | 676 | 1 |
| `written_end` | MetalResolveProbe field | End of the range the resolve wrote. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6451, 6800, 6813, 6837 | 4 |
| `width` | MetalResolveProbe field | Destination pitch (RB_COPY_DEST_PITCH). | `src/xenia/gpu/metal/metal_command_processor.h` | 677 | 1 |
| `width` | MetalResolveProbe field | Destination pitch (RB_COPY_DEST_PITCH). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6452, 6766, 6793, 6801, 6808, 6811, 6835, 6896, 6897, 6898, 6899, 6925, 6930, 6955, 6961, 7529, 7532 | 18 |
| `height` | MetalResolveProbe field | Destination height (RB_COPY_DEST_PITCH). | `src/xenia/gpu/metal/metal_command_processor.h` | 678 | 1 |
| `height` | MetalResolveProbe field | Destination height (RB_COPY_DEST_PITCH). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6453, 6487, 6766, 6793, 6801, 6805, 6847, 6848, 7534, 7546 | 10 |
| `endian` | MetalResolveProbe field | Destination endianness (RB_COPY_DEST_INFO, 32-bit cases). | `src/xenia/gpu/metal/metal_command_processor.h` | 679 | 1 |
| `endian` | MetalResolveProbe field | Destination endianness (RB_COPY_DEST_INFO, 32-bit cases). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6454, 6821, 6844 | 3 |
| `is_float` | MetalResolveProbe field | The depth is D24FS8 (20e4) rather than 24-bit unorm. | `src/xenia/gpu/metal/metal_command_processor.h` | 681 | 1 |
| `is_float` | MetalResolveProbe field | The depth is D24FS8 (20e4) rather than 24-bit unorm. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6456, 6473, 6801, 6823, 6829 | 5 |
| `draw` | MetalResolveProbe field | frame_detail_draws_ at the resolve. | `src/xenia/gpu/metal/metal_command_processor.h` | 682 | 1 |
| `draw` | MetalResolveProbe field | frame_detail_draws_ at the resolve. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6458, 6800 | 2 |
| `submission` | MetalResolveProbe field | The command buffer of the resolve. | `src/xenia/gpu/metal/metal_command_processor.h` | 683 | 1 |
| `submission` | MetalResolveProbe field | The command buffer of the resolve. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6459, 6766, 6767, 7574 | 4 |
| `probe` | local in IssueCopy; iterator in LogResolveProbes | The probe being recorded or logged. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6448, 6449, 6450, 6451, 6452, 6453, 6454, 6456, 6458, 6459, 6464, 6466, 6468, 6473, 6480, 6482, 6486, 6487, 6491, 6494, 6497, 6499, 6500, 6502, 6512, 6765, 6766, 6767, 6768, 6770, 6771, 6773, 6774, 6776, 6783, 6784, 6785, 6790, 6793, 6795, 6800, 6801, 6805, 6808, 6810, 6811, 6812, 6813, 6821, 6823, 6826, 6829, 6834, 6835, 6836, 6837, 6844, 6847, 6848, 6859, 6863, 6865, 6866, 6875, 6892, 6896, 6897, 6898, 6899, 6914, 6915, 6925, 6930, 6950, 6955, 6961, 6974, 6978, 6979, 6983, 6988, 6989, 6991, 6992, 6994 | 99 |
| `dest_pitch` | local in IssueCopy | RB_COPY_DEST_PITCH of the probed resolve. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6445, 6452, 6453 | 3 |
| `dest_endian` | local in IssueCopy | RB_COPY_DEST_INFO endianness of the probed resolve. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6446, 6455 | 3 |
| `probe_cols` | local in LogResolveProbes | Grid columns (16). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6788, 6802, 6807, 6808 | 4 |
| `probe_rows` | local in LogResolveProbes | Grid rows, about square, 1 to 16. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6789, 6802, 6804, 6805 | 4 |
| `probe_line` | local in LogResolveProbes | One grid row's values for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6806, 6815, 6822, 6827 | 4 |
| `texel_address` | local in LogResolveProbes | Guest address of a sampled texel. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6809, 6812, 6813, 6814, 6819, 6833, 6836, 6837, 6838, 6842 | 10 |
| `depth24` | local in LogResolveProbes | The texel's 24-bit depth. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6821, 6823, 6824, 6832, 6844, 6849, 6850, 6884, 6889, 6901, 6903, 6908, 6935, 6936, 6939, 6941, 6962, 6963, 6965 | 19 |
| `ApplyXenosSamplePositions` | member function of MetalRenderTargetCache | Programs the Xenos MSAA sample positions (and the position-independent depth store) on a 2x/4x pass when xenos_sample_positions is on, else resets the defaults. | `src/xenia/gpu/metal/metal_render_target_cache.h` | 165 | 1 |
| `ApplyXenosSamplePositions` | member function of MetalRenderTargetCache | Programs the Xenos MSAA sample positions (and the position-independent depth store) on a 2x/4x pass when xenos_sample_positions is on, else resets the defaults. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3875, 6175 | 2 |
| `ApplyXenosSamplePositions` | member function of MetalRenderTargetCache | Programs the Xenos MSAA sample positions (and the position-independent depth store) on a 2x/4x pass when xenos_sample_positions is on, else resets the defaults. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7509 | 1 |
| `programmable_sample_positions_supported_` | member of MetalRenderTargetCache | MTLDevice areProgrammableSamplePositionsSupported, read at Initialize. | `src/xenia/gpu/metal/metal_render_target_cache.h` | 440 | 1 |
| `programmable_sample_positions_supported_` | member of MetalRenderTargetCache | MTLDevice areProgrammableSamplePositionsSupported, read at Initialize. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 734, 739, 741, 3900 | 4 |
| `use_xenos_positions` | local in ApplyXenosSamplePositions | Whether this pass gets the Xenos positions. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3899, 3905, 3909, 3911 | 4 |
| `metal_gpu_ordered_fences` | cvar (Metal) | Publish shader-done fences only once the GPU has completed the work before them (zero-copy shared memory). Default true; Settings > Accuracy. | `src/xenia/gpu/metal/metal_command_processor.cc` | 97, 1971 | 2 |
| `metal_gpu_ordered_fences` | cvar (Metal) | Publish shader-done fences only once the GPU has completed the work before them (zero-copy shared memory). Default true; Settings > Accuracy. | `src/xenia/ui/windowed_app_main_ios.mm` | 3666 | 1 |
| `AttachShaderDoneFences` | member function of MetalCommandProcessor | Moves pending_shader_done_fences_ into a completion handler of a command buffer about to be committed, which stores them in guest memory. | `src/xenia/gpu/metal/metal_command_processor.h` | 973 | 1 |
| `AttachShaderDoneFences` | member function of MetalCommandProcessor | Moves pending_shader_done_fences_ into a completion handler of a command buffer about to be committed, which stores them in guest memory. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1991, 1996, 2055, 3030, 6606, 8204 | 6 |
| `pending_shader_done_fences_` | member of MetalCommandProcessor | Shader-done fences (physical address, value) waiting for the open command buffer. | `src/xenia/gpu/metal/metal_command_processor.h` | 970 | 1 |
| `pending_shader_done_fences_` | member of MetalCommandProcessor | Shader-done fences (physical address, value) waiting for the open command buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1964, 1980, 1990, 1998, 2004, 2005 | 6 |
| `fence_command_buffer` | local in TryWriteShaderDoneFence | Empty command buffer carrying fences when none is open. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1985, 1987, 1991, 1992 | 4 |
| `fences` | lambda capture in AttachShaderDoneFences | The fences a completion handler stores. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2003, 2007, 2008 | 3 |
| `read_depth24` | lambda in LogResolveProbes | Reads one 24-bit depth texel of a probed resolve, false if not written. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6832, 6850, 6903, 6982 | 4 |
| `written_rows` | local in LogResolveProbes | Rows of the probed resolve that were written. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6847, 6851, 6892, 6894, 6895, 6896, 6899, 6921, 6925, 6933, 6953, 6959 | 12 |
| `kEdgeNames` | local constant in LogResolveProbes | Edge names for the resolve edge lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6918, 6950 | 2 |
| `edge_line` | local in LogResolveProbes | One resolve edge line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6922, 6944, 6950 | 3 |
| `depth_in` | local in LogResolveProbes | Row or column index counted from the edge (0-7). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6923, 6930, 6933, 6934 | 7 |
| `not_clear` | local in LogResolveProbes | Texels in that row or column that are not 1.0. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6926, 6940, 6945 | 3 |
| `min_depth24` | local in LogResolveProbes | Their minimum depth. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6927, 6941, 6945 | 4 |
| `block_y` | local in LogResolveProbes | Row of 8x8 blocks in the resolve map. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6953, 6958, 6959, 6975 | 6 |
| `block_x` | local in LogResolveProbes | Column of 8x8 blocks in the resolve map. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6955, 6960, 6961 | 5 |
| `map_line` | local in LogResolveProbes | One row of the resolve map. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6954, 6969, 6975 | 3 |
| `snapshot` | MetalResolveProbe field | Copy of the range the resolve wrote, made by a blit right after it (later resolves to the same address overwrite it before completion). | `src/xenia/gpu/metal/metal_command_processor.h` | 688 | 1 |
| `snapshot` | MetalResolveProbe field | Copy of the range the resolve wrote, made by a blit right after it (later resolves to the same address overwrite it before completion). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6464, 6466, 6468, 6770, 6771, 6783, 6784, 6915, 6988, 6989 | 10 |
| `edram_snapshot` | MetalResolveProbe field | Copy of the EDRAM tiles of the 1x depth surface the resolve read (the dumped host render target). | `src/xenia/gpu/metal/metal_command_processor.h` | 687, 689 | 2 |
| `edram_snapshot` | MetalResolveProbe field | Copy of the EDRAM tiles of the 1x depth surface the resolve read (the dumped host render target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6491, 6494, 6500, 6502, 6773, 6774, 6859, 6866, 6875, 6983, 6991, 6992, 7554, 7558, 7566, 7569, 7576, 7577 | 18 |
| `edram_base_tiles` | MetalResolveProbe field | RB_DEPTH_INFO depth base: the first tile in edram_snapshot. | `src/xenia/gpu/metal/metal_command_processor.h` | 687, 690 | 2 |
| `edram_base_tiles` | MetalResolveProbe field | RB_DEPTH_INFO depth base: the first tile in edram_snapshot. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6480, 6497, 6499, 7526, 7551, 7564, 7566 | 7 |
| `edram_pitch_tiles` | MetalResolveProbe field | Pitch of the depth surface in EDRAM tiles (RB_SURFACE_INFO). | `src/xenia/gpu/metal/metal_command_processor.h` | 691 | 1 |
| `edram_pitch_tiles` | MetalResolveProbe field | Pitch of the depth surface in EDRAM tiles (RB_SURFACE_INFO). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6482, 6486, 6863, 6865, 7531, 7545, 7551, 7552 | 9 |
| `snapshot_blit` | local in IssueCopy (frame detail) | Blit encoder that copies the resolved range and EDRAM tiles into the probe snapshots. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6461, 6463, 6467, 6498, 6502, 6509, 6510 | 7 |
| `edram` | local in IssueCopy (frame detail) | The render target cache EDRAM buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6471, 6473, 6499, 6502 | 4 |
| `surface_info` | local in IssueCopy (frame detail) | RB_SURFACE_INFO of the resolve (MSAA, pitch). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6472, 6474, 6483 | 3 |
| `tile_bytes` | local in IssueCopy (frame detail) | Bytes per EDRAM tile (80x16 32-bit samples). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6477, 6492, 6499, 6500, 6503, 6504 | 6 |
| `tiles` | local in IssueCopy (frame detail) | EDRAM tiles copied into edram_snapshot. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6485, 6490, 6492, 6497, 6501, 6504 | 6 |
| `first_tiles` | local in IssueCopy (frame detail) | Tiles copied before the EDRAM end wraps around to tile 0. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6496, 6500, 6501, 6503, 6504 | 5 |
| `probe_memory` | local in LogResolveProbes | Memory the probe is read from: its snapshot, else the shared memory buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6781, 6784, 6819, 6842 | 4 |
| `probe_memory_base` | local in LogResolveProbes | Guest address of the first byte of probe_memory (written_begin for a snapshot, else 0). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6782, 6785, 6819, 6842 | 4 |
| `edram_tile_bytes` | local in LogResolveProbes | Bytes per EDRAM tile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6855, 6866, 6876 | 3 |
| `read_edram` | lambda in LogResolveProbes | Reads the EDRAM dword of a depth surface pixel from edram_snapshot (tile halves swapped), false outside it. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6858, 6886, 6905 | 3 |
| `read_edram_depth24` | lambda in LogResolveProbes | The 24-bit depth of read_edram. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6884, 6980, 6984 | 3 |
| `tile` | local in read_edram | Tile of the pixel, counted from edram_base_tiles. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6862, 6866, 6876 | 3 |
| `tile_x` | local in read_edram | Sample column within the tile, halves swapped for depth. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6870, 6871, 6879 | 4 |
| `texels_line` | local in LogResolveProbes | The resolve texels log line (raw values at six border positions). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6893, 6906, 6916 | 3 |
| `positions` | local constant in LogResolveProbes | The six border positions logged. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6894, 6900 | 2 |
| `position` | loop variable in LogResolveProbes | One of positions. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6900, 6903, 6905, 6907 | 7 |
| `edram_texel` | local in LogResolveProbes | The EDRAM dword at the position. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6901, 6905, 6909 | 3 |
| `in_resolve` | local in LogResolveProbes | The resolve wrote the position. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6902, 6908 | 2 |
| `in_edram` | local in LogResolveProbes | edram_snapshot holds the position. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6904, 6909 | 2 |
| `log_coverage` | lambda in LogResolveProbes | Logs the edge and 8x8 map lines for the resolved texels or the EDRAM. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6920, 6979, 6982, 6984 | 4 |
| `source` | lambda parameter of log_coverage | "resolve" or "edram", in the log lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6920, 6950, 6974 | 3 |
| `read` | lambda parameter of log_coverage | read_depth24 or read_edram_depth24. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6920, 6936, 6963 | 3 |
| `pass` | MetalResolveProbe field | Not 0: an EDRAM probe of a 24-bit unorm depth target at the start of this frame detail pass, not a resolve (width and height in EDRAM samples). | `src/xenia/gpu/metal/metal_command_processor.h` | 692, 694 | 2 |
| `pass` | MetalResolveProbe field | Not 0: an EDRAM probe of a 24-bit unorm depth target at the start of this frame detail pass, not a resolve (width and height in EDRAM samples). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6790, 6795, 6847, 6848, 6892, 6978, 6979, 7525 | 8 |
| `depth_info` | local in BeginCommandBuffer (frame detail) | RB_DEPTH_INFO of the pass (24-bit unorm only, base tiles). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7521, 7523, 7526 | 3 |
| `surface_info` | local in BeginCommandBuffer (frame detail) | RB_SURFACE_INFO of the pass (pitch, MSAA). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7522, 7529, 7530, 7539 | 4 |
| `probe` | local in BeginCommandBuffer (frame detail) | The pass-start EDRAM probe being recorded. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7524, 7525, 7526, 7529, 7531, 7532, 7534, 7545, 7546, 7551, 7552, 7554, 7558, 7564, 7566, 7569, 7574, 7575, 7576, 7577 | 23 |
| `tile_bytes` | local in BeginCommandBuffer (frame detail) | Bytes per EDRAM tile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7541, 7555, 7566, 7567, 7570, 7571 | 6 |
| `tiles` | local in BeginCommandBuffer (frame detail) | EDRAM tiles dumped and copied. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7544, 7549, 7552, 7555, 7564, 7568, 7571 | 7 |
| `snapshot_blit` | local in BeginCommandBuffer (frame detail) | Blit encoder copying the dumped EDRAM tiles. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7557, 7560, 7565, 7569, 7573 | 5 |
| `edram` | local in BeginCommandBuffer (frame detail) | The render target cache EDRAM buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7561, 7566, 7569 | 3 |
| `first_tiles` | local in BeginCommandBuffer (frame detail) | Tiles copied before the EDRAM end wraps around. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7563, 7567, 7568, 7570, 7571 | 5 |
| `KernelCallReadable` | function in xe::kernel::shim (declaration) | Whether a guest memory range is committed and readable. | `src/xenia/kernel/util/shim_utils.h` | 561 | 1 |
| `LogKernelCallStack` | function in xe::kernel::shim (declaration) | Guest return addresses up the stack from a frame. | `src/xenia/kernel/util/shim_utils.h` | 564 | 1 |
| `KernelCallReadable` | function in xe::kernel::shim | Checks the guest code around the crash PC is readable. | `src/xenia/emulator.cc` | 1794 | 1 |
| `LogKernelCallStack` | function in xe::kernel::shim | Guest stack of the crashed thread. | `src/xenia/emulator.cc` | 1790 | 1 |
| `crash_global` | local in Emulator::ExceptionCallback (crash dump) | The global critical region (try-locked; the page table and the stack walk need it). | `src/xenia/emulator.cc` | 1755, 1756, 1838 | 3 |
| `crash_locked` | local in Emulator::ExceptionCallback (crash dump) | Whether crash_global was taken. | `src/xenia/emulator.cc` | 1756, 1767, 1787 | 3 |
| `crash_fault` | local in Emulator::ExceptionCallback (crash dump) | Host fault address. | `src/xenia/emulator.cc` | 1758, 1761, 1762, 1763 | 4 |
| `crash_virtual` | local in Emulator::ExceptionCallback (crash dump) | Host address of guest virtual address 0. | `src/xenia/emulator.cc` | 1759, 1761, 1762, 1763, 1783 | 5 |
| `crash_guest` | local in Emulator::ExceptionCallback (crash dump) | Guest address of the fault. | `src/xenia/emulator.cc` | 1763, 1764, 1765, 1768 | 4 |
| `crash_heap` | local in Emulator::ExceptionCallback (crash dump) | Heap holding crash_guest. | `src/xenia/emulator.cc` | 1765, 1767, 1768, 1772, 1775 | 5 |
| `crash_info` | local in Emulator::ExceptionCallback (crash dump) | Region and allocation of crash_guest (state, protection). | `src/xenia/emulator.cc` | 1766, 1768, 1772, 1773, 1774 | 8 |
| `crash_pc` | local in Emulator::ExceptionCallback (crash dump) | Guest PC of the fault, for the code words around it. | `src/xenia/emulator.cc` | 1791, 1793, 1794, 1795, 1799 | 5 |
| `kernel_call_wait_objects` | local in LogKernelCallThreads | The handles of a NtWaitForMultipleObjectsEx with their object type and name. | `src/xenia/kernel/util/shim_utils.cc` | 274, 285, 289, 299, 303, 307 | 6 |
| `kernel_call_handles` | local in LogKernelCallThreads | Guest address of the handle array (r4). | `src/xenia/kernel/util/shim_utils.cc` | 275, 276, 280 | 3 |
| `kernel_call_wait_handle` | local in LogKernelCallThreads | One handle of the array. | `src/xenia/kernel/util/shim_utils.cc` | 278, 284, 286 | 3 |
| `kernel_call_wait_object` | local in LogKernelCallThreads | The object of that handle. | `src/xenia/kernel/util/shim_utils.cc` | 282, 287, 291, 292, 293, 295 | 6 |
| `frame_detail_bindings` | local in IssueDraw (frame detail) | Vertex bindings of the draw's vertex shader, for the small-draw vertex dump. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5933, 5936, 5937 | 3 |
| `vertex_words` | local in IssueDraw (frame detail) | Words of vertex data dumped (at most 48). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5941, 5947, 5950 | 3 |
| `frame_detail_vertices` | local in IssueDraw (frame detail) | The dumped vertex data as text. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5946, 5954, 5965 | 3 |
| `word_float` | local in IssueDraw (frame detail) | One vertex word as a float. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5952, 5953, 5958 | 3 |
| `OnlyFrameDetailLogChanged` | function | Whether edited settings differ from the stored ones only in metal_frame_detail_log (then saving closes Settings without the alert). | `src/xenia/ui/windowed_app_main_ios.mm` | 3810, 8424 | 2 |
| `frame_capture` | local in BuildIOSConfigSections | The Frame Capture section, first in Settings. | `src/xenia/ui/windowed_app_main_ios.mm` | 3509, 3510, 3511, 3514, 3531, 3532 | 6 |
| `frame_detail_changed` | local in OnlyFrameDetailLogChanged | Whether the Frame Detail Log differs. | `src/xenia/ui/windowed_app_main_ios.mm` | 3815, 3836, 3839 | 3 |
| `only_frame_detail_log` | local in saveTapped: | Result of OnlyFrameDetailLogChanged before the settings are applied. | `src/xenia/ui/windowed_app_main_ios.mm` | 8424, 8428 | 2 |
| `skip_unchanged_render_target_transfers` | cvar; iOS settings key | Skip transfers back to a render target whose copy of the EDRAM range is unchanged (default off). Settings > Performance > Skip Unchanged Render Target Copies. | `src/xenia/gpu/render_target_cache.cc` | 38, 1668 | 2 |
| `skip_unchanged_render_target_transfers` | cvar; iOS settings key | Skip transfers back to a render target whose copy of the EDRAM range is unchanged (default off). Settings > Performance > Skip Unchanged Render Target Copies. | `src/xenia/gpu/metal/metal_command_processor.cc` | 95, 2981 | 2 |
| `skip_unchanged_render_target_transfers` | cvar; iOS settings key | Skip transfers back to a render target whose copy of the EDRAM range is unchanged (default off). Settings > Performance > Skip Unchanged Render Target Copies. | `src/xenia/emulator.cc` | 122, 2256 | 2 |
| `skip_unchanged_render_target_transfers` | cvar; iOS settings key | Skip transfers back to a render target whose copy of the EDRAM range is unchanged (default off). Settings > Performance > Skip Unchanged Render Target Copies. | `src/xenia/ui/windowed_app_main_ios.mm` | 3597 | 1 |
| `TakeUnchangedTransferCounts` | member function of RenderTargetCache | Returns and resets unchanged_transfers_ and skipped_transfers_ for the Metal perf log. | `src/xenia/gpu/render_target_cache.h` | 229 | 1 |
| `TakeUnchangedTransferCounts` | member function of RenderTargetCache | Returns and resets unchanged_transfers_ and skipped_transfers_ for the Metal perf log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2974 | 1 |
| `unchanged_out` | parameter of TakeUnchangedTransferCounts | Receives unchanged_transfers_. | `src/xenia/gpu/render_target_cache.h` | 229, 231 | 2 |
| `skipped_out` | parameter of TakeUnchangedTransferCounts | Receives skipped_transfers_. | `src/xenia/gpu/render_target_cache.h` | 230, 232 | 2 |
| `unchanged_transfers_` | member of RenderTargetCache | Ownership ranges given back to a render target whose copy was unchanged. | `src/xenia/gpu/render_target_cache.h` | 231, 233, 765 | 3 |
| `unchanged_transfers_` | member of RenderTargetCache | Ownership ranges given back to a render target whose copy was unchanged. | `src/xenia/gpu/render_target_cache.cc` | 1667 | 1 |
| `skipped_transfers_` | member of RenderTargetCache | How many of those skip_unchanged_render_target_transfers skipped. | `src/xenia/gpu/render_target_cache.h` | 232, 234, 766 | 3 |
| `skipped_transfers_` | member of RenderTargetCache | How many of those skip_unchanged_render_target_transfers skipped. | `src/xenia/gpu/render_target_cache.cc` | 1669 | 1 |
| `intact_render_target` | member of RenderTargetCache::OwnershipRange | A render target other than the owner whose copy of the range still holds the current contents. | `src/xenia/gpu/render_target_cache.h` | 673, 718 | 3 |
| `intact_render_target` | member of RenderTargetCache::OwnershipRange | A render target other than the owner whose copy of the range still holds the current contents. | `src/xenia/gpu/render_target_cache.cc` | 568, 569, 1622, 1634, 1635, 1665, 1735 | 7 |
| `dest_written` | parameter of ChangeOwnership | The destination will be written in the range (draw writing depth, stencil or color; resolve clear), so no other copy stays intact. | `src/xenia/gpu/render_target_cache.h` | 750 | 1 |
| `dest_written` | parameter of ChangeOwnership | The destination will be written in the range (draw writing depth, stencil or color; resolve clear), so no other copy stays intact. | `src/xenia/gpu/render_target_cache.cc` | 1587, 1619, 1634, 1735 | 4 |
| `depth_written` | local in Update | The draw writes depth or stencil. | `src/xenia/gpu/render_target_cache.cc` | 912, 914, 921, 942 | 4 |
| `writes_stencil` | lambda in Update | Whether one face's stencil state can write stencil. | `src/xenia/gpu/render_target_cache.cc` | 915, 922, 927 | 3 |
| `fail` | parameter of writes_stencil | Stencil op on stencil test failure. | `src/xenia/gpu/render_target_cache.cc` | 915, 917 | 2 |
| `zpass` | parameter of writes_stencil | Stencil op when both tests pass. | `src/xenia/gpu/render_target_cache.cc` | 916, 918 | 2 |
| `zfail` | parameter of writes_stencil | Stencil op on depth test failure. | `src/xenia/gpu/render_target_cache.cc` | 916, 919 | 2 |
| `dest_intact` | local in ChangeOwnership | The destination's copy of the range is unchanged since it last owned it. | `src/xenia/gpu/render_target_cache.cc` | 1663, 1666 | 2 |
| `previous_owner_matches` | local in ChangeOwnership | The previous owner's copy will match the destination's after this change (transferred, or skipped as unchanged). | `src/xenia/gpu/render_target_cache.cc` | 1657, 1670, 1674, 1701, 1722, 1735 | 6 |
| `previous_owner` | local in ChangeOwnership | Owner of the range before the claim. | `src/xenia/gpu/render_target_cache.cc` | 1730, 1736 | 2 |
| `unchanged_transfers` | local in the Metal perf log | Value from TakeUnchangedTransferCounts. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2973, 2974, 2980 | 3 |
| `skipped_transfers` | local in the Metal perf log | Value from TakeUnchangedTransferCounts. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2973, 2975, 2980 | 3 |
| `UserApcPendingForHostWait` | function (xobject.cc) | Whether an alertable host wait should end at once for a pending guest user APC (POSIX host waits are not woken by one). | `src/xenia/kernel/xobject.cc` | 42, 442, 450, 680, 688, 722, 730 | 7 |
| `kAlertableWaitPollIntervalIOS` | constant (iOS only) | Slice of an alertable host wait on iOS, so an APC queued by another thread is seen within 10 ms. | `src/xenia/kernel/xobject.cc` | 59, 138 | 2 |
| `slice` | local in WaitWithTitleStopPollIOS (iOS only) | The next wait slice, capped at kAlertableWaitPollIntervalIOS for alertable waits. | `src/xenia/kernel/xobject.cc` | 136, 138, 140 | 4 |
| `typed_utf8` | local in the keyboard prompt callback | UTF-8 of the typed text (nil-safe) before the copy is released. | `src/xenia/ui/windowed_app_main_ios.mm` | 12012, 12013 | 3 |
| `ordinal` | member of MetalCommandProcessor::MetalZPDQuery | The query's place among the frame's query ends. | `src/xenia/gpu/metal/metal_command_processor.h` | 486 | 1 |
| `ordinal` | member of MetalCommandProcessor::MetalZPDQuery | The query's place among the frame's query ends. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7087, 7088, 7089, 7095, 7096, 7106, 7107, 7191, 7192, 7216 | 11 |
| `address_guess` | member of MetalCommandProcessor::MetalZPDQuery | What the last count at its address predicted. | `src/xenia/gpu/metal/metal_command_processor.h` | 487 | 1 |
| `address_guess` | member of MetalCommandProcessor::MetalZPDQuery | What the last count at its address predicted. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7105, 7111, 7184, 7217 | 4 |
| `ordinal_guess` | member of MetalCommandProcessor::MetalZPDQuery | What the last count at its place in the frame predicted (UINT32_MAX: none). | `src/xenia/gpu/metal/metal_command_processor.h` | 488 | 1 |
| `ordinal_guess` | member of MetalCommandProcessor::MetalZPDQuery | What the last count at its place in the frame predicted (UINT32_MAX: none). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7107, 7109, 7110, 7177, 7183, 7217 | 6 |
| `zpd_frame_query_ends_` | member of MetalCommandProcessor | Query ends since the last swap. | `src/xenia/gpu/metal/metal_command_processor.h` | 513 | 1 |
| `zpd_frame_query_ends_` | member of MetalCommandProcessor | Query ends since the last swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2804, 7087 | 2 |
| `zpd_results_by_ordinal_` | member of MetalCommandProcessor | Last count per place in the frame (UINT32_MAX: none). | `src/xenia/gpu/metal/metal_command_processor.h` | 514 | 1 |
| `zpd_results_by_ordinal_` | member of MetalCommandProcessor | Last count per place in the frame (UINT32_MAX: none). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7088, 7089, 7095, 7096, 7106, 7107, 7191, 7192 | 8 |
| `zpd_ordinal_score_` | member of MetalCommandProcessor | Which guess has been closer lately, -8...8 (above 0: by place in the frame). | `src/xenia/gpu/metal/metal_command_processor.h` | 515 | 1 |
| `zpd_ordinal_score_` | member of MetalCommandProcessor | Which guess has been closer lately, -8...8 (above 0: by place in the frame). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2995, 2996, 7109, 7186, 7188, 7218 | 8 |
| `miss` | lambda in ProcessCompletedZPDQueries | Distance of a guess from the count. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7179, 7183, 7184 | 3 |
| `ordinal_miss` | local in ProcessCompletedZPDQueries | How far the guess by place was. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7182, 7185, 7187 | 3 |
| `address_miss` | local in ProcessCompletedZPDQueries | How far the guess by address was. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7184, 7185, 7187 | 3 |
| `signin_state_logged` | function-local static in XamUserGetSigninState_entry | The value last logged per user, to log only changes. | `src/xenia/kernel/xam/xam_user.cc` | 104, 106, 107 | 3 |
| `metal_gpu_trace_frames` | cvar (Metal); iOS settings key | Changed to N during a game: records the GPU commands of the next N frames into a trace under trace_gpu_prefix. Settings > Frame Capture > GPU Trace. | `src/xenia/gpu/metal/metal_command_processor.cc` | 124, 2819, 2820 | 3 |
| `metal_gpu_trace_frames` | cvar (Metal); iOS settings key | Changed to N during a game: records the GPU commands of the next N frames into a trace under trace_gpu_prefix. Settings > Frame Capture > GPU Trace. | `src/xenia/ui/windowed_app_main_ios.mm` | 3523, 3833 | 2 |
| `gpu_trace_setting_` | member of MetalCommandProcessor | metal_gpu_trace_frames when last seen (a change starts a recording). | `src/xenia/gpu/metal/metal_command_processor.h` | 665 | 1 |
| `gpu_trace_setting_` | member of MetalCommandProcessor | metal_gpu_trace_frames when last seen (a change starts a recording). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2819, 2820, 2821, 2836 | 4 |
| `gpu_trace_frames_left_` | member of MetalCommandProcessor | Frames of the current recording still to write. | `src/xenia/gpu/metal/metal_command_processor.h` | 666 | 1 |
| `gpu_trace_frames_left_` | member of MetalCommandProcessor | Frames of the current recording still to write. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2808, 2810, 2816, 2821, 2836, 2838 | 6 |
| `trace_path` | local in IssueSwap | Folder of a new GPU trace (storage root / trace_gpu_prefix / date and time). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2823, 2824, 2826, 2834, 2835, 2838 | 7 |
| `trace_time` | local in IssueSwap | Time the recording was requested. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2828, 2830 | 2 |
| `trace_local_time` | local in IssueSwap | trace_time in the device time zone. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2829, 2830, 2833 | 3 |
| `trace_folder` | local in IssueSwap | Folder name from the date and time. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2831, 2832, 2834 | 4 |
| `LogKernelCallStackCode` | function in xe::kernel::shim | Logs the guest code around the caller and each return address of a stuck thread (first storm report). | `src/xenia/kernel/util/shim_utils.h` | 567 | 1 |
| `LogKernelCallStackCode` | function in xe::kernel::shim | Logs the guest code around the caller and each return address of a stuck thread (first storm report). | `src/xenia/kernel/util/shim_utils.cc` | 130, 624 | 2 |
| `kernel_call_returns` | local in LogKernelCallStackCode | The caller and the return addresses up the stack. | `src/xenia/kernel/util/shim_utils.cc` | 139, 153, 160 | 3 |
| `kernel_call_return` | local in LogKernelCallStackCode | One of them. | `src/xenia/kernel/util/shim_utils.cc` | 160, 161, 162, 163, 178 | 6 |
| `kernel_call_code_logged` | function-local static in LogKernelCallStackCode | Return addresses already dumped this run. | `src/xenia/kernel/util/shim_utils.cc` | 159, 163 | 2 |
| `zero_commit` | local in NtAllocateVirtualMemory_entry | The call commits without X_MEM_NOZERO. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 230, 248, 280 | 3 |
| `zero_runs` | local in NtAllocateVirtualMemory_entry | Ranges this call newly commits, to zero. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 239, 260, 261, 262, 264, 292, 301 | 8 |
| `run_address` | local in NtAllocateVirtualMemory_entry | Start of the region being checked. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 250, 254, 256, 261, 262, 264, 267 | 9 |
| `run_info` | local in NtAllocateVirtualMemory_entry | QueryRegionInfo of it. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 251, 254, 255, 256, 257 | 5 |
| `run_end` | local in NtAllocateVirtualMemory_entry | End of that region within the range. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 252, 256, 262, 264, 267 | 5 |
| `run_committed` | local in NtAllocateVirtualMemory_entry | Whether that region was already committed. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 253, 257, 259 | 3 |
| `zero_address` | structured binding in NtAllocateVirtualMemory_entry | Start of a range to zero. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 301, 304, 311, 314 | 4 |
| `zero_size` | structured binding in NtAllocateVirtualMemory_entry | Its size. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 301, 304, 311, 314 | 4 |
| `VirtualMemoryEvent` | struct (xboxkrnl_memory.cc) | One recorded NtAllocateVirtualMemory or NtFreeVirtualMemory call: thread, address, size, type, free. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 75, 83, 109 | 3 |
| `virtual_memory_history_mutex` | static (xboxkrnl_memory.cc) | Guards the history. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 82, 90, 99 | 3 |
| `virtual_memory_history` | static (xboxkrnl_memory.cc) | The last 1024 calls. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 83, 91, 92, 106, 110 | 6 |
| `virtual_memory_history_next` | static (xboxkrnl_memory.cc) | Calls recorded so far. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 84, 91, 106, 107, 108, 113 | 6 |
| `RecordVirtualMemoryEvent` | function (xboxkrnl_memory.cc) | Records a call. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 86, 320, 421 | 3 |
| `DescribeVirtualMemoryHistory` | function in xe::kernel::xboxkrnl | The recorded calls whose range holds an address, for the crash dump. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.h` | 63 | 1 |
| `DescribeVirtualMemoryHistory` | function in xe::kernel::xboxkrnl | The recorded calls whose range holds an address, for the crash dump. | `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc` | 98 | 1 |
| `DescribeVirtualMemoryHistory` | function in xe::kernel::xboxkrnl | The recorded calls whose range holds an address, for the crash dump. | `src/xenia/emulator.cc` | 1836 | 1 |
| `crash_reg` | local in Emulator::ExceptionCallback (crash dump) | Register index. | `src/xenia/emulator.cc` | 1806, 1807, 1822 | 5 |
| `crash_pointer` | local in Emulator::ExceptionCallback (crash dump) | The register as a guest address. | `src/xenia/emulator.cc` | 1807, 1808, 1811, 1814, 1822, 1825, 1836 | 8 |
| `crash_pointer_heap` | local in Emulator::ExceptionCallback (crash dump) | Heap holding it. | `src/xenia/emulator.cc` | 1811, 1813, 1814 | 3 |
| `crash_pointer_info` | local in Emulator::ExceptionCallback (crash dump) | Its allocation. | `src/xenia/emulator.cc` | 1812, 1815, 1816, 1822, 1823, 1824 | 7 |
| `crash_bytes` | local in Emulator::ExceptionCallback (crash dump) | Start of the 32 bytes dumped around it. | `src/xenia/emulator.cc` | 1825, 1826, 1831 | 3 |
| `signed_in_mask` | local in KernelState::RegisterNotifyListener | Users signed in, for the startup XN_SYS_SIGNINCHANGED. | `src/xenia/kernel/kernel_state.cc` | 1219, 1223, 1227 | 3 |
| `msaa_samples_as_pixels` | cvar; iOS settings key | Keep MSAA render targets as 1x render targets of their samples, drawing to them scaled by the sample grid (default off). Settings > Performance > MSAA Samples as Pixels. | `src/xenia/gpu/render_target_cache.cc` | 48, 617, 1234 | 3 |
| `msaa_samples_as_pixels` | cvar; iOS settings key | Keep MSAA render targets as 1x render targets of their samples, drawing to them scaled by the sample grid (default off). Settings > Performance > MSAA Samples as Pixels. | `src/xenia/gpu/render_target_cache.h` | 36 | 1 |
| `msaa_samples_as_pixels` | cvar; iOS settings key | Keep MSAA render targets as 1x render targets of their samples, drawing to them scaled by the sample grid (default off). Settings > Performance > MSAA Samples as Pixels. | `src/xenia/emulator.cc` | 123, 2257 | 2 |
| `msaa_samples_as_pixels` | cvar; iOS settings key | Keep MSAA render targets as 1x render targets of their samples, drawing to them scaled by the sample grid (default off). Settings > Performance > MSAA Samples as Pixels. | `src/xenia/ui/windowed_app_main_ios.mm` | 3606 | 1 |
| `last_update_samples_as_pixels_` | member of RenderTargetCache | Guest MSAA of the surface the last Update bound as 1x render targets of its samples, or k1X. | `src/xenia/gpu/render_target_cache.h` | 219, 222, 640 | 3 |
| `last_update_samples_as_pixels_` | member of RenderTargetCache | Guest MSAA of the surface the last Update bound as 1x render targets of its samples, or k1X. | `src/xenia/gpu/render_target_cache.cc` | 616, 620 | 2 |
| `last_update_samples_as_pixels_x_log2` | member function of RenderTargetCache | Horizontal draw scale (log2) on top of the resolution scale for msaa_samples_as_pixels: 1 at 4x MSAA. | `src/xenia/gpu/render_target_cache.h` | 218 | 1 |
| `last_update_samples_as_pixels_x_log2` | member function of RenderTargetCache | Horizontal draw scale (log2) on top of the resolution scale for msaa_samples_as_pixels: 1 at 4x MSAA. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3406, 4982, 8411, 11522, 11617 | 5 |
| `last_update_samples_as_pixels_y_log2` | member function of RenderTargetCache | Vertical draw scale (log2) on top of the resolution scale for msaa_samples_as_pixels: 1 at 2x and 4x MSAA. | `src/xenia/gpu/render_target_cache.h` | 221 | 1 |
| `last_update_samples_as_pixels_y_log2` | member function of RenderTargetCache | Vertical draw scale (log2) on top of the resolution scale for msaa_samples_as_pixels: 1 at 2x and 4x MSAA. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3408, 4979, 4984, 7540, 8413, 11525, 11619 | 7 |
| `SupportsMsaaSamplesAsPixels` | virtual member function of RenderTargetCache | Whether the implementation scales draws by the sample grid (Metal: the SPIRV-Cross path). | `src/xenia/gpu/render_target_cache.h` | 255 | 1 |
| `SupportsMsaaSamplesAsPixels` | virtual member function of RenderTargetCache | Whether the implementation scales draws by the sample grid (Metal: the SPIRV-Cross path). | `src/xenia/gpu/render_target_cache.cc` | 618, 1234 | 2 |
| `SupportsMsaaSamplesAsPixels` | virtual member function of RenderTargetCache | Whether the implementation scales draws by the sample grid (Metal: the SPIRV-Cross path). | `src/xenia/gpu/metal/metal_render_target_cache.h` | 140 | 1 |
| `SupportsMsaaSamplesAsPixels` | virtual member function of RenderTargetCache | Whether the implementation scales draws by the sample grid (Metal: the SPIRV-Cross path). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3175 | 1 |
| `storage_msaa_samples` | local in Update and PrepareHostRenderTargetsResolveClear | MSAA of the host render target keys: k1X with msaa_samples_as_pixels, else the guest MSAA. | `src/xenia/gpu/render_target_cache.cc` | 615, 619, 771, 861, 985, 1233, 1363, 1379, 1395 | 9 |
| `samples_as_pixels` | local in IssueDrawMsl | The draw goes to render targets keeping MSAA samples as pixels; its scale and divisors include the sample grid. | `src/xenia/gpu/metal/metal_command_processor.cc` | 4977, 4980, 4989, 4992 | 4 |
| `kSysFlag_SamplesAsPixelsX_Shift` | enumerator of SpirvShaderTranslator | Bit of kSysFlag_SamplesAsPixelsX. | `src/xenia/gpu/spirv_shader_translator.h` | 157, 199 | 2 |
| `kSysFlag_SamplesAsPixelsY_Shift` | enumerator of SpirvShaderTranslator | Bit of kSysFlag_SamplesAsPixelsY. | `src/xenia/gpu/spirv_shader_translator.h` | 158, 200 | 2 |
| `kSysFlag_SamplesAsPixelsX` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel horizontally (msaa_samples_as_pixels at 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/spirv_shader_translator.h` | 199 | 1 |
| `kSysFlag_SamplesAsPixelsX` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel horizontally (msaa_samples_as_pixels at 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/spirv_shader_translator.cc` | 3429 | 1 |
| `kSysFlag_SamplesAsPixelsX` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel horizontally (msaa_samples_as_pixels at 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/metal/metal_command_processor.cc` | 11523 | 1 |
| `kSysFlag_SamplesAsPixelsY` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel vertically (msaa_samples_as_pixels at 2x and 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/spirv_shader_translator.h` | 200 | 1 |
| `kSysFlag_SamplesAsPixelsY` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel vertically (msaa_samples_as_pixels at 2x and 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/spirv_shader_translator.cc` | 3484 | 1 |
| `kSysFlag_SamplesAsPixelsY` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel vertically (msaa_samples_as_pixels at 2x and 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/metal/metal_command_processor.cc` | 11526 | 1 |
| `DescribeLastSignal` | member function of XEvent | Who last set or pulsed the event (guest thread, return address, how long ago, how many times), for the stuck thread report. | `src/xenia/kernel/xevent.h` | 47 | 1 |
| `DescribeLastSignal` | member function of XEvent | Who last set or pulsed the event (guest thread, return address, how long ago, how many times), for the stuck thread report. | `src/xenia/kernel/xevent.cc` | 78 | 1 |
| `DescribeLastSignal` | member function of XEvent | Who last set or pulsed the event (guest thread, return address, how long ago, how many times), for the stuck thread report. | `src/xenia/kernel/util/shim_utils.cc` | 258, 296 | 2 |
| `NoteSignal` | private member function of XEvent | Records the caller of Set and Pulse for DescribeLastSignal. | `src/xenia/kernel/xevent.h` | 57 | 1 |
| `NoteSignal` | private member function of XEvent | Records the caller of Set and Pulse for DescribeLastSignal. | `src/xenia/kernel/xevent.cc` | 64, 92, 98 | 3 |
| `last_signal_thread_` | member of XEvent | Handle of the guest thread that last set or pulsed the event (0 for a host thread). | `src/xenia/kernel/xevent.h` | 61 | 1 |
| `last_signal_thread_` | member of XEvent | Handle of the guest thread that last set or pulsed the event (0 for a host thread). | `src/xenia/kernel/xevent.cc` | 68, 85 | 2 |
| `last_signal_lr_` | member of XEvent | Guest link register of that call. | `src/xenia/kernel/xevent.h` | 62 | 1 |
| `last_signal_lr_` | member of XEvent | Guest link register of that call. | `src/xenia/kernel/xevent.cc` | 70, 86 | 2 |
| `last_signal_ms_` | member of XEvent | Host uptime in milliseconds of that call. | `src/xenia/kernel/xevent.h` | 63 | 1 |
| `last_signal_ms_` | member of XEvent | Host uptime in milliseconds of that call. | `src/xenia/kernel/xevent.cc` | 73, 88 | 2 |
| `signal_count_` | member of XEvent | How many times the event was set or pulsed. | `src/xenia/kernel/xevent.h` | 64 | 1 |
| `signal_count_` | member of XEvent | How many times the event was set or pulsed. | `src/xenia/kernel/xevent.cc` | 75, 79 | 2 |
| `signal_thread` | local in NoteSignal | The calling guest thread, if any. | `src/xenia/kernel/xevent.cc` | 65, 68 | 3 |
| `signal_count` | local in DescribeLastSignal | signal_count_ as read once. | `src/xenia/kernel/xevent.cc` | 79, 80, 85 | 3 |
| `shown_sections_` | ivar of XeniaConfigViewController | Sections the settings search shows, as indices into sections_. | `src/xenia/ui/windowed_app_main_ios.mm` | 7868, 8023, 8044, 8211, 8218, 8222, 8226, 8233, 8236, 8240, 8243 | 11 |
| `shown_items_` | ivar of XeniaConfigViewController | Per shown section, the items the settings search shows, as indices into its items. | `src/xenia/ui/windowed_app_main_ios.mm` | 7869, 8024, 8045, 8214, 8229 | 5 |
| `applySettingsSearch` | method of XeniaConfigViewController | Fills shown_sections_ and shown_items_ with the settings matching every word of the search text. | `src/xenia/ui/windowed_app_main_ios.mm` | 7984, 8016, 8051 | 3 |
| `search_text` | parameter of applySettingsSearch: | The search bar text (nil for none). | `src/xenia/ui/windowed_app_main_ios.mm` | 8016, 8018 | 2 |
| `filtering` | local in applySettingsSearch: | The search text has a word, so sections without matches are hidden. | `src/xenia/ui/windowed_app_main_ios.mm` | 8019, 8021, 8043 | 4 |
| `shown` | local in tableView:titleForFooterInSection: | The section shown at that index. | `src/xenia/ui/windowed_app_main_ios.mm` | 8243, 8244 | 3 |
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
| `DrawStats` | 3 |
| `draw_stats_` | 55 |
| `kDrawStatsFrames` | 3 |
| `app_version` | 2 |
| `app_build` | 2 |
| `artifact_suffix` | 5 |
| `boost_context` | 15 |
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
| `position` (MetalGpuPixelShaderFeatures local) | 4 |
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
| `cache-submodules-cpu-tests` | 1 |
| `bin_count` | 3 |
| `build_cpu_tests` | 2 |
| `run` (run_cpu_tests.py) | 10 |
| `tail` (run_cpu_tests.py) | 3 |
| `escape_catch_name` | 2 |
| `run_hir_tests` | 2 |
| `run_ppc_suite` | 2 |
| `run_ppc_suite_once` | 3 |
| `run_ppc_tests` | 2 |
| `write_report` | 2 |
| `main` (run_cpu_tests.py) | 2 |
| `ROOT` (build_cpu_tests.py) | 10 |
| `host` (build_cpu_tests.py) | 4 |
| `wanted` | 3 |
| `collect_sources` | 2 |
| `rel` (build_cpu_tests.py) | 9 |
| `flags` (build_cpu_tests.py) | 2 |
| `ensure_version_header` | 2 |
| `git` (build_cpu_tests.py) | 4 |
| `compile_one` | 2 |
| `main` (build_cpu_tests.py) | 2 |
| `cpu_tests_duplicate_record_mmio_access_exceptions` | 1 |
| `cpu_tests_duplicate_emit_mmio_aware_stores` | 1 |
| `mmio_repeat_address` | 2 |
| `mmio_repeat_count` | 6 |
| `mmio_guest_context` | 16 |
| `shm_attempt` | 5 |
| `mmio_thread_state` | 4 |
| `mmio_function` | 8 |
| `mmio_dump` | 4 |
| `mmio_start` | 3 |
| `mmio_words` | 3 |
| `mmio_line` | 12 |
| `mmio_text` | 6 |
| `mmio_word` | 9 |
| `mmio_end` | 3 |
| `kXeniaAutoEnableTrollStoreJITOnLaunchPreferenceKey` | 4 |
| `jit_provider_name` | 6 |
| `trollstore_url` | 4 |
| `mmio_frame` | 6 |
| `mmio_depth` | 5 |
| `mmio_caller_frame` | 6 |
| `mmio_return` | 5 |
| `LogFrequentXmaContextCreation` | 2 |
| `xma_create_count` | 2 |
| `xma_count` | 5 |
| `xma_thread_state` | 3 |
| `xma_context` | 5 |
| `xma_memory` | 4 |
| `xma_dump` | 4 |
| `xma_start` | 3 |
| `xma_words` | 3 |
| `xma_line` | 6 |
| `xma_text` | 3 |
| `xma_word` | 5 |
| `xma_frame` | 6 |
| `xma_depth` | 6 |
| `xma_caller_frame` | 7 |
| `xma_return` | 6 |
| `xma_repeat_key` | 2 |
| `xma_repeat_count` | 6 |
| `xma_key` | 3 |
| `xma_thread_state` (xma_decoder.cc) | 3 |
| `xma_frames` | 5 |
| `xma_frame_count` | 2 |
| `xma_frame` | 8 |
| `xma_symbol` | 10 |
| `kEmitInlineMmioChecksDefault` | 3 |
| `kernel_call_repeat_threshold` | 5 |
| `NoteKernelCall` | 4 |
| `kernel_call_history` | 9 |
| `kernel_call_runs` | 4 |
| `kernel_call_period` | 12 |
| `kernel_call_index` | 21 |
| `kernel_call_threshold` | 7 |
| `kernel_call_run` | 8 |
| `kernel_call_names` | 12 |
| `kernel_call_thread` | 5 |
| `kernel_call_memory` | 7 |
| `kernel_call_frame` | 8 |
| `kernel_call_stack` | 4 |
| `kernel_call_depth` | 3 |
| `kernel_call_caller_frame` | 6 |
| `NoteKernelCallDone` | 3 |
| `kernel_call_noted` | 3 |
| `KernelCallThreadState` | 5 |
| `kernel_call_threads_mutex` | 3 |
| `kernel_call_threads` | 3 |
| `kernel_call_this_thread` | 17 |
| `LogKernelCallThreads` | 5 |
| `kernel_call_now` | 6 |
| `kernel_call_lock` | 2 |
| `kernel_call_state` | 12 |
| `kernel_call_last` | 5 |
| `kernel_call_inside` | 5 |
| `kernel_call_owner` | 3 |
| `kernel_call_only_sleeps` | 3 |
| `kernel_call_code` | 7 |
| `kernel_call_line` | 12 |
| `kernel_call_words` | 6 |
| `kernel_call_word` | 8 |
| `LogKernelCallStack` | 7 |
| `guest_stack_pointer` | 3 |
| `guest_arg0` | 3 |
| `guest_arg1` | 3 |
| `kernel_call_ms` | 3 |
| `kernel_call_handle` | 3 |
| `kernel_call_object` | 11 |
| `kernel_call_xobject` | 5 |
| `kernel_call_total` | 3 |
| `kernel_call_history_lr` | 5 |
| `kernel_call_storm_ms` | 3 |
| `kernel_call_storm_reports` | 3 |
| `kernel_call_storm_elapsed` | 3 |
| `kernel_call_global` | 13 |
| `kernel_call_xthreads` | 2 |
| `kernel_call_xthread` | 15 |
| `kernel_call_host_pc` | 11 |
| `kernel_call_host_thread` | 6 |
| `kernel_call_port` | 6 |
| `kernel_call_arm_state` | 3 |
| `kernel_call_state_count` | 2 |
| `kernel_call_function` | 7 |
| `kernel_call_context` | 5 |
| `KernelCallReadable` | 9 |
| `kernel_call_heap` | 3 |
| `kernel_call_symbol` | 5 |
| `SampleKernelCallHostPc` | 3 |
| `kernel_call_code_cache` | 3 |
| `kernel_call_guest_pc` | 5 |
| `kernel_call_sample` | 3 |
| `kernel_call_sample_pc` | 4 |
| `MetalPerfCounters` | 2 |
| `gpu_us` | 3 |
| `gpu_command_buffers` | 3 |
| `cpu_wait_us` | 3 |
| `cpu_waits` | 3 |
| `shader_compiles` | 3 |
| `shader_compile_us` | 3 |
| `pipeline_compiles` | 4 |
| `pipeline_compile_us` | 4 |
| `upload_bytes` | 3 |
| `metal_perf_counters` | 68 |
| `MetalPerfMicrosecondsSince` | 7 |
| `metal_perf_begin` | 4 |
| `WaitUntilCompletedCounted` | 9 |
| `metal_perf_command_buffer` | 2 |
| `MetalPerfThreadCpu` | 2 |
| `metal_perf_text` | 5 |
| `metal_perf_previous_ns` | 4 |
| `metal_perf_current_ns` | 3 |
| `metal_perf_threads` | 7 |
| `metal_perf_list` | 6 |
| `metal_perf_count` | 5 |
| `metal_perf_total_ns` | 3 |
| `metal_perf_index` | 11 |
| `metal_perf_identifier` | 5 |
| `metal_perf_info_count` | 2 |
| `metal_perf_extended` | 9 |
| `metal_perf_extended_count` | 2 |
| `metal_perf_ns` | 5 |
| `metal_perf_previous` | 3 |
| `metal_perf_delta` | 4 |
| `metal_perf_window_ns` | 3 |
| `metal_perf_left` | 6 |
| `metal_perf_right` | 6 |
| `metal_perf_footprint_mb` | 3 |
| `metal_perf_vm_info` | 3 |
| `metal_perf_vm_count` | 2 |
| `metal_perf_available_mb` | 3 |
| `metal_perf_window_us` | 4 |
| `metal_perf_gpu_us` | 3 |
| `metal_perf_swap_time` | 3 |
| `metal_perf_frame_us` | 5 |
| `pipeline_compile_begin` | 4 |
| `completed_buffer` | 3 |
| `metal_perf_gpu_seconds` | 3 |
| `frame_us_total` | 4 |
| `frame_us_max` | 4 |
| `frames_over_33ms` | 3 |
| `frames_over_50ms` | 3 |
| `metal_perf_last_swap_` | 4 |
| `unhandled_host_exception_count` | 2 |
| `unhandled_count` | 4 |
| `PreferPerformanceCores` | 4 |
| `fifo_min` | 3 |
| `fifo_max` | 2 |
| `MetalGpuPassRecord` | 22 |
| `sample_buffer` | 12 |
| `sample_buffer_index` | 5 |
| `first_sample` | 11 |
| `sample_count` | 12 |
| `label` | 22 |
| `width` | 30 |
| `height` | 31 |
| `samples` | 22 |
| `color_formats` | 6 |
| `depth_format` | 3 |
| `load_actions` | 3 |
| `store_actions` | 3 |
| `draws` | 32 |
| `vertices` | 23 |
| `MetalGpuPassTimer` | 14 |
| `Initialize` | 2 |
| `RenderEncoder` | 6 |
| `ComputeEncoder` | 8 |
| `BlitEncoder` | 5 |
| `Log` | 4 |
| `Enabled` | 5 |
| `AllocateSamples` | 5 |
| `Complete` | 4 |
| `Release` | 3 |
| `MetalGpuPassStats` | 8 |
| `passes` | 15 |
| `total_ticks` | 12 |
| `vertex_ticks` | 12 |
| `fragment_ticks` | 11 |
| `record_out` | 7 |
| `mutex_` | 13 |
| `device_` | 4 |
| `supported_` | 5 |
| `sample_buffers_` | 16 |
| `sample_buffer_outstanding_` | 8 |
| `sample_buffer_size_` | 4 |
| `current_sample_buffer_` | 7 |
| `next_sample_` | 6 |
| `untimed_passes_` | 6 |
| `invalid_passes_` | 6 |
| `calibration_gpu_` | 6 |
| `calibration_time_` | 4 |
| `ns_per_tick_` | 7 |
| `stats_by_label_` | 7 |
| `stats_by_pass_` | 6 |
| `metal_gpu_pass_timer` | 14 |
| `metal_gpu_pass_timing` | 8 |
| `metal_gpu_pass_record_` | 7 |
| `timestamp_set` | 5 |
| `counter_sets` | 3 |
| `counter_set` | 7 |
| `cpu_timestamp` | 5 |
| `gpu_timestamp` | 8 |
| `holder` | 3 |
| `released` | 3 |
| `describe_attachment` | 4 |
| `sample_attachment` | 19 |
| `timestamps` | 34 |
| `resolved` | 3 |
| `results` | 2 |
| `valid` | 7 |
| `vertex_ok` | 6 |
| `fragment_ok` | 6 |
| `stale_ticks` | 3 |
| `usable` | 7 |
| `add` | 4 |
| `ms_per_frame` | 7 |
| `describe` | 3 |
| `by_time` | 4 |
| `listed` | 6 |
| `stats` | 34 |
| `ticks` | 3 |
| `load_bytes` | 13 |
| `store_bytes` | 14 |
| `pipelines` | 9 |
| `MetalGpuPassPipeline` | 4 |
| `vertex_translation` | 10 |
| `pixel_translation` | 18 |
| `shaders` | 6 |
| `features` | 22 |
| `texture_samples` | 6 |
| `NoteDraw` | 4 |
| `MetalGpuPixelShaderFeatures` | 4 |
| `stats_by_pipeline_` | 5 |
| `pixel_shader_features_` | 7 |
| `window_us` | 4 |
| `MetalGpuBytesPerPixel` | 2 |
| `stencil` | 79 |
| `load_action` | 3 |
| `store_action` | 6 |
| `pixel_bytes` | 3 |
| `attachment_bytes` | 3 |
| `resolve_texture` | 4 |
| `counter_text` | 5 |
| `pipeline_vertex_ticks` | 3 |
| `pipeline_fragment_ticks` | 3 |
| `source` | 9 |
| `call` | 3 |
| `window_seconds` | 2 |
| `MetalCompileStallTracker` | 8 |
| `MetalCompileKey` | 11 |
| `Queued` | 4 |
| `Skipped` | 5 |
| `Finished` | 5 |
| `Clear` | 3 |
| `Describe` | 4 |
| `MetalCompileStall` | 7 |
| `key` | 39 |
| `queued_at` | 4 |
| `skipped` | 23 |
| `pending_us` | 9 |
| `compile_us` | 16 |
| `pending_` | 11 |
| `bucket_compiles_` | 6 |
| `bucket_skipped_` | 6 |
| `queue_us_` | 5 |
| `compile_us_` | 5 |
| `finished_` | 11 |
| `metal_compile_stall_tracker` | 10 |
| `stall` | 31 |
| `note` | 5 |
| `bucket_limits_ms` | 2 |
| `bucket` | 8 |
| `compiles` | 7 |
| `waiting` | 4 |
| `waiting_skipped` | 3 |
| `oldest_us` | 4 |
| `bucket_names` | 2 |
| `worst` | 3 |
| `XE_TRANSFER_STENCIL_OUTPUT` | 6 |
| `stencil` (TransferDepthOut member) | 3 |
| `stencil_packed` | 3 |
| `stencil_packed_set` | 3 |
| `transfer_stencil` | 4 |
| `depth_stencil_output_state` | 3 |
| `named_candidate_count` | 2 |
| `argument_encoder_begin` | 2 |
| `argument_encoder_ms` | 3 |
| `metal_async_compile_wait_ms` | 4 |
| `queue_arrival` | 9 |
| `msl_compile_arrivals_` | 3 |
| `msl_compile_completions_` | 5 |
| `msl_compile_done_cv_` | 6 |
| `msl_compile_wait_used_us_` | 5 |
| `WaitForMslCompileProgress` | 5 |
| `compile_waits` | 3 |
| `compile_wait_us` | 3 |
| `wait_ms` | 3 |
| `budget_us` | 3 |
| `wait_begin` | 3 |
| `completions` | 2 |
| `waited_us` | 3 |
| `metal_render_pass_area_from_scissor` | 8 |
| `render_area_needed_width_` | 7 |
| `render_area_needed_height_` | 6 |
| `current_render_area_width_` | 7 |
| `current_render_area_height_` | 5 |
| `render_area_by_descriptor_` | 3 |
| `area_scissor` | 6 |
| `area_scale_x` | 3 |
| `area_scale_y` | 3 |
| `area_width` | 10 |
| `area_height` | 8 |
| `attachments_width` | 6 |
| `attachments_height` | 5 |
| `clears_on_load` | 3 |
| `limit_to_attachment` | 4 |
| `known_area` | 16 |
| `pass_width` | 6 |
| `pass_height` | 6 |
| `transfer_area_width` | 4 |
| `transfer_area_height` | 3 |
| `area_right` | 5 |
| `area_bottom` | 9 |
| `add_to_area` | 3 |
| `log_guest_thread_hotspots` | 4 |
| `LogGuestThreadHotspots` | 3 |
| `HotspotThread` | 4 |
| `xthread` (HotspotThread field) | 2 |
| `host_thread` | 3 |
| `places` (HotspotThread field) | 4 |
| `guest_samples` | 4 |
| `samples` (HotspotThread field) | 5 |
| `kHotspotRounds` | 2 |
| `hotspot_global` | 3 |
| `hotspot_xthreads` | 2 |
| `hotspot_threads` | 5 |
| `hotspot_xthread` | 6 |
| `hotspot_code_cache` | 2 |
| `hotspot_round` | 4 |
| `hotspot_thread` | 13 |
| `hotspot_pc` | 4 |
| `hotspot_place` | 12 |
| `hotspot_function` | 2 |
| `hotspot_symbol` | 4 |
| `hotspot_busy` | 3 |
| `hotspot_count` | 3 |
| `hotspot_sorted` | 7 |
| `hotspot_text` | 3 |
| `metal_shader_preserve_invariance` | 3 |
| `last_update_height_used_` | 4 |
| `last_update_height_used` | 3 |
| `render_area_draw_passes` | 3 |
| `render_area_draw_limited` | 3 |
| `render_area_draw_rows` | 3 |
| `render_area_restarts` | 3 |
| `render_area_transfer_passes` | 3 |
| `render_area_transfer_limited` | 3 |
| `render_area_transfer_rows` | 3 |
| `occlusion_query_events` | 3 |
| `occlusion_queries` | 3 |
| `take` (IssueSwap lambda) | 15 |
| `area_draw_passes` | 2 |
| `area_draw_limited` | 4 |
| `area_draw_rows` | 2 |
| `area_transfer_passes` | 2 |
| `area_transfer_limited` | 4 |
| `area_transfer_rows` | 2 |
| `area_descriptor` | 1 |
| `accuracy` (settings section) | 9 |
| `metal_uniform_buffers_max` | 4 |
| `MetalUniformBuffersMax` | 4 |
| `uniform_waits` | 3 |
| `uniform_wait_us` | 3 |
| `uniform_rollovers` | 3 |
| `uniform_wait_begin` | 2 |
| `float_constants_vertex_size` | 2 |
| `float_constants_pixel_size` | 2 |
| `ProcessCompletedZPDQueries` | 4 |
| `MetalZPDQuery` | 5 |
| `address` (MetalZPDQuery field) | 8 |
| `submission` (MetalZPDQuery field) | 4 |
| `ticket` (MetalZPDQuery field) | 4 |
| `guess` (MetalZPDQuery field) | 6 |
| `inexact` (MetalZPDQuery field) | 8 |
| `slots` (MetalZPDQuery field) | 6 |
| `zpd_query_active_` | 10 |
| `zpd_segment_open_` | 7 |
| `zpd_query_` | 12 |
| `zpd_compile_pending_at_begin_` | 3 |
| `zpd_query_ends_` | 3 |
| `zpd_queries_in_flight_` | 10 |
| `zpd_results_by_address_` | 7 |
| `zpd_queries_measured` | 4 |
| `zpd_queries_hidden` | 4 |
| `zpd_queries_corrected` | 3 |
| `zpd_queries_guessed` | 3 |
| `zpd_pass_restarts` | 3 |
| `kQueryFinished` | 5 |
| `sample_count_addr` | 8 |
| `sample_counts` | 22 |
| `is_end` | 5 |
| `real_queries` | 3 |
| `query` (ZPD locals) | 46 |
| `address_result` | 8 |
| `samples` (ZPD locals) | 12 |
| `scale_area` | 4 |
| `slot_samples` | 3 |
| `counted` (ZPD local) | 3 |
| `count` (ZPD local) | 12 |
| `slot_index` | 7 |
| `slot_generation` | 6 |
| `slot_offset` | 3 |
| `zpd_pass` | 5 |
| `metal_frame_detail_log` | 7 |
| `frame_detail_frames` | 12 |
| `frame_detail_setting_` | 7 |
| `frame_detail_passes_` | 5 |
| `frame_detail_draws_` | 9 |
| `frame_detail_stats_` | 4 |
| `frame_start` | 7 |
| `frame_detail_attachments` | 3 |
| `add_attachment` | 3 |
| `kColorNames` | 2 |
| `frame_detail_textures` | 3 |
| `fetch_bits` | 3 |
| `frame_detail_sources` | 3 |
| `metal_inline_shader_constants` | 6 |
| `inline_constants` | 6 |
| `write_constant_block` | 12 |
| `pixel_stage` (write_constant_block parameter) | 3 |
| `slot` (write_constant_block parameter) | 3 |
| `buffer_index` (write_constant_block parameter) | 3 |
| `offset` (write_constant_block parameter) | 3 |
| `data` (write_constant_block parameter) | 5 |
| `size` (write_constant_block parameter) | 6 |
| `version` (write_constant_block parameter) | 4 |
| `table` (write_constant_block local) | 3 |
| `msl_float_constants_version_vertex_` | 3 |
| `msl_float_constants_version_pixel_` | 3 |
| `msl_bool_loop_constants_version_` | 4 |
| `msl_fetch_constants_version_` | 4 |
| `msl_inline_constants_bound_versions_` | 5 |
| `metal_log_shader_hashes` | 2 |
| `ShouldLogMslShader` | 4 |
| `log_shader_lines` | 3 |
| `line_begin` | 6 |
| `line_end` | 5 |
| `frame_detail_shader` | 6 |
| `is_pixel` | 3 |
| `frame_detail_constants` | 3 |
| `MetalResolveProbe` | 4 |
| `resolve_probes_` | 9 |
| `LogResolveProbes` | 4 |
| `address` (MetalResolveProbe field) | 5 |
| `written_begin` (MetalResolveProbe field) | 4 |
| `written_end` (MetalResolveProbe field) | 4 |
| `width` (MetalResolveProbe field) | 8 |
| `height` (MetalResolveProbe field) | 6 |
| `endian` (MetalResolveProbe field) | 3 |
| `is_float` (MetalResolveProbe field) | 4 |
| `draw` (MetalResolveProbe field) | 3 |
| `submission` (MetalResolveProbe field) | 4 |
| `probe` (resolve probe locals) | 42 |
| `dest_pitch` | 3 |
| `dest_endian` | 3 |
| `probe_cols` | 4 |
| `probe_rows` | 4 |
| `probe_line` | 4 |
| `texel_address` | 10 |
| `depth24` | 19 |
| `ApplyXenosSamplePositions` | 4 |
| `programmable_sample_positions_supported_` | 5 |
| `use_xenos_positions` | 4 |
| `metal_gpu_ordered_fences` | 3 |
| `AttachShaderDoneFences` | 7 |
| `pending_shader_done_fences_` | 7 |
| `fence_command_buffer` | 4 |
| `fences` | 3 |
| `read_depth24` | 4 |
| `written_rows` | 12 |
| `kEdgeNames` | 2 |
| `edge_line` | 3 |
| `depth_in` | 7 |
| `not_clear` | 3 |
| `min_depth24` | 4 |
| `block_y` | 6 |
| `block_x` | 5 |
| `map_line` | 3 |
| `snapshot` | 11 |
| `edram_snapshot` | 20 |
| `edram_base_tiles` | 9 |
| `edram_pitch_tiles` | 10 |
| `snapshot_blit` | 12 |
| `edram` | 7 |
| `surface_info` | 7 |
| `tile_bytes` | 12 |
| `tiles` | 13 |
| `first_tiles` | 10 |
| `probe_memory` | 4 |
| `probe_memory_base` | 4 |
| `edram_tile_bytes` | 3 |
| `read_edram` | 3 |
| `read_edram_depth24` | 3 |
| `tile` | 3 |
| `tile_x` | 4 |
| `texels_line` | 3 |
| `positions` | 2 |
| `position` | 15 |
| `edram_texel` | 3 |
| `in_resolve` | 2 |
| `in_edram` | 2 |
| `log_coverage` | 4 |
| `read` | 3 |
| `frame_detail_bindings` | 3 |
| `vertex_words` | 3 |
| `frame_detail_vertices` | 3 |
| `word_float` | 3 |
| `OnlyFrameDetailLogChanged` | 2 |
| `frame_capture` | 6 |
| `frame_detail_changed` | 3 |
| `only_frame_detail_log` | 2 |
| `pass` | 10 |
| `depth_info` | 3 |
| `probe` | 122 |
| `crash_global` | 3 |
| `crash_locked` | 3 |
| `crash_fault` | 4 |
| `crash_virtual` | 5 |
| `crash_guest` | 4 |
| `crash_heap` | 5 |
| `crash_info` | 8 |
| `crash_pc` | 5 |
| `kernel_call_wait_objects` | 6 |
| `kernel_call_handles` | 3 |
| `kernel_call_wait_handle` | 3 |
| `kernel_call_wait_object` | 6 |
| `skip_unchanged_render_target_transfers` | 7 |
| `TakeUnchangedTransferCounts` | 2 |
| `unchanged_out` | 2 |
| `skipped_out` | 2 |
| `unchanged_transfers_` | 4 |
| `skipped_transfers_` | 4 |
| `intact_render_target` | 10 |
| `dest_written` | 5 |
| `depth_written` | 4 |
| `writes_stencil` | 3 |
| `fail` | 2 |
| `zpass` | 2 |
| `zfail` | 2 |
| `dest_intact` | 2 |
| `previous_owner_matches` | 6 |
| `previous_owner` | 2 |
| `unchanged_transfers` | 3 |
| `skipped_transfers` | 3 |
| `UserApcPendingForHostWait` | 7 |
| `kAlertableWaitPollIntervalIOS` | 2 |
| `slice` | 4 |
| `typed_utf8` | 3 |
| `ordinal` | 12 |
| `address_guess` | 5 |
| `ordinal_guess` | 7 |
| `zpd_frame_query_ends_` | 3 |
| `zpd_results_by_ordinal_` | 9 |
| `zpd_ordinal_score_` | 9 |
| `miss` | 3 |
| `ordinal_miss` | 3 |
| `address_miss` | 3 |
| `signin_state_logged` | 3 |
| `metal_gpu_trace_frames` | 5 |
| `gpu_trace_setting_` | 5 |
| `gpu_trace_frames_left_` | 7 |
| `trace_path` | 7 |
| `trace_time` | 2 |
| `trace_local_time` | 3 |
| `trace_folder` | 4 |
| `LogKernelCallStackCode` | 3 |
| `kernel_call_returns` | 3 |
| `kernel_call_return` | 6 |
| `kernel_call_code_logged` | 2 |
| `zero_commit` | 3 |
| `zero_runs` | 8 |
| `run_address` | 9 |
| `run_info` | 5 |
| `run_end` | 5 |
| `run_committed` | 3 |
| `zero_address` | 4 |
| `zero_size` | 4 |
| `VirtualMemoryEvent` | 3 |
| `virtual_memory_history_mutex` | 3 |
| `virtual_memory_history` | 6 |
| `virtual_memory_history_next` | 6 |
| `RecordVirtualMemoryEvent` | 3 |
| `DescribeVirtualMemoryHistory` | 3 |
| `crash_reg` | 5 |
| `crash_pointer` | 8 |
| `crash_pointer_heap` | 3 |
| `crash_pointer_info` | 7 |
| `crash_bytes` | 3 |
| `signed_in_mask` | 3 |
| `msaa_samples_as_pixels` | 7 |
| `last_update_samples_as_pixels_` | 5 |
| `last_update_samples_as_pixels_x_log2` | 6 |
| `last_update_samples_as_pixels_y_log2` | 8 |
| `SupportsMsaaSamplesAsPixels` | 5 |
| `storage_msaa_samples` | 9 |
| `samples_as_pixels` | 4 |
| `kSysFlag_SamplesAsPixelsX_Shift` | 2 |
| `kSysFlag_SamplesAsPixelsY_Shift` | 2 |
| `kSysFlag_SamplesAsPixelsX` | 3 |
| `kSysFlag_SamplesAsPixelsY` | 3 |
| `DescribeLastSignal` | 4 |
| `NoteSignal` | 4 |
| `last_signal_thread_` | 3 |
| `last_signal_lr_` | 3 |
| `last_signal_ms_` | 3 |
| `signal_count_` | 3 |
| `signal_thread` | 3 |
| `signal_count` | 3 |
| `shown_sections_` | 11 |
| `shown_items_` | 5 |
| `applySettingsSearch` | 3 |
| `search_text` | 2 |
| `filtering` | 4 |
| `shown` | 3 |

## Checks

- `src/xenia/cpu/testing/util.h`: `RequireFoldMatchesBackend`,
  `RequireVectorFoldMatchesBackend`, `RequireConstantOperandsMatchRegisters`
  and `RequireScalarFoldMatchesBackend` are copied from xenia-edge
  (`has207/xenia-edge`, testing helpers added with its fold-vs-backend tests),
  so they are not listed. `constant_operand_test.cc` is a new test file built
  only into `xenia-cpu-tests`; its names (`Ops`, `values`, `amounts`, `value`,
  `shift`, `vec`, `build`, `mask`, `value1`, `value2`, `type`, `i`) are locals
  of its test cases, used nowhere else.
- The a64 backend and HIR fixes for constant operands (`VECTOR_ROTATE_LEFT_V128`,
  `VECTOR_DENORMFLUSH`, `SHL_V128`, `SHR_V128`, `DOT_PRODUCT_3/4_V128`,
  `SELECT_V128_V128`, `Value::Extract`, `Value::VectorCompareEQ`,
  `SameValueOrEqualConstant`, the `OPCODE_ROUND` fold) follow xenia-edge
  commits e6a86116f, ef4b3a466, 5ce7c336d, 920f35c93, eddcd8f31 and
  378c95215, and add no names. `STVL_V128`/`STVR_V128` only reorder two
  existing lines.
- Ported verbatim from XeniOS `77cc1358`, so not listed: the range-keyed
  `ContextPromotionPass` (`context_promote_vec128`, `LookupTrackedValue`,
  `TrackValue`, `InvalidateTrackedRange`, `context_value_size_`,
  `context_value_base_`; only the `FUNCTION_ATTRIB_KEEP_CONTEXT_STORES` check
  is left out, as this tree has no such attribute),
  `xe::threading::DeadlineSleep` (and its local `tb`, `deadline`), and the
  frame limiter loop's locals in `graphics_system.cc` (`tick_freq`,
  `target_duration_ticks`, `current_time`, `time_delta`, `sleep_ns`).
- None of these names is defined anywhere else in this repository, so there are
  no conflicting or circular definitions.
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
- The names in `tools/ci/run_cpu_tests.py` and `tools/ci/build_cpu_tests.py`
  are local to each script; `run`, `tail`, `main`, `host`, `flags`, `rel` and
  `git` are common names elsewhere in the tree but are not shared with any
  other file. Only module-level names and functions of the scripts are
  listed. `tools/ci/cpu_tests_support.cc` only defines existing functions
  (system dialogs and `XexModule` members) for the CPU test programs, so it
  adds no names.
- `xe_stikdebug_enable_jit_url_for_bundle_identifier` (existing) gained a
  `scheme` parameter so the same builder serves `stikjit` and
  `apple-magnifier`; `apple-magnifier` was already in
  `LSApplicationQueriesSchemes` in `src/xenia/app/Info_ios.plist`.
- `xma_thread_state` is declared in two files (`xboxkrnl_audio_xma.cc` and
  `xma_decoder.cc`); both are function locals and unrelated.
- `KernelCallThreadState` fields (`last_export`, `call_count`, `enter_ms`,
  `in_call`, `handle`, `lr`, `guest_stack_pointer`, `guest_arg0`,
  `guest_arg1`, `name`) are only used in `shim_utils.cc`. `guest_stack_pointer`
  was chosen because `guest_stack` already exists in the x64 backend.
- `metal_perf_counters.h` is a new header, included only by Metal GPU files
  (built for macOS and iOS). `command_buffers` and `gpu_seconds` already
  exist elsewhere (XeniOS, xenia-edge), so the new names are
  `gpu_command_buffers` and `metal_perf_gpu_seconds`.
- `metal_gpu_pass_timing.{h,cc}` are new files, built only for macOS and iOS
  (Metal). The encoder helpers, the record and the timer are only used by
  Metal GPU files. Generic locals and parameters with the same name and role
  upstream (`device`, `command_buffer`, `descriptor`, `encoder`, `error`,
  `size`, `i`, `lock`, `pool`, `data`, `texture`, `attachment`, `index`,
  `key`, `value`, `text`, `sorted`, `found`, `candidate`, `frames`,
  `timestamp`, `record`) are not listed. `metal_perf_left`/`metal_perf_right`
  are reused as the sort lambda parameters in `MetalGpuPassTimer::Log`.
- `draws` and `vertices` are also used in `metal_command_processor.cc` at the
  draw count (`metal_gpu_pass_record_->draws`, `->vertices`), next to
  `++draw_stats_.encoded`; that file has other, unrelated uses of the words.
- The compile stall tracker and per-pipeline attribution live in
  `metal_gpu_pass_timing.{h,cc}` too. Generic locals there with the same name
  and role upstream (`counters`, `counter`, `j`, `pipeline`, `now`, `text`,
  `compile_begin` in `metal_command_processor.cc`) are not listed.
  `compile_us` in `MslShaderCompileThread` is two separate locals (pipeline
  and shader branches), each passed to `Finished`.
- The previous commit's line refresh counted short names that are only
  listed for one block (the `GetLogFilePath` and `XeniaTouchPadView` locals
  such as `now`, `width`, `left`) across all of
  `windowed_app_main_ios.mm`. Those rows are re-derived here from the commit
  that wrote them, mapped to the current line numbers; names counted across
  the whole file are recounted across it.
- Native stencil output in render target transfers is ported from XeniOS
  77cc1358 (xenia-edge has the same): `native_stencil_output` (a
  `TransferShaderKey` field here, a `TransferPipelineKey` field there),
  `transfer_depth_stencil_output_state_`, `native_stencil_output_probed_`,
  `GetTransferDepthAndStencilOutputState`, `UseNativeStencilOutputInTransfers`,
  `ProbeNativeStencilOutputSupport` (with its `kProbeSource`,
  `XeStencilProbeOut`, `xe_stencil_probe_vs`/`ps` and locals),
  `use_native_stencil_output` and `writes_stencil_with_depth` have the same
  names and roles there, so they are not listed.
- Xenos MSAA sample positions are ported from XeniOS 77cc1358 (xenia-edge
  4fb8372d has the same shared code): `xenos_sample_positions` (cvar, default
  true here, false upstream where only Direct3D 12 and Vulkan use it),
  `kXenosSamplePositions2x`, `kXenosSamplePositions4x` and
  `GetHostSampleXenosPositions` have the same names and roles there, so they
  are not listed. In `ApplyXenosSamplePositions`, `xenos_positions`,
  `position_count` and `positions` have the same names and roles as in
  upstream's `D3D12CommandProcessor::UpdateSamplePositions`; `sample_count`,
  `depth_attachment`, `attachments`, `attachment` and `texture` are generic
  locals with the same role upstream, so they are not listed.
- This commit's line refresh maps every listed line through the diff of each
  changed file (unchanged lines keep their row, shifted by the lines added
  above them); nothing was recounted.
- Shader-done fence ordering follows XeniOS 77cc1358 (xenia-edge 4fb8372d
  has none of it): `TryWriteShaderDoneFence`, `SubmitBeforeShortMemoryPoll`,
  `is_zero_copy`, `write_physical_address` and `submitted_memory_producers`
  have the same names and roles there, so they are not listed. XeniOS keeps
  its queued fences in `pending_shader_done_fence_ranges_`/`_values_` and
  copies them on the GPU; here they are stored by a completion handler, hence
  the different `pending_shader_done_fences_` and `AttachShaderDoneFences`.
  `fences` also names unrelated members in `xenia/ui` and
  `xenia/base/chrono_steady_cast.h`; the row counts only the capture in
  `AttachShaderDoneFences`. `differs` (local in `OnlyFrameDetailLogChanged`)
  is a generic word used in other files and is not listed; `stored`,
  `edited`, `saved`, `binding`, `vertex_fetch`, `vertex_address`, `words`,
  `word`, `edge`, `vertical`, `length`, `clear`, `written`, `memory` and
  `fence` are generic locals with the same role elsewhere, not listed.
- GPU trace capture: `GuestReadPtrOffset` (CommandProcessor; the guest
  physical address of the PM4 reader position, which the trace writer needs
  where the 32-bit truncated host pointer was used) and `XE_OPTION_GPU_TRACE`
  (keeps the trace writer in Release) have the same names and roles in
  XeniOS HEAD (`refs/remotes/xenios/HEAD`), so they are not listed. Here
  `trace_writer.h` defines `XE_OPTION_GPU_TRACE` on iOS, where XeniOS sets it
  from a CMake option.
- `history` (local in DescribeVirtualMemoryHistory), `event` (its loop
  variable), `lock` and `thread` (locals in RecordVirtualMemoryEvent and
  DescribeVirtualMemoryHistory), the VirtualMemoryEvent fields (`thread`,
  `address`, `size`, `type`, `free`) and `user_index` (loop variable in
  RegisterNotifyListener) are generic names with the same roles elsewhere,
  not listed.
- `EmitPartialVectorStore` (a64 backend, used by STVL/STVR) and its
  parameters and labels (`dst`, `src`, `count`, `from8`, `from4`, `from2`,
  `from1`, `done`), as well as `addr` and `s` in the STVL/STVR sequences,
  have the same names and roles in xenia-edge HEAD
  (`refs/remotes/xenia-edge-deep/HEAD`), where the fix comes from, so they
  are not listed.
- Settings search: `search` (the UISearchBar local in viewDidLoad),
  `words`, `word`, `items`, `text`, `matches`, `section_index`,
  `item_index`, `section` and `item` (locals in applySettingsSearch:) and
  `searchBar` and `searchText` (UIKit's parameter names in the
  UISearchBarDelegate methods) are generic names with the same role
  elsewhere, not listed. `thread_state` in XEvent::NoteSignal is the
  usual name of a `cpu::ThreadState*`, not listed.
