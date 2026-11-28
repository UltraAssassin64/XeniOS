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
| `session_log_path` | function-local static in GetLogFilePath | The log file chosen for this launch; read by the logger setup and the in-app log viewer. | `src/xenia/ui/windowed_app_main_ios.mm` | 1286, 1299 | 2 |
| `now` | local in GetLogFilePath | Current time used to name the log file. | `src/xenia/ui/windowed_app_main_ios.mm` | 1291, 1293 | 2 |
| `local_time` | local in GetLogFilePath | now broken down in the device's time zone. | `src/xenia/ui/windowed_app_main_ios.mm` | 1292, 1293, 1296 | 3 |
| `log_name` | local in GetLogFilePath | File name xenia_<date>_<time>.log. | `src/xenia/ui/windowed_app_main_ios.mm` | 1294, 1295, 1297 | 4 |
| `kText` | enumerator of IOSConfigControlType | Settings row whose value is edited as text. | `src/xenia/ui/windowed_app_main_ios.mm` | 568, 1257, 3985, 8591, 8701 | 5 |
| `AddAllConfigVarSettings` | function | Adds a settings section per config category with every variable not already shown. | `src/xenia/ui/windowed_app_main_ios.mm` | 1231, 3901 | 2 |
| `existing_keys` | local in AddAllConfigVarSettings | Config keys the curated sections already expose, so they are not listed twice. | `src/xenia/ui/windowed_app_main_ios.mm` | 1235, 1239, 1245 | 3 |
| `by_category` | local in AddAllConfigVarSettings | Generated sections keyed by config category. | `src/xenia/ui/windowed_app_main_ios.mm` | 1243, 1261, 1265 | 3 |
| `parsed` | local in ApplyIOSConfigSections | The edited text parsed as a TOML value. | `src/xenia/ui/windowed_app_main_ios.mm` | 3999, 4001, 4004, 4011 | 4 |
| `editor` | local in tableView:didSelectRowAtIndexPath: | Alert with a text field used to edit a kText row. | `src/xenia/ui/windowed_app_main_ios.mm` | 8705, 8709, 8745, 8748, 8751, 8761 | 6 |
| `DrawStats` | struct in MetalCommandProcessor | Counters describing what happened to guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 667, 685, 695 | 3 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.h` | 685, 687 | 2 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2921, 2928, 2929, 2930, 2931, 2932, 2933, 2984, 2992, 2993, 2994, 2995, 2996, 2999, 3006, 3007, 3008, 3009, 3010, 3011, 3012, 3013, 3030, 3041, 3042, 3043, 3044, 3076, 3106, 3113, 3124, 3133, 3143, 3144, 3146, 3423, 3425, 3461, 3486, 3547, 5005, 5036, 5137, 6866, 6867, 7547, 7560 | 57 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.h` | 664, 666 | 2 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2999 | 1 |
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
| `SetTouchState` | function | Stores the on-screen controller state; called by the UI, read by the driver. | `src/xenia/ui/windowed_app_main_ios.mm` | 8895, 9173 | 2 |
| `TouchInputDriver` | class (anonymous namespace) | Input driver presenting the on-screen controller as a fallback gamepad. | `src/xenia/hid/touch_input_driver_ios.cc` | 29, 31, 182 | 3 |
| `touch_state_mutex_` | file-scope variable | Guards `touch_state_`. | `src/xenia/hid/touch_input_driver_ios.cc` | 24, 25, 66, 107, 162 | 5 |
| `touch_state_` | file-scope variable | Latest on-screen controller state and packet number. | `src/xenia/hid/touch_input_driver_ios.cc` | 27, 67, 108, 109, 113, 163, 177 | 8 |
| `kKeystrokeKeys` | local constant in GetKeystroke | Virtual key for each button bit. | `src/xenia/hid/touch_input_driver_ios.cc` | 85, 120, 123, 129, 153 | 5 |
| `keystroke_buttons` | local in GetKeystroke | Current buttons and triggers as a bit field. | `src/xenia/hid/touch_input_driver_ios.cc` | 105, 108, 111, 115, 122, 124 | 6 |
| `release_pass` | local in GetKeystroke | Loop counter: releases are reported before presses. | `src/xenia/hid/touch_input_driver_ios.cc` | 119, 125 | 3 |
| `keystroke_bit` | local in GetKeystroke | Bit of the button being examined. | `src/xenia/hid/touch_input_driver_ios.cc` | 121, 122, 124, 128 | 4 |
| `pressed` | local in GetKeystroke | Whether that button is currently down. | `src/xenia/hid/touch_input_driver_ios.cc` | 122, 125, 132 | 3 |
| `last_keystroke_buttons_` | member of TouchInputDriver | Buttons already reported through GetKeystroke. | `src/xenia/hid/touch_input_driver_ios.cc` | 124, 128, 154 | 3 |
| `XeniaTouchPadView` | Objective-C class | The on-screen controller view. | `src/xenia/ui/windowed_app_main_ios.mm` | 8926, 8955, 9480, 9601, 11389 | 5 |
| `XeniaTouchPadControl` | struct | One control of the on-screen controller: kind, button bit, label, centre, radius. | `src/xenia/ui/windowed_app_main_ios.mm` | 8907, 8956, 9004, 9411 | 4 |
| `XeniaTouchPadControlKind` | enum | Kind of control. | `src/xenia/ui/windowed_app_main_ios.mm` | 8897, 8908 | 2 |
| `XeniaTouchPadControlKindButton` | enumerator | A digital button. | `src/xenia/ui/windowed_app_main_ios.mm` | 8898, 9009, 9014, 9019, 9024, 9025, 9030, 9035, 9040, 9045, 9050, 9055, 9060, 9065, 9070, 9075 | 16 |
| `XeniaTouchPadControlKindLeftTrigger` | enumerator | Left trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8899, 9007, 9165 | 3 |
| `XeniaTouchPadControlKindRightTrigger` | enumerator | Right trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8900, 9008, 9167 | 3 |
| `XeniaTouchPadControlKindLeftStick` | enumerator | Left thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8901, 9005, 9144, 9155, 9417, 9452 | 6 |
| `XeniaTouchPadControlKindRightStick` | enumerator | Right thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8902, 9006, 9145, 9418, 9453 | 5 |
| `kXeniaTouchPadControlCount` | file-scope constant | Number of controls (19). | `src/xenia/ui/windowed_app_main_ios.mm` | 8915, 8956, 8957, 9004, 9082, 9102, 9267, 9278, 9304, 9407 | 10 |
| `controls_` | instance variable of XeniaTouchPadView | The controls with their current positions. | `src/xenia/ui/windowed_app_main_ios.mm` | 8956, 9083, 9086, 9089, 9097, 9106, 9107, 9109, 9144, 9145, 9147, 9148, 9155, 9165, 9167, 9170, 9199, 9212, 9232, 9279, 9318, 9327, 9363, 9384, 9411 | 28 |
| `pad_touches_` | instance variable of XeniaTouchPadView | Touches currently on the pad. | `src/xenia/ui/windowed_app_main_ios.mm` | 8959, 8973, 8982, 9141, 9142, 9179, 9182, 9218, 9251, 9253, 9415, 9416 | 12 |
| `pad_touch_controls_` | instance variable of XeniaTouchPadView | Control each of those touches started on. | `src/xenia/ui/windowed_app_main_ios.mm` | 8960, 8974, 8983, 9143, 9183, 9219, 9254, 9419 | 8 |
| `controlIndexAtPoint` | method of XeniaTouchPadView | Index of the control at a point, or -1. | `src/xenia/ui/windowed_app_main_ios.mm` | 8934, 9100, 9120, 9161, 9194, 9208, 9420, 11241 | 8 |
| `resetTouches` | method of XeniaTouchPadView | Releases every control and reports the idle state. | `src/xenia/ui/windowed_app_main_ios.mm` | 8936, 9127, 9178, 10450 | 4 |
| `reportState` | method of XeniaTouchPadView | Computes the gamepad state from the touches and passes it to SetTouchState. | `src/xenia/ui/windowed_app_main_ios.mm` | 9137, 9184, 9221, 9240, 9257 | 5 |
| `touchPadView` | property of XeniaViewController | The on-screen controller view instance. | `src/xenia/ui/windowed_app_main_ios.mm` | 9480, 9601, 9602, 9604, 9605, 10447, 10448, 10450, 11240, 11241 | 11 |
| `hide_touch_pad` | local in pollControllerNavigation | Whether the on-screen controller should be hidden now. | `src/xenia/ui/windowed_app_main_ios.mm` | 10443, 10447, 10448, 10449 | 4 |
| `width` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8998, 9002, 9003, 9087, 9230, 9234, 9464 | 8 |
| `height` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8999, 9005, 9006, 9033, 9038, 9043, 9048, 9053, 9058, 9063, 9068, 9073, 9078, 9080, 9087, 9231, 9235, 9465 | 19 |
| `insets` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9000, 9001, 9002 | 3 |
| `left` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9001, 9005, 9007, 9012, 9033, 9038, 9043, 9048, 9073 | 10 |
| `right` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9002, 9006, 9008, 9017, 9053, 9058, 9063, 9068, 9078 | 10 |
| `middle` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9003, 9022, 9024, 9028, 9080 | 5 |
| `controls` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9004, 9083, 9276, 9277, 9279, 9303, 9318, 9321 | 8 |
| `reach` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9109, 9110 | 3 |
| `dx` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9106, 9110 | 3 |
| `dy` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9107, 9110 | 3 |
| `buttons` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9138, 9170, 9173 | 3 |
| `triggers` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9139, 9166, 9168, 9173 | 5 |
| `axes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9140, 9156, 9157, 9173, 9174 | 7 |
| `point` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8934, 9100, 9106, 9107, 9119, 9120, 9142, 9147, 9148, 9161, 9193, 9194, 9199, 9227, 9229, 9416, 9420, 9423, 9424 | 21 |
| `index` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9094, 9096, 9097, 9143, 9144, 9145, 9147, 9148, 9155, 9161, 9162, 9165, 9167, 9170, 9194, 9195, 9196, 9199, 9208, 9209, 9212, 9219 | 25 |
| `x` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9085, 9087, 9106, 9147, 9149, 9151, 9156, 9199, 9229, 9230, 9234, 9284, 9286, 9288, 9307, 9308, 9423, 9425, 9427, 9430, 9464 | 32 |
| `y` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9085, 9087, 9107, 9148, 9149, 9152, 9157, 9199, 9229, 9231, 9235, 9285, 9286, 9288, 9307, 9309, 9424, 9425, 9428, 9430, 9465 | 32 |
| `length` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9149, 9150, 9151, 9152, 9425, 9426, 9427, 9428 | 8 |
| `axis` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9154, 9156, 9157 | 3 |
| `position` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8921, 9085, 9087, 9233, 9251, 9252, 9253, 9254, 9287, 9307, 9308, 9309 | 15 |
| `attributes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9398, 9463, 9466 | 3 |
| `control` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9411, 9412, 9414, 9417, 9418, 9423, 9424, 9426, 9427, 9428, 9430, 9435, 9436, 9452, 9453, 9456, 9463, 9464 | 19 |
| `active` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9413, 9421, 9440, 9441 | 4 |
| `knob` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9414, 9430, 9455, 9464, 9465 | 5 |
| `is_stick` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9417, 9419, 9422 | 3 |
| `outline` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9434, 9442, 9446, 9449, 9451 | 5 |
| `knob_path` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9454, 9461 | 2 |
| `label_size` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 9463, 9464, 9465 | 3 |
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
| `mmio_repeat_address` | file-static atomic, MMIOHandler hang diagnostics | Guest address of the last MMIO store handled. | `src/xenia/cpu/mmio_handler.cc` | 37, 613 | 2 |
| `mmio_repeat_count` | file-static atomic, MMIOHandler hang diagnostics | How many MMIO stores in a row went to mmio_repeat_address. | `src/xenia/cpu/mmio_handler.cc` | 38, 615, 616, 624, 640 | 6 |
| `mmio_guest_context` | local in MMIOHandler::ExceptionCallback (ARM64) | Guest register context of the faulting thread, read through host register x20 for the repeated-store report. | `src/xenia/cpu/mmio_handler.cc` | 617, 628, 629, 630, 631, 632, 633, 634, 635, 636, 654, 671, 688, 693, 698 | 16 |
| `shm_attempt` | local in CreateFileMappingHandle (macOS only) | Attempt number when creating a shared memory object exclusively; attempts after the first add the process ID to the name. | `src/xenia/base/memory_posix.cc` | 486, 488, 490 | 5 |
| `mmio_thread_state` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest thread state of the faulting thread, to reach the processor. | `src/xenia/cpu/mmio_handler.cc` | 639, 640, 641, 642 | 4 |
| `mmio_function` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest function whose generated code holds the repeated store. | `src/xenia/cpu/mmio_handler.cc` | 642, 661, 662, 663, 664, 665, 667, 669 | 8 |
| `mmio_dump` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Lambda that logs guest code words, eight per line. | `src/xenia/cpu/mmio_handler.cc` | 646, 667, 671, 703 | 4 |
| `mmio_start` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest address the dump starts at (lambda parameter). | `src/xenia/cpu/mmio_handler.cc` | 646, 655, 657 | 3 |
| `mmio_words` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Number of guest words to dump (lambda parameter). | `src/xenia/cpu/mmio_handler.cc` | 646, 647, 650 | 3 |
| `mmio_line` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Index of the first word on the current dump line. | `src/xenia/cpu/mmio_handler.cc` | 647, 649, 650, 657, 674, 676, 683 | 12 |
| `mmio_text` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Text of one dump line. | `src/xenia/cpu/mmio_handler.cc` | 648, 652, 658, 675, 678, 683 | 6 |
| `mmio_word` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Index of the word being added to a dump line. | `src/xenia/cpu/mmio_handler.cc` | 649, 650, 651, 655, 676, 677, 680 | 9 |
| `mmio_end` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | End address of the guest function, or start + 0x200 if unknown. | `src/xenia/cpu/mmio_handler.cc` | 662, 666, 669 | 3 |
| `kXeniaAutoEnableTrollStoreJITOnLaunchPreferenceKey` | static NSString constant, iOS UI | User-defaults key of the "Auto-Enable JIT via TrollStore" toggle (stored as ios_auto_enable_jit_via_trollstore). | `src/xenia/ui/windowed_app_main_ios.mm` | 621, 3864, 12156, 12186 | 4 |
| `jit_provider_name` | local in requestAutomaticStikDebugJITHandoffForPendingLaunchPath: | "TrollStore" or "StikDebug", for the status label and log lines. | `src/xenia/ui/windowed_app_main_ios.mm` | 12184, 12191, 12223, 12224, 12234, 12236 | 6 |
| `trollstore_url` | local in requestAutomaticStikDebugJITHandoffForPendingLaunchPath: | apple-magnifier://enable-jit?bundle-id=... request for TrollStore. | `src/xenia/ui/windowed_app_main_ios.mm` | 12187, 12189, 12190 | 4 |
| `mmio_frame` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest stack frame being walked (starts at r1). | `src/xenia/cpu/mmio_handler.cc` | 688, 690, 693, 694, 705 | 6 |
| `mmio_depth` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Depth of the frame in the guest stack walk. | `src/xenia/cpu/mmio_handler.cc` | 689, 690, 691, 699, 701 | 5 |
| `mmio_caller_frame` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Back-chain pointer: the caller's frame. | `src/xenia/cpu/mmio_handler.cc` | 692, 694, 698, 700, 705 | 6 |
| `mmio_return` | local in MMIOHandler::ExceptionCallback (ARM64), hang diagnostics | Guest return address saved at -8 from the caller's frame. | `src/xenia/cpu/mmio_handler.cc` | 697, 700, 701, 702, 703 | 5 |
| `unhandled_guest_fault_count` | function static atomic, MMIOHandler::ExceptionCallback | Guest-memory faults that were neither MMIO nor a watch, to log only the first ten. | `src/xenia/cpu/mmio_handler.cc` | 502, 503 | 2 |
| `EmitVmxFmaV128InPlace` | function template, a64_sequences.cc | vmaddfp/vnmsubfp where FPCR.FZ flushes inputs: fmla in place, NaN test, PowerPC NaN order in the tail. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3985, 4021, 4119 | 3 |
| `Args` | template parameter of EmitVmxFmaV128InPlace | The sequence's EmitArgType. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3984, 3985 | 2 |
| `is_sub` | parameter of EmitVmxFmaV128InPlace | Multiply-subtract (negate the addend). | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3985, 3989 | 2 |
| `d` | local in EmitVmxFmaV128InPlace | Destination register index. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3987, 4012 | 2 |
| `s1` | local in EmitVmxFmaV128InPlace | First multiplicand register. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3992, 3994 | 2 |
| `s2` | local in EmitVmxFmaV128InPlace | Second multiplicand register. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3993, 3994 | 2 |
| `done` | local label in EmitVmxFmaV128InPlace | Where the tail fixup returns. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3995, 3997, 4004, 4011 | 4 |
| `nan_lane` | local label in EmitVmxFmaV128InPlace | Tail code for a result with a NaN lane. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3996, 4010 | 2 |
| `src1` | lambda init-capture in EmitVmxFmaV128InPlace (with `i.src1`) | The first source operand, for the tail. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3992, 3996, 4000 | 4 |
| `src2` | lambda init-capture in EmitVmxFmaV128InPlace (with `i.src2`) | The second source operand, for the tail. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3993, 3996, 4001 | 4 |
| `src3` | lambda init-capture in EmitVmxFmaV128InPlace (with `i.src3`) | The addend, for the tail. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3988, 3996, 4002 | 4 |
| `scratch` | local in the tail lambda | GUEST_SCRATCH stack offset where the sources are saved for the fixup. | `src/xenia/cpu/backend/a64/a64_sequences.cc` | 3999, 4000, 4001, 4002 | 4 |
| `alpha` | parameter of XeResolveUnpack32bpp8RedSamples, XeResolveUnpack64bpp8RedUnswappedSamples and XeResolveLoad8RedPixelSamplesFromRaw; local of XeResolveLoad8RedColors | An 8 bit resolve reads alpha (k_8 with the swap bit, D3D's A8 copy). | `src/xenia/gpu/shaders/resolve.xesli` | 360, 365, 371, 383, 413, 484, 487, 505, 598, 627, 631, 803, 813, 825, 838, 847 | 16 |
| `half_shift` | local in XeResolveUnpack64bpp8RedUnswappedSamples | 16 for the alpha half of the second dword, else 0. | `src/xenia/gpu/shaders/resolve.xesli` | 487, 490, 491, 494, 495, 496, 497, 498, 499, 500, 501 | 11 |
| `LogFrequentXmaContextCreation` | static function, XMA hang diagnostics | Logs the caller, guest stack and code when XMACreateContext is called 1000 times (and every 200,000 after). | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 69, 130 | 2 |
| `xma_create_count` | function-static atomic, XMA hang diagnostics | Number of XMACreateContext calls so far. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 70, 71 | 2 |
| `xma_count` | local, XMA hang diagnostics | This call's number. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 71, 72, 82, 83 | 5 |
| `xma_thread_state` | local, XMA hang diagnostics | Guest thread state of the caller. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 75, 76, 79 | 3 |
| `xma_context` | local, XMA hang diagnostics | Guest register context of the caller. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 79, 82, 98, 101 | 5 |
| `xma_memory` | local, XMA hang diagnostics | Guest memory, for reading the stack and code. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 80, 92, 107, 113 | 4 |
| `xma_dump` | local lambda, XMA hang diagnostics | Logs guest code words, eight per line. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 86, 98, 120, 122 | 4 |
| `xma_start` | lambda parameter, XMA hang diagnostics | Guest address the dump starts at. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 86, 93, 95 | 3 |
| `xma_words` | lambda parameter, XMA hang diagnostics | Number of words to dump. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 86, 87, 90 | 3 |
| `xma_line` | local, XMA hang diagnostics | Index of the first word on a dump line. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 87, 89, 90, 95 | 6 |
| `xma_text` | local, XMA hang diagnostics | Text of one dump line. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 88, 91, 95 | 3 |
| `xma_word` | local, XMA hang diagnostics | Index of the word being added. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 89, 90, 93 | 5 |
| `xma_frame` | local, XMA hang diagnostics | Guest stack frame being walked. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 101, 103, 107, 108, 125 | 6 |
| `xma_depth` | local, XMA hang diagnostics | Depth in the stack walk. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 102, 114, 116, 119 | 6 |
| `xma_caller_frame` | local, XMA hang diagnostics | Back-chain pointer to the caller's frame. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 106, 108, 109, 113, 115, 125 | 7 |
| `xma_return` | local, XMA hang diagnostics | Return address saved at -8 from the caller's frame. | `src/xenia/kernel/xboxkrnl/xboxkrnl_audio_xma.cc` | 112, 115, 116, 117, 120, 122 | 6 |
| `xma_repeat_key` | function-static atomic in XmaDecoder::WriteRegister, hang diagnostics | Register number and value of the last XMA register write. | `src/xenia/apu/xma_decoder.cc` | 377, 380 | 2 |
| `xma_repeat_count` | function-static atomic in XmaDecoder::WriteRegister, hang diagnostics | How many times in a row that pair was written. | `src/xenia/apu/xma_decoder.cc` | 378, 381, 382, 386, 389 | 6 |
| `xma_key` | local in XmaDecoder::WriteRegister, hang diagnostics | This write's register/value pair. | `src/xenia/apu/xma_decoder.cc` | 379, 380 | 3 |
| `xma_thread_state` | local in XmaDecoder::WriteRegister, hang diagnostics | Guest thread state of the writer, for its link register. | `src/xenia/apu/xma_decoder.cc` | 383, 387 | 3 |
| `xma_frames` | local array in XmaDecoder::WriteRegister, hang diagnostics | Native return addresses from backtrace(). | `src/xenia/apu/xma_decoder.cc` | 390, 391, 394, 398, 404 | 5 |
| `xma_frame_count` | local in XmaDecoder::WriteRegister, hang diagnostics | Number of native frames captured. | `src/xenia/apu/xma_decoder.cc` | 391, 392 | 2 |
| `xma_frame` | local in XmaDecoder::WriteRegister, hang diagnostics | Index of the frame being logged. | `src/xenia/apu/xma_decoder.cc` | 392, 394, 396, 398, 403, 404 | 8 |
| `xma_symbol` | local in XmaDecoder::WriteRegister, hang diagnostics | dladdr() result for one frame. | `src/xenia/apu/xma_decoder.cc` | 393, 394, 395, 397, 399, 400, 401 | 10 |
| `kEmitInlineMmioChecksDefault` | file-scope constexpr | Default of the `emit_inline_mmio_checks` cvar: true on iOS, false elsewhere. | `src/xenia/memory.cc` | 42, 44, 46 | 3 |
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/kernel_flags.cc` | 18 | 1 |
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/kernel_flags.h` | 16 | 1 |
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/util/shim_utils.h` | 547, 595 | 2 |
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/util/shim_utils.cc` | 819 | 1 |
| `NoteKernelCall` | function, kernel shim | Called at the start of every kernel export trampoline; detects repeating calls per guest thread. | `src/xenia/kernel/util/shim_utils.h` | 548, 549, 597 | 3 |
| `NoteKernelCall` | function, kernel shim | Called at the start of every kernel export trampoline; detects repeating calls per guest thread. | `src/xenia/kernel/util/shim_utils.cc` | 776 | 1 |
| `kernel_call_history` | thread_local array | The thread's last 32 kernel exports, a ring written at kernel_call_history_head. | `src/xenia/kernel/util/shim_utils.cc` | 801, 807, 814, 838, 842, 881, 899 | 7 |
| `kernel_call_runs` | thread_local array | For cycle lengths 1-8, how many calls in a row matched the call that many places earlier; only the shortest cycle at the threshold is reported. | `src/xenia/kernel/util/shim_utils.cc` | 804, 809, 811, 865 | 4 |
| `kernel_call_period` | local | Cycle length being checked (1-8). | `src/xenia/kernel/util/shim_utils.cc` | 805, 806, 807, 809, 811, 863, 864, 865, 879, 893 | 12 |
| `kernel_call_index` | local | Index into the history. | `src/xenia/kernel/util/shim_utils.cc` | 834, 835, 837, 879, 880, 882, 893, 894, 899 | 12 |
| `kernel_call_threshold` | local | Copy of the cvar for this call. | `src/xenia/kernel/util/shim_utils.cc` | 819, 868, 873, 874, 910, 934, 935 | 7 |
| `kernel_call_run` | local | Run length for the cycle length being checked. | `src/xenia/kernel/util/shim_utils.cc` | 865, 868, 873, 874, 906, 910, 934, 935 | 8 |
| `kernel_call_names` | local | Text of the repeating cycle ("A -> B"), of the last 32 calls in a storm report, or of a listed thread's suspend count, priority, CPU and start address. | `src/xenia/kernel/util/shim_utils.cc` | 404, 409, 418, 429, 833, 841, 852, 892, 895, 896, 898, 907 | 12 |
| `kernel_call_thread` | local | Current guest XThread, for its handle. | `src/xenia/kernel/util/shim_utils.cc` | 820, 848, 906 | 5 |
| `kernel_call_memory` | local | Guest memory, for the stack walk and the code dumps. | `src/xenia/kernel/util/shim_utils.cc` | 97, 116, 124, 442, 452, 914, 924 | 7 |
| `kernel_call_frame` | parameter of LogKernelCallStack | Guest stack frame being walked. | `src/xenia/kernel/util/shim_utils.cc` | 96, 107, 110, 112, 116, 117, 126 | 8 |
| `kernel_call_stack` | local | Text of the guest return addresses. | `src/xenia/kernel/util/shim_utils.cc` | 98, 111, 122, 129 | 4 |
| `kernel_call_depth` | local | Depth in the stack walk. | `src/xenia/kernel/util/shim_utils.cc` | 105, 106 | 3 |
| `kernel_call_caller_frame` | local | Back-chain pointer to the caller's frame. | `src/xenia/kernel/util/shim_utils.cc` | 115, 117, 118, 119, 125, 126 | 6 |
| `NoteKernelCallDone` | function, kernel shim | Clears the thread's in-call mark after every kernel export returns. | `src/xenia/kernel/util/shim_utils.h` | 550, 627 | 2 |
| `NoteKernelCallDone` | function, kernel shim | Clears the thread's in-call mark after every kernel export returns. | `src/xenia/kernel/util/shim_utils.cc` | 769 | 1 |
| `kernel_call_noted` | local in the export trampoline | Whether this call was reported to NoteKernelCall (so NoteKernelCallDone is called). | `src/xenia/kernel/util/shim_utils.h` | 595, 596, 626 | 3 |
| `KernelCallThreadState` | struct (anonymous namespace) | One guest thread's kernel-call state, readable from any thread. | `src/xenia/kernel/util/shim_utils.cc` | 61, 76, 77, 221, 778 | 5 |
| `kernel_call_threads_mutex` | file-static mutex | Guards kernel_call_threads. | `src/xenia/kernel/util/shim_utils.cc` | 75, 220, 783 | 3 |
| `kernel_call_threads` | file-static vector | Every thread's KernelCallThreadState; entries are never freed. | `src/xenia/kernel/util/shim_utils.cc` | 76, 221, 784 | 3 |
| `kernel_call_this_thread` | thread_local pointer | This thread's KernelCallThreadState. | `src/xenia/kernel/util/shim_utils.cc` | 77, 770, 771, 772, 777, 778, 780, 781, 784, 786, 787, 788, 789, 790, 792, 793, 794 | 17 |
| `LogKernelCallThreads` | function, kernel shim | Logs every guest thread: inside or last kernel call, for how long, guest lr, call count; for a thread blocked a second or more, its wait object and guest stack. Also called when the iOS app stops a game. | `src/xenia/kernel/util/shim_utils.cc` | 218, 858, 936 | 3 |
| `kernel_call_now` | local | Current host uptime in ms (in LogKernelCallThreads and in NoteKernelCall). | `src/xenia/kernel/util/shim_utils.cc` | 219, 230, 791, 792, 829, 830 | 6 |
| `kernel_call_lock` | local | Lock on kernel_call_threads_mutex. | `src/xenia/kernel/util/shim_utils.cc` | 220, 783 | 2 |
| `kernel_call_state` | local | Thread state being logged. | `src/xenia/kernel/util/shim_utils.cc` | 221, 222, 223, 230, 234, 238, 244, 275, 276, 277, 280, 281 | 14 |
| `kernel_call_last` | local | That thread's last export. | `src/xenia/kernel/util/shim_utils.cc` | 222, 225, 226, 236 | 5 |
| `kernel_call_inside` | local | Whether that thread is inside a kernel call now. | `src/xenia/kernel/util/shim_utils.cc` | 223, 225, 235, 237, 243 | 5 |
| `kernel_call_owner` | local | The new thread's XThread, for handle and name. | `src/xenia/kernel/util/shim_utils.cc` | 779, 780, 781 | 3 |
| `kernel_call_only_sleeps` | local | True when the repeating cycle is only KeDelayExecutionThread (idle, not reported). | `src/xenia/kernel/util/shim_utils.cc` | 878, 886, 889 | 3 |
| `kernel_call_code` | local | Guest address a code dump starts at (lr - 0x80 in a repeat report; sampled guest PC - 0x40 in the guest-thread list). | `src/xenia/kernel/util/shim_utils.cc` | 443, 444, 453, 457, 915, 925, 928 | 7 |
| `kernel_call_line` | local | First word of a code dump line. | `src/xenia/kernel/util/shim_utils.cc` | 445, 446, 448, 449, 457, 917, 918, 920, 921, 928 | 12 |
| `kernel_call_words` | local | Text of one code dump line. | `src/xenia/kernel/util/shim_utils.cc` | 447, 450, 457, 919, 922, 928 | 6 |
| `kernel_call_word` | local | Word being added to a code dump line. | `src/xenia/kernel/util/shim_utils.cc` | 448, 449, 453, 920, 921, 925 | 8 |
| `LogKernelCallStack` | function in xe::kernel::shim | Returns the guest return addresses up the stack from a frame (back chain at 0(r1), saved LR at caller frame - 8); also used by the guest crash dump. | `src/xenia/kernel/util/shim_utils.cc` | 96, 277, 432, 854, 912 | 5 |
| `guest_stack_pointer` | KernelCallThreadState field | Guest r1 at the thread's last kernel call, for its stack when it is blocked. | `src/xenia/kernel/util/shim_utils.cc` | 70, 277, 281, 788 | 4 |
| `guest_arg0` | KernelCallThreadState field | Guest r3 at the thread's last kernel call (the handle for NtWaitForSingleObjectEx). | `src/xenia/kernel/util/shim_utils.cc` | 71, 244, 789 | 3 |
| `guest_arg1` | KernelCallThreadState field | Guest r4 at the thread's last kernel call. | `src/xenia/kernel/util/shim_utils.cc` | 72, 276, 790 | 3 |
| `kernel_call_ms` | local | How long the logged thread has been inside, or out of, its last call. | `src/xenia/kernel/util/shim_utils.cc` | 229, 237, 243 | 3 |
| `kernel_call_handle` | local | The blocked thread's r3, looked up in the object table. | `src/xenia/kernel/util/shim_utils.cc` | 244, 254, 275 | 3 |
| `kernel_call_object` | local | Text naming a wait object, or the guest function / host symbol a sampled host PC is in. | `src/xenia/kernel/util/shim_utils.cc` | 245, 255, 262, 272, 276, 354, 365, 370, 380, 396, 429 | 11 |
| `kernel_call_xobject` | local | Object-table lookup result for that handle. | `src/xenia/kernel/util/shim_utils.cc` | 252, 257, 258, 259, 261 | 5 |
| `kernel_call_total` | local | This thread's call count including this call. | `src/xenia/kernel/util/shim_utils.cc` | 794, 827, 849 | 3 |
| `kernel_call_history_lr` | thread_local array | Guest return address of each call in kernel_call_history. | `src/xenia/kernel/util/shim_utils.cc` | 802, 815, 843 | 3 |
| `kernel_call_history_head` | thread_local | Calls this thread has recorded; the next ring slot is head & 31. | `src/xenia/kernel/util/shim_utils.cc` | 797, 803, 807, 814, 815, 817, 837, 881, 899 | 9 |
| `kernel_call_slot` | local in NoteKernelCall | Ring slot of the call being listed in a storm report. | `src/xenia/kernel/util/shim_utils.cc` | 836, 838, 842, 843 | 4 |
| `kernel_call_storm_ms` | thread_local | Uptime at the last millionth call on this thread. | `src/xenia/kernel/util/shim_utils.cc` | 825, 829, 830 | 3 |
| `kernel_call_storm_reports` | thread_local | How many million-call bursts under a second this thread has had (every tenth is logged). | `src/xenia/kernel/util/shim_utils.cc` | 826, 832, 855 | 3 |
| `kernel_call_storm_elapsed` | local | Milliseconds this thread took for its last million calls. | `src/xenia/kernel/util/shim_utils.cc` | 828, 831, 849 | 3 |
| `kernel_call_global` | local | The global critical region mutex, only try-locked so a report never waits on a stuck thread (wait-object lookup, guest-thread list, stack walk). | `src/xenia/kernel/util/shim_utils.cc` | 101, 102, 128, 250, 251, 270, 334, 335, 341, 405, 416, 441, 460 | 13 |
| `kernel_call_xthreads` | local | Every XThread in the object table. | `src/xenia/kernel/util/shim_utils.cc` | 339, 342 | 2 |
| `kernel_call_xthread` | local | The XThread being listed. | `src/xenia/kernel/util/shim_utils.cc` | 342, 343, 348, 406, 408, 411, 412, 413, 414, 421, 422, 427, 428, 456 | 15 |
| `kernel_call_host_pc` | local / parameter | Host program counter of a thread (in SampleKernelCallHostPc and the guest-thread list). | `src/xenia/kernel/util/shim_utils.cc` | 194, 207, 213, 350, 358, 359, 364, 390, 393, 398, 429 | 11 |
| `kernel_call_host_thread` | local / parameter | A running guest thread's host thread, to sample. | `src/xenia/kernel/util/shim_utils.cc` | 193, 197, 347, 351, 352, 375 | 6 |
| `kernel_call_port` | local | Mach thread port of the host thread (in SampleKernelCallHostPc). | `src/xenia/kernel/util/shim_utils.cc` | 196, 198, 199, 200, 204, 210 | 6 |
| `kernel_call_arm_state` | local | ARM64 register state read with thread_get_state (in SampleKernelCallHostPc). | `src/xenia/kernel/util/shim_utils.cc` | 201, 205, 208 | 3 |
| `kernel_call_state_count` | local | Size argument for thread_get_state (in SampleKernelCallHostPc). | `src/xenia/kernel/util/shim_utils.cc` | 202, 206 | 2 |
| `kernel_call_function` | local | JIT guest function containing a sampled host PC, from the code cache. | `src/xenia/kernel/util/shim_utils.cc` | 357, 362, 363, 367, 376, 381, 384 | 7 |
| `kernel_call_context` | local | That thread's guest PPC context, for lr and r1. | `src/xenia/kernel/util/shim_utils.cc` | 420, 430, 431, 432, 436, 437, 438 | 8 |
| `KernelCallReadable` | function in xe::kernel::shim | Whether a guest memory range is committed and readable, so the reports and the crash dump never read a page an exited thread released. | `src/xenia/kernel/util/shim_utils.cc` | 87, 110, 119, 294, 406, 408, 444 | 7 |
| `kernel_call_heap` | local | Guest heap containing the address being checked. | `src/xenia/kernel/util/shim_utils.cc` | 88, 89, 92 | 3 |
| `kernel_call_symbol` | local | dladdr() result naming the host function a sampled PC is in. | `src/xenia/kernel/util/shim_utils.cc` | 392, 394, 395, 397, 398 | 5 |
| `SampleKernelCallHostPc` | function (anonymous namespace) | Reads another thread's host PC while it is suspended for a moment (Apple ARM64; 0 elsewhere). | `src/xenia/kernel/util/shim_utils.cc` | 192, 352, 375 | 3 |
| `kernel_call_code_cache` | local | The JIT code cache, to map sampled host PCs to guest functions. | `src/xenia/kernel/util/shim_utils.cc` | 355, 359, 378 | 3 |
| `kernel_call_guest_pc` | local | Guest address of the first sample for a thread running game code; 0 if it is not in game code. | `src/xenia/kernel/util/shim_utils.cc` | 361, 363, 367, 441, 443 | 5 |
| `kernel_call_sample` | local | Index of the extra samples (7, 1 ms apart). | `src/xenia/kernel/util/shim_utils.cc` | 371, 372 | 3 |
| `kernel_call_sample_pc` | local | Host PC of one extra sample. | `src/xenia/kernel/util/shim_utils.cc` | 374, 377, 378, 385 | 4 |
| `LogKernelCallThreads` | function, kernel shim | Logs every guest thread: inside or last kernel call, for how long, guest lr, call count; for a thread blocked a second or more, its wait object and guest stack. Also called when the iOS app stops a game. | `src/xenia/kernel/util/shim_utils.h` | 554 | 1 |
| `LogKernelCallThreads` | function, kernel shim | Logs every guest thread: inside or last kernel call, for how long, guest lr, call count; for a thread blocked a second or more, its wait object and guest stack. Also called when the iOS app stops a game. | `src/xenia/app/xenia_main_ios.mm` | 641 | 1 |

| `MetalPerfCounters` | struct | Performance counters filled in across the Metal backend, logged and reset with the draw stats. | `src/xenia/gpu/metal/metal_perf_counters.h` | 26, 76 | 2 |
| `gpu_us` | MetalPerfCounters field | GPU execution time of completed submission command buffers (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 28 | 1 |
| `gpu_us` | MetalPerfCounters field | GPU execution time of completed submission command buffers (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3032, 7197 | 2 |
| `gpu_command_buffers` | MetalPerfCounters field | Number of those command buffers. | `src/xenia/gpu/metal/metal_perf_counters.h` | 29 | 1 |
| `gpu_command_buffers` | MetalPerfCounters field | Number of those command buffers. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3045, 7201 | 2 |
| `cpu_wait_us` | MetalPerfCounters field | Time the CPU spent blocked in waitUntilCompleted (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 31, 90 | 2 |
| `cpu_wait_us` | MetalPerfCounters field | Time the CPU spent blocked in waitUntilCompleted (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3049 | 1 |
| `cpu_waits` | MetalPerfCounters field | Number of those waits. | `src/xenia/gpu/metal/metal_perf_counters.h` | 32, 92 | 2 |
| `cpu_waits` | MetalPerfCounters field | Number of those waits. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3048 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/metal_perf_counters.h` | 34 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/msl_shader.cc` | 2177 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3051 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 35 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/msl_shader.cc` | 2178 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3053 | 1 |
| `pipeline_compiles` | MetalPerfCounters field | Render pipeline states created. | `src/xenia/gpu/metal/metal_perf_counters.h` | 37 | 1 |
| `pipeline_compiles` | MetalPerfCounters field | Render pipeline states created. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1213, 3056, 9293 | 3 |
| `pipeline_compile_us` | MetalPerfCounters field | Time spent creating them (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 38 | 1 |
| `pipeline_compile_us` | MetalPerfCounters field | Time spent creating them (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 1214, 3058, 9294 | 3 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_perf_counters.h` | 70 | 1 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_shared_memory.cc` | 144 | 1 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3066 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_perf_counters.h` | 76, 90, 92 | 3 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 14 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 32, 5086 | 2 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/msl_shader.cc` | 39, 2177, 2178 | 3 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_shared_memory.cc` | 11, 144 | 2 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_command_processor.cc` | 50, 1025, 1026, 1213, 1214, 2918, 2934, 2941, 3032, 3045, 3048, 3049, 3051, 3053, 3056, 3058, 3061, 3063, 3066, 3090, 3092, 3094, 3096, 3098, 3100, 3108, 3112, 3115, 3133, 3134, 3135, 3136, 3137, 6363, 6908, 7197, 7201, 7502, 7505, 7530, 7548, 7586, 7588, 7614, 7651, 7659, 7662, 7696, 7699, 7928, 7937, 8103, 8106, 8108, 8122, 9293, 9294 | 57 |
| `MetalPerfMicrosecondsSince` | inline function | Microseconds since a steady_clock time point. | `src/xenia/gpu/metal/metal_perf_counters.h` | 78, 91 | 2 |
| `MetalPerfMicrosecondsSince` | inline function | Microseconds since a steady_clock time point. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1023, 1215, 1273, 1338, 9295 | 5 |
| `metal_perf_begin` | parameter / local | Start time of a timed operation. | `src/xenia/gpu/metal/metal_perf_counters.h` | 79, 81, 88, 91 | 4 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_perf_counters.h` | 86 | 1 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1379, 1538, 3127 | 3 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3371, 4433, 4989 | 3 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2191, 2223 | 2 |
| `metal_perf_command_buffer` | parameter | Command buffer waited on. | `src/xenia/gpu/metal/metal_perf_counters.h` | 87, 89 | 2 |
| `MetalPerfThreadCpu` | function (anonymous namespace) | Host CPU share of every thread since the previous call, busiest first, with name, ID and host priority (Apple only). | `src/xenia/gpu/metal/metal_command_processor.cc` | 675, 3071 | 2 |
| `metal_perf_text` | local | Text of the thread CPU report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 676, 736, 742, 748, 750 | 5 |
| `metal_perf_previous_ns` | function static map | CPU time per system thread ID at the previous report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 678, 705, 707, 729 | 4 |
| `metal_perf_current_ns` | local map | CPU time per system thread ID now. | `src/xenia/gpu/metal/metal_command_processor.cc` | 679, 703, 729 | 3 |
| `metal_perf_threads` | local vector | (CPU time in the window, label) per thread. | `src/xenia/gpu/metal/metal_command_processor.cc` | 680, 715, 730, 740, 743, 744 | 7 |
| `metal_perf_list` | local | Thread ports from task_threads. | `src/xenia/gpu/metal/metal_command_processor.cc` | 681, 683, 695, 698, 725, 727 | 6 |
| `metal_perf_count` | local | Number of threads in the process. | `src/xenia/gpu/metal/metal_command_processor.cc` | 682, 683, 689, 728, 738 | 5 |
| `metal_perf_total_ns` | local | CPU time of all threads in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 687, 711, 738 | 3 |
| `metal_perf_index` | local | Loop index over threads. | `src/xenia/gpu/metal/metal_command_processor.cc` | 688, 689, 695, 698, 725, 739, 740, 741, 743, 744 | 11 |
| `metal_perf_identifier` | local | thread_identifier_info (system thread ID). | `src/xenia/gpu/metal/metal_command_processor.cc` | 690, 696, 703, 705, 721 | 5 |
| `metal_perf_info_count` | local | Size argument for THREAD_IDENTIFIER_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 691, 697 | 2 |
| `metal_perf_extended` | local | thread_extended_info (CPU time, name, priority). | `src/xenia/gpu/metal/metal_command_processor.cc` | 692, 699, 701, 702, 713, 718, 719, 722 | 9 |
| `metal_perf_extended_count` | local | Size argument for THREAD_EXTENDED_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 693, 700 | 2 |
| `metal_perf_ns` | local | A thread's total CPU time. | `src/xenia/gpu/metal/metal_command_processor.cc` | 701, 703, 708, 709, 710 | 5 |
| `metal_perf_previous` | local iterator | Its entry from the previous report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 704, 707, 709 | 3 |
| `metal_perf_delta` | local | Its CPU time in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 706, 711, 712, 716 | 4 |
| `metal_perf_window_ns` | local | Window length in ns. | `src/xenia/gpu/metal/metal_command_processor.cc` | 734, 738, 745 | 3 |
| `metal_perf_left` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_command_processor.cc` | 731, 732 | 2 |
| `metal_perf_left` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 648, 649, 857, 859 | 4 |
| `metal_perf_right` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_command_processor.cc` | 731, 732 | 2 |
| `metal_perf_right` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 648, 650, 858, 859 | 4 |
| `metal_perf_footprint_mb` | local | Process physical footprint (task_info TASK_VM_INFO). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3015, 3022, 3069 | 3 |
| `metal_perf_vm_info` | local | TASK_VM_INFO result. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3017, 3020, 3022 | 3 |
| `metal_perf_vm_count` | local | Size argument for TASK_VM_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3018, 3021 | 2 |
| `metal_perf_available_mb` | local | os_proc_available_memory() before iOS ends the app (iOS only). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3025, 3027, 3069 | 3 |
| `metal_perf_window_us` | local | Window length: host time between the window's swaps. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3029, 3047, 3071, 3143 | 4 |
| `metal_perf_gpu_us` | local | GPU time in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3031, 3044, 3047 | 3 |
| `metal_perf_swap_time` | local | Host time of this swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2986, 2990, 2998 | 3 |
| `metal_perf_frame_us` | local | Host time since the previous swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2988, 2992, 2994, 2995, 2996 | 5 |
| `pipeline_compile_begin` | local (two functions) | Start time of a newRenderPipelineState call. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1210, 1215, 9291, 9295 | 4 |
| `completed_buffer` | lambda parameter | Completed submission command buffer, for its GPU times. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7192, 7195 | 3 |
| `metal_perf_gpu_seconds` | local | Its GPUEndTime - GPUStartTime. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7194, 7196, 7198 | 3 |
| `frame_us_total` | DrawStats field | Host time between swaps in the window (us). | `src/xenia/gpu/metal/metal_command_processor.h` | 680 | 1 |
| `frame_us_total` | DrawStats field | Host time between swaps in the window (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2992, 3030, 3042 | 3 |
| `frame_us_max` | DrawStats field | Longest frame in the window (us). | `src/xenia/gpu/metal/metal_command_processor.h` | 681 | 1 |
| `frame_us_max` | DrawStats field | Longest frame in the window (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2993, 2994, 3043 | 3 |
| `frames_over_33ms` | DrawStats field | Frames longer than 33 ms. | `src/xenia/gpu/metal/metal_command_processor.h` | 682 | 1 |
| `frames_over_33ms` | DrawStats field | Frames longer than 33 ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2995, 3043 | 2 |
| `frames_over_50ms` | DrawStats field | Frames longer than 50 ms. | `src/xenia/gpu/metal/metal_command_processor.h` | 683 | 1 |
| `frames_over_50ms` | DrawStats field | Frames longer than 50 ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2996, 3044 | 2 |
| `metal_perf_last_swap_` | MetalCommandProcessor member | Host time of the previous swap. | `src/xenia/gpu/metal/metal_command_processor.h` | 723 | 1 |
| `metal_perf_last_swap_` | MetalCommandProcessor member | Host time of the previous swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2987, 2990, 2998 | 3 |
| `unhandled_host_exception_count` | function static atomic | Unhandled host faults so far, to log only the first ten and then one in a million. | `src/xenia/base/exception_handler_mac.cc` | 231, 233 | 2 |
| `unhandled_count` | local | This fault's number. | `src/xenia/base/exception_handler_mac.cc` | 232, 235, 244, 245, 298 | 6 |
| `unhandled_symbol` | local (first unhandled fault only) | dladdr result for one host address. | `src/xenia/base/exception_handler_mac.cc` | 251, 255, 256, 258, 259 | 5 |
| `unhandled_backtrace` | local (first unhandled fault only) | The pc, lr and frame return addresses, named, as logged. | `src/xenia/base/exception_handler_mac.cc` | 252, 257, 261, 291 | 4 |
| `unhandled_name` | local lambda (first unhandled fault only) | Appends one host address as symbol+offset, or as a number. | `src/xenia/base/exception_handler_mac.cc` | 253, 264, 265, 274 | 4 |
| `address` | lambda parameter of unhandled_name | The host address to name, with its pointer authentication bits removed. | `src/xenia/base/exception_handler_mac.cc` | 253, 254, 255, 259, 261 | 5 |
| `unhandled_stack_top` | local (first unhandled fault only) | Top of the faulting thread's stack; the frame walk stays below it. | `src/xenia/base/exception_handler_mac.cc` | 266, 271 | 2 |
| `unhandled_frame` | local (first unhandled fault only) | Frame record being walked (x29 chain). | `src/xenia/base/exception_handler_mac.cc` | 268, 270, 271, 272, 274, 275, 276, 279 | 9 |
| `unhandled_depth` | loop variable | Frames walked so far (at most 16). | `src/xenia/base/exception_handler_mac.cc` | 269, 270, 273 | 3 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading.h` | 130 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading_posix.cc` | 200 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading_win.cc` | 164 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/gpu/command_processor.cc` | 412 | 1 |
| `fifo_min` | function static in PosixCondition<Thread>::set_priority | Darwin SCHED_FIFO minimum priority, for the 1..32 remap. Ported from XeniOS (same name). | `src/xenia/base/threading_posix.cc` | 932, 935 | 3 |
| `fifo_max` | function static in PosixCondition<Thread>::set_priority | Darwin SCHED_FIFO maximum priority, for the 1..32 remap. Ported from XeniOS (same name). | `src/xenia/base/threading_posix.cc` | 933, 935 | 2 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 31, 83, 89, 114, 117, 119 | 6 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 224, 227, 251, 263, 264, 272, 276, 368, 391, 412, 496, 534, 569, 581 | 14 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 66, 465 | 2 |
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
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_command_processor.cc` | 8087 | 1 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4113, 6188 | 2 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 74, 84 | 2 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 366 | 1 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4126, 4234, 4957, 5173 | 4 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1112 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 75, 86 | 2 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 389 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 5704 | 1 |
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
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1639, 3143, 6869, 8087 | 4 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4113, 4126, 4234, 4957, 5173, 5704, 6188 | 7 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1112, 1241 | 2 |
| `metal_gpu_pass_timing` | cvar (Metal); iOS settings key | Turns pass timing, the traffic estimate, per-pipeline time and compile stall tracking on (default) or off; Diagnostics section toggle. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 10, 19, 217, 221, 737, 750 | 7 |
| `metal_gpu_pass_timing` | cvar (Metal); iOS settings key | Turns pass timing, the traffic estimate, per-pipeline time and compile stall tracking on (default) or off; Diagnostics section toggle. | `src/xenia/ui/windowed_app_main_ios.mm` | 3882 | 1 |
| `metal_gpu_pass_record_` | MetalCommandProcessor member | Record of the current render encoder, for its draw count. | `src/xenia/gpu/metal/metal_command_processor.h` | 465 | 1 |
| `metal_gpu_pass_record_` | MetalCommandProcessor member | Record of the current render encoder, for its draw count. | `src/xenia/gpu/metal/metal_command_processor.cc` | 768, 2161, 2235, 6870, 7801, 8089 | 6 |
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
| `NoteDraw` | MetalGpuPassTimer member function | Counts a guest draw in its pass, per pipeline. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6869 | 1 |
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
| `Queued` | MetalCompileStallTracker member function | A compile was queued. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1068, 1107 | 2 |
| `Skipped` | MetalCompileStallTracker member function | A draw was skipped for a pending compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 162 | 1 |
| `Skipped` | MetalCompileStallTracker member function | A draw was skipped for a pending compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 749 | 1 |
| `Skipped` | MetalCompileStallTracker member function | A draw was skipped for a pending compile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5002, 5033, 5131 | 3 |
| `Finished` | MetalCompileStallTracker member function | A compile finished: buckets it and notes it if draws waited 100 ms or more. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 164, 181 | 2 |
| `Finished` | MetalCompileStallTracker member function | A compile finished: buckets it and notes it if draws waited 100 ms or more. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 762 | 1 |
| `Finished` | MetalCompileStallTracker member function | A compile finished: buckets it and notes it if draws waited 100 ms or more. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1296, 1356 | 2 |
| `Clear` | MetalCompileStallTracker member function | Forgets everything for a new title. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 167 | 1 |
| `Clear` | MetalCompileStallTracker member function | Forgets everything for a new title. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 724 | 1 |
| `Clear` | MetalCompileStallTracker member function | Forgets everything for a new title. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1640 | 1 |
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
| `compile_us` | MetalCompileStall field; Finished parameter; locals in MslShaderCompileThread | Time of the compile itself. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1273, 1301, 1333, 1338, 1362 | 5 |
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
| `metal_compile_stall_tracker` | inline global | The one MetalCompileStallTracker. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1068, 1107, 1296, 1356, 1640, 3144, 5002, 5033, 5131 | 9 |
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
| `XE_TRANSFER_STENCIL_OUTPUT` | MSL preprocessor define (transfer shaders) | 1 when a depth-output transfer shader also writes the guest stencil through [[stencil]]. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 7208, 8382, 8699, 8737, 8795, 8966 | 6 |
| `stencil` | TransferDepthOut member (MSL) | The [[stencil]] output of a depth transfer with native stencil output (only these two lines; the word is common in the file). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 747, 751, 752, 1915, 1976, 2093, 2155, 2272, 2328, 3643, 3644, 4058, 4067, 6126, 6202, 6566, 7138, 8383, 8701, 8702, 8796, 8797, 8967, 9325, 9326, 9328, 9329, 9330, 9331, 9334, 9335, 9336, 9337, 9338, 9340, 9350, 9351, 9362, 9367, 9435, 9436, 9437, 9438, 9439, 9440, 9441, 9442, 9443, 9445, 9585, 9586, 9587, 9588, 9589, 9590, 9591, 9592, 9593, 9595, 9613, 9614, 9615, 9616, 9617, 9618, 9619, 9620, 9621, 9623 | 72 |
| `stencil_packed` | local in transfer_ps (MSL) | Gamma color packed after conversion, as the per-bit stencil draws pack it. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 8703, 8739, 8801 | 3 |
| `stencil_packed_set` | local in transfer_ps (MSL) | stencil_packed was computed (8_8_8_8_GAMMA source). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 8704, 8741, 8801 | 3 |
| `transfer_stencil` | local in transfer_ps (MSL) | Guest stencil written with the depth. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 8800, 8803, 8806, 8967 | 4 |
| `depth_stencil_output_state` | local in PerformTransfersAndResolveClears | GetTransferDepthAndStencilOutputState() for a draw writing stencil with depth. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6568, 6570, 6573 | 3 |
| `named_candidate_count` | local in CompileToMsl | Entry point candidates before those found in the MSL source, which are only searched for when none of them resolves. | `src/xenia/gpu/metal/msl_shader.cc` | 2079, 2082 | 2 |
| `argument_encoder_begin` | local in CompileToMsl | Start of the argument encoder creation, timed for the compile timings line. | `src/xenia/gpu/metal/msl_shader.cc` | 2146, 2171 | 2 |
| `argument_encoder_ms` | local in CompileToMsl | Its time (argument-encoder= in the compile timings line). | `src/xenia/gpu/metal/msl_shader.cc` | 2170, 2188, 2197 | 3 |
| `metal_async_compile_wait_ms` | cvar (Metal); iOS settings key | How long draws may wait per frame for a compile before being skipped; 0 (default) never waits. Settings > Performance > Wait for Shader Compiles. | `src/xenia/gpu/metal/metal_command_processor.cc` | 144, 153, 999 | 3 |
| `metal_async_compile_wait_ms` | cvar (Metal); iOS settings key | How long draws may wait per frame for a compile before being skipped; 0 (default) never waits. Settings > Performance > Wait for Shader Compiles. | `src/xenia/ui/windowed_app_main_ios.mm` | 3654 | 1 |
| `queue_arrival` | MslShaderCompileRequest and MslPipelineCompileRequest field | Queue order within a priority: the latest shader request first (a shader still needed in a later frame is queued again), pipelines first in, first out. | `src/xenia/gpu/metal/metal_command_processor.h` | 580, 600, 613, 622 | 6 |
| `queue_arrival` | MslShaderCompileRequest and MslPipelineCompileRequest field | Queue order within a priority: the latest shader request first (a shader still needed in a later frame is queued again), pipelines first in, first out. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1060, 1065, 1067, 1104, 1258 | 5 |
| `msl_compile_arrivals_` | MetalCommandProcessor member | Compiles queued so far: the next queue_arrival. | `src/xenia/gpu/metal/metal_command_processor.h` | 648 | 1 |
| `msl_compile_arrivals_` | MetalCommandProcessor member | Compiles queued so far: the next queue_arrival. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1060, 1104, 3152 | 3 |
| `msl_compile_completions_` | MetalCommandProcessor member | Compiles finished so far, for waiting draws to see progress. | `src/xenia/gpu/metal/metal_command_processor.h` | 649 | 1 |
| `msl_compile_completions_` | MetalCommandProcessor member | Compiles finished so far, for waiting draws to see progress. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1013, 1019, 1292, 1352 | 4 |
| `msl_compile_done_cv_` | MetalCommandProcessor member | Notified when a compile finishes (and at shutdown). | `src/xenia/gpu/metal/metal_command_processor.h` | 646, 653 | 2 |
| `msl_compile_done_cv_` | MetalCommandProcessor member | Notified when a compile finishes (and at shutdown). | `src/xenia/gpu/metal/metal_command_processor.cc` | 943, 1014, 1294, 1354 | 4 |
| `msl_compile_wait_used_us_` | MetalCommandProcessor member | Time draws waited for compiles in the current frame; reset at each swap. | `src/xenia/gpu/metal/metal_command_processor.h` | 655 | 1 |
| `msl_compile_wait_used_us_` | MetalCommandProcessor member | Time draws waited for compiles in the current frame; reset at each swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1004, 1017, 1024, 3149 | 4 |
| `WaitForMslCompileProgress` | MetalCommandProcessor member function | Waits for any compile to finish within the frame's remaining wait budget. | `src/xenia/gpu/metal/metal_command_processor.h` | 658 | 1 |
| `WaitForMslCompileProgress` | MetalCommandProcessor member function | Waits for any compile to finish within the frame's remaining wait budget. | `src/xenia/gpu/metal/metal_command_processor.cc` | 998, 4997, 5028, 5099 | 4 |
| `compile_waits` | MetalPerfCounters field | Draws that waited for a compile. | `src/xenia/gpu/metal/metal_perf_counters.h` | 40 | 1 |
| `compile_waits` | MetalPerfCounters field | Draws that waited for a compile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1025, 3061 | 2 |
| `compile_wait_us` | MetalPerfCounters field | Time they waited. | `src/xenia/gpu/metal/metal_perf_counters.h` | 41 | 1 |
| `compile_wait_us` | MetalPerfCounters field | Time they waited. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1026, 3063 | 2 |
| `wait_ms` | local in WaitForMslCompileProgress | metal_async_compile_wait_ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 999, 1000, 1003 | 3 |
| `budget_us` | local in WaitForMslCompileProgress | The per-frame budget in us. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1003, 1004, 1017 | 3 |
| `wait_begin` | local in WaitForMslCompileProgress | Start of the wait. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1007, 1016, 1023 | 3 |
| `completions` | local in WaitForMslCompileProgress | msl_compile_completions_ when the wait started. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1013, 1019 | 2 |
| `waited_us` | local in WaitForMslCompileProgress | Time waited. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1023, 1024, 1026 | 3 |
| `metal_render_pass_area_from_scissor` | cvar (Metal); iOS settings key | Limit guest draw render passes to the area their scissors cover, and transfer passes to their rectangles (default off). Settings > Performance > Limit Render Pass Area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 132, 142, 3552, 3556 | 4 |
| `metal_render_pass_area_from_scissor` | cvar (Metal); iOS settings key | Limit guest draw render passes to the area their scissors cover, and transfer passes to their rectangles (default off). Settings > Performance > Limit Render Pass Area. | `src/xenia/ui/windowed_app_main_ios.mm` | 3696 | 1 |
| `render_area_needed_width_` | MetalCommandProcessor member | Right edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 473 | 1 |
| `render_area_needed_width_` | MetalCommandProcessor member | Right edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3554, 3579, 7934, 7949, 7990, 8102 | 6 |
| `render_area_needed_height_` | MetalCommandProcessor member | Bottom edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 474 | 1 |
| `render_area_needed_height_` | MetalCommandProcessor member | Bottom edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3555, 3582, 7935, 7949, 7992 | 5 |
| `current_render_area_width_` | MetalCommandProcessor member | Render area width of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 475 | 1 |
| `current_render_area_width_` | MetalCommandProcessor member | Render area width of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7802, 7933, 7934, 8100, 8171, 8172 | 6 |
| `current_render_area_height_` | MetalCommandProcessor member | Render area height of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 476 | 1 |
| `current_render_area_height_` | MetalCommandProcessor member | Render area height of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7803, 7935, 8101, 8173 | 4 |
| `render_area_by_attachments_` | MetalCommandProcessor member | Largest render area so far per set of attachment textures, so passes stop growing. | `src/xenia/gpu/metal/metal_command_processor.h` | 479 | 1 |
| `render_area_by_attachments_` | MetalCommandProcessor member | Largest render area so far per set of attachment textures, so passes stop growing. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3155, 7988 | 2 |
| `area_scissor` | local in IssueDraw | The draw's scissor. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3557, 3558, 3570, 3580 | 6 |
| `area_scale_x` | local in IssueDraw | Draw resolution scale (x), times the sample grid with msaa_samples_as_pixels. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3559, 3565, 3580 | 3 |
| `area_scale_y` | local in IssueDraw | Draw resolution scale (y), times the sample grid with msaa_samples_as_pixels. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3561, 3567, 3583 | 3 |
| `area_width` | local in BeginCommandBuffer | Render area width for the new encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7947, 7993, 7997, 7999, 8004, 8005, 8090, 8100, 8105, 8154 | 10 |
| `area_height` | local in BeginCommandBuffer | Render area height for the new encoder. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7948, 7995, 7998, 8000, 8006, 8101, 8109, 8154 | 8 |
| `attachments_width` | local in BeginCommandBuffer | Smallest attachment width, the most the area may be. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7950, 7960, 7961, 7971, 7994, 7997 | 6 |
| `attachments_height` | local in BeginCommandBuffer | Smallest attachment height. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7951, 7962, 7963, 7996, 7998 | 5 |
| `clears_on_load` | local in BeginCommandBuffer | An attachment is cleared on load: keep the whole area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7952, 7964, 7971 | 3 |
| `limit_to_attachment` | lambda in BeginCommandBuffer | Takes an attachment's size and load action into account. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7953, 7967, 7969, 7970 | 4 |
| `known_area` | local in BeginCommandBuffer | render_area_by_attachments_ entry. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3155, 3156, 3157, 3158, 3159, 7988, 7989, 7990, 7991, 7992, 7994, 7995 | 16 |
| `pass_width` | local in MetalGpuPassTimer::RenderEncoder | Render area width, or the attachment's, for the traffic estimate. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 290, 293, 294, 299, 311, 325 | 6 |
| `pass_height` | local in MetalGpuPassTimer::RenderEncoder | Render area height, or the attachment's. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 291, 295, 296, 300, 311, 327 | 6 |
| `transfer_area_width` | local in PerformTransfersAndResolveClears | Render area width of the transfer pass (0: whole target), for its full-target scissor. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6097, 6148, 6496, 6499 | 4 |
| `transfer_area_height` | local in PerformTransfersAndResolveClears | Render area height of the transfer pass (0: whole target). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6098, 6149, 6500 | 3 |
| `area_right` | local in ensure_transfer_encoder | Right edge of the rectangles drawn in the transfer pass. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6113, 6122, 6142, 6144 | 5 |
| `area_bottom` | local in ensure_transfer_encoder | Bottom edge of the rectangles drawn in the transfer pass. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6114, 6123, 6142, 6146 | 5 |
| `add_to_area` | lambda in ensure_transfer_encoder | Extends the area by a (scaled) rectangle. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6115, 6136, 6140 | 3 |
| `metal_render_pass_area_from_scissor` | cvar (Metal); iOS settings key | Limit guest draw render passes to the area their scissors cover, and transfer passes to their rectangles (default off). Settings > Performance > Limit Render Pass Area. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 73, 6095, 6111 | 3 |
| `log_guest_thread_hotspots` | cvar (Logging); iOS settings key | Sample running guest threads with the Metal perf lines and log where busy ones spend their time (default on). Settings > Diagnostics > Guest Thread Hotspots. | `src/xenia/kernel/kernel_flags.cc` | 24 | 1 |
| `log_guest_thread_hotspots` | cvar (Logging); iOS settings key | Sample running guest threads with the Metal perf lines and log where busy ones spend their time (default on). Settings > Diagnostics > Guest Thread Hotspots. | `src/xenia/kernel/kernel_flags.h` | 17 | 1 |
| `log_guest_thread_hotspots` | cvar (Logging); iOS settings key | Sample running guest threads with the Metal perf lines and log where busy ones spend their time (default on). Settings > Diagnostics > Guest Thread Hotspots. | `src/xenia/kernel/util/shim_utils.cc` | 604 | 1 |
| `log_guest_thread_hotspots` | cvar (Logging); iOS settings key | Sample running guest threads with the Metal perf lines and log where busy ones spend their time (default on). Settings > Diagnostics > Guest Thread Hotspots. | `src/xenia/ui/windowed_app_main_ios.mm` | 3889 | 1 |
| `LogGuestThreadHotspots` | function, kernel shim | Samples every running guest thread 16 times, 1 ms apart, and logs where each busy one spends its time (Apple ARM64). | `src/xenia/kernel/util/shim_utils.h` | 558 | 1 |
| `LogGuestThreadHotspots` | function, kernel shim | Samples every running guest thread 16 times, 1 ms apart, and logs where each busy one spends its time (Apple ARM64). | `src/xenia/kernel/util/shim_utils.cc` | 603 | 1 |
| `LogGuestThreadHotspots` | function, kernel shim | Samples every running guest thread 16 times, 1 ms apart, and logs where each busy one spends its time (Apple ARM64). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3145 | 1 |
| `HotspotThread` | struct in LogGuestThreadHotspots | A sampled thread and where its samples were. | `src/xenia/kernel/util/shim_utils.cc` | 615, 623, 651, 682 | 4 |
| `xthread` | HotspotThread field | The guest thread (kept referenced). | `src/xenia/kernel/util/shim_utils.cc` | 616, 710 | 2 |
| `host_thread` | HotspotThread field | Its host thread, sampled. | `src/xenia/kernel/util/shim_utils.cc` | 617, 653, 710 | 3 |
| `places` | HotspotThread field | Samples per place: guest function address or host symbol. | `src/xenia/kernel/util/shim_utils.cc` | 619, 679, 685, 698 | 4 |
| `guest_samples` | HotspotThread field | Samples in guest (JIT) code. | `src/xenia/kernel/util/shim_utils.cc` | 620, 662, 684, 711 | 4 |
| `samples` | HotspotThread field | Samples taken. | `src/xenia/kernel/util/shim_utils.cc` | 621, 657, 683, 709, 711 | 5 |
| `kHotspotRounds` | constant in LogGuestThreadHotspots | Sampling rounds (16). | `src/xenia/kernel/util/shim_utils.cc` | 645, 646 | 2 |
| `hotspot_global` | local | The global critical region, try-locked for the object table. | `src/xenia/kernel/util/shim_utils.cc` | 608, 609, 614 | 3 |
| `hotspot_xthreads` | local | Every XThread. | `src/xenia/kernel/util/shim_utils.cc` | 612, 624 | 2 |
| `hotspot_threads` | local | The running guest threads being sampled. | `src/xenia/kernel/util/shim_utils.cc` | 623, 627, 631, 651, 682 | 5 |
| `hotspot_xthread` | loop variable | One XThread. | `src/xenia/kernel/util/shim_utils.cc` | 624, 625, 626, 628 | 6 |
| `hotspot_code_cache` | local | JIT code cache, to map host PCs to guest functions. | `src/xenia/kernel/util/shim_utils.cc` | 634, 660 | 2 |
| `hotspot_round` | loop variable | Sampling round. | `src/xenia/kernel/util/shim_utils.cc` | 646, 647, 648 | 4 |
| `hotspot_thread` | loop variable | A sampled thread. | `src/xenia/kernel/util/shim_utils.cc` | 651, 653, 657, 662, 679, 682, 684, 685, 698, 710, 711 | 13 |
| `hotspot_pc` | local | Its host program counter. | `src/xenia/kernel/util/shim_utils.cc` | 652, 654, 660, 666, 674 | 5 |
| `hotspot_place` | local; structured binding | Guest function address or host symbol of a sample. | `src/xenia/kernel/util/shim_utils.cc` | 658, 661, 676, 679, 685, 686, 687, 688, 689, 690, 698, 699 | 12 |
| `hotspot_function` | local | Guest function at the PC. | `src/xenia/kernel/util/shim_utils.cc` | 659, 661, 663, 664, 665 | 5 |
| `hotspot_symbol` | local | dladdr result for host code. | `src/xenia/kernel/util/shim_utils.cc` | 673, 674, 675, 676 | 4 |
| `hotspot_busy` | local | The thread had samples outside host waits. | `src/xenia/kernel/util/shim_utils.cc` | 684, 691, 694 | 3 |
| `hotspot_count` | structured binding | Samples at a place. | `src/xenia/kernel/util/shim_utils.cc` | 685, 698, 699 | 3 |
| `hotspot_sorted` | local | Places by sample count. | `src/xenia/kernel/util/shim_utils.cc` | 697, 699, 701, 703, 705, 706 | 7 |
| `hotspot_text` | local | The logged list. | `src/xenia/kernel/util/shim_utils.cc` | 702, 704, 711 | 3 |
| `hotspot_totals` | function static map, LogGuestThreadHotspots | Samples since launch per guest function. | `src/xenia/kernel/util/shim_utils.cc` | 640, 663, 718 | 3 |
| `hotspot_total_lines` | function static map, LogGuestThreadHotspots | Samples since launch per function and guest instruction (function << 32 | instruction). | `src/xenia/kernel/util/shim_utils.cc` | 641, 664, 730, 731 | 4 |
| `hotspot_dumped` | function static set, LogGuestThreadHotspots | Functions whose code has been logged. | `src/xenia/kernel/util/shim_utils.cc` | 642, 744 | 2 |
| `hotspot_sorted` | local (second, totals report) | Functions by samples since launch; same name as the per-thread list above, in a later scope. | `src/xenia/kernel/util/shim_utils.cc` | 716, 719, 722, 725, 726, 741, 743 | 8 |
| `hotspot_text` | local (second, totals report) | The logged totals list. | `src/xenia/kernel/util/shim_utils.cc` | 723, 739, 764 | 3 |
| `hotspot_total` | local | All samples since launch, for percentages. | `src/xenia/kernel/util/shim_utils.cc` | 717, 720, 741, 762, 763 | 5 |
| `hotspot_memory` | local | Guest memory, to read the code of hot functions. | `src/xenia/kernel/util/shim_utils.cc` | 724, 754 | 2 |
| `hotspot_address` | structured binding; local | Guest function address in the totals. | `src/xenia/kernel/util/shim_utils.cc` | 718, 719, 726, 727, 730, 732, 740, 744, 748 | 9 |
| `hotspot_count` | structured binding (second, totals report) | Samples of that function. | `src/xenia/kernel/util/shim_utils.cc` | 718, 719, 720 | 3 |
| `hotspot_line` | local | The function's most sampled guest instruction. | `src/xenia/kernel/util/shim_utils.cc` | 727, 736, 742, 748 | 4 |
| `hotspot_line_count` | local | Samples at hotspot_line. | `src/xenia/kernel/util/shim_utils.cc` | 728, 734, 735, 742 | 4 |
| `it` | loop iterator | Walks hotspot_total_lines for one function. | `src/xenia/kernel/util/shim_utils.cc` | 729, 731, 732, 733, 734, 735, 736 | 7 |
| `i` | loop variable (totals report) | Rank in the totals. | `src/xenia/kernel/util/shim_utils.cc` | 725, 726, 740, 741, 743 | 8 |
| `hotspot_start` | local | First guest address of the logged code. | `src/xenia/kernel/util/shim_utils.cc` | 747, 755, 757 | 3 |
| `row` | loop variable | Line of eight words in the code dump. | `src/xenia/kernel/util/shim_utils.cc` | 749, 755, 757 | 5 |
| `word` | loop variable | Word within that line. | `src/xenia/kernel/util/shim_utils.cc` | 751, 755 | 4 |
| `metal_shader_preserve_invariance` | cvar (GPU); iOS settings key | Compile Metal shaders with preserveInvariance so [[invariant]] positions are bit-identical across vertex shaders (default on). Settings > Accuracy > Consistent Vertex Positions. | `src/xenia/gpu/metal/msl_shader.cc` | 52, 2021 | 2 |
| `metal_shader_preserve_invariance` | cvar (GPU); iOS settings key | Compile Metal shaders with preserveInvariance so [[invariant]] positions are bit-identical across vertex shaders (default on). Settings > Accuracy > Consistent Vertex Positions. | `src/xenia/ui/windowed_app_main_ios.mm` | 3717 | 1 |
| `last_update_height_used_` | RenderTargetCache member | Height in guest pixels the last Update estimated the draw uses (draw_extent_estimator_), 0 if not estimated. | `src/xenia/gpu/render_target_cache.h` | 226, 673 | 2 |
| `last_update_height_used_` | RenderTargetCache member | Height in guest pixels the last Update estimated the draw uses (draw_extent_estimator_), 0 if not estimated. | `src/xenia/gpu/render_target_cache.cc` | 623, 841 | 2 |
| `last_update_height_used` | RenderTargetCache member function | Getter for last_update_height_used_. | `src/xenia/gpu/render_target_cache.h` | 226 | 1 |
| `last_update_height_used` | RenderTargetCache member function | Getter for last_update_height_used_. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3575, 3577 | 2 |
| `area_bottom` | local in IssueDraw | Bottom of the draw's needed render area: scissor bottom, clamped to last_update_height_used(). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3570, 3576, 3583 | 4 |
| `render_area_draw_passes` | MetalPerfCounters field | Guest draw render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_perf_counters.h` | 50 | 1 |
| `render_area_draw_passes` | MetalPerfCounters field | Guest draw render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3090, 8103 | 2 |
| `render_area_draw_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_perf_counters.h` | 51 | 1 |
| `render_area_draw_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3092, 8106 | 2 |
| `render_area_draw_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_perf_counters.h` | 52 | 1 |
| `render_area_draw_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3094, 8108 | 2 |
| `render_area_restarts` | MetalPerfCounters field | Draw passes restarted to grow their render area. | `src/xenia/gpu/metal/metal_perf_counters.h` | 53 | 1 |
| `render_area_restarts` | MetalPerfCounters field | Draw passes restarted to grow their render area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3108, 7937 | 2 |
| `render_area_transfer_passes` | MetalPerfCounters field | Transfer render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_perf_counters.h` | 54 | 1 |
| `render_area_transfer_passes` | MetalPerfCounters field | Transfer render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3096 | 1 |
| `render_area_transfer_passes` | MetalPerfCounters field | Transfer render passes created with the render area switch on. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6108 | 1 |
| `render_area_transfer_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_perf_counters.h` | 55 | 1 |
| `render_area_transfer_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3098 | 1 |
| `render_area_transfer_limited` | MetalPerfCounters field | Of those, passes with a limited render area. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6152 | 1 |
| `render_area_transfer_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_perf_counters.h` | 56 | 1 |
| `render_area_transfer_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3100 | 1 |
| `render_area_transfer_rows` | MetalPerfCounters field | Sum of their render area heights. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6154 | 1 |
| `occlusion_query_events` | MetalPerfCounters field | Guest EVENT_WRITE_ZPD events (query begins and ends). | `src/xenia/gpu/metal/metal_perf_counters.h` | 59 | 1 |
| `occlusion_query_events` | MetalPerfCounters field | Guest EVENT_WRITE_ZPD events (query begins and ends). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3115, 7502 | 2 |
| `occlusion_queries` | MetalPerfCounters field | Guest occlusion query ends. | `src/xenia/gpu/metal/metal_perf_counters.h` | 60 | 1 |
| `occlusion_queries` | MetalPerfCounters field | Guest occlusion query ends. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3112, 7505 | 2 |
| `take` | lambda in IssueSwap | Reads and resets a counter. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3086, 3090, 3092, 3094, 3096, 3098, 3100, 3108, 3112, 3115, 3133, 3134, 3135, 3136, 3137 | 15 |
| `area_draw_passes` | local in IssueSwap | render_area_draw_passes for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3089, 3106 | 2 |
| `area_draw_limited` | local in IssueSwap | render_area_draw_limited for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3091, 3106, 3107 | 4 |
| `area_draw_rows` | local in IssueSwap | render_area_draw_rows for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3093, 3107 | 2 |
| `area_transfer_passes` | local in IssueSwap | render_area_transfer_passes for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3095, 3109 | 2 |
| `area_transfer_limited` | local in IssueSwap | render_area_transfer_limited for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3097, 3108, 3110 | 4 |
| `area_transfer_rows` | local in IssueSwap | render_area_transfer_rows for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3099, 3110 | 2 |
| `area_attachments` | structured binding in IssueSwap, local in BeginCommandBuffer | Attachment textures keying render_area_by_attachments_. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3155, 7977, 7979, 7983, 7986, 7988 | 6 |
| `accuracy` | local in BuildIOSConfigSections | The Accuracy settings section. | `src/xenia/ui/windowed_app_main_ios.mm` | 3712, 3713, 3714, 3717, 3722, 3726, 3744, 3750, 3751 | 9 |
| `metal_uniform_buffers_max` | cvar (GPU) | Most SPIRV-Cross uniform buffers (8 draws each) in flight before draws wait for the GPU; 0 for the default (96 on iOS, 12 elsewhere). | `src/xenia/gpu/metal/metal_command_processor.cc` | 112, 185, 187, 188 | 4 |
| `MetalUniformBuffersMax` | function (anonymous namespace) | metal_uniform_buffers_max, or its platform default. | `src/xenia/gpu/metal/metal_command_processor.cc` | 186, 3084, 8247, 8476 | 4 |
| `uniform_waits` | MetalPerfCounters field | Draws that waited for a free SPIRV-Cross uniform buffer. | `src/xenia/gpu/metal/metal_perf_counters.h` | 44 | 1 |
| `uniform_waits` | MetalPerfCounters field | Draws that waited for a free SPIRV-Cross uniform buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3077, 8414 | 2 |
| `uniform_wait_us` | MetalPerfCounters field | Microseconds those draws waited. | `src/xenia/gpu/metal/metal_perf_counters.h` | 45 | 1 |
| `uniform_wait_us` | MetalPerfCounters field | Microseconds those draws waited. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3079, 8415 | 2 |
| `uniform_rollovers` | MetalPerfCounters field | Command buffers ended because no uniform buffer was free and the pool was at its cap. | `src/xenia/gpu/metal/metal_perf_counters.h` | 46 | 1 |
| `uniform_rollovers` | MetalPerfCounters field | Command buffers ended because no uniform buffer was free and the pool was at its cap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3082, 8522 | 2 |
| `uniform_wait_begin` | local in EnsureSpirvUniformBuffer | Start time of a wait for a free uniform buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8411, 8416 | 2 |
| `float_constants_vertex_size` | local in IssueDrawMsl | Bytes of packed float constants the vertex shader reads (float_count x 16). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5570, 5574 | 2 |
| `float_constants_pixel_size` | local in IssueDrawMsl | Bytes of packed float constants the pixel shader reads (0 without one). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5576, 5585 | 2 |
| `ProcessCompletedZPDQueries` | MetalCommandProcessor member function | occlusion_query_enable: reads the counts of guest occlusion queries whose command buffers completed, releases their slots and corrects the guess written at their end. | `src/xenia/gpu/metal/metal_command_processor.h` | 198 | 1 |
| `ProcessCompletedZPDQueries` | MetalCommandProcessor member function | occlusion_query_enable: reads the counts of guest occlusion queries whose command buffers completed, releases their slots and corrects the guess written at their end. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7239, 7541, 7623 | 3 |
| `MetalZPDQuery` | struct in MetalCommandProcessor | A guest occlusion query counted with Metal visibility results: report address, slots, guess written at its end. | `src/xenia/gpu/metal/metal_command_processor.h` | 485, 511, 514 | 3 |
| `MetalZPDQuery` | struct in MetalCommandProcessor | A guest occlusion query counted with Metal visibility results: report address, slots, guess written at its end. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7556, 7636 | 2 |
| `address` | MetalZPDQuery field | Guest report address the count goes to, 0 for an abandoned query. | `src/xenia/gpu/metal/metal_command_processor.h` | 487 | 1 |
| `address` | MetalZPDQuery field | Guest report address the count goes to, 0 for an abandoned query. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7523, 7572, 7650, 7653, 7682, 7689, 7704 | 7 |
| `submission` | MetalZPDQuery field | Command buffer of its last segment; read once it completes. | `src/xenia/gpu/metal/metal_command_processor.h` | 489 | 1 |
| `submission` | MetalZPDQuery field | Command buffer of its last segment; read once it completes. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7524, 7601, 7635 | 3 |
| `ticket` | MetalZPDQuery field | zpd_query_ends_ at its end; only the latest query of an address is corrected. | `src/xenia/gpu/metal/metal_command_processor.h` | 491 | 1 |
| `ticket` | MetalZPDQuery field | zpd_query_ends_ at its end; only the latest query of an address is corrected. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7573, 7574, 7690 | 3 |
| `guess` | MetalZPDQuery field | Count written to the guest at its end. | `src/xenia/gpu/metal/metal_command_processor.h` | 493 | 1 |
| `guess` | MetalZPDQuery field | Count written to the guest at its end. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7600, 7691, 7692, 7704 | 5 |
| `inexact` | MetalZPDQuery field | A draw was skipped for a pending compile, or found no slot or pass to count in: the guess stays. | `src/xenia/gpu/metal/metal_command_processor.h` | 501 | 1 |
| `inexact` | MetalZPDQuery field | A draw was skipped for a pending compile, or found no slot or pass to count in: the guess stays. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6480, 6492, 7561, 7564, 7580, 7638, 7923 | 7 |
| `slots` | MetalZPDQuery field | {index, generation} of the pool slot of each render pass it counted in. | `src/xenia/gpu/metal/metal_command_processor.h` | 503 | 1 |
| `slots` | MetalZPDQuery field | {index, generation} of the pool slot of each render pass it counted in. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6489, 7514, 7525, 7580, 7639 | 5 |
| `zpd_query_active_` | MetalCommandProcessor member | A guest occlusion query began and has not ended. | `src/xenia/gpu/metal/metal_command_processor.h` | 509 | 1 |
| `zpd_query_active_` | MetalCommandProcessor member | A guest occlusion query began and has not ended. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2242, 6411, 6480, 7520, 7522, 7546, 7557, 7559, 7923 | 9 |
| `zpd_segment_open_` | MetalCommandProcessor member | A pool slot is counting in the current render encoder for that query. | `src/xenia/gpu/metal/metal_command_processor.h` | 510 | 1 |
| `zpd_segment_open_` | MetalCommandProcessor member | A pool slot is counting in the current render encoder for that query. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2243, 6480, 6490, 7509, 7515, 7798 | 6 |
| `zpd_query_` | MetalCommandProcessor member | The query being counted. | `src/xenia/gpu/metal/metal_command_processor.h` | 511 | 1 |
| `zpd_query_` | MetalCommandProcessor member | The query being counted. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6480, 6489, 6492, 7514, 7523, 7524, 7525, 7526, 7545, 7558, 7923 | 11 |
| `zpd_compile_pending_at_begin_` | MetalCommandProcessor member | draw_stats_.compile_pending at the query begin; a change by its end means draws were skipped. | `src/xenia/gpu/metal/metal_command_processor.h` | 512 | 1 |
| `zpd_compile_pending_at_begin_` | MetalCommandProcessor member | draw_stats_.compile_pending at the query begin; a change by its end means draws were skipped. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7547, 7560 | 2 |
| `zpd_query_ends_` | MetalCommandProcessor member | Query ends so far, giving each its ticket. | `src/xenia/gpu/metal/metal_command_processor.h` | 490, 513 | 2 |
| `zpd_query_ends_` | MetalCommandProcessor member | Query ends so far, giving each its ticket. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7573 | 1 |
| `zpd_queries_in_flight_` | MetalCommandProcessor member | Ended (and abandoned) queries waiting for their command buffers. | `src/xenia/gpu/metal/metal_command_processor.h` | 514 | 1 |
| `zpd_queries_in_flight_` | MetalCommandProcessor member | Ended (and abandoned) queries waiting for their command buffers. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2245, 3138, 7526, 7602, 7624, 7634, 7635, 7636, 7710 | 9 |
| `zpd_results_by_address_` | MetalCommandProcessor member | Per guest report address: last count (the next guess) and the ticket of its latest query. | `src/xenia/gpu/metal/metal_command_processor.h` | 517 | 1 |
| `zpd_results_by_address_` | MetalCommandProcessor member | Per guest report address: last count (the next guess) and the ticket of its latest query. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7566, 7567, 7568, 7571, 7682, 7683 | 6 |
| `zpd_queries_measured` | MetalPerfCounters field | Query ends given a counted result. | `src/xenia/gpu/metal/metal_perf_counters.h` | 64 | 1 |
| `zpd_queries_measured` | MetalPerfCounters field | Query ends given a counted result. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3133, 7586, 7659 | 3 |
| `zpd_queries_hidden` | MetalPerfCounters field | Of those, with no samples. | `src/xenia/gpu/metal/metal_perf_counters.h` | 65 | 1 |
| `zpd_queries_hidden` | MetalPerfCounters field | Of those, with no samples. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3134, 7588, 7662 | 3 |
| `zpd_queries_corrected` | MetalPerfCounters field | Of those, written over the guess. | `src/xenia/gpu/metal/metal_perf_counters.h` | 66 | 1 |
| `zpd_queries_corrected` | MetalPerfCounters field | Of those, written over the guess. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3135, 7696 | 2 |
| `zpd_queries_guessed` | MetalPerfCounters field | Query ends left with the guess. | `src/xenia/gpu/metal/metal_perf_counters.h` | 67 | 1 |
| `zpd_queries_guessed` | MetalPerfCounters field | Query ends left with the guess. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3136, 7651 | 2 |
| `zpd_pass_restarts` | MetalPerfCounters field | Draw passes restarted to count a query in. | `src/xenia/gpu/metal/metal_perf_counters.h` | 68 | 1 |
| `zpd_pass_restarts` | MetalPerfCounters field | Draw passes restarted to count a query in. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3137, 7928 | 2 |
| `kQueryFinished` | local in MetalCommandProcessor::ExecutePacketType3_EVENT_WRITE_ZPD (as in the Vulkan override) | 0xFFFFFEED, byte-swapped: the query end marker D3D writes. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7491, 7498, 7499, 7500, 7501 | 5 |
| `sample_count_addr` | local in ExecutePacketType3_EVENT_WRITE_ZPD (as in the Vulkan override) | RB_SAMPLE_COUNT_ADDR, the guest report of the event. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7492, 7496, 7533, 7551, 7567, 7571, 7572, 7618 | 8 |
| `sample_counts` | local in ExecutePacketType3_EVENT_WRITE_ZPD and ProcessCompletedZPDQueries (as in the Vulkan override) | The guest report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7494, 7498, 7499, 7500, 7501, 7518, 7544, 7604, 7605, 7606, 7607, 7608, 7609, 7610, 7613, 7687, 7691, 7692, 7693, 7695 | 22 |
| `is_end` | local in ExecutePacketType3_EVENT_WRITE_ZPD (as in the Vulkan override) | The event ends a query. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7497, 7504, 7520, 7533, 7542 | 5 |
| `real_queries` | local in ExecutePacketType3_EVENT_WRITE_ZPD | occlusion_query_enable is on, on the SPIRV-Cross path, with the pool ready. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7517, 7520, 7529 | 3 |
| `query` | local in ExecutePacketType3_EVENT_WRITE_ZPD and ProcessCompletedZPDQueries | The query ending, or the oldest in flight. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7556, 7558, 7561, 7564, 7572, 7573, 7574, 7575, 7576, 7577, 7580, 7583, 7584, 7593, 7594, 7595, 7597, 7598, 7599, 7600, 7601, 7602, 7636, 7638, 7639, 7650, 7653, 7665, 7671, 7672, 7679, 7680, 7682, 7689, 7690, 7691, 7692, 7704, 7705 | 46 |
| `address_result` | local in ExecutePacketType3_EVENT_WRITE_ZPD and ProcessCompletedZPDQueries | zpd_results_by_address_ entry of the query address. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7570, 7574, 7582, 7593, 7682, 7683, 7684, 7690 | 8 |
| `samples` | local in ExecutePacketType3_EVENT_WRITE_ZPD and ProcessCompletedZPDQueries | Count written at the end; summed slot results. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7579, 7597, 7600, 7604, 7613, 7618, 7637, 7644, 7654, 7655, 7656, 7658 | 12 |
| `scale_area` | local in ProcessCompletedZPDQueries | Draw resolution scale area the counts are divided by. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7630, 7654, 7656 | 4 |
| `slot_samples` | local in ProcessCompletedZPDQueries | Result of one slot. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7640, 7643, 7644 | 3 |
| `counted` | local in ProcessCompletedZPDQueries | Every slot was read and the query is exact. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7638, 7646, 7650 | 3 |
| `count` | local in ProcessCompletedZPDQueries | The normalized count for the guest. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7658, 7661, 7667, 7668, 7680, 7684, 7691, 7693, 7695, 7704 | 12 |
| `slot_index` | local in IssueDrawMsl; structured binding in ProcessCompletedZPDQueries | Pool slot index. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6481, 6485, 6489, 7639, 7641, 7643, 7648 | 7 |
| `slot_generation` | local in IssueDrawMsl; structured binding in ProcessCompletedZPDQueries | Pool slot generation. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6482, 6485, 6489, 7639, 7642, 7648 | 6 |
| `slot_offset` | local in IssueDrawMsl | Byte offset of the slot in the visibility buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6483, 6486, 6488 | 3 |
| `zpd_pass` | local in BeginCommandBuffer | The draw pass must be created with the visibility buffer (a query is being counted). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7923, 7925, 8008, 8095, 8099 | 5 |
| `metal_frame_detail_log` | cvar (GPU); iOS settings key | Changed to N while a game runs: logs every render pass, draw, texture fetch constant, resolve, render target transfer and occlusion query of the next N frames. Settings > Diagnostics > Frame Detail Log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 118, 1617, 2915, 2937, 2938 | 5 |
| `metal_frame_detail_log` | cvar (GPU); iOS settings key | Changed to N while a game runs: logs every render pass, draw, texture fetch constant, resolve, render target transfer and occlusion query of the next N frames. Settings > Frame Capture > Frame Detail Log (first section). | `src/xenia/ui/windowed_app_main_ios.mm` | 3558, 3931 | 2 |
| `frame_detail_frames` | MetalPerfCounters field | Frames left to log in detail; counted down at swaps, read by the render target cache for transfers. | `src/xenia/gpu/metal/metal_perf_counters.h` | 73 | 1 |
| `frame_detail_frames` | MetalPerfCounters field | Frames left to log in detail; counted down at swaps, read by the render target cache for transfers. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2918, 2934, 2941, 6363, 6908, 7530, 7548, 7614, 7699, 8122 | 10 |
| `frame_detail_frames` | MetalPerfCounters field | Frames left to log in detail; counted down at swaps, read by the render target cache for transfers. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 5086 | 1 |
| `frame_detail_setting_` | MetalCommandProcessor member | metal_frame_detail_log when last seen; a change starts a capture. | `src/xenia/gpu/metal/metal_command_processor.h` | 688 | 1 |
| `frame_detail_setting_` | MetalCommandProcessor member | metal_frame_detail_log when last seen; a change starts a capture. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1617, 2937, 2938, 2939, 2940, 2942 | 6 |
| `frame_detail_passes_` | MetalCommandProcessor member | Draw passes so far in the logged frame (their number in the log). | `src/xenia/gpu/metal/metal_command_processor.h` | 693 | 1 |
| `frame_detail_passes_` | MetalCommandProcessor member | Draw passes so far in the logged frame (their number in the log). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2928, 2945, 6389, 8154 | 4 |
| `frame_detail_draws_` | MetalCommandProcessor member | Draws encoded so far in the logged frame (their number in the log). | `src/xenia/gpu/metal/metal_command_processor.h` | 694 | 1 |
| `frame_detail_draws_` | MetalCommandProcessor member | Draws encoded so far in the logged frame (their number in the log). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2946, 6389, 6473, 6915, 6941, 7533, 7551, 7618 | 8 |
| `frame_detail_stats_` | MetalCommandProcessor member | draw_stats_ at the frame start, for the frame's totals. | `src/xenia/gpu/metal/metal_command_processor.h` | 695 | 1 |
| `frame_detail_stats_` | MetalCommandProcessor member | draw_stats_ at the frame start, for the frame's totals. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2921, 2984 | 3 |
| `frame_start` | local in IssueSwap | frame_detail_stats_, or zeros if draw_stats_ was reset since. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2920, 2928, 2929, 2930, 2931, 2932, 2933 | 7 |
| `frame_detail_attachments` | local in BeginCommandBuffer | Host attachments of the new draw pass, for its log line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8127, 8134, 8159 | 3 |
| `add_attachment` | lambda in BeginCommandBuffer | Appends one attachment's size, format, samples and load/store actions. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8128, 8145, 8148 | 3 |
| `kColorNames` | local static in BeginCommandBuffer | Names c0-c3 of the color attachments. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8143, 8145 | 2 |
| `frame_detail_textures` | local in IssueDrawMsl | The draw's texture fetch constants, for its log line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6366, 6374, 6412 | 3 |
| `fetch_bits` | local in IssueDrawMsl | used_texture_mask fetches not yet logged. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6367, 6369, 6370 | 3 |
| `frame_detail_sources` | local in MetalRenderTargetCache::PerformTransfersAndResolveClears | A render target's transfer sources, for its log line. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 5092, 5094, 5103 | 3 |
| `metal_inline_shader_constants` | cvar (GPU); iOS settings key | SPIRV-Cross path: pass each draw's shader constants with setVertexBytes/setFragmentBytes when they change instead of through the pooled 8-draw uniform buffers (default on). Settings > Performance > Inline Shader Constants. | `src/xenia/gpu/metal/metal_command_processor.cc` | 105, 3627, 5388, 7171 | 4 |
| `metal_inline_shader_constants` | cvar (GPU); iOS settings key | SPIRV-Cross path: pass each draw's shader constants with setVertexBytes/setFragmentBytes when they change instead of through the pooled 8-draw uniform buffers (default on). Settings > Performance > Inline Shader Constants. | `src/xenia/ui/windowed_app_main_ios.mm` | 3702 | 1 |
| `inline_constants` | local in IssueDrawMsl | metal_inline_shader_constants for this draw. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5387, 5396, 5407, 5460, 5684, 5745 | 6 |
| `write_constant_block` | lambda in IssueDrawMsl | Passes one constant block with set*Bytes if its version is not bound in this encoder, or writes it to the draw's table. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5453, 5482, 5487, 5572, 5583, 5598, 5602, 5618, 5622, 5629, 5637, 5643 | 12 |
| `pixel_stage` | write_constant_block parameter | The block is for the pixel stage. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5454, 5464, 5472 | 3 |
| `slot` | write_constant_block parameter | Index in msl_inline_constants_bound_versions_. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5454, 5461, 5469 | 3 |
| `buffer_index` | write_constant_block parameter | Metal buffer index (MslBufferIndex). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5454, 5465, 5467 | 3 |
| `offset` | write_constant_block parameter | Offset of the block in the stage's table (table path). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5455, 5474, 5477 | 3 |
| `data` | write_constant_block parameter | The block's data. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5455, 5465, 5467, 5474, 5477 | 5 |
| `size` | write_constant_block parameter | The block's size in bytes; 0 skips it. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5455, 5457, 5465, 5467, 5474, 5477 | 6 |
| `version` | write_constant_block parameter | Version of the block's data. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5455, 5461, 5469, 5475 | 4 |
| `table` | local in write_constant_block | The stage's table in the uniform buffer (table path). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5472, 5474, 5477 | 3 |
| `msl_float_constants_version_vertex_` | MetalCommandProcessor member | Version of the cached packed vertex float constants, bumped when rebuilt. | `src/xenia/gpu/metal/metal_command_processor.h` | 944 | 1 |
| `msl_float_constants_version_vertex_` | MetalCommandProcessor member | Version of the cached packed vertex float constants, bumped when rebuilt. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5559, 5575 | 2 |
| `msl_float_constants_version_pixel_` | MetalCommandProcessor member | Version of the cached packed pixel float constants, bumped when rebuilt. | `src/xenia/gpu/metal/metal_command_processor.h` | 945 | 1 |
| `msl_float_constants_version_pixel_` | MetalCommandProcessor member | Version of the cached packed pixel float constants, bumped when rebuilt. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5566, 5586 | 2 |
| `msl_bool_loop_constants_version_` | MetalCommandProcessor member | Version of the cached bool/loop constants, bumped when recopied. | `src/xenia/gpu/metal/metal_command_processor.h` | 946 | 1 |
| `msl_bool_loop_constants_version_` | MetalCommandProcessor member | Version of the cached bool/loop constants, bumped when recopied. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5596, 5601, 5605 | 3 |
| `msl_fetch_constants_version_` | MetalCommandProcessor member | Version of the cached fetch constants, bumped when recopied. | `src/xenia/gpu/metal/metal_command_processor.h` | 947 | 1 |
| `msl_fetch_constants_version_` | MetalCommandProcessor member | Version of the cached fetch constants, bumped when recopied. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5616, 5620, 5624 | 3 |
| `msl_inline_constants_bound_versions_` | MetalCommandProcessor member | Per constant block slot (11), the version passed with set*Bytes in the current render encoder; 0 for none, cleared with the encoder state. | `src/xenia/gpu/metal/metal_command_processor.h` | 948 | 1 |
| `msl_inline_constants_bound_versions_` | MetalCommandProcessor member | Per constant block slot (11), the version passed with set*Bytes in the current render encoder; 0 for none, cleared with the encoder state. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5461, 5469, 5742, 7754 | 4 |
| `metal_log_shader_hashes` | cvar (GPU) | Comma-separated shader ucode hashes whose microcode disassembly and generated MSL are logged when translated ("Shader log" lines), and whose float constants the frame detail log adds per draw. Settings > Metal (generated section). | `src/xenia/gpu/metal/msl_shader.cc` | 81, 455 | 2 |
| `ShouldLogMslShader` | function (xe::gpu::metal) | metal_log_shader_hashes lists the hash. | `src/xenia/gpu/metal/msl_shader.h` | 26 | 1 |
| `ShouldLogMslShader` | function (xe::gpu::metal) | metal_log_shader_hashes lists the hash. | `src/xenia/gpu/metal/msl_shader.cc` | 454, 1893 | 2 |
| `ShouldLogMslShader` | function (xe::gpu::metal) | metal_log_shader_hashes lists the hash. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6453 | 1 |
| `log_shader_lines` | lambda in MslTranslation::CompileToMsl | Logs a text line by line with the shader's stage, hash and modification. | `src/xenia/gpu/metal/msl_shader.cc` | 1896, 1910, 1911 | 3 |
| `line_begin` | local in log_shader_lines | Start of the line being logged. | `src/xenia/gpu/metal/msl_shader.cc` | 1897, 1898, 1899, 1906, 1907 | 6 |
| `line_end` | local in log_shader_lines | End of the line being logged. | `src/xenia/gpu/metal/msl_shader.cc` | 1899, 1900, 1901, 1906, 1907 | 5 |
| `frame_detail_shader` | loop variable in IssueDrawMsl | The draw's vertex or pixel shader, for its float constants in the frame detail log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6450, 6452, 6453, 6456, 6459, 6474 | 6 |
| `is_pixel` | local in IssueDrawMsl | frame_detail_shader is the pixel shader. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6456, 6464, 6473 | 3 |
| `frame_detail_constants` | local in IssueDrawMsl | The shader's float constants for the log line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6457, 6467, 6474 | 3 |
| `MetalResolveProbe` | struct in MetalCommandProcessor | A depth resolve during a frame detail capture, sampled once its command buffer completes. | `src/xenia/gpu/metal/metal_command_processor.h` | 698, 722 | 2 |
| `MetalResolveProbe` | struct in MetalCommandProcessor | A depth resolve during a frame detail capture, sampled once its command buffer completes. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6931, 8028 | 2 |
| `resolve_probes_` | MetalCommandProcessor member | Depth resolves waiting to be sampled. | `src/xenia/gpu/metal/metal_command_processor.h` | 199, 722 | 2 |
| `resolve_probes_` | MetalCommandProcessor member | Depth resolves waiting to be sampled. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6995, 7244, 7253, 7264, 7482, 8079 | 7 |
| `LogResolveProbes` | MetalCommandProcessor member function | Logs a grid of the values of each completed probe's resolve, read from the shared memory buffer (tiled addressing). | `src/xenia/gpu/metal/metal_command_processor.h` | 201, 697 | 2 |
| `LogResolveProbes` | MetalCommandProcessor member function | Logs a grid of the values of each completed probe's resolve, read from the shared memory buffer (tiled addressing). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7240, 7243 | 2 |
| `address` | MetalResolveProbe field | RB_COPY_DEST_BASE. | `src/xenia/gpu/metal/metal_command_processor.h` | 699 | 1 |
| `address` | MetalResolveProbe field | RB_COPY_DEST_BASE. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6932, 7288, 7298, 7314, 7322, 7402, 7438, 7462 | 8 |
| `written_begin` | MetalResolveProbe field | Start of the range the resolve wrote. | `src/xenia/gpu/metal/metal_command_processor.h` | 701 | 1 |
| `written_begin` | MetalResolveProbe field | Start of the range the resolve wrote. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6933, 7273, 7288, 7300, 7324 | 5 |
| `written_end` | MetalResolveProbe field | End of the range the resolve wrote. | `src/xenia/gpu/metal/metal_command_processor.h` | 702 | 1 |
| `written_end` | MetalResolveProbe field | End of the range the resolve wrote. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6934, 7288, 7301, 7325 | 4 |
| `width` | MetalResolveProbe field | Destination pitch (RB_COPY_DEST_PITCH). | `src/xenia/gpu/metal/metal_command_processor.h` | 703 | 1 |
| `width` | MetalResolveProbe field | Destination pitch (RB_COPY_DEST_PITCH). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6935, 7254, 7281, 7289, 7296, 7299, 7323, 7384, 7385, 7386, 7387, 7413, 7418, 7443, 7449, 8033, 8036 | 18 |
| `height` | MetalResolveProbe field | Destination height (RB_COPY_DEST_PITCH). | `src/xenia/gpu/metal/metal_command_processor.h` | 704 | 1 |
| `height` | MetalResolveProbe field | Destination height (RB_COPY_DEST_PITCH). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6936, 6970, 7254, 7281, 7289, 7293, 7335, 7336, 8038, 8050 | 10 |
| `endian` | MetalResolveProbe field | Destination endianness (RB_COPY_DEST_INFO, 32-bit cases). | `src/xenia/gpu/metal/metal_command_processor.h` | 705 | 1 |
| `endian` | MetalResolveProbe field | Destination endianness (RB_COPY_DEST_INFO, 32-bit cases). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6937, 7309, 7332 | 3 |
| `is_float` | MetalResolveProbe field | The depth is D24FS8 (20e4) rather than 24-bit unorm. | `src/xenia/gpu/metal/metal_command_processor.h` | 707 | 1 |
| `is_float` | MetalResolveProbe field | The depth is D24FS8 (20e4) rather than 24-bit unorm. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6939, 6956, 7289, 7311, 7317 | 5 |
| `draw` | MetalResolveProbe field | frame_detail_draws_ at the resolve. | `src/xenia/gpu/metal/metal_command_processor.h` | 708 | 1 |
| `draw` | MetalResolveProbe field | frame_detail_draws_ at the resolve. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6941, 7288 | 2 |
| `submission` | MetalResolveProbe field | The command buffer of the resolve. | `src/xenia/gpu/metal/metal_command_processor.h` | 709 | 1 |
| `submission` | MetalResolveProbe field | The command buffer of the resolve. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6942, 7254, 7255, 8078 | 4 |
| `probe` | local in IssueCopy; iterator in LogResolveProbes | The probe being recorded or logged. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6931, 6932, 6933, 6934, 6935, 6936, 6937, 6939, 6941, 6942, 6947, 6949, 6951, 6956, 6963, 6965, 6969, 6970, 6974, 6977, 6980, 6982, 6983, 6985, 6995, 7253, 7254, 7255, 7256, 7258, 7259, 7261, 7262, 7264, 7271, 7272, 7273, 7278, 7281, 7283, 7288, 7289, 7293, 7296, 7298, 7299, 7300, 7301, 7309, 7311, 7314, 7317, 7322, 7323, 7324, 7325, 7332, 7335, 7336, 7347, 7351, 7353, 7354, 7363, 7380, 7384, 7385, 7386, 7387, 7402, 7403, 7413, 7418, 7438, 7443, 7449, 7462, 7466, 7467, 7471, 7476, 7477, 7479, 7480, 7482 | 99 |
| `dest_pitch` | local in IssueCopy | RB_COPY_DEST_PITCH of the probed resolve. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6928, 6935, 6936 | 3 |
| `dest_endian` | local in IssueCopy | RB_COPY_DEST_INFO endianness of the probed resolve. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6929, 6938 | 3 |
| `probe_cols` | local in LogResolveProbes | Grid columns (16). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7276, 7290, 7295, 7296 | 4 |
| `probe_rows` | local in LogResolveProbes | Grid rows, about square, 1 to 16. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7277, 7290, 7292, 7293 | 4 |
| `probe_line` | local in LogResolveProbes | One grid row's values for the log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7294, 7303, 7310, 7315 | 4 |
| `texel_address` | local in LogResolveProbes | Guest address of a sampled texel. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7297, 7300, 7301, 7302, 7307, 7321, 7324, 7325, 7326, 7330 | 10 |
| `depth24` | local in LogResolveProbes | The texel's 24-bit depth. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7309, 7311, 7312, 7320, 7332, 7337, 7338, 7372, 7377, 7389, 7391, 7396, 7423, 7424, 7427, 7429, 7450, 7451, 7453 | 19 |
| `ApplyXenosSamplePositions` | member function of MetalRenderTargetCache | Programs the Xenos MSAA sample positions (and the position-independent depth store) on a 2x/4x pass when xenos_sample_positions is on, else resets the defaults. | `src/xenia/gpu/metal/metal_render_target_cache.h` | 165 | 1 |
| `ApplyXenosSamplePositions` | member function of MetalRenderTargetCache | Programs the Xenos MSAA sample positions (and the position-independent depth store) on a 2x/4x pass when xenos_sample_positions is on, else resets the defaults. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3886, 6186 | 2 |
| `ApplyXenosSamplePositions` | member function of MetalRenderTargetCache | Programs the Xenos MSAA sample positions (and the position-independent depth store) on a 2x/4x pass when xenos_sample_positions is on, else resets the defaults. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8013 | 1 |
| `programmable_sample_positions_supported_` | member of MetalRenderTargetCache | MTLDevice areProgrammableSamplePositionsSupported, read at Initialize. | `src/xenia/gpu/metal/metal_render_target_cache.h` | 447 | 1 |
| `programmable_sample_positions_supported_` | member of MetalRenderTargetCache | MTLDevice areProgrammableSamplePositionsSupported, read at Initialize. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 734, 739, 741, 3911 | 4 |
| `use_xenos_positions` | local in ApplyXenosSamplePositions | Whether this pass gets the Xenos positions. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3910, 3916, 3920, 3922 | 4 |
| `metal_gpu_ordered_fences` | cvar (Metal) | Publish shader-done fences only once the GPU has completed the work before them (zero-copy shared memory). Default true; Settings > Accuracy. | `src/xenia/gpu/metal/metal_command_processor.cc` | 98, 2097 | 2 |
| `metal_gpu_ordered_fences` | cvar (Metal) | Publish shader-done fences only once the GPU has completed the work before them (zero-copy shared memory). Default true; Settings > Accuracy. | `src/xenia/ui/windowed_app_main_ios.mm` | 3738 | 1 |
| `AttachShaderDoneFences` | member function of MetalCommandProcessor | Moves pending_shader_done_fences_ into a completion handler of a command buffer about to be committed, which stores them in guest memory. | `src/xenia/gpu/metal/metal_command_processor.h` | 1001 | 1 |
| `AttachShaderDoneFences` | member function of MetalCommandProcessor | Moves pending_shader_done_fences_ into a completion handler of a command buffer about to be committed, which stores them in guest memory. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2117, 2122, 2181, 3178, 7094, 8708 | 6 |
| `pending_shader_done_fences_` | member of MetalCommandProcessor | Shader-done fences (physical address, value) waiting for the open command buffer. | `src/xenia/gpu/metal/metal_command_processor.h` | 998 | 1 |
| `pending_shader_done_fences_` | member of MetalCommandProcessor | Shader-done fences (physical address, value) waiting for the open command buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2090, 2106, 2116, 2124, 2130, 2131 | 6 |
| `fence_command_buffer` | local in TryWriteShaderDoneFence | Empty command buffer carrying fences when none is open. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2111, 2113, 2117, 2118 | 4 |
| `fences` | lambda capture in AttachShaderDoneFences | The fences a completion handler stores. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2129, 2133, 2134 | 3 |
| `read_depth24` | lambda in LogResolveProbes | Reads one 24-bit depth texel of a probed resolve, false if not written. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7320, 7338, 7391, 7470 | 4 |
| `written_rows` | local in LogResolveProbes | Rows of the probed resolve that were written. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7335, 7339, 7380, 7382, 7383, 7384, 7387, 7409, 7413, 7421, 7441, 7447 | 12 |
| `kEdgeNames` | local constant in LogResolveProbes | Edge names for the resolve edge lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7406, 7438 | 2 |
| `edge_line` | local in LogResolveProbes | One resolve edge line. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7410, 7432, 7438 | 3 |
| `depth_in` | local in LogResolveProbes | Row or column index counted from the edge (0-7). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7411, 7418, 7421, 7422 | 7 |
| `not_clear` | local in LogResolveProbes | Texels in that row or column that are not 1.0. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7414, 7428, 7433 | 3 |
| `min_depth24` | local in LogResolveProbes | Their minimum depth. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7415, 7429, 7433 | 4 |
| `block_y` | local in LogResolveProbes | Row of 8x8 blocks in the resolve map. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7441, 7446, 7447, 7463 | 6 |
| `block_x` | local in LogResolveProbes | Column of 8x8 blocks in the resolve map. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7443, 7448, 7449 | 5 |
| `map_line` | local in LogResolveProbes | One row of the resolve map. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7442, 7457, 7463 | 3 |
| `snapshot` | MetalResolveProbe field | Copy of the range the resolve wrote, made by a blit right after it (later resolves to the same address overwrite it before completion). | `src/xenia/gpu/metal/metal_command_processor.h` | 714 | 1 |
| `snapshot` | MetalResolveProbe field | Copy of the range the resolve wrote, made by a blit right after it (later resolves to the same address overwrite it before completion). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6947, 6949, 6951, 7258, 7259, 7271, 7272, 7403, 7476, 7477 | 10 |
| `edram_snapshot` | MetalResolveProbe field | Copy of the EDRAM tiles of the 1x depth surface the resolve read (the dumped host render target). | `src/xenia/gpu/metal/metal_command_processor.h` | 713, 715 | 2 |
| `edram_snapshot` | MetalResolveProbe field | Copy of the EDRAM tiles of the 1x depth surface the resolve read (the dumped host render target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6974, 6977, 6983, 6985, 7261, 7262, 7347, 7354, 7363, 7471, 7479, 7480, 8058, 8062, 8070, 8073, 8080, 8081 | 18 |
| `edram_base_tiles` | MetalResolveProbe field | RB_DEPTH_INFO depth base: the first tile in edram_snapshot. | `src/xenia/gpu/metal/metal_command_processor.h` | 713, 716 | 2 |
| `edram_base_tiles` | MetalResolveProbe field | RB_DEPTH_INFO depth base: the first tile in edram_snapshot. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6963, 6980, 6982, 8030, 8055, 8068, 8070 | 7 |
| `edram_pitch_tiles` | MetalResolveProbe field | Pitch of the depth surface in EDRAM tiles (RB_SURFACE_INFO). | `src/xenia/gpu/metal/metal_command_processor.h` | 717 | 1 |
| `edram_pitch_tiles` | MetalResolveProbe field | Pitch of the depth surface in EDRAM tiles (RB_SURFACE_INFO). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6965, 6969, 7351, 7353, 8035, 8049, 8055, 8056 | 9 |
| `snapshot_blit` | local in IssueCopy (frame detail) | Blit encoder that copies the resolved range and EDRAM tiles into the probe snapshots. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6944, 6946, 6950, 6981, 6985, 6992, 6993 | 7 |
| `edram` | local in IssueCopy (frame detail) | The render target cache EDRAM buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6954, 6956, 6982, 6985 | 4 |
| `surface_info` | local in IssueCopy (frame detail) | RB_SURFACE_INFO of the resolve (MSAA, pitch). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6955, 6957, 6966 | 3 |
| `tile_bytes` | local in IssueCopy (frame detail) | Bytes per EDRAM tile (80x16 32-bit samples). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6960, 6975, 6982, 6983, 6986, 6987 | 6 |
| `tiles` | local in IssueCopy (frame detail) | EDRAM tiles copied into edram_snapshot. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6968, 6973, 6975, 6980, 6984, 6987 | 6 |
| `first_tiles` | local in IssueCopy (frame detail) | Tiles copied before the EDRAM end wraps around to tile 0. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6979, 6983, 6984, 6986, 6987 | 5 |
| `probe_memory` | local in LogResolveProbes | Memory the probe is read from: its snapshot, else the shared memory buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7269, 7272, 7307, 7330 | 4 |
| `probe_memory_base` | local in LogResolveProbes | Guest address of the first byte of probe_memory (written_begin for a snapshot, else 0). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7270, 7273, 7307, 7330 | 4 |
| `edram_tile_bytes` | local in LogResolveProbes | Bytes per EDRAM tile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7343, 7354, 7364 | 3 |
| `read_edram` | lambda in LogResolveProbes | Reads the EDRAM dword of a depth surface pixel from edram_snapshot (tile halves swapped), false outside it. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7346, 7374, 7393 | 3 |
| `read_edram_depth24` | lambda in LogResolveProbes | The 24-bit depth of read_edram. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7372, 7468, 7472 | 3 |
| `tile` | local in read_edram | Tile of the pixel, counted from edram_base_tiles. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7350, 7354, 7364 | 3 |
| `tile_x` | local in read_edram | Sample column within the tile, halves swapped for depth. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7358, 7359, 7367 | 4 |
| `texels_line` | local in LogResolveProbes | The resolve texels log line (raw values at six border positions). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7381, 7394, 7404 | 3 |
| `positions` | local constant in LogResolveProbes | The six border positions logged. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7382, 7388 | 2 |
| `position` | loop variable in LogResolveProbes | One of positions. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7388, 7391, 7393, 7395 | 7 |
| `edram_texel` | local in LogResolveProbes | The EDRAM dword at the position. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7389, 7393, 7397 | 3 |
| `in_resolve` | local in LogResolveProbes | The resolve wrote the position. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7390, 7396 | 2 |
| `in_edram` | local in LogResolveProbes | edram_snapshot holds the position. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7392, 7397 | 2 |
| `log_coverage` | lambda in LogResolveProbes | Logs the edge and 8x8 map lines for the resolved texels or the EDRAM. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7408, 7467, 7470, 7472 | 4 |
| `source` | lambda parameter of log_coverage | "resolve" or "edram", in the log lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7408, 7438, 7462 | 3 |
| `read` | lambda parameter of log_coverage | read_depth24 or read_edram_depth24. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7408, 7424, 7451 | 3 |
| `pass` | MetalResolveProbe field | Not 0: an EDRAM probe of a 24-bit unorm depth target at the start of this frame detail pass, not a resolve (width and height in EDRAM samples). | `src/xenia/gpu/metal/metal_command_processor.h` | 718, 720 | 2 |
| `pass` | MetalResolveProbe field | Not 0: an EDRAM probe of a 24-bit unorm depth target at the start of this frame detail pass, not a resolve (width and height in EDRAM samples). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7278, 7283, 7335, 7336, 7380, 7466, 7467, 8029 | 8 |
| `depth_info` | local in BeginCommandBuffer (frame detail) | RB_DEPTH_INFO of the pass (24-bit unorm only, base tiles). | `src/xenia/gpu/metal/metal_command_processor.cc` | 8025, 8027, 8030 | 3 |
| `surface_info` | local in BeginCommandBuffer (frame detail) | RB_SURFACE_INFO of the pass (pitch, MSAA). | `src/xenia/gpu/metal/metal_command_processor.cc` | 8026, 8033, 8034, 8043 | 4 |
| `probe` | local in BeginCommandBuffer (frame detail) | The pass-start EDRAM probe being recorded. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8028, 8029, 8030, 8033, 8035, 8036, 8038, 8049, 8050, 8055, 8056, 8058, 8062, 8068, 8070, 8073, 8078, 8079, 8080, 8081 | 23 |
| `tile_bytes` | local in BeginCommandBuffer (frame detail) | Bytes per EDRAM tile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8045, 8059, 8070, 8071, 8074, 8075 | 6 |
| `tiles` | local in BeginCommandBuffer (frame detail) | EDRAM tiles dumped and copied. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8048, 8053, 8056, 8059, 8068, 8072, 8075 | 7 |
| `snapshot_blit` | local in BeginCommandBuffer (frame detail) | Blit encoder copying the dumped EDRAM tiles. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8061, 8064, 8069, 8073, 8077 | 5 |
| `edram` | local in BeginCommandBuffer (frame detail) | The render target cache EDRAM buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8065, 8070, 8073 | 3 |
| `first_tiles` | local in BeginCommandBuffer (frame detail) | Tiles copied before the EDRAM end wraps around. | `src/xenia/gpu/metal/metal_command_processor.cc` | 8067, 8071, 8072, 8074, 8075 | 5 |
| `KernelCallReadable` | function in xe::kernel::shim (declaration) | Whether a guest memory range is committed and readable. | `src/xenia/kernel/util/shim_utils.h` | 564 | 1 |
| `LogKernelCallStack` | function in xe::kernel::shim (declaration) | Guest return addresses up the stack from a frame. | `src/xenia/kernel/util/shim_utils.h` | 567 | 1 |
| `KernelCallReadable` | function in xe::kernel::shim | Checks the guest code around the crash PC is readable. | `src/xenia/emulator.cc` | 1817 | 1 |
| `LogKernelCallStack` | function in xe::kernel::shim | Guest stack of the crashed thread. | `src/xenia/emulator.cc` | 1813 | 1 |
| `crash_global` | local in Emulator::ExceptionCallback (crash dump) | The global critical region (try-locked; the page table and the stack walk need it). | `src/xenia/emulator.cc` | 1778, 1779, 1861 | 3 |
| `crash_locked` | local in Emulator::ExceptionCallback (crash dump) | Whether crash_global was taken. | `src/xenia/emulator.cc` | 1779, 1790, 1810 | 3 |
| `crash_fault` | local in Emulator::ExceptionCallback (crash dump) | Host fault address. | `src/xenia/emulator.cc` | 1781, 1784, 1785, 1786 | 4 |
| `crash_virtual` | local in Emulator::ExceptionCallback (crash dump) | Host address of guest virtual address 0. | `src/xenia/emulator.cc` | 1782, 1784, 1785, 1786, 1806 | 5 |
| `crash_guest` | local in Emulator::ExceptionCallback (crash dump) | Guest address of the fault. | `src/xenia/emulator.cc` | 1786, 1787, 1788, 1791 | 4 |
| `crash_heap` | local in Emulator::ExceptionCallback (crash dump) | Heap holding crash_guest. | `src/xenia/emulator.cc` | 1788, 1790, 1791, 1795, 1798 | 5 |
| `crash_info` | local in Emulator::ExceptionCallback (crash dump) | Region and allocation of crash_guest (state, protection). | `src/xenia/emulator.cc` | 1789, 1791, 1795, 1796, 1797 | 8 |
| `crash_pc` | local in Emulator::ExceptionCallback (crash dump) | Guest PC of the fault, for the code words around it. | `src/xenia/emulator.cc` | 1814, 1816, 1817, 1818, 1822 | 5 |
| `kernel_call_wait_objects` | local in LogKernelCallThreads | The handles of a NtWaitForMultipleObjectsEx with their object type and name. | `src/xenia/kernel/util/shim_utils.cc` | 292, 303, 307, 317, 321, 325 | 6 |
| `kernel_call_handles` | local in LogKernelCallThreads | Guest address of the handle array (r4). | `src/xenia/kernel/util/shim_utils.cc` | 293, 294, 298 | 3 |
| `kernel_call_wait_handle` | local in LogKernelCallThreads | One handle of the array. | `src/xenia/kernel/util/shim_utils.cc` | 296, 302, 304 | 3 |
| `kernel_call_wait_object` | local in LogKernelCallThreads | The object of that handle. | `src/xenia/kernel/util/shim_utils.cc` | 300, 305, 309, 310, 311, 313 | 6 |
| `frame_detail_bindings` | local in IssueDraw (frame detail) | Vertex bindings of the draw's vertex shader, for the small-draw vertex dump. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6415, 6418, 6419 | 3 |
| `vertex_words` | local in IssueDraw (frame detail) | Words of vertex data dumped (at most 48). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6423, 6429, 6432 | 3 |
| `frame_detail_vertices` | local in IssueDraw (frame detail) | The dumped vertex data as text. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6428, 6436, 6447 | 3 |
| `word_float` | local in IssueDraw (frame detail) | One vertex word as a float. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6434, 6435, 6440 | 3 |
| `OnlyFrameDetailLogChanged` | function | Whether edited settings differ from the stored ones only in metal_frame_detail_log (then saving closes Settings without the alert). | `src/xenia/ui/windowed_app_main_ios.mm` | 3909, 8862 | 2 |
| `frame_capture` | local in BuildIOSConfigSections | The Frame Capture section, first in Settings. | `src/xenia/ui/windowed_app_main_ios.mm` | 3553, 3554, 3555, 3558, 3575, 3576 | 6 |
| `frame_detail_changed` | local in OnlyFrameDetailLogChanged | Whether the Frame Detail Log differs. | `src/xenia/ui/windowed_app_main_ios.mm` | 3914, 3935, 3938 | 3 |
| `only_frame_detail_log` | local in saveTapped: | Result of OnlyFrameDetailLogChanged before the settings are applied. | `src/xenia/ui/windowed_app_main_ios.mm` | 8862, 8866 | 2 |
| `skip_unchanged_render_target_transfers` | cvar; iOS settings key | Skip transfers back to a render target whose copy of the EDRAM range is unchanged (default off). Settings > Performance > Skip Unchanged Render Target Copies. | `src/xenia/gpu/render_target_cache.cc` | 50, 2084 | 2 |
| `skip_unchanged_render_target_transfers` | cvar; iOS settings key | Skip transfers back to a render target whose copy of the EDRAM range is unchanged (default off). Settings > Performance > Skip Unchanged Render Target Copies. | `src/xenia/gpu/metal/metal_command_processor.cc` | 95, 3125 | 2 |
| `skip_unchanged_render_target_transfers` | cvar; iOS settings key | Skip transfers back to a render target whose copy of the EDRAM range is unchanged (default off). Settings > Performance > Skip Unchanged Render Target Copies. | `src/xenia/emulator.cc` | 122, 2279 | 2 |
| `skip_unchanged_render_target_transfers` | cvar; iOS settings key | Skip transfers back to a render target whose copy of the EDRAM range is unchanged (default off). Settings > Performance > Skip Unchanged Render Target Copies. | `src/xenia/ui/windowed_app_main_ios.mm` | 3670 | 1 |
| `TakeUnchangedTransferCounts` | member function of RenderTargetCache | Returns and resets unchanged_transfers_ and skipped_transfers_ for the Metal perf log. | `src/xenia/gpu/render_target_cache.h` | 243 | 1 |
| `TakeUnchangedTransferCounts` | member function of RenderTargetCache | Returns and resets unchanged_transfers_ and skipped_transfers_ for the Metal perf log. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3118 | 1 |
| `unchanged_out` | parameter of TakeUnchangedTransferCounts | Receives unchanged_transfers_. | `src/xenia/gpu/render_target_cache.h` | 243, 245 | 2 |
| `skipped_out` | parameter of TakeUnchangedTransferCounts | Receives skipped_transfers_. | `src/xenia/gpu/render_target_cache.h` | 244, 246 | 2 |
| `unchanged_transfers_` | member of RenderTargetCache | Ownership ranges given back to a render target whose copy was unchanged. | `src/xenia/gpu/render_target_cache.h` | 245, 247, 817 | 3 |
| `unchanged_transfers_` | member of RenderTargetCache | Ownership ranges given back to a render target whose copy was unchanged. | `src/xenia/gpu/render_target_cache.cc` | 2083 | 1 |
| `skipped_transfers_` | member of RenderTargetCache | How many of those skip_unchanged_render_target_transfers skipped. | `src/xenia/gpu/render_target_cache.h` | 246, 248, 818 | 3 |
| `skipped_transfers_` | member of RenderTargetCache | How many of those skip_unchanged_render_target_transfers skipped. | `src/xenia/gpu/render_target_cache.cc` | 2085 | 1 |
| `intact_render_target` | member of RenderTargetCache::OwnershipRange | A render target other than the owner whose copy of the range still holds the current contents. | `src/xenia/gpu/render_target_cache.h` | 723, 768 | 3 |
| `intact_render_target` | member of RenderTargetCache::OwnershipRange | A render target other than the owner whose copy of the range still holds the current contents. | `src/xenia/gpu/render_target_cache.cc` | 582, 583, 2038, 2050, 2051, 2081, 2151 | 7 |
| `dest_written` | parameter of ChangeOwnership | The destination will be written in the range (draw writing depth, stencil or color; resolve clear), so no other copy stays intact. | `src/xenia/gpu/render_target_cache.h` | 800 | 1 |
| `dest_written` | parameter of ChangeOwnership | The destination will be written in the range (draw writing depth, stencil or color; resolve clear), so no other copy stays intact. | `src/xenia/gpu/render_target_cache.cc` | 2003, 2035, 2050, 2151 | 4 |
| `depth_written` | local in Update | The draw writes depth or stencil. | `src/xenia/gpu/render_target_cache.cc` | 1035, 1037, 1044, 1065 | 4 |
| `writes_stencil` | lambda in Update | Whether one face's stencil state can write stencil. | `src/xenia/gpu/render_target_cache.cc` | 1038, 1045, 1050 | 3 |
| `fail` | parameter of writes_stencil | Stencil op on stencil test failure. | `src/xenia/gpu/render_target_cache.cc` | 1038, 1040 | 2 |
| `zpass` | parameter of writes_stencil | Stencil op when both tests pass. | `src/xenia/gpu/render_target_cache.cc` | 1039, 1041 | 2 |
| `zfail` | parameter of writes_stencil | Stencil op on depth test failure. | `src/xenia/gpu/render_target_cache.cc` | 1039, 1042 | 2 |
| `dest_intact` | local in ChangeOwnership | The destination's copy of the range is unchanged since it last owned it. | `src/xenia/gpu/render_target_cache.cc` | 2079, 2082 | 2 |
| `previous_owner_matches` | local in ChangeOwnership | The previous owner's copy will match the destination's after this change (transferred, or skipped as unchanged). | `src/xenia/gpu/render_target_cache.cc` | 2073, 2086, 2090, 2117, 2138, 2151 | 6 |
| `previous_owner` | local in ChangeOwnership | Owner of the range before the claim. | `src/xenia/gpu/render_target_cache.cc` | 2146, 2152 | 2 |
| `unchanged_transfers` | local in the Metal perf log | Value from TakeUnchangedTransferCounts. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3117, 3118, 3124 | 3 |
| `skipped_transfers` | local in the Metal perf log | Value from TakeUnchangedTransferCounts. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3117, 3119, 3124 | 3 |
| `UserApcPendingForHostWait` | function (xobject.cc) | Whether an alertable host wait should end at once for a pending guest user APC (POSIX host waits are not woken by one). | `src/xenia/kernel/xobject.cc` | 42, 442, 450, 680, 688, 722, 730 | 7 |
| `kAlertableWaitPollIntervalIOS` | constant (iOS only) | Slice of an alertable host wait on iOS, so an APC queued by another thread is seen within 10 ms. | `src/xenia/kernel/xobject.cc` | 59, 138 | 2 |
| `slice` | local in WaitWithTitleStopPollIOS (iOS only) | The next wait slice, capped at kAlertableWaitPollIntervalIOS for alertable waits. | `src/xenia/kernel/xobject.cc` | 136, 138, 140 | 4 |
| `typed_utf8` | local in the keyboard prompt callback | UTF-8 of the typed text (nil-safe) before the copy is released. | `src/xenia/ui/windowed_app_main_ios.mm` | 13248, 13249 | 3 |
| `ordinal` | member of MetalCommandProcessor::MetalZPDQuery | The query's place among the frame's query ends. | `src/xenia/gpu/metal/metal_command_processor.h` | 497 | 1 |
| `ordinal` | member of MetalCommandProcessor::MetalZPDQuery | The query's place among the frame's query ends. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7575, 7576, 7577, 7583, 7584, 7594, 7595, 7679, 7680, 7704 | 11 |
| `address_guess` | member of MetalCommandProcessor::MetalZPDQuery | What the last count at its address predicted. | `src/xenia/gpu/metal/metal_command_processor.h` | 498 | 1 |
| `address_guess` | member of MetalCommandProcessor::MetalZPDQuery | What the last count at its address predicted. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7593, 7599, 7672, 7705 | 4 |
| `ordinal_guess` | member of MetalCommandProcessor::MetalZPDQuery | What the last count at its place in the frame predicted (UINT32_MAX: none). | `src/xenia/gpu/metal/metal_command_processor.h` | 499 | 1 |
| `ordinal_guess` | member of MetalCommandProcessor::MetalZPDQuery | What the last count at its place in the frame predicted (UINT32_MAX: none). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7595, 7597, 7598, 7665, 7671, 7705 | 6 |
| `zpd_frame_query_ends_` | member of MetalCommandProcessor | Query ends since the last swap. | `src/xenia/gpu/metal/metal_command_processor.h` | 524 | 1 |
| `zpd_frame_query_ends_` | member of MetalCommandProcessor | Query ends since the last swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2947, 7575 | 2 |
| `zpd_results_by_ordinal_` | member of MetalCommandProcessor | Last count per place in the frame (UINT32_MAX: none). | `src/xenia/gpu/metal/metal_command_processor.h` | 525 | 1 |
| `zpd_results_by_ordinal_` | member of MetalCommandProcessor | Last count per place in the frame (UINT32_MAX: none). | `src/xenia/gpu/metal/metal_command_processor.cc` | 7576, 7577, 7583, 7584, 7594, 7595, 7679, 7680 | 8 |
| `zpd_ordinal_score_` | member of MetalCommandProcessor | Which guess has been closer lately, -8...8 (above 0: by place in the frame). | `src/xenia/gpu/metal/metal_command_processor.h` | 526 | 1 |
| `zpd_ordinal_score_` | member of MetalCommandProcessor | Which guess has been closer lately, -8...8 (above 0: by place in the frame). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3139, 3140, 7597, 7674, 7676, 7706 | 8 |
| `miss` | lambda in ProcessCompletedZPDQueries | Distance of a guess from the count. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7667, 7671, 7672 | 3 |
| `ordinal_miss` | local in ProcessCompletedZPDQueries | How far the guess by place was. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7670, 7673, 7675 | 3 |
| `address_miss` | local in ProcessCompletedZPDQueries | How far the guess by address was. | `src/xenia/gpu/metal/metal_command_processor.cc` | 7672, 7673, 7675 | 3 |
| `signin_state_logged` | function-local static in XamUserGetSigninState_entry | The value last logged per user, to log only changes. | `src/xenia/kernel/xam/xam_user.cc` | 104, 106, 107 | 3 |
| `metal_gpu_trace_frames` | cvar (Metal); iOS settings key | Changed to N during a game: records the GPU commands of the next N frames into a trace under trace_gpu_prefix. Settings > Frame Capture > GPU Trace. | `src/xenia/gpu/metal/metal_command_processor.cc` | 125, 2962, 2963 | 3 |
| `metal_gpu_trace_frames` | cvar (Metal); iOS settings key | Changed to N during a game: records the GPU commands of the next N frames into a trace under trace_gpu_prefix. Settings > Frame Capture > GPU Trace. | `src/xenia/ui/windowed_app_main_ios.mm` | 3567, 3932 | 2 |
| `gpu_trace_setting_` | member of MetalCommandProcessor | metal_gpu_trace_frames when last seen (a change starts a recording). | `src/xenia/gpu/metal/metal_command_processor.h` | 691 | 1 |
| `gpu_trace_setting_` | member of MetalCommandProcessor | metal_gpu_trace_frames when last seen (a change starts a recording). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2962, 2963, 2964, 2979 | 4 |
| `gpu_trace_frames_left_` | member of MetalCommandProcessor | Frames of the current recording still to write. | `src/xenia/gpu/metal/metal_command_processor.h` | 692 | 1 |
| `gpu_trace_frames_left_` | member of MetalCommandProcessor | Frames of the current recording still to write. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2951, 2953, 2959, 2964, 2979, 2981 | 6 |
| `trace_path` | local in IssueSwap | Folder of a new GPU trace (storage root / trace_gpu_prefix / date and time). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2966, 2967, 2969, 2977, 2978, 2981 | 7 |
| `trace_time` | local in IssueSwap | Time the recording was requested. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2971, 2973 | 2 |
| `trace_local_time` | local in IssueSwap | trace_time in the device time zone. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2972, 2973, 2976 | 3 |
| `trace_folder` | local in IssueSwap | Folder name from the date and time. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2974, 2975, 2977 | 4 |
| `LogKernelCallStackCode` | function in xe::kernel::shim | Logs the guest code around the caller and each return address of a stuck thread (first storm report). | `src/xenia/kernel/util/shim_utils.h` | 570 | 1 |
| `LogKernelCallStackCode` | function in xe::kernel::shim | Logs the guest code around the caller and each return address of a stuck thread (first storm report). | `src/xenia/kernel/util/shim_utils.cc` | 132, 280, 284, 437, 856 | 5 |
| `kernel_call_returns` | local in LogKernelCallStackCode | The caller and the return addresses up the stack. | `src/xenia/kernel/util/shim_utils.cc` | 141, 155, 162 | 3 |
| `kernel_call_return` | local in LogKernelCallStackCode | One of them. | `src/xenia/kernel/util/shim_utils.cc` | 162, 163, 164, 165, 180 | 6 |
| `kernel_call_code_logged` | function-local static in LogKernelCallStackCode | Return addresses already dumped this run. | `src/xenia/kernel/util/shim_utils.cc` | 161, 165 | 2 |
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
| `DescribeVirtualMemoryHistory` | function in xe::kernel::xboxkrnl | The recorded calls whose range holds an address, for the crash dump. | `src/xenia/emulator.cc` | 1859 | 1 |
| `crash_reg` | local in Emulator::ExceptionCallback (crash dump) | Register index. | `src/xenia/emulator.cc` | 1829, 1830, 1845 | 5 |
| `crash_pointer` | local in Emulator::ExceptionCallback (crash dump) | The register as a guest address. | `src/xenia/emulator.cc` | 1830, 1831, 1834, 1837, 1845, 1848, 1859 | 8 |
| `crash_pointer_heap` | local in Emulator::ExceptionCallback (crash dump) | Heap holding it. | `src/xenia/emulator.cc` | 1834, 1836, 1837 | 3 |
| `crash_pointer_info` | local in Emulator::ExceptionCallback (crash dump) | Its allocation. | `src/xenia/emulator.cc` | 1835, 1838, 1839, 1845, 1846, 1847 | 7 |
| `crash_bytes` | local in Emulator::ExceptionCallback (crash dump) | Start of the 32 bytes dumped around it. | `src/xenia/emulator.cc` | 1848, 1849, 1854 | 3 |
| `signed_in_mask` | local in KernelState::RegisterNotifyListener | Users signed in, for the startup XN_SYS_SIGNINCHANGED. | `src/xenia/kernel/kernel_state.cc` | 1220, 1224, 1228 | 3 |
| `msaa_samples_as_pixels` | cvar; iOS settings key | Draw MSAA rectangle lists (clears, screen-space passes) into the 1x render target whose pixels are the samples, scaled by the sample grid, unless the host MSAA render target holds the EDRAM (default off). Settings > Performance > MSAA Samples as Pixels. | `src/xenia/gpu/render_target_cache.cc` | 60, 781, 1748 | 3 |
| `msaa_samples_as_pixels` | cvar; iOS settings key | Draw MSAA rectangle lists (clears, screen-space passes) into the 1x render target whose pixels are the samples, scaled by the sample grid, unless the host MSAA render target holds the EDRAM (default off). Settings > Performance > MSAA Samples as Pixels. | `src/xenia/gpu/render_target_cache.h` | 36 | 1 |
| `msaa_samples_as_pixels` | cvar; iOS settings key | Draw MSAA rectangle lists (clears, screen-space passes) into the 1x render target whose pixels are the samples, scaled by the sample grid, unless the host MSAA render target holds the EDRAM (default off). Settings > Performance > MSAA Samples as Pixels. | `src/xenia/emulator.cc` | 123, 2280 | 2 |
| `msaa_samples_as_pixels` | cvar; iOS settings key | Draw MSAA rectangle lists (clears, screen-space passes) into the 1x render target whose pixels are the samples, scaled by the sample grid, unless the host MSAA render target holds the EDRAM (default off). Settings > Performance > MSAA Samples as Pixels. | `src/xenia/ui/windowed_app_main_ios.mm` | 3679 | 1 |
| `last_update_samples_as_pixels_` | member of RenderTargetCache | Guest MSAA of the surface the last Update bound as 1x render targets of its samples, or k1X. | `src/xenia/gpu/render_target_cache.h` | 233, 236, 676 | 3 |
| `last_update_samples_as_pixels_` | member of RenderTargetCache | Guest MSAA of the surface the last Update bound as 1x render targets of its samples, or k1X. | `src/xenia/gpu/render_target_cache.cc` | 624, 793 | 2 |
| `last_update_samples_as_pixels_x_log2` | member function of RenderTargetCache | Horizontal draw scale (log2) on top of the resolution scale for msaa_samples_as_pixels: 1 at 4x MSAA. | `src/xenia/gpu/render_target_cache.h` | 232 | 1 |
| `last_update_samples_as_pixels_x_log2` | member function of RenderTargetCache | Horizontal draw scale (log2) on top of the resolution scale for msaa_samples_as_pixels: 1 at 4x MSAA. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3566, 5256, 8915, 12060, 12168 | 5 |
| `last_update_samples_as_pixels_y_log2` | member function of RenderTargetCache | Vertical draw scale (log2) on top of the resolution scale for msaa_samples_as_pixels: 1 at 2x and 4x MSAA. | `src/xenia/gpu/render_target_cache.h` | 235 | 1 |
| `last_update_samples_as_pixels_y_log2` | member function of RenderTargetCache | Vertical draw scale (log2) on top of the resolution scale for msaa_samples_as_pixels: 1 at 2x and 4x MSAA. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3568, 5253, 5258, 8044, 8917, 12063, 12170 | 7 |
| `SupportsMsaaSamplesAsPixels` | virtual member function of RenderTargetCache | Whether the implementation scales draws by the sample grid (Metal: the SPIRV-Cross path). | `src/xenia/gpu/render_target_cache.h` | 269 | 1 |
| `SupportsMsaaSamplesAsPixels` | virtual member function of RenderTargetCache | Whether the implementation scales draws by the sample grid (Metal: the SPIRV-Cross path). | `src/xenia/gpu/render_target_cache.cc` | 781, 1748 | 2 |
| `SupportsMsaaSamplesAsPixels` | virtual member function of RenderTargetCache | Whether the implementation scales draws by the sample grid (Metal: the SPIRV-Cross path). | `src/xenia/gpu/metal/metal_render_target_cache.h` | 140 | 1 |
| `SupportsMsaaSamplesAsPixels` | virtual member function of RenderTargetCache | Whether the implementation scales draws by the sample grid (Metal: the SPIRV-Cross path). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3186 | 1 |
| `storage_msaa_samples` | local in Update and PrepareHostRenderTargetsResolveClear | MSAA of the host render target keys: k1X when msaa_samples_as_pixels applies to the draw or clear, else the guest MSAA. | `src/xenia/gpu/render_target_cache.cc` | 778, 792, 819, 984, 1108, 1746, 1770, 1779, 1795, 1811 | 10 |
| `IsEdramTileOwnedBy` | private member function of RenderTargetCache | Whether the render target with a key last used an EDRAM tile (ownership_ranges_ lookup); msaa_samples_as_pixels keeps host MSAA while it holds the data. | `src/xenia/gpu/render_target_cache.h` | 815 | 1 |
| `IsEdramTileOwnedBy` | private member function of RenderTargetCache | Whether the render target with a key last used an EDRAM tile (ownership_ranges_ lookup); msaa_samples_as_pixels keeps host MSAA while it holds the data. | `src/xenia/gpu/render_target_cache.cc` | 607, 791, 1768 | 3 |
| `msaa_key` | local in Update and PrepareHostRenderTargetsResolveClear | Host MSAA key of the first render target of the draw or clear, checked against the tile owner. | `src/xenia/gpu/render_target_cache.cc` | 785, 786, 787, 788, 789, 790, 791, 1750, 1751, 1752, 1755, 1756, 1757, 1761, 1762, 1769 | 17 |
| `first_rt_index` | local in Update | Bit of the first bound render target (depth, else the lowest color). | `src/xenia/gpu/render_target_cache.cc` | 779, 784, 786, 789, 790 | 5 |
| `clear_first_tile` | local in PrepareHostRenderTargetsResolveClear | First EDRAM tile the resolve clear covers. | `src/xenia/gpu/render_target_cache.cc` | 1753, 1758, 1765, 1768 | 4 |
| `samples_as_pixels` | local in IssueDrawMsl | The draw goes to render targets keeping MSAA samples as pixels; its scale and divisors include the sample grid. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5251, 5254, 5263, 5266 | 4 |
| `kSysFlag_SamplesAsPixelsX_Shift` | enumerator of SpirvShaderTranslator | Bit of kSysFlag_SamplesAsPixelsX. | `src/xenia/gpu/spirv_shader_translator.h` | 159, 206 | 2 |
| `kSysFlag_SamplesAsPixelsY_Shift` | enumerator of SpirvShaderTranslator | Bit of kSysFlag_SamplesAsPixelsY. | `src/xenia/gpu/spirv_shader_translator.h` | 160, 207 | 2 |
| `kSysFlag_SamplesAsPixelsX` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel horizontally (msaa_samples_as_pixels at 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/spirv_shader_translator.h` | 206 | 1 |
| `kSysFlag_SamplesAsPixelsX` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel horizontally (msaa_samples_as_pixels at 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/spirv_shader_translator.cc` | 3429 | 1 |
| `kSysFlag_SamplesAsPixelsX` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel horizontally (msaa_samples_as_pixels at 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/metal/metal_command_processor.cc` | 12061 | 1 |
| `kSysFlag_SamplesAsPixelsY` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel vertically (msaa_samples_as_pixels at 2x and 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/spirv_shader_translator.h` | 207 | 1 |
| `kSysFlag_SamplesAsPixelsY` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel vertically (msaa_samples_as_pixels at 2x and 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/spirv_shader_translator.cc` | 3484 | 1 |
| `kSysFlag_SamplesAsPixelsY` | enumerator of SpirvShaderTranslator | System flag: a host pixel is half a guest pixel vertically (msaa_samples_as_pixels at 2x and 4x MSAA), for the pixel position parameter. | `src/xenia/gpu/metal/metal_command_processor.cc` | 12064 | 1 |
| `DescribeLastSignal` | member function of XEvent | Who last set or pulsed the event (guest thread, return address, how long ago, how many times), for the stuck thread report. | `src/xenia/kernel/xevent.h` | 47 | 1 |
| `DescribeLastSignal` | member function of XEvent | Who last set or pulsed the event (guest thread, return address, how long ago, how many times), for the stuck thread report. | `src/xenia/kernel/xevent.cc` | 102 | 1 |
| `DescribeLastSignal` | member function of XEvent | Who last set or pulsed the event (guest thread, return address, how long ago, how many times), for the stuck thread report. | `src/xenia/kernel/util/shim_utils.cc` | 263, 314 | 2 |
| `NoteSignal` | private member function of XEvent | Records the caller of Set and Pulse for DescribeLastSignal. | `src/xenia/kernel/xevent.h` | 61 | 1 |
| `NoteSignal` | private member function of XEvent | Records the caller of Set and Pulse for DescribeLastSignal. | `src/xenia/kernel/xevent.cc` | 64, 119, 125 | 3 |
| `last_signal_thread_` | member of XEvent | Handle of the guest thread that last set or pulsed the event (0 for a host thread). | `src/xenia/kernel/xevent.h` | 65 | 1 |
| `last_signal_thread_` | member of XEvent | Handle of the guest thread that last set or pulsed the event (0 for a host thread). | `src/xenia/kernel/xevent.cc` | 68, 110 | 2 |
| `last_signal_lr_` | member of XEvent | Guest link register of that call. | `src/xenia/kernel/xevent.h` | 66 | 1 |
| `last_signal_lr_` | member of XEvent | Guest link register of that call. | `src/xenia/kernel/xevent.cc` | 70, 111 | 2 |
| `last_signal_ms_` | member of XEvent | Host uptime in milliseconds of that call. | `src/xenia/kernel/xevent.h` | 69 | 1 |
| `last_signal_ms_` | member of XEvent | Host uptime in milliseconds of that call. | `src/xenia/kernel/xevent.cc` | 97, 115 | 2 |
| `signal_count_` | member of XEvent | How many times the event was set or pulsed. | `src/xenia/kernel/xevent.h` | 70 | 1 |
| `signal_count_` | member of XEvent | How many times the event was set or pulsed. | `src/xenia/kernel/xevent.cc` | 99, 103 | 2 |
| `signal_thread` | local in NoteSignal | The calling guest thread, if any. | `src/xenia/kernel/xevent.cc` | 65, 68, 77, 78, 81, 82, 87 | 9 |
| `signal_count` | local in DescribeLastSignal | signal_count_ as read once. | `src/xenia/kernel/xevent.cc` | 103, 104, 110 | 3 |
| `last_signal_callers_` | member of XEvent | The two return addresses above last_signal_lr_ on the guest stack (callers of the game's SetEvent wrapper). | `src/xenia/kernel/xevent.h` | 50, 68 | 2 |
| `last_signal_callers_` | member of XEvent | The two return addresses above last_signal_lr_ on the guest stack (callers of the game's SetEvent wrapper). | `src/xenia/kernel/xevent.cc` | 95, 96, 112, 113 | 4 |
| `callers` | local in NoteSignal | Return addresses read from the guest stack. | `src/xenia/kernel/xevent.cc` | 76, 80, 95, 96 | 4 |
| `caller_frame` | local in NoteSignal | Back chain of the current guest stack frame. | `src/xenia/kernel/xevent.cc` | 85, 87, 91, 92 | 5 |
| `metal_hud` | cvar; iOS settings key | Show the Metal performance HUD (developerHUDProperties mode; MTL_HUD_ENABLED unset when off). Settings > Display > Metal Performance HUD. | `src/xenia/ui/metal/metal_presenter.mm` | 50, 911, 914 | 3 |
| `metal_hud` | cvar; iOS settings key | Show the Metal performance HUD (developerHUDProperties mode; MTL_HUD_ENABLED unset when off). Settings > Display > Metal Performance HUD. | `src/xenia/app/xenia_main_ios.mm` | 82, 186 | 2 |
| `metal_hud` | cvar; iOS settings key | Show the Metal performance HUD (developerHUDProperties mode; MTL_HUD_ENABLED unset when off). Settings > Display > Metal Performance HUD. | `src/xenia/ui/windowed_app_main_ios.mm` | 3582 | 1 |
| `metal_hud_elements` | cvar; iOS settings key | MTL_HUD_ELEMENTS for the HUD, set after the config is read (empty: Apple default). Settings > Display > HUD Contents. | `src/xenia/ui/metal/metal_presenter.mm` | 54 | 1 |
| `metal_hud_elements` | cvar; iOS settings key | MTL_HUD_ELEMENTS for the HUD, set after the config is read (empty: Apple default). Settings > Display > HUD Contents. | `src/xenia/app/xenia_main_ios.mm` | 84, 187, 188 | 3 |
| `metal_hud_elements` | cvar; iOS settings key | MTL_HUD_ELEMENTS for the HUD, set after the config is read (empty: Apple default). Settings > Display > HUD Contents. | `src/xenia/ui/windowed_app_main_ios.mm` | 3587 | 1 |
| `metal_hud_alignment` | cvar; iOS settings key | MTL_HUD_ALIGNMENT for the HUD. Settings > Display > HUD Position. | `src/xenia/ui/metal/metal_presenter.mm` | 60 | 1 |
| `metal_hud_alignment` | cvar; iOS settings key | MTL_HUD_ALIGNMENT for the HUD. Settings > Display > HUD Position. | `src/xenia/app/xenia_main_ios.mm` | 83, 190, 191 | 3 |
| `metal_hud_alignment` | cvar; iOS settings key | MTL_HUD_ALIGNMENT for the HUD. Settings > Display > HUD Position. | `src/xenia/ui/windowed_app_main_ios.mm` | 3598 | 1 |
| `ios_device_status` | cvar; iOS settings key | Show the iOS version and thermal state while a game runs. Settings > Display > Device Status. | `src/xenia/ui/windowed_app_main_ios.mm` | 71, 3606, 10456 | 3 |
| `device_status_label_` | ivar of XeniaViewController | The device status label (top left, below menus). | `src/xenia/ui/windowed_app_main_ios.mm` | 9559, 9619, 9620, 9621, 9623, 9624, 9625, 9626, 9627, 9629, 9631, 9633, 10481, 10484, 10487, 10488 | 16 |
| `device_status_thermal_` | ivar of XeniaViewController | Thermal state the label shows (-1 before the first). | `src/xenia/ui/windowed_app_main_ios.mm` | 9560, 9628, 10461, 10462 | 4 |
| `show_device_status` | local in pollControllerNavigation: | Whether the device status label is shown now. | `src/xenia/ui/windowed_app_main_ios.mm` | 10456, 10459, 10487, 10488 | 4 |
| `thermal` | local in pollControllerNavigation: | NSProcessInfo thermalState as read this tick. | `src/xenia/ui/windowed_app_main_ios.mm` | 10460, 10461, 10462, 10465 | 4 |
| `thermal_name` | local in pollControllerNavigation: | Name of that thermal state for the label. | `src/xenia/ui/windowed_app_main_ios.mm` | 10463, 10467, 10471, 10475, 10483 | 5 |
| `thermal_color` | local in pollControllerNavigation: | Label color for that thermal state. | `src/xenia/ui/windowed_app_main_ios.mm` | 10464, 10468, 10472, 10476, 10484 | 5 |
| `shown_sections_` | ivar of XeniaConfigViewController | Sections the settings search shows, as indices into sections_. | `src/xenia/ui/windowed_app_main_ios.mm` | 8097, 8315, 8336, 8503, 8510, 8514, 8518, 8525, 8528, 8532, 8535 | 11 |
| `shown_items_` | ivar of XeniaConfigViewController | Per shown section, the items the settings search shows, as indices into its items. | `src/xenia/ui/windowed_app_main_ios.mm` | 8098, 8316, 8337, 8506, 8521 | 5 |
| `applySettingsSearch` | method of XeniaConfigViewController | Fills shown_sections_ and shown_items_ with the settings matching every word of the search text. | `src/xenia/ui/windowed_app_main_ios.mm` | 8268, 8308, 8343 | 3 |
| `search_text` | parameter of applySettingsSearch: | The search bar text (nil for none). | `src/xenia/ui/windowed_app_main_ios.mm` | 8308, 8310 | 2 |
| `filtering` | local in applySettingsSearch: | The search text has a word, so sections without matches are hidden. | `src/xenia/ui/windowed_app_main_ios.mm` | 8311, 8313, 8335 | 4 |
| `shown` | local in tableView:titleForFooterInSection: | The section shown at that index. | `src/xenia/ui/windowed_app_main_ios.mm` | 8535, 8536 | 3 |
| `last_signal_caller` | member function of XEvent | Return address 0 or 1 above the last set or pulse, for the code dump in the thread report. | `src/xenia/kernel/xevent.h` | 49 | 1 |
| `last_signal_caller` | member function of XEvent | Return address 0 or 1 above the last set or pulse, for the code dump in the thread report. | `src/xenia/kernel/util/shim_utils.cc` | 265, 267 | 2 |
| `kernel_call_signal_callers` | local in LogKernelCallThreads | The two callers that last set the event a thread waits on, whose code is dumped. | `src/xenia/kernel/util/shim_utils.cc` | 247, 264, 266, 282 | 4 |
| `kernel_call_event` | local in LogKernelCallThreads | The waited-on object as an XEvent. | `src/xenia/kernel/util/shim_utils.cc` | 260, 263, 265, 267 | 4 |
| `kernel_call_signal_caller` | loop variable in LogKernelCallThreads | One of kernel_call_signal_callers. | `src/xenia/kernel/util/shim_utils.cc` | 282, 283, 284 | 3 |
| `debug_watch_guest_addresses` | cvar | Guest addresses whose 32-bit values StartGuestValueWatch samples every 0.1 ms and logs (min, max, last, % zero) every 2 s. | `src/xenia/kernel/kernel_flags.cc` | 31 | 1 |
| `debug_watch_guest_addresses` | cvar | Guest addresses whose 32-bit values StartGuestValueWatch samples every 0.1 ms and logs (min, max, last, % zero) every 2 s. | `src/xenia/kernel/kernel_flags.h` | 18 | 1 |
| `debug_watch_guest_addresses` | cvar | Guest addresses whose 32-bit values StartGuestValueWatch samples every 0.1 ms and logs (min, max, last, % zero) every 2 s. | `src/xenia/kernel/util/shim_utils.cc` | 467, 498, 500, 502, 504 | 5 |
| `StartGuestValueWatch` | function in xe::kernel::shim | Starts the debug_watch_guest_addresses sampling thread once per run (called at launch). | `src/xenia/kernel/util/shim_utils.h` | 561 | 1 |
| `StartGuestValueWatch` | function in xe::kernel::shim | Starts the debug_watch_guest_addresses sampling thread once per run (called at launch). | `src/xenia/kernel/util/shim_utils.cc` | 465 | 1 |
| `StartGuestValueWatch` | function in xe::kernel::shim | Starts the debug_watch_guest_addresses sampling thread once per run (called at launch). | `src/xenia/emulator.cc` | 2281 | 1 |
| `watch_started` | local in StartGuestValueWatch | Function-local static: the thread was started. | `src/xenia/kernel/util/shim_utils.cc` | 466, 469 | 2 |
| `watch_addresses` | local in StartGuestValueWatch | Parsed addresses. | `src/xenia/kernel/util/shim_utils.cc` | 496, 510, 514, 517, 542, 546, 564, 569, 595 | 9 |
| `watch_start` | local in StartGuestValueWatch | Parse position. | `src/xenia/kernel/util/shim_utils.cc` | 474, 475, 476, 481, 494, 497, 498, 500, 505, 512 | 12 |
| `watch_end` | local in StartGuestValueWatch | End of one address in the list. | `src/xenia/kernel/util/shim_utils.cc` | 476, 477, 478, 481, 494, 499, 501, 502, 505, 512 | 10 |
| `watch_text` | local in StartGuestValueWatch | One address as text. | `src/xenia/kernel/util/shim_utils.cc` | 480, 484, 485, 504, 508, 509 | 6 |
| `watch_parse_end` | local in StartGuestValueWatch | Strtoul end pointer. | `src/xenia/kernel/util/shim_utils.cc` | 482, 484, 485, 487, 506, 508, 509 | 8 |
| `watch_address` | local in StartGuestValueWatch | One parsed address. | `src/xenia/kernel/util/shim_utils.cc` | 507, 510 | 2 |
| `watch_count` | local in StartGuestValueWatch | Number of addresses. | `src/xenia/kernel/util/shim_utils.cc` | 546, 547, 548, 563, 575, 592 | 8 |
| `watch_min` | local in StartGuestValueWatch | Minimum per address over the window. | `src/xenia/kernel/util/shim_utils.cc` | 547, 550, 578, 595 | 6 |
| `watch_max` | local in StartGuestValueWatch | Maximum per address. | `src/xenia/kernel/util/shim_utils.cc` | 547, 551, 579, 595 | 6 |
| `watch_last` | local in StartGuestValueWatch | Last value per address. | `src/xenia/kernel/util/shim_utils.cc` | 548, 580, 595 | 3 |
| `watch_zero` | local in StartGuestValueWatch | Samples that were 0, per address. | `src/xenia/kernel/util/shim_utils.cc` | 548, 552, 581, 596 | 5 |
| `watch_samples` | local in StartGuestValueWatch | Samples taken in the window. | `src/xenia/kernel/util/shim_utils.cc` | 553, 583, 588, 596, 598 | 5 |
| `watch_window_start` | local in StartGuestValueWatch | Window start time. | `src/xenia/kernel/util/shim_utils.cc` | 554, 555 | 2 |
| `watch_readable` | local in StartGuestValueWatch | All addresses mapped (checked every 5 ms). | `src/xenia/kernel/util/shim_utils.cc` | 558, 562, 565, 574 | 4 |
| `watch_host` | local in StartGuestValueWatch | Host pointers of the addresses. | `src/xenia/kernel/util/shim_utils.cc` | 559, 568, 577 | 3 |
| `watch_global` | local in StartGuestValueWatch | The global critical region mutex. | `src/xenia/kernel/util/shim_utils.cc` | 560, 561, 571 | 3 |
| `watch_step` | local in StartGuestValueWatch | Sample in a 5 ms batch. | `src/xenia/kernel/util/shim_utils.cc` | 573 | 3 |
| `watch_value` | local in StartGuestValueWatch | One sampled value. | `src/xenia/kernel/util/shim_utils.cc` | 576, 578, 579, 580, 581 | 5 |
| `watch_line` | local in StartGuestValueWatch | The log line. | `src/xenia/kernel/util/shim_utils.cc` | 591, 593, 598 | 3 |
| `debug_dump_guest_code` | cvar | Guest address ranges whose code StartGuestValueWatch logs once, 20 s after launch ("Guest code dump at"). | `src/xenia/kernel/kernel_flags.cc` | 38 | 1 |
| `debug_dump_guest_code` | cvar | Guest address ranges whose code StartGuestValueWatch logs once, 20 s after launch ("Guest code dump at"). | `src/xenia/kernel/kernel_flags.h` | 19 | 1 |
| `debug_dump_guest_code` | cvar | Guest address ranges whose code StartGuestValueWatch logs once, 20 s after launch ("Guest code dump at"). | `src/xenia/kernel/util/shim_utils.cc` | 468, 475, 476, 478, 480 | 5 |
| `dump_ranges` | local in StartGuestValueWatch | Parsed debug_dump_guest_code ranges. | `src/xenia/kernel/util/shim_utils.cc` | 473, 490, 514, 517, 519, 521 | 6 |
| `dump_first` | local in StartGuestValueWatch | Range start. | `src/xenia/kernel/util/shim_utils.cc` | 483, 489, 490, 521, 522 | 6 |
| `dump_last` | local in StartGuestValueWatch | Range end. | `src/xenia/kernel/util/shim_utils.cc` | 486, 489, 491, 521, 522 | 6 |
| `dump_line` | local in StartGuestValueWatch | Address of one dumped line (8 words). | `src/xenia/kernel/util/shim_utils.cc` | 522, 523, 528, 533, 538 | 6 |
| `dump_global` | local in StartGuestValueWatch | The global critical region mutex. | `src/xenia/kernel/util/shim_utils.cc` | 524, 527 | 3 |
| `dump_words` | local in StartGuestValueWatch | One dumped line's words. | `src/xenia/kernel/util/shim_utils.cc` | 525, 530, 537, 538 | 4 |
| `dump_lock` | local in StartGuestValueWatch | Holds dump_global. | `src/xenia/kernel/util/shim_utils.cc` | 527 | 1 |
| `IOSConfigItemValuesDiffer` | function | Whether a settings row holds a different value from its global counterpart (per-game settings). | `src/xenia/ui/windowed_app_main_ios.mm` | 4031, 4107, 8131 | 3 |
| `ApplyGameConfigToSections` | function | Puts a game's config overrides onto the settings rows. | `src/xenia/ui/windowed_app_main_ios.mm` | 4046, 8264 | 2 |
| `BuildGameConfigTable` | function | The game's config with the rows that differ from global written and the others' overrides removed. | `src/xenia/ui/windowed_app_main_ios.mm` | 4096, 8829 | 2 |
| `game_title_id_` | ivar of XeniaConfigViewController | The game whose settings are shown (0: global settings). | `src/xenia/ui/windowed_app_main_ios.mm` | 8103, 8111, 8120, 8247, 8264, 8284, 8797, 8827, 8829, 8833, 8835 | 11 |
| `global_sections_` | ivar of XeniaConfigViewController | The rows' global values, to save only what differs. | `src/xenia/ui/windowed_app_main_ios.mm` | 8105, 8127, 8128, 8132, 8263, 8829 | 6 |
| `initWithGameTitleId` | method of XeniaConfigViewController | Settings screen for one game. | `src/xenia/ui/windowed_app_main_ios.mm` | 8090, 8108, 12901 | 3 |
| `isGameOverrideAtIndexPath` | method of XeniaConfigViewController | Whether a row differs from global (shown as "Changed for this game"). | `src/xenia/ui/windowed_app_main_ios.mm` | 8119, 8570, 8600 | 3 |
| `useGlobalSettingsTapped` | method of XeniaConfigViewController | Deletes the game's overrides after confirming. | `src/xenia/ui/windowed_app_main_ios.mm` | 8289, 8785 | 2 |
| `presentGameSettingsForIndex` | method of XeniaViewController | Long-press menu > Game Settings. | `src/xenia/ui/windowed_app_main_ios.mm` | 12764, 12895 | 2 |
| `presentSettingsController` | method of XeniaViewController | Presents a settings screen (global or a game's), as Settings did. | `src/xenia/ui/windowed_app_main_ios.mm` | 12890, 12900, 12905 | 3 |
| `game_settings_action` | local in the library context menu | The Game Settings menu item. | `src/xenia/ui/windowed_app_main_ios.mm` | 12759, 12767, 12801 | 3 |
| `use_global` | local in XeniaConfigViewController viewDidLoad | The Use Global button. | `src/xenia/ui/windowed_app_main_ios.mm` | 8285, 8290 | 2 |
| `game_saved` | local in saveTapped: | Whether the game's config was written. | `src/xenia/ui/windowed_app_main_ios.mm` | 8830, 8833, 8838, 8843, 8844 | 5 |
| `game_alert` | local in saveTapped: | The saved alert for a game. | `src/xenia/ui/windowed_app_main_ios.mm` | 8842, 8853, 8859 | 3 |
| `copy_button` | local in the text setting editor | Copies the field's text. | `src/xenia/ui/windowed_app_main_ios.mm` | 8718, 8719, 8720, 8721, 8722, 8726 | 6 |
| `paste_button` | local in the text setting editor | Pastes the clipboard into the field. | `src/xenia/ui/windowed_app_main_ios.mm` | 8728, 8729, 8731, 8732, 8733, 8742 | 6 |
| `pasted` | local in the paste button handler | The clipboard text. | `src/xenia/ui/windowed_app_main_ios.mm` | 8734, 8735, 8736 | 3 |
| `text_stream` | local in ApplyGameConfigToSections | Writes an override as TOML text. | `src/xenia/ui/windowed_app_main_ios.mm` | 4058, 4059, 4060 | 3 |
| `metal_share_argument_encoders` | cvar; iOS settings key | Share one texture/sampler argument encoder between shaders with the same argument buffer layout instead of reflecting each compiled function (default on). Settings > All Options: Metal. | `src/xenia/gpu/metal/msl_shader.cc` | 89, 506, 2232, 2234 | 4 |
| `g_argument_encoder_cache_mutex` | file-local static (anonymous namespace) | Guards g_argument_encoder_cache (compile threads translate concurrently). | `src/xenia/gpu/metal/msl_shader.cc` | 509, 2238, 2249 | 3 |
| `g_argument_encoder_cache` | file-local static (anonymous namespace) | Argument encoders by GetArgumentBufferLayoutKey, one reference each, kept for the process lifetime. | `src/xenia/gpu/metal/msl_shader.cc` | 510, 2239, 2240, 2250, 2259 | 5 |
| `GetArgumentBufferLayoutKey` | function (anonymous namespace) | Hash of the spvDescriptorSetBuffer structs' member types and [[id]] slots (names left out) and the device; 0 if absent or unexpected. | `src/xenia/gpu/metal/msl_shader.cc` | 506, 517, 2235 | 3 |
| `argument_buffer_layout_key` | local in CompileToMsl | This translation's layout key, 0 when sharing is off or the key could not be made. | `src/xenia/gpu/metal/msl_shader.cc` | 2233, 2237, 2239, 2248, 2251, 2257 | 6 |
| `kStructPrefix` | constant in GetArgumentBufferLayoutKey | The text that starts an argument buffer struct in SPIRV-Cross output. | `src/xenia/gpu/metal/msl_shader.cc` | 519, 523 | 2 |
| `struct_begin` | local in GetArgumentBufferLayoutKey | Position of the struct being read. | `src/xenia/gpu/metal/msl_shader.cc` | 522, 523, 525, 526, 532, 564 | 8 |
| `struct_end` | local in GetArgumentBufferLayoutKey | Position of the struct's closing `};`. | `src/xenia/gpu/metal/msl_shader.cc` | 525, 527, 528, 535, 536, 564 | 6 |
| `id_pos` | local in GetArgumentBufferLayoutKey | Position of a member's `[[id(` attribute. | `src/xenia/gpu/metal/msl_shader.cc` | 540, 541, 547, 561 | 4 |
| `name_begin` | local in GetArgumentBufferLayoutKey | Start of the member name left out of the key. | `src/xenia/gpu/metal/msl_shader.cc` | 551, 552, 553, 554, 555, 557, 560 | 7 |
| `device_address` | local in GetArgumentBufferLayoutKey | The device pointer, hashed into the key. | `src/xenia/gpu/metal/msl_shader.cc` | 569, 570, 571 | 3 |
| `unresolved_calls` | static local in ResolveFunction (a64) | How many calls the a64 resolve thunk could not resolve (no function at the guest address); the first 16 are logged with the address and guest lr. | `src/xenia/cpu/backend/a64/a64_backend.cc` | 905, 906 | 2 |
| `host_crash_context` | local in Emulator::ExceptionCallback (ARM64) | Host registers of the fault, for the host PC, link register and JIT state registers in the crash dump. | `src/xenia/emulator.cc` | 1744, 1755, 1756, 1757, 1758 | 8 |
| `host_code_start` | local in Emulator::ExceptionCallback (ARM64) | Start of the machine code of the guest function the host PC was mapped to, for the offset in the crash dump. | `src/xenia/emulator.cc` | 1745, 1751, 1752, 1753, 1754 | 6 |
| `longjmp_resumes` | static local in ResolveFunction (a64) | How many calls resumed inside a live frame (longjmp); the first 16 are logged with the target, its function, the frame and r1. | `src/xenia/cpu/backend/a64/a64_backend.cc` | 879, 880 | 2 |
| `integer_bytes` | local in SpirvShaderTranslator::ProcessTextureFetchInstruction (integer textures) | Component width code the CPU put in bits 24:27 / 28:31 of the swizzle word: 0 none, 1 for 8 bits, 2 for 16 bits. | `src/xenia/gpu/spirv_shader_translator_fetch.cc` | 2413, 2429 | 2 |
| `integer_unsigned_max` | local in SpirvShaderTranslator::ProcessTextureFetchInstruction (integer textures) | 2^(8 * integer_bytes) - 1 as uint. | `src/xenia/gpu/spirv_shader_translator_fetch.cc` | 2423, 2435, 2442 | 3 |
| `integer_unsigned_scale` | local in SpirvShaderTranslator::ProcessTextureFetchInstruction (integer textures) | integer_unsigned_max as float, at least 1. | `src/xenia/gpu/spirv_shader_translator_fetch.cc` | 2432, 2464 | 2 |
| `integer_signed_scale` | local in SpirvShaderTranslator::ProcessTextureFetchInstruction (integer textures) | Half of integer_unsigned_max (2^(bits - 1) - 1) as float, at least 1. | `src/xenia/gpu/spirv_shader_translator_fetch.cc` | 2437, 2464 | 2 |
| `integer_remaining_components` | local in SpirvShaderTranslator::ProcessTextureFetchInstruction (integer textures) | Result components still to scale. | `src/xenia/gpu/spirv_shader_translator_fetch.cc` | 2445, 2448, 2450 | 3 |
| `integer_component_index` | local in SpirvShaderTranslator::ProcessTextureFetchInstruction (integer textures) | The result component being scaled. | `src/xenia/gpu/spirv_shader_translator_fetch.cc` | 2447, 2449, 2451, 2456, 2459, 2461 | 6 |
| `integer_sign_signed` | local in SpirvShaderTranslator::ProcessTextureFetchInstruction (integer textures) | Its sign is signed or unsigned biased (takes the signed scale). | `src/xenia/gpu/spirv_shader_translator_fetch.cc` | 2453, 2463 | 2 |
| `texture_integer_bytes` | local in MetalCommandProcessor::UpdateSpirvSystemConstantValues | For an integer (num_format) fetch: 1 for formats with 8 bits in every component, 2 for 16, else 0 (also 0 with a constant 1 in the swizzle); stored in bits 24:27 / 28:31 of texture_swizzles. | `src/xenia/gpu/metal/metal_command_processor.cc` | 12205, 12214, 12219, 12228, 12231 | 5 |
| `swizzle_component` | local in MetalCommandProcessor::UpdateSpirvSystemConstantValues (loop) | The guest swizzle component checked for a constant 1. | `src/xenia/gpu/metal/metal_command_processor.cc` | 12224, 12225, 12226 | 4 |
| `xma_census_next_ms` | function-local static atomic in XmaDecoder::WriteRegister | Steady-clock time (ms) of the next XMA census. | `src/xenia/apu/xma_decoder.cc` | 450, 455, 457 | 3 |
| `xma_census_now_ms` | local in XmaDecoder::WriteRegister | Steady-clock time (ms) at the kick. | `src/xenia/apu/xma_decoder.cc` | 451, 456, 458, 512 | 4 |
| `xma_census_due_ms` | local in XmaDecoder::WriteRegister | xma_census_next_ms as read, for the compare-exchange that lets one thread run the census. | `src/xenia/apu/xma_decoder.cc` | 455, 456, 457 | 3 |
| `xma_census_keys` | function-local static array in XmaDecoder::WriteRegister | Per context, its read and output offsets with the valid bits at the previous census. | `src/xenia/apu/xma_decoder.cc` | 461, 468, 485, 507 | 4 |
| `xma_census_allocated` | local in XmaDecoder::WriteRegister | Allocated XMA contexts counted by the census. | `src/xenia/apu/xma_decoder.cc` | 462, 471, 512 | 3 |
| `xma_census_holding` | local in XmaDecoder::WriteRegister | Allocated XMA contexts holding input, counted by the census. | `src/xenia/apu/xma_decoder.cc` | 462, 484, 512 | 3 |
| `xma_census_stalled` | local in XmaDecoder::WriteRegister | Contexts holding input whose state did not change since the previous census. | `src/xenia/apu/xma_decoder.cc` | 463, 486, 513 | 3 |
| `xma_census_context` | local in XmaDecoder::WriteRegister | The context being counted. | `src/xenia/apu/xma_decoder.cc` | 466, 467, 473 | 3 |
| `xma_census_data` | local in XmaDecoder::WriteRegister | A context's XMA_CONTEXT_DATA read from guest memory. | `src/xenia/apu/xma_decoder.cc` | 472, 475, 476, 477, 478, 479, 480, 481, 482, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504 | 24 |
| `xma_census_key` | local in XmaDecoder::WriteRegister | A context's state packed for comparison with xma_census_keys. | `src/xenia/apu/xma_decoder.cc` | 474, 485, 507 | 3 |
| `xma_census_list` | local in XmaDecoder::WriteRegister | The fields of up to three stalled contexts, appended to the census line. | `src/xenia/apu/xma_decoder.cc` | 464, 487, 513 | 3 |
| `use_dedicated_thread` | local in XmaDecoder::WriteRegister | use_dedicated_xma_thread as read once for a kick, so the kick is decoded either on the kicking thread or by the worker. | `src/xenia/apu/xma_decoder.cc` | 424, 429, 434 | 3 |
| `metal_resolve_keep_command_buffer` | cvar (bool, Metal) | SPIRV-Cross path: keep the command buffer open after a resolve in a submission with draws and load textures read from resolved memory into it. Default false since build 9676 (GTA IV's main pass restarted 24-88 times a frame, ACR menus flickered); per-game opt-in. | `src/xenia/gpu/metal/metal_command_processor.cc` | 155, 4828, 4872, 7079 | 4 |
| `resolve_keep_command_buffer` | local in DrawReadsPendingResolveOutput | metal_resolve_keep_command_buffer with blit texture uploads on: load in the current command buffer instead of committing it. | `src/xenia/gpu/metal/metal_command_processor.cc` | 4827, 4832 | 2 |
| `resolve_check_mask` | local in DrawReadsPendingResolveOutput | Texture slots checked against resolved memory: all used slots, or with metal_resolve_keep_command_buffer only those whose texture would be loaded. | `src/xenia/gpu/metal/metal_command_processor.cc` | 4830, 4836 | 2 |
| `would_load` | local lambda in TextureCache::GetUsedTexturePotentialLoadMask | Whether RequestTextures would load the texture for a key: none cached, or the cached one outdated (always with resolution scaling). | `src/xenia/gpu/texture_cache.cc` | 509, 533, 537, 540 | 4 |
| `texture_it` | local in would_load | The cached texture for the key. | `src/xenia/gpu/texture_cache.cc` | 513, 514, 515, 516 | 4 |
| `signed_key` | local in GetUsedTexturePotentialLoadMask | Key of the separate signed version; same name and role as in RequestTextures. | `src/xenia/gpu/texture_cache.cc` | 535, 536, 537 | 3 |
| `metal_shader_fast_math` | cvar (bool, GPU) | Compile SPIRV-Cross Metal shaders with fast math (default on); off compiles IEEE math that keeps infinities and NaN, for games relying on 0 x infinity = 0 or on infinities (A/B for Spider-Man: Edge of Time). Settings > All Options: GPU. | `src/xenia/gpu/metal/msl_shader.cc` | 62, 72, 2101 | 3 |
| `metal_vertex_shader_fast_math` | cvar (bool, GPU) | Vertex shaders also use fast math (default off): off keeps positions bit-identical between shaders computing them with different but equivalent instructions (depth pre-pass then an equal-depth pass, Spider-Man: Edge of Time's hypersense copies). Settings > All Options: GPU. | `src/xenia/gpu/metal/msl_shader.cc` | 61, 2103 | 2 |
| `sleep_zero_backoff_us` | cvar (uint32, Kernel) | Sleep length for a guest thread polling with Sleep(0); 0 always yields. Default 50. | `src/xenia/kernel/xthread.cc` | 50, 1275 | 2 |
| `sleep_zero_last_us` | thread_local in XThread::Delay | Time (us) of this thread's previous Sleep(0). | `src/xenia/kernel/xthread.cc` | 1273, 1281, 1285 | 3 |
| `sleep_zero_run` | thread_local in XThread::Delay | Quick Sleep(0) calls in a row on this thread. | `src/xenia/kernel/xthread.cc` | 1274, 1281, 1283, 1287 | 4 |
| `sleep_zero_backoff` | local in XThread::Delay | sleep_zero_backoff_us as read. | `src/xenia/kernel/xthread.cc` | 1275, 1276, 1282, 1287, 1288 | 5 |
| `sleep_zero_now_us` | local in XThread::Delay | Steady-clock time (us) of this Sleep(0). | `src/xenia/kernel/xthread.cc` | 1277, 1281, 1285 | 3 |
| `kSysFlag_Color0Fixed16Scaled_Shift` | enumerator of SpirvShaderTranslator system flags | Bit of kSysFlag_Color0Fixed16Scaled. | `src/xenia/gpu/spirv_shader_translator.h` | 165, 208 | 2 |
| `kSysFlag_Color0Fixed16Scaled` | enumerator of SpirvShaderTranslator system flags | Render target 0 is 16_16 / 16_16_16_16 divided by 32 for the host; the RT0 pre-multiply factors are taken from the output times 32. | `src/xenia/gpu/spirv_shader_translator.h` | 208 | 1 |
| `kSysFlag_Color0Fixed16Scaled` | enumerator of SpirvShaderTranslator system flags | Render target 0 is 16_16 / 16_16_16_16 divided by 32 for the host; the RT0 pre-multiply factors are taken from the output times 32. | `src/xenia/gpu/spirv_shader_translator_rb.cc` | 1343 | 1 |
| `kSysFlag_Color0Fixed16Scaled` | enumerator of SpirvShaderTranslator system flags | Render target 0 is 16_16 / 16_16_16_16 divided by 32 for the host; the RT0 pre-multiply factors are taken from the output times 32. | `src/xenia/gpu/metal/metal_command_processor.cc` | 12104 | 1 |
| `factor_color` | local in CompleteFragmentShaderInMain (FBO path) | RT0 output the pre-multiply factors are read from: times 32 with kSysFlag_Color0Fixed16Scaled. | `src/xenia/gpu/spirv_shader_translator_rb.cc` | 1334, 1350 | 2 |
| `GetFixed16SourceFactorPremult` | function (anonymous namespace) | For a 16_16 / 16_16_16_16 RT0 divided by 32: the source factors to pre-multiply in the shader when only they read the source and the ops are add/subtract. | `src/xenia/gpu/metal/metal_command_processor.cc` | 610, 659, 11744 | 3 |
| `ApplyFixed16SourceFactorPremult` | function (anonymous namespace) | Sets RT0 host source factors to one where GetFixed16SourceFactorPremult pre-multiplies (RG16Snorm / RGBA16Snorm attachments). | `src/xenia/gpu/metal/metal_command_processor.cc` | 650, 1175, 2843, 11589 | 4 |
| `linear_op` | local lambda in GetFixed16SourceFactorPremult | Whether a blend op is add, subtract or reverse subtract. | `src/xenia/gpu/metal/metal_command_processor.cc` | 618, 629, 630 | 3 |
| `reads_source` | local lambda in GetFixed16SourceFactorPremult | Whether a blend factor reads the source color or alpha. | `src/xenia/gpu/metal/metal_command_processor.cc` | 622, 631, 632, 636, 640 | 5 |
| `rgb_factor_out` | parameter of GetFixed16SourceFactorPremult | RGB factor to pre-multiply by (kOne for none). | `src/xenia/gpu/metal/metal_command_processor.cc` | 611, 613, 638, 644 | 4 |
| `a_factor_out` | parameter of GetFixed16SourceFactorPremult | Alpha factor to pre-multiply by (kOne for none). | `src/xenia/gpu/metal/metal_command_processor.cc` | 612, 614, 642, 645 | 4 |
| `rgb_premult` | local in ApplyFixed16SourceFactorPremult and GetCurrentSpirvPixelShaderModification | RGB result of GetFixed16SourceFactorPremult. | `src/xenia/gpu/metal/metal_command_processor.cc` | 658, 659, 662, 11741, 11747, 11748 | 6 |
| `a_premult` | local in ApplyFixed16SourceFactorPremult and GetCurrentSpirvPixelShaderModification | Alpha result of GetFixed16SourceFactorPremult. | `src/xenia/gpu/metal/metal_command_processor.cc` | 658, 659, 665, 11741, 11747, 11749 | 6 |
| `DrawReadsPendingResolveOutput` | MetalCommandProcessor member function | Whether a draw reads a texture a resolve still in the open command buffer wrote (the copy->draw split check, moved out of IssueDrawMsl). | `src/xenia/gpu/metal/metal_command_processor.h` | 443 | 1 |
| `DrawReadsPendingResolveOutput` | MetalCommandProcessor member function | Whether a draw reads a texture a resolve still in the open command buffer wrote (the copy->draw split check, moved out of IssueDrawMsl). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3600, 4815, 5151 | 3 |
| `SplitForPendingResolveOutput` | MetalCommandProcessor member function | The copy->draw split: ends the render pass and loads the textures (keep-CB), or commits the command buffer. | `src/xenia/gpu/metal/metal_command_processor.h` | 445 | 1 |
| `SplitForPendingResolveOutput` | MetalCommandProcessor member function | The copy->draw split: ends the render pass and loads the textures (keep-CB), or commits the command buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3601, 4870, 5152 | 3 |
| `known_texture_mask` | local in IssueDraw | Textures of already translated shaders, for splitting before the render pass begins. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3593, 3596, 3599, 3600, 3601 | 5 |
| `metal_spirv2air_probe` | cvar (bool, Metal) | Also compile each guest shader with spirv2air (iOS 16 target) without drawing with it, and log Metal's acceptance, rejections and timings ("spirv2air probe" lines); not with metal_shader_compiler "spirv2air". | `src/xenia/gpu/metal/msl_shader.cc` | 102, 2479 | 2 |
| `metal_shader_compiler` | cvar (string, Metal) | Guest shader compiler: "msl" (SPIRV-Cross, default) or "spirv2air" (falls back to msl per shader and for tessellation). Same name and role as XeniOS's, which selects between spirv2air and msc on its DXIL path. | `src/xenia/gpu/metal/msl_shader.cc` | 146, 431, 1622, 2480 | 4 |
| `Spirv2airProbeStats` | struct (anonymous namespace) | spirv2air totals (probe or compiler). | `src/xenia/gpu/metal/msl_shader.cc` | 181, 201, 364 | 3 |
| `shaders` | member of Spirv2airProbeStats | Shaders given to spirv2air. | `src/xenia/gpu/metal/msl_shader.cc` | 182, 365, 415, 432, 435 | 6 |
| `translated` | member of Spirv2airProbeStats | Shaders spirv2air translated (or found in the disk cache). | `src/xenia/gpu/metal/msl_shader.cc` | 183, 375, 432, 435 | 4 |
| `libraries` | member of Spirv2airProbeStats | Libraries Metal accepted. | `src/xenia/gpu/metal/msl_shader.cc` | 184, 377, 432 | 3 |
| `functions` | member of Spirv2airProbeStats | Libraries whose main function Metal returned. | `src/xenia/gpu/metal/msl_shader.cc` | 185, 380, 420, 433 | 4 |
| `cached` | member of Spirv2airProbeStats, local in CompileSpirv2air | Accepted shaders whose library came from metal_shader_disk_cache / this one did. | `src/xenia/gpu/metal/msl_shader.cc` | 187, 311, 318, 321, 356, 381, 382, 433 | 8 |
| `translate_ms` | member of Spirv2airProbeStats, local in CompileSpirv2air | spirv2air translation (or cache load) time (sum over accepted shaders / of this shader). | `src/xenia/gpu/metal/msl_shader.cc` | 189, 325, 384, 433 | 5 |
| `library_ms` | member of Spirv2airProbeStats, local in CompileSpirv2air | newLibraryWithData plus newFunction time. | `src/xenia/gpu/metal/msl_shader.cc` | 190, 331, 354, 385, 434 | 6 |
| `msl_library_ms` | member of Spirv2airProbeStats, parameter of CompileSpirv2air and ProbeSpirv2air | The Metal Shading Language library compile time of the same shader (probe only, else 0). | `src/xenia/gpu/metal/msl_shader.cc` | 191, 300, 386, 422, 424, 446, 450 | 8 |
| `rejections` | member of Spirv2airProbeStats, local in CompileSpirv2air | spirv2air rejection reasons with counts (and their text for the summary). | `src/xenia/gpu/metal/msl_shader.cc` | 192, 369, 416, 417, 418, 435 | 6 |
| `library_errors` | member of Spirv2airProbeStats | Metal library or function errors with counts. | `src/xenia/gpu/metal/msl_shader.cc` | 193, 389 | 2 |
| `dumped` | member of Spirv2airProbeStats | Modules written to spirv2air_probe (at most 16). | `src/xenia/gpu/metal/msl_shader.cc` | 194, 398, 399 | 3 |
| `g_spirv2air_probe_mutex` | global (anonymous namespace) | Guards g_spirv2air_probe_stats. | `src/xenia/gpu/metal/msl_shader.cc` | 200, 363 | 2 |
| `g_spirv2air_probe_stats` | global (anonymous namespace) | The spirv2air totals. | `src/xenia/gpu/metal/msl_shader.cc` | 201, 364 | 2 |
| `kSpirv2airOptionsRevision` | constant (anonymous namespace) | Bumped when GetSpirv2airOptions changes; seeds the disk cache key. | `src/xenia/gpu/metal/msl_shader.cc` | 205, 268 | 2 |
| `GetSpirv2airOptions` | function (anonymous namespace) | spirv2air options for this backend's SPIR-V and the draw path's Spirv2airLayout. | `src/xenia/gpu/metal/msl_shader.cc` | 212, 323, 807 | 3 |
| `GetSpirvExecutionModel` | function (anonymous namespace) | The execution model of a SPIR-V module's first entry point (spirv2air takes vertex and fragment shaders). | `src/xenia/gpu/metal/msl_shader.cc` | 272, 1625 | 2 |
| `word_count_op` | local in GetSpirvExecutionModel | The instruction's word count. | `src/xenia/gpu/metal/msl_shader.cc` | 275, 276, 283 | 3 |
| `CompileSpirv2air` | function (anonymous namespace) | Compiles SPIR-V with spirv2air (or loads it from the disk cache), loads the library and main function and records the result; used by the probe and the compiler. | `src/xenia/gpu/metal/msl_shader.cc` | 297, 449, 1636 | 3 |
| `library_out` | parameter of CompileSpirv2air | Where the library goes. | `src/xenia/gpu/metal/msl_shader.cc` | 301, 303, 437 | 3 |
| `function_out` | parameter of CompileSpirv2air | Where the main function goes. | `src/xenia/gpu/metal/msl_shader.cc` | 302, 304, 438 | 3 |
| `ProbeSpirv2air` | function (anonymous namespace) | metal_spirv2air_probe: CompileSpirv2air without the disk cache, releasing what it made. | `src/xenia/gpu/metal/msl_shader.cc` | 444, 2481 | 2 |
| `translate_begin` | local in CompileSpirv2air | Start of the translation. | `src/xenia/gpu/metal/msl_shader.cc` | 309, 326 | 2 |
| `library_error` | local in CompileSpirv2air | Metal's error for the library or function. | `src/xenia/gpu/metal/msl_shader.cc` | 328, 345, 350, 388, 393 | 5 |
| `library_begin` | local in CompileSpirv2air | Start of the library load. | `src/xenia/gpu/metal/msl_shader.cc` | 334, 354 | 2 |
| `accepted` | local in CompileSpirv2air | Accepted shaders (at least 1) to average over. | `src/xenia/gpu/metal/msl_shader.cc` | 420, 424, 433, 434 | 4 |
| `msl_compile` | local in CompileSpirv2air | The Metal Shading Language compile time for the summary, when the probe measured it. | `src/xenia/gpu/metal/msl_shader.cc` | 421, 423, 434 | 3 |
| `cached_metallib` | local in CompileSpirv2air | The disk cache entry. | `src/xenia/gpu/metal/msl_shader.cc` | 313, 314, 315, 316, 317 | 5 |
| `spirv2air_model` | local in MslTranslation::CompileToMsl | The module's execution model. | `src/xenia/gpu/metal/msl_shader.cc` | 1624, 1626, 1627 | 3 |
| `spirv2air_cache_key` | local in MslTranslation::CompileToMsl | Disk cache key: the SPIR-V hashed with the spirv2air version, kSpirv2airOptionsRevision and the platform. | `src/xenia/gpu/metal/msl_shader.cc` | 1629, 1632, 1640 | 3 |
| `spirv2air_library_` | MslTranslation member | The spirv2air library. | `src/xenia/gpu/metal/msl_shader.cc` | 1230, 1231, 1232, 1640 | 4 |
| `spirv2air_function_` | MslTranslation member | The spirv2air main function. | `src/xenia/gpu/metal/msl_shader.cc` | 1226, 1227, 1228, 1622, 1640 | 5 |
| `spirv2air_library_` | MslTranslation member | The spirv2air library. | `src/xenia/gpu/metal/msl_shader.h` | 80 | 1 |
| `spirv2air_function_` | MslTranslation member | The spirv2air main function. | `src/xenia/gpu/metal/msl_shader.h` | 53, 60, 81 | 3 |
| `spirv2air_function` | MslTranslation method | The spirv2air main function, if compiled with metal_shader_compiler "spirv2air". | `src/xenia/gpu/metal/msl_shader.h` | 53 | 1 |
| `spirv2air_function` | MslTranslation method | The spirv2air main function, if compiled with metal_shader_compiler "spirv2air". | `src/xenia/gpu/metal/metal_command_processor.cc` | 5053, 5054, 11782, 11786 | 4 |
| `Spirv2airLayout` | namespace | Buffer indices and top-level argument buffer and heap layout of spirv2air shaders. | `src/xenia/gpu/metal/msl_bindings.h` | 91 | 1 |
| `Spirv2airLayout` | namespace | Buffer indices and top-level argument buffer and heap layout of spirv2air shaders. | `src/xenia/gpu/metal/msl_shader.cc` | 220, 221, 222, 228, 229, 230, 232, 234, 235, 236, 238, 244, 246, 247, 249, 258, 261, 723, 724, 726 | 20 |
| `Spirv2airLayout` | namespace | Buffer indices and top-level argument buffer and heap layout of spirv2air shaders. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1854, 1865, 5993, 5999, 6001, 6007, 6015, 6023, 6044, 6046, 6049, 6051, 6057, 6096, 6097, 6121, 6128, 6137, 6147, 6150, 6153, 6156, 6159, 6162 | 24 |
| `kResourceHeapBuffer` | constant in Spirv2airLayout | Buffer index of the resource heap (9). | `src/xenia/gpu/metal/msl_bindings.h` | 92 | 1 |
| `kResourceHeapBuffer` | constant in Spirv2airLayout | Buffer index of the resource heap (9). | `src/xenia/gpu/metal/msl_shader.cc` | 220 | 1 |
| `kResourceHeapBuffer` | constant in Spirv2airLayout | Buffer index of the resource heap (9). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6150, 6159 | 2 |
| `kSamplerHeapBuffer` | constant in Spirv2airLayout | Buffer index of the sampler heap (10). | `src/xenia/gpu/metal/msl_bindings.h` | 93 | 1 |
| `kSamplerHeapBuffer` | constant in Spirv2airLayout | Buffer index of the sampler heap (10). | `src/xenia/gpu/metal/msl_shader.cc` | 221 | 1 |
| `kSamplerHeapBuffer` | constant in Spirv2airLayout | Buffer index of the sampler heap (10). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6153, 6162 | 2 |
| `kArgumentBuffer` | constant in Spirv2airLayout | Buffer index of the top-level argument buffer (11). | `src/xenia/gpu/metal/msl_bindings.h` | 94 | 1 |
| `kArgumentBuffer` | constant in Spirv2airLayout | Buffer index of the top-level argument buffer (11). | `src/xenia/gpu/metal/msl_shader.cc` | 222 | 1 |
| `kArgumentBuffer` | constant in Spirv2airLayout | Buffer index of the top-level argument buffer (11). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6147, 6156 | 2 |
| `kConstantBufferCount` | constant in Spirv2airLayout | Constant buffer pointers at the top-level argument buffer's start (7). | `src/xenia/gpu/metal/msl_bindings.h` | 95, 96 | 2 |
| `kConstantBufferCount` | constant in Spirv2airLayout | Constant buffer pointers at the top-level argument buffer's start (7). | `src/xenia/gpu/metal/msl_shader.cc` | 228 | 1 |
| `kSharedMemoryReadOffset` | constant in Spirv2airLayout | Offset of the shared memory pointer for reading. | `src/xenia/gpu/metal/msl_bindings.h` | 96, 97 | 2 |
| `kSharedMemoryReadOffset` | constant in Spirv2airLayout | Offset of the shared memory pointer for reading. | `src/xenia/gpu/metal/msl_shader.cc` | 229 | 1 |
| `kSharedMemoryReadOffset` | constant in Spirv2airLayout | Offset of the shared memory pointer for reading. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6044 | 1 |
| `kSharedMemoryWriteOffset` | constant in Spirv2airLayout | Offset of the shared memory pointer for writing. | `src/xenia/gpu/metal/msl_bindings.h` | 97, 98 | 2 |
| `kSharedMemoryWriteOffset` | constant in Spirv2airLayout | Offset of the shared memory pointer for writing. | `src/xenia/gpu/metal/msl_shader.cc` | 230 | 1 |
| `kSharedMemoryWriteOffset` | constant in Spirv2airLayout | Offset of the shared memory pointer for writing. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6046 | 1 |
| `kVertexTextureIndicesOffset` | constant in Spirv2airLayout | Offset of the vertex stage's index records pointer. | `src/xenia/gpu/metal/msl_bindings.h` | 98, 100 | 2 |
| `kVertexTextureIndicesOffset` | constant in Spirv2airLayout | Offset of the vertex stage's index records pointer. | `src/xenia/gpu/metal/msl_shader.cc` | 232 | 1 |
| `kVertexTextureIndicesOffset` | constant in Spirv2airLayout | Offset of the vertex stage's index records pointer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6049 | 1 |
| `kFragmentTextureIndicesOffset` | constant in Spirv2airLayout | Offset of the fragment stage's index records pointer. | `src/xenia/gpu/metal/msl_bindings.h` | 99, 101 | 2 |
| `kFragmentTextureIndicesOffset` | constant in Spirv2airLayout | Offset of the fragment stage's index records pointer. | `src/xenia/gpu/metal/msl_shader.cc` | 234 | 1 |
| `kFragmentTextureIndicesOffset` | constant in Spirv2airLayout | Offset of the fragment stage's index records pointer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6051 | 1 |
| `kArgumentBufferBytes` | constant in Spirv2airLayout | Size of the top-level argument buffer. | `src/xenia/gpu/metal/msl_bindings.h` | 101 | 1 |
| `kArgumentBufferBytes` | constant in Spirv2airLayout | Size of the top-level argument buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6007, 6023 | 2 |
| `kHeapRecordBytes` | constant in Spirv2airLayout | Size of a heap record. | `src/xenia/gpu/metal/msl_bindings.h` | 102 | 1 |
| `kHeapRecordBytes` | constant in Spirv2airLayout | Size of a heap record. | `src/xenia/gpu/metal/msl_shader.cc` | 235 | 1 |
| `kHeapRecordBytes` | constant in Spirv2airLayout | Size of a heap record. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6015, 6057, 6096, 6121 | 4 |
| `kHeapTextureHandleOffset` | constant in Spirv2airLayout | Offset of a texture's handle in its heap record. | `src/xenia/gpu/metal/msl_bindings.h` | 103 | 1 |
| `kHeapTextureHandleOffset` | constant in Spirv2airLayout | Offset of a texture's handle in its heap record. | `src/xenia/gpu/metal/msl_shader.cc` | 236 | 1 |
| `kHeapTextureHandleOffset` | constant in Spirv2airLayout | Offset of a texture's handle in its heap record. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6097 | 1 |
| `kMaxBindings` | constant in Spirv2airLayout | Records in the {binding, binding} index table. | `src/xenia/gpu/metal/msl_bindings.h` | 106 | 1 |
| `kMaxBindings` | constant in Spirv2airLayout | Records in the {binding, binding} index table. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1854, 1865, 5999, 6001 | 4 |
| `spirv2air_texture_indices_` | MetalCommandProcessor member | The {binding, binding} index records of every spirv2air stage. | `src/xenia/gpu/metal/metal_command_processor.h` | 805 | 1 |
| `spirv2air_texture_indices_` | MetalCommandProcessor member | The {binding, binding} index records of every spirv2air stage. | `src/xenia/gpu/metal/metal_command_processor.cc` | 835, 836, 837, 1853, 1856, 1860, 1864, 6048, 6174 | 9 |
| `records` | local in SetupContext | The index records being filled. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1863, 1866, 1867 | 3 |
| `use_spirv2air` | local in IssueDrawMsl | The draw's pipeline takes its shaders from spirv2air (both have a spirv2air function and it isn't tessellated); false again when Metal rejects the spirv2air pipeline. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5052, 5081, 5095, 5101, 5104, 5111, 5389, 6167, 6178, 6182, 6354, 6358 | 12 |
| `compile_msl_for_draw` | local lambda in IssueDrawMsl | Compiles the Metal Shading Language function of a draw's shaders that only have a spirv2air one, for a draw that can't use spirv2air. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5055, 5081, 5112 | 3 |
| `metal_spirv2air_self_test` | cvar (bool, default false) | Run RunSpirv2airSelfTest when a title's shader storage is set up, before the title runs. | `src/xenia/gpu/metal/msl_shader.cc` | 112 | 1 |
| `metal_spirv2air_self_test` | cvar (bool, default false) | Run RunSpirv2airSelfTest when a title's shader storage is set up, before the title runs. | `src/xenia/gpu/metal/metal_command_processor.cc` | 96, 2401 | 2 |
| `metal_spirv2air_options` | cvar (int, default 3) | The Spirv2airLayout option bits 0-3 guest shaders are compiled with by spirv2air. | `src/xenia/gpu/metal/msl_shader.cc` | 122, 257 | 2 |
| `metal_spirv2air_declaration_attributes` | cvar (int, default 2) | spirv2air's declarationAttributes for guest shaders (Spirv2airLayout bits 4-5). | `src/xenia/gpu/metal/msl_shader.cc` | 134, 260 | 2 |
| `RunSpirv2airSelfTest` | function | Builds pipelines one at a time, each with a time limit, from the self-test shaders (those with calls with each declaration attributes mode) and the pairs of shaders in spirv2air_probe, and logs which ones Metal builds. | `src/xenia/gpu/metal/msl_shader.h` | 101 | 1 |
| `RunSpirv2airSelfTest` | function | Builds pipelines one at a time, each with a time limit, from the self-test shaders (those with calls with each declaration attributes mode) and the pairs of shaders in spirv2air_probe, and logs which ones Metal builds. | `src/xenia/gpu/metal/msl_shader.cc` | 668 | 1 |
| `RunSpirv2airSelfTest` | function | Builds pipelines one at a time, each with a time limit, from the self-test shaders (those with calls with each declaration attributes mode) and the pairs of shaders in spirv2air_probe, and logs which ones Metal builds. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2410 | 1 |
| `DumpSpirv2airPipelineShaders` | function | Writes the SPIR-V of the shaders of a pipeline Metal refused to build from their spirv2air functions to spirv2air_probe, named after the pair (first 8 pairs). | `src/xenia/gpu/metal/msl_shader.h` | 106 | 1 |
| `DumpSpirv2airPipelineShaders` | function | Writes the SPIR-V of the shaders of a pipeline Metal refused to build from their spirv2air functions to spirv2air_probe, named after the pair (first 8 pairs). | `src/xenia/gpu/metal/msl_shader.cc` | 1060 | 1 |
| `DumpSpirv2airPipelineShaders` | function | Writes the SPIR-V of the shaders of a pipeline Metal refused to build from their spirv2air functions to spirv2air_probe, named after the pair (first 8 pairs). | `src/xenia/gpu/metal/metal_command_processor.cc` | 5110 | 1 |
| `GetSpirv2airCacheSeed` | function (anonymous namespace) | The cache key seed of a variant's spirv2air libraries. | `src/xenia/gpu/metal/msl_shader.cc` | 265, 1634 | 2 |
| `GetSpirv2airVariant` | function (anonymous namespace) | The variant guest shaders are compiled with, from metal_spirv2air_options and metal_spirv2air_declaration_attributes. | `src/xenia/gpu/metal/msl_shader.cc` | 256, 449, 1628 | 3 |
| `variant` | parameter of GetSpirv2airOptions, GetSpirv2airCacheSeed, CompileSpirv2air, function_of and build; Test field | Spirv2airLayout bits. | `src/xenia/gpu/metal/msl_shader.cc` | 212, 238, 241, 244, 246, 247, 249, 265, 268, 297, 323, 721, 799, 800, 807, 812, 830, 852, 894, 901, 904, 909, 911, 916, 919, 1021 | 26 |
| `spirv2air_variant` | local in MslTranslation::CompileToMsl | The variant for this compile. | `src/xenia/gpu/metal/msl_shader.cc` | 1628, 1634, 1637, 1641 | 4 |
| `spirv2air_options_` | MslTranslation member | The variant spirv2air_function_ was compiled with. | `src/xenia/gpu/metal/msl_shader.h` | 56, 82 | 2 |
| `spirv2air_options_` | MslTranslation member | The variant spirv2air_function_ was compiled with. | `src/xenia/gpu/metal/msl_shader.cc` | 1641 | 1 |
| `spirv2air_options` | MslTranslation member function | Returns spirv2air_options_. | `src/xenia/gpu/metal/msl_shader.h` | 56 | 1 |
| `spirv2air_options` | MslTranslation member function | Returns spirv2air_options_. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5992 | 1 |
| `kDirectSharedMemory` | constant in Spirv2airLayout | Variant bit: shared memory as the stage's kSharedMemory buffer binding. | `src/xenia/gpu/metal/msl_bindings.h` | 120 | 1 |
| `kDirectSharedMemory` | constant in Spirv2airLayout | Variant bit: shared memory as the stage's kSharedMemory buffer binding. | `src/xenia/gpu/metal/msl_shader.cc` | 238, 723 | 2 |
| `kDirectTextures` | constant in Spirv2airLayout | Variant bit: texture k at texture index k and sampler j at sampler index j, no heaps. | `src/xenia/gpu/metal/msl_bindings.h` | 121 | 1 |
| `kDirectTextures` | constant in Spirv2airLayout | Variant bit: texture k at texture index k and sampler j at sampler index j, no heaps. | `src/xenia/gpu/metal/msl_shader.cc` | 244, 724 | 2 |
| `kDirectTextures` | constant in Spirv2airLayout | Variant bit: texture k at texture index k and sampler j at sampler index j, no heaps. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5993 | 1 |
| `kDirectConstantBuffers` | constant in Spirv2airLayout | Variant bit, always set for guest shaders: constant buffer n at buffer index kSystemConstants + n, where the Metal Shading Language path binds it, so the draw passes the constants as that path does. | `src/xenia/gpu/metal/msl_bindings.h` | 126 | 1 |
| `kDirectConstantBuffers` | constant in Spirv2airLayout | Variant bit, always set for guest shaders: constant buffer n at buffer index kSystemConstants + n, where the Metal Shading Language path binds it, so the draw passes the constants as that path does. | `src/xenia/gpu/metal/msl_shader.cc` | 241, 259, 735 | 3 |
| `kDirectConstantBuffers` | constant in Spirv2airLayout | Variant bit, always set for guest shaders: constant buffer n at buffer index kSystemConstants + n, where the Metal Shading Language path binds it, so the draw passes the constants as that path does. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5393, 6030 | 2 |
| `kConstantTextureIndices` | constant in Spirv2airLayout | Variant bit: the index records read through a constant pointer. | `src/xenia/gpu/metal/msl_bindings.h` | 122 | 1 |
| `kConstantTextureIndices` | constant in Spirv2airLayout | Variant bit: the index records read through a constant pointer. | `src/xenia/gpu/metal/msl_shader.cc` | 246 | 1 |
| `kPreciseMath` | constant in Spirv2airLayout | Variant bit: precise math intrinsics. | `src/xenia/gpu/metal/msl_bindings.h` | 123 | 1 |
| `kPreciseMath` | constant in Spirv2airLayout | Variant bit: precise math intrinsics. | `src/xenia/gpu/metal/msl_shader.cc` | 247 | 1 |
| `kOptionMask` | constant in Spirv2airLayout | Bits 0-3, metal_spirv2air_options's. | `src/xenia/gpu/metal/msl_bindings.h` | 124 | 1 |
| `kOptionMask` | constant in Spirv2airLayout | Bits 0-3, metal_spirv2air_options's. | `src/xenia/gpu/metal/msl_shader.cc` | 258 | 1 |
| `kDeclarationAttributesShift` | constant in Spirv2airLayout | Where spirv2air's declarationAttributes mode sits in a variant. | `src/xenia/gpu/metal/msl_bindings.h` | 125 | 1 |
| `kDeclarationAttributesShift` | constant in Spirv2airLayout | Where spirv2air's declarationAttributes mode sits in a variant. | `src/xenia/gpu/metal/msl_shader.cc` | 249, 261, 726 | 3 |
| `pipelines_dumped` | Spirv2airProbeStats field | Pairs of shaders DumpSpirv2airPipelineShaders wrote this run. | `src/xenia/gpu/metal/msl_shader.cc` | 197, 1080, 1084 | 3 |
| `dumped_pipeline_shaders` | Spirv2airProbeStats field | Their shaders' hashes and modifications. | `src/xenia/gpu/metal/msl_shader.cc` | 198, 1081 | 2 |
| `spirv2air_self_test` | namespace | The self-test shaders' SPIR-V. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 26 | 1 |
| `spirv2air_self_test` | namespace | The self-test shaders' SPIR-V. | `src/xenia/gpu/metal/msl_shader.cc` | 670 | 1 |
| `kVertexPosition` | constant array in spirv2air_self_test | SPIR-V of the self-test's vertex shader writing only the position. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 35 | 1 |
| `kVertexPosition` | constant array in spirv2air_self_test | SPIR-V of the self-test's vertex shader writing only the position. | `src/xenia/gpu/metal/msl_shader.cc` | 682 | 2 |
| `kVertexOutputs` | constant array in spirv2air_self_test | SPIR-V of the self-test's vertex shader with two outputs. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 91 | 1 |
| `kVertexOutputs` | constant array in spirv2air_self_test | SPIR-V of the self-test's vertex shader with two outputs. | `src/xenia/gpu/metal/msl_shader.cc` | 683, 684 | 2 |
| `kVertexSharedMemory` | constant array in spirv2air_self_test | SPIR-V of the self-test's vertex shader reading shared memory. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 159 | 1 |
| `kVertexSharedMemory` | constant array in spirv2air_self_test | SPIR-V of the self-test's vertex shader reading shared memory. | `src/xenia/gpu/metal/msl_shader.cc` | 686, 687 | 2 |
| `kPixelConstant` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader writing a constant. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 214 | 1 |
| `kPixelConstant` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader writing a constant. | `src/xenia/gpu/metal/msl_shader.cc` | 688, 689 | 2 |
| `kPixelInput` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with an input. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 237 | 1 |
| `kPixelInput` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with an input. | `src/xenia/gpu/metal/msl_shader.cc` | 690, 691 | 2 |
| `kPixelCentroidInput` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with a centroid input. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 260 | 1 |
| `kPixelCentroidInput` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with a centroid input. | `src/xenia/gpu/metal/msl_shader.cc` | 692, 693 | 2 |
| `kPixelConstantBuffer` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader reading a constant buffer. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 283 | 1 |
| `kPixelConstantBuffer` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader reading a constant buffer. | `src/xenia/gpu/metal/msl_shader.cc` | 695, 696 | 2 |
| `kPixelConstantBufferIndexed` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader indexing a constant buffer. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 330 | 1 |
| `kPixelConstantBufferIndexed` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader indexing a constant buffer. | `src/xenia/gpu/metal/msl_shader.cc` | 698, 699 | 2 |
| `kPixelSharedMemory` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader reading shared memory. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 372 | 1 |
| `kPixelSharedMemory` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader reading shared memory. | `src/xenia/gpu/metal/msl_shader.cc` | 700, 701 | 2 |
| `kPixelMath` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with math functions. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 416 | 1 |
| `kPixelMath` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with math functions. | `src/xenia/gpu/metal/msl_shader.cc` | 702, 703 | 2 |
| `kPixelTexture` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader sampling a texture. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 464 | 1 |
| `kPixelTexture` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader sampling a texture. | `src/xenia/gpu/metal/msl_shader.cc` | 704, 705 | 2 |
| `kPixelTextureArrayGradients` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader sampling a texture array with gradients. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 505 | 1 |
| `kPixelTextureArrayGradients` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader sampling a texture array with gradients. | `src/xenia/gpu/metal/msl_shader.cc` | 707, 708 | 2 |
| `kPixelDiscard` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with discard. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 554 | 1 |
| `kPixelDiscard` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with discard. | `src/xenia/gpu/metal/msl_shader.cc` | 709, 710 | 2 |
| `kPixelEarlyFragmentTests` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with early fragment tests. | `src/xenia/gpu/metal/spirv2air_self_test_spirv.h` | 584 | 1 |
| `kPixelEarlyFragmentTests` | constant array in spirv2air_self_test | SPIR-V of the self-test's pixel shader with early fragment tests. | `src/xenia/gpu/metal/msl_shader.cc` | 712, 713 | 2 |
| `test` | namespace alias in RunSpirv2airSelfTest | spirv2air_self_test. | `src/xenia/gpu/metal/msl_shader.cc` | 670, 682, 683, 684, 686, 687, 688, 689, 690, 691, 692, 693, 695, 696, 698, 699, 700, 701, 702, 703, 704, 705, 707, 708, 709, 710, 712, 713 | 29 |
| `Module` | local struct in RunSpirv2airSelfTest | A shader: its name and SPIR-V. | `src/xenia/gpu/metal/msl_shader.cc` | 671, 677, 680, 804 | 4 |
| `module_of` | local lambda in RunSpirv2airSelfTest | A Module from a name and SPIR-V words. | `src/xenia/gpu/metal/msl_shader.cc` | 675, 681, 683, 685, 688, 690, 692, 694, 697, 700, 702, 704, 706, 709, 711 | 15 |
| `modules` | local in RunSpirv2airSelfTest | The self-test shaders, then the shaders from probe_dir. | `src/xenia/gpu/metal/msl_shader.cc` | 680, 778, 779, 782, 783, 804, 1035, 1039 | 8 |
| `Test` | local struct in RunSpirv2airSelfTest | A pipeline: its name, shader indices and variant. | `src/xenia/gpu/metal/msl_shader.cc` | 717, 728, 739, 908, 1007, 1019, 1020 | 7 |
| `kSharedMemory` | local constant in RunSpirv2airSelfTest | Spirv2airLayout::kDirectSharedMemory. | `src/xenia/gpu/metal/msl_shader.cc` | 723, 731, 743 | 3 |
| `kTextures` | local constant in RunSpirv2airSelfTest | Spirv2airLayout::kDirectTextures. | `src/xenia/gpu/metal/msl_shader.cc` | 724, 744, 746 | 3 |
| `kAttributesShift` | local constant in RunSpirv2airSelfTest | Spirv2airLayout::kDeclarationAttributesShift. | `src/xenia/gpu/metal/msl_shader.cc` | 725, 1014, 1021, 1037, 1041 | 5 |
| `baseline` | local in RunSpirv2airSelfTest | Pipelines from shaders with no calls. | `src/xenia/gpu/metal/msl_shader.cc` | 728, 988, 1007 | 3 |
| `with_calls` | local in RunSpirv2airSelfTest | Pipelines from shaders calling Metal intrinsics, built with each declaration attributes mode. | `src/xenia/gpu/metal/msl_shader.cc` | 739, 988, 1019, 1027, 1049 | 5 |
| `dumps` | local in RunSpirv2airSelfTest | The vertex and pixel shader files in probe_dir by pair_<id> prefix. | `src/xenia/gpu/metal/msl_shader.cc` | 750, 761, 774 | 3 |
| `file_name` | local in RunSpirv2airSelfTest | A probe_dir entry's file name. | `src/xenia/gpu/metal/msl_shader.cc` | 754, 755, 759, 761 | 5 |
| `read_words` | local lambda in RunSpirv2airSelfTest | A file's contents as SPIR-V words. | `src/xenia/gpu/metal/msl_shader.cc` | 764, 779, 783 | 3 |
| `bytes` | local in read_words | The file's bytes. | `src/xenia/gpu/metal/msl_shader.cc` | 766, 768, 769 | 3 |
| `words` | Module field; local in read_words | SPIR-V words. | `src/xenia/gpu/metal/msl_shader.cc` | 673, 675, 677, 768, 769, 770, 806 | 10 |
| `game_pairs` | local in RunSpirv2airSelfTest | Each dumped pair's vertex and pixel shader (-1: none) modules indices. | `src/xenia/gpu/metal/msl_shader.cc` | 773, 785, 989, 1034 | 4 |
| `pair_name` | local in RunSpirv2airSelfTest | A dumps key (unused name of the structured binding). | `src/xenia/gpu/metal/msl_shader.cc` | 774 | 1 |
| `paths` | local in RunSpirv2airSelfTest | A dumps value: the vertex and pixel shader files. | `src/xenia/gpu/metal/msl_shader.cc` | 774, 775, 779, 781, 783 | 7 |
| `vertex_index` | local in RunSpirv2airSelfTest | A game pair's vertex shader modules index. | `src/xenia/gpu/metal/msl_shader.cc` | 778, 785, 1034, 1035, 1036, 1040 | 6 |
| `pixel_index` | local in RunSpirv2airSelfTest | A game pair's pixel shader modules index, or -1. | `src/xenia/gpu/metal/msl_shader.cc` | 780, 782, 785, 1034, 1038, 1039, 1040 | 7 |
| `name` | Module and Test field; parameter of build | What a shader or pipeline is called in the log. | `src/xenia/gpu/metal/msl_shader.cc` | 672, 675, 677, 718, 812, 830, 852, 894, 901, 903, 916, 919, 1035, 1039 | 14 |
| `error_text` | local lambda in RunSpirv2airSelfTest | An NS::Error's description, or "unknown error". | `src/xenia/gpu/metal/msl_shader.cc` | 791, 830, 876, 882, 979 | 5 |
| `libraries` | local in RunSpirv2airSelfTest | The libraries loaded, released at the end. | `src/xenia/gpu/metal/msl_shader.cc` | 796, 823, 1055 | 3 |
| `functions` | local in RunSpirv2airSelfTest | Functions by module index and variant; null where one didn't load. | `src/xenia/gpu/metal/msl_shader.cc` | 798, 800, 1050 | 3 |
| `function_of` | local lambda in RunSpirv2airSelfTest | A module's function for a variant, translated and loaded on first use. | `src/xenia/gpu/metal/msl_shader.cc` | 799, 909, 911 | 3 |
| `index` | parameter of function_of | The module. | `src/xenia/gpu/metal/msl_shader.cc` | 799, 800, 804 | 3 |
| `inserted` | local in function_of | Whether the function is new to functions. | `src/xenia/gpu/metal/msl_shader.cc` | 800, 801 | 2 |
| `kAnswerTimeout` | local constant in RunSpirv2airSelfTest | How long a pipeline gets (40 s) before the self-test stops. | `src/xenia/gpu/metal/msl_shader.cc` | 839, 889 | 2 |
| `kTimeBudget` | local constant in RunSpirv2airSelfTest | How long the self-test may run (10 minutes). | `src/xenia/gpu/metal/msl_shader.cc` | 840, 858 | 2 |
| `self_test_begin` | local in RunSpirv2airSelfTest | When the self-test started. | `src/xenia/gpu/metal/msl_shader.cc` | 841, 858 | 2 |
| `stopped` | local in RunSpirv2airSelfTest | The self-test stopped early (time spent, or Metal didn't answer). | `src/xenia/gpu/metal/msl_shader.cc` | 842, 855, 860, 895, 995, 1048 | 6 |
| `Answer` | local struct in RunSpirv2airSelfTest | Metal's answer to a pipeline request, shared with its completion handler. | `src/xenia/gpu/metal/msl_shader.cc` | 843, 873 | 2 |
| `answered` | Answer field | The completion handler ran. | `src/xenia/gpu/metal/msl_shader.cc` | 846, 879, 890 | 3 |
| `built` | Answer field; local in RunSpirv2airSelfTest and the control lambda | The pipeline built (count of pipelines that built). | `src/xenia/gpu/metal/msl_shader.cc` | 847, 880, 899, 906, 958, 965, 982, 990, 993, 1048 | 10 |
| `build` | local lambda in RunSpirv2airSelfTest | Builds a pipeline with the time limit and logs a failure (and a success if asked). | `src/xenia/gpu/metal/msl_shader.cc` | 852, 919, 965 | 3 |
| `log_success` | parameter of build | Whether to log a pipeline that builds. | `src/xenia/gpu/metal/msl_shader.cc` | 854, 902 | 2 |
| `answer` | local in build | This request's Answer. | `src/xenia/gpu/metal/msl_shader.cc` | 873, 876, 878, 879, 880, 882, 884, 888, 889, 890, 899, 901, 906 | 13 |
| `begin` | local in build and count | When the build started. | `src/xenia/gpu/metal/msl_shader.cc` | 874, 898, 996, 1001 | 4 |
| `pipeline_ms` | local in build | Time Metal took to build or fail the pipeline. | `src/xenia/gpu/metal/msl_shader.cc` | 898, 901, 904 | 3 |
| `run` | local lambda in RunSpirv2airSelfTest | Loads a Test's functions and builds its pipeline. | `src/xenia/gpu/metal/msl_shader.cc` | 908, 1008, 1022, 1036, 1039 | 5 |
| `t` | parameter of run; loop variable | The pipeline to build. | `src/xenia/gpu/metal/msl_shader.cc` | 908, 909, 911, 912, 916, 919, 1007, 1008, 1019, 1020 | 15 |
| `controls` | local in RunSpirv2airSelfTest | Control pipelines built so far. | `src/xenia/gpu/metal/msl_shader.cc` | 924, 951, 966 | 3 |
| `control_salt` | local in RunSpirv2airSelfTest | Makes this run's control pipelines new to Metal's caches. | `src/xenia/gpu/metal/msl_shader.cc` | 925, 951 | 2 |
| `control` | local lambda in RunSpirv2airSelfTest | Compiles and builds a new Metal Shading Language control pipeline. | `src/xenia/gpu/metal/msl_shader.cc` | 927, 997, 1006 | 3 |
| `kMslSource` | local static string in the control lambda | The control's Metal Shading Language source. | `src/xenia/gpu/metal/msl_shader.cc` | 928, 951 | 2 |
| `source` | local in the control lambda | kMslSource with this control's constant. | `src/xenia/gpu/metal/msl_shader.cc` | 950, 955 | 2 |
| `failed` | local in RunSpirv2airSelfTest | Pipelines Metal failed to build. | `src/xenia/gpu/metal/msl_shader.cc` | 991, 993, 1048 | 3 |
| `count` | local lambda in RunSpirv2airSelfTest | Counts a result, and after a failure checks the compiler with a new control. | `src/xenia/gpu/metal/msl_shader.cc` | 789, 926, 992, 1006, 1008, 1022, 1036, 1039 | 8 |
| `best_mode` | local in RunSpirv2airSelfTest | The declaration attributes mode that built the most pipelines with calls. | `src/xenia/gpu/metal/msl_shader.cc` | 1015, 1029, 1037, 1041, 1049 | 5 |
| `best_built` | local in RunSpirv2airSelfTest | How many it built. | `src/xenia/gpu/metal/msl_shader.cc` | 1016, 1028, 1030, 1033, 1049 | 5 |
| `mode` | local in RunSpirv2airSelfTest | A declaration attributes mode. | `src/xenia/gpu/metal/msl_shader.cc` | 1017, 1021, 1027, 1029 | 4 |
| `mode_built` | local in RunSpirv2airSelfTest | Pipelines with calls it built. | `src/xenia/gpu/metal/msl_shader.cc` | 1018, 1022, 1027, 1028, 1030 | 6 |
| `variant_test` | local in RunSpirv2airSelfTest | A with_calls Test with the mode in its variant. | `src/xenia/gpu/metal/msl_shader.cc` | 1020, 1021, 1022 | 3 |
| `game_variant` | local in RunSpirv2airSelfTest | The variant guest shaders are compiled with, without the declaration attributes mode: the cases with calls and the game pairs are built with it and each mode. | `src/xenia/gpu/metal/msl_shader.cc` | 1013, 1021, 1037, 1041 | 4 |
| `vertex_name` | local in RunSpirv2airSelfTest | A game pair's vertex shader name. | `src/xenia/gpu/metal/msl_shader.cc` | 1035, 1036, 1039 | 3 |
| `key` | local in RunSpirv2airSelfTest | A functions key (unused name of the structured binding). | `src/xenia/gpu/metal/msl_shader.cc` | 1050 | 1 |
| `module` | local in function_of | The module being translated. | `src/xenia/gpu/metal/msl_shader.cc` | 804, 806, 812, 830 | 5 |
| `entry` | local in RunSpirv2airSelfTest | A probe_dir directory entry. | `src/xenia/gpu/metal/msl_shader.cc` | 752, 754, 756, 761 | 4 |
| `probe_dir` | parameter of RunSpirv2airSelfTest | The title's spirv2air_probe folder. | `src/xenia/gpu/metal/msl_shader.h` | 102 | 1 |
| `probe_dir` | parameter of RunSpirv2airSelfTest | The title's spirv2air_probe folder. | `src/xenia/gpu/metal/msl_shader.cc` | 669, 752 | 2 |
| `vertex` | Test field; parameter of DumpSpirv2airPipelineShaders | The pipeline's vertex shader. | `src/xenia/gpu/metal/msl_shader.h` | 106 | 1 |
| `vertex` | Test field; parameter of DumpSpirv2airPipelineShaders | The pipeline's vertex shader. | `src/xenia/gpu/metal/msl_shader.cc` | 719, 909, 1060, 1073, 1088 | 6 |
| `pixel` | Test field; parameter of DumpSpirv2airPipelineShaders | The pipeline's pixel shader (index -1, or null, for none). | `src/xenia/gpu/metal/msl_shader.h` | 107 | 1 |
| `pixel` | Test field; parameter of DumpSpirv2airPipelineShaders | The pipeline's pixel shader (index -1, or null, for none). | `src/xenia/gpu/metal/msl_shader.cc` | 720, 911, 912, 1061, 1074, 1075, 1088 | 10 |
| `spirv` | local in DumpSpirv2airPipelineShaders | The translation's SPIR-V. | `src/xenia/gpu/metal/msl_shader.cc` | 1097, 1099, 1100 | 3 |
| `pair` | local in DumpSpirv2airPipelineShaders | The pair's shader hashes and modifications. | `src/xenia/gpu/metal/msl_shader.cc` | 1072, 1076, 1081 | 4 |
| `pair_id` | local in DumpSpirv2airPipelineShaders | Hash of the pair, in the file names. | `src/xenia/gpu/metal/msl_shader.cc` | 1076, 1093, 1105 | 3 |
| `direct_textures` | local in bind_spirv2air_stage | The shader was compiled with Spirv2airLayout::kDirectTextures. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5992, 6012, 6056, 6083, 6108, 6125 | 6 |
| `sampler_index` | local in bind_spirv2air_stage | The sampler's index with direct bindings. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6109, 6112, 6115, 6117 | 4 |
| `spirv2air_msl_fallbacks` | function-static atomic in IssueDrawMsl | Shaders compiled with the Metal Shading Language as well for a draw that can't use spirv2air; the first 16 are logged. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5061, 5062 | 2 |
| `bind_spirv2air_stage` | local lambda in IssueDrawMsl | Writes and binds a spirv2air stage's top-level argument buffer and heap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5989, 6169, 6171 | 3 |
| `record_count` | local in bind_spirv2air_stage | Textures plus samplers of the stage. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5997, 5999, 6000, 6014, 6057, 6101, 6132, 6141 | 8 |
| `kSliceAlignment` | local constant in bind_spirv2air_stage | Alignment of the slice and the heap in it (16 on iOS, 256 on macOS). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6005, 6007, 6008, 6016 | 4 |
| `kHeapOffset` | local constant in bind_spirv2air_stage | Offset of the heap in the slice. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6006, 6012, 6055, 6149, 6152, 6158, 6161 | 7 |
| `slice_buffer` | local in bind_spirv2air_stage | Buffer of the argument buffer slice. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6009, 6016, 6021, 6147, 6149, 6152, 6156, 6158, 6161, 6164 | 10 |
| `slice_offset` | local in bind_spirv2air_stage | Offset of the slice in it. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6010, 6016, 6021, 6147, 6149, 6152, 6156, 6158, 6161 | 9 |
| `slice` | local in bind_spirv2air_stage | CPU pointer to the slice. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6020, 6053, 6055 | 3 |
| `argument_buffer` | local array in bind_spirv2air_stage | The top-level argument buffer as written. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6023, 6034, 6035, 6036, 6037, 6038, 6039, 6040, 6044, 6046, 6049, 6051, 6053 | 14 |
| `uniforms_address` | local in bind_spirv2air_stage | GPU address of the uniforms buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6031, 6033, 6035, 6036 | 4 |
| `stage_uniforms` | local in bind_spirv2air_stage | GPU address of the stage's constant table. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6032, 6034, 6037, 6038, 6039, 6040 | 6 |
| `shared_memory_address` | local in bind_spirv2air_stage | GPU address of shared memory. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6042, 6045, 6047 | 3 |
| `indices_address` | local in bind_spirv2air_stage | GPU address of the index records. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6048, 6050, 6052 | 3 |
| `heap` | local in bind_spirv2air_stage | CPU pointer to the heap in the slice. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6055, 6057, 6096, 6121 | 4 |
| `handle` | local in bind_spirv2air_stage | A texture's or sampler's resource ID. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6095, 6098, 6120, 6121, 6122 | 6 |
| `spirv2air` | parameter of MetalCommandProcessor::GetOrCreateMslPipelineState | The pipeline takes the shaders' spirv2air functions. | `src/xenia/gpu/metal/metal_command_processor.cc` | 11777, 11782, 11786, 11879 | 4 |
| `spirv2air` | member of MslPipelineKey (local struct in GetOrCreateMslPipelineState) | The pipeline takes spirv2air functions, so it is a different pipeline. | `src/xenia/gpu/metal/metal_command_processor.cc` | 11875, 11879 | 2 |
| `spirv2air` | parameter of MetalCommandProcessor::GetOrCreateMslPipelineState | The pipeline takes the shaders' spirv2air functions. | `src/xenia/gpu/metal/metal_command_processor.h` | 438 | 1 |
| `spirv2air` | premake project name | Builds third_party/spirv2air. | `premake5.lua` | 827 | 1 |
| `spirv2air` | premake project name | Builds third_party/spirv2air. | `src/xenia/gpu/metal/premake5.lua` | 36, 40 | 3 |
| `spirv2air` | premake project name | Builds third_party/spirv2air. | `src/xenia/app/premake5.lua` | 242 | 1 |
| `spirv2air` | premake project name | Builds third_party/spirv2air. | `third_party/spirv2air.lua` | 1, 5, 11, 12, 13, 18, 21, 22, 23, 24, 25, 26 | 16 |
| `msl_compile_frame_arrival_` | MetalCommandProcessor member | msl_compile_arrivals_ at the start of the frame: a pending shader whose latest entry is older is queued again. | `src/xenia/gpu/metal/metal_command_processor.h` | 652 | 1 |
| `msl_compile_frame_arrival_` | MetalCommandProcessor member | msl_compile_arrivals_ at the start of the frame: a pending shader whose latest entry is older is queued again. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1050, 3152 | 2 |
| `pending_it` | local in EnqueueMslShaderCompilation and MslShaderCompileThread | The shader's msl_shader_compile_pending_ entry (latest queue_arrival, UINT64_MAX while compiling). | `src/xenia/gpu/metal/metal_command_processor.cc` | 1048, 1049, 1050, 1062, 1065, 1255, 1257, 1258, 1262 | 9 |
| `stale_end` | local in RenderTargetCache::Update | End of the tiles whose other copies a retargeted depth clear makes stale. | `src/xenia/gpu/render_target_cache.cc` | 890, 895 | 2 |
| `has_stencil` | local in MetalRenderTargetCache::PerformDepthClearRetarget | The owner's host format has stencil. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 9463, 9471, 9539 | 3 |
| `depth_clear_retargets` | DrawStats field | Depth clears through a 4x view written into the owner of the tiles instead of drawn (depth_clear_retarget_4x). | `src/xenia/gpu/metal/metal_command_processor.h` | 676 | 1 |
| `depth_clear_retargets` | DrawStats field | Depth clears through a 4x view written into the owner of the tiles instead of drawn (depth_clear_retarget_4x). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3008, 3012, 3547 | 3 |
| `ios_tap_to_open_menu` | cvar (UI); iOS settings key | Tapping the game away from the touch controls opens the in-game menu (default on). | `src/xenia/ui/windowed_app_main_ios.mm` | 75, 11247 | 2 |
| `ios_signin_prompt` | cvar (UI); iOS settings key | Show the Xbox sign-in prompt; off answers it with no one signed in (default on). | `src/xenia/ui/windowed_app_main_ios.mm` | 80, 13193 | 2 |
| `ios_keyboard_prompt` | cvar (UI); iOS settings key | Show the keyboard prompt; off answers it as cancelled (default on). | `src/xenia/ui/windowed_app_main_ios.mm` | 84, 13216 | 2 |
| `kChooseAchievementSound` | IOSConfigAction enumerator | Settings action: pick the achievement sound file. | `src/xenia/ui/windowed_app_main_ios.mm` | 580, 3773, 8675 | 3 |
| `kClearAchievementSound` | IOSConfigAction enumerator | Settings action: remove the achievement sound. | `src/xenia/ui/windowed_app_main_ios.mm` | 581, 3777, 8685 | 3 |
| `interface` | local in BuildIOSConfigSections | The Interface settings section. | `src/xenia/ui/windowed_app_main_ios.mm` | 3754, 3755, 3756, 3761, 3765, 3769, 3773, 3777, 3779 | 9 |
| `xe_play_achievement_sound` | file-scope function | Plays achievement_sound_path once unless muted. | `src/xenia/ui/windowed_app_main_ios.mm` | 1593, 8655, 11209 | 3 |
| `player` | static local in xe_play_achievement_sound | The AVAudioPlayer kept for the sound. | `src/xenia/ui/windowed_app_main_ios.mm` | 1594, 1599, 1600, 1601, 1602, 1604, 1605 | 8 |
| `destination` | local in the settings documentPicker | Where the picked sound is copied. | `src/xenia/ui/windowed_app_main_ios.mm` | 8637, 8641, 8644, 8653 | 4 |
| `extension` | local in the settings documentPicker | The picked file's extension. | `src/xenia/ui/windowed_app_main_ios.mm` | 8636, 8639 | 2 |
| `XeniaTouchPadControlKindMenu` | enumerator | The menu control: opens the in-game menu, never reaches the game. | `src/xenia/ui/windowed_app_main_ios.mm` | 8904, 9080, 9097, 9212, 9384, 9412 | 6 |
| `XeniaTouchPadPlacement` | struct | A control's place in a saved layout: position, scale, hidden. | `src/xenia/ui/windowed_app_main_ios.mm` | 8920, 8957, 9084, 9268, 9283, 9305 | 6 |
| `placements_` | instance variable of XeniaTouchPadView | Each control's placement in the loaded layout. | `src/xenia/ui/windowed_app_main_ios.mm` | 8957, 9084, 9096, 9233, 9268, 9283, 9305, 9367, 9374, 9380, 9387, 9412 | 13 |
| `placement` | local in XeniaTouchPadView methods | One control's placement. | `src/xenia/ui/windowed_app_main_ios.mm` | 9084, 9085, 9087, 9089, 9283, 9287, 9292, 9294, 9305, 9307, 9308, 9309, 9311, 9312, 9314 | 18 |
| `scale` | member of XeniaTouchPadPlacement; locals and parameter in XeniaTouchPadView | Size against the default (0.5-2). | `src/xenia/ui/windowed_app_main_ios.mm` | 8922, 9089, 9290, 9291, 9292, 9311, 9312, 9367, 9370, 9374 | 12 |
| `hidden` | member of XeniaTouchPadPlacement; local in drawRect | Whether the control is hidden. | `src/xenia/ui/windowed_app_main_ios.mm` | 8923, 9096, 9294, 9314, 9380, 9387, 9412, 9441, 9445, 9447 | 12 |
| `edit_touch_` | instance variable of XeniaTouchPadView | The touch dragging a control in edit mode. | `src/xenia/ui/windowed_app_main_ios.mm` | 8962, 9129, 9189, 9197, 9226, 9227, 9245, 9246 | 10 |
| `edit_offset_` | instance variable of XeniaTouchPadView | Its offset from the control's center. | `src/xenia/ui/windowed_app_main_ios.mm` | 8963, 9198, 9229 | 4 |
| `editing` | property of XeniaTouchPadView, and setEditing: parameter | Edit mode: touches move controls instead of reaching the game. | `src/xenia/ui/windowed_app_main_ios.mm` | 8928, 9096, 9120, 9123, 9124, 9128, 9188, 9225, 9244, 9402, 9443, 10425, 10426, 10435, 10444, 11247, 11287, 11408, 11447 | 19 |
| `menuHandler` | property of XeniaTouchPadView | Called when the menu control is pressed. | `src/xenia/ui/windowed_app_main_ios.mm` | 8930, 9213, 9214, 9608 | 4 |
| `selectionHandler` | property of XeniaTouchPadView | Called when the selected control changes in edit mode. | `src/xenia/ui/windowed_app_main_ios.mm` | 8932, 9131, 9132, 9201, 9202, 9357, 9358, 9611 | 8 |
| `loadLayoutForTitleID` | method of XeniaTouchPadView | Loads a title's layout, or the one for all games. | `src/xenia/ui/windowed_app_main_ios.mm` | 8939, 9324, 9614, 10428, 12489 | 5 |
| `layoutTitleID` | property of XeniaTouchPadView | The title whose layout is loaded. | `src/xenia/ui/windowed_app_main_ios.mm` | 8941, 11406 | 2 |
| `layoutIsPerGame` | property of XeniaTouchPadView | Whether that title has a layout of its own. | `src/xenia/ui/windowed_app_main_ios.mm` | 8942, 11407 | 2 |
| `saveLayoutPerGame` | method of XeniaTouchPadView | Saves the layout for the title alone or for all games. | `src/xenia/ui/windowed_app_main_ios.mm` | 8945, 9340, 11446 | 3 |
| `resetLayout` | method of XeniaTouchPadView | Back to the default layout. | `src/xenia/ui/windowed_app_main_ios.mm` | 8946, 9355, 11432 | 3 |
| `selectedControl` | property of XeniaTouchPadView | The control selected in edit mode, or -1. | `src/xenia/ui/windowed_app_main_ios.mm` | 8947 | 1 |
| `selectedControlLabel` | property of XeniaTouchPadView | Its label. | `src/xenia/ui/windowed_app_main_ios.mm` | 8948, 9362, 11390 | 3 |
| `selectedScale` | property of XeniaTouchPadView | Its size. | `src/xenia/ui/windowed_app_main_ios.mm` | 8949, 9366, 11415, 11419 | 6 |
| `toggleSelectedHidden` | method of XeniaTouchPadView | Hides or shows it (not the menu control). | `src/xenia/ui/windowed_app_main_ios.mm` | 8950, 9383, 11423 | 3 |
| `selectedHidden` | property of XeniaTouchPadView | Whether it is hidden. | `src/xenia/ui/windowed_app_main_ios.mm` | 8951, 9379, 11392 | 3 |
| `layoutOpacity` | property of XeniaTouchPadView | Opacity of the whole pad (0.1-1). | `src/xenia/ui/windowed_app_main_ios.mm` | 8952, 11396, 11428 | 3 |
| `XeniaTouchLayoutKey` | function in XeniaTouchPadView | NSUserDefaults key of a layout. | `src/xenia/ui/windowed_app_main_ios.mm` | 8991, 9332, 9335, 9344, 9347, 9349 | 6 |
| `controlIsActive` | method of XeniaTouchPadView | Whether a control takes touches and is drawn. | `src/xenia/ui/windowed_app_main_ios.mm` | 9094, 9103, 9408 | 3 |
| `applyLayoutDictionary` | method of XeniaTouchPadView | Applies a saved layout. | `src/xenia/ui/windowed_app_main_ios.mm` | 9266, 9337, 9356 | 3 |
| `layoutDictionary` | method of XeniaTouchPadView | The layout as saved. | `src/xenia/ui/windowed_app_main_ios.mm` | 9302, 9342 | 2 |
| `opacity` | locals in XeniaTouchPadView methods | A layout's opacity. | `src/xenia/ui/windowed_app_main_ios.mm` | 9272, 9273, 9274, 9391, 9392, 9397, 9400, 9441, 9445, 9460 | 10 |
| `defaults` | local in XeniaTouchPadView methods | NSUserDefaults. | `src/xenia/ui/windowed_app_main_ios.mm` | 9331, 9332, 9335, 9341, 9344, 9347, 9349 | 7 |
| `layout` | local in XeniaTouchPadView methods | A saved layout dictionary. | `src/xenia/ui/windowed_app_main_ios.mm` | 9266, 9271, 9272, 9276, 9332, 9333, 9334, 9335, 9337, 9342, 9344, 9347 | 12 |
| `per_game` | parameter of saveLayoutPerGame: | For the title alone. | `src/xenia/ui/windowed_app_main_ios.mm` | 8945, 9340, 9343 | 3 |
| `center` | local in touchesMoved | A dragged control's new center. | `src/xenia/ui/windowed_app_main_ios.mm` | 8911, 9086, 9106, 9107, 9147, 9148, 9199, 9229, 9230, 9231, 9232, 9234, 9235, 9414, 9423, 9424, 9430, 9435 | 23 |
| `selected` | local in drawRect | Whether the control is selected in edit mode. | `src/xenia/ui/windowed_app_main_ios.mm` | 9443, 9444, 9446 | 3 |
| `dashes` | local in drawRect | Dash pattern of a hidden control. | `src/xenia/ui/windowed_app_main_ios.mm` | 9448, 9449 | 2 |
| `system_ui_reported_` | instance variable of XeniaViewController | Whether the app's UI over the game was last reported as system UI. | `src/xenia/ui/windowed_app_main_ios.mm` | 9563, 10436, 10437 | 3 |
| `system_ui` | local in pollControllerNavigation | Whether the menu, a sheet or the editor covers the game. | `src/xenia/ui/windowed_app_main_ios.mm` | 10433, 10436, 10437, 10438 | 4 |
| `controller_connected` | local in pollControllerNavigation | Whether a GCController is connected. | `src/xenia/ui/windowed_app_main_ios.mm` | 10422, 10446, 10512 | 3 |
| `achievement_queue_` | instance variable of XeniaViewController | Achievement pop-ups waiting. | `src/xenia/ui/windowed_app_main_ios.mm` | 9566, 11132, 11133, 11135, 11142, 11147, 11148 | 7 |
| `achievement_showing_` | instance variable of XeniaViewController | Whether one is on screen. | `src/xenia/ui/windowed_app_main_ios.mm` | 9567, 11136, 11143, 11146 | 4 |
| `showAchievementNamed` | method of XeniaViewController | Queues an achievement pop-up. | `src/xenia/ui/windowed_app_main_ios.mm` | 9523, 11131, 13184 | 3 |
| `showNextAchievement` | method of XeniaViewController | Shows the next pop-up and plays the sound. | `src/xenia/ui/windowed_app_main_ios.mm` | 11137, 11141, 11225 | 3 |
| `banner` | local in showNextAchievement | The pop-up view. | `src/xenia/ui/windowed_app_main_ios.mm` | 11150, 11153, 11154, 11155, 11156, 11180, 11181, 11182, 11183, 11185, 11189, 11190, 11191, 11192, 11194, 11196, 11200, 11202, 11206, 11224 | 20 |
| `icon_image` | local in showNextAchievement | The achievement's icon. | `src/xenia/ui/windowed_app_main_ios.mm` | 11158, 11160 | 2 |
| `icon_view` | local in showNextAchievement | Its image view. | `src/xenia/ui/windowed_app_main_ios.mm` | 11159, 11161, 11162, 11163, 11164, 11165, 11180, 11194, 11196, 11197, 11198, 11199 | 12 |
| `heading` | local in showNextAchievement | "Achievement unlocked" label. | `src/xenia/ui/windowed_app_main_ios.mm` | 11167, 11168, 11169, 11170, 11171, 11181, 11199, 11200, 11202, 11203, 11204, 11205 | 12 |
| `top` | local in showNextAchievement | The banner's top constraint, animated. | `src/xenia/ui/windowed_app_main_ios.mm` | 11184, 11188, 11210, 11218 | 4 |
| `name_string` | local in the achievement callback | The name for the main thread. | `src/xenia/ui/windowed_app_main_ios.mm` | 13178, 13180, 13184, 13185 | 4 |
| `inGameEditControlsButton` | property of XeniaViewController | The menu's Edit Touch Controls button. | `src/xenia/ui/windowed_app_main_ios.mm` | 9495, 11047, 11049, 11050, 11053, 11110, 11113, 11115, 11119 | 9 |
| `edit_controls_config` | local in setupInGameMenuOverlay | Its configuration. | `src/xenia/ui/windowed_app_main_ios.mm` | 11039, 11040, 11041, 11042, 11043, 11044, 11045, 11046, 11047 | 9 |
| `openInGameMenu` | method of XeniaViewController | Opens the in-game menu (the menu control). | `src/xenia/ui/windowed_app_main_ios.mm` | 9524, 9609, 11285 | 3 |
| `inGameEditControlsTapped` | method of XeniaViewController | Starts the touch layout editor. | `src/xenia/ui/windowed_app_main_ios.mm` | 9525, 11051, 11399 | 3 |
| `weak_self` | local in viewDidLoad | Self, unretained by the pad's handlers. | `src/xenia/ui/windowed_app_main_ios.mm` | 9607, 9609, 9612 | 3 |
| `layout_title_id` | local in launchGameAtPath: | The launched game's title ID, for its touch layout. | `src/xenia/ui/windowed_app_main_ios.mm` | 12482, 12485, 12489 | 3 |
| `touch_editor_bar_` | instance variable of XeniaViewController | Touch layout editor: the editor bar. | `src/xenia/ui/windowed_app_main_ios.mm` | 9569, 10427, 11315, 11317, 11318, 11319, 11361, 11362, 11365, 11367, 11372, 11373, 11375, 11377, 11379, 11380, 11386, 11401, 11405, 11410, 11448 | 21 |
| `touch_editor_label_` | instance variable of XeniaViewController | Touch layout editor: the selected control's name. | `src/xenia/ui/windowed_app_main_ios.mm` | 9570, 11321, 11322, 11323, 11324, 11346, 11391 | 7 |
| `touch_editor_hide_button_` | instance variable of XeniaViewController | Touch layout editor: Hide/Show. | `src/xenia/ui/windowed_app_main_ios.mm` | 9571, 11327, 11349, 11392, 11395 | 5 |
| `touch_editor_opacity_slider_` | instance variable of XeniaViewController | Touch layout editor: Opacity. | `src/xenia/ui/windowed_app_main_ios.mm` | 9572, 11334, 11335, 11336, 11337, 11340, 11351, 11396 | 8 |
| `touch_editor_scope_` | instance variable of XeniaViewController | Touch layout editor: All Games / This Game. | `src/xenia/ui/windowed_app_main_ios.mm` | 9573, 11342, 11343, 11352, 11406, 11407, 11446 | 7 |
| `touch_editor_top_` | instance variable of XeniaViewController | Touch layout editor: the bar at the top. | `src/xenia/ui/windowed_app_main_ios.mm` | 9574, 11364, 11371, 11436, 11437 | 5 |
| `touch_editor_bottom_` | instance variable of XeniaViewController | Touch layout editor: the bar at the bottom. | `src/xenia/ui/windowed_app_main_ios.mm` | 9575, 11367, 11438 | 3 |
| `XeniaTouchEditorButton` | file-scope function of XeniaViewController | Touch layout editor: a button of the editor bar. | `src/xenia/ui/windowed_app_main_ios.mm` | 11304, 11328, 11347, 11348, 11353, 11354, 11355 | 7 |
| `buildTouchEditorBar` | method of XeniaViewController | Touch layout editor: builds the bar. | `src/xenia/ui/windowed_app_main_ios.mm` | 11314, 11402 | 2 |
| `updateTouchEditorBar` | method of XeniaViewController | Touch layout editor: shows the selection. | `src/xenia/ui/windowed_app_main_ios.mm` | 9612, 11385, 11411, 11424 | 4 |
| `touchEditorSmallerTapped` | action of XeniaViewController | Touch layout editor: smaller. | `src/xenia/ui/windowed_app_main_ios.mm` | 11347, 11414 | 2 |
| `touchEditorLargerTapped` | action of XeniaViewController | Touch layout editor: larger. | `src/xenia/ui/windowed_app_main_ios.mm` | 11348, 11418 | 2 |
| `touchEditorHideTapped` | action of XeniaViewController | Touch layout editor: hide or show. | `src/xenia/ui/windowed_app_main_ios.mm` | 11328, 11422 | 2 |
| `touchEditorOpacityChanged` | action of XeniaViewController | Touch layout editor: opacity. | `src/xenia/ui/windowed_app_main_ios.mm` | 11338, 11427 | 2 |
| `touchEditorResetTapped` | action of XeniaViewController | Touch layout editor: default layout. | `src/xenia/ui/windowed_app_main_ios.mm` | 11353, 11431 | 2 |
| `touchEditorMoveBarTapped` | action of XeniaViewController | Touch layout editor: bar top or bottom. | `src/xenia/ui/windowed_app_main_ios.mm` | 11354, 11435 | 2 |
| `touchEditorDoneTapped` | action of XeniaViewController | Touch layout editor: save and leave. | `src/xenia/ui/windowed_app_main_ios.mm` | 11355, 11445 | 2 |
| `opacity_label` | local in buildTouchEditorBar | "Opacity" label. | `src/xenia/ui/windowed_app_main_ios.mm` | 11330, 11331, 11332, 11333, 11350 | 5 |
| `row` | local in buildTouchEditorBar | The bar's stack view. | `src/xenia/ui/windowed_app_main_ios.mm` | 11345, 11357, 11358, 11359, 11360, 11361, 11375, 11377, 11379, 11380 | 10 |
| `pad` | local in updateTouchEditorBar | The touch pad. | `src/xenia/ui/windowed_app_main_ios.mm` | 11389, 11390, 11392, 11396 | 4 |
| `to_bottom` | local in touchEditorMoveBarTapped: | Whether the bar moves to the bottom. | `src/xenia/ui/windowed_app_main_ios.mm` | 11436, 11437, 11438 | 3 |
| `SystemUICallback` | type in IOSWindowedAppContext | Callback: the app's UI covers the game or not. | `src/xenia/ui/windowed_app_context_ios.h` | 204, 205, 255 | 3 |
| `set_system_ui_callback` | method of IOSWindowedAppContext | Sets the callback. | `src/xenia/ui/windowed_app_context_ios.h` | 205 | 1 |
| `system_ui_callback_` | member of IOSWindowedAppContext | The callback. | `src/xenia/ui/windowed_app_context_ios.h` | 206, 209, 210, 255 | 4 |
| `AchievementCallback` | type in IOSWindowedAppContext | Callback: an achievement unlocked (name, gamerscore, icon). | `src/xenia/ui/windowed_app_context_ios.h` | 216, 218, 256 | 3 |
| `set_achievement_callback` | method of IOSWindowedAppContext | Sets the callback. | `src/xenia/ui/windowed_app_context_ios.h` | 218 | 1 |
| `achievement_callback_` | member of IOSWindowedAppContext | The callback. | `src/xenia/ui/windowed_app_context_ios.h` | 219, 223, 224, 256 | 4 |
| `SetSystemUIActive` | method of IOSWindowedAppContext | Reports the app's UI over the game to XAM. | `src/xenia/ui/windowed_app_context_ios.h` | 208 | 1 |
| `SetSystemUIActive` | method of IOSWindowedAppContext | Reports the app's UI over the game to XAM. | `src/xenia/ui/windowed_app_main_ios.mm` | 10438 | 1 |
| `set_system_ui_callback` | method of IOSWindowedAppContext | Setter. | `src/xenia/app/xenia_main_ios.mm` | 527 | 1 |
| `counted` | capture of the system UI callback | The kernel state the dialog count was taken on. | `src/xenia/app/xenia_main_ios.mm` | 528, 531, 532, 534, 539, 542 | 7 |
| `ShowAchievement` | method of IOSWindowedAppContext | Shows the achievement pop-up. | `src/xenia/ui/windowed_app_context_ios.h` | 221 | 1 |
| `ShowAchievement` | method of IOSWindowedAppContext | Shows the achievement pop-up. | `src/xenia/kernel/xam/achievement_manager.cc` | 172 | 1 |
| `set_achievement_callback` | method of IOSWindowedAppContext | Setter. | `src/xenia/ui/windowed_app_main_ios.mm` | 13175 | 1 |
| `OnUIState` | method of UIInputFilter | Runs OnUIClosed on the first input call after XAM UI closes. | `src/xenia/kernel/xam/ui_input_filter.h` | 42 | 1 |
| `OnUIState` | method of UIInputFilter | Runs OnUIClosed on the first input call after XAM UI closes. | `src/xenia/kernel/xam/ui_input_filter.cc` | 47 | 1 |
| `OnUIState` | method of UIInputFilter | Runs OnUIClosed on the first input call after XAM UI closes. | `src/xenia/kernel/xam/xam_input.cc` | 112, 187, 217 | 3 |
| `ui_was_active_` | member of UIInputFilter | Whether UI was active at the last input call. | `src/xenia/kernel/xam/ui_input_filter.h` | 53 | 1 |
| `ui_was_active_` | member of UIInputFilter | Whether UI was active at the last input call. | `src/xenia/kernel/xam/ui_input_filter.cc` | 49, 50 | 2 |
| `ui_active` | parameter of OnUIState | Whether XAM UI is active now. | `src/xenia/kernel/xam/ui_input_filter.h` | 42 | 1 |
| `ui_active` | parameter of OnUIState | Whether XAM UI is active now. | `src/xenia/kernel/xam/ui_input_filter.cc` | 47, 48, 54 | 3 |

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
| `draw_stats_` | 59 |
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
| `kXeniaTouchPadControlCount` | 10 |
| `controls_` | 28 |
| `pad_touches_` | 12 |
| `pad_touch_controls_` | 8 |
| `controlIndexAtPoint` | 8 |
| `resetTouches` | 4 |
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
| `xma_thread_state` | 6 |
| `xma_context` | 5 |
| `xma_memory` | 4 |
| `xma_dump` | 4 |
| `xma_start` | 3 |
| `xma_words` | 3 |
| `xma_line` | 6 |
| `xma_text` | 3 |
| `xma_word` | 5 |
| `xma_frame` | 14 |
| `xma_depth` | 6 |
| `xma_caller_frame` | 7 |
| `xma_return` | 6 |
| `xma_repeat_key` | 2 |
| `xma_repeat_count` | 6 |
| `xma_key` | 3 |
| `xma_thread_state` (xma_decoder.cc) | 3 |
| `xma_frames` | 5 |
| `xma_frame_count` | 2 |
| `xma_frame` | 14 |
| `xma_symbol` | 10 |
| `kEmitInlineMmioChecksDefault` | 3 |
| `kernel_call_repeat_threshold` | 5 |
| `NoteKernelCall` | 4 |
| `kernel_call_history` | 7 |
| `kernel_call_runs` | 4 |
| `kernel_call_period` | 12 |
| `kernel_call_index` | 12 |
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
| `kernel_call_state` | 14 |
| `kernel_call_last` | 5 |
| `kernel_call_inside` | 5 |
| `kernel_call_owner` | 3 |
| `kernel_call_only_sleeps` | 3 |
| `kernel_call_code` | 7 |
| `kernel_call_line` | 12 |
| `kernel_call_words` | 6 |
| `kernel_call_word` | 8 |
| `LogKernelCallStack` | 7 |
| `guest_stack_pointer` | 4 |
| `guest_arg0` | 3 |
| `guest_arg1` | 3 |
| `kernel_call_ms` | 3 |
| `kernel_call_handle` | 3 |
| `kernel_call_object` | 11 |
| `kernel_call_xobject` | 5 |
| `kernel_call_total` | 3 |
| `kernel_call_history_lr` | 3 |
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
| `kernel_call_context` | 8 |
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
| `unhandled_count` | 6 |
| `PreferPerformanceCores` | 4 |
| `fifo_min` | 3 |
| `fifo_max` | 2 |
| `MetalGpuPassRecord` | 22 |
| `sample_buffer` | 12 |
| `sample_buffer_index` | 5 |
| `first_sample` | 11 |
| `sample_count` | 12 |
| `label` | 22 |
| `width` | 33 |
| `height` | 35 |
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
| `shaders` | 12 |
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
| `source` | 11 |
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
| `key` | 40 |
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
| `queue_arrival` | 11 |
| `msl_compile_arrivals_` | 4 |
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
| `render_area_by_attachments_` | 3 |
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
| `hotspot_pc` | 5 |
| `hotspot_place` | 12 |
| `hotspot_function` | 5 |
| `hotspot_symbol` | 4 |
| `hotspot_busy` | 3 |
| `hotspot_count` | 6 |
| `hotspot_sorted` | 15 |
| `hotspot_text` | 6 |
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
| `area_attachments` | 6 |
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
| `metal_inline_shader_constants` | 5 |
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
| `position` | 26 |
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
| `slice` | 7 |
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
| `LogKernelCallStackCode` | 6 |
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
| `storage_msaa_samples` | 10 |
| `IsEdramTileOwnedBy` | 4 |
| `msaa_key` | 17 |
| `first_rt_index` | 5 |
| `clear_first_tile` | 4 |
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
| `signal_thread` | 9 |
| `signal_count` | 3 |
| `last_signal_callers_` | 6 |
| `callers` | 4 |
| `caller_frame` | 5 |
| `metal_hud` | 6 |
| `metal_hud_elements` | 5 |
| `metal_hud_alignment` | 5 |
| `ios_device_status` | 3 |
| `device_status_label_` | 16 |
| `device_status_thermal_` | 4 |
| `show_device_status` | 4 |
| `thermal` | 4 |
| `thermal_name` | 5 |
| `thermal_color` | 5 |
| `shown_sections_` | 11 |
| `shown_items_` | 5 |
| `applySettingsSearch` | 3 |
| `search_text` | 2 |
| `filtering` | 4 |
| `shown` | 3 |
| `last_signal_caller` | 3 |
| `kernel_call_signal_callers` | 4 |
| `kernel_call_event` | 4 |
| `kernel_call_signal_caller` | 3 |
| `debug_watch_guest_addresses` | 7 |
| `StartGuestValueWatch` | 3 |
| `watch_started` | 2 |
| `watch_addresses` | 9 |
| `watch_start` | 12 |
| `watch_end` | 10 |
| `watch_text` | 6 |
| `watch_parse_end` | 8 |
| `watch_address` | 2 |
| `watch_count` | 8 |
| `watch_min` | 6 |
| `watch_max` | 6 |
| `watch_last` | 3 |
| `watch_zero` | 5 |
| `watch_samples` | 5 |
| `watch_window_start` | 2 |
| `watch_readable` | 4 |
| `watch_host` | 3 |
| `watch_global` | 3 |
| `watch_step` | 3 |
| `watch_value` | 5 |
| `watch_line` | 3 |
| `debug_dump_guest_code` | 7 |
| `dump_ranges` | 6 |
| `dump_first` | 6 |
| `dump_last` | 6 |
| `dump_line` | 6 |
| `dump_global` | 3 |
| `dump_words` | 4 |
| `dump_lock` | 1 |
| `IOSConfigItemValuesDiffer` | 3 |
| `ApplyGameConfigToSections` | 2 |
| `BuildGameConfigTable` | 2 |
| `game_title_id_` | 11 |
| `global_sections_` | 6 |
| `initWithGameTitleId` | 3 |
| `isGameOverrideAtIndexPath` | 3 |
| `useGlobalSettingsTapped` | 2 |
| `presentGameSettingsForIndex` | 2 |
| `presentSettingsController` | 3 |
| `game_settings_action` | 3 |
| `use_global` | 2 |
| `game_saved` | 5 |
| `game_alert` | 3 |
| `copy_button` | 6 |
| `paste_button` | 6 |
| `pasted` | 3 |
| `text_stream` | 3 |
| `metal_share_argument_encoders` | 4 |
| `g_argument_encoder_cache_mutex` | 3 |
| `g_argument_encoder_cache` | 5 |
| `GetArgumentBufferLayoutKey` | 3 |
| `argument_buffer_layout_key` | 6 |
| `kStructPrefix` | 2 |
| `struct_begin` | 8 |
| `struct_end` | 6 |
| `id_pos` | 4 |
| `name_begin` | 7 |
| `device_address` | 3 |
| `unresolved_calls` | 2 |
| `host_crash_context` | 8 |
| `host_code_start` | 6 |
| `longjmp_resumes` | 2 |
| `integer_bytes` | 2 |
| `integer_unsigned_max` | 3 |
| `integer_unsigned_scale` | 2 |
| `integer_signed_scale` | 2 |
| `integer_remaining_components` | 3 |
| `integer_component_index` | 6 |
| `integer_sign_signed` | 2 |
| `texture_integer_bytes` | 5 |
| `swizzle_component` | 4 |
| `xma_census_next_ms` | 3 |
| `xma_census_now_ms` | 4 |
| `xma_census_due_ms` | 3 |
| `xma_census_keys` | 4 |
| `xma_census_allocated` | 3 |
| `xma_census_holding` | 3 |
| `xma_census_stalled` | 3 |
| `xma_census_context` | 3 |
| `xma_census_data` | 24 |
| `xma_census_key` | 3 |
| `kernel_call_history_head` | 9 |
| `kernel_call_slot` | 4 |
| `xma_census_list` | 3 |
| `use_dedicated_thread` | 3 |
| `metal_resolve_keep_command_buffer` | 4 |
| `resolve_keep_command_buffer` | 2 |
| `metal_shader_fast_math` | 3 |
| `sleep_zero_backoff_us` | 2 |
| `sleep_zero_last_us` | 3 |
| `sleep_zero_run` | 4 |
| `sleep_zero_backoff` | 5 |
| `sleep_zero_now_us` | 3 |
| `unhandled_symbol` | 5 |
| `unhandled_backtrace` | 4 |
| `unhandled_name` | 4 |
| `address` | 22 |
| `unhandled_stack_top` | 2 |
| `unhandled_frame` | 9 |
| `unhandled_depth` | 3 |
| `unhandled_guest_fault_count` | 2 |
| `hotspot_totals` | 3 |
| `hotspot_total_lines` | 4 |
| `hotspot_dumped` | 2 |
| `hotspot_sorted` | 15 |
| `hotspot_text` | 6 |
| `hotspot_total` | 5 |
| `hotspot_memory` | 2 |
| `hotspot_address` | 9 |
| `hotspot_count` | 6 |
| `hotspot_line` | 4 |
| `hotspot_line_count` | 4 |
| `it` | 7 |
| `i` | 8 |
| `hotspot_start` | 3 |
| `row` | 15 |
| `word` | 4 |
| `resolve_check_mask` | 2 |
| `would_load` | 4 |
| `texture_it` | 4 |
| `signed_key` | 3 |
| `EmitVmxFmaV128InPlace` | 3 |
| `Args` | 2 |
| `is_sub` | 2 |
| `d` | 2 |
| `s1` | 2 |
| `s2` | 2 |
| `done` | 4 |
| `nan_lane` | 2 |
| `src1` | 4 |
| `src2` | 4 |
| `src3` | 4 |
| `scratch` | 4 |
| `alpha` | 16 |
| `half_shift` | 11 |
| `metal_vertex_shader_fast_math` | 2 |
| `kSysFlag_Color0Fixed16Scaled_Shift` | 2 |
| `kSysFlag_Color0Fixed16Scaled` | 3 |
| `factor_color` | 2 |
| `GetFixed16SourceFactorPremult` | 3 |
| `ApplyFixed16SourceFactorPremult` | 4 |
| `linear_op` | 3 |
| `reads_source` | 5 |
| `rgb_factor_out` | 4 |
| `a_factor_out` | 4 |
| `rgb_premult` | 6 |
| `a_premult` | 6 |
| `DrawReadsPendingResolveOutput` | 4 |
| `SplitForPendingResolveOutput` | 4 |
| `known_texture_mask` | 5 |
| `metal_spirv2air_probe` | 2 |
| `Spirv2airProbeStats` | 3 |
| `translated` | 4 |
| `libraries` | 6 |
| `functions` | 7 |
| `translate_ms` | 5 |
| `library_ms` | 6 |
| `msl_library_ms` | 8 |
| `rejections` | 6 |
| `library_errors` | 2 |
| `dumped` | 3 |
| `g_spirv2air_probe_mutex` | 2 |
| `g_spirv2air_probe_stats` | 2 |
| `ProbeSpirv2air` | 2 |
| `translate_begin` | 2 |
| `library_error` | 5 |
| `library_begin` | 2 |
| `accepted` | 4 |
| `spirv2air` | 28 |
| `msl_compile_frame_arrival_` | 3 |
| `pending_it` | 9 |
| `metal_shader_compiler` | 4 |
| `cached` | 8 |
| `kSpirv2airOptionsRevision` | 2 |
| `GetSpirv2airOptions` | 3 |
| `GetSpirvExecutionModel` | 2 |
| `word_count_op` | 3 |
| `CompileSpirv2air` | 3 |
| `library_out` | 3 |
| `function_out` | 3 |
| `msl_compile` | 3 |
| `cached_metallib` | 5 |
| `spirv2air_model` | 3 |
| `spirv2air_cache_key` | 3 |
| `spirv2air_library_` | 5 |
| `spirv2air_function_` | 8 |
| `spirv2air_function` | 5 |
| `Spirv2airLayout` | 45 |
| `kResourceHeapBuffer` | 4 |
| `kSamplerHeapBuffer` | 4 |
| `kArgumentBuffer` | 4 |
| `kConstantBufferCount` | 3 |
| `kSharedMemoryReadOffset` | 4 |
| `kSharedMemoryWriteOffset` | 4 |
| `kVertexTextureIndicesOffset` | 4 |
| `kFragmentTextureIndicesOffset` | 4 |
| `kArgumentBufferBytes` | 3 |
| `kHeapRecordBytes` | 6 |
| `kHeapTextureHandleOffset` | 3 |
| `kMaxBindings` | 5 |
| `spirv2air_texture_indices_` | 10 |
| `records` | 3 |
| `use_spirv2air` | 12 |
| `compile_msl_for_draw` | 3 |
| `metal_spirv2air_declaration_attributes` | 2 |
| `GetSpirv2airVariant` | 3 |
| `kDeclarationAttributesShift` | 4 |
| `kAttributesShift` | 5 |
| `baseline` | 3 |
| `with_calls` | 5 |
| `game_pairs` | 4 |
| `pair_name` | 1 |
| `kAnswerTimeout` | 2 |
| `kTimeBudget` | 2 |
| `self_test_begin` | 2 |
| `stopped` | 6 |
| `Answer` | 2 |
| `answered` | 3 |
| `answer` | 13 |
| `run` | 15 |
| `controls` | 11 |
| `control_salt` | 2 |
| `control` | 22 |
| `count` | 20 |
| `best_mode` | 5 |
| `best_built` | 5 |
| `mode` | 4 |
| `mode_built` | 6 |
| `variant_test` | 3 |
| `game_variant` | 4 |
| `vertex_name` | 3 |
| `pair_id` | 3 |
| `metal_spirv2air_options` | 2 |
| `GetSpirv2airCacheSeed` | 2 |
| `variant` | 26 |
| `spirv2air_variant` | 4 |
| `spirv2air_options_` | 3 |
| `spirv2air_options` | 2 |
| `kDirectSharedMemory` | 3 |
| `kDirectTextures` | 4 |
| `kConstantTextureIndices` | 2 |
| `kPreciseMath` | 2 |
| `kOptionMask` | 2 |
| `dumped_pipeline_shaders` | 2 |
| `kVertexSharedMemory` | 3 |
| `kPixelConstantBufferIndexed` | 3 |
| `kPixelSharedMemory` | 3 |
| `kPixelMath` | 3 |
| `module_of` | 15 |
| `modules` | 8 |
| `kSharedMemory` | 3 |
| `kTextures` | 3 |
| `dumps` | 3 |
| `file_name` | 5 |
| `read_words` | 3 |
| `bytes` | 3 |
| `paths` | 7 |
| `vertex_index` | 6 |
| `pixel_index` | 7 |
| `function_of` | 3 |
| `module` | 5 |
| `entry` | 4 |
| `index` | 28 |
| `inserted` | 2 |
| `build` | 3 |
| `log_success` | 2 |
| `probe_dir` | 3 |
| `pair` | 4 |
| `direct_textures` | 6 |
| `sampler_index` | 4 |
| `metal_spirv2air_self_test` | 3 |
| `RunSpirv2airSelfTest` | 3 |
| `DumpSpirv2airPipelineShaders` | 3 |
| `pipelines_dumped` | 3 |
| `spirv2air_self_test` | 2 |
| `kVertexPosition` | 3 |
| `kVertexOutputs` | 3 |
| `kPixelConstant` | 3 |
| `kPixelInput` | 3 |
| `kPixelCentroidInput` | 3 |
| `kPixelConstantBuffer` | 3 |
| `kPixelTexture` | 3 |
| `kPixelTextureArrayGradients` | 3 |
| `kPixelDiscard` | 3 |
| `kPixelEarlyFragmentTests` | 3 |
| `test` | 29 |
| `Module` | 4 |
| `kMslSource` | 2 |
| `Test` | 7 |
| `error_text` | 5 |
| `built` | 10 |
| `failed` | 3 |
| `t` | 15 |
| `begin` | 4 |
| `pipeline_ms` | 3 |
| `spirv` | 3 |
| `name` | 14 |
| `words` | 10 |
| `vertex` | 7 |
| `pixel` | 11 |
| `spirv2air_msl_fallbacks` | 2 |
| `bind_spirv2air_stage` | 3 |
| `record_count` | 8 |
| `kSliceAlignment` | 4 |
| `kHeapOffset` | 7 |
| `slice_buffer` | 10 |
| `slice_offset` | 9 |
| `argument_buffer` | 14 |
| `uniforms_address` | 4 |
| `stage_uniforms` | 6 |
| `shared_memory_address` | 3 |
| `indices_address` | 3 |
| `heap` | 4 |
| `handle` | 6 |
| `stale_end` | 2 |
| `has_stencil` | 3 |
| `depth_clear_retargets` | 4 |
| `ios_tap_to_open_menu` | 2 |
| `ios_signin_prompt` | 2 |
| `ios_keyboard_prompt` | 2 |
| `kChooseAchievementSound` | 3 |
| `kClearAchievementSound` | 3 |
| `interface` | 9 |
| `xe_play_achievement_sound` | 3 |
| `player` | 8 |
| `destination` | 4 |
| `extension` | 2 |
| `XeniaTouchPadControlKindMenu` | 6 |
| `XeniaTouchPadPlacement` | 6 |
| `placements_` | 13 |
| `placement` | 18 |
| `scale` | 12 |
| `hidden` | 12 |
| `edit_touch_` | 10 |
| `edit_offset_` | 4 |
| `editing` | 19 |
| `menuHandler` | 4 |
| `selectionHandler` | 8 |
| `loadLayoutForTitleID` | 5 |
| `layoutTitleID` | 2 |
| `layoutIsPerGame` | 2 |
| `saveLayoutPerGame` | 3 |
| `resetLayout` | 3 |
| `selectedControl` | 1 |
| `selectedControlLabel` | 3 |
| `selectedScale` | 6 |
| `toggleSelectedHidden` | 3 |
| `selectedHidden` | 3 |
| `layoutOpacity` | 3 |
| `XeniaTouchLayoutKey` | 6 |
| `controlIsActive` | 3 |
| `applyLayoutDictionary` | 3 |
| `layoutDictionary` | 2 |
| `opacity` | 10 |
| `defaults` | 7 |
| `layout` | 12 |
| `per_game` | 3 |
| `center` | 23 |
| `selected` | 3 |
| `dashes` | 2 |
| `system_ui_reported_` | 3 |
| `system_ui` | 4 |
| `controller_connected` | 3 |
| `achievement_queue_` | 7 |
| `achievement_showing_` | 4 |
| `showAchievementNamed` | 3 |
| `showNextAchievement` | 3 |
| `banner` | 20 |
| `icon_image` | 2 |
| `icon_view` | 12 |
| `heading` | 12 |
| `top` | 4 |
| `name_string` | 4 |
| `inGameEditControlsButton` | 9 |
| `edit_controls_config` | 9 |
| `openInGameMenu` | 3 |
| `inGameEditControlsTapped` | 3 |
| `weak_self` | 3 |
| `layout_title_id` | 3 |
| `touch_editor_bar_` | 21 |
| `touch_editor_label_` | 7 |
| `touch_editor_hide_button_` | 5 |
| `touch_editor_opacity_slider_` | 8 |
| `touch_editor_scope_` | 7 |
| `touch_editor_top_` | 5 |
| `touch_editor_bottom_` | 3 |
| `XeniaTouchEditorButton` | 7 |
| `buildTouchEditorBar` | 2 |
| `updateTouchEditorBar` | 4 |
| `touchEditorSmallerTapped` | 2 |
| `touchEditorLargerTapped` | 2 |
| `touchEditorHideTapped` | 2 |
| `touchEditorOpacityChanged` | 2 |
| `touchEditorResetTapped` | 2 |
| `touchEditorMoveBarTapped` | 2 |
| `touchEditorDoneTapped` | 2 |
| `opacity_label` | 5 |
| `pad` | 4 |
| `to_bottom` | 3 |
| `SystemUICallback` | 3 |
| `set_system_ui_callback` | 2 |
| `system_ui_callback_` | 4 |
| `AchievementCallback` | 3 |
| `set_achievement_callback` | 2 |
| `achievement_callback_` | 4 |
| `SetSystemUIActive` | 2 |
| `counted` | 10 |
| `ShowAchievement` | 2 |
| `OnUIState` | 5 |
| `ui_was_active_` | 3 |
| `ui_active` | 4 |

## Checks

- Input after XAM UI closes (`src/xenia/kernel/xam/ui_input_filter.h`,
  `.cc`) is ported from XeniOS 7a7b08993: `UIInputFilter`, `OnUIClosed`,
  `FilterState`, `ShouldDropKeystroke`, `held_buttons_`,
  `kMaxQueuedKeystrokes`, XamState's `ui_input_filter()`/`ui_input_filter_`
  and the `filter` locals in `xam_input.cc` have upstream's names and roles,
  so they are not listed; only `OnUIState` and `ui_was_active_`, which this
  branch adds because its dialog counters change in many places, are.
  `achievement_sound_path` (DEFINE_path in `windowed_app_main_ios.mm`) has
  the name and role of the Qt build's cvar of that name (the pop-up's
  sound), which isn't built on iOS. In the iOS interface work
  (`windowed_app_main_ios.mm`: achievement pop-up, settings sound picker,
  touch layout editor, prompt toggles; `xenia_main_ios.mm`;
  `achievement_manager.cc`) the locals and parameters `title`, `url`, `ec`,
  `button`, `label`, `name`, `icon`, `icon_data`, `gamerscore`, `target`,
  `action`, `finished`, `done`, `size`, `touch`, `entry`, `access_granted`,
  `kernel_state`, `ios_context`, `game`, `title_id`, `slider` and `sender`
  are named and used as elsewhere in those files, so they are not listed.
  `position` is also XeniaTouchPadPlacement's member, counted in its row.

- `msl_shader_compile_pending_` (not listed, from the branch's base) is now a
  map from the translation to the queue_arrival of its latest entry instead of
  a set.
- `third_party/spirv2air` is copied from xenios-jp/spirv2air 84638cec9 (BSD-3, the
  commit XeniOS pins), changed only to target iOS 16 (its XENIOS.md lists the
  changes), so its names are not listed. In `CompileSpirv2air`,
  `GetSpirv2airOptions` and `ProbeSpirv2air` the locals and parameters
  `to_ms`, `options`, `bindings`, `result`, `pool`, `data`, `error`,
  `library`, `function`, `stage`, `lock`, `stats`, `dump`, `reason`, `ec`,
  `base`, `spv`, `metallib`, `count`, `device`, `is_ios`, `words`,
  `word_count`, `is_vertex`, `shader_hash`, `modification` and `cache_key`
  are named as the same locals are elsewhere in `msl_shader.cc` and in
  XeniOS's Metal code (`dump_dir` as in this file's MSL failure dump), so they
  are not listed.
- spirv2air drawing path (metal_shader_compiler "spirv2air"): in
  `bind_spirv2air_stage` the locals `texture_bindings`, `sampler_bindings`,
  `texture_count`, `binding`, `texture`, `sampler_state` and
  `metal_texture_cache` are named and used as in the Metal Shading Language
  path's `bind_msl_argument_buffer` above it, and `shader`/`is_pixel_stage` as
  its parameters; `translation` (IssueDrawMsl's fallback loop) and
  `vertex_function`/`fragment_function` (GetOrCreateMslPipelineState, the
  functions going into MslPipelineCompileRequest's fields of those names) are
  named after what they hold, as elsewhere in `metal_command_processor.cc`.
  Not listed. `metal_shader_compiler` has the name and role (which compiler
  translates guest shaders, spirv2air falling back to another) of XeniOS's
  cvar, whose values there are "spirv2air" and "msc" on its DXIL path; it is
  listed because this branch's values and path differ.
- spirv2air self-test (metal_spirv2air_self_test), pipeline dumps and
  direct bindings (metal_spirv2air_options): in `RunSpirv2airSelfTest` the
  locals `to_ms`, `result`, `pool`, `data`, `error`, `library`, `function`,
  `options` (the MTL::CompileOptions), `stage` ("VS"/"PS"), `ec` and the
  parameters `device` and `is_ios` are named and used as in
  `CompileSpirv2air`, `desc`, `pipeline`, `vertex_function` and
  `fragment_function` as in `CreateMslPipelineState` (`pipeline` and
  `error` also as the completion handler's parameters), `lock` as a
  lock_guard/unique_lock elsewhere, and `it` as an iterator elsewhere; in `DumpSpirv2airPipelineShaders` `dump_dir`, `lock`,
  `stats`, `ec`, `base` and `spv` are as in `CompileSpirv2air`, and
  `translation` as in IssueDrawMsl's loop; in `bind_spirv2air_stage` the
  parameter `translation` and the locals `bound_textures` and
  `bound_samplers` are named and used as in `bind_msl_textures` and
  `bind_msl_samplers`. Not listed. The test shaders' GLSL is in
  `spirv2air_self_test_spirv.h` above each array. spirv2air's own new options
  (`sharedMemoryBuffer`, `directTextures`, `constantTextureIndices`,
  `firstConstantBuffer`, `preciseMath`, `declarationAttributes`, the lambda
  `drop`, and the locals `directTextures`, `sharedVars`, `writable`, `arg`,
  `firstSampler`, `binding`, `index`, `typeName`, `argOfBinding`, `it`,
  `inserted` and `direct`, and `b`, `i`, `e` and `z` in the fneg lowering, in
  translate.cpp) are in the vendored copy, listed in its XENIOS.md instead.
- `depth_attachment` and `stencil_attachment` in `BeginCommandBuffer` (the
  attachment textures keying `render_area_by_attachments_`) are render pass
  attachment descriptors, as the same names are elsewhere in
  `metal_command_processor.cc` and in XeniOS's Metal backend, so they are
  not listed.
- `rt0_format` (local in `GetCurrentSpirvPixelShaderModification`,
  `src/xenia/gpu/metal/metal_command_processor.cc` 11360, 11364, 11365; 3
  uses) has the same name and role in XeniOS's version of that function
  (render target 0's color format), so it is not listed.
- Memory read watches (`src/xenia/memory.h`, `src/xenia/memory.cc`,
  `src/xenia/kernel/xfile.cc`) are ported verbatim from XeniOS 0fa163a23,
  77f2cca80, 935e03876, 78315ec19 and 98b6319c5, so their names are not
  listed: `PhysicalAccess` (`kRead`, `kWrite`, `kDiscard`),
  `PhysicalMemoryReadCallback`, `RegisterPhysicalMemoryReadCallback`,
  `UnregisterPhysicalMemoryReadCallback`, `physical_memory_read_callbacks_`,
  `physical_memory_read_callback_calls_mutex_`, `notify_on_read`,
  `ProvideReadWatchedPage`, `ProvideReadWatchedEdgePages`, the
  `TriggerCallbacks` parameter `contents_discarded`, the second
  `EnableAccessCallbacksInner` template parameter `enable_data_providers`, and
  the locals (`block_last`, `block_mask`, `block`, `armed`, `calls_lock`,
  `callbacks`, `callback_count`, `access`, `host_first`, `host_last`,
  `page_mask`, `first_partial`, `last_partial`, `one_page`, `provide_page`,
  `any_read_watched`, `read_callback`, `bit`, `flags`, `guest_access`,
  `target`, `dropped_first`, `dropped_last`, `dropped_count`, `discard_first`,
  `discard_last`, `host_end`, `notify`, `start`, `end`, `file_size`,
  `read_length`, `buffer`, `buffer_heap`). Two differences kept on purpose:
  the write path still returns true when it finds no watch (XeniOS returns
  whether the guest may write the page), and our comment on
  `SystemPageGuestAccess` stays in place of XeniOS's out-of-range log.
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
- `tile` and `key` (parameters of IsEdramTileOwnedBy), `it` (its map
  iterator), and `frame` and `caller` (locals in XEvent::NoteSignal) are
  generic names with the same role elsewhere, not listed.
- `has_notified_xmp_startup_` (KernelState) and the `signal` parameter of
  XNotifyListener::EnqueueNotification have the same names and roles in
  xenia-edge HEAD (`refs/remotes/xenia-edge-deep/HEAD`), where the startup
  music player notifications come from, so they are not listed.
- Per-game settings: `item`, `global_item`, `section`, `sections`,
  `global_sections`, `game_config`, `var`, `node`, `text`, `category`,
  `value`, `parsed`, `s`, `i`, `section_index`, `items`, `title`,
  `title_id`, `game`, `game_index`, `settings_vc`, `confirm`, and `field`
  (the __block text field in the editor) are generic names with the same
  role elsewhere, not listed.
- `game_title_` in XeniaConfigViewController has the same name and role as
  the existing ivar of the compatibility and content sheets, not listed.
- Shared argument encoders (msl_shader.cc): `layout`, `line`, `it` and
  `lock` are generic locals with the same role elsewhere, and `line_begin`
  and `line_end` already name line bounds in the same file (the
  strip_nocontract_optnone pass in CompileToMsl), so they are not listed;
  `name_end` has the same name and role in XeniOS HEAD.
- Notification listeners (xnotifylistener.{h,cc}): `QueueKey`,
  `RemoveNotification`, the `Notifications` alias, and the locals `area`,
  `rest`, `key`, `notification`, `id` and `data` have the same names and
  roles in xenia-edge HEAD (`refs/remotes/xenia-edge-deep/HEAD`, commit
  a573178da "Fix notify listeners queueing repeats and staying signaled"),
  where the change comes from, so they are not listed.
- Black Ops kernel exports (xboxkrnl_io.cc: StfsCreateDevice,
  StfsControlDevice, IoDismountVolume, IoDismountVolumeByFileHandle and their
  parameters `params`, `params_size`, `device`, `control_code`, `buffer`,
  `device_object`, `file_handle`, `file`; xboxkrnl_crypt.cc: XECRYPT_SIG,
  XE_CONSOLE_ID and its fields, X_CONSOLE_PUBLIC_KEY, XConsoleType and its
  enumerators, XE_CONSOLE_CERTIFICATE and its fields, XE_CONSOLE_SIGNATURE,
  FillConsoleCertificate, XeKeysConsolePrivateKeySign,
  XeKeysConsoleSignatureVerification and their `hash`, `output`,
  `signature`, `same_console`, `certificate`, `matches`) have the same names
  and roles in xenia-edge HEAD (`refs/remotes/xenia-edge-deep/HEAD`), where
  they come from, so they are not listed.
- The a64 thunk order change (resolve thunk first) and the
  user_language/user_country fix (xboxkrnl_xconfig.cc now reads XConfig's
  int32 cvars; the string definitions are gone) add no names.
- iOS title relaunch (xenia_main_ios.mm): `relaunch_extension`,
  `relaunch_target_is_xex`, `relaunch_target_error` and
  `relaunch_target_is_directory` have the same names and roles in XeniOS HEAD
  (`refs/remotes/xenios/HEAD`), where the container case comes from, so they
  are not listed. Clearing the stale title stop request uses the existing
  Processor::ClearTitleStopRequestIOS. The resolve.xesli unsigned integer
  scale, the metal_presenter.mm swap through gamma_output_texture_ and the
  ResolveFunction first-instruction skip (XeniOS 6e304ab86) add no names.
- Physical memory write watches (memory.h, memory.cc), the write-watch parts
  of xenia-edge aed81ca93 and d1541a579: `PhysicalHeap::SystemPageGuestAccess`
  (with its locals `offset`, `system_base`, `system_last`, `guest_page_first`,
  `guest_page_count`, `guest_page_last`, `access`, `page_access`) and the
  `invalidate_unwatched` parameter of `PhysicalHeap::TriggerCallbacks` have the
  same names and roles in xenia-edge and XeniOS HEAD, so they are not listed.
  `ToPageAccess` moved from memory.cc to memory.h as upstream. Removed:
  `GuestPagenumToSystemPagenum` and EnableAccessCallbacksInner's unused locals
  (`page_table_ptr`, `first_guest_page`, `last_guest_page`, `guest_one`,
  `system_one`, `guest_page_number`), as upstream.
- Integer texture scale rework (spirv_shader_translator_fetch.cc,
  metal_command_processor.cc): the shader no longer reads the format; the
  names `integer_format`, `integer_format_is_any`, `integer_8`, `integer_16`,
  `integer_word_3`, `integer_num_format`, `integer_is_component`,
  `integer_sign` and `integer_scale` are gone. `texture_fetch` (a fetch
  constant read with RegisterFile::GetTextureFetch) has the same name and
  role in shader_translator.cc here and in xenia-edge and XeniOS HEAD.
- Socket errors (xsocket.cc): AsioErrorToWSAError's body is xenia-edge
  250fb40dc as in XeniOS HEAD, no new names. NtRemoveIoCompletion
  (xboxkrnl_io.cc, xenia-edge d287b30be) and the fresh emulator per title
  (xenia_main_ios.mm, reusing emulator_cpu_initialized_) add no names.
- Zero-copy GPU reads (shared_memory.h/.cc, metal_shared_memory.cc,
  primitive_processor.cc, metal_command_processor.cc), XeniOS 0f21056f3:
  `SharedMemory::RequestRangeForGpuRead` and `buffer_is_guest_memory_` have
  the same names and roles in XeniOS HEAD, so they are not listed. Its trace
  half (babc60b29, `IsTraceOpen`) is not ported: this branch's traces do not
  record shared memory uploads.
- POSIX file opening (filesystem_posix.cc), xenia-edge 1e7c1f677, c9536890c
  and b9e00e6cc as in XeniOS HEAD: `wants_read`, `wants_write`,
  `append_only` and `PosixFileHandle::append_only_` have the same names and
  roles there.
- Host file lifetimes, in-place overwrite and file times (entry.h/.cc,
  file.h, host_path_entry.h/.cc, host_path_file.h/.cc, virtual_file_system.cc,
  filesystem.h, filesystem_posix.cc, filesystem_win.cc, xfile.h/.cc,
  info/file.h, xboxkrnl_io_info.cc), xenia-edge 494e2ef67, 98fad6f14 and
  8a179c7af as in XeniOS HEAD: `has_open_files`, `RetainFile`, `ReleaseFile`,
  `Overwrite`, `SetOverwrittenSize`, `open_files_`, `overwritten_`, the
  `SetTimes` members (FileHandle, PosixFileHandle, Win32FileHandle, File,
  HostPathFile, HostPathEntry, XFile) with their locals (`times`, `create`,
  `access`, `write`), `convertWinFiletimeToTimespec` (with `ticks`,
  `seconds`, `remainder`), `kTimeUnit` and X_FILE_BASIC_INFORMATION's `pad`
  have the same names and roles there. Removed as upstream:
  `Entry::SetAttributes`, `SetCreateTimestamp`, `SetAccessTimestamp`,
  `SetWriteTimestamp`, their HostPathEntry overrides and
  `xe::filesystem::SetAttributes` (posix, win, android). Not ported: the
  Windows FILE_WRITE_ATTRIBUTES access right (a POSIX owner sets times through
  any descriptor) and xe::strip_pii (not in this branch).
- Kernel call repeat detector (shim_utils.cc): the history is a ring
  (`kernel_call_history_head`, `kernel_call_slot`, listed above) instead of
  being shifted by 31 entries twice on every kernel call; the reports are
  unchanged.
- XMA census rework (xma_decoder.cc): contexts are counted by their state
  (holding input) rather than the enabled flag, which a kick clears at once:
  `xma_census_enabled` is now `xma_census_holding`, `xma_census_first` is
  gone and `xma_census_list` is new (listed above).
- metal_resolve_keep_command_buffer (metal_command_processor.cc) and
  sleep_zero_backoff_us (xthread.cc) with their locals are listed above.
  `metal_resolve_keep_command_buffer` has no XeniOS HEAD counterpart under
  that name; XeniOS HEAD keeps the buffer open after resolves unconditionally
  with a read watch on the output (237a78443), not ported. Default off since
  build 9676: the resolve-pending flag stays set for the whole buffer, so
  every later draw reading resolved memory with changed bindings restarted
  the render pass (GTA IV 24-88 passes a frame, attachment traffic doubled,
  GPU-bound frames slower), and ACR's save menu flickered with it.
- metal_shader_fast_math (msl_shader.cc): XeniOS and xenia-edge HEAD pass
  setFastMathEnabled(true) unconditionally for these shaders; the switch is
  new here (default unchanged).
- Diagnostics added for build 9678: the unhandled host fault log names the
  code (pc, lr, frame-pointer walk; `unhandled_*` above), MMIOHandler says
  which guest address a fault that is neither MMIO nor a watch hit
  (`unhandled_guest_fault_count`), Memory logs where guest memory is mapped,
  and the guest hotspot sampler keeps totals since launch (`hotspot_totals`
  and the names after it). `hotspot_sorted`, `hotspot_text` and
  `hotspot_count` are declared again in that later scope.
- MetalSharedMemory's `trace_writer_` member and `trace_writer` constructor
  parameter have the same names and role in XeniOS HEAD (metal_shared_memory
  .h/.cc), as does its WriteMemoryRead of each upload range; ported so Metal
  traces hold the guest memory draws read. Not listed.
- db16cyc spin-wait release (a64_seq_memory.cc, a64_backend.h, backend.cc):
  ported from XeniOS 9ae719d98 with the same names and roles, so not listed:
  `release_core_in_spin_waits`, `kSpinWaitReleaseNs`, `kDb16cycReleaseAfter`,
  `kDb16cycGapNs`, `SpinWaitTimerFrequency`, `frequency`, `SpinWaitRelease`,
  `EmitDb16cycCount`, `count_offset`, `last_tick_offset`, `gap_ticks`,
  `done`, `release`, and A64BackendContext's `db16cyc_count` and
  `db16cyc_last_tick`. Not ported: `spin_wait_release_handler` and the guest
  scheduler's fiber yield (the guest scheduler is off by default here; with
  it on, the release sleeps the dispatch thread). This branch keeps `yield`
  after the count where XeniOS emits `isb`.
- TextureCache::GetUsedTexturePotentialLoadMask (no callers before) now looks
  the texture up for a changed key; TraceWriter::WriteEvent clears the
  WriteMemoryReadCached set at each swap (no other callers of
  WriteMemoryReadCached), so Metal's zero-copy vertex buffers are recorded
  once a frame. No new names besides the rows above.
- `a64_vmx_nan_fixup_in_tail` (a64_backend.cc, a64_sequences.cc) has the same
  name and default as XeniOS d4bd5eae9/9064ede4e, where it also covers
  vaddfp/vsubfp/vmulfp/vdivfp; here it covers only the multiply-add and
  multiply-subtract (EmitVmxFmaV128InPlace, listed above). Not listed as new.
- `ffmpeg_disable_neon_on_arm64` default false on ARM64 too since build 9681
  (UPDATE_from_bool 2026-11-27 13h; `kLastCommittedUpdateDate` raised to it).
- The 8 bit resolve's alpha selection (resolve.xesli) follows xenia-edge
  2b3f0cb45 ("Select alpha for A8 resolves"); written for this branch's
  XeResolveLoad8RedColors, its names are listed above.
- XMA context port (build after 9691): `src/xenia/apu/xma_context_new.h/.cc`
  are XeniOS HEAD's (the XMA series 37bf631a3..0ce969b9e that went with its
  FFmpeg updates), so their names (`kPacketHandle`, `kDecodeStop`,
  `kPacketResult`, `kPendingAdvance`, `kNextFrame`, `FlushDecoder`,
  `GetPacketHandle`, `FindNextFrame`, `IsLoopEndFrame`, `GetPassBlocks`,
  `AdvanceAfterFrame`, `ApplyAdvance`, `WritePosition`, `SetError`,
  `StopOnParserError`, `current_frame_end_block_`, `frame_skipped_`,
  `advance_`, `decode_stop_`, `held_offset_`, `held_subframe_`,
  `held_buffer_`, the `kError*`/`kParserError*` constants and the locals) are
  not listed. The one change from XeniOS: a frame FFmpeg gives no samples for
  is output as silence instead of skipped (this branch's 59cb4515c rule), no
  new names. XMA_CONTEXT_DATA's DWORD 1 (`loop_subframe_end` at bits 12-13,
  `unk_dword_1_a`, `decode_subframe` at bits 24-26, replacing
  `loop_subframe_start` and `output_buffer_padding`), XmaContext::ConvertFrame's
  NEON path (`ConvertToInt32X64` and its locals), the zeroing in
  XmaDecoder::AllocateContext, and XMAInitializeContext (clear first, channel
  bit 0) and XMASetLoopData (XMA_LOOP_DATA) in xboxkrnl_audio_xma.cc are
  XeniOS HEAD's too. Not ported: XeniOS's work buffer pointer,
  XMABlockWhileInUse and XMADisableContext changes (they need its guest
  scheduler), and its removal of this branch's diagnostics.

- Depth clear retarget (depth_clear_retarget_4x): written for this branch's
  render target cache after XeniOS 38f08b521, with its names and roles:
  `depth_clear_retarget_4x`, `SetDepthClearRetargetCandidate`,
  `last_update_depth_clear_retargeted`, `DepthClearRetarget` and its members
  (`owner`, `rectangle`, `write_depth`, `guest_depth`, `write_stencil`,
  `stencil_value`, `stencil_write_mask`), `PerformDepthClearRetarget`,
  `GetDepthClearRetarget` and its locals and parameters,
  `depth_clear_retarget_candidate(_)`, `last_update_depth_clear_retargeted_`,
  `depth_clear_retarget_last_`, `depth_clear_retarget_stay_`, `depth_key`,
  `draw_key`, `retarget`, and in the Metal override `retarget_clear_states_`,
  `retarget_clear_state`, `state_key`, `clear_state`, `depth_texture`,
  `sample_count`, `constants`; `length_tiles` in Update is
  GetDepthClearRetarget's parameter of that name, and `rp`, `viewport`,
  `scissor`, `area_width`, `area_height`, `depth_attachment`,
  `stencil_attachment`, `depth_format`, `encoder`, `it`, `it_prev` are named
  as elsewhere in these files. The draw extent estimator's positions are
  XeniOS 032f00cd4's (`kMaxLastVertices`, `LastVertexPositions` and its
  `valid`, `count`, `x`, `y`, `z`, `w`, `last_vertex_positions(_)`,
  `position_x(_)`, `position_z(_)`, `positions`), here invalidated at the
  start of each estimate so a draw never sees an earlier draw's. Not ported
  from that series: strip merging, held transfers behind clear quads, chain
  cuts, constant colour clears, DontCare loads under clear quads and
  skip_no_effect_draws. Rows are listed only for `stale_end`, `has_stencil`
  and `depth_clear_retargets`.