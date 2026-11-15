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

### Current step

A **scene draw probe** was added at the end of `IssueDrawMsl`
(`src/xenia/gpu/metal/metal_command_processor.cc`). In the first frame of
each 120-frame statistics window it logs up to 16 draws that target a
multisampled surface:

```
Metal scene draw probe: vertices=… primitive=… surface=0x… color0=0x… depth=0x… mode=0x… depth_control=0x… color_mask=0x… cull=0x… viewport=x,y WxH z=min..max ndc_scale=… ndc_offset=… scissor=x,y WxH
```

The resolve probe and frame texture probe are still in place.

Check CI for that commit first, then ask the owner for one log (title screen
for about a minute, press Start, ten seconds).

### How to read the result

- Viewport or scissor empty, off the target, or `z` range inverted or
  degenerate: the viewport/scissor math is wrong for the tiled 2x MSAA
  surface (compare with `draw_util::GetHostViewportInfo` callers in the
  Vulkan backend).
- `depth_control` shows a depth test that can never pass against the cleared
  value (decode `RB_DEPTHCONTROL` in `src/xenia/gpu/registers.h`; the
  first-use clear is 1.0 at `metal_render_target_cache.cc` ~3580 and the
  resolve clear is computed near line 6680): the depth clear value or the
  reversed-Z handling is wrong.
- `color_mask` 0 on all draws: only depth is being written, look at depth.
- Everything looks sane: the draws are not reaching the target for another
  reason (pipeline sample count, sample mask in the translated shader); next
  probe should read the multisampled target directly.

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
