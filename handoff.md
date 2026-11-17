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

## The open problem: no 3D models on the title screen

On the title screen only the title text and "PRESS START" appear. The 3D scene
behind them is black.

### Established facts

- The game submits the scene and the emulator encodes it: about 250 draws and
  200k to 290k vertices per frame, with about 17 resolves. Nothing is dropped
  (`no_rasterization=0 no_vertices=0 compile_pending=0 other=0`). Source: the
  `Metal draw stats over 120 frames` log line.
- Per-frame order (from the owner's debug-level log):
  1. Scene drawn into a 2x MSAA float colour target (key `0x00C88400`) and a
     float depth target (key `0x00688000`), in two bands (64 and 26 tile rows).
  2. Both are resolved to guest memory and read back as 1280x720 textures:
     `k_16_16_16_16_FLOAT` at `0x15540000` and `k_24_8_FLOAT` at `0x15C78000`.
  3. Half- and quarter-resolution post-processing passes
     (`k_16_16_FLOAT` 640x360, `k_16_16_16_16_FLOAT` 322x182, `k_16_16`).
  4. Final 1280x720 `k_8_8_8_8` image at `0x1F5F8000`, then present.
- The UI is drawn without MSAA and is visible.

### Ruled out (do not retry)

- **BC/DXT texture decoding.** The device has no native BC support, so DXT
  textures are decoded to RGBA by compute shaders. A readback probe on build
  #9524 showed all 32 sampled textures (30 DXT1, 2 DXT5) decode to real data.
  A CPU decoder would not help.
- **Occlusion queries.** Draw stats show nothing is being culled.
- **Config toggles**, each tried by the owner with no change:
  `metal_shared_memory_zero_copy`, `metal_texture_upload_via_blit`,
  `metal_texture_cache_use_private`, `metal_use_heaps`, `native_2x_msaa`,
  `depth_float24_convert_in_pixel_shader`, `metal_transfer_msaa_sample_id`.
  (`metal_use_heaps=true` reduces stutter; leave it at its default.)

### Result of the frame texture probe (build #9525)

- The final image (`0x1F5F8000`, `k_8_8_8_8`) has data (the UI).
- The resolved scene colour (`0x15540000`, `k_16_16_16_16_FLOAT`) is **all
  zero in every sample**, including alpha, across 8 probes.
- The depth texture (`0x15C78000`) was never sampled by that probe.

So the scene is already empty when the game reads it back. Key decoding: the
scene target `0x00C88400` is `k_2_10_10_10_FLOAT`, 2x MSAA, base tile 1024,
pitch 16; `0x00C082D0` is the same format without MSAA at base 720. Both
resolve to the 16F texture through the `kFull64bpp` copy shader. The path is
render target -> `DumpRenderTargets` (compute, into the private EDRAM buffer)
-> resolve compute shader -> shared memory buffer -> texture load.

### Result of the resolve probe (build #9528)

- Every resolve from a 2x MSAA source has `edram_nonzero=0` and
  `dest_nonzero=0`: the scene colour (`source_format=3`, tiles 1024+64x16 and
  1024+26x16) and the scene depth (`depth=1`, tiles 0+64x16 and 0+26x16).
- Resolves of non-MSAA 8888 targets (the UI / final image) have matching,
  non-zero `edram_nonzero` and `dest_nonzero`, so the dump and resolve paths
  work in general.
- So the multisampled scene colour **and** depth targets are empty (or dump as
  empty) right after ~250 draws were encoded into them. Depth being all zero,
  stencil included, means no draw wrote depth either. `native_2x_msaa=false`
  (2x emulated with 4x, different dump shader) was tried earlier with no
  change, which argues against a 2x-specific dump shader bug.
- Render pass store actions are `Store` and the dump reads `rt->texture()`,
  which is the multisampled texture, so those were checked and look right.

Also noticed: the owner's config has drifted from defaults through their own
experiments (`spirv_version_override="1.4"`, `metal_transfer_tile_instancing
=true`, `metal_force_bc_decompress=true`, `async_shader_compilation=false`,
`depth_bias_decal_clamp=5e-05`, `gpu_allow_invalid_upload_range=true`). The
models were missing with defaults too, so these are not the cause, but ask
the owner to reset them so runs are comparable. The log prints a full
`CONFIG DUMP` at startup; check it on every log.

### Result of the scene draw probe (build #9529)

The scene's draws look valid. Typical line:
`surface=0x14010500` (pitch 1280, 2x MSAA), `color0=0x00030400` (base 1024,
2_10_10_10_FLOAT), `depth=0x00010000` (base 0, D24FS8),
`depth_control=0x00700766` (z test + write, GREATER_EQUAL, stencil off: the
game uses reversed Z), `color_mask=0xF`, `viewport=0,0 1280x720 z=0..0.5`,
`ndc_scale=1,1,-1 ndc_offset=…,…,1`, `scissor=0,0 1280x512` for the first
band and `viewport=0,0 1280x208`, `scissor=0,0 1280x208` for the second.
Vertex counts range from 1080 to 20550.

Before the scene, the game clears the EDRAM areas with 4-vertex quads drawn to
4x MSAA depth surfaces of pitch 640 (`surface=0x0A020280`, `color_mask=0x0`,
`mode=5`, keys `00308400` at base 1024 and `00708000` at base 0). The scene
colour and depth targets therefore start from ownership transfers out of
those 4x depth targets (depth -> colour reinterpretation, and 4x -> 2x depth).

A depth buffer of all zeros with a GREATER_EQUAL test should let every
fragment pass and write non-zero depth, yet depth dumps as zero. So either no
fragments are produced at all, or the targets do have content and the dump's
compute-shader read of the multisampled textures (`texture2d_ms` /
`depth2d_ms`) returns zeros on this device/OS. Every multisampled dump in the
resolve probe was zero and every single-sample one was not.

The owner's config was still non-default in both logs of that run (the reset
did not take effect); check the `CONFIG DUMP` again.

### Result of the multisample probe (build #9530, default settings)

`Metal multisample probe: key=0x00C88400 … nonzero=0/10485760` on every
sample. Metal's own resolve of the scene colour target is also all zero, so
the target really is empty; the compute dump is not at fault. The owner's
settings were back to defaults in this run.

Checked by reading and found in order: depth/stencil state creation,
rasterizer state, viewport setup (`allow_reverse_z=false` flips the guest's
reversed range the same way for all targets), render pass load/store actions
(`Store`; first-use clear only once), render target storage (private, not
memoryless; only dummy colour targets are memoryless on iOS), transfers go
into the same command buffer in order, and every non-depth-only fragment
shader writes `gl_SampleMask = -1` unless alpha-to-mask is enabled.

Upstream (branch `xenios`, fetched with `git fetch --depth=600 upstream
xenios`) has many later commits in exactly this area, for example "Store
constant depth rectangles drawn through a 4x view in the tiles' owner",
"Leave color quads that overwrite depth and stencil out of the depth
transfers", "Match D3D12 color-to-depth transfer packing". They rework the
file heavily and cannot be cherry-picked as they are.

### Result of the scene pixel probe (build #9532, default settings)

- Every probed scene draw passes **no samples**: 64 of 64 lines with
  `vertices=1440` have `samples_passed=0`. These are the draws on
  `surface=0x14010500` with `depth_control=0x00700766`, issued once with
  `cull=0x00018006` and once with `cull=0x00018002` in each of the two bands.
- Every 4-vertex clear quad passes its full area (for example 1843200 =
  640x720x4 samples), so the counter and rasterization on multisampled
  targets work. All of those quads use depth function ALWAYS or have the depth
  test off, so they say nothing about what the depth buffer holds.
- So the fragments are rejected before they are written; nothing clears the
  target afterwards. Since the same mesh is drawn with both winding orders
  and both give 0, culling alone is an unlikely cause.
- That run had only 152 draws and about 6400 vertices per frame, fewer than
  the 250 draws seen on the title screen before; the first 16 multisampled
  draws of a frame are all that is probed.

### Result of the four-pass pixel probe (build #9534, default settings)

- Title screen and the start of a new game. Every scene draw (1080 to 20550
  vertices, `surface=0x14010500`) has `samples_passed=0 depth_test_off=0
  cull_off=0 depth_clip_off=0`. Every clear quad passes its full area in all
  four passes.
- So the depth test, the stencil test, culling and depth clipping are **not**
  what rejects the scene. The triangles never cover a sample, or every
  fragment is discarded.
- Scene draw state: `clip=0x00080000` (DX clip space, clipping on, no user
  clip planes), `vte=0x0000043F` (viewport scale and offset on, W not
  reciprocal), `color_control=0x87000007` (alpha test and alpha-to-mask off).
  So user clip planes and the alpha test are ruled out too.
- What is left: the vertex shader's inputs (vertex streams, index data, float
  constants) or its output position. All register writes do reach
  `MetalCommandProcessor::WriteRegister`, so the float constant dirty flags
  were checked and look right.

### Result of the scene data probe (build #9539, default settings)

This is the first direct cause found. For the scene vertex shader
`vs=F215CCB53656BEFB` (cached source `3CB0952BDED69281.mslsrcache`):

- Position is `ViewProjection (packed constants 0 to 3) x LocalToWorld
  (packed 5 to 8) x vertex position`.
- The first four packed vertex constants were, in 20 of 24 lines:
  `2.74, 0.094, -0.073, -0.073` / `-1204.7, -2257.7, -2011.3, -2013.4` /
  `0, 0, 0, 0` / `-nan, -nan, -nan, -nan`, and in the other 4:
  `0, 0, 0.999, 1` / `0.995, 0, 0, 0` / `0, 0, 0, 0` / `-nan x4`.
  Two rows look like real matrix columns; one is all zero and one is all NaN.
  A NaN column makes every output position NaN, so nothing is rasterized,
  which matches every earlier probe (`as_lines=0 as_points=0` too).
- Everything else is sane: `guest_match=true` for both vertex streams, the
  stream bytes are plausible big-endian floats, indices run 0 to 266 for a
  267-vertex stream, targets are 1280x1024 2x MSAA (`format=115`
  RGBA16Float, `format=260` Depth32Float_Stencil8).
- The cached vertex shaders were read and are translated correctly (vertex
  index swap, fetch, matrix multiply, final W / NDC step, buffer slots).
- Checked by reading: every constant write (`SET_CONSTANT`,
  `LOAD_ALU_CONSTANT`, type 0 packets) ends in the virtual `WriteRegister`,
  the packing loop matches the Vulkan backend's, and the dirty tracking is
  right. So the bad values are most likely what the **game itself wrote**,
  i.e. the guest CPU computed a wrong matrix. That points at the a64 JIT
  (vector / VMX128 code), which ties in with the owed audit of
  `cpu/compiler`, `cpu/hir`, `cpu/ppc` against the newer a64 backend: the
  whole `src/xenia/cpu` tree differs from upstream (262 files).
  **This last step is an inference, not yet confirmed.**

### Result of the constant probe (build #9540): confirmed, the game writes NaN

- `nan_constant_writes` is about 8760 per 120 frames on the title screen (0
  during the intro videos). The first writes in each window are
  `0xFFC00000` to c3.xyzw and c4.xyz, and `0x7FC00000` to c8.x.
- Raw constants of the scene shader (`vs=F215CCB53656BEFB`), typical line:
  - c0 `402F54F7,3DC166BD,BD96140C,BD963A81`, c1 `C496965E,C50D1B03,…`:
    real view-projection data.
  - c2 all `00000000`; c3 all `FFC00000`: the rest of the view-projection.
  - c4 `FFC00000 x3, 3F800000`: looks like the pre-view translation
    (minus the camera position), so **the camera position is NaN**.
  - c5 to c7: a sane rotation times 10; c8 `7FC00000,7FC00000,7F800000,
    3F800000` (NaN, NaN, +Inf, 1): the object translation after the NaN
    camera offset. The +Inf shows a division by zero somewhere upstream.
  - c9 to c11: sane.
- So the GPU side is doing what it is told. The guest computes a NaN camera
  position and a zero row in the view-projection matrix. The fault is in
  guest code execution: the a64 JIT, or its mismatch with the older
  `cpu/hir`, `cpu/ppc`, `cpu/compiler` on this branch.

### The front end and the JIT backend are from different releases

Found by diffing `src/xenia/cpu` against upstream tags (fetch one with
`git fetch --depth=1 https://github.com/xenios-jp/XeniOS tag <name>`):

- `src/xenia/cpu/backend/a64` is upstream `release-v2.0.1-build-9730`
  (only include paths differ).
- `src/xenia/cpu/ppc`, `cpu/hir` and `cpu/compiler` are still
  `release-v1.0.1-build-9288`.
- Between the two, the behavioural differences in the front end are few.
  The one in the vector path: 9730's `ContextPromotionPass` does not forward
  VEC128 context values or remove VEC128 context stores (upstream turned that
  off on 2026-01-25 "to work around undiagnosed instability in some games";
  later releases put it back behind `context_promote_vec128` with range-keyed
  tracking). This branch still did both.

**Candidate fix, pushed, not yet tested on the device:** commit "[CPU] Stop
promoting vector registers across instructions, as the a64 backend's release
does" takes 9730's `context_promotion_pass.cc`. If the title screen's 3D
scene appears with this build, this was the cause. If not, the test job
below is the next source of information.

Full audit of instruction coverage: `docs/ppc_instruction_coverage.md`.
Summary: no user-mode instruction the game has hit is missing (no
`Invalid instruction` / `Unimplemented instruction` /
`Unable to process HIR opcode` in any log); 28 instructions are stubs (all
integer AltiVec, string load/store, `mtfsb0/1`), 22 are partial (mostly the
overflow-recording `o` forms); `vupkhsh128` / `vupklsh128` are absent from
the decode table (upstream added them later).

### CPU test job

`.github/workflows/cpu-tests.yml` ("CPU tests"), triggered by pushes that
touch `src/xenia/cpu/**`, `src/xenia/base/**`, the workflow or
`tools/ci/run_cpu_tests.py`, and by hand (workflow_dispatch).

- Job 1 (Ubuntu) builds the patched binutils in `third_party/binutils`,
  runs `./xenia-build.py gentests` and uploads the `.bin` / `.map` files.
  Verified locally in the cloud container: 570 test files assemble.
- Job 2 (`macos-26`, Apple Silicon) generates the desktop projects with
  `--tests`, builds `xenia-cpu-tests` and `xenia-cpu-ppc-tests`, and runs
  `tools/ci/run_cpu_tests.py`, which runs every HIR test case and every
  instruction test file in its own process and writes
  `cpu-test-results/report.md` (also shown as the job summary). Failing
  tests do not fail the job; a test program that does not build does.
- `src/xenia/cpu/testing` gained the tests upstream wrote for this backend
  at 9730 (compare, NaN propagation, pack/unpack, memory, rounding mode,
  opcode coverage and so on), plus `test_backend_cvars.cc`. `util.h` keeps
  this branch's Apple-specific stack setup. All of them pass a syntax check
  against this branch's headers on Linux/x64; none has been built for macOS
  yet.
- Known risks for the first runs: the macOS desktop build of this branch has
  not been tried since the iOS 16 work, and tests that call
  `memory->SystemHeapAlloc` themselves may not suit this branch's memory
  setup on Apple platforms. Treat harness crashes separately from wrong
  results.

All GPU probes are still in place and should be removed once this is fixed.

### Input after starting a new game (reported by the owner, build #9534)

Input stopped registering after "new game". A physical controller was
connected in that run (`New controller connected to slot 0`), so this is not
the on-screen controller. The log shows new shaders still being compiled and
drawn until the owner quit, so the game was running; it was most likely in
the opening cutscene, which cannot be seen while the scene is black. Not
investigated further until the scene renders.

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
