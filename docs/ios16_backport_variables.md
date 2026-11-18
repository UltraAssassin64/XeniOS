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
| `kText` | enumerator of IOSConfigControlType | Settings row whose value is edited as text. | `src/xenia/ui/windowed_app_main_ios.mm` | 543, 1230, 3744, 8086, 8139 | 5 |
| `AddAllConfigVarSettings` | function | Adds a settings section per config category with every variable not already shown. | `src/xenia/ui/windowed_app_main_ios.mm` | 1204, 3695 | 2 |
| `existing_keys` | local in AddAllConfigVarSettings | Config keys the curated sections already expose, so they are not listed twice. | `src/xenia/ui/windowed_app_main_ios.mm` | 1208, 1212, 1218 | 3 |
| `by_category` | local in AddAllConfigVarSettings | Generated sections keyed by config category. | `src/xenia/ui/windowed_app_main_ios.mm` | 1216, 1234, 1238 | 3 |
| `parsed` | local in ApplyIOSConfigSections | The edited text parsed as a TOML value. | `src/xenia/ui/windowed_app_main_ios.mm` | 3758, 3760, 3763, 3770 | 4 |
| `editor` | local in tableView:didSelectRowAtIndexPath: | Alert with a text field used to edit a kText row. | `src/xenia/ui/windowed_app_main_ios.mm` | 8143, 8147, 8153, 8156, 8159, 8169 | 6 |
| `DrawStats` | struct in MetalCommandProcessor | Counters describing what happened to guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 578, 593 | 2 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.h` | 593 | 1 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2658, 2659, 2660, 2661, 2662, 2665, 2672, 2673, 2674, 2675, 2676, 2677, 2678, 2695, 2706, 2707, 2708, 2709, 2737, 2738, 2739, 3004, 3006, 3042, 3067, 4458, 4489, 4537, 5843, 5844 | 39 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.h` | 575, 577 | 2 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2665 | 1 |
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
| `SetTouchState` | function | Stores the on-screen controller state; called by the UI, read by the driver. | `src/xenia/ui/windowed_app_main_ios.mm` | 8243, 8399 | 2 |
| `TouchInputDriver` | class (anonymous namespace) | Input driver presenting the on-screen controller as a fallback gamepad. | `src/xenia/hid/touch_input_driver_ios.cc` | 29, 31, 182 | 3 |
| `touch_state_mutex_` | file-scope variable | Guards `touch_state_`. | `src/xenia/hid/touch_input_driver_ios.cc` | 24, 25, 66, 107, 162 | 5 |
| `touch_state_` | file-scope variable | Latest on-screen controller state and packet number. | `src/xenia/hid/touch_input_driver_ios.cc` | 27, 67, 108, 109, 113, 163, 177 | 8 |
| `kKeystrokeKeys` | local constant in GetKeystroke | Virtual key for each button bit. | `src/xenia/hid/touch_input_driver_ios.cc` | 85, 120, 123, 129, 153 | 5 |
| `keystroke_buttons` | local in GetKeystroke | Current buttons and triggers as a bit field. | `src/xenia/hid/touch_input_driver_ios.cc` | 105, 108, 111, 115, 122, 124 | 6 |
| `release_pass` | local in GetKeystroke | Loop counter: releases are reported before presses. | `src/xenia/hid/touch_input_driver_ios.cc` | 119, 125 | 3 |
| `keystroke_bit` | local in GetKeystroke | Bit of the button being examined. | `src/xenia/hid/touch_input_driver_ios.cc` | 121, 122, 124, 128 | 4 |
| `pressed` | local in GetKeystroke | Whether that button is currently down. | `src/xenia/hid/touch_input_driver_ios.cc` | 122, 125, 132 | 3 |
| `last_keystroke_buttons_` | member of TouchInputDriver | Buttons already reported through GetKeystroke. | `src/xenia/hid/touch_input_driver_ios.cc` | 124, 128, 154 | 3 |
| `XeniaTouchPadView` | Objective-C class | The on-screen controller view. | `src/xenia/ui/windowed_app_main_ios.mm` | 8241, 8263, 8270, 8510, 8609 | 5 |
| `XeniaTouchPadControl` | struct | One control of the on-screen controller: kind, button bit, label, centre, radius. | `src/xenia/ui/windowed_app_main_ios.mm` | 8253, 8271, 8304, 8449 | 4 |
| `XeniaTouchPadControlKind` | enum | Kind of control. | `src/xenia/ui/windowed_app_main_ios.mm` | 8245, 8254 | 2 |
| `XeniaTouchPadControlKindButton` | enumerator | A digital button. | `src/xenia/ui/windowed_app_main_ios.mm` | 8246, 8309, 8311, 8313, 8315, 8316, 8318, 8320, 8322, 8324, 8326, 8328, 8330, 8332, 8334, 8336 | 16 |
| `XeniaTouchPadControlKindLeftTrigger` | enumerator | Left trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8247, 8307, 8391 | 3 |
| `XeniaTouchPadControlKindRightTrigger` | enumerator | Right trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8248, 8308, 8393 | 3 |
| `XeniaTouchPadControlKindLeftStick` | enumerator | Left thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8249, 8305, 8370, 8381, 8454, 8482 | 6 |
| `XeniaTouchPadControlKindRightStick` | enumerator | Right thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8250, 8306, 8371, 8455, 8483 | 5 |
| `kXeniaTouchPadControlCount` | file-scope constant | Number of controls (19). | `src/xenia/ui/windowed_app_main_ios.mm` | 8261, 8271, 8304, 8339, 8346, 8448 | 6 |
| `controls_` | instance variable of XeniaTouchPadView | The controls with their current positions. | `src/xenia/ui/windowed_app_main_ios.mm` | 8271, 8340, 8347, 8348, 8350, 8370, 8371, 8373, 8374, 8381, 8391, 8393, 8396, 8449 | 16 |
| `pad_touches_` | instance variable of XeniaTouchPadView | Touches currently on the pad. | `src/xenia/ui/windowed_app_main_ios.mm` | 8273, 8284, 8291, 8367, 8368, 8405, 8408, 8417, 8430, 8432, 8452, 8453 | 12 |
| `pad_touch_controls_` | instance variable of XeniaTouchPadView | Control each of those touches started on. | `src/xenia/ui/windowed_app_main_ios.mm` | 8274, 8285, 8292, 8369, 8409, 8418, 8433, 8456 | 8 |
| `controlIndexAtPoint` | method of XeniaTouchPadView | Index of the control at a point, or -1. | `src/xenia/ui/windowed_app_main_ios.mm` | 8265, 8345, 8360, 8387, 8415, 8457, 10019 | 7 |
| `resetTouches` | method of XeniaTouchPadView | Releases every control and reports the idle state. | `src/xenia/ui/windowed_app_main_ios.mm` | 8267, 8404, 9407 | 3 |
| `reportState` | method of XeniaTouchPadView | Computes the gamepad state from the touches and passes it to SetTouchState. | `src/xenia/ui/windowed_app_main_ios.mm` | 8363, 8410, 8421, 8425, 8436 | 5 |
| `touchPadView` | property of XeniaViewController | The on-screen controller view instance. | `src/xenia/ui/windowed_app_main_ios.mm` | 8510, 8609, 8610, 8612, 8613, 9404, 9405, 9407, 10018, 10019 | 11 |
| `hide_touch_pad` | local in pollControllerNavigation | Whether the on-screen controller should be hidden now. | `src/xenia/ui/windowed_app_main_ios.mm` | 9401, 9404, 9405, 9406 | 4 |
| `width` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8298, 8302, 8303, 8494 | 5 |
| `height` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8299, 8305, 8306, 8319, 8321, 8323, 8325, 8327, 8329, 8331, 8333, 8335, 8337, 8495 | 15 |
| `insets` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8300, 8301, 8302 | 3 |
| `left` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8301, 8305, 8307, 8310, 8319, 8321, 8323, 8325, 8335 | 10 |
| `right` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8302, 8306, 8308, 8312, 8327, 8329, 8331, 8333, 8337 | 10 |
| `middle` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8303, 8314, 8315, 8317 | 4 |
| `controls` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8304, 8340, 8358 | 3 |
| `reach` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8350, 8351 | 3 |
| `dx` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8347, 8351 | 3 |
| `dy` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8348, 8351 | 3 |
| `buttons` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8255, 8364, 8396, 8399 | 4 |
| `triggers` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8365, 8386, 8392, 8394, 8399 | 6 |
| `axes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8366, 8382, 8383, 8399, 8400 | 7 |
| `point` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8264, 8265, 8345, 8347, 8348, 8359, 8360, 8368, 8373, 8374, 8387, 8453, 8457, 8460, 8461 | 15 |
| `index` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8369, 8370, 8371, 8373, 8374, 8381, 8387, 8388, 8391, 8393, 8396, 8415, 8416, 8418 | 16 |
| `x` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8347, 8373, 8375, 8377, 8382, 8460, 8462, 8464, 8467, 8494 | 18 |
| `y` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8348, 8374, 8375, 8378, 8383, 8461, 8462, 8465, 8467, 8495 | 18 |
| `length` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8375, 8376, 8377, 8378, 8462, 8463, 8464, 8465 | 8 |
| `axis` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8380, 8382, 8383 | 3 |
| `position` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8430, 8431, 8432, 8433 | 4 |
| `attributes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8444, 8493, 8496 | 3 |
| `control` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8264, 8266, 8272, 8349, 8449, 8451, 8454, 8455, 8460, 8461, 8463, 8464, 8465, 8467, 8472, 8473, 8482, 8483, 8486, 8493, 8494 | 22 |
| `active` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8450, 8458, 8477 | 4 |
| `knob` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8451, 8467, 8485, 8494, 8495 | 5 |
| `is_stick` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8454, 8456, 8459 | 3 |
| `outline` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8471, 8478, 8480, 8481 | 4 |
| `knob_path` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8484, 8491 | 2 |
| `label_size` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8493, 8494, 8495 | 3 |
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
| `kXeniaAutoEnableTrollStoreJITOnLaunchPreferenceKey` | static NSString constant, iOS UI | User-defaults key of the "Auto-Enable JIT via TrollStore" toggle (stored as ios_auto_enable_jit_via_trollstore). | `src/xenia/ui/windowed_app_main_ios.mm` | 594, 3663, 10764, 10794 | 4 |
| `jit_provider_name` | local in requestAutomaticStikDebugJITHandoffForPendingLaunchPath: | "TrollStore" or "StikDebug", for the status label and log lines. | `src/xenia/ui/windowed_app_main_ios.mm` | 10792, 10799, 10831, 10832, 10842, 10844 | 6 |
| `trollstore_url` | local in requestAutomaticStikDebugJITHandoffForPendingLaunchPath: | apple-magnifier://enable-jit?bundle-id=... request for TrollStore. | `src/xenia/ui/windowed_app_main_ios.mm` | 10795, 10797, 10798 | 4 |
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
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/util/shim_utils.h` | 547, 578 | 2 |
| `kernel_call_repeat_threshold` | cvar (uint32, Logging) | Calls in a row before a repeating kernel call or cycle is logged; 0 disables. Default 20000. | `src/xenia/kernel/util/shim_utils.cc` | 383 | 1 |
| `NoteKernelCall` | function, kernel shim | Called at the start of every kernel export trampoline; detects repeating calls per guest thread. | `src/xenia/kernel/util/shim_utils.h` | 548, 549, 580 | 3 |
| `NoteKernelCall` | function, kernel shim | Called at the start of every kernel export trampoline; detects repeating calls per guest thread. | `src/xenia/kernel/util/shim_utils.cc` | 339 | 1 |
| `kernel_call_history` | thread_local array | The thread's last 32 kernel exports, newest first. | `src/xenia/kernel/util/shim_utils.cc` | 362, 367, 375, 376, 380, 400, 404, 441, 455 | 9 |
| `kernel_call_runs` | thread_local array | For cycle lengths 1-8, how many calls in a row matched the call that many places earlier; only the shortest cycle at the threshold is reported. | `src/xenia/kernel/util/shim_utils.cc` | 364, 368, 370, 425 | 4 |
| `kernel_call_period` | local | Cycle length being checked (1-8). | `src/xenia/kernel/util/shim_utils.cc` | 365, 366, 367, 368, 370, 423, 424, 425, 439, 450 | 12 |
| `kernel_call_index` | local | Index into the history. | `src/xenia/kernel/util/shim_utils.cc` | 373, 374, 375, 376, 377, 378, 398, 399, 400, 404, 405, 439, 440, 441, 450, 451, 455 | 21 |
| `kernel_call_threshold` | local | Copy of the cvar for this call. | `src/xenia/kernel/util/shim_utils.cc` | 383, 428, 433, 434, 464, 488, 489 | 7 |
| `kernel_call_run` | local | Run length for the cycle length being checked. | `src/xenia/kernel/util/shim_utils.cc` | 425, 428, 433, 434, 460, 464, 488, 489 | 8 |
| `kernel_call_names` | local | Text of the repeating cycle ("A -> B"), of the last 32 calls in a storm report, or of a listed thread's suspend count, priority, CPU and start address. | `src/xenia/kernel/util/shim_utils.cc` | 278, 283, 292, 303, 397, 403, 414, 449, 452, 453, 455, 461 | 12 |
| `kernel_call_thread` | local | Current guest XThread, for its handle. | `src/xenia/kernel/util/shim_utils.cc` | 384, 410, 460 | 5 |
| `kernel_call_memory` | local | Guest memory, for the stack walk and the code dumps. | `src/xenia/kernel/util/shim_utils.cc` | 89, 108, 116, 310, 320, 468, 478 | 7 |
| `kernel_call_frame` | parameter of LogKernelCallStack | Guest stack frame being walked. | `src/xenia/kernel/util/shim_utils.cc` | 88, 99, 102, 104, 108, 109, 118 | 8 |
| `kernel_call_stack` | local | Text of the guest return addresses. | `src/xenia/kernel/util/shim_utils.cc` | 90, 103, 114, 121 | 4 |
| `kernel_call_depth` | local | Depth in the stack walk. | `src/xenia/kernel/util/shim_utils.cc` | 97, 98 | 3 |
| `kernel_call_caller_frame` | local | Back-chain pointer to the caller's frame. | `src/xenia/kernel/util/shim_utils.cc` | 107, 109, 110, 111, 117, 118 | 6 |
| `NoteKernelCallDone` | function, kernel shim | Clears the thread's in-call mark after every kernel export returns. | `src/xenia/kernel/util/shim_utils.h` | 550, 610 | 2 |
| `NoteKernelCallDone` | function, kernel shim | Clears the thread's in-call mark after every kernel export returns. | `src/xenia/kernel/util/shim_utils.cc` | 332 | 1 |
| `kernel_call_noted` | local in the export trampoline | Whether this call was reported to NoteKernelCall (so NoteKernelCallDone is called). | `src/xenia/kernel/util/shim_utils.h` | 578, 579, 609 | 3 |
| `KernelCallThreadState` | struct (anonymous namespace) | One guest thread's kernel-call state, readable from any thread. | `src/xenia/kernel/util/shim_utils.cc` | 55, 70, 71, 156, 341 | 5 |
| `kernel_call_threads_mutex` | file-static mutex | Guards kernel_call_threads. | `src/xenia/kernel/util/shim_utils.cc` | 69, 155, 346 | 3 |
| `kernel_call_threads` | file-static vector | Every thread's KernelCallThreadState; entries are never freed. | `src/xenia/kernel/util/shim_utils.cc` | 70, 156, 347 | 3 |
| `kernel_call_this_thread` | thread_local pointer | This thread's KernelCallThreadState. | `src/xenia/kernel/util/shim_utils.cc` | 71, 333, 334, 335, 340, 341, 343, 344, 347, 349, 350, 351, 352, 353, 355, 356, 357 | 17 |
| `LogKernelCallThreads` | function, kernel shim | Logs every guest thread: inside or last kernel call, for how long, guest lr, call count; for a thread blocked a second or more, its wait object and guest stack. Also called when the iOS app stops a game. | `src/xenia/kernel/util/shim_utils.cc` | 153, 418, 490 | 3 |
| `kernel_call_now` | local | Current host uptime in ms (in LogKernelCallThreads and in NoteKernelCall). | `src/xenia/kernel/util/shim_utils.cc` | 154, 165, 354, 355, 393, 394 | 6 |
| `kernel_call_lock` | local | Lock on kernel_call_threads_mutex. | `src/xenia/kernel/util/shim_utils.cc` | 155, 346 | 2 |
| `kernel_call_state` | local | Thread state being logged. | `src/xenia/kernel/util/shim_utils.cc` | 156, 157, 158, 165, 169, 173, 179, 198, 199, 200 | 12 |
| `kernel_call_last` | local | That thread's last export. | `src/xenia/kernel/util/shim_utils.cc` | 157, 160, 161, 171 | 5 |
| `kernel_call_inside` | local | Whether that thread is inside a kernel call now. | `src/xenia/kernel/util/shim_utils.cc` | 158, 160, 170, 172, 178 | 5 |
| `kernel_call_owner` | local | The new thread's XThread, for handle and name. | `src/xenia/kernel/util/shim_utils.cc` | 342, 343, 344 | 3 |
| `kernel_call_only_sleeps` | local | True when the repeating cycle is only KeDelayExecutionThread (idle, not reported). | `src/xenia/kernel/util/shim_utils.cc` | 438, 443, 446 | 3 |
| `kernel_call_code` | local | Guest address a code dump starts at (lr - 0x80 in a repeat report; sampled guest PC - 0x40 in the guest-thread list). | `src/xenia/kernel/util/shim_utils.cc` | 311, 312, 321, 325, 469, 479, 482 | 7 |
| `kernel_call_line` | local | First word of a code dump line. | `src/xenia/kernel/util/shim_utils.cc` | 313, 314, 316, 317, 325, 471, 472, 474, 475, 482 | 12 |
| `kernel_call_words` | local | Text of one code dump line. | `src/xenia/kernel/util/shim_utils.cc` | 315, 318, 325, 473, 476, 482 | 6 |
| `kernel_call_word` | local | Word being added to a code dump line. | `src/xenia/kernel/util/shim_utils.cc` | 316, 317, 321, 474, 475, 479 | 8 |
| `LogKernelCallStack` | function (anonymous namespace) | Returns the guest return addresses up the stack from a frame (back chain at 0(r1), saved LR at caller frame - 8). | `src/xenia/kernel/util/shim_utils.cc` | 88, 200, 306, 416, 466 | 5 |
| `guest_stack_pointer` | KernelCallThreadState field | Guest r1 at the thread's last kernel call, for its stack when it is blocked. | `src/xenia/kernel/util/shim_utils.cc` | 64, 200, 351 | 3 |
| `guest_arg0` | KernelCallThreadState field | Guest r3 at the thread's last kernel call (the handle for NtWaitForSingleObjectEx). | `src/xenia/kernel/util/shim_utils.cc` | 65, 179, 352 | 3 |
| `guest_arg1` | KernelCallThreadState field | Guest r4 at the thread's last kernel call. | `src/xenia/kernel/util/shim_utils.cc` | 66, 199, 353 | 3 |
| `kernel_call_ms` | local | How long the logged thread has been inside, or out of, its last call. | `src/xenia/kernel/util/shim_utils.cc` | 164, 172, 178 | 3 |
| `kernel_call_handle` | local | The blocked thread's r3, looked up in the object table. | `src/xenia/kernel/util/shim_utils.cc` | 179, 187, 198 | 3 |
| `kernel_call_object` | local | Text naming a wait object, or the guest function / host symbol a sampled host PC is in. | `src/xenia/kernel/util/shim_utils.cc` | 180, 188, 195, 199, 228, 239, 244, 254, 270, 303 | 10 |
| `kernel_call_xobject` | local | Object-table lookup result for that handle. | `src/xenia/kernel/util/shim_utils.cc` | 185, 190, 191 | 3 |
| `kernel_call_total` | local | This thread's call count including this call. | `src/xenia/kernel/util/shim_utils.cc` | 357, 391, 411 | 3 |
| `kernel_call_history_lr` | thread_local array | Guest return address of each call in kernel_call_history. | `src/xenia/kernel/util/shim_utils.cc` | 363, 377, 378, 381, 405 | 5 |
| `kernel_call_storm_ms` | thread_local | Uptime at the last millionth call on this thread. | `src/xenia/kernel/util/shim_utils.cc` | 389, 393, 394 | 3 |
| `kernel_call_storm_reports` | thread_local | How many million-call bursts under a second this thread has had (every tenth is logged). | `src/xenia/kernel/util/shim_utils.cc` | 390, 396, 417 | 3 |
| `kernel_call_storm_elapsed` | local | Milliseconds this thread took for its last million calls. | `src/xenia/kernel/util/shim_utils.cc` | 392, 395, 411 | 3 |
| `kernel_call_global` | local | The global critical region mutex, only try-locked so a report never waits on a stuck thread (wait-object lookup, guest-thread list, stack walk). | `src/xenia/kernel/util/shim_utils.cc` | 93, 94, 120, 183, 184, 193, 208, 209, 215, 279, 290, 309, 328 | 13 |
| `kernel_call_xthreads` | local | Every XThread in the object table. | `src/xenia/kernel/util/shim_utils.cc` | 213, 216 | 2 |
| `kernel_call_xthread` | local | The XThread being listed. | `src/xenia/kernel/util/shim_utils.cc` | 216, 217, 222, 280, 282, 285, 286, 287, 288, 295, 296, 301, 302, 324 | 15 |
| `kernel_call_host_pc` | local / parameter | Host program counter of a thread (in SampleKernelCallHostPc and the guest-thread list). | `src/xenia/kernel/util/shim_utils.cc` | 129, 142, 148, 224, 232, 233, 238, 264, 267, 272, 303 | 11 |
| `kernel_call_host_thread` | local / parameter | A running guest thread's host thread, to sample. | `src/xenia/kernel/util/shim_utils.cc` | 128, 132, 221, 225, 226, 249 | 6 |
| `kernel_call_port` | local | Mach thread port of the host thread (in SampleKernelCallHostPc). | `src/xenia/kernel/util/shim_utils.cc` | 131, 133, 134, 135, 139, 145 | 6 |
| `kernel_call_arm_state` | local | ARM64 register state read with thread_get_state (in SampleKernelCallHostPc). | `src/xenia/kernel/util/shim_utils.cc` | 136, 140, 143 | 3 |
| `kernel_call_state_count` | local | Size argument for thread_get_state (in SampleKernelCallHostPc). | `src/xenia/kernel/util/shim_utils.cc` | 137, 141 | 2 |
| `kernel_call_function` | local | JIT guest function containing a sampled host PC, from the code cache. | `src/xenia/kernel/util/shim_utils.cc` | 231, 236, 237, 241, 250, 255, 258 | 7 |
| `kernel_call_context` | local | That thread's guest PPC context, for lr and r1. | `src/xenia/kernel/util/shim_utils.cc` | 294, 304, 305, 306 | 5 |
| `KernelCallReadable` | function (anonymous namespace) | Whether a guest memory range is committed and readable, so the reports never read a page an exited thread released. | `src/xenia/kernel/util/shim_utils.cc` | 79, 102, 111, 280, 282, 312 | 6 |
| `kernel_call_heap` | local | Guest heap containing the address being checked. | `src/xenia/kernel/util/shim_utils.cc` | 80, 81, 84 | 3 |
| `kernel_call_symbol` | local | dladdr() result naming the host function a sampled PC is in. | `src/xenia/kernel/util/shim_utils.cc` | 266, 268, 269, 271, 272 | 5 |
| `SampleKernelCallHostPc` | function (anonymous namespace) | Reads another thread's host PC while it is suspended for a moment (Apple ARM64; 0 elsewhere). | `src/xenia/kernel/util/shim_utils.cc` | 127, 226, 249 | 3 |
| `kernel_call_code_cache` | local | The JIT code cache, to map sampled host PCs to guest functions. | `src/xenia/kernel/util/shim_utils.cc` | 229, 233, 252 | 3 |
| `kernel_call_guest_pc` | local | Guest address of the first sample for a thread running game code; 0 if it is not in game code. | `src/xenia/kernel/util/shim_utils.cc` | 235, 237, 241, 309, 311 | 5 |
| `kernel_call_sample` | local | Index of the extra samples (7, 1 ms apart). | `src/xenia/kernel/util/shim_utils.cc` | 245, 246 | 3 |
| `kernel_call_sample_pc` | local | Host PC of one extra sample. | `src/xenia/kernel/util/shim_utils.cc` | 248, 251, 252, 259 | 4 |
| `LogKernelCallThreads` | function, kernel shim | Logs every guest thread: inside or last kernel call, for how long, guest lr, call count; for a thread blocked a second or more, its wait object and guest stack. Also called when the iOS app stops a game. | `src/xenia/kernel/util/shim_utils.h` | 554 | 1 |
| `LogKernelCallThreads` | function, kernel shim | Logs every guest thread: inside or last kernel call, for how long, guest lr, call count; for a thread blocked a second or more, its wait object and guest stack. Also called when the iOS app stops a game. | `src/xenia/app/xenia_main_ios.mm` | 586 | 1 |

| `MetalPerfCounters` | struct | Performance counters filled in across the Metal backend, logged and reset with the draw stats. | `src/xenia/gpu/metal/metal_perf_counters.h` | 26, 46 | 2 |
| `gpu_us` | MetalPerfCounters field | GPU execution time of completed submission command buffers (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 28 | 1 |
| `gpu_us` | MetalPerfCounters field | GPU execution time of completed submission command buffers (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2697, 6077 | 2 |
| `gpu_command_buffers` | MetalPerfCounters field | Number of those command buffers. | `src/xenia/gpu/metal/metal_perf_counters.h` | 29 | 1 |
| `gpu_command_buffers` | MetalPerfCounters field | Number of those command buffers. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2710, 6081 | 2 |
| `cpu_wait_us` | MetalPerfCounters field | Time the CPU spent blocked in waitUntilCompleted (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 31, 60 | 2 |
| `cpu_wait_us` | MetalPerfCounters field | Time the CPU spent blocked in waitUntilCompleted (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2714 | 1 |
| `cpu_waits` | MetalPerfCounters field | Number of those waits. | `src/xenia/gpu/metal/metal_perf_counters.h` | 32, 62 | 2 |
| `cpu_waits` | MetalPerfCounters field | Number of those waits. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2713 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/metal_perf_counters.h` | 34 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/msl_shader.cc` | 1428 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2716 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 35 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/msl_shader.cc` | 1429 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2718 | 1 |
| `pipeline_compiles` | MetalPerfCounters field | Render pipeline states created. | `src/xenia/gpu/metal/metal_perf_counters.h` | 37 | 1 |
| `pipeline_compiles` | MetalPerfCounters field | Render pipeline states created. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1058, 2721, 7525 | 3 |
| `pipeline_compile_us` | MetalPerfCounters field | Time spent creating them (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 38 | 1 |
| `pipeline_compile_us` | MetalPerfCounters field | Time spent creating them (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 1059, 2723, 7526 | 3 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_perf_counters.h` | 43 | 1 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_shared_memory.cc` | 141 | 1 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2731 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_perf_counters.h` | 46, 60, 62 | 3 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 14 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 32 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/msl_shader.cc` | 33, 1428, 1429 | 3 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_shared_memory.cc` | 11, 141 | 2 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_command_processor.cc` | 48, 881, 882, 1058, 1059, 2697, 2710, 2713, 2714, 2716, 2718, 2721, 2723, 2726, 2728, 2731, 6077, 6081, 7525, 7526 | 20 |
| `MetalPerfMicrosecondsSince` | inline function | Microseconds since a steady_clock time point. | `src/xenia/gpu/metal/metal_perf_counters.h` | 48, 61 | 2 |
| `MetalPerfMicrosecondsSince` | inline function | Microseconds since a steady_clock time point. | `src/xenia/gpu/metal/metal_command_processor.cc` | 879, 1060, 1108, 1173, 7527 | 5 |
| `metal_perf_begin` | parameter / local | Start time of a timed operation. | `src/xenia/gpu/metal/metal_perf_counters.h` | 49, 51, 58, 61 | 4 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_perf_counters.h` | 56 | 1 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1379, 1538, 3127 | 3 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3330, 4332, 4888 | 3 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1949, 1981 | 2 |
| `metal_perf_command_buffer` | parameter | Command buffer waited on. | `src/xenia/gpu/metal/metal_perf_counters.h` | 57, 59 | 2 |
| `MetalPerfThreadCpu` | function (anonymous namespace) | Host CPU share of every thread since the previous call, busiest first, with name, ID and host priority (Apple only). | `src/xenia/gpu/metal/metal_command_processor.cc` | 535, 2736 | 2 |
| `metal_perf_text` | local | Text of the thread CPU report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 536, 596, 602, 608, 610 | 5 |
| `metal_perf_previous_ns` | function static map | CPU time per system thread ID at the previous report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 538, 565, 567, 589 | 4 |
| `metal_perf_current_ns` | local map | CPU time per system thread ID now. | `src/xenia/gpu/metal/metal_command_processor.cc` | 539, 563, 589 | 3 |
| `metal_perf_threads` | local vector | (CPU time in the window, label) per thread. | `src/xenia/gpu/metal/metal_command_processor.cc` | 540, 575, 590, 600, 603, 604 | 7 |
| `metal_perf_list` | local | Thread ports from task_threads. | `src/xenia/gpu/metal/metal_command_processor.cc` | 541, 543, 555, 558, 585, 587 | 6 |
| `metal_perf_count` | local | Number of threads in the process. | `src/xenia/gpu/metal/metal_command_processor.cc` | 542, 543, 549, 588, 598 | 5 |
| `metal_perf_total_ns` | local | CPU time of all threads in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 547, 571, 598 | 3 |
| `metal_perf_index` | local | Loop index over threads. | `src/xenia/gpu/metal/metal_command_processor.cc` | 548, 549, 555, 558, 585, 599, 600, 601, 603, 604 | 11 |
| `metal_perf_identifier` | local | thread_identifier_info (system thread ID). | `src/xenia/gpu/metal/metal_command_processor.cc` | 550, 556, 563, 565, 581 | 5 |
| `metal_perf_info_count` | local | Size argument for THREAD_IDENTIFIER_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 551, 557 | 2 |
| `metal_perf_extended` | local | thread_extended_info (CPU time, name, priority). | `src/xenia/gpu/metal/metal_command_processor.cc` | 552, 559, 561, 562, 573, 578, 579, 582 | 9 |
| `metal_perf_extended_count` | local | Size argument for THREAD_EXTENDED_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 553, 560 | 2 |
| `metal_perf_ns` | local | A thread's total CPU time. | `src/xenia/gpu/metal/metal_command_processor.cc` | 561, 563, 568, 569, 570 | 5 |
| `metal_perf_previous` | local iterator | Its entry from the previous report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 564, 567, 569 | 3 |
| `metal_perf_delta` | local | Its CPU time in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 566, 571, 572, 576 | 4 |
| `metal_perf_window_ns` | local | Window length in ns. | `src/xenia/gpu/metal/metal_command_processor.cc` | 594, 598, 605 | 3 |
| `metal_perf_left` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_command_processor.cc` | 591, 592 | 2 |
| `metal_perf_left` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 648, 649, 857, 859 | 4 |
| `metal_perf_right` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_command_processor.cc` | 591, 592 | 2 |
| `metal_perf_right` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 648, 650, 858, 859 | 4 |
| `metal_perf_footprint_mb` | local | Process physical footprint (task_info TASK_VM_INFO). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2680, 2687, 2734 | 3 |
| `metal_perf_vm_info` | local | TASK_VM_INFO result. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2682, 2685, 2687 | 3 |
| `metal_perf_vm_count` | local | Size argument for TASK_VM_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2683, 2686 | 2 |
| `metal_perf_available_mb` | local | os_proc_available_memory() before iOS ends the app (iOS only). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2690, 2692, 2734 | 3 |
| `metal_perf_window_us` | local | Window length: host time between the window's swaps. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2694, 2712, 2736, 2737 | 4 |
| `metal_perf_gpu_us` | local | GPU time in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2696, 2709, 2712 | 3 |
| `metal_perf_swap_time` | local | Host time of this swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2652, 2656, 2664 | 3 |
| `metal_perf_frame_us` | local | Host time since the previous swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2654, 2658, 2660, 2661, 2662 | 5 |
| `pipeline_compile_begin` | local (two functions) | Start time of a newRenderPipelineState call. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1055, 1060, 7523, 7527 | 4 |
| `completed_buffer` | lambda parameter | Completed submission command buffer, for its GPU times. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6072, 6075 | 3 |
| `metal_perf_gpu_seconds` | local | Its GPUEndTime - GPUStartTime. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6074, 6076, 6078 | 3 |
| `frame_us_total` | DrawStats field | Host time between swaps in the window (us). | `src/xenia/gpu/metal/metal_command_processor.h` | 588 | 1 |
| `frame_us_total` | DrawStats field | Host time between swaps in the window (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2658, 2695, 2707 | 3 |
| `frame_us_max` | DrawStats field | Longest frame in the window (us). | `src/xenia/gpu/metal/metal_command_processor.h` | 589 | 1 |
| `frame_us_max` | DrawStats field | Longest frame in the window (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2659, 2660, 2708 | 3 |
| `frames_over_33ms` | DrawStats field | Frames longer than 33 ms. | `src/xenia/gpu/metal/metal_command_processor.h` | 590 | 1 |
| `frames_over_33ms` | DrawStats field | Frames longer than 33 ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2661, 2708 | 2 |
| `frames_over_50ms` | DrawStats field | Frames longer than 50 ms. | `src/xenia/gpu/metal/metal_command_processor.h` | 591 | 1 |
| `frames_over_50ms` | DrawStats field | Frames longer than 50 ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2662, 2709 | 2 |
| `metal_perf_last_swap_` | MetalCommandProcessor member | Host time of the previous swap. | `src/xenia/gpu/metal/metal_command_processor.h` | 594 | 1 |
| `metal_perf_last_swap_` | MetalCommandProcessor member | Host time of the previous swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2653, 2656, 2664 | 3 |
| `unhandled_host_exception_count` | function static atomic | Unhandled host faults so far, to log only the first ten and then one in a million. | `src/xenia/base/exception_handler_mac.cc` | 224, 226 | 2 |
| `unhandled_count` | local | This fault's number. | `src/xenia/base/exception_handler_mac.cc` | 225, 228, 233 | 4 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading.h` | 130 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading_posix.cc` | 199 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading_win.cc` | 164 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/gpu/command_processor.cc` | 401 | 1 |
| `fifo_min` | function static in PosixCondition<Thread>::set_priority | Darwin SCHED_FIFO minimum priority, for the 1..32 remap. Ported from XeniOS (same name). | `src/xenia/base/threading_posix.cc` | 907, 910 | 3 |
| `fifo_max` | function static in PosixCondition<Thread>::set_priority | Darwin SCHED_FIFO maximum priority, for the 1..32 remap. Ported from XeniOS (same name). | `src/xenia/base/threading_posix.cc` | 908, 910 | 2 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 31, 83, 89, 114, 117, 119 | 6 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 224, 227, 251, 263, 264, 272, 276, 368, 391, 412, 496, 534, 569, 581 | 14 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 63, 438 | 2 |
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
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6378 | 1 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4012, 6008 | 2 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 74, 84 | 2 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 366 | 1 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4025, 4133, 4856, 5052 | 4 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1112 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 75, 86 | 2 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 389 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 5583 | 1 |
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
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1472, 2737, 5846, 6378 | 4 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4012, 4025, 4133, 4856, 5052, 5583, 6008 | 7 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1112, 1241 | 2 |
| `metal_gpu_pass_timing` | cvar (Metal); iOS settings key | Turns pass timing, the traffic estimate, per-pipeline time and compile stall tracking on (default) or off; Diagnostics section toggle. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 10, 19, 217, 221, 737, 750 | 7 |
| `metal_gpu_pass_timing` | cvar (Metal); iOS settings key | Turns pass timing, the traffic estimate, per-pipeline time and compile stall tracking on (default) or off; Diagnostics section toggle. | `src/xenia/ui/windowed_app_main_ios.mm` | 3681 | 1 |
| `metal_gpu_pass_record_` | MetalCommandProcessor member | Record of the current render encoder, for its draw count. | `src/xenia/gpu/metal/metal_command_processor.h` | 438 | 1 |
| `metal_gpu_pass_record_` | MetalCommandProcessor member | Record of the current render encoder, for its draw count. | `src/xenia/gpu/metal/metal_command_processor.cc` | 628, 1920, 1993, 5847, 6200, 6380 | 6 |
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
| `NoteDraw` | MetalGpuPassTimer member function | Counts a guest draw in its pass, per pipeline. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5846 | 1 |
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
| `Queued` | MetalCompileStallTracker member function | A compile was queued. | `src/xenia/gpu/metal/metal_command_processor.cc` | 918, 956 | 2 |
| `Skipped` | MetalCompileStallTracker member function | A draw was skipped for a pending compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 162 | 1 |
| `Skipped` | MetalCompileStallTracker member function | A draw was skipped for a pending compile. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 749 | 1 |
| `Skipped` | MetalCompileStallTracker member function | A draw was skipped for a pending compile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 4455, 4486, 4531 | 3 |
| `Finished` | MetalCompileStallTracker member function | A compile finished: buckets it and notes it if draws waited 100 ms or more. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 164, 181 | 2 |
| `Finished` | MetalCompileStallTracker member function | A compile finished: buckets it and notes it if draws waited 100 ms or more. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 762 | 1 |
| `Finished` | MetalCompileStallTracker member function | A compile finished: buckets it and notes it if draws waited 100 ms or more. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1131, 1191 | 2 |
| `Clear` | MetalCompileStallTracker member function | Forgets everything for a new title. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 167 | 1 |
| `Clear` | MetalCompileStallTracker member function | Forgets everything for a new title. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 724 | 1 |
| `Clear` | MetalCompileStallTracker member function | Forgets everything for a new title. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1473 | 1 |
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
| `compile_us` | MetalCompileStall field; Finished parameter; locals in MslShaderCompileThread | Time of the compile itself. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1108, 1136, 1168, 1173, 1197 | 5 |
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
| `metal_compile_stall_tracker` | inline global | The one MetalCompileStallTracker. | `src/xenia/gpu/metal/metal_command_processor.cc` | 918, 956, 1131, 1191, 1473, 2738, 4455, 4486, 4531 | 9 |
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
| `XE_TRANSFER_STENCIL_OUTPUT` | MSL preprocessor define (transfer shaders) | 1 when a depth-output transfer shader also writes the guest stencil through [[stencil]]. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 7022, 8196, 8513, 8551, 8609, 8780 | 6 |
| `stencil` | TransferDepthOut member (MSL) | The [[stencil]] output of a depth transfer with native stencil output (only these two lines; the word is common in the file). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 735, 739, 740, 1894, 1955, 2072, 2131, 2248, 2304, 3602, 3603, 3957, 3966, 6022, 6380, 6952, 8197, 8515, 8516, 8610, 8611, 8781, 9139, 9140, 9142, 9143, 9144, 9145, 9148, 9149, 9150, 9151, 9152, 9154, 9164, 9165, 9176, 9181, 9249, 9250, 9251, 9252, 9253, 9254, 9255, 9256, 9257, 9259, 9272, 9273, 9274, 9275, 9276, 9277, 9278, 9279, 9280, 9282, 9300, 9301, 9302, 9303, 9304, 9305, 9306, 9307, 9308, 9310 | 71 |
| `stencil_packed` | local in transfer_ps (MSL) | Gamma color packed after conversion, as the per-bit stencil draws pack it. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 8517, 8553, 8615 | 3 |
| `stencil_packed_set` | local in transfer_ps (MSL) | stencil_packed was computed (8_8_8_8_GAMMA source). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 8518, 8555, 8615 | 3 |
| `transfer_stencil` | local in transfer_ps (MSL) | Guest stencil written with the depth. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 8614, 8617, 8620, 8781 | 4 |
| `depth_stencil_output_state` | local in PerformTransfersAndResolveClears | GetTransferDepthAndStencilOutputState() for a draw writing stencil with depth. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 6382, 6384, 6387 | 3 |
| `named_candidate_count` | local in CompileToMsl | Entry point candidates before those found in the MSL source, which are only searched for when none of them resolves. | `src/xenia/gpu/metal/msl_shader.cc` | 1334, 1337 | 2 |
| `argument_encoder_begin` | local in CompileToMsl | Start of the argument encoder creation, timed for the compile timings line. | `src/xenia/gpu/metal/msl_shader.cc` | 1397, 1422 | 2 |
| `argument_encoder_ms` | local in CompileToMsl | Its time (argument-encoder= in the compile timings line). | `src/xenia/gpu/metal/msl_shader.cc` | 1421, 1439, 1448 | 3 |
| `metal_async_compile_wait_ms` | cvar (Metal); iOS settings key | How long draws may wait per frame for a compile before being skipped; 0 (default) never waits. Settings > Performance > Wait for Shader Compiles. | `src/xenia/gpu/metal/metal_command_processor.cc` | 101, 855 | 2 |
| `metal_async_compile_wait_ms` | cvar (Metal); iOS settings key | How long draws may wait per frame for a compile before being skipped; 0 (default) never waits. Settings > Performance > Wait for Shader Compiles. | `src/xenia/ui/windowed_app_main_ios.mm` | 3554 | 1 |
| `queue_arrival` | MslShaderCompileRequest and MslPipelineCompileRequest field | Queue order within a priority (first in, first out). | `src/xenia/gpu/metal/metal_command_processor.h` | 500, 520, 530, 539, 559 | 7 |
| `queue_arrival` | MslShaderCompileRequest and MslPipelineCompileRequest field | Queue order within a priority (first in, first out). | `src/xenia/gpu/metal/metal_command_processor.cc` | 915, 953 | 2 |
| `msl_compile_arrivals_` | MetalCommandProcessor member | Compiles queued so far: the next queue_arrival. | `src/xenia/gpu/metal/metal_command_processor.h` | 562 | 1 |
| `msl_compile_arrivals_` | MetalCommandProcessor member | Compiles queued so far: the next queue_arrival. | `src/xenia/gpu/metal/metal_command_processor.cc` | 915, 953 | 2 |
| `msl_compile_completions_` | MetalCommandProcessor member | Compiles finished so far, for waiting draws to see progress. | `src/xenia/gpu/metal/metal_command_processor.h` | 563 | 1 |
| `msl_compile_completions_` | MetalCommandProcessor member | Compiles finished so far, for waiting draws to see progress. | `src/xenia/gpu/metal/metal_command_processor.cc` | 869, 875, 1127, 1187 | 4 |
| `msl_compile_done_cv_` | MetalCommandProcessor member | Notified when a compile finishes (and at shutdown). | `src/xenia/gpu/metal/metal_command_processor.h` | 560, 564 | 2 |
| `msl_compile_done_cv_` | MetalCommandProcessor member | Notified when a compile finishes (and at shutdown). | `src/xenia/gpu/metal/metal_command_processor.cc` | 799, 870, 1129, 1189 | 4 |
| `msl_compile_wait_used_us_` | MetalCommandProcessor member | Time draws waited for compiles in the current frame; reset at each swap. | `src/xenia/gpu/metal/metal_command_processor.h` | 566 | 1 |
| `msl_compile_wait_used_us_` | MetalCommandProcessor member | Time draws waited for compiles in the current frame; reset at each swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 860, 873, 880, 2742 | 4 |
| `WaitForMslCompileProgress` | MetalCommandProcessor member function | Waits for any compile to finish within the frame's remaining wait budget. | `src/xenia/gpu/metal/metal_command_processor.h` | 569 | 1 |
| `WaitForMslCompileProgress` | MetalCommandProcessor member function | Waits for any compile to finish within the frame's remaining wait budget. | `src/xenia/gpu/metal/metal_command_processor.cc` | 854, 4450, 4481, 4511 | 4 |
| `compile_waits` | MetalPerfCounters field | Draws that waited for a compile. | `src/xenia/gpu/metal/metal_perf_counters.h` | 40 | 1 |
| `compile_waits` | MetalPerfCounters field | Draws that waited for a compile. | `src/xenia/gpu/metal/metal_command_processor.cc` | 881, 2726 | 2 |
| `compile_wait_us` | MetalPerfCounters field | Time they waited. | `src/xenia/gpu/metal/metal_perf_counters.h` | 41 | 1 |
| `compile_wait_us` | MetalPerfCounters field | Time they waited. | `src/xenia/gpu/metal/metal_command_processor.cc` | 882, 2728 | 2 |
| `wait_ms` | local in WaitForMslCompileProgress | metal_async_compile_wait_ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 855, 856, 859 | 3 |
| `budget_us` | local in WaitForMslCompileProgress | The per-frame budget in us. | `src/xenia/gpu/metal/metal_command_processor.cc` | 859, 860, 873 | 3 |
| `wait_begin` | local in WaitForMslCompileProgress | Start of the wait. | `src/xenia/gpu/metal/metal_command_processor.cc` | 863, 872, 879 | 3 |
| `completions` | local in WaitForMslCompileProgress | msl_compile_completions_ when the wait started. | `src/xenia/gpu/metal/metal_command_processor.cc` | 869, 875 | 2 |
| `waited_us` | local in WaitForMslCompileProgress | Time waited. | `src/xenia/gpu/metal/metal_command_processor.cc` | 879, 880, 882 | 3 |
| `metal_render_pass_area_from_scissor` | cvar (Metal); iOS settings key | Limit guest draw render passes to the area their scissors cover (default off). Settings > Performance > Limit Render Pass Area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 93, 3121, 3125 | 3 |
| `metal_render_pass_area_from_scissor` | cvar (Metal); iOS settings key | Limit guest draw render passes to the area their scissors cover (default off). Settings > Performance > Limit Render Pass Area. | `src/xenia/ui/windowed_app_main_ios.mm` | 3570 | 1 |
| `render_area_needed_width_` | MetalCommandProcessor member | Right edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 444 | 1 |
| `render_area_needed_width_` | MetalCommandProcessor member | Right edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3123, 3132, 6322, 6335, 6360 | 5 |
| `render_area_needed_height_` | MetalCommandProcessor member | Bottom edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 445 | 1 |
| `render_area_needed_height_` | MetalCommandProcessor member | Bottom edge of the current draw's scissor (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3124, 3135, 6323, 6335, 6362 | 5 |
| `current_render_area_width_` | MetalCommandProcessor member | Render area width of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 446 | 1 |
| `current_render_area_width_` | MetalCommandProcessor member | Render area width of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6201, 6321, 6322, 6386, 6408, 6409 | 6 |
| `current_render_area_height_` | MetalCommandProcessor member | Render area height of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.h` | 447 | 1 |
| `current_render_area_height_` | MetalCommandProcessor member | Render area height of the current render encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6202, 6323, 6387, 6410 | 4 |
| `render_area_by_descriptor_` | MetalCommandProcessor member | Largest render area so far per render pass descriptor, so passes stop growing. | `src/xenia/gpu/metal/metal_command_processor.h` | 449 | 1 |
| `render_area_by_descriptor_` | MetalCommandProcessor member | Largest render area so far per render pass descriptor, so passes stop growing. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6358 | 1 |
| `area_scissor` | local in IssueDraw | The draw's scissor. | `src/xenia/gpu/metal/metal_command_processor.cc` | 3126, 3127, 3133, 3136 | 6 |
| `area_scale_x` | local in IssueDraw | Draw resolution scale (x). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3128, 3133 | 2 |
| `area_scale_y` | local in IssueDraw | Draw resolution scale (y). | `src/xenia/gpu/metal/metal_command_processor.cc` | 3130, 3136 | 2 |
| `area_width` | local in BeginCommandBuffer | Render area width for the new encoder (0: whole target). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6333, 6363, 6365, 6367, 6372, 6373, 6381, 6386 | 8 |
| `area_height` | local in BeginCommandBuffer | Render area height for the new encoder. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6334, 6364, 6366, 6368, 6374, 6387 | 6 |
| `attachments_width` | local in BeginCommandBuffer | Smallest attachment width, the most the area may be. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6336, 6346, 6347, 6357, 6363, 6365 | 6 |
| `attachments_height` | local in BeginCommandBuffer | Smallest attachment height. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6337, 6348, 6349, 6364, 6366 | 5 |
| `clears_on_load` | local in BeginCommandBuffer | An attachment is cleared on load: keep the whole area. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6338, 6350, 6357 | 3 |
| `limit_to_attachment` | lambda in BeginCommandBuffer | Takes an attachment's size and load action into account. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6339, 6353, 6355, 6356 | 4 |
| `known_area` | local in BeginCommandBuffer | render_area_by_descriptor_ entry. | `src/xenia/gpu/metal/metal_command_processor.cc` | 6358, 6359, 6361, 6362, 6363, 6364 | 7 |
| `pass_width` | local in MetalGpuPassTimer::RenderEncoder | Render area width, or the attachment's, for the traffic estimate. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 290, 293, 294, 299, 311, 325 | 6 |
| `pass_height` | local in MetalGpuPassTimer::RenderEncoder | Render area height, or the attachment's. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 291, 295, 296, 300, 311, 327 | 6 |
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
| `draw_stats_` | 40 |
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
| `LogKernelCallStack` | 5 |
| `guest_stack_pointer` | 3 |
| `guest_arg0` | 3 |
| `guest_arg1` | 3 |
| `kernel_call_ms` | 3 |
| `kernel_call_handle` | 3 |
| `kernel_call_object` | 10 |
| `kernel_call_xobject` | 3 |
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
| `KernelCallReadable` | 6 |
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
| `metal_perf_counters` | 30 |
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
| `width` | 11 |
| `height` | 20 |
| `samples` | 5 |
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
| `stencil` | 78 |
| `load_action` | 3 |
| `store_action` | 6 |
| `pixel_bytes` | 3 |
| `attachment_bytes` | 3 |
| `resolve_texture` | 4 |
| `counter_text` | 5 |
| `pipeline_vertex_ticks` | 3 |
| `pipeline_fragment_ticks` | 3 |
| `source` | 6 |
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
| `metal_async_compile_wait_ms` | 3 |
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
| `metal_render_pass_area_from_scissor` | 4 |
| `render_area_needed_width_` | 6 |
| `render_area_needed_height_` | 6 |
| `current_render_area_width_` | 7 |
| `current_render_area_height_` | 5 |
| `render_area_by_descriptor_` | 2 |
| `area_scissor` | 6 |
| `area_scale_x` | 2 |
| `area_scale_y` | 2 |
| `area_width` | 8 |
| `area_height` | 6 |
| `attachments_width` | 6 |
| `attachments_height` | 5 |
| `clears_on_load` | 3 |
| `limit_to_attachment` | 4 |
| `known_area` | 7 |
| `pass_width` | 6 |
| `pass_height` | 6 |

## Checks

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
- The names in `tools/ci/run_cpu_tests.py` are local to that script; `run`,
  `tail` and `main` are common names elsewhere in the tree but are not
  shared with any other file.
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
