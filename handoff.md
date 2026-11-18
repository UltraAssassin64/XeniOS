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
10. **Upstream references are two repositories, used together:**
    `https://github.com/xenios-jp/XeniOS` (the iOS/ARM64 project) **and**
    `https://github.com/has207/xenia-edge` (the project XeniOS itself is
    based on). xenia-edge also has an ARM64 backend, but compare against its
    **x64** logic (`src/xenia/cpu/backend/x64`): the x64 backend is more
    mature and handles more edge cases. Treat xenia-edge's x64 changes as
    the analogue of XeniOS's ARM64 changes; for shared code (base, kernel,
    GPU), compare the two directly. Whichever of the two is newer for the
    behaviour in question is the one to retrofit into this branch, as with
    earlier ports. If choosing between them is too difficult, stop and give
    the owner a rundown of the options instead of picking.
    Fetch with `git fetch --depth=1 https://github.com/has207/xenia-edge HEAD`
    and `git fetch --depth=1 https://github.com/xenios-jp/XeniOS HEAD` (or a
    tag); this chat kept them as `refs/remotes/xenia-edge/HEAD` and
    `refs/remotes/xenios/HEAD`. At the time this rule was added (2026-11-18)
    XeniOS HEAD was `77cc1358` (= `release-v2.3.3-build-11940`, 2026-11-15)
    and xenia-edge HEAD was `4fb8372d` (2026-11-18).

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

## Kernel call repeat detector

Every kernel export goes through `ExportRegistrerHelper::X::Trampoline` in
`src/xenia/kernel/util/shim_utils.h`, which now calls `NoteKernelCall`
(`shim_utils.cc`) first. Per guest thread it keeps the last eight exports
and, for cycle lengths 1 to 4, how many calls in a row repeated. When a run
reaches `kernel_call_repeat_threshold` (cvar, Logging, default 20000, 0 =
off) it logs `Kernel calls repeating <n> times in a row on thread <handle>:
A -> B ...; guest lr … r3 … r6 …` and the guest stack, then again every
100 thresholds. Works at any log level and for high-frequency exports.
It cannot see loops that make no kernel calls (like the MMIO store loop).

Results on build #9561, eight runs in three sets (owner's pattern: run 1
hangs after Press Start, run 2 hangs after the first cutscene, run 3
reaches gameplay unless a skip is rushed):

- No `XMA register` lines: the inline MMIO default fixed the store loop.
- Hung runs: the main thread `F8000008` loops "KeDelayExecutionThread ->
  RtlTryEnterCriticalSection -> RtlLeaveCriticalSection" at guest lr
  `8227EC40`/`8227EC60` with critical section `832E5B74`, 2 to 4 million
  times: a sleep-and-poll wait for another thread's job (a load). Runs
  that progress only show short bursts of about 20,000.
- Idle worker threads (`F8000078`, `F80000E0`) loop on KeDelayExecutionThread
  alone; that is normal and is no longer reported.

Next build: at a report (first, then every tenth threshold) it logs every
guest thread: `Kernel thread <handle> (<name>): inside|last called <export>
for <ms>, guest lr …, <n> calls`, plus 64 words of guest code around the
looping caller (`Kernel calls repeating: code …`).

Results on build #9562 (logs named by time; 19-29 and 19-34 hung at the
first cutscene, 19-41 is an empty log from a launch that failed after a
reinstall, followed by a phone crash; 19-44 after a reboot reached gameplay):

- The main thread's 2-4 million poll at `8227EC40` happens in the good run
  too. It is a slow load, not the hang itself.
- In both hung runs, thread `F8000040` (opens `\BmGame\CookedXenon`, the
  package loader) goes from about 21,000 kernel calls to **268-271
  million**, then waits in NtWaitForSingleObjectEx (lr `82399190`) for
  good. In the good run it ends at 58,106. The repeat detector never
  reported it, so its loop is longer than four calls or varies.
- At the end of a hung run, every game thread is waiting: the main thread
  (short waits), `F8000040`, and the movie thread `F80000E0`/`E4` (opens
  `\BmGame\Movies`, spins KfAcquire/ReleaseSpinLock at lr `823582BC`; it
  does the same in the good run). The GPU still draws a static frame
  (480 draws per 120 frames).
- Threads shown "inside ExTerminateThread" have exited; the next build
  leaves them out.

Next build (commit "[Kernel] Report kernel call storms and what blocked
threads wait on"):
- `Kernel call storm on thread …: 1000000 calls in <ms> ms …; last 32
  calls: Export@lr …` whenever a thread makes a million calls in under a
  second (first time, then every tenth), plus its guest stack.
- Cycle detection up to eight calls long.
- In the thread table, each thread inside a call for a second or more
  gets a second line: r3 (with the object type and name if it is a
  handle; XObject::Type numbers: 2 Event, 3 File, 4 IOCompletion,
  6 Mutant, 8 Semaphore, 12 Thread, 13 Timer), r4 and its guest stack.

Stopping a game (Exit to Library in the quick menu, or the app closing
normally) logs `iOS: guest threads at TerminateCurrentGame:` (or `OnDestroy`
/ `Queued launch`) followed by the same thread table, before the title is
terminated. A force-killed app logs nothing. "last called X, returned for
N ms" means the thread has been running game code without any kernel call
for N ms (a pure guest loop shows up this way). The object lookup only
try-locks the global critical region and prints "object table busy" if a
thread holds it, so the table cannot hang the exit.

Result on build #9564 (one run, log 20-27-14; the skip prompt appeared,
then the first cutscene hung; Exit to Library after about two minutes):

- The movie thread `F80000E8` (the same role as `E0`/`E4` before: opens
  `\BmGame\Movies`, spins KfAcquire/ReleaseSpinLock at lr `823582BC`) is
  blocked in NtWaitForSingleObjectEx on handle `F80001C0`, **a thread
  object** (type 12), for 336 s. Its guest stack starts in `8313AB98
  8313B240 8313B3E8 83139CDC` (probably the movie library's close/stop
  path) and comes from `8244B8E4 8244AA18 82447E58 82277F38`.
- `F80001C0` never appears in the kernel-call table: it has **never made a
  kernel call**. Either it never started, or it runs only game code (a
  spin in guest memory) and never reaches an exit.
- At shutdown one guest thread did not respond to termination ("timed out
  waiting for 1 guest thread(s) to exit"), consistent with a thread stuck
  in JIT code.
- Before stopping, `F80000E8` called KeDelayExecutionThread about 28
  million times (1 million per 300 ms) at lr `82D29D1C`: the sleeps return
  at once (a yield or zero-length sleep). The earlier "270 million calls"
  thread `F8000040` was probably the same kind of loop; the repeat
  detector ignores sleep-only loops, which is why it was never reported.
- Every other thread is idle waiting on events, including the loader
  `F8000040` (event `F8000044`).

Next build (commit "[Kernel] List every guest thread with its host PC at a
report"): after the kernel-call table, one `Guest thread <handle>: running
| not running, suspend count, priority, cpu, start <addr>; host pc … in
guest function <addr> near <addr>; guest lr; guest stack` line for every
guest thread in the object table, including threads that never made a
kernel call. On Apple ARM64 the host thread is suspended for a moment with
thread_suspend, its PC read with thread_get_state, and resumed; the PC is
mapped through the code cache to the guest function. The repeat detector
now reports only the shortest cycle (it used to repeat the report for 2x,
3x and 4x the cycle).

Results on build #9565 (logs 21-01-50, 21-05-37, 21-11-25):

- 21-01-50 (hang after Press Start, exited after a few minutes): every
  thread waits on events except `F80000E0`, **priority 31**, running game
  code in guest function `82309E90` (near `8230A6F4`, stack `82333E2C
  82653420 82653464 822F403C 82479490`), and the audio thread `F80000D0`
  polling XMAGetOutputBufferWriteOffset (9.5 million calls) in
  `82D0C490`. So the Press Start hang is a different stall from the
  cutscene one: a high-priority thread spinning in game code.
- 21-05-37 and 21-11-25 (2.4 GB and 0.9 GB): **the diagnostic itself
  crashed**. Right after a repeat report on `F800006C` printed its
  guest-thread list, that thread took SIGBUS at a fixed host PC with
  address `0x310C20000`, and the fault handler retried forever (24
  million and 9 million "Unhandled host exception" lines). The game
  logic thread froze, so only the music played on. The list had stopped
  just before the thread being waited on (`F80001D8`); reading the stack
  or kernel object of an exited thread whose guest memory was released
  is the likely fault. These two runs say nothing about the original
  bug.
- In 21-11-25, before the crash, the movie thread `F80000E4` (priority
  32) waited on thread handle `F80001D8`; the three movie helper threads
  listed (start `8313AD18`) are all "not running", that is exited. In
  build #9564 the "never made a kernel call" conclusion about `F80001C0`
  was wrong: the kernel-call table hides threads that are inside
  ExTerminateThread, so an exited thread looks like one that never
  called. Open question: does a wait on an exited thread ever return?
  `PosixCondition<Thread>::post_execution` calls pthread_join while the
  wait still holds the handle's mutex; XThread::Exit on iOS takes the
  `#else` branch in XThread::Create's lambda (no pthread cleanup push),
  so OnHostThreadExitCleanup/ReleaseHandle never run for exited threads
  on iOS (a leak; not shown to cause the hang).

Fix (commit "[Kernel] Never read released guest memory in the thread
reports"): every guest read in the stack walk, and the kernel-object/PCR
reads in the guest-thread list, first check the page table
(`KernelCallReadable`, under a try-locked global critical region). An
unreadable frame prints `(unreadable <addr>)`; an unreadable thread
object prints `kernel object unreadable`. A host PC outside JIT code is
now named with dladdr (`host code <symbol>+<offset>`).

Results on build #9566 (logs 21-45-39 Press Start hang, 21-49-36 reached
gameplay, 21-52-49 first-cutscene hang, 21-58-49 with
`emit_inline_mmio_checks` off):

- **First-cutscene hang, root cause found.** The movie job thread
  (`F80000E4`, priority 31, stack `8313AB84 8313B240 8313B3E8 83139CDC
  8244B8E4`) waits on thread `F80001C8` (start `8313AD18`, a movie helper
  thread). The guest-thread list shows `F80001C8` "not running" with
  guest lr `00000000` and its stack untouched (`unreadable 70BC0000`, the
  top of a fresh stack): **the thread was created and never ran.** Exited
  helper threads instead show lr `82D31274` (ExTerminateThread).
- Cause: in `PosixCondition<Thread>::ThreadStartRoutine`
  (`src/xenia/base/threading_posix.cc`) the new thread published
  `state_ = kSuspended` in one critical section and set `suspend_count_
  = 1` in a second one. `XThread::Create` calls `thread_->Resume()` right
  after creation (ignoring its result); `Resume()` returns as soon as the
  state is published, and if it ran between the two sections it saw
  `suspend_count_ == 0`, did nothing, and the thread then waited at the
  gate forever. Upstream fixed the same race (release-v2.3.3-build-11940,
  comment "Arm suspend_count_ in the same lock that publishes kSuspended,
  else a Resume() can land between, see count==0, bail, and wedge us at
  the gate").
- Fix (commit "[Base] Fix lost resume of newly created threads"): set
  `suspend_count_ = 1` in the same locked section that publishes
  `kSuspended`. A local stress test (scratchpad `racetest/`: create
  suspended, `Resume()` at once, check the thread runs; real
  `threading_posix.cc` built for Linux x64) lost 6, 9 and 8 of 3000
  thread starts with the old code pinned to two cores (1 of 3000
  unpinned), and 0 of 15000 with the fix.
- Press Start hang (21-45-39): no never-started thread. Every thread is
  waiting except job thread `F80000E4` (priority 31), running game code in
  `8230B3D0` near `8230B724` under `8230B380 8230ACD0 82309E90 82333E2C
  82653420 82653464 822F403C`. Build #9565's Press Start hang had the
  job thread in the same job (`82309E90` near `8230A6F4`, same stack
  root). It still makes occasional kernel calls (RtlLeaveCriticalSection
  at `82310A80`). Cause not known yet: a very long job or a loop that
  never ends. The next build samples a thread in game code eight times,
  1 ms apart, and dumps 32 words of guest code around the first sample
  (`Guest thread <h> code <addr>: ...`) so the loop can be disassembled.
- 21-58-49 (inline MMIO checks off): the old XMA register store loop is
  back (4723 `XMA register ... written` lines, `MMIO store repeated` at
  guest `7FEA1A84`). Same known cause (the iOS fault handler's resume PC
  is not honoured); keep `emit_inline_mmio_checks` on (the iOS default).
- Storms: job threads spin on KeDelayExecutionThread at lr `82D29D1C`
  (1 million calls in 300-400 ms); `XThread::Delay` turns sub-millisecond
  sleeps into `MaybeYield()`. This burns a core but is not the hang.
- Checked against both upstreams: xenia-edge `4fb8372d` and XeniOS
  `77cc1358` both contain the same "arm suspend_count_ in the same lock"
  fix, so the port matches both.
- XeniOS (newer for iOS) uses `XE_PLATFORM_APPLE` for the pthread cleanup
  push in `XThread::Create`/`Restore` and for skipping the direct
  `ReleaseHandle()` in `Exit`/`Terminate`; xenia-edge uses `XE_PLATFORM_MAC`
  (it does not target iOS). This branch mixes the two (cleanup push on
  `MAC`, skip on `APPLE`), which is the iOS handle leak below. Candidate
  next port: XeniOS's `APPLE` form.
- Both upstreams join exited threads outside the handle lock
  (`post_execution_unlocked`, "Terminate() can signal before the thread
  exits; join unlocked, once"), as part of a larger rewrite of
  `threading_posix.cc` (suspend gate semaphore, multi-wait registration).
- Not changed, noted for later: `PosixCondition<Thread>::post_execution`
  calls pthread_join while the waiter still holds the handle's mutex
  (upstream moved the join outside the lock); on iOS the
  `XThread::Create` lambda has no pthread cleanup push, so
  `OnHostThreadExitCleanup`/`ReleaseHandle` never run for threads that
  exit through ExTerminateThread (a handle leak).

## JIT automation

Settings > Automation has "Auto-Enable JIT via StikDebug" (stikjit://) and,
since this session, "Auto-Enable JIT via TrollStore"
(`apple-magnifier://enable-jit?bundle-id=<bundle ID>`). TrollStore is tried
first when both are on. Not yet confirmed on the device.

## Occlusion queries (owner's question, 2026-11-18; not started)

Owner reports flickering of a vignette and of fog decals on the main menu
(Unreal Engine 3), and asked how upstream implements occlusion queries
and their fake results.

This branch: only the old fake path. `ExecutePacketType3_EVENT_WRITE_ZPD`
in `pm4_command_processor_implement.h` writes a made-up sample count into
the report on each query end: it starts at
`query_occlusion_sample_upper_threshold` (100) and goes down by one per
query to `query_occlusion_sample_lower_threshold` (80), then wraps
(ZPass and Total both set to it). Metal has no override; D3D12 and Vulkan
do. `occlusion_query_enable` (gpu_flags.cc) is not used by Metal.
Note: a count that changes on every query is a plausible cause of
flicker in effects that scale with visible pixels. Cheap test without a
build: set `query_occlusion_sample_lower_threshold = 100` (equal to the
upper one) so the fake count is constant.

Upstream (XeniOS `77cc1358` and xenia-edge `4fb8372d`): a shared ZPD
system in `CommandProcessor` (`command_processor.cc/.h`, about 120
references), `xenos_zpd_report.h` (identical in both), per-backend query
pools (D3D12, Vulkan, and Metal `metal_zpd_visibility_pool.cc/.h`), and
`EVENT_WRITE_ZPD` handling in `pm4_command_processor_implement.h`.
Settings: `occlusion_query` = `fake` | `fast` (default: ask the GPU, write
a cached result at once, update when the GPU answers, biased to visible)
| `fast-alt` (keeps cached zero results) | `strict` (waits for the real
result); `occlusion_query_fake_lower/upper_threshold` (renamed from the
`query_occlusion_*` names here); `occlusion_query_log`;
`occlusion_query_viz` (VIZ_QUERY, the 360's predicated rendering, can skip
occluded draws); `occlusion_query_full_counters` (in-shader ZFail,
StencilFail and Total; off by default, slow).
Which is newer: xenia-edge's HEAD date is later, but for this feature
XeniOS is ahead: its shared code has `ApplyZPDMode`/`ReloadZPDMode` and
inexact-report stats, and its Metal pool has a path for devices without
the iOS 26 accumulate visibility result type (copies each render pass's
slots to a readback buffer when the pass ends). xenia-edge's Metal pool
lacks that path, so on iOS 16 only the XeniOS version can work. Metal
uses `setVisibilityResultMode(Counting/Disabled, offset)` on the render
encoder and `setVisibilityResultBuffer` on the pass descriptor (both
iOS 8+); the accumulate type (`setVisibilityResultType`) is iOS 26 and
not in this tree's metal-cpp `MTL::RenderPassDescriptor`, so the port
would keep only the readback path.
Port size: this branch's `command_processor.cc` is 957 lines against
1,847 in XeniOS, so the shared ZPD code cannot be copied without the
parts of the newer `CommandProcessor` it relies on (half-port risk).



### Findings on build #9568 (three runs into gameplay, 2026-11-18)

- Gameplay: about 3,500 draws and 3.5 million vertices per frame (draw
  stats: issued 420,000-444,000 per 120 frames).
- **The textures blacking out on the way to the over-the-shoulder camera
  are skipped draws.** In that window `compile_pending=125160` of 387,919
  draws: a third of all draws were dropped because their shader or
  pipeline was still compiling. The SPIR-V path skips a draw rather than
  wait for its pipeline, so objects disappear until compilation finishes.
  Same place in runs 2 and 3, because the same shaders are compiled again.
- **Shaders are recompiled every run.** 326-376 `MslShader: Compile
  timings` warnings per run (only compiles of 100 ms or more, or large
  SPIR-V, are warnings), typically `metal-library` 50-90 ms each.
  `metal_shader_disk_cache` caches only the MSL source (skips the ~5 ms
  SPIRV-Cross step); `newLibraryWithSource` runs every time. The
  `pipeline_binary_archive_` (MTLBinaryArchive) is only fed in the MSC
  path and `CreateMslPipelineState`; whether it is used and reloaded on
  iOS needs checking.
- Known CPU costs not yet measured: VEC128 context promotion off, inline
  MMIO checks on every store, 11 `waitUntilCompleted` sites, job threads
  spinning on zero-length KeDelayExecutionThread (`MaybeYield`).

Diagnostics added (commit "[GPU] Log frame, GPU, wait, compile, memory and
per-thread CPU numbers"), printed with the draw stats every 120 frames:
- `Metal perf over N frames:` frame time avg/max and frames over 33/50 ms
  (host time between guest swaps); GPU busy time and command buffers
  (GPUEndTime - GPUStartTime of every submission); CPU blocked on GPU
  (all `waitUntilCompleted` in the Metal GPU code now go through
  `WaitUntilCompletedCounted`); shaders compiled and pipelines created
  with their time; MB uploaded into shared memory; process footprint and
  `os_proc_available_memory`; `NSProcessInfo.thermalState` (0 nominal ...
  3 critical).
- `Metal perf threads:` every thread's CPU share in the window (task_threads
  + THREAD_EXTENDED_INFO), busiest 16 with name, low system thread ID and
  host priority, plus the total in "% of one core".
- Metal HUD: commit "[iOS] Ask for the Metal performance HUD in code as
  well" (MTL_HUD_ENABLED env at main, layer developerHUDProperties).

Owner's answers (2026-11-18): options 1-2 relate to the existing
"Async Shader Compilation" setting; option 3 should be an entry in the
game's long-press context menu; 4-5 wait for the diagnostics.
Checked: `async_shader_compilation` (default on) only chooses skip vs wait.
On: MslShader and pipelines compile on worker threads and draws are skipped
until ready (the blackouts). Off: compiled on the GPU thread, which waits
(hitches, no blackout). Neither keeps compiled Metal libraries or
pipelines between runs, so option 1 is still open. Worker count is
`metal_pipeline_creation_threads` (default -1 = 3/4 of the cores, so 4 on
a 6-core iPhone, competing with the guest threads during compile bursts).
XenosRecomp (hedge-dev, 70a084c) was reviewed: it is less exact than this
translator for emulation (plain multiply without the Xenos 0 * x = 0 rule,
rcp/rsq/log clamped to [FLT_MIN, FLT_MAX] which drops negative signs, no
dynamic register indexing or integer constants, vertex fetch replaced by
native input layouts that need the game's declarations up front), and its
one speed trick, skipping the control-flow loop for shaders without
jumps, is already in `SpirvShaderTranslator` (`has_main_switch`).

"Persistent Shader Cache" (`store_shaders`) on iOS: it gates
`InitializeShaderStorage`, whose non-MSC branch (the one iOS builds) only
sets the MSL source cache directory (saves SPIRV-Cross, about 5 ms per
shader). `g_metal_shader_cache` is initialized but `MslShader` never uses
it; the pipeline disk cache, MTLBinaryArchive and prewarm all sit in the
`METAL_SHADER_CONVERTER_AVAILABLE` (macOS) branch. So `newLibrary` from
source (50-90 ms) and `newRenderPipelineState` run again every run, as the
logs show. XeniOS HEAD's SPIR-V path is the same (MSL source cache only),
so there is no upstream code to port; a real cache would be new work
(e.g. MTLBinaryArchive for SPIR-V-path pipelines, iOS 14+, which saves the
GPU back-end compile but not the MSL front end).

Build #9574, one run with `emit_inline_mmio_checks` off (owner, log
23-22-21): no hang, no `Unhandled host exception`, no `MMIO store
repeated`, no `XMA register` lines; reached gameplay. The MMIO fault path
fix looks good. (The config dump at the top of the log prints `true`; it
did the same in build #9566's run where the loop proved the setting was
off, so that dump does not reflect the in-app setting.)
Perf numbers from that run (gameplay windows of 120 guest frames):
- frame avg 164-209 ms (about 5 guest frames per second), every frame
  over 50 ms; menus run at a steady 34.9 ms.
- GPU busy 72-91% (sum of command buffer GPU times; it can exceed 100%
  in places, so overlapping buffers are counted twice and this is an
  upper bound). About 70-75 command buffers and 27-32 EDRAM resolves per
  frame, 3,500 draws, 2.7-3.7 million vertices.
- CPU blocked on GPU: 0 in every window, so the `waitUntilCompleted`
  sites are not hit; owed item "reduce CPU calls scheduling the GPU" is
  not where the time goes.
- One guest thread (host name `XThread1EA0F (F`, truncated; guest priority
  31-32, so the job/render thread seen in the hang work) uses 77-100% of a
  core in every gameplay window: the game is bound by that thread's JIT
  code. Main thread 9-19%, `XThread16D07` 16-22%, GPU Commands 18-25%.
- Shader compile bursts: up to 118 shaders in one window taking 55 s of
  CPU across the 4 compile threads (`?#8A21`-`8A24`, about 25% each), with
  up to 241,000 skipped draws.
- Uploads into shared memory: 1.2-1.4 GB per 120 frames (about 11 MB per
  frame).
- Memory footprint 1.2-1.9 GB, 2.2-2.9 GB available; thermal state went
  from 0 to 1 (fair) by the end.
- `Metal presenter: requested the Metal performance HUD` was logged.

Build #9574, four more runs (logs 23-34-11, 23-36-18, 23-39-22,
23-43-58; config dumps show `emit_inline_mmio_checks` false, false, true,
false at launch). Owner: the HUD shows; title screen about 10 FPS, menus
after Press Start 13-22, cutscenes 28-30, then about 5 FPS from the
shoulder-camera transition on, including the pause menu; it gets worse as
the phone heats up. Numbers agree: gameplay 150-230 ms per guest frame,
menus 34.9 ms. Inline checks on or off makes no clear difference (on:
161-229 ms; off: 151-219 ms, within run-to-run noise).
In every gameplay window one guest thread at host priority 32 uses 96-100%
of a core and a second at host priority 16 uses 23-32%. Host priority 16
is Darwin's utility band, which prefers the efficiency cores: this
branch passes xenia's priority tiers (1/8/16/24/32) straight to
`pthread_setschedparam(SCHED_FIFO)`.

Ported from XeniOS `77cc1358` (newer than xenia-edge for these
Apple-only paths; xenia-edge still uses plain `sched_yield` and raw
priorities), commit "[Base] Port XeniOS's iOS thread scheduling fixes":
- `set_priority` on Apple remaps 1..32 linearly onto Darwin's SCHED_FIFO
  range (`sched_get_priority_min/max`, typically 15..47), so tier 16 runs
  at about 30 and tier 32 at 47.
- `MaybeYield` on Apple uses `thread_switch(MACH_PORT_NULL,
  SWITCH_OPTION_NONE, 0)`: `sched_yield` is `swtch_pri`, which drops the
  thread to the lowest priority until a 10 ms quantum passes; the guest's
  Sleep(0) polling loops go through `MaybeYield`.
- `PreferPerformanceCores()` (QOS_CLASS_USER_INTERACTIVE on iOS), called
  at the start of `CommandProcessor::WorkerThreadMain` ("GPU Commands").
Build #9578 (`xenios_ios_arm64_0.2.9-9578`, commit b4958de1): CI and CPU
tests pass. Not yet run on device; asked the owner for 2 gameplay runs
comparing FPS and the "Metal perf" lines, and to watch for new hangs.
Owner's 9578 runs (zip "Quite_a_few_runs", 5 logs, plus a force-quit
.ips): FPS unchanged; host priorities now read 31 and 47 as intended.
Gameplay: 80-220 ms per frame, GPU busy 85-105% of wall time, about 75
command buffers and 340-1100 draws (1.0-1.5M vertices) per frame,
"uniforms pool busy" waits; one guest thread (XThreadDB07) at 100% of a
core, GPU Commands under 10%. The occlusion threshold test (lower = upper
= 100) did not stop the vignette/fog flicker, so the fake count is ruled
out; depth handling is next.
The .ips (bug type 509, 0x8badf00d "failed to terminate gracefully"),
symbolicated against the 9578 binary: main thread in
`PosixConditionBase::Wait` from applicationWillTerminate, waiting on the
"GPU Commands" thread, which was blocked in `MetalCommandProcessor::
PrepareForWait` in `waitUntilSignaledValue(..., UINT64_MAX)`. iOS refuses
GPU work in the background, so the event is never signaled.
That exposed that our `PrepareForWait`, run on every ring-empty stall,
committed the command buffer and then waited for all GPU work twice (the
second time with a dummy command buffer). XeniOS and xenia-edge both
just commit there ("must not block on the GPU"). The shared-event branch
wasn't counted by `WaitUntilCompletedCounted`, so "CPU blocked on GPU 0
times" in the perf line was wrong: the CPU was blocked dozens of times per
frame. Commit "[GPU] Don't block on the GPU when the ring runs empty":
`PrepareForWait` now only calls `EndCommandBuffer()`, and the two shutdown
waits are bounded to 2 s (XeniOS bounds the first; the second is bounded
here too since iOS refuses the dummy command buffer in the background as
well). Risk: no CPU-GPU sync point is left in the frame loop, as in
xenia-edge; watch for stale resolve or readback results (UI, exposure).

GPU pass timing (owner asked for diagnostics of where the GPU time goes;
no existing option breaks it down, and neither upstream has per-pass
timing). Commit "[GPU] Log the GPU time of every pass": new
`metal_gpu_pass_timing.{h,cc}`, cvar `metal_gpu_pass_timing` (default on;
also a toggle in Settings > Diagnostics). Uses Metal stage boundary
timestamps (MTLCounterSampleBuffer, iOS 14+): render passes get vertex and
fragment start/end, compute and blit passes start/end. 16 sample buffers of
4096 timestamps, reused once their passes complete. Timed passes: guest
draws (with draw and vertex counts), rt transfer (render and blit), edram
store (+ msaa resolve), edram dump, resolve, host depth store, texture load,
texture upload blit. Every 120 frames, after "Metal perf threads":
"Metal GPU passes per frame ..." (per kind: ms per frame, passes, draws,
vertices, vertex/fragment ms) and "Metal GPU top passes per frame" (top 12
render pass configurations: size, sample count, MTL::PixelFormat numbers,
load/store actions 2 bits per attachment, colors 0-3 then depth). Passes
overlap, so the sum can exceed GPU busy. GPU ticks are calibrated against
the steady clock (logged as "1 GPU tick = X ns"). Not timed: init/snapshot
blits, scaled resolve buffer growth, texture dumps, the presenter.

Not ported: `ios_guest_threads_user_initiated_qos` is defined in
`xthread.cc` here but never read; in XeniOS it is only a settings entry.

Ideas to discuss with the owner (not started):
1. Cache compiled shaders across runs: MTLBinaryArchive for SPIR-V-path
   pipelines (iOS 14+), or compiling libraries with
   `MTLCompileOptions` and keeping a per-shader archive; check what
   XeniOS HEAD does here first (standing rule 10).
2. Choose skip vs wait for pipelines still compiling: skipping causes the
   blackouts, waiting causes hitches; a short bounded wait is a middle
   ground.
3. Precompile at boot from the shader cache list of the last run.
4. Reduce `waitUntilCompleted` stalls (owed item above).
5. Revisit VEC128 context promotion and the inline MMIO default once the
   MMIO fault fix is confirmed.


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

### Owed once the threading hangs are resolved (owner's request, 2026-11-18)

**Status (build after #9568):** both done, not yet tested on device.
1. Thread handle leak: commit "[Kernel] Release the handle of exited
   threads on iOS" (`XE_PLATFORM_APPLE` in `XThread::Create`'s and
   `Terminate`'s guards, matching XeniOS). The join-outside-the-lock
   rewrite was not ported.
2. MMIO fault resume: commit "[Base] Keep the resume PC set by iOS fault
   handlers". Root cause: `exception_handler_mac.cc` memcpy'd its aligned
   `ucontext_t` copy back over the signal context after the handlers ran.
   With `_XOPEN_SOURCE`, `ucontext_t` includes `__mcontext_data`, a copy of
   the registers taken before the handlers ran, so the copy back undid the
   new PC. XeniOS and xenia-edge copy in but never back. The owner should
   test once with `emit_inline_mmio_checks` off; if the store loop is gone,
   the inline default could be revisited (it costs a check per store).
   Unhandled faults now log the first ten and then one per million.

Original notes:

Both are held so they are not tested at the same time as the thread start
fix. Compare against both upstreams (standing rule 10).

1. **Thread handle leak on iOS.** `XThread::Create`/`Restore` only push the
   pthread cleanup handler (`HostThreadExitCleanupThunk` →
   `OnHostThreadExitCleanup` → `ReleaseHandle`) on `XE_PLATFORM_MAC`, while
   `XThread::Exit`/`Terminate` skip their own `ReleaseHandle()` on
   `XE_PLATFORM_APPLE`. On iOS neither runs, so every guest thread that
   exits keeps its handle and object. XeniOS (`77cc1358`) uses
   `XE_PLATFORM_APPLE` in all of these places; xenia-edge uses `MAC` because
   it does not target iOS. Port the XeniOS form. Related: both upstreams
   join exited threads outside the handle lock (`post_execution_unlocked`),
   part of a larger `threading_posix.cc` rewrite.
2. **MMIO fault path does not resume on iOS.** With
   `emit_inline_mmio_checks` off, the XMA context-clear store (guest
   `7FEA1A84`) faults, `MMIOHandler::ExceptionCallback` handles it, but the
   resume PC written back by `exception_handler_mac.cc` is not taken, so
   the store repeats forever (build #9566 log 21-58-49). Inline checks on
   iOS avoid it. Compare the exception and MMIO handling with xenia-edge's
   x64 backend and exception handler (more mature, more edge cases) and
   with XeniOS's ARM64 code; port the newer, or bring the owner the
   options if they diverge too much.

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
