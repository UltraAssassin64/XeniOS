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

## Occlusion queries (owner's question, 2026-11-18; ported as an experiment in 27800f3f)

Now: with `occlusion_query_enable` (Settings > Accuracy > Real Occlusion
Queries, off by default) the Metal SPIRV-Cross path counts guest queries;
see "Build 9596" below. The analysis that led there follows.

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
logs show. XeniOS HEAD's SPIRV-Cross path is the same (MSL source cache
only). Correction (2026-11-19): that path is not XeniOS HEAD's default.
`metal_use_dxil` defaults to true there, with `metal_shader_compiler =
spirv2air` (SPIR-V straight to AIR, submodule `xenios-jp/spirv2air`,
falling back to Metal Shader Converter, `cmake/ios/ShaderConverter.cmake`).
Everything that hides compiles is tied to that path (`UseDxilPath()`):
`MetalShaderLibraryCache` (per-title metallibs and bindings on disk, so a
later launch skips translation and the MSL front end),
`metal_bake_fetch_state`, the vertex shader ucode interpreter that draws new
geometry while its real shader compiles (`async_shader_vs_interpreter`,
`ucode_interpreter.vs.slang`), `async_shader_skip_draws` only for draws
without a stand-in, and `metal_stand_in_exact_depth_stencil`. Porting it
means porting the DXIL/spirv2air path. Only one iOS 26 guard was found in
its command processor. Unknown: whether spirv2air's AIR/metallib output
loads on iOS 16. A smaller alternative is MTLBinaryArchive for SPIR-V-path
pipelines (iOS 14+), which saves the GPU back-end compile but not the MSL
front end.

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

Bandwidth and pipeline diagnostics (owner's request, same switch,
commit "[GPU] Estimate attachment traffic, time per pipeline and compile
stalls"):
- "Metal GPU counter sets: ..." at startup lists every hardware counter the
  GPU offers. Apple GPUs are documented to offer only timestamps, so no
  memory traffic counter is expected; if one is listed, it can be sampled.
- Traffic estimate: per render pass, whole attachments loaded
  (LoadActionLoad) and stored (Store, multisample resolve output) from size,
  samples and format (stencil plane 1 byte, D32S8 depth plane 4). Per kind
  and per top pass ("attachments X MB loaded, Y MB stored") and a total line
  "Metal GPU estimated attachment traffic: ... GB/s over the window".
  Upper bound (Apple GPUs may compress attachments); texture sampling,
  vertex fetch, compute and blits are not included. For scale, the
  iPhone 14 Pro Max (A16) memory is LPDDR5 at about 51 GB/s.
- Per pipeline (VS/PS hash and modification): pass vertex time shared by
  vertices, fragment time by draws; exact when a pipeline has a pass to
  itself (many post-process passes). Flags from the pixel shader MSL:
  discard (defeats hidden surface removal), depth-output, raster-order, and
  its texture sample calls. Top 12 in "Metal GPU top pipelines per frame".
- Compile stalls (for step 2, skip vs bounded wait): every async shader and
  pipeline compile is timed from queueing to done, split into queue wait
  and compile; draws skipped for it are counted. Per window: compiles and
  draws skipped by pending-time bucket (<=16, 33, 50, 100, 250, 500 ms, 1 s,
  more): a bounded wait as long as a bucket limit would have drawn that
  bucket's skipped draws, at the cost of stalling. Each compile draws
  waited 100 ms or more for is noted on its own line ("Metal compile
  stall: ...").
- Build #9583 (`xenios_ios_arm64_0.2.9-9583`, commit 756f0db6, includes
  72d2ccc0 and f06c5c2e): iOS CI passes (the CPU tests workflow didn't run
  for it; no CPU code changed, and it passed on f06c5c2e). Not yet run on
  device; asked for 2 gameplay runs with timing on, 1 with it off, a force
  quit, and watching for stale UI or brightness.
- The previous commit's doc refresh had recounted block-only names in
  windowed_app_main_ios.mm across the whole file; fixed (see the doc's
  Checks). The refresh script must not be run on that file again as is.

Build 9583 results (owner, zip "As_many_runs_as_I_can_do", 6 logs: 4 with
pass timing off, 2 on; plus a force-quit .ips):
- FPS unchanged by the ring-empty fix: gameplay 150-220 ms per frame, menus
  35 ms. No new glitches reported.
- Counter sets: only "timestamp: GPUTimestamp" (no hardware memory
  counters on iOS 16 / A16, as expected). GPU tick = 1 ns.
- GPU time per gameplay frame (pass timing): rt transfer 60-156 ms (49-85
  passes), draws 13-30 ms (54-167 passes, 300-3500 draws, 0.8-3.4M
  vertices), texture load + upload blit 4-11 ms, resolve + edram dump 2-5
  ms. Top passes: rt transfer 640x1024 x4 D32S8 (load/store depth+stencil)
  ~45-75 ms for ~15 per frame; rt transfer 1280x2048 x1 D32S8 13-24 ms.
  Even a menu frame spends 7.25 ms on one 640x1024 x4 depth transfer.
  Cause: every depth transfer = stencil clear + depth draw + 8 per-bit
  stencil draws, each shaded per sample at 4x MSAA.
- Attachment traffic estimate up to 7.5 GB per frame, 43 GB/s (A16 ~51
  GB/s): draw passes load and store whole 1280x2048 targets (34 MB per
  pass for RGBA16F+D32S8); a menu pass with one 4-vertex draw = 0.53 ms.
- Top pipelines: nothing dominant at runtime (~2 ms each at most); one
  pixel shader with 64 texture samples (~1 ms, 1 draw); few use discard.
- Compile stalls: ~640 compiles per 120-frame window when gameplay starts,
  ~100k draws skipped, queue wait totals ~1,100 s vs ~97 s compiling;
  228 compiles pending over 1 s, oldest 10.8 s. Compile timings over all
  logs: median 300 ms per shader, of which spirv-cross ~1.6 ms, Metal
  library 0.3 ms after the first run (190 ms on the first, cold system
  cache), and ~290 ms unaccounted every run (784 s of 911 s).
  Found: AddEntryPointCandidatesFromMslSource ran two backtracking
  std::regex over the ~60 KB MSL; locally libc++ takes 72 ms on 57 KB
  (libstdc++ 18 ms) on a desktop core. Both upstreams still have it.
- Compile threads are plain std::threads (default QoS; their reported
  priority 16-17 is Darwin's decay of CPU-bound threads), 4 of them, ~52%
  CPU each. One guest thread (XThreadE10F) at ~94% of a core.
- Force quit no longer hangs, but crashed (SIGSEGV 0x191, main thread,
  EmulatorThread's exit lambda run from ExecutePendingFunctionsFromUIThread
  after the app was destroyed).

Changes after 9583 (each separately toggleable where it changes output):
- 1ff156a7 native stencil output in transfers (port of XeniOS/xenia-edge,
  probe at startup, `native_stencil_value_output`, Settings > Performance >
  Native Stencil Output, default on): one draw instead of ten per depth
  transfer. Shader checked locally as C++ with a stub metal_stdlib for 8
  define combinations (gen in scratchpad mslcheck/).
- adce620f entry-point regex only when no named candidate resolves;
  argument-encoder time added to the compile timings line; compile threads
  named "Metal Shaders N".
- 16eb1471 force-quit crash: exit notification not queued during OnDestroy,
  and OnDestroy runs queued UI functions after joining the emulator thread.
- 6797a3d8 compile queues FIFO within a priority (queue_arrival);
  `metal_async_compile_wait_ms` (Settings > Performance > Wait for Shader
  Compiles, default off): draws wait up to N ms per frame for compiles;
  perf line "draws waited for compiles N times, X ms".
- dfafc8eb `metal_render_pass_area_from_scissor` (Settings > Performance >
  Limit Render Pass Area, default off, experimental): draw passes limited
  to the largest scissor so far per descriptor (32 px), restarted to grow;
  not for passes clearing on load. Top passes show the limited size.
- 93f1892f same switch also limits transfer passes that load their
  destination to the bounding box of their rectangles (transfers' and the
  resolve clear's), with the full-target scissor clamped to it.
- d0e3a6e6 `log_guest_thread_hotspots` (Settings > Diagnostics > Guest
  Thread Hotspots, default on): every perf window, 16 samples 1 ms apart of
  every running guest thread, mapped to guest function / host symbol;
  "Guest thread hotspots <handle> (<name>): n/16 samples in guest code; ..."
Build #9593 (`xenios_ios_arm64_0.2.9-9593`, commit d0e3a6e6) has all of
the above; iOS CI passes for every commit in the series. Not yet run on
device. Test plan given: 2 default gameplay runs (FPS, blackouts), 1 with
Limit Render Pass Area on, 1 with Wait for Shader Compiles at 16 or 33 ms,
a force quit, and watching stencil effects and screen regions.
Build 9593 results (owner, zip "Wow_i_can_t_believe_it_s_not_native", 4
logs; runs 1-3 defaults with Wait for Shader Compiles toggled in-session,
run 4 Limit Render Pass Area on):
- Gameplay 57-90 ms per frame (was 150-220): "just over double" FPS. Title
  ~+5 FPS (15 -> 20), save select ~27 FPS. Menus still 35 ms.
- GPU per gameplay frame: rt transfer 24-26 ms (61-70 passes, fragment
  ~14 ms), draws 20-23 ms (108-122 passes, 1500-2500 draws, 1.9-2.7M
  vertices), texture load + upload ~7 ms, resolve + dump ~5 ms. GPU busy
  57-85%. Passes cost a fixed ~0.5-0.7 ms each even nearly empty (e.g. 28
  draws in 4 passes = 2.74 ms), ~180 passes per frame.
- Attachment traffic estimate reached 82-97 GB/s, above the A16's ~51
  GB/s: it overcounts (compression, or untouched tiles skipped); use it as
  a relative measure only.
- Limit Render Pass Area (run 4) didn't limit anything: top passes stayed
  1280x2048 / 640x1024. Cause: scissors are the whole surface and the area
  kept the session maximum per descriptor.
- Compiles: the regex fix worked (unaccounted ~1 ms, was ~290 ms). After
  each install the first run recompiles everything (Metal's system cache is
  per binary: 610 shaders, avg 436 ms); later runs avg 7-69 ms.
  argument-encoder p90 297 ms on cold compiles (sum 70 s across the logs).
  Blackouts now only at loads/transitions.
- Guest thread hotspots: the 98% thread (XThread13807, F8000078, start
  823EBAC8) spends ~60-80% of samples in 82A23160 and ~25% in 8229AC60.
  82A23160 is a spin-wait: mtctr; 8x "or r31,r31,r31" (db16cyc delay) and
  bdnz; then reads a flag at r29+0x2ABD and calls 82311598 (returns a
  field of the current thread). 8229AC60 loops calling it until a counter
  catches up (cmplw/blt). Reads as UE3's game thread waiting for the
  render thread (one frame ahead at most); the render thread (XThread13407,
  ~31-35%) mostly sleeps (__semwait_signal) and decodes audio
  (ff_wma_*). So the limit is the GPU, not guest CPU work.
- Owner: decals and shadows flicker (shadows always did), lighting on
  characters slightly off (AO?), FPS dips near NPCs even off-camera
  (culling?).
Changes (c0bf3f9e):
- `metal_shader_preserve_invariance` (default on, Settings > Accuracy >
  Consistent Vertex Positions): MTLCompileOptions.preserveInvariance. The
  SPIR-V has Invariant position and NoContraction everywhere; SPIRV-Cross
  emits [[position, invariant]], which Metal only honours with that option
  (iOS 14+); with fast math and the NoContraction optnone wrappers stripped
  (metal_spirvcross_strip_nocontract_optnone), positions could differ
  between a depth pre-pass and later passes: z-fighting flicker. Neither
  upstream sets it.
- Settings > Accuracy also: "Fast Shader Math"
  (metal_spirvcross_strip_nocontract_optnone) and "Exact 24-bit Float
  Depth" (depth_float24_convert_in_pixel_shader).
- Render area v2: needed height = min(scissor bottom,
  RenderTargetCache::last_update_height_used() (draw_extent_estimator_));
  per descriptor {this frame, previous frame} maxima instead of the session
  maximum.
- "Metal render area and queries" line: draw/transfer passes limited of all
  (average rows), restarts, occlusion queries per frame (counted via
  XE_GPU_NOTE_FAKE_OCCLUSION_QUERY hook in the PM4 implementation, defined
  only by the Metal command processor).
CI for c0bf3f9e (run 44296452338): Lint passed; "Build iOS ARM64" was
cancelled after 15 min queued with no steps and no log (no macOS runner
picked it up). Not a compile failure. Asked the owner whether to re-run.
Ideas not done: real occlusion queries (port of XeniOS's Metal ZPD with
the pre-iOS-26 readback path; see the occlusion section) if the query
count is high; gamma render targets are RGBA8 with manual PWL unless
metal_allow_gamma_unorm16 ("known issues") - a lighting suspect;
argument encoders from descriptors for cold-cache compiles.
Local checks: metal_render_target_cache.cc, msl_shader.cc, shim_utils.cc
(Apple branch forced on) and the command processor compile with clang on
Linux against the real metal-cpp/glslang/SPIRV-Cross/Vulkan headers
(submodules fetched with --depth 1; Apple system headers and generated
shader bytecode headers stubbed in scratchpad stubs/ and genstubs/). The
command processor's only error there is the Linux-only SpirvShaderTranslator
constructor parameter.
Next candidates: argument encoders from descriptors if argument-encoder=
is large; what the hot guest thread runs (from the hotspot lines).

### Build 9596 (c0bf3f9e, re-run by the owner) and the next build

Owner's runs (five zips, 2026-11-18): two default, two with Limit Render
Pass Area, one with Exact 24-bit Float Depth, two with Fast Shader Math
off. Render area v2 works (draw passes limited). Exact float depth changed
nothing; Fast Shader Math off is a regression (black environment, much
slower): both stay at their defaults. Flicker did not stop and is cyclic
(clearest on the title screen: moon, fog). Shadows depend on the camera: a
rectangular shadow map that orbits a point as the camera circles. FPS
rises when the camera faces Batman while NPCs walk beside the Joker. GPU
busy only about half the frame; about 170 occlusion queries per frame in
gameplay, all faked as visible.

Shadow hypothesis: UE3's modulated/projected shadows rebuild world
positions from the resolved scene depth (k_24_8 / k_24_8_FLOAT fetches); a
wrong or stale depth texture there gives exactly a shadow frustum that
moves with the camera. The 0.5 host scale of D24FS8 is undone (x2) in
every dump and transfer shader, so that is not it. Needs the frame detail
log (below) to see which texture the shadow pass reads and where it was
resolved from.

Changes for the next build:
- 80f3fc96: SPIRV-Cross uniform buffers (8 draws each) capped at
  `metal_uniform_buffers_max` (default 96 on iOS, was 24; 384 KB each,
  grown on demand); waits for a free one and forced command buffer ends are
  logged ("Metal uniforms over N frames"). Float constants copied per draw
  only up to the shader's float_count (was 4 KB per stage per draw).
- 27800f3f: real occlusion queries, experimental. Upstream XeniOS's
  `MetalZPDVisibilityPool` ported without the iOS 26 accumulate path
  (readback copy of each pass's slots at the pass end). Upstream's shared
  ZPD system was not ported (it needs the newer CommandProcessor); instead
  `MetalCommandProcessor::ExecutePacketType3_EVENT_WRITE_ZPD` overrides
  the packet like D3D12/Vulkan (XE_GPU_OVERRIDES_EVENT_WRITE_ZPD; the
  earlier counting macro hook was removed). Like upstream's default "fast"
  mode: begin zeroes the report; end writes a guess at once (the last count
  at that address, or 1) and the real count replaces it when the command
  buffer completes, only if the guest has not re-armed the address and no
  newer query used it. Queries with draws skipped for compiles keep the
  guess. A draw pass created before a query began is restarted with the
  visibility buffer. Log: "Metal occlusion queries over N frames".
- af206569: frame detail log (Settings > Diagnostics > Frame Detail Log,
  `metal_frame_detail_log`): choosing a number of frames during the game
  logs those frames: passes (guest RT registers, host attachments), draws
  (shaders, state registers, viewport, texture fetch constants raw with a
  "resolved" mark), resolves (copy registers, memory written), transfers
  (target and sources), occlusion queries, per-frame totals. Raw register
  values are logged; decode them offline with registers.h.
- 198fb502 (docs only): rows for the CPU test build script, stale CI rows.
- 028920a8: the iOS build of af206569 (run 44336513400, attempt 7) failed
  to link: new Metal sources must be listed in
  `src/xenia/gpu/metal/premake5.lua` (it does not collect the folder);
  `metal_zpd_visibility_pool.cc/.h` added there. Merged the owner's
  comment-only commit 1274515a (pushed to start CI) as d27a059a.
- Build #9606 (`xenios_ios_arm64_0.2.9-9606.6`, d27a059a, run 44361266692
  attempt 6) built and uploaded; it has all of the above.

### Build 9606 results (owner, zip "I_think_occlusion_query_definitely_helped")

Three runs: real queries off with a 4-frame capture; on with 16 frames; on
with 16+60+16 frames (the 60 span a cutscene into gameplay). Owner: the
title screen flicker is gone in every run (even with queries off); real
queries did not slow the title screen but seemed to affect gameplay.
- Real queries work: ~65% of gameplay queries come back with no samples,
  no guesses kept, one pass restart per frame. Title-screen GPU time looked
  ~45% higher with queries on, but every pass type (compute included) was
  ~45% slower in that run: thermal throttling (state 2), not the queries.
- Gameplay (queries on, 2x MSAA 1280x720 scene, ~3,300 draws, ~2.8M vertices,
  61 ms frames): the command processor waited ~21 ms per frame for uniform
  buffers and ended ~17 command buffers per frame for want of one (96 cap,
  8 draws per 384 KB buffer); the game's render thread spun at 99%. GPU per
  frame: rt transfers 25 ms in 62 passes, draws 21.6 ms in 110 passes (the
  1280x2048 RGBA16F+D32S8 target: 27 passes, 923 MB loaded and stored),
  ~6 GB attachment traffic. Limit Render Pass Area was off in all runs.
- Cutscene end: two frames skipped ~1,450 draws each for compiles
  (metal_async_compile_wait_ms was 0; 334 of 532 compiles finished within
  16 ms).
- Shadows (UE3 modulated shadows, ~9 per frame): per shadow the game clears
  depth through a 4x MSAA alias of the same EDRAM (UE3's fast clear; each
  alias switch is two full depth transfers), draws the casters' depth at
  tile 1440 (D24S8, 512x512), resolves it to 151A8000 (k_24_8), marks
  stencil with the shadow box (VS 6C5E1DC495A6B168, z test GreaterEqual
  against a just-cleared depth) and projects with VS B5A1208D037C2F4F /
  PS 6C76CC934779C668 (t0 = scene depth 15C78000 k_24_8_FLOAT from the 2x
  scene depth resolve, t1 = scene color 15540000, t2 = shadow map, t3 =
  64x64 noise). Texture formats, sizes and addresses match the resolves; the
  scene depth float load is upstream's shader. Not found yet; the shader is
  needed (metal_log_shader_hashes, below). The cutscene capture alternates
  frames with and without the shadow passes (gameplay frames have them
  every frame).
- Found: the 2x MSAA EDRAM dumps (resolves) read host sample 0 for the
  guest's top sample; draws, transfers and the host depth store use host
  sample 1 (upstream's GetD3D10SampleIndexForGuest2xMSAA). Sub-pixel for
  depth, so not the shadow bug by itself.

Changes for the next build:
- aea02862: 2x MSAA dumps swap the sample (color 32/64bpp, depth).
- d2f6d0d4: metal_inline_shader_constants (default on): constant blocks via
  setVertexBytes/setFragmentBytes when their version changes in the encoder;
  no pooled uniform buffer per command buffer or per 8 draws. Defaults with
  one-time config updates (kLastCommittedUpdateDate 2026-11-18 21h):
  metal_render_pass_area_from_scissor on, metal_async_compile_wait_ms 16.
- 1ff32888: metal_log_shader_hashes: listed shaders' microcode disassembly
  and MSL logged at translation ("Shader log"), and their float constants
  per draw in the frame detail log.
Build #9611 (`xenios_ios_arm64_0.2.9-9611`, 1ff32888, run 44391308505)
built on the first attempt and has all three.

### Build 9611 results (owner, zip "9611-1") and the next build

Three runs: defaults (queries off); queries on; queries on with
metal_log_shader_hashes set and captures (4 frames, then 1 facing Batman).
- Uniform waits gone. Gameplay ~44-57 ms per frame at 3,300-3,600 draws
  (61 ms in 9606), now GPU bound (busy 105-115%). Render area limits every
  pass: attachment traffic ~1.7 GB per frame (was ~6 GB). FPS rises as
  shaders compile instead of falling (owner). Real queries: NPCs take about
  a second to reappear when the camera whips around (owner).
- GPU per gameplay frame: rt transfers 43 ms in 91 passes (top: 640x384 4x
  and 1280x736 1x D24 depth, 20 each), draws 25 ms in 130 passes.
- Predicated tiling: the 2x MSAA 720p scene is drawn twice (scissor rows
  0-511, then 512-719 with the window offset), each resolved (with EDRAM
  clear) into one 1280x720 color (15540000) and depth (15C78000) texture.
  Then a full-screen draw (VS 6D769D8D32187155, PS E9E3FEBA4586548E,
  depth Always + write, samples both textures) restores scene color and
  depth into a 1x EDRAM target: oDepth.
- Shadow loop per shadow: stencil-only clear through a 4x alias of tile 0
  (DEPTHCONTROL 00008701), depth clears at tile 1440 (1x and 4x alias),
  casters, resolve to 151A8000, shadow volume (VS 6C5E1DC495A6B168: z-fail,
  front Incr, back Decr, GreaterEqual against the restored depth), then the
  projection (stencil NotEqual 0; PS 6C76CC934779C668: scene depth tf0 via
  c1, Z = 1/(d*c2.z - c2.w) (reversed Z, near 10, infinite far), shadow
  position = c3*xZ + c4*yZ + c5*Z + c6, 4 or 16 PCF taps on tf2, fade by
  distance c16-c22; multiplies the color (BLENDCONTROL0 01000008)).
- Root cause found: our SPIR-V translator wrote oDepth unscaled to
  gl_FragDepth, while float24 (D24FS8) host depth is kept at half the guest
  value (full_float24_in_0_to_1). The restored scene depth was twice what
  everything else stores, so shadow volumes marked each box's whole screen
  area (the rectangle that orbits the character) and later depth tests
  (decals, fog, particles) were off. Upstream XeniOS halves oDepth in
  CompleteFragmentShader_DSV_DepthTo24Bit.

Changes for the next build:
- ba065060: oDepth goes to a variable copied to gl_FragDepth at the end of
  the shader, halved when kSysFlag_DepthFloat24 is set (host RT path).
- c13223b5: "Frame detail resolve probe": during a capture, each depth
  resolve is sampled on a 16-wide grid (tiled addressing) once its command
  buffer completes.
Build #9615 (`xenios_ios_arm64_0.2.9-9615`, c13223b5, run 44420063578)
built on the first attempt and has both.
Owner's two more 9611 runs (zip "9611-2"): every captured frame has the
same shadow structure (one restore, 9-10 shadows each with resolve,
volume and projection). Frame time hardly follows the draw count (765
draws: 42-45 ms, GPU 112%): ~270 GPU passes per frame, rt transfers
33 ms, draws 18 ms; about half of the transfer time is pass overhead.
Next candidates: the 4x-alias stencil/depth clears cost two full depth
transfers each (~40 per frame); every resolve in a draw command buffer
commits it (~33 command buffers per frame); query reappearance latency.
Next candidates: recognize UE3's 4x-alias clear draws (full-target depth
write, always pass) and skip transferring the old contents into the alias
(~25 ms of transfers per frame in gameplay, ~27 ms on the title screen);
the forced command buffer split when a draw samples memory just resolved
(once per shadow).

Test plan sent to the owner: default run (title screen, then gameplay) for
the uniform line; same with Real Occlusion Queries on (FPS near the NPCs,
flicker, pop-in; the occlusion line); Frame Detail Log of 4 frames on the
title screen while it flickers, and 1 frame in gameplay with the shadow
visible.

### Build 9615 results (owner, zip "9615", 4 logs + 6 photos) and the next build

Runs: 19-03-24 (defaults, two 1-frame gameplay captures), 19-14-19 (shader
hashes listed; 1 frame, then 4 frames of the save select screen), 19-16-23
(two 1-frame gameplay captures), 19-20-16 (Real Occlusion Queries on).
Owner: shadows now have the caster's shape, but a faint box remains around
each (box edges show as a thin shadow-coloured line, the inside is slightly
darker); a doorway is no longer solid black; queries on recover from the
camera-transition blackout almost at once. New: close-up, Batman shows
differently shaded triangles; on the save select screen his knees are
almost black.
- Probe: the shadow map (151A8000, k_24_8) holds 1.0 where cleared and the
  casters' depths; the scene depth (15C78000) holds 0.02-0.08 (reversed Z).
  Both plausible, so the box is not bad depth data.
- Save select frame (41 passes): base pass at 2x MSAA (VS 709923, viewport
  offset 639.75/360.25), colour and depth resolved per tile, depth restored
  into a 1x target with oDepth (draw 32), then the per-light passes redraw
  Batman at 1x (VS D32E36, viewport offset 640/360) with GreaterEqual and no
  depth write, additive. Also a half-res UE3 ambient occlusion chain: depth
  downsample via oDepth (pass 18, PS 9E0981), occlusion into k_16_16_FLOAT
  (pass 19, PS 6209D0), blurs, multiplied into the scene (pass 26).
- Cause of the faceted/black triangles (and likely of the box edges): UE3
  shifts its 2x MSAA passes by (-0.25, +0.25) pixel so that Xenos sample 0
  lands on the 1x pixel centre; the restored depth then equals what the 1x
  light passes rasterize. Our host 2x samples sat at the Direct3D 10.1
  positions (+-0.25, +-0.25 diagonally the other way), so the resolved
  sample is half a pixel off: per triangle, depending on its slope, the
  light passes pass or fail the GreaterEqual test. Before 9615 the restored
  depth was doubled, so every light pass failed alike and nothing looked
  faceted. Upstream documents exactly this in `draw_util.h`
  (`kXenosSamplePositions2x`, "restored depth can fail subsequent depth
  tests and leave parts of the scene unshaded") and programs the Xenos
  positions on Direct3D 12 and Vulkan (`xenos_sample_positions`, default
  off there); neither upstream has it for Metal. The shadow volume's z-fail
  test and the projection's world-position rebuild read the same restored
  or resolved depth, so the box edge lines should shrink too (not proven).
- Fix (next build): `xenos_sample_positions` (Settings > Accuracy > Xbox 360
  MSAA Sample Positions, default on here, applies at once).
  `MetalRenderTargetCache::ApplyXenosSamplePositions` is called before every
  draw and render-target-transfer encoder: for 2x/4x passes it sets
  `setSamplePositions` from `draw_util::GetHostSampleXenosPositions` (native
  2x: host sample 0 at Xenos sample 1 (4,-4), host 1 at Xenos 0 (-4,4), the
  same pairing as the EDRAM dumps; 4x in guest order) and stores depth with
  `StoreActionOptionCustomSamplePositions` so later passes, transfers and
  dumps read plain per-sample values. Otherwise it resets to the default
  positions. All APIs are iOS 11+. Startup line: "MetalRenderTargetCache:
  programmable sample positions supported; Xenos MSAA sample positions
  used".
- Build #9618 (`xenios_ios_arm64_0.2.9-9618`, 10592e38, run 44436797777)
  built on the first attempt and has the fix. Not yet run on device; test
  plan given: the startup line above, Batman's knees on save select and
  close-ups in gameplay with the setting on and off (it applies at once),
  and a shadow's box with it on.
- Queries on (19-20-16): 10,000-12,000 queries per 120 frames, about 60%
  with no samples; gameplay frames 51-58 ms against 44-50 ms in the other
  runs (one run each, not conclusive).
- The projection pixel shader 6C76CC934779C668 was not logged in 9615 (the
  "Shader log" lines only appear when the shader is translated; it was
  compiled from the source cache). Its disassembly is in the owner's 9611-1
  zip, which is in the previous session's container only.
- Seen upstream, not ported: `GetHostDepthPolygonOffsetIfNeeded` (draw_util)
  moves UE3 decal depth bias into the pixel shader to stop coplanar
  z-fighting; a candidate if decals still flicker.

### Build 9618 results (owner, zips "9618_logs-1", "9618_pics", plus "9611-1" re-uploaded) and the next build

Owner: the MSAA sample positions fixed the character shading (knees, faceted
triangles). The shadow box remains. New: left in the pause menu long enough,
the menu backdrop's vertices explode and the scene behind turns to rainbow
bands (photos image, image1 = normal, image2). Owner also stresses that Real
Occlusion Queries fix the cutscene-to-gameplay transition; with them off,
parts of the scene keep flickering in and out after loading.

Shadow box, narrowed down:
- PS 6C76CC934779C668 (disassembly in 9611-1, log 17-54-07): rebuilds the
  receiver from scene depth (tf0, Z = 1/(d*c2.z - c2.w)), shadow position =
  c3*xZ + c4*yZ + c5*Z + c6 (xy divided by w, z not), 4 PCF taps on tf2
  rotated by the 64x64 noise tf3 (offsets c7-c14, about 2 texels), 16 more
  in the penumbra, lit = mean(map > min(z, 0.999)); distance fade to the
  shadow colour c16 (light at c17); out = lerp(shadowColor, 1, lit^3). A lit
  receiver gives exactly 1.0, and out-of-range UV/z all read as lit, so a
  line can only come from map texels that are not 1.0.
- Contrast-enhanced 9615 photos: the box is a thin shadow-coloured line along
  the shadow frustum's edges; inside the box is as bright as outside. So the
  PCF taps at the frustum edge read non-1.0 texels in the 5-texel border
  (casters are drawn at viewport 5..507 x 5..339 of 512x344; the clear covers
  all of it). The 9615 probe grid (every 32 texels from 16) never sampled
  the border.
- Lead, unproven: the 4x-alias fast clear's ownership range transfers back
  8-9 rows short of the shadow viewport (tiles 1440-1587 = 336 of 344 rows;
  1440-1636 = 448 of 457), so the bottom rows keep the 1x clear or older
  EDRAM contents. That explains one edge, not the L-shaped line seen.
- Next build logs, during a frame capture, for every 24-bit depth resolve:
  per edge, for the 8 outermost rows/columns, how many texels are not 1.0
  and their minimum ("Frame detail resolve edge"), and an 8x8-block map
  ("Frame detail resolve map": '#' all 1.0, '.' none, '+' some, '-' not
  written); and for draws of 12 vertices or fewer, the vertex data of the
  first binding ("Frame detail draw N vertices"), which shows the clear
  rectangles.

Pause menu corruption, cause found by reading (not yet confirmed on device):
- Shared memory is zero-copy on iOS (`metal_shared_memory_zero_copy`, the
  GPU reads guest RAM directly). The game learns the GPU is done with memory
  from EVENT_WRITE_SHD fences, which this branch stored on the CPU as soon as
  the command processor read the packet. Since 9583 nothing waits for the GPU
  in the frame loop, so the GPU can run up to the queue's in-flight limit
  behind (about 2 frames), and the game rewrites dynamic vertex buffers and
  textures the GPU has yet to draw with. Worst when GPU-bound with an idle
  CPU: the pause menu.
- Upstream XeniOS has exactly this fix (`TryWriteShaderDoneFence`, fences
  copied on the GPU timeline in zero-copy mode, `SubmitBeforeShortMemoryPoll`;
  also `CopyToBufferGpuOrdered` for the non-zero-copy upload path, and a
  held-signal system for interrupts). Ported the essentials: in zero-copy
  mode the fence is queued on the open command buffer and stored by its
  completion handler (or an empty command buffer committed at once when none
  is open); a short-delay WAIT_REG_MEM memory poll commits the open buffer if
  fences wait on it; ring-empty waits commit as before. Option
  `metal_gpu_ordered_fences` (Settings > Accuracy > Wait for the GPU at Game
  Fences, default on, applies at once). Not ported: the non-zero-copy path's
  GPU-ordered uploads (that path still memcpys into the live buffer) and the
  held interrupts. Risk: fewer frames in flight could cost some FPS where the
  game waits on a fence mid-frame.

Settings: "Frame Capture" is now the first section (Frame Detail Log); saving
when only it changed closes Settings without the "Settings Saved" alert.

All of the above is build 9620 (`xenios_ios_arm64_0.2.9-9620`, commit
c4fcde3, CI run 316 passed). Test plan given to the owner: leave the pause
menu open a few minutes with "Wait for the GPU at Game Fences" on, then off;
compare gameplay FPS with 9618; one 1-frame Frame Detail Log capture with a
shadow box on screen.

Owner's decisions (2026-11-19):
- Real Occlusion Queries stay off by default (`occlusion_query_enable =
  false`), like XeniOS's fake default, even though they fix Batman's
  cutscene-to-gameplay transition; the owner turns them on per game.
- Order of work: finish Batman: Arkham Asylum's accuracy issues (shadow box,
  pause-menu corruption), then its performance (without giving up accuracy),
  then other games with different issues.
- Before other games: per-game config.toml files. The loading side exists:
  `config::LoadGameConfigForFile` (`src/xenia/config.cc`, called from
  `xenia_main_ios.mm` at launch) overlays
  `<config folder>/config/<TITLE ID>.config.toml` on the main config. Missing
  on iOS: a way to create and edit it (likely an entry in the game's
  long-press menu, opening the settings screen against that file, and a
  marker in the settings for values the game overrides). Check XeniOS HEAD
  and xenia-edge (`game_config_dialog_qt.cc` exists here, desktop only) for
  an existing design first (standing rule 10).

### Build 9620 results (owner, zip "9620": logs 23-22-24 and 23-26-13) and the next build

- Pause menu: fixed. "Wait for the GPU at Game Fences" works as intended with
  no major FPS cost (owner). Cause confirmed.
- First run: while shaders compile, the scene (mostly the floor) pops in and
  out when moving, consistently in the same section (owner). Log 23-22-24:
  draws skipped for pending compiles reach 187,571 of 285,029 in one
  120-frame window (`compile_pending`). This is the async compile skip
  (`metal_async_compile_wait_ms`), not the shadow or fence bugs; it belongs
  with the performance work (persistent pipeline cache, ideas 1-3 below).
- Shadow box, log 23-26-13 (1-frame capture): every shadow map's border reads
  0.0 (fully shadowed) instead of the clear value 1.0, which is what the PCF
  taps at the frustum edge sample. Shadow map 1 (draws 512-519): clear
  viewport 512x475, depth cleared with Z=1 by a 1x rectangle list (x 480-511
  rows 0-474, and x 0-479 rows 464-474; DEPTHCONTROL 00000076, Always +
  write) and a 4x alias rectangle (240x232 = x 0-479 rows 0-463); casters at
  viewport/scissor (5,5)-(507,470), DEPTHCONTROL 00700736 (LessEqual); the
  resolve is clamped to that scissor and aligned to (0,0)-(512,472).
  Observed: 1.0 survives only inside the caster scissor. Columns 0-4 and
  507-511 and rows 0-4 and 470-471 are never 1.0, even where only the 1x
  clear rectangle covered them. Where the 4x rectangle cleared, columns and
  rows 5-7 are also lost. Not explained yet; the render pass area (32-aligned,
  512x480 here) is not it. Caveat: the 9620 probes read memory when the
  frame's command buffer completed, after later shadow maps (and a 352x192
  colour resolve at draws 721-723) had overwritten 151A8000, so the maps are
  a mix of several shadow maps. The border pattern is the same for all of
  them.
- Next build (frame detail log only): each depth resolve's written range is
  copied by a blit right after the resolve (the probe reads that copy), and
  for 1x depth surfaces so are the EDRAM tiles it read (the dumped host
  render target). New lines: "Frame detail resolve texels" (raw depth at six
  border/inner positions, resolved and EDRAM), and "Frame detail edram
  edge/map" next to "resolve edge/map". If EDRAM has 1.0 in the border and the
  resolve doesn't, the resolve is wrong; if EDRAM lacks it too, the clears or
  the 1x/4x transfers are. Built as 9623 (`xenios_ios_arm64_0.2.9-9623`,
  commit 4868e78, CI run 317 passed).

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

### Build 9623 results (owner, zip "9623": Batman logs 00-12-48 and 00-15-41, Spider-Man debug log 00-17-27) and the next build

- Shadow box: the EDRAM the resolve read already has 00000000 (depth 0,
  stencil 0) in the border, so the dump and the resolve are faithful; the
  border is lost before. Per shadow map, 1.0 survives exactly inside the
  casters' scissor (e.g. (5,5)-(507,282)); in the part the 4x alias rectangle
  cleared, rows/columns 5-7 are lost too. Elsewhere the values are left over
  from the scene colour that the colour-to-depth transfer (from the
  k_2_10_10_10_FLOAT target at 1024t, "host depth") put there (00000000,
  45D9E400, 40BCD400 ...). So the game's depth clears (draws 460/461:
  rectangle lists, VS 1E6883FCCDE1F688, no pixel shader, colour mask 0,
  DEPTHCONTROL 00000076, Z = 1) do not land, or do not survive, outside the
  casters' scissor. Not found by reading: render areas are 32-aligned and
  cover the border; the draw scissor is set per draw.
- Next build: during a frame detail capture each 24-bit unorm depth pass
  (shadow maps) is dumped to EDRAM as it starts (after its transfers) and
  logged ("Frame detail pass N start edram edge/map", EDRAM sample
  coordinates = 1x pixels): pass 12 start shows the 1x clear rectangles
  (through the 1x->4x transfer), pass 13 start the 4x clear (through the
  4x->1x transfer), the resolve the casters.
- Spider-Man (debug log): the loader thread opens D:\packs\game.XEPACK with
  create options 0x48 (FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_
  BUFFERING, no synchronous flag: an async handle; earlier files used 0x64,
  synchronous), queries it (class 0x22) and waits forever. Reads are not in
  the log (NtReadFile is high-frequency). Our NtReadFile completes async
  reads inline, returns PENDING, signals the event and queues the completion
  routine as a user APC, which runs only in an alertable wait. XeniOS HEAD
  queues it as a kernel APC that runs before the service returns (bit 0 of
  the routine asks for a user APC), via `QueueIoApc`/`xeProcessKernelApcs`,
  plus its request queue. Likely cause: a completion routine that never runs.
  Next build logs every async NtReadFile/NtReadFileScatter at debug level
  (event, APC, context, offset, length, status, bytes) to confirm.
- Built as 9628 (`xenios_ios_arm64_0.2.9-9628`, commit 9c1e6c5, CI run 318
  passed), with the crash dump and wait-handle additions.

### Build 9628 result (owner, log 05-54-49): shadow box cause, fix in the next build

- Pass-start EDRAM probes, first shadow map (512x266, caster scissor and
  viewport (5,5)-(507,261)): pass 10 start (after the colour-to-depth
  transfer) has no 1.0 anywhere. Pass 11 start (after the 1x clear,
  through the 1x->4x transfer) has 1.0 only in the clear rectangles
  intersected with the caster rectangle: x 480-506, rows 5-260. Rows 256-265
  of the second 1x rectangle, which are never transferred, also have 1.0
  only for x >= 5 and rows <= 260. Pass 12 start (after the 4x clear) adds
  1.0 from row/column 8. So the clear draws themselves are cut, before any
  caster is drawn, although their guest scissor is the full 8192x8192 and
  their guest viewport the full map.
- Cause found by reading: IssueDrawMsl clamped the draw scissor only to the
  bound render target texture (560x4688 here), while the render pass is
  limited (metal_render_pass_area_from_scissor, on by default since 9611) to
  560x736 (1x) and 280x512 (4x). Metal requires the scissor within the
  render pass; the clears' scissor (560x4688, 280x2344) was invalid, and on
  device it ended up as an earlier, smaller one. BeginCommandBuffer already
  clamped its own scissor to the area. Matches the timeline: the box was
  first seen in 9615, after area limiting became the default in 9611.
- Fix (next build): the draw scissor is clamped to the render area too (MSL
  path and the macOS MSC path). Frame detail draws also log the host
  viewport, scissor and render area, PA_CL_CLIP_CNTL, PA_CL_VTE_CNTL,
  PA_SC_SCREEN_SCISSOR and PA_SC_WINDOW_OFFSET ("Frame detail draw N host").
  Quick check on 9628 without a new build: Settings > Performance > Limit
  Render Pass Area off should remove the box.
- Confirmed (owner, 9628 with Limit Render Pass Area off, log 06-05-14):
  the boxes are gone. Every pass has area 0x0, and every shadow map
  resolve has 1.0 (FFFFFF00 in EDRAM) in its left, right and top border and
  in the bottom border rows of the map (e.g. map 1, cleared rows 0-315: rows
  312-315 all 1.0). The only non-1.0 rows left are those the resolve rounds
  up past the map (316-319), which the projection never samples.
- The fix is build 9632 (`xenios_ios_arm64_0.2.9-9632`, commit d280cd6, CI
  run 319); to check with Limit Render Pass Area back on.

### Build 9632 results: shadow box fixed; performance, first round

Owner: the box is gone with Limit Render Pass Area on; everything works.
Shadow box closed. On to performance. Owner: 15-19 FPS looking at NPCs,
24-25 FPS turning about 180 degrees away.

Logs 06-24-31 and 06-29-19 (gameplay windows of 120 guest frames):
- Two regimes. Facing NPCs: ~3,300 draws and ~2.75M vertices per frame,
  frame 46-48 ms (some windows 54-62 ms). Facing away: ~750 draws and
  ~0.7M vertices, 42-45 ms. Render target transfers are the same in both
  (~67-69 passes, ~34 ms of pass time per frame, ~190-220 MB loaded and
  stored); draw passes 21-22 ms vs 18-19 ms (vertex 5.8 vs 2.3 ms). Pass
  times overlap on the GPU (they add up to 60-70 ms in a 45 ms frame).
- Guest thread F8000078 (XThreadC20B, the game's render thread) uses
  99-100% of a core in every window. Facing away most of its samples sit at
  82A23160 (a wait loop): the frame is GPU-bound there. Facing NPCs its
  samples are spread over real work (8230B3D0, 82309CC0, 822FD...): the
  frame is CPU-bound on that thread's JIT code. GPU Commands goes from
  ~31% to ~55-59% of a core.
- Menus and idle scenes run at exactly 35.2 ms per frame, not 33.3: off
  Windows the frame limiter slept a full 16.67 ms after each vblank, so
  every vblank came ~0.9 ms late (60 Hz became ~57 Hz, Batman's 30 FPS cap
  28.4 FPS).

Changes for the next build:
- Vblank pacing ported from XeniOS: vblanks at fixed guest-time deadlines,
  `threading::DeadlineSleep` (mach_wait_until on Apple) to the next one.
  D1MODE_V_COUNTER still advances once per vblank. Expect menus at 33.3 ms.
- Context promotion ported from XeniOS (`ContextPromotionPass`, range-keyed
  tracking, `context_promote_vec128`, default on as upstream): vector
  registers stay in host registers within a block; dead context stores are
  removed by byte range, loads that survive keep earlier stores, and GPR
  stores before guest memory accesses are kept. This branch had disabled
  VEC128 promotion after the "no 3D models" NaN bug (see the section near
  the top); the old pass tracked values by base offset only, an aliasing
  hazard the new one fixes, but whether that was the cause is not proven.
  Settings > Performance > Keep Vector Registers in CPU turns it off
  (applies to code compiled afterwards; restart the game). Check the title
  screen's 3D models and gameplay first.
- Next after this: the render thread's hot functions (longer sampling), and
  the GPU's render target transfers.
- Built as 9635.2 (`xenios_ios_arm64_0.2.9-9635.2`, commit cf82827, CI run
  320 attempt 2; attempt 1's iOS job was cancelled after 15 minutes without
  a runner). This workflow has no CPU test job, so the new pass is only
  checked on the device.

### Build 9635.2 results: vector promotion breaks Batman; cause and fixes

Owner (zip "Vector_promotion_broke", logs 08-00-44, 08-01-51, 08-27-53):
with Keep Vector Registers in CPU on, the title screen's 3D models are
gone (about 10k vertices per frame instead of about 1M), the loading screen
misses its tip text and Loading/Skip icons, and gameplay is a black screen
with a stuck looping sound while the music plays. Off (run 2) everything
works. The config dumps at the top of the logs do not show the in-app value.

Cause, from xenia-edge's history (fetched deeper: has207/xenia-edge, 9459
commits; this branch's a64 sequences match its 74f34818e/7eb0c7671 of late
May 2026): turning VEC128 promotion on lets constant propagation see
through vector registers, so ops receive constant vector operands they never
got before. xenia-edge hit the same in September 2026 ("Promote VEC128
context values again" 01dc16a6, reverted 0e1307bd) and fixed:
- HIR folds: Extract read the wrong lane (no VEC128_B/W flip;
  5ce7c336d, "fixes rendering issues in FIFA games and State of Decay caused
  by enabling context_promote_vec128"); float VectorCompareEQ folded bitwise
  (920f35c93); the int16 vperm fold half (63ea6e272, already right here).
- a64 sequences that load a constant operand into scratch v0-v3 and then
  overwrite that scratch before the last read (e6a86116f): DOT_PRODUCT_3/4
  (vmsum3fp/vmsum4fp, the vertex transforms), VECTOR_ROTATE_LEFT,
  VECTOR_DENORMFLUSH, SHL_V128/SHR_V128.
- Also found here by an audit script (scratchpad tools/scratchaudit.py):
  STVL/STVR computed the address in x0 and then loaded a constant source,
  whose LoadV128Const uses x0 as its GPR scratch: a store to a wrong
  address. The constant is now loaded first.
- Taken too: SELECT_V128 with dest aliasing a data operand uses BIT/BIF
  (ef4b3a466), compare merging compares constants by raw bits (eddcd8f31),
  and ROUND in the dynamic mode is no longer constant-folded.
- CPU tests: `src/xenia/cpu/testing/constant_operand_test.cc` (with
  xenia-edge's RequireFoldMatchesBackend helpers in util.h) runs each fixed
  op with constant operands against the same op on registers; the CPU tests
  workflow (cpu-tests.yml, macOS ARM64) runs them on push. The earlier run
  for cf82827 (promotion on) passed all 249 HIR tests and 141,712 PPC cases:
  those tests never feed constants, which is why they missed it.
- `context_promote_vec128` now defaults to off here until the device
  confirms the fixes (the owner turns it on to test).
- Built as 9637 (commit b290a9b5f; CI run 44537599866, after five attempts
  cancelled without a macOS runner). The owner confirmed the fix: models,
  loading screen and gameplay are back with the setting on.

Owner's decisions (2026-11-19, after 9623):
- Include the profiling/stability tools now (crash dump, wait objects);
  the owner sends logs of the other games when possible.
- Main focus stays Batman: Arkham Asylum's major bugs and performance until
  past the intro into the next cutscene (then clear until the open map).
- Aim high: try a backport of XeniOS HEAD's async shader compilation (its
  DXIL/spirv2air path, shader library cache, VS interpreter stand-ins) for
  performance and to avoid issues with effects such as the rain and
  detective mode. Goal: the intro from ~20 FPS to 28-30 FPS.

### Build 9637 results: promotion fixed, no FPS change; the GPU is the limit

Owner (zip "9637-1", logs 11-56-55, 12-04-14, 12-09-24, 12-12-57; each run
into gameplay, facing the NPCs, then away, then sweeping the room): the fix
works, but performance felt worse, not better. Runs 1-2 with Keep Vector
Registers in CPU on (1 to let shaders compile), run 3 as a reference with
it off, run 4 also with Real Occlusion Queries on: almost no blackout in the
loading transition, and 21-22 FPS whether facing the NPCs or not.

Settings actually in effect: the config dump shows the file at app start,
not later changes in the app. The occlusion log line shows the live value:
queries were counted in both 12-09-24 and 12-12-57 (runs 3 and 4). The
next build logs "Settings at launch: ..." just before the title starts.

Steady gameplay windows (no compiles, max under 200 ms; ms per frame):
| Log | Owner's setup | Facing NPCs | Facing away | Thermal state |
|---|---|---|---|---|
| 11-56-55 | promotion on | 51.3 (19.5 FPS) | 47.7 (21.0) | 2 |
| 12-04-14 | promotion on | 48.5 (20.6) | 42.2 (23.7) | 1 |
| 12-09-24 | promotion off, queries counted | 47.7 (21.0) | 45.0 (22.2) | 2 |
| 12-12-57 | promotion off, queries on | 48.6 (20.6) | 42.5 (23.6) | 2 |
Build 9632 (no promotion, no queries): 46-48 facing NPCs, 42-45 away.

- The differences are within run-to-run noise and follow the phone's
  thermal state (iOS ProcessInfo: 2 = serious, throttled) more than any
  setting. The four runs were back to back in 16 minutes; 3 of 4 ran hot.
- GPU busy 98-112% of the time in every gameplay window (pass times
  overlap): GPU-bound. The render thread F8000078 also stays at 99% of a
  core, so a CPU gain (vector promotion) cannot show until the GPU work
  drops.
- Per frame: render target transfers 32-37 ms of pass time (~68 passes,
  ~200 MB loaded and stored), draw passes 19-23 ms, total attachment traffic
  ~1.2-1.3 GB. The biggest items are depth transfers between the same EDRAM
  data as 1280x736 1x and as 640x384 4x MSAA (UE3's half-resolution trick):
  ~12-13 each way per frame, 9-12 ms and 5-9 ms.
- Real occlusion queries cost nothing measurable (about one draw pass
  restarted per frame; ~150 queries per frame facing the NPCs, about half
  with no samples) and fix the transition blackout. Issued draws do not
  drop with them, so they are an accuracy win, not a speed-up here.

Changes for the next build:
- `skip_unchanged_render_target_transfers` (Settings > Performance > Skip
  Unchanged Render Target Copies, default off; applies at once). Ownership
  ranges remember a render target whose copy is still the current one
  (`OwnershipRange::intact_render_target`): set when a range changes owner
  by a transfer and nothing writes it, cleared by any draw writing depth,
  stencil (non-KEEP op with a write mask) or color to it, and by resolve
  clears. Taking the range back then needs no transfer. Such render
  targets are kept by ClearCache.
- Logged either way: "Metal render target transfers over N frames: X ranges
  went back to a render target whose copy was unchanged, Y of them
  skipped". With the setting off X shows how many transfers it would save;
  if X is 0, the 4x passes write depth or stencil and this does not help.
- "Settings at launch: ..." log line (vector promotion, occlusion queries,
  native stencil output, the skip).
- Asked of the owner: one run with the skip off and one on, same route,
  starting with the phone cool (thermal state is in every perf line).
- Built as 9640 (`xenios_ios_arm64_0.2.9-9640`, commit cd16c089f, CI run 324;
  runs 322 and 323 were cancelled while queued so one build carries all
  three changes).

### Build 9640 results (owner, zip "All_logs": 3 Batman, 3 Spider-Man, 1 AC)

Batman (logs 13-50-39 and 13-53-37 skip off, 13-56-00 skip on; all with
vector promotion and real occlusion queries; thermal state 1):
- Owner: nothing broken, feels the same. Facing NPCs ~46-49 ms (20-22 FPS),
  away ~43-44 ms; the skip-on run's last windows were worse (55-60 ms, rtx
  40-45 ms, 1.7 GB per frame) in what looks like another part of the room.
- skip_unchanged_render_target_transfers: about 1 range per frame went back
  unchanged (120 per 120 frames), not 12. It does not help Batman; leave it
  off.
- Why, from the 9632 frame detail log (06-29-19): per light, UE3 draws (1)
  1x depth-only: the light volume with stencil (zfail incr/decr wrap, no
  depth write), (2) 1x color: the light with a stencil test, (3) after a
  1x->4x transfer of the main depth, one full-screen triangle on the
  640x384 4x alias with z off, stencil ALWAYS/REPLACE ref 0: a stencil
  clear at a quarter of the pixels, then (4) a 4x->1x transfer back. The
  4x pass writes stencil, so the range is not unchanged; depth is. The
  same goes for the shadow depth at 1440t. Main-depth round trips cost
  ~8-11.5 ms (to 4x) + ~5-9 ms (back) of pass time per frame.
- What could still help: a stencil-only transfer back when the depth is
  unchanged (like XeniOS HEAD's KeepDepthOfStencilRoundTrips, which covers
  a depth -> 8_8_8_8 -> depth round trip): maybe 2-4 ms of pass time.
  Turning the whole round trip into a stencil clear of the 1x target would
  save more but needs knowing the 4x pass is only that draw. XeniOS HEAD's
  render target cache also has ElideRedundantTransfers (same idea as the
  skip), deferred clears, DontCare loads for pending passes, memoryless
  transient targets and direct resolve refreshes; ours has none of them.

Spider-Man (13-59-31, 14-00-51, 14-13-11 debug):
- The APC fix works: the loader reads game.XEPACK and many more packs
  (Q07, P19, P18, P20, P05...), past Press Start and the sign-in prompt.
- Then the game sits on a loading screen at 30 FPS (~105 draws per frame,
  music streaming from SOUNDSRC_RVB.PCK on F800005C) and later shows
  "Please Wait". After XamShowSigninUI it only reads profile settings and
  sets two contexts; no XamContent*, no XamShowDeviceSelectorUI, no
  XamTaskSchedule. Game Master (F8000008) sleeps in a job wait
  (KeDelayExecutionThread at 82B1A684, stack 829635F0 82963664 82962E30
  8292FF54 829168A8 822ED134 8290B4D4); JQ worker 1 (F8000018) renders
  (spins in D3D at 82AC0C10 between frames); the loader waits for work.
- The upstream compatibility report (xenia-project/game-compatibility
  #916, state-playable) has "endless loadings" fixed on Canary by enabling
  VSync; guest_display_refresh_cap is already true here.
- Next build logs, at info level, every notification the game takes
  (XNotifyGetNext) and each change in what XamUserGetSigninState returns,
  the two high-frequency calls a title polls while it waits.

Assassin's Creed Revelations (14-01-51, debug and frame detail):
- The save fix works: the name prompt returned, XamContentCreateEx and
  XamContentClose ran for the new save, no host exception.
- Flicker: the game ends 5 occlusion queries per frame (counts 230400,
  71489, ~199000, 921600, ~36100: sizes such as 1280x720 and 190x190, a
  sun or glare test). The guess written at a query's end was the last
  count at the same address, but the game takes its reports from a
  rotating pool, so an address often held another query last time: one
  frame guessed 36100 for a query that counted 198944. If the game reads
  the report before the correction, the effect flickers.
- Next build: a second guess, the last count at the same place among the
  frame's query ends (zpd_results_by_ordinal_), used when it has been the
  closer of the two lately (zpd_ordinal_score_). The occlusion perf line
  says which guess is in use; frame detail shows both.

Built as 9642 (`xenios_ios_arm64_0.2.9-9642`, commit bb1239c05, CI run 325):
the occlusion guess by place in the frame and the Spider-Man notification
logging.

### Depth buffer investigation: GPU trace capture (owner's request, 2026-11-19)

The owner wants to understand the depth buffer as a whole before choosing
faster routes, without game-specific pattern recognition, and ideally with
the route picked per situation. Plan agreed: step 3, a GPU trace of a few
Batman frames, replayed offline through the render target ownership logic
under different rules (copies skipped when unchanged, stencil-only copy
back, lazy copies, MSAA samples stored as plain pixels so 1x/2x/4x views
share a texture, XeniOS HEAD's transfer optimizations), to measure what each
would remove before writing Metal code. The per-copy rules are cheaper
whenever they apply, so they need no choosing; only the storage choice
(samples as pixels against native MSAA) is a trade-off, to pick per buffer
from recent use, with hysteresis.

- The trace writer was compiled out of Release (NDEBUG). trace_writer.h now
  keeps it on iOS (XE_OPTION_GPU_TRACE, the XeniOS name; XeniOS sets it from
  a CMake option). Cost per upstream's comment: 0.4-0.6% CPU.
- Its packet addresses were the host pointer truncated to 32 bits, which is
  only right when physical memory starts at a 4 GB boundary (here it is
  0x...A0000000): ported XeniOS's CommandProcessor::GuestReadPtrOffset.
- Settings > Frame Capture > GPU Trace (metal_gpu_trace_frames: Off, 2, 4,
  8 frames): at the next swap, a streaming trace into
  Documents/scratch/gpu/<date_time>/<title>_stream.xtr, closed after N
  frames. Log lines "GPU trace: recording...", "writing...", "done".
- The Metal backend does not record its memory reads (textures, vertex
  buffers), so the trace cannot be replayed to render; it has every packet
  and the full register state, which is what the analysis needs.
- Offline: a parser for the .xtr (header, commands, snappy) and PM4 packets
  into a per-draw register timeline, then the ownership simulation.

### Build 9643 results (owner, zip "9643_runs": 2 Batman with GPU traces, Spider-Man, 2 AC)

Build 9643 is c3b49c6f6 re-run (GPU trace capture works: two Batman traces,
4 frames each, ~17 MB, in Documents/scratch/gpu/<date_time>/).

Assassin's Creed Revelations (15-56-59 crash, 15-58-45 crash at the prologue
to chapter 1 transition; owner: the flicker is gone with the place-in-frame
occlusion guess):
- Both crashes are on guest thread F80001B0 and hit corrupt heap memory:
  (1) PC 833378E0 (the game allocator's free-list unlink, called from
  8232121C, the same site as the 9618-era crashes), node 0x44A877A8 whose
  fields are all 0xFFFFFFFF, so `stw r11,0xC(r10)` writes to -1+0xC;
  (2) PC 824C437C, object 0x43D458B8 with a null vtable (lwz r10,0(r31) ->
  lwz r9,0(r10)).
- The game reserves 0x40000000 (1 MB) and commits it at fixed bases, and
  allocates/releases 64 KB X_MEM_NOZERO blocks (type 0x20803000) at
  0x40400000-0x43AFFFFF all the time; both corrupt objects are among them.
  The GPU can't have written there (0x40000000 virtual has no physical
  alias), so the writer is guest code or a kernel export.
- Found and fixed (not proven to be this crash): NtAllocateVirtualMemory
  judged a commit at a base by its first page: committed -> nothing zeroed
  (new pages after it kept stale data), uncommitted -> the whole range
  zeroed (live pages after it wiped), and it zeroed after the commit
  without the lock. Now, as in XeniOS HEAD, only the pages the call newly
  commits are zeroed, under the global lock. In these logs no commit at a
  base overlapped committed pages, so this may not be the cause.
- Diagnostics for the next crash: the dump now lists every register that
  points into a guest heap (allocation, state, 32 bytes there) with the
  last NtAllocateVirtualMemory/NtFreeVirtualMemory calls that covered it
  (1024 kept), to see a block freed or handed out twice while in use.

Spider-Man (15-54-56): notifications at start were XN_SYS_UI 0 and
XN_SYS_SIGNINCHANGED 1 while XamUserGetSigninState(0) returned 0; after the
sign-in prompt UI 1, SIGNINCHANGED 1, UI 0 and state(0) 1. Then the same
wait. Worker F8000064 (start 82A25CE8) waits on auto-reset event F8000060,
which was signaled ~96,000 times before sign-in and never after.
- The startup SIGNINCHANGED now carries the users actually signed in
  (XeniOS HEAD does), not a fixed 1.
- The first kernel call storm report of a thread now dumps the guest code
  around its caller and every return address up the stack (64 words before,
  8 after, once per address), to disassemble what Game Master's job wait
  is waiting for. Disassembler: capstone (PPC 32-bit big endian) in the
  session scratchpad's pylib.

Batman: two traces (15-50-11, 15-53-37); offline parser in the session
scratchpad (tools/xtr_parse.py; not in the repository).

### Async shader compilation backport: first scoping (2026-11-19)

- spirv2air (`xenios-jp/spirv2air` 84638ce, BSD-3, no LLVM, writes its own
  bitcode and metallib) targets Metal 3.1 with AIR 2.6+ and floors iOS at 18
  (AIR 2.7, `air64_v27-apple-ios18.0.0`); requests for older iOS are raised to
  18 (docs/air.md "Platforms"). iOS 16 has Metal 3.0 (AIR 2.5), so its
  libraries most likely will not load as they are. Needed: an AIR 2.5 /
  Metal 3.0 output mode (header fields, triple, `air.version`, language
  version) and a device test that `newLibraryWithData` accepts it, plus a
  check that no 3.1-only intrinsic is emitted.
- The Metal Shader Converter fallback needs Apple's iPhoneOS
  libmetalirconverter (`cmake/ios/ShaderConverter.cmake`); this repo has only
  the headers and CI has the copy step commented out.
- The library cache and the VS interpreter stand-ins are tied to that path
  (`UseDxilPath()`), with its binding layout.
- spirv2air's target is set in a few places: `mtlb::air_version` and
  `make_target` (src/metallib/metallib.cpp, floor iOS 18), `Version
  language{3, 1}` (metallib.h), and `air.version`/`air.language_version`
  metadata (translate.cpp 1397-1434, airkit.cpp 523). An iOS 16 mode (AIR
  2.5, Metal 3.0, file version 2.6, triple `air64_v25-apple-ios16.0.0`) is a
  small patch; whether iOS 16's GPU compiler accepts the emitted IR (LLVM 13
  layout bitcode, Metal 3.1 era intrinsics; 32-bit atomics are noted as
  relaxed-only per Metal 3.1) can only be tested on the device. It needs
  CommonCrypto (SHA-256 of the bitcode), which iOS has.
- Size of the port: XeniOS HEAD's Metal backend has diverged a lot from this
  branch. Files only upstream: dxil_shader, metal_dxil_binder,
  metal_shader_library_cache, metal_pipeline_archive, metal_buffer_hazards,
  metal_compile_activity, metal_upload_cache, metal_transfer_spirv_cross,
  metal_tessellation_shaders, ir_runtime_impl.cc, shader_cache_version.cmake,
  CMake build files (this branch builds the Metal backend with premake).
  Headers: metal_command_processor.h 1929 lines upstream vs 985 here,
  metal_render_target_cache.h 1314 vs 549, metal_texture_cache.h 584 vs 282.
  So "the async compile backport" is close to adopting upstream's Metal
  backend, not a contained change.
- Suggested first step: a probe build that links spirv2air with an iOS 16
  mode, translates one of our SPIR-V shaders at launch, loads the metallib
  (`newLibraryWithData`) and creates a pipeline, and logs each step. Only
  if that works does the port make sense.
- Steady state is not compile-bound: in 9623's gameplay windows the frame is
  ~40 ms with the GPU busy 110%, no compiles, and per frame ~31 ms of GPU in
  render target transfers (67 passes, ~213 MB loaded and stored) against
  ~17 ms of draws. The async path mainly removes the first-run popping and
  hitches; the path to 28-30 FPS is cutting transfers (shadow loop 1x/4x
  aliasing and colour-to-depth, predicated tiling) and attachment traffic.

## Profiler and stability tools (owner's request 2026-11-19, outline only)

Owner: two other games are unstable, one with an access violation, one stuck
in an endless load (unrelated to Batman). Wanted: a profiler to run with the
emulator that points at what needs work.

Already in this branch (corrected after reading the 2026-11-19 game logs):
perf lines every 120 frames (frame times, GPU busy, CPU waits on the GPU,
compiles, memory, thermal state, CPU share per thread), GPU time per pass
and compile stalls, Frame Detail Log; `log_guest_thread_hotspots` (default
on: 16 samples 1 ms apart per busy guest thread each perf window, guest
function or host symbol); the kernel call repeat detector
(`kernel_call_repeat_threshold`), which with the reports at
TerminateCurrentGame/OnDestroy logs every guest thread: what it is inside or
last called and for how long, wait object type and name, guest lr, a guest
stack walk (12 frames), host pc and symbol, and guest code words for threads
in guest code; the guest crash dump (`Emulator::ExceptionCallback`: guest
PC, host fault address, registers). Gaps: the crash dump prints the host
fault address only (no guest address, page state or code at the PC) and
relies on the thread report that happens to follow; multi-object waits show
only the count, not the handles; kernel call results are never logged
(shim_utils.h "TODO: log result"); hotspots are 16 samples per window.

Upstream: MicroProfile zones (`XENIA_ENABLE_PROFILER`, off by default,
covers host code only); a crash reporter extension (iOS 27, unusable here)
and `ios_diagnostics*`, which upload to an endpoint (not wanted).

Proposed, in this order (items 1 and 2 partly in the build after 9623:
crash dump with guest address, page region, LR, guest stack and code; the
handles of a NtWaitForMultipleObjectsEx in the thread report; async reads
logged at debug level):
1. Crash dump: guest fault address (host address minus the membase), the
   page's heap, state and protection, guest code words around the PC, the
   thread's guest stack and last kernel calls (from the existing reports),
   host backtrace for host-side faults.
2. Waits: list every handle of a NtWaitForMultipleObjectsEx /
   KeWaitForMultipleObjects with type, name and signal state; log the result
   of logged kernel calls (file calls especially).
3. Longer sampling on demand (e.g. 1 kHz for a few seconds from a button or
   a no-swap watchdog), aggregated per guest function and host area.
4. Settings toggles, later per game.

## Other games (owner's runs 2026-11-19, zip "Miscellaneous_runs-1", build 9618 era)

Assassin's Creed Revelations (55530879), logs 22-06-39 and 22-27-38:
- Gameplay ran (43 ms frames, GPU busy 120%) for minutes, then a guest write
  fault in the same function in both runs: PC 833378E4 / 833378E0 (likely a
  copy routine; r5 sizes 0x15C30 and 0x1220), called from 8232121C in both
  (r12 and the first stack entry), whose callers differ. Host fault
  addresses 0x2A0000008 and 0x2A000100B. Host heap pointers in these logs
  sit at 0x28xxxxxxx, so if the guest mapping base is 0x2A0000000 the guest
  addresses are 0x8 and 0x100B: a null pointer plus an offset (unconfirmed,
  the dump does not print the base).
- Afterwards another thread waits forever in RtlEnterCriticalSection
  (839C43E8, lr 823890BC) on a lock the crashed thread presumably held, so
  the game freezes. The second "Unhandled host exception" is the same thread
  faulting again when shutdown resumed it.
- "BaseHeap::AllocFixed attempting commit on unreserved page" (x4) happens
  at setup in every run, not related.
- Next: crash dump item 1 above (guest address, page state, code at PC).

Assassin's Creed Revelations on build 9637 (logs 13-05-24 debug, 13-06-55):
new save hangs. The owner picks a new save, enters a name, and the game
stays on the save select menu; A on a save shows no prompt again; the
main menu still works.
- XamShowKeyboardUI (description "ENTER NAME", default text the gamertag)
  goes to the iOS prompt on the Kernel Dispatch thread (01000020). Right
  after it, that thread faults in host code (pc 0x1B8E76820, an iOS system
  library; address 0x509DE9833910, garbage) and the signal handler retries
  it forever ("Unhandled host exception", millions of times). The prompt's
  overlapped never completes, so the game waits on it for good.
- Cause: windowed_app_main_ios.mm is built without ARC (it calls
  autorelease), so the keyboard callback's `__block NSString* typed_text`
  kept the alert's autoreleased text without retaining it; the dispatch
  thread then sent UTF8String to a freed string (objc_msgSend on a bad
  isa). A race: it can work when the dispatch thread reads first.
- Fix (next build): the completion stores a copy (+1), the dispatch thread
  reads it and releases it. In build 9640.

Spider-Man: Web of Shadows (41560815), logs 21-59-42, 22-00-32, 22-04-43:
- Same in all three runs, before the first frame (no swap, no draw stats):
  loader thread F8000038 logs `StfsContainerDevice::ResolvePath(\packs)`
  and then waits in NtWaitForMultipleObjectsEx on 2 objects forever. The
  main thread pulses event F800000C in a loop (NtPulseEvent, ~2.7 M calls)
  and five workers (start 82963840) wake on it (NtWaitForSingleObjectEx,
  ~11-12 M calls each, loop at 82B22AB8 retries on 0x101): busy-waiting for
  the loader, not the cause.
- Our file I/O is the older Xenia code: NtReadFile has a TODO for async, and
  NtQueryDirectoryFile ignores its event and APC (so does XeniOS HEAD), but
  XeniOS HEAD reworked xboxkrnl_io.cc (1141 vs 739 lines): SubmitRequest
  with an I/O worker for async handles, completion APCs (QueueIoApc),
  completion ports, request validation. Suspect: an async file request in
  \packs whose completion (event or APC) never arrives here.
- Next: one run with Log Verbosity = Debug (kernel calls with arguments;
  the spinning calls are high-frequency and stay out), then compare the
  calls with XeniOS HEAD's I/O.

Spider-Man debug run on build 9637 (log 12-58-05): cause found.
- Loader thread F8000038 opens D:\packs\game.XEPACK (async handle), then
  `NtReadFile async F8000094: event 00000000 APC 82B16659 context 829688C0
  offset 0 length 80000: status 00000000, 80000 bytes`: the read completes
  inline and its only completion signal is the APC. Bit 0 of the routine
  asks for a user APC (XAPI's ReadFileEx trampoline, see XeniOS HEAD's
  QueueIoApc). The thread then waits forever in NtWaitForMultipleObjectsEx
  on events F8000028 and F8000034, guest lr 82B16D4C, which can only end
  when the APC runs (alertable wait).
- The APC is queued (xeNtQueueApcThread, user mode) but never delivered:
  with guest threads on host threads (guest_scheduler off here, on by
  default in XeniOS HEAD since 2026-08), a POSIX host wait never returns
  kUserCallback; the user callback signal xeNtQueueApcThread sends runs a
  no-op. XeniOS HEAD's cooperative waits check HasPendingUserApc; its host
  path has the same gap. KeDelayExecutionThread here only delivered APCs
  pending at entry.
- Fix (next build): alertable host waits (XObject::Wait, WaitMultiple) end
  with USER_APC when the thread has a pending user APC, checked before
  waiting and, on iOS, every 10 ms slice (kAlertableWaitPollIntervalIOS);
  the alertable KeDelayExecutionThread slices check too. The callers
  already run xeProcessUserApcs on USER_APC. Risk: user APCs that were
  never delivered before (in any game) now run at the thread's next
  alertable wait, as on the console.
- Not ported: XeniOS HEAD runs an I/O APC without bit 0 as a kernel APC
  before the service returns; here every I/O APC is a user APC.
- In build 9640.

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
  Both include `com.apple.developer.kernel.increased-memory-limit` and
  `com.apple.developer.kernel.extended-virtual-addressing` (true); checked
  in the signature of build 9578's binary (owner asked, 2026-11-18). The
  perf lines' footprint + available memory always sums to 4095 MB, so the
  increased limit is in effect; gameplay peaks near 1.9 GB.
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
