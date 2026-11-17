# Handoff: XeniOS iOS 16 backport

Read this first if you are picking the work up in a new chat. It is written so
you need nothing from the earlier conversation.

## What this is

- Repository: `UltraAssassin64/XeniOS`, an Xbox 360 emulator for iPhone (a fork
  of Xenia). Upstream reference: `https://github.com/xenios-jp/XeniOS`.
- Goal: make it build and run on **iOS 16** (upstream targets iOS 26). Test
  device: iPhone 14 Pro Max, iOS 16.5.1, installed through TrollStore.
- Test game: Batman: Arkham Asylum (Unreal Engine 3).

## Standing rules from the owner (apply to every change)

1. Only edit branch **`Only-the-Trollstore-jit-imp`**. No pull request unless
   asked.
2. Nothing may be added that cannot compile for iOS 16: every framework,
   function and graphics API must exist on iOS 16 and up. CI enforces this with
   `-Werror=unguarded-availability-new` and deployment target 16.0.
3. Do not add unnecessary variable names. Before adding one, check both repos
   that it does not already exist, that it creates no circular reference, and
   that it exists during the GitHub Actions build.
4. **Every new name must be recorded in `docs/ios16_backport_variables.md`**:
   name, kind, purpose, file, line numbers, number of uses, and the totals
   table at the bottom. Update it in the same commit.
5. Reduce CPU calls that schedule the GPU (still not done, see "Owed").
6. Emulator functionality is the priority. The UI backport was dropped.
7. If there is a problem, stop and ask the owner with full context.
8. `AGENTS.md`: never commit `NEXT_STEPS.md`.
9. Commit messages and code must not name any AI model.

## How the branch relates to upstream

The branch is upstream `release-v1.0.1-build-9288` (premake build, March 2026)
plus the owner's file-by-file copies from newer upstream (CMake build,
rebuilt history). The recurring bug class is a **half-port**: a newer upstream
file copied in while the files it depends on, or that use it, are still old.
Upstream fixes cannot be cherry-picked blindly for the same reason.

## Build and CI

- Build is premake5 to Xcode: `./xenia-build.py build --target_os=ios`.
- CI is `.github/workflows/CI.yml`: a Lint job (clang-format 21) gates the
  "Build iOS ARM64" job on `macos-26`. A run takes about 15 minutes.
- Artifact name: `xenios_ios_arm64_<version>-<build>` (for example
  `xenios_ios_arm64_0.2.9-9524`). The app logs its build at startup:
  `iOS: Build <branch>@<commit> #<build> on <date>`. Always check that line in
  a log to confirm the owner ran the build you expect.
- There is no macOS machine locally. What can be checked on Linux:
  - `clang-format --style=file --dry-run <file>` (install `clang-format==21.1.8`
    with pip).
  - `clang++ --target=aarch64-linux-gnu -fsyntax-only -std=c++20 -w -I. -Isrc
    -Ithird_party <file>` for plain C++ files.
  - Objective-C blocks and `.mm` files can only be verified by CI.
- Formatting traps: `.mm` files are not lint-checked. Some files use CRLF line
  endings (`src/xenia/cpu/backend/code_cache_base.h`,
  `src/xenia/hid/sdl/sdl_input_driver.{h,cc}`). Never run `clang-format -i` on
  a whole CRLF file; use `--lines=a:b` or edit while preserving newlines.

## What works now (confirmed by the owner on device)

- iOS 16 build is green in CI.
- The game boots, plays its intro videos and reaches the "Press Start" title
  screen every run.
- Audio works. A controller works (Start is recognised).
- The in-app live log viewer follows the per-launch log file
  (`xenia_<date>_<time>.log`).
- Settings shows every config option ("All Options: <category>" sections).

Main fixes behind that, for context: creating the graphics/audio systems at
launch (`xenia_main_ios.mm`), stashing host handles for guest events and
semaphores (`xevent.cc`, `xsemaphore.cc`), a dual-mapped JIT code cache on
iOS 16 instead of flipping page protection (`code_cache_base.h`), mixing all
SDL audio clients into one device (`apu/sdl/`), and implementing
`EnumerateDevices` / `NotifyDevicesChanged` in the SDL input driver.

## Solved: no 3D models on the title screen

Confirmed by the owner on build #9543: the title screen's 3D scene renders.
The log has `nan_constant_writes=0` (it was about 8,760 per 120 frames).

- **Cause:** a half-port inside `src/xenia/cpu`. The a64 JIT backend is
  upstream `release-v2.0.1-build-9730`; `cpu/ppc`, `cpu/hir` and
  `cpu/compiler` are `release-v1.0.1-build-9288`. The older
  `ContextPromotionPass` forwarded VEC128 context values and removed VEC128
  context stores, which 9730 does not do. With it, the game computed a NaN
  camera position and a zero row in its view-projection matrix, so every
  scene vertex was NaN and nothing rasterized.
- **Fix:** commit "[CPU] Stop promoting vector registers across
  instructions, as the a64 backend's release does" (takes 9730's
  `context_promotion_pass.cc`). Why promotion breaks with this backend is
  not understood; upstream never diagnosed it either, and later releases
  re-enabled it behind `context_promote_vec128` with range-keyed tracking.
  It costs speed: every vector register access goes through guest context
  memory.
- **How it was found (for next time):** GPU probes narrowed it step by step
  (resolved texture empty -> render target empty -> draws pass no samples
  with every test off -> vertex constants hold NaN -> the game writes NaN),
  then a diff of `src/xenia/cpu` against upstream tags showed the mismatch.
  Fetch a tag with
  `git fetch --depth=1 https://github.com/xenios-jp/XeniOS tag <name>`.
- All GPU probes were removed afterwards by restoring the four Metal files
  to their state at commit `5039ed3` (draw statistics kept).
- Instruction coverage audit: `docs/ppc_instruction_coverage.md`.

## Open problem: a guest thread spins on an XMA register (hang after "Press Start")

Reported on build #9543: pressing Start sometimes hangs before the save
menu; a new game reaches the first cutscene but skipping it shows
"Loading..." forever, and the cutscene's end leaves a black screen.

- In the debug log, guest thread `F8000008` writes guest address
  `7FEA1A84` 1,726,629 times in a row: `MMIOHandler: handling fault at
  0x000000031FEA1A84, is_write=true` followed by `XmaContext: reset context
  48`. That is the XMA "context clear" register for contexts 32 to 63. It
  starts early (line 507,527 of 4.1 million) right after a burst of
  allocations on that thread, and never stops; that thread then never loads
  anything again.
- No MMIO read happens in between, the store decodes as a plain 32-bit
  store, and the handler advances the PC, so the guest code itself is
  looping. Clears of contexts 0 to 9 (register `7FEA1A80`) earlier in the
  run do not loop.
- `src/xenia/apu` matches 9288 apart from the SDL driver changes, so the
  XMA code is not a half-port.
- The owner saw input stop after "new game" on an earlier build too, so
  this probably predates the vector fix.
- Not known: whether the loop condition is something the XMA emulation
  should set (the context data in guest memory) or a miscompiled guest
  branch.

**Current step:** `MMIOHandler::ExceptionCallback` now logs, when one MMIO
address is stored to 1000 times in a row (and every 500,000 after):

```
MMIO store repeated <n> times: guest address … value … host pc …
    instruction … previous … next … guest lr … ctr … r1 … r3 … r4 … r9 …
    r10 … r11 … r30 … r31 …
```

`guest lr` identifies the guest function; `value` and the registers show
what it is waiting on. It is a warning-level line, so the default log level
shows it.

Result on build #9549: `guest lr 829FF8C4`, value `00000100` (the decoder
byte-swaps it to bit 16 of `Context1Clear` = context 48), `r4 7FEA1A84`,
`r9 = r10 = 0x30` (48), `r11 0xC00`, `r3 1FCA7C00`, `r30 B410A090`,
`r31 B410A0D0`. Over 3.5 million repeats. No MMIO read happens between the
stores, so the loop polls normal guest memory (likely the context data).

Next build also logs, once per hang, `MMIO hang function <start>-<end>` and
`MMIO hang code <address>: <8 words>` lines: the guest code of the function
holding the store and 32 words around `lr`. Disassemble them with the
patched binutils (`third_party/binutils/build.sh`, then
`powerpc-none-elf-objdump -D -b binary -m powerpc -EB -Mvmx128` on the words
written to a file).

Result on build #9552 (disassembled with the patched binutils):

- `829FF858` (the function holding the store): for each of `count = obj[0]`
  slots of 96 bytes at `obj[8]`, if `slot+0x40` holds no context it calls
  `XMACreateContext(&slot[0x40])`, then `MmGetPhysicalAddress` on it,
  stores `(phys - context_array_base) >> 6` (the context index) at
  `slot+0x50`, and writes `1 << (index & 31)` to the clear register
  `0x7FEA1A80 + (index >> 5) * 4`. Then sets bit 0x40000 in `obj[4]`.
- `829FF920` (next function): for each slot with a context, calls
  `XMAReleaseContext` and zeroes `slot+0x40`; clears that bit in `obj[4]`.
- So nothing in these functions loops forever. Something above them keeps
  calling create-then-release, and context 48 (the lowest free one) is
  handed out each time. The caller is the next thing to find.

Next build logs `MMIO hang caller <n>: frame … return …` for up to 8 guest
stack frames and dumps the code before the first four return addresses.

Build #9554: same hang, but **no MMIO lines at all**. Most likely the JIT's
MMIO recording (`record_mmio_access_exceptions`, persisted per game in
`Library/Caches/xenia/modules/<hash>/executable_addr_flags.bin`) now marks
that store, so it is compiled as a direct MMIO-aware store and never faults.
The MMIO-handler diagnostic can therefore only fire on a fresh cache. (Not
verified; deleting that file would confirm it.)

In the earlier debug log (`xenia_2026-11-16_20-45-00.log`), the loop starts
right after the loader thread (`F8000040`) opens `Max_IntroAudio.xxx`; the
looping thread is `F8000008`, the game's main thread.

The diagnostic moved to the `XMACreateContext` export, which every pass of
the loop calls: on the 1000th call it logs `XMACreateContext called …
caller lr … r1 …`, `XMA hang code …` and `XMA hang caller <n>: …` with
the guest stack (10 frames, code dumped for the first five).

Build #9554, second hang log (the MMIO path fired this time) gave the stack:

- caller 0 `829FF6F4` in `829FF530`: calls the create function `829FF858`;
  on failure calls the release function `829FF920`, maybe reports error
  `0xA7820007`, returns the error. On success it continues at `829FF734`
  (not yet dumped).
- caller 1 `82A69C80`: calls `829FF530`; if the result is negative and
  equals `0x8007000E` (E_OUTOFMEMORY) it jumps back to `82A69B98` and
  tries again, forever. Any other error returns `0x3A`.
- callers 2 to 7: `82A71EAC`, `82A45AD4`, `82A488F4`, `82A3F884`,
  `8286D044`, `8286CD9C`.
- Every kernel call the main thread makes just before the loop succeeds
  (virtual and physical allocations, MmGetPhysicalAddress), so the
  E_OUTOFMEMORY most likely comes from the game's own code after context
  creation, from `829FF734` on.

The XMACreateContext diagnostic now dumps 192 words from return - 0x100
for the two nearest callers, which covers `829FF734`.

Build #9556 (four runs): the loop appeared in three, always with the same
guest stack, but **`XMACreateContext called` never appeared**, so
XMACreateContext ran fewer than 1000 times while the store repeated
millions of times. The old debug log agrees: in its last million lines the
main thread makes no kernel call at all (no MmGetPhysicalAddress, which is
logged), only the faulting store. So the game-level retry reading above is
wrong: the **host** keeps re-running the one JIT store, either because the
generated code branches back to it or because the fault does not resume
past it. The MMIO handler and the Apple signal handler were read and look
right (resume = pc + 4, written back through the mcontext pointer).

Build #9557 host dump (four of five runs hung, identical code each time),
disassembled with `llvm-objcopy -I binary -O elf64-littleaarch64` then
`llvm-objdump --triple=aarch64 -D -j .data`: the faulting
`str w22, [x21, x0]` is preceded by the guest address computation (with
the `>= 0xE0000000 ? +0x1000` adjust) and followed by `dmb ish` (eieio),
then the iOS title-stop poll (`ldr w15, [stop_word]; cbnz`), then the next
guest load. Straight-line code; nothing branches back to the store. So the
same store keeps faulting: the fault-resume path is the prime suspect, not
the generated code or the game.

Experiment asked of the owner: set `emit_inline_mmio_checks = true`
(Settings > All Options: CPU). Then stores to 0x7FC00000-0x7FFFFFFF call
`MMIOAwareStore` directly and never fault. If the hang goes away, the bug
is in the fault path (MMIOHandler / exception_handler_mac.cc on iOS).

Build #9557 with `emit_inline_mmio_checks = true`: first run still hung
(no diagnostics can fire in that mode; at quit one guest thread had to be
force-terminated). Second run with `log_level = 3` (debug) got through
both cutscenes into gameplay. So the hang is timing-dependent (a race), and
not only the fault path.

Working debug log (`xenia_2026-11-17_14-51-29.log`): the main thread clears
context 48, calls MmGetPhysicalAddress(FFCA6C40), clears 49, then decodes
context 48 itself (on ARM64 `use_dedicated_xma_thread` defaults to false, so
a kick runs `XmaContext::Work()` on the writing guest thread). In the hung
runs it never gets from "clear 48" to the next MmGetPhysicalAddress.
Contexts 0-43 are used by the XAudio thread `F80000D0`.

Build #9557, two debug-level runs (`log_level = 3`):

- `emit_inline_mmio_checks = true`: reached gameplay.
- `emit_inline_mmio_checks = false`: hung; the clear store faulted 1,596,843
  times. In that log it is **the first and only fault in the MMIO range
  path** (virtual 0x7FC00000-0x7FFFFFFF); every other fault is a
  physical-memory write watch, which just retries the instruction and never
  changes the PC. So on iOS the range path's "emulate the store and resume
  at pc + 4" has never worked once: the thread resumes on the same store.
- iOS compiles `exception_handler_mac.cc` (platform_files: `_ios`, then
  `_mac`, then `_posix`). It writes `mcontext->__ss.__pc`, as upstream's
  `exception_handler_posix.cc` does. Why the kernel does not take the new
  PC here is unknown. Unverified idea: pointer-authentication handling of
  signal thread state on arm64e hardware (the SDK's
  `__darwin_arm_thread_state64_set_pc_fptr`).
- **Fix applied:** `emit_inline_mmio_checks` defaults to true on iOS
  (`kEmitInlineMmioChecksDefault` in `memory.cc`). Existing configs that
  saved `false` keep it; the owner must turn it on or delete the line.
- One earlier run with inline checks at `log_level = 2` still hung; where is
  unknown. The XMA repeat detector below covers that case.

Next build: `XmaDecoder::WriteRegister` logs `XMA register <r> written <v>
<n> times in a row; guest lr …` after 1000 identical writes and, once,
`XMA repeat frame <n>: <symbol> + <offset> (<image>)` for up to 24 native
frames (`backtrace()`/`dladdr()`), whichever path the write took.

Next build adds `MMIO hang host <address>: <8 words>` lines: 96 ARM64
words from 256 bytes before the faulting store. Disassemble with
`llvm-objdump -D -b binary`-style tooling (on Linux:
`llvm-objdump --triple=aarch64 -D` on a raw file, or write the words to a
file and use `--start-address`).

CPU tests on ARM64 (CI "CPU tests", macOS) after the shared-memory fix:
249 of 249 HIR tests and 141,712 of 141,712 instruction cases pass. The
earlier failures were all `FAILED SETUP` from shared-memory name clashes in
the test harness on macOS (fixed in `memory_posix.cc`, macOS only).

## JIT automation

Settings > Automation has "Auto-Enable JIT via StikDebug" (stikjit://) and,
since this session, "Auto-Enable JIT via TrollStore"
(`apple-magnifier://enable-jit?bundle-id=<bundle ID>`). TrollStore is tried
first when both are on. Not yet confirmed on the device.

## Performance (reported on build #9543, not yet investigated)

Very slow, with flickering. Known contributors: about 1.06 million vertices
and 290 to 360 draws per frame on the title screen; vector register
promotion now off; the GPU probes re-drew the first 16 multisampled draws
six times every 120 frames with tests off (removed now, likely part of the
flicker); the blocking `waitUntilCompleted` sites listed under "Owed"; and
the `uniforms pool busy` waits. The Metal Performance HUD did not appear;
`MetalHudEnabled` was added to `src/xenia/app/Info_ios.plist` at the owner's
request and has not been confirmed to work on iOS 16.

## On-screen controller (added at the owner's request)

- `src/xenia/hid/touch_input_driver_ios.{h,cc}`: `xe::hid::touch::Create`
  returns a driver that enumerates one device with `fallback_auto_bind`, so
  `InputSystem` gives it player one only while no physical controller is
  connected. `xe::hid::touch::SetTouchState` receives the state from the UI.
  The files are in the `xenia-hid` project and, through the build's `_ios`
  filter, compiled only for iOS.
- `src/xenia/app/xenia_main_ios.mm`: `CreateInputDrivers` adds the driver
  after SDL.
- `src/xenia/ui/windowed_app_main_ios.mm`: `XeniaTouchPadView` draws and
  tracks 19 controls (two sticks, LT/RT, LB/RB, Back/Guide/Start, D-pad,
  A/B/X/Y, L3/R3). `XeniaViewController` shows it from
  `pollControllerNavigation` while a game is on screen, no menu is open and
  `[GCController controllers]` is empty. A tap on a control no longer toggles
  the in-game menu; a tap elsewhere still does.
- Not customizable, fixed layout, no haptics. Upstream has a much larger
  customizable touch system under `src/xenia/hid/touch/` and
  `src/xenia/ui/ios/touch/`; this is deliberately a small stand-in with a
  compatible driver idea (fallback device), not a port.
- This file is Objective-C++ under manual reference counting and is not
  lint-checked; it can only be compiled by CI.

### Other observations, unverified as causes

- Once per frame: `Metal transfer tile instancing fallback: exceeded 64 MiB
  instance-buffer budget this frame`. From reading
  `metal_render_target_cache.cc` (around lines 5196 to 5260 and 6500 to 6650),
  this fires when the per-frame instance buffer cannot grow mid-frame, and the
  code then draws the same rectangles through a slower path. It is probably
  not the black scene but is a likely source of stutter.
- `SPIRV-Cross: uniforms pool busy; waiting for an in-flight command buffer to
  retire` appears repeatedly: the CPU is blocking on the GPU.

## Owed from the original request

- **Reduce CPU calls scheduling the GPU.** Nothing has been changed yet.
  Known blocking sites (`commit()` followed by `waitUntilCompleted()`):
  `metal_render_target_cache.cc` near lines 3315, 4314, 4869;
  `metal_texture_cache.cc` near 1375, 1534, 3123; and the swap path in
  `metal_command_processor.cc`. The two warnings above belong here too. This
  was deliberately held until the models render, so a scheduling change is not
  confused with the rendering bug.
- Continue the upstream audit, notably `src/xenia/cpu/compiler`, `cpu/hir` and
  `cpu/ppc`, which have not been matched to the newer a64 backend.
- Remove the diagnostics once the rendering bug is fixed: the frame texture
  probe and, if the owner agrees, the draw stats. Update the variable sheet
  when doing so.

## Known loose ends (left alone on purpose)

- A stray tracked path named `“third_party/xbyak_aarch64”` (with curly quotes)
  exists in the tree.
- CI signs with `xenia_ios.entitlements`, not
  `xenia_ios_trollstore.entitlements`.
- The unhandled host fault path in `exception_handler_mac.cc` only logs and
  retries; making it fatal was tried and reverted because the JIT may rely on
  the retry.

## Working method that has been effective

1. Make one small, targeted change.
2. Check formatting locally, update `docs/ios16_backport_variables.md`.
3. Commit and push to the branch; wait for CI (about 15 minutes).
4. Give the owner the artifact name and the exact steps to run.
5. Read the returned log: confirm the build number first, then the specific
   lines the change was meant to produce.

The owner uploads logs as zip files. They are large (30 to 50 MB); search them
with `grep -a` rather than reading them whole. Log line prefixes: `i>` info,
`w>` warning, `G>` GPU debug, `A>` audio debug, `d>` kernel debug.
