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
| `now` | local in GetLogFilePath | Current time used to name the log file. | `src/xenia/ui/windowed_app_main_ios.mm` | 609, 1264, 1266, 5663, 10755, 10758, 10802 | 7 |
| `local_time` | local in GetLogFilePath | now broken down in the device's time zone. | `src/xenia/ui/windowed_app_main_ios.mm` | 1265, 1266, 1269 | 3 |
| `log_name` | local in GetLogFilePath | File name xenia_<date>_<time>.log. | `src/xenia/ui/windowed_app_main_ios.mm` | 1267, 1268, 1270 | 4 |
| `kText` | enumerator of IOSConfigControlType | Settings row whose value is edited as text. | `src/xenia/ui/windowed_app_main_ios.mm` | 543, 1230, 3719, 8061, 8114 | 5 |
| `AddAllConfigVarSettings` | function | Adds a settings section per config category with every variable not already shown. | `src/xenia/ui/windowed_app_main_ios.mm` | 1204, 3670 | 2 |
| `existing_keys` | local in AddAllConfigVarSettings | Config keys the curated sections already expose, so they are not listed twice. | `src/xenia/ui/windowed_app_main_ios.mm` | 1208, 1212, 1218 | 3 |
| `by_category` | local in AddAllConfigVarSettings | Generated sections keyed by config category. | `src/xenia/ui/windowed_app_main_ios.mm` | 1216, 1234, 1238 | 3 |
| `parsed` | local in ApplyIOSConfigSections | The edited text parsed as a TOML value. | `src/xenia/ui/windowed_app_main_ios.mm` | 805, 806, 809, 949, 953, 2506, 2507, 2508, 3088, 3091, 3095, 3096, 3120, 3121, 3733, 3735, 3738, 3745 | 20 |
| `editor` | local in tableView:didSelectRowAtIndexPath: | Alert with a text field used to edit a kText row. | `src/xenia/ui/windowed_app_main_ios.mm` | 8118, 8122, 8128, 8131, 8134, 8144 | 6 |
| `DrawStats` | struct in MetalCommandProcessor | Counters describing what happened to guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 545, 560 | 2 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.h` | 560 | 1 |
| `draw_stats_` | member of MetalCommandProcessor | Current DrawStats; logged and reset every kDrawStatsFrames frames. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2573, 2574, 2575, 2576, 2577, 2580, 2587, 2588, 2589, 2590, 2591, 2592, 2593, 2610, 2620, 2621, 2622, 2623, 2646, 2647, 2911, 2913, 2949, 2974, 4339, 4363, 4398, 5704, 5705 | 38 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.h` | 542, 544 | 2 |
| `kDrawStatsFrames` | constant in MetalCommandProcessor | Number of presented frames between draw-stat log lines. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2580 | 1 |
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
| `SetTouchState` | function | Stores the on-screen controller state; called by the UI, read by the driver. | `src/xenia/ui/windowed_app_main_ios.mm` | 8218, 8374 | 2 |
| `TouchInputDriver` | class (anonymous namespace) | Input driver presenting the on-screen controller as a fallback gamepad. | `src/xenia/hid/touch_input_driver_ios.cc` | 29, 31, 182 | 3 |
| `touch_state_mutex_` | file-scope variable | Guards `touch_state_`. | `src/xenia/hid/touch_input_driver_ios.cc` | 24, 25, 66, 107, 162 | 5 |
| `touch_state_` | file-scope variable | Latest on-screen controller state and packet number. | `src/xenia/hid/touch_input_driver_ios.cc` | 27, 67, 108, 109, 113, 163, 177 | 8 |
| `kKeystrokeKeys` | local constant in GetKeystroke | Virtual key for each button bit. | `src/xenia/hid/touch_input_driver_ios.cc` | 85, 120, 123, 129, 153 | 5 |
| `keystroke_buttons` | local in GetKeystroke | Current buttons and triggers as a bit field. | `src/xenia/hid/touch_input_driver_ios.cc` | 105, 108, 111, 115, 122, 124 | 6 |
| `release_pass` | local in GetKeystroke | Loop counter: releases are reported before presses. | `src/xenia/hid/touch_input_driver_ios.cc` | 119, 125 | 3 |
| `keystroke_bit` | local in GetKeystroke | Bit of the button being examined. | `src/xenia/hid/touch_input_driver_ios.cc` | 121, 122, 124, 128 | 4 |
| `pressed` | local in GetKeystroke | Whether that button is currently down. | `src/xenia/hid/touch_input_driver_ios.cc` | 122, 125, 132 | 3 |
| `last_keystroke_buttons_` | member of TouchInputDriver | Buttons already reported through GetKeystroke. | `src/xenia/hid/touch_input_driver_ios.cc` | 124, 128, 154 | 3 |
| `XeniaTouchPadView` | Objective-C class | The on-screen controller view. | `src/xenia/ui/windowed_app_main_ios.mm` | 8216, 8238, 8245, 8485, 8584 | 5 |
| `XeniaTouchPadControl` | struct | One control of the on-screen controller: kind, button bit, label, centre, radius. | `src/xenia/ui/windowed_app_main_ios.mm` | 8228, 8246, 8279, 8424 | 4 |
| `XeniaTouchPadControlKind` | enum | Kind of control. | `src/xenia/ui/windowed_app_main_ios.mm` | 8220, 8229 | 2 |
| `XeniaTouchPadControlKindButton` | enumerator | A digital button. | `src/xenia/ui/windowed_app_main_ios.mm` | 8221, 8284, 8286, 8288, 8290, 8291, 8293, 8295, 8297, 8299, 8301, 8303, 8305, 8307, 8309, 8311 | 16 |
| `XeniaTouchPadControlKindLeftTrigger` | enumerator | Left trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8222, 8282, 8366 | 3 |
| `XeniaTouchPadControlKindRightTrigger` | enumerator | Right trigger. | `src/xenia/ui/windowed_app_main_ios.mm` | 8223, 8283, 8368 | 3 |
| `XeniaTouchPadControlKindLeftStick` | enumerator | Left thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8224, 8280, 8345, 8356, 8429, 8457 | 6 |
| `XeniaTouchPadControlKindRightStick` | enumerator | Right thumbstick. | `src/xenia/ui/windowed_app_main_ios.mm` | 8225, 8281, 8346, 8430, 8458 | 5 |
| `kXeniaTouchPadControlCount` | file-scope constant | Number of controls (19). | `src/xenia/ui/windowed_app_main_ios.mm` | 8236, 8246, 8279, 8314, 8321, 8423 | 6 |
| `controls_` | instance variable of XeniaTouchPadView | The controls with their current positions. | `src/xenia/ui/windowed_app_main_ios.mm` | 8246, 8315, 8322, 8323, 8325, 8345, 8346, 8348, 8349, 8356, 8366, 8368, 8371, 8424 | 16 |
| `pad_touches_` | instance variable of XeniaTouchPadView | Touches currently on the pad. | `src/xenia/ui/windowed_app_main_ios.mm` | 8248, 8259, 8266, 8342, 8343, 8380, 8383, 8392, 8405, 8407, 8427, 8428 | 12 |
| `pad_touch_controls_` | instance variable of XeniaTouchPadView | Control each of those touches started on. | `src/xenia/ui/windowed_app_main_ios.mm` | 8249, 8260, 8267, 8344, 8384, 8393, 8408, 8431 | 8 |
| `controlIndexAtPoint` | method of XeniaTouchPadView | Index of the control at a point, or -1. | `src/xenia/ui/windowed_app_main_ios.mm` | 8240, 8320, 8335, 8362, 8390, 8432, 9994 | 7 |
| `resetTouches` | method of XeniaTouchPadView | Releases every control and reports the idle state. | `src/xenia/ui/windowed_app_main_ios.mm` | 8242, 8379, 9382 | 3 |
| `reportState` | method of XeniaTouchPadView | Computes the gamepad state from the touches and passes it to SetTouchState. | `src/xenia/ui/windowed_app_main_ios.mm` | 8338, 8385, 8396, 8400, 8411 | 5 |
| `touchPadView` | property of XeniaViewController | The on-screen controller view instance. | `src/xenia/ui/windowed_app_main_ios.mm` | 8485, 8584, 8585, 8587, 8588, 9379, 9380, 9382, 9993, 9994 | 11 |
| `hide_touch_pad` | local in pollControllerNavigation | Whether the on-screen controller should be hidden now. | `src/xenia/ui/windowed_app_main_ios.mm` | 9376, 9379, 9380, 9381 | 4 |
| `width` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 444, 459, 2777, 2779, 2785, 2788, 5071, 5083, 5084, 5100, 5163, 5256, 5257, 5258, 5260, 5264, 5292, 5294, 5331, 5332, 5333, 5335, 5336, 5339, 5340, 7166, 7167, 7170, 7172, 7755, 7756, 7757, 7759, 7760, 7762, 7763, 7765, 7769, 7774, 7782, 8273, 8277, 8278, 8469, 8899, 8903, 8942, 11410, 11787 | 53 |
| `height` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 445, 460, 2783, 2786, 2788, 4948, 5084, 5101, 5102, 5165, 5264, 5292, 5294, 5339, 5340, 5531, 7166, 7167, 7168, 7170, 7172, 7601, 7608, 7774, 7777, 7785, 8274, 8280, 8281, 8294, 8296, 8298, 8300, 8302, 8304, 8306, 8308, 8310, 8312, 8470, 8899, 11789 | 44 |
| `insets` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 5274, 5637, 5642, 5643, 5644, 6776, 6779, 7346, 7351, 7354, 7357, 8275, 8276, 8277 | 15 |
| `left` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 444, 457, 459, 4670, 5080, 6779, 7354, 7870, 7872, 8276, 8280, 8282, 8285, 8294, 8296, 8298, 8300, 8310, 8773, 8779, 8787, 9459, 9551, 9671 | 25 |
| `right` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 444, 459, 5086, 6779, 7357, 7870, 7872, 8277, 8281, 8283, 8287, 8302, 8304, 8306, 8308, 8312, 8767, 8780, 9460, 9551, 9671, 9782, 9929 | 24 |
| `middle` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8278, 8289, 8290, 8292 | 4 |
| `controls` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8279, 8315, 8333 | 3 |
| `reach` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8325, 8326 | 3 |
| `dx` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8322, 8326 | 3 |
| `dy` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8323, 8326 | 3 |
| `buttons` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8230, 8339, 8371, 8374, 9450, 9451, 9453, 9480 | 9 |
| `triggers` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8340, 8361, 8367, 8369, 8374 | 6 |
| `axes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8341, 8357, 8358, 8374, 8375 | 7 |
| `point` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8080, 8081, 8239, 8240, 8320, 8322, 8323, 8334, 8335, 8343, 8348, 8349, 8362, 8428, 8432, 8435, 8436, 10873, 11303, 11305, 11952 | 22 |
| `index` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 134, 135, 136, 137, 3317, 3320, 3324, 3326, 3336, 3343, 3349, 3353, 3369, 3380, 3711, 8344, 8345, 8346, 8348, 8349, 8356, 8362, 8363, 8366, 8368, 8371, 8390, 8391, 8393, 8724, 8726, 8728, 8729, 8732, 8733, 8737 | 40 |
| `x` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 457, 1331, 3972, 3973, 3987, 5154, 5171, 5312, 5315, 8322, 8348, 8350, 8352, 8357, 8435, 8437, 8439, 8442, 8469, 11786 | 30 |
| `y` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 458, 3969, 3971, 3974, 3975, 3976, 3977, 5102, 5145, 5179, 5280, 5297, 7588, 7613, 8323, 8349, 8350, 8353, 8358, 8436, 8437, 8440, 8442, 8470 | 34 |
| `length` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 516, 642, 646, 659, 673, 686, 705, 713, 767, 774, 796, 960, 970, 980, 987, 994, 1001, 1004, 1018, 1031, 1103, 1309, 1502, 1512, 1726, 1821, 1841, 1844, 1847, 1851, 1857, 1875, 1876, 1877, 1911, 1922, 1925, 2001, 2023, 2143, 2149, 2156, 2215, 2218, 2227, 2232, 2236, 2240, 2254, 2255, 2260, 2261, 2266, 2269, 2274, 2279, 2283, 2286, 2290, 2304, 2306, 2316, 2340, 2341, 2346, 2349, 2355, 2358, 2361, 2364, 2382, 2383, 2385, 2386, 2388, 2389, 2391, 2392, 2407, 2422, 2488, 2505, 2553, 2554, 2555, 2556, 2557, 2558, 2561, 2562, 2563, 2564, 2565, 2574, 2582, 2583, 2605, 2626, 2648, 2818, 2888, 2937, 2953, 3025, 3064, 3084, 3101, 3110, 3117, 3154, 3168, 3205, 3232, 3238, 3267, 3278, 3335, 3374, 3384, 3398, 4285, 4512, 4661, 5208, 5214, 5225, 5241, 5795, 5817, 5855, 5878, 5913, 6013, 6019, 6099, 6101, 6107, 6111, 6118, 6126, 6133, 6139, 6323, 6325, 6331, 6337, 6341, 6346, 6360, 6388, 6403, 6761, 6944, 6984, 7002, 7054, 7082, 7085, 7094, 7102, 7243, 7360, 7668, 7848, 8350, 8351, 8352, 8353, 8437, 8438, 8439, 8440, 9531, 10149, 10195, 10276, 10377, 10584, 10655, 10663, 10844, 10851, 10863, 10892, 10893, 10921, 10948, 10949, 10980, 11005, 11032, 11131, 11257 | 201 |
| `axis` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 5435, 5456, 6007, 6172, 6177, 6183, 6487, 6835, 6850, 7880, 8355, 8357, 8358, 9702 | 14 |
| `position` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 5531, 8405, 8406, 8407, 8408 | 5 |
| `attributes` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 2910, 2984, 8419, 8468, 8471, 9567, 9569, 11355, 11358, 11361 | 10 |
| `control` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 542, 3559, 8239, 8241, 8247, 8324, 8424, 8426, 8429, 8430, 8435, 8436, 8438, 8439, 8440, 8442, 8447, 8448, 8457, 8458, 8461, 8468, 8469 | 24 |
| `active` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 154, 158, 510, 511, 5428, 6465, 6468, 6471, 6881, 8425, 8433, 8452, 9608, 10065, 10084, 10759 | 17 |
| `knob` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8426, 8442, 8460, 8469, 8470 | 5 |
| `is_stick` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8429, 8431, 8434 | 3 |
| `outline` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8446, 8453, 8455, 8456 | 4 |
| `knob_path` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8459, 8466 | 2 |
| `label_size` | local in a XeniaTouchPadView method | Working value for layout, hit-testing, state or drawing. | `src/xenia/ui/windowed_app_main_ios.mm` | 8468, 8469, 8470 | 3 |
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
| `kXeniaAutoEnableTrollStoreJITOnLaunchPreferenceKey` | static NSString constant, iOS UI | User-defaults key of the "Auto-Enable JIT via TrollStore" toggle (stored as ios_auto_enable_jit_via_trollstore). | `src/xenia/ui/windowed_app_main_ios.mm` | 594, 3640, 10739, 10769 | 4 |
| `jit_provider_name` | local in requestAutomaticStikDebugJITHandoffForPendingLaunchPath: | "TrollStore" or "StikDebug", for the status label and log lines. | `src/xenia/ui/windowed_app_main_ios.mm` | 10767, 10774, 10806, 10807, 10817, 10819 | 6 |
| `trollstore_url` | local in requestAutomaticStikDebugJITHandoffForPendingLaunchPath: | apple-magnifier://enable-jit?bundle-id=... request for TrollStore. | `src/xenia/ui/windowed_app_main_ios.mm` | 10770, 10772, 10773 | 4 |
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

| `MetalPerfCounters` | struct | Performance counters filled in across the Metal backend, logged and reset with the draw stats. | `src/xenia/gpu/metal/metal_perf_counters.h` | 26, 43 | 2 |
| `gpu_us` | MetalPerfCounters field | GPU execution time of completed submission command buffers (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 28 | 1 |
| `gpu_us` | MetalPerfCounters field | GPU execution time of completed submission command buffers (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2612, 5940 | 2 |
| `gpu_command_buffers` | MetalPerfCounters field | Number of those command buffers. | `src/xenia/gpu/metal/metal_perf_counters.h` | 29 | 1 |
| `gpu_command_buffers` | MetalPerfCounters field | Number of those command buffers. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2624, 5944 | 2 |
| `cpu_wait_us` | MetalPerfCounters field | Time the CPU spent blocked in waitUntilCompleted (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 31, 57 | 2 |
| `cpu_wait_us` | MetalPerfCounters field | Time the CPU spent blocked in waitUntilCompleted (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2628 | 1 |
| `cpu_waits` | MetalPerfCounters field | Number of those waits. | `src/xenia/gpu/metal/metal_perf_counters.h` | 32, 59 | 2 |
| `cpu_waits` | MetalPerfCounters field | Number of those waits. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2627 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/metal_perf_counters.h` | 34 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/msl_shader.cc` | 1411 | 1 |
| `shader_compiles` | MetalPerfCounters field | Shaders translated and compiled (MslShader). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2630 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 35 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/msl_shader.cc` | 1412 | 1 |
| `shader_compile_us` | MetalPerfCounters field | Time spent on them (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2632 | 1 |
| `pipeline_compiles` | MetalPerfCounters field | Render pipeline states created. | `src/xenia/gpu/metal/metal_perf_counters.h` | 37 | 1 |
| `pipeline_compiles` | MetalPerfCounters field | Render pipeline states created. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1001, 2635, 7323 | 3 |
| `pipeline_compile_us` | MetalPerfCounters field | Time spent creating them (us). | `src/xenia/gpu/metal/metal_perf_counters.h` | 38 | 1 |
| `pipeline_compile_us` | MetalPerfCounters field | Time spent creating them (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 1002, 2637, 7324 | 3 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_perf_counters.h` | 40 | 1 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_shared_memory.cc` | 141 | 1 |
| `upload_bytes` | MetalPerfCounters field | Guest memory copied into the shared memory buffer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2640 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_perf_counters.h` | 43, 57, 59 | 3 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 14 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 32 | 1 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/msl_shader.cc` | 33, 1411, 1412 | 3 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_shared_memory.cc` | 11, 141 | 2 |
| `metal_perf_counters` | inline global | The one MetalPerfCounters instance. | `src/xenia/gpu/metal/metal_command_processor.cc` | 48, 1001, 1002, 2612, 2624, 2627, 2628, 2630, 2632, 2635, 2637, 2640, 5940, 5944, 7323, 7324 | 16 |
| `MetalPerfMicrosecondsSince` | inline function | Microseconds since a steady_clock time point. | `src/xenia/gpu/metal/metal_perf_counters.h` | 45, 58 | 2 |
| `MetalPerfMicrosecondsSince` | inline function | Microseconds since a steady_clock time point. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1003, 7325 | 2 |
| `metal_perf_begin` | parameter / local | Start time of a timed operation. | `src/xenia/gpu/metal/metal_perf_counters.h` | 46, 48, 55, 58 | 4 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_perf_counters.h` | 53 | 1 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1379, 1538, 3127 | 3 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3317, 4319, 4875 | 3 |
| `WaitUntilCompletedCounted` | inline function | waitUntilCompleted that adds the blocked time to cpu_wait_us; replaces the direct calls in the Metal GPU code. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1864, 1896 | 2 |
| `metal_perf_command_buffer` | parameter | Command buffer waited on. | `src/xenia/gpu/metal/metal_perf_counters.h` | 54, 56 | 2 |
| `MetalPerfThreadCpu` | function (anonymous namespace) | Host CPU share of every thread since the previous call, busiest first, with name, ID and host priority (Apple only). | `src/xenia/gpu/metal/metal_command_processor.cc` | 520, 2645 | 2 |
| `metal_perf_text` | local | Text of the thread CPU report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 521, 581, 587, 593, 595 | 5 |
| `metal_perf_previous_ns` | function static map | CPU time per system thread ID at the previous report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 523, 550, 552, 574 | 4 |
| `metal_perf_current_ns` | local map | CPU time per system thread ID now. | `src/xenia/gpu/metal/metal_command_processor.cc` | 524, 548, 574 | 3 |
| `metal_perf_threads` | local vector | (CPU time in the window, label) per thread. | `src/xenia/gpu/metal/metal_command_processor.cc` | 525, 560, 575, 585, 588, 589 | 7 |
| `metal_perf_list` | local | Thread ports from task_threads. | `src/xenia/gpu/metal/metal_command_processor.cc` | 526, 528, 540, 543, 570, 572 | 6 |
| `metal_perf_count` | local | Number of threads in the process. | `src/xenia/gpu/metal/metal_command_processor.cc` | 527, 528, 534, 573, 583 | 5 |
| `metal_perf_total_ns` | local | CPU time of all threads in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 532, 556, 583 | 3 |
| `metal_perf_index` | local | Loop index over threads. | `src/xenia/gpu/metal/metal_command_processor.cc` | 533, 534, 540, 543, 570, 584, 585, 586, 588, 589 | 11 |
| `metal_perf_identifier` | local | thread_identifier_info (system thread ID). | `src/xenia/gpu/metal/metal_command_processor.cc` | 535, 541, 548, 550, 566 | 5 |
| `metal_perf_info_count` | local | Size argument for THREAD_IDENTIFIER_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 536, 542 | 2 |
| `metal_perf_extended` | local | thread_extended_info (CPU time, name, priority). | `src/xenia/gpu/metal/metal_command_processor.cc` | 537, 544, 546, 547, 558, 563, 564, 567 | 9 |
| `metal_perf_extended_count` | local | Size argument for THREAD_EXTENDED_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 538, 545 | 2 |
| `metal_perf_ns` | local | A thread's total CPU time. | `src/xenia/gpu/metal/metal_command_processor.cc` | 546, 548, 553, 554, 555 | 5 |
| `metal_perf_previous` | local iterator | Its entry from the previous report. | `src/xenia/gpu/metal/metal_command_processor.cc` | 549, 552, 554 | 3 |
| `metal_perf_delta` | local | Its CPU time in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 551, 556, 557, 561 | 4 |
| `metal_perf_window_ns` | local | Window length in ns. | `src/xenia/gpu/metal/metal_command_processor.cc` | 579, 583, 590 | 3 |
| `metal_perf_left` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_command_processor.cc` | 576, 577 | 2 |
| `metal_perf_left` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 437, 438 | 2 |
| `metal_perf_right` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_command_processor.cc` | 576, 577 | 2 |
| `metal_perf_right` | lambda parameter | Sort comparison operand. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 437, 439 | 2 |
| `metal_perf_footprint_mb` | local | Process physical footprint (task_info TASK_VM_INFO). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2595, 2602, 2643 | 3 |
| `metal_perf_vm_info` | local | TASK_VM_INFO result. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2597, 2600, 2602 | 3 |
| `metal_perf_vm_count` | local | Size argument for TASK_VM_INFO. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2598, 2601 | 2 |
| `metal_perf_available_mb` | local | os_proc_available_memory() before iOS ends the app (iOS only). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2605, 2607, 2643 | 3 |
| `metal_perf_window_us` | local | Window length: host time between the window's swaps. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2609, 2626, 2645 | 3 |
| `metal_perf_gpu_us` | local | GPU time in the window. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2611, 2623, 2626 | 3 |
| `metal_perf_swap_time` | local | Host time of this swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2567, 2571, 2579 | 3 |
| `metal_perf_frame_us` | local | Host time since the previous swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2569, 2573, 2575, 2576, 2577 | 5 |
| `pipeline_compile_begin` | local (two functions) | Start time of a newRenderPipelineState call. | `src/xenia/gpu/metal/metal_command_processor.cc` | 998, 1003, 7321, 7325 | 4 |
| `completed_buffer` | lambda parameter | Completed submission command buffer, for its GPU times. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5935, 5938 | 3 |
| `metal_perf_gpu_seconds` | local | Its GPUEndTime - GPUStartTime. | `src/xenia/gpu/metal/metal_command_processor.cc` | 5937, 5939, 5941 | 3 |
| `frame_us_total` | DrawStats field | Host time between swaps in the window (us). | `src/xenia/gpu/metal/metal_command_processor.h` | 555 | 1 |
| `frame_us_total` | DrawStats field | Host time between swaps in the window (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2573, 2610, 2621 | 3 |
| `frame_us_max` | DrawStats field | Longest frame in the window (us). | `src/xenia/gpu/metal/metal_command_processor.h` | 556 | 1 |
| `frame_us_max` | DrawStats field | Longest frame in the window (us). | `src/xenia/gpu/metal/metal_command_processor.cc` | 2574, 2575, 2622 | 3 |
| `frames_over_33ms` | DrawStats field | Frames longer than 33 ms. | `src/xenia/gpu/metal/metal_command_processor.h` | 557 | 1 |
| `frames_over_33ms` | DrawStats field | Frames longer than 33 ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2576, 2622 | 2 |
| `frames_over_50ms` | DrawStats field | Frames longer than 50 ms. | `src/xenia/gpu/metal/metal_command_processor.h` | 558 | 1 |
| `frames_over_50ms` | DrawStats field | Frames longer than 50 ms. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2577, 2623 | 2 |
| `metal_perf_last_swap_` | MetalCommandProcessor member | Host time of the previous swap. | `src/xenia/gpu/metal/metal_command_processor.h` | 561 | 1 |
| `metal_perf_last_swap_` | MetalCommandProcessor member | Host time of the previous swap. | `src/xenia/gpu/metal/metal_command_processor.cc` | 2568, 2571, 2579 | 3 |
| `unhandled_host_exception_count` | function static atomic | Unhandled host faults so far, to log only the first ten and then one in a million. | `src/xenia/base/exception_handler_mac.cc` | 224, 226 | 2 |
| `unhandled_count` | local | This fault's number. | `src/xenia/base/exception_handler_mac.cc` | 225, 228, 233 | 4 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading.h` | 130 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading_posix.cc` | 199 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/base/threading_win.cc` | 164 | 1 |
| `PreferPerformanceCores` | function (xe::threading) | Asks iOS to run the current thread on the performance cores (QOS_CLASS_USER_INTERACTIVE); false elsewhere. Ported from XeniOS 77cc1358 (same name there). | `src/xenia/gpu/command_processor.cc` | 401 | 1 |
| `fifo_min` | function static in PosixCondition<Thread>::set_priority | Darwin SCHED_FIFO minimum priority, for the 1..32 remap. Ported from XeniOS (same name). | `src/xenia/base/threading_posix.cc` | 907, 910 | 3 |
| `fifo_max` | function static in PosixCondition<Thread>::set_priority | Darwin SCHED_FIFO maximum priority, for the 1..32 remap. Ported from XeniOS (same name). | `src/xenia/base/threading_posix.cc` | 908, 910 | 2 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 29, 64, 83, 86, 88 | 5 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 141, 144, 168, 180, 181, 189, 193, 251, 274, 295, 389 | 11 |
| `MetalGpuPassRecord` | struct | One timed pass: its timestamp slots, what it was, and its guest draws. | `src/xenia/gpu/metal/metal_command_processor.h` | 63, 438 | 2 |
| `sample_buffer` | MetalGpuPassRecord field; local in Initialize | Counter sample buffer holding the pass timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 30 | 1 |
| `sample_buffer` | MetalGpuPassRecord field; local in Initialize | Counter sample buffer holding the pass timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 89, 91, 94, 101, 102, 169, 225, 260, 283, 301, 393 | 11 |
| `sample_buffer_index` | MetalGpuPassRecord field | Index of that buffer in sample_buffers_, to free its slots. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 31 | 1 |
| `sample_buffer_index` | MetalGpuPassRecord field | Index of that buffer in sample_buffers_, to free its slots. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 170, 392, 393, 394 | 4 |
| `first_sample` | MetalGpuPassRecord field | First of the pass timestamp slots. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 32 | 1 |
| `first_sample` | MetalGpuPassRecord field | First of the pass timestamp slots. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 171, 226, 227, 228, 229, 261, 262, 284, 285, 302 | 10 |
| `sample_count` | MetalGpuPassRecord field; AllocateSamples parameter | Timestamps per pass: 4 for render passes (vertex and fragment start and end), 2 otherwise. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 34, 84 | 2 |
| `sample_count` | MetalGpuPassRecord field; AllocateSamples parameter | Timestamps per pass: 4 for render passes (vertex and fragment start and end), 2 otherwise. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 142, 150, 172, 174, 302, 303, 307, 326, 375 | 10 |
| `label` | MetalGpuPassRecord field; encoder helper parameter | Static string naming what the pass is for; null discards the pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 36, 64, 66, 68, 84 | 5 |
| `label` | MetalGpuPassRecord field; encoder helper parameter | Static string naming what the pass is for; null discards the pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 143, 173, 189, 194, 240, 250, 252, 267, 273, 275, 290, 300, 314, 374, 380, 385 | 17 |
| `width` | MetalGpuPassRecord field | Width of the first attachment (counts include texture->width()). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 38 | 1 |
| `width` | MetalGpuPassRecord field | Width of the first attachment (counts include texture->width()). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 206, 207, 380 | 4 |
| `height` | MetalGpuPassRecord field | Height of the first attachment (counts include texture->height()). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 39 | 1 |
| `height` | MetalGpuPassRecord field | Height of the first attachment (counts include texture->height()). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 208, 380 | 3 |
| `samples` | MetalGpuPassRecord field | Sample count of the first attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 40 | 1 |
| `samples` | MetalGpuPassRecord field | Sample count of the first attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 209, 380 | 2 |
| `color_formats` | MetalGpuPassRecord field | MTL::PixelFormat of color attachments 0-3. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 41 | 1 |
| `color_formats` | MetalGpuPassRecord field | MTL::PixelFormat of color attachments 0-3. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 218, 381, 382 | 5 |
| `depth_format` | MetalGpuPassRecord field | MTL::PixelFormat of the depth attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 42 | 1 |
| `depth_format` | MetalGpuPassRecord field | MTL::PixelFormat of the depth attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 221, 383 | 2 |
| `load_actions` | MetalGpuPassRecord field | Load actions, 2 bits per attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 45 | 1 |
| `load_actions` | MetalGpuPassRecord field | Load actions, 2 bits per attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 211, 383 | 2 |
| `store_actions` | MetalGpuPassRecord field | Store actions, 2 bits per attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 46 | 1 |
| `store_actions` | MetalGpuPassRecord field | Store actions, 2 bits per attachment. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 213, 383 | 2 |
| `draws` | MetalGpuPassRecord and MetalGpuPassStats field | Guest draws encoded in the pass (in metal_command_processor.cc only at the draw count lines, see Checks). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 47, 48, 75 | 3 |
| `draws` | MetalGpuPassRecord and MetalGpuPassStats field | Guest draws encoded in the pass (in metal_command_processor.cc only at the draw count lines, see Checks). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 368, 421, 423 | 4 |
| `vertices` | MetalGpuPassRecord and MetalGpuPassStats field | Host vertices drawn in the pass (in metal_command_processor.cc only at the draw count lines, see Checks). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 49, 76 | 2 |
| `vertices` | MetalGpuPassRecord and MetalGpuPassStats field | Host vertices drawn in the pass (in metal_command_processor.cc only at the draw count lines, see Checks). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 369, 421, 423 | 4 |
| `MetalGpuPassTimer` | class | Per-pass GPU time from stage boundary timestamps; creates the timed encoders and logs the totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 57, 109 | 2 |
| `MetalGpuPassTimer` | class | Per-pass GPU time from stage boundary timestamps; creates the timed encoders and logs the totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 29, 137, 141, 187, 249, 272, 295, 389, 400 | 9 |
| `Initialize` | MetalGpuPassTimer member function | Creates the sample buffers for a device and calibrates GPU ticks. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 59 | 1 |
| `Initialize` | MetalGpuPassTimer member function | Creates the sample buffers for a device and calibrates GPU ticks. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 29 | 1 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 55, 62 | 2 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 187 | 1 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_command_processor.cc` | 6187 | 1 |
| `RenderEncoder` | MetalGpuPassTimer member function | Render encoder with the pass timestamps attached (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3999, 5995 | 2 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 55, 65 | 2 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 249 | 1 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 4012, 4120, 4843, 5039 | 4 |
| `ComputeEncoder` | MetalGpuPassTimer member function | Compute encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1112 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 56, 67 | 2 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 272 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 5570 | 1 |
| `BlitEncoder` | MetalGpuPassTimer member function | Blit encoder with timestamps (plain when off). | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1241 | 1 |
| `Log` | MetalGpuPassTimer member function | Logs and resets the window totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 70 | 1 |
| `Log` | MetalGpuPassTimer member function | Logs and resets the window totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 400 | 1 |
| `Enabled` | MetalGpuPassTimer member function | Supported and metal_gpu_pass_timing on. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 82 | 1 |
| `Enabled` | MetalGpuPassTimer member function | Supported and metal_gpu_pass_timing on. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 137, 193, 252, 275 | 4 |
| `AllocateSamples` | MetalGpuPassTimer member function | Takes timestamp slots for a pass and registers its completion handler. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 83 | 1 |
| `AllocateSamples` | MetalGpuPassTimer member function | Takes timestamp slots for a pass and registers its completion handler. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 141, 194, 252, 275 | 4 |
| `Complete` | MetalGpuPassTimer member function | Adds a completed pass GPU time to the totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 86 | 1 |
| `Complete` | MetalGpuPassTimer member function | Adds a completed pass GPU time to the totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 183, 239, 295 | 3 |
| `Release` | MetalGpuPassTimer member function | Frees a pass slots and record when its handler is destroyed. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 88 | 1 |
| `Release` | MetalGpuPassTimer member function | Frees a pass slots and record when its handler is destroyed. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 181, 389 | 2 |
| `MetalGpuPassStats` | struct in MetalGpuPassTimer | Totals for a kind of pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 73, 105, 106 | 3 |
| `MetalGpuPassStats` | struct in MetalGpuPassTimer | Totals for a kind of pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 366, 419, 431, 432 | 4 |
| `passes` | MetalGpuPassStats field | Passes timed. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 33, 37, 53, 54, 55, 74 | 6 |
| `passes` | MetalGpuPassStats field | Passes timed. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 232, 367, 421, 422, 449, 460 | 6 |
| `total_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks from the first stage start to the last stage end. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 77 | 1 |
| `total_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks from the first stage start to the last stage end. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 324, 349, 352, 359, 370, 422, 438, 439 | 9 |
| `vertex_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks of the vertex stage. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 78 | 1 |
| `vertex_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks of the vertex stage. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 322, 343, 352, 371, 424, 426 | 7 |
| `fragment_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks of the fragment stage. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 79 | 1 |
| `fragment_ticks` | MetalGpuPassStats field; local in Complete | GPU ticks of the fragment stage. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 323, 346, 352, 372, 424, 427 | 7 |
| `record_out` | RenderEncoder parameter | Receives the pass record, for the draw count. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 60, 64 | 2 |
| `record_out` | RenderEncoder parameter | Receives the pass record, for the draw count. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 189, 190, 191, 243, 244 | 5 |
| `mutex_` | MetalGpuPassTimer member | Guards the sample buffers and totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 90 | 1 |
| `mutex_` | MetalGpuPassTimer member | Guards the sample buffers and totals. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 30, 146, 318, 391, 401 | 5 |
| `device_` | MetalGpuPassTimer member | Device the sample buffers belong to. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 91 | 1 |
| `device_` | MetalGpuPassTimer member | Device the sample buffers belong to. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 31, 50, 408 | 3 |
| `supported_` | MetalGpuPassTimer member | The device can take stage boundary timestamps and the buffers exist. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 92 | 1 |
| `supported_` | MetalGpuPassTimer member | The device can take stage boundary timestamps and the buffers exist. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 49, 129, 138, 402 | 4 |
| `sample_buffers_` | MetalGpuPassTimer member | The 16 counter sample buffers. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 93 | 1 |
| `sample_buffers_` | MetalGpuPassTimer member | The 16 counter sample buffers. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 36, 38, 41, 94, 97, 101, 104, 108, 133, 147, 153, 155, 169, 392, 393 | 15 |
| `sample_buffer_outstanding_` | MetalGpuPassTimer member | Pending passes per sample buffer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 95 | 1 |
| `sample_buffer_outstanding_` | MetalGpuPassTimer member | Pending passes per sample buffer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 37, 42, 95, 105, 156, 175, 394 | 7 |
| `sample_buffer_size_` | MetalGpuPassTimer member | Timestamps per sample buffer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 96 | 1 |
| `sample_buffer_size_` | MetalGpuPassTimer member | Timestamps per sample buffer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 98, 133, 150 | 3 |
| `current_sample_buffer_` | MetalGpuPassTimer member | Sample buffer being filled. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 97 | 1 |
| `current_sample_buffer_` | MetalGpuPassTimer member | Sample buffer being filled. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 47, 155, 157, 169, 170, 175 | 6 |
| `next_sample_` | MetalGpuPassTimer member | Next free slot in it. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 98 | 1 |
| `next_sample_` | MetalGpuPassTimer member | Next free slot in it. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 48, 150, 158, 171, 174 | 5 |
| `untimed_passes_` | MetalGpuPassTimer member | Passes not timed because no sample buffer was free. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 99 | 1 |
| `untimed_passes_` | MetalGpuPassTimer member | Passes not timed because no sample buffer was free. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 45, 164, 403, 451, 463 | 5 |
| `invalid_passes_` | MetalGpuPassTimer member | Passes whose timestamps were unusable. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 100 | 1 |
| `invalid_passes_` | MetalGpuPassTimer member | Passes whose timestamps were unusable. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 46, 363, 403, 451, 464 | 5 |
| `calibration_gpu_` | MetalGpuPassTimer member | GPU timestamp at Initialize. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 102 | 1 |
| `calibration_gpu_` | MetalGpuPassTimer member | GPU timestamp at Initialize. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 118, 122, 127, 409, 414 | 5 |
| `calibration_time_` | MetalGpuPassTimer member | Steady clock time at Initialize. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 103 | 1 |
| `calibration_time_` | MetalGpuPassTimer member | Steady clock time at Initialize. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 119, 125, 412 | 3 |
| `ns_per_tick_` | MetalGpuPassTimer member | Nanoseconds per GPU tick, from the calibration. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 104 | 1 |
| `ns_per_tick_` | MetalGpuPassTimer member | Nanoseconds per GPU tick, from the calibration. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 123, 133, 334, 410, 417, 451 | 6 |
| `stats_by_label_` | MetalGpuPassTimer member | Totals per kind of pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 105 | 1 |
| `stats_by_label_` | MetalGpuPassTimer member | Totals per kind of pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 43, 374, 403, 445, 461 | 5 |
| `stats_by_pass_` | MetalGpuPassTimer member | Totals per render pass configuration. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 106 | 1 |
| `stats_by_pass_` | MetalGpuPassTimer member | Totals per render pass configuration. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 44, 378, 385, 454, 462 | 5 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_gpu_pass_timing.h` | 109 | 1 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_command_processor.cc` | 1388, 2646, 6187 | 3 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_render_target_cache.cc` | 3999, 4012, 4120, 4843, 5039, 5570, 5995 | 7 |
| `metal_gpu_pass_timer` | inline global | The one MetalGpuPassTimer. | `src/xenia/gpu/metal/metal_texture_cache.cc` | 1112, 1241 | 2 |
| `metal_gpu_pass_timing` | cvar (Metal); iOS settings key | Turns pass timing on (default) or off; Diagnostics section toggle. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 10, 19, 134, 138 | 5 |
| `metal_gpu_pass_timing` | cvar (Metal); iOS settings key | Turns pass timing on (default) or off; Diagnostics section toggle. | `src/xenia/ui/windowed_app_main_ios.mm` | 3658 | 1 |
| `metal_gpu_pass_record_` | MetalCommandProcessor member | Record of the current render encoder, for its draw count. | `src/xenia/gpu/metal/metal_command_processor.h` | 438 | 1 |
| `metal_gpu_pass_record_` | MetalCommandProcessor member | Record of the current render encoder, for its draw count. | `src/xenia/gpu/metal/metal_command_processor.cc` | 613, 1835, 1908, 5707, 5708, 5709, 6063, 6189 | 8 |
| `timestamp_set` | local in Initialize | The timestamp counter set. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 61, 68, 73, 81 | 4 |
| `counter_sets` | local in Initialize | The device counter sets. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 62, 63, 64 | 3 |
| `counter_set` | local in Initialize | One of them. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 64, 65, 66, 68 | 5 |
| `cpu_timestamp` | local | CPU half of sampleTimestamps (unused). | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 116, 118, 121, 406, 408 | 5 |
| `gpu_timestamp` | local | GPU half of sampleTimestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 117, 121, 122, 127, 407, 408, 409, 414 | 8 |
| `holder` | local in AllocateSamples | shared_ptr owning the record, captured by the completion handler. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 180, 183 | 3 |
| `released` | lambda parameter | Record to release. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 178, 181 | 3 |
| `describe_attachment` | lambda in RenderEncoder | Records an attachment format and actions. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 199, 219, 221 | 3 |
| `sample_attachment` | local | Sample buffer attachment of the pass descriptor. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 223, 225, 226, 227, 228, 229, 233, 234, 235, 236, 237, 258, 260, 261, 262, 281, 283, 284, 285 | 19 |
| `timestamps` | local in Complete | Resolved timestamps of the pass. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 21, 58, 114, 131, 298, 308, 327, 328, 329, 330, 332, 336, 338, 343, 346, 349, 350, 356, 357, 359 | 33 |
| `resolved` | local in Complete | The timestamps were resolved. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 299, 310, 325 | 3 |
| `results` | local in Complete | resolveCounterRange data as timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 305, 308 | 2 |
| `valid` | lambda in Complete | A timestamp is neither 0 nor CounterErrorValue. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 319, 327, 329, 356 | 7 |
| `vertex_ok` | local in Complete | The vertex timestamps are usable. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 327, 335, 337, 342, 348, 354 | 6 |
| `fragment_ok` | local in Complete | The fragment timestamps are usable. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 329, 335, 339, 345, 348, 354 | 6 |
| `stale_ticks` | local in Complete | One second in GPU ticks, to spot a stage left with old timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 334, 336, 338 | 3 |
| `usable` | local in Complete | The pass has usable timestamps. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 325, 326, 354, 355, 356, 358, 362 | 7 |
| `add` | lambda in Complete | Adds the pass to a MetalGpuPassStats. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 366, 374, 378, 385 | 4 |
| `ms_per_frame` | lambda in Log | GPU ticks to milliseconds per frame. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 416, 422, 426, 427 | 4 |
| `describe` | lambda in Log | Text for a MetalGpuPassStats. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 419, 446, 458 | 3 |
| `by_time` | lambda in Log | Totals sorted by GPU time. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 431, 445, 454 | 3 |
| `listed` | local in Log | Top passes printed so far. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 453, 455 | 2 |
| `stats` | lambda parameter | Totals to add to or describe. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 366, 367, 368, 369, 370, 371, 372, 419, 422, 423, 424, 426, 427, 431, 433 | 18 |
| `ticks` | lambda parameter | GPU ticks to convert. | `src/xenia/gpu/metal/metal_gpu_pass_timing.cc` | 114, 416, 417 | 3 |
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
| `now` | 7 |
| `local_time` | 3 |
| `log_name` | 4 |
| `kText` | 5 |
| `AddAllConfigVarSettings` | 2 |
| `existing_keys` | 3 |
| `by_category` | 3 |
| `parsed` | 20 |
| `editor` | 6 |
| `DrawStats` | 2 |
| `draw_stats_` | 39 |
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
| `metal_perf_counters` | 26 |
| `MetalPerfMicrosecondsSince` | 4 |
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
| `metal_perf_left` | 4 |
| `metal_perf_right` | 4 |
| `metal_perf_footprint_mb` | 3 |
| `metal_perf_vm_info` | 3 |
| `metal_perf_vm_count` | 2 |
| `metal_perf_available_mb` | 3 |
| `metal_perf_window_us` | 3 |
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
| `MetalGpuPassRecord` | 18 |
| `sample_buffer` | 12 |
| `sample_buffer_index` | 5 |
| `first_sample` | 11 |
| `sample_count` | 12 |
| `label` | 22 |
| `width` | 58 |
| `height` | 48 |
| `samples` | 3 |
| `color_formats` | 6 |
| `depth_format` | 3 |
| `load_actions` | 3 |
| `store_actions` | 3 |
| `draws` | 7 |
| `vertices` | 6 |
| `MetalGpuPassTimer` | 11 |
| `Initialize` | 2 |
| `RenderEncoder` | 6 |
| `ComputeEncoder` | 8 |
| `BlitEncoder` | 5 |
| `Log` | 2 |
| `Enabled` | 5 |
| `AllocateSamples` | 5 |
| `Complete` | 4 |
| `Release` | 3 |
| `MetalGpuPassStats` | 7 |
| `passes` | 12 |
| `total_ticks` | 10 |
| `vertex_ticks` | 8 |
| `fragment_ticks` | 8 |
| `record_out` | 7 |
| `mutex_` | 6 |
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
| `stats_by_label_` | 6 |
| `stats_by_pass_` | 6 |
| `metal_gpu_pass_timer` | 13 |
| `metal_gpu_pass_timing` | 6 |
| `metal_gpu_pass_record_` | 9 |
| `timestamp_set` | 4 |
| `counter_sets` | 3 |
| `counter_set` | 5 |
| `cpu_timestamp` | 5 |
| `gpu_timestamp` | 8 |
| `holder` | 3 |
| `released` | 3 |
| `describe_attachment` | 3 |
| `sample_attachment` | 19 |
| `timestamps` | 33 |
| `resolved` | 3 |
| `results` | 2 |
| `valid` | 7 |
| `vertex_ok` | 6 |
| `fragment_ok` | 6 |
| `stale_ticks` | 3 |
| `usable` | 7 |
| `add` | 4 |
| `ms_per_frame` | 4 |
| `describe` | 3 |
| `by_time` | 3 |
| `listed` | 2 |
| `stats` | 18 |
| `ticks` | 3 |

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
