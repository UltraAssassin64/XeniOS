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

### Current step

Commit `e3d763b26` replaced the decode probe with a **frame texture probe** in
`src/xenia/gpu/metal/metal_texture_cache.cc`. Every 512th load of an
uncompressed 1280x720 texture it logs:

```
Metal frame texture probe: #<n> format=<id> base=0x<guest address> bytes=<n> nonzero=<n> texels=<a> <b> <c>
```

`texels` are 8 bytes each from the middle column at 1/4, 1/2 and 3/4 of the
image height. Match `base` against the addresses in "Established facts" to
tell the stages apart.

- CI for this commit is run 259
  (`https://github.com/UltraAssassin64/XeniOS/actions/runs/43672541314`). It
  was still building when this file was written. **Check it first**; the probe
  uses an Objective-C block inside a `.cc`, which could not be compiled
  locally. If it failed, fix the build before anything else.
- Then ask the owner for one log: boot to the title screen, wait about a
  minute, press Start, wait about ten seconds.

### How to read the result

- Scene colour (`0x15540000`) is all or nearly all zero: the scene is lost in
  rendering or in the MSAA resolve. Look at the resolve path in
  `metal_render_target_cache.cc` and at upstream's later fixes for 2x/4x float
  resolve averaging and first-use clears.
- Scene colour has data but depth (`0x15C78000`) is zero or constant: the depth
  resolve / `k_24_8_FLOAT` texture load is wrong. UE3 samples that depth for
  lighting and fog. Upstream has later fixes for MSAA depth transfers.
- Both have data but the final image (`0x1F5F8000`) does not: the loss is in
  the post-processing passes; probe the 640x360 and 322x182 stages next.
- No probe lines at all: these loads do not go through the probed upload path;
  move the probe to where they are handled.

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
