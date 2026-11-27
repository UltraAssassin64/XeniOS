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

Built as 9644 (`xenios_ios_arm64_0.2.9-9644`, commit 6f71b9bd8, CI run 327):
the commit zeroing fix, the crash dump pointer details, the storm code dump
and the startup sign-in mask.

Batman: two traces (15-50-11, 15-53-37), 4 frames each (~3,550 draws and
27-32 resolves per frame). Offline tools in the session scratchpad (not in
the repository): tools/xtr_parse.py (trace -> per-draw register JSONL, with
a pure-Python snappy decoder) and tools/edram_sim.py (a simplified model of
the render target cache's EDRAM ownership: extent from the scissors and, with
the viewport transform on, the viewport; a draw bounded by neither inherits
the last claim at its base; resolve clears claim their area).
- Model check: 97-123 transfers and 196-252 MB moved per frame, against the
  device's ~68 transfer passes and 190-220 MB loaded/stored: close enough
  to compare rules.
- Rules, per frame (trace 15-50-11 / 15-53-37):
  current: 123 / 97 transfers, 49,248 / 38,231 tiles (252 / 196 MB);
  skip unchanged (the 9637 setting): no change, the 4x passes write stencil;
  + stencil-only copy back: 23,408 / 16,208 of those tiles need only the
  stencil (depth intact), same count of passes;
  samples as pixels (1x/2x/4x views of a base, width in samples and format
  share one texture): 36.5 / 32.5 transfers, 16,088 / 14,224 tiles
  (82 / 73 MB), about two thirds less; stencil-only adds nothing on top.
- Largest transfers now: main depth D24FS8@0 1x <-> 4x (8,600-12,600 tiles
  each way), color c3 (2_10_10_10_FLOAT) <-> c12 (its AS_16_16_16_16 view)
  at 720 (2,900-4,300), shadow depth D24S8@1440 1x <-> 4x (2,000-3,600).
- Cost of samples as pixels here: the 4x surfaces get 15-38 draws per frame
  with ~100 vertices in all (full-screen stencil clears and the like), so
  per-sample shading costs next to nothing in Batman. Games that draw real
  scenes with MSAA would pay 2-4x pixel shading on those draws: hence the
  owner's idea of choosing per surface from recent use.
- Next: design samples-as-pixels storage for the Metal render target cache
  (keys without msaa for storage, viewport/scissor scaled by the sample grid,
  Xenos sample positions approximated by the 2x2 / 1x2 pixel grid, resolves
  reading samples as pixels), behind a setting, first for depth.

### Build 9644 results (owner, zip "9644_logs": AC 17-54-53, Spider-Man 17-52-32 and 17-53-35)

AC Revelations crash (17-54-53), same site again: PC 833378E0 (allocator
free-list unlink) from 8232121C on thread F80001E0; node r4 = 0x44FFEA08
holds 40370037 00001161 FFFFFFFF 00000000 (prev -1, next 0), heap header r3
= 0x40102150. The new dump: the node's page is a 64 KB allocation at
0x44FF0000, allocated last (#24 of the 1024-call history) by thread
F80001C0 with 0x20803000 (64 KB pages, NOZERO, reserve+commit), right after
F8000154 freed 44FF0000+20000 (#27) that it had allocated at #28; before
that, F8000154 allocated and freed 0x160000/0xB0000/0x40000 regions over the
same address many times. So three threads use the game's allocator on that
page within a few calls. Not found in our code yet:
- RtlEnterCriticalSection's recovery/steal path never fired (no warning in
  any AC log); lock_count is host-endian as upstream.
- protect_on_release is false: released blocks stay readable and writable,
  so a use after NtFreeVirtualMemory corrupts silently. Asked of the owner:
  one run with protect_on_release = true (Settings, Memory section), which
  is console-like and makes such a use crash at the culprit.

Spider-Man: the storm code dump works. Game Master's wait (82962D70, called
with r26 = -1) loops up to 10000 times while the job system's pending +
active counters (globals at 0x82F718C8 + 0x48/0x4C, read with lwsync)
are non-zero, calling 82963600 (help/sleep) in between: it waits for every
job to finish. Workers 0, 2, 3 find nothing to run; worker 1 runs the
loading-screen render loop; worker F8000064 (start 82A25CE8) waits on event
F8000060, signaled ~96,000 times before sign-in and never after. Next
diagnostic idea: per event, the thread and guest lr that last set it, shown
in the thread report (who stopped signaling F8000060).

Samples as pixels: the owner asked before any build (2026-11-19): (1) the
effect on other games and the downsides, (2) whether it can be a setting
that falls back to the current path. Answer given: only MSAA render targets
change; no-MSAA games are unaffected; real-MSAA scenes cost 2-4x pixel
shading and change AA quality (sample positions, alpha to coverage,
derivatives, resolves); it will be a setting (Off = today's path, untouched;
On; later Auto per surface), off by default.

### AC protect_on_release runs (owner, logs 18-10-14 and 18-12-08) and the next build

Both runs stop at the same instruction. 18-10-14 (the setting was still
off) crash-dumps on TaskThread1 F80001B0 at PC 82338710, lr 82338580:
read of guest 0x43000000, which is unreserved; r3 = 0x42FF0000 is a 64 KB
block (allocated with 0x20803000) and r10 = 0x43000000 is its end. The
code there is a vector fill loop (stvx, then `stvlx`/`stvrx` for the
unaligned head and tail). 18-12-08 (protect_on_release on) hits the same
fault ("Unhandled host exception ... address 0x2E3000000" = membase +
0x43000000, same lr), but the thread was suspended instead of dumped, so the
game waits for it forever at the Ubisoft logo (F8000008 waits on a task
event; F80001B0 has made no kernel call for 7 s, suspend count 1).

Cause: our a64 STVL/STVR were a read-modify-write of the whole aligned
16-byte block. `stvrx` with a 16-aligned address stores nothing on the
console, so at the end of a buffer the game legally points it at the next,
unmapped page; ours read (and wrote back) that page. The same RMW also
explains the earlier AC heap corruption: it rewrites bytes outside the
stored range non-atomically, so another thread's store to the same 16 bytes
in between is lost (the allocator free-list nodes found holding stale
values, 9643/9644). Fix (next build): xenia-edge's partial store,
`EmitPartialVectorStore`, which writes exactly the bytes of the range with
overlapping 8/4/2/1-byte stores and nothing for an empty range; no read.
To test: AC with protect_on_release off and on, past the logo into chapter
1; the free-list crash (833378E0) should be gone too if this was it.

Spider-Man diagnostic in the same build: the thread report now shows, for
each event a thread waits on, who last set or pulsed it ("set N times, last
by thread X at guest lr Y Z ms ago", or "never set"), from
XEvent::NoteSignal in Set and Pulse. Run to "Please Wait" and let the storm
report fire: the line for F8000060 names the thread and code that stopped
signaling it after sign-in.

### MSAA samples as pixels (msaa_samples_as_pixels, next build)

What it does: with the setting on, an MSAA surface's render targets are the
1x render targets of the same EDRAM (base, pitch in tiles, format), whose
pixels are its samples - the EDRAM layout of 4x samples (2x2 per pixel) and
2x (1x2) is the 1x layout, which is why games alias them in the first
place. So Batman's per-light 1x -> 4x depth copy, the 4x stencil clear, and
the copy back become one draw into the 1x depth buffer, and the render pass
is not broken. Draws to such a surface are scaled by the sample grid on top
of the resolution scale (RenderTargetCache::last_update_samples_as_pixels_
x/y_log2): viewport and scissor (IssueDrawMsl, with MagicDiv of the combined
scale), the render pass area (IssueDraw), point size and depth bias slope.
Resolve clears use the same keys and scale the clear rectangle to samples.
Resolves need nothing: owners are dumped to EDRAM by key, and the 1x dump
has the MSAA sample layout. Pipelines take the sample count from the
attachments (1). The pixel position parameter (vPos) is divided back to the
guest pixel with two new system flags (kSysFlag_SamplesAsPixelsX/Y) in the
SPIR-V translator; that changes only param-gen pixel shaders, which
recompile once (the MSL cache key includes the SPIR-V hash).

Where: RenderTargetCache::Update and PrepareHostRenderTargetsResolveClear
(storage_msaa_samples), SupportsMsaaSamplesAsPixels (Metal: SPIRV-Cross
path only, i.e. iOS; MSC/D3D12/Vulkan keep the old path whatever the cvar
says). Off = the exact old path. Settings > Performance > MSAA Samples as
Pixels; logged in "Settings at launch". Applies at once (switching mid-game
just moves EDRAM ownership between the two kinds of render targets).

Downsides, as told to the owner: in games that really draw with MSAA, every
sample is shaded (2x/4x pixel shader work, supersampling-like look),
alpha-to-coverage becomes a per-sample threshold, derivatives and texture
LOD are per sample (sharper mips), and sample positions are an ordered grid.

Owner wants this automatic later (2026-11-19): the target is an Auto mode
that picks per surface without game-specific patterns - for example, store
samples as pixels for a surface only while its MSAA draws are few or
depth/stencil-only (Batman's stencil clears), and keep host MSAA for
surfaces with many colour draws; or measure transfer time per surface and
switch the cheaper way. Needs per-surface draw/transfer counts (the
transfer log already has totals) before the rule can be chosen. To test in
the next build: Batman with the setting off and on (FPS, transfers per
frame in the log, shadows/lighting correct), and one real-MSAA game on and
off for looks. Update after 9648: the setting now applies by draw type and
current owner (rectangle lists while host MSAA does not hold the data, see
"Build 9648 results"); still to come for full Auto: choosing by measured
cost, and the setting on by default once it has been checked in more games.

Settings search (owner's request, same build): a search bar in the
navigation bar of Settings (a plain UISearchBar as the title view; a
UISearchController would be presented by the settings controller while
active, and its Save/Discard alerts and text editor could not be presented)
filters every row, including the generated per-category ones, by section,
title, description and config name; all words must match, ignoring case and
accents. Edits made while filtered go to the same items. Dragging the list
hides the keyboard.

Built as 9648 (`xenios_ios_arm64_0.2.9-9648`, commit d2530fc43, CI run
328). CPU tests ran on this commit for the first time on the branch (macOS
ARM64 runner): 258/258 HIR, 141,712 PowerPC cases, 0 failures, including
stvl/stvr/stvlx128/stvrx128.

### Build 9648 results (owner, zip "9648_logs_and_dumps": 8 logs, 10 GPU traces)

Runs: Batman 18-59-50 and 19-03-06 (samples as pixels off), 19-06-09 (on),
19-21-51 (on, skip unchanged off); AC 19-09-00 (off, real queries),
19-28-09 (protect_on_release on), 19-30-02 (queries off, samples as pixels
on); Spider-Man 19-48-17. Owner: settings search works.

Batman, samples as pixels (msaa_samples_as_pixels):
- Faster: frame avg 41.5 / 37.9 ms on against 57.6 / 60.7 ms off; rt
  transfer passes 89.9 -> 19.9 per frame, 43.3 -> 23.4 ms.
- But Batman's shading regressed ("more triangles shaded", the 9615
  facets) and the floodlights showed through him. Cause, from traces
  19-25-21 and 19-26-13: some scenes have UE3's 2x predicated-tiling base
  pass (336 colour + 124 depth draws per frame, viewport transform on, 4
  tiles resolved with clears). UE3 shifts it so Xenos sample 0 (-0.25,
  +0.25) lands on the 1x pixel centre; drawn as samples-as-pixels, sample 0
  is the top row pixel at (0, -0.25), (0.25, 0.5) px off, so the restored
  depth disagrees with the 1x light passes again. No single scale and shift
  maps the samples-as-pixels grid onto the Xenos positions (4x: (-2,-2),
  (2,2), (-6,6), (6,-6) in 1/16 px, stored TL, BL, TR, BR). The occlusion
  queries are 1x depth-only draws against that same depth at base 0, which
  fits the floodlights.
- The gameplay trace (19-08-22) has only 4x rectangle lists with the
  viewport transform off (per-light stencil clears, shadow map depth
  clears), exact as samples as pixels.
- Fix (next build): samples as pixels only for rectangle lists (and resolve
  clears), and only while the host MSAA render target does not hold the
  EDRAM (RenderTargetCache::IsEdramTileOwnedBy on the first render
  target's base tile), so real MSAA geometry keeps host MSAA with Xenos
  sample positions and a real-MSAA game's clears don't bounce its data
  between the two. Model (tools/edram_auto.py): gameplay trace 35.2
  transfers per frame, the same as all-MSAA-as-pixels and against 103.5
  off; 2x scenes within 1% of all-as-pixels. This is the first step of the
  owner's "automatic" choice: by draw type and current owner, not by game.
- To check: shading and floodlights correct with the setting on; FPS as
  in 19-06-09.

Assassin's Creed Revelations:
- The a64 STVL/STVR fix works: no crash with protect_on_release off, and
  the prologue -> chapter 1 transition now works (queries off run). The
  owner got further than in any XeniOS or xenia-edge build.
- protect_on_release on (19-28-09): the GPU command thread (01000010)
  faults 12 million times at host pc 0x103102B30 on a protected page: the
  game frees physical memory the GPU still reads, and PhysicalHeap::Release
  also protects the parent (physical) heap, the view the GPU reads through.
  The console GPU reads physical memory whatever the CPU mapping, so this
  option can't work for AC; leave it off. (An unhandled host fault on a
  host thread retries forever instead of stopping; not addressed.)
- Real queries on (19-09-00): flicker of the whole environment. Chapter 1
  ends ~100-400 queries per frame (98-400 begin/end pairs in the traces;
  the prologue had 5), so the game culls objects and lights with them.
  98% of our guesses get corrected (e.g. 8130 of 8322; 82% count 0), and
  the game reads some before the correction. Queries off (19-30-02): no
  flicker, but lights that should be culled underground show through the
  scenery: every query reads as visible. AC's own MSAA draws are 4x point
  lists (18-24 per frame) and a few rectangle lists, not the cause.
- Next for AC (owner: back burner, performance later): a strict mode like
  XeniOS HEAD's `occlusion_query = strict`: leave the report's ZPass lanes
  at 0xFFFFFEED until the GPU count is in, commit the command buffer when
  the ring runs empty (PrepareForWait does) and process completed queries
  while idle (the base wait loop needs a hook; XeniOS calls PrepareForWait
  again in its 2 ms waits while a report is owed), with a guess for
  inexact queries so the game never waits forever.

Spider-Man (19-48-17, ~1 minute on "Please Wait"):
- Event F8000060, which worker F8000064 (start 82A25CE8) waits on, was set
  7 times in the whole run, last by Game Master (F8000008) through the
  game's SetEvent wrapper (guest lr 82B16C5C, the same lr sets F8000030),
  ~11 s before. The worker's 96,471 waits are the kernel32-style
  WaitForSingleObjectEx wrapper at 82B22A8C (which retries on 0x101 when
  alertable), not signals, so the 9643 reading "signaled ~96,000 times"
  was wrong. Its first waits are at stack 82A25D10, the final one at
  82A25D30.
- Next build: XEvent::NoteSignal also records two return addresses up the
  guest stack (last_signal_callers_, read within the thread's stack), so
  the report names the code that called the wrapper.

Owner request (2026-11-20): Metal Performance HUD toggles, to show thermal
state and the iOS version. Apple's HUD has a `thermal` element
(MTL_HUD_ELEMENTS) but no OS version element, and Apple's pages don't say
from which iOS element choice works (the customization pages are newer than
iOS 16). Next build, Settings > Display:
- Metal Performance HUD (metal_hud, default on: the layer's
  developerHUDProperties mode, and MTL_HUD_ENABLED unset when off),
  HUD Contents (metal_hud_elements -> MTL_HUD_ELEMENTS: Apple default,
  default + thermal, compact, detailed) and HUD Position
  (metal_hud_alignment -> MTL_HUD_ALIGNMENT), set after the config is read
  (xenia_main_ios.mm) and before Metal starts; relaunch to apply.
- Device Status (ios_device_status, default off, applies at once): our own
  label, top left while a game runs, "iOS <version> · Thermal: <state>"
  from NSProcessInfo.thermalState (Nominal/Fair/Serious/Critical; iOS has
  no temperature API), coloured by state. Works on iOS 16 whatever the HUD
  supports.

Built as 9650 (`xenios_ios_arm64_0.2.9-9650`, commit 3b04d54f5, CI run 329;
the owner's Actions were briefly disabled, hence an extra comment commit).

### Build 9650 results (owner, zip "9650-log": Batman 20-43-05 off and 20-47-00 samples as pixels on, Spider-Man debug 20-49-48)

- Batman: owner: fine, the second run (samples as pixels on) 3-4 FPS better;
  rt transfer 61.4 -> 17.8 passes, 33.1 -> 16.8 ms per frame. Shading
  regression not reported again (to confirm on screen).
- Spider-Man: F8000060 set 7 times, last by Game Master at the SetEvent
  wrapper 82B16C5C, called from 82A23390, called from 82A18710, ~2 s before
  the sign-in UI. The worker F8000064 (start 82A25CE8) is the game's music
  manager: it calls XMsgInProcessCall(0xFA, 0x0007001B) =
  XMPGetPlaybackController(2, ...), and F8000058 (start 82A7CD00) waits on
  F8000054 (never set), F800004C (set by F8000064 from 82A7B7CC/82A7C14C)
  and F8000050 (set by the XMA thread F800008C). Game Master's last calls
  after sign-in: XamUserReadProfileSettings, XamUserGetName, two
  XGIUserSetContext; then the job wait.
- Next build: the thread report dumps the guest code (64 words before, 8
  after, once per address) of each blocked thread's call stack and of the
  two callers that last set the event it waits on, to disassemble Game
  Master's job wait, the music worker's waits and 82A23390/82A18710
  (tools/dis_storm.py reads these lines).

### Build 9651 results (owner, zip "SpideyWoS_logs_1": 23-03-41, 23-04-41, stuck 154 s)

- Music worker F8000064 (82A25CE8): `wait(event at this+0x4C, INFINITE);
  while (!byte this+0x50) { process queue (82A25B50); wait again }`. Idle,
  as designed. Game Master posts to it through 82A231F0 (from 82A18710,
  which then waits for the reply at 82A181E0); 82A2338C sets the event.
  Not the deadlock.
- Game Master: 82962D70 with r26 = -1 loops (up to 10000 times, then
  again) while the job system's pending + active counters (0x82F718C8
  +0x48 / +0x4C) are non-zero, trying to run a job itself (82962FE8) and
  sleeping (82B13248 -> KeDelayExecutionThread) when there is none.
  Nothing is pending, so one job is active: JQ worker 1 (F8000018) runs
  the loading-screen render job (stack 82ABA2F8 829258C4 829636A0), sitting
  most of the time in D3D's GPU fence wait (82ABA2F0: until the value at
  *(device+0x2A90) reaches the target, 82AC0C10 spinning) - ordinary frame
  pacing, frames do come out at 30 FPS. So the question is what ends that
  render job (or what Game Master's wait expects to end it).
- Next build: the report also dumps the code of each running guest
  thread's stack (829258C4 etc.), and dumps are 128 words before each
  return address (was 64). tools/dis_storm.py must start 0x200 before.

### Build 9652 result (owner, Spider-Man debug log 23-30-47)

- The loading screen is one job per frame: 82925778 (timing with mftb,
  fills debug data with 0xDEADBEEF, draws), run by JQ worker 1. Before
  drawing it waits for the previous frame's fence when it isn't done
  (829258AC: `if (fence != -1 && !done(fence)) wait(fence)` -> 82ABA520 ->
  the 82ABA2F0/82AC0C10 spin on *(device+0x2A90)).
- The GPU is busy only 9-14% of the time and frames are 33.3 ms, yet the
  worker spends 12-14 of 16 samples in that fence spin: the fence follows
  the 30 Hz frame pacing, not GPU work. A frame job is therefore active
  almost all the time, and Game Master's wait (82962D70, r26 = -1: until
  pending + active jobs of every queue are 0) never sees 0.
- Hypothesis: on the console the frame job finishes early in each frame
  and leaves an idle gap Game Master's polling catches; here the fence
  (swap/vblank pacing, or our fence write-back at command buffer
  completion with metal_gpu_ordered_fences) holds it until the next frame.
  The upstream compatibility report's "endless loadings fixed by enabling
  VSync" points at the same pacing.
- Asked of the owner (no build needed): one run with Settings > Display >
  Cap Guest Display Refresh off, and one with Settings > Accuracy > Wait
  for the GPU at Game Fences off (each on its own), to see which one lets
  the loading finish. Then fix the pacing or fence timing accordingly.

### Build 9652 settings runs (owner, zip "spideyWoS9652": defaults 23-35-41, cap off 23-37-16, ordered fences off 23-39-39)

- None got past "Please Wait"; with the refresh cap off, rare big stutters
  (17 FPS). JQ worker 1 sits in the D3D fence spin in all three, even at
  ~4 ms frames with the cap off, so pacing isn't what holds Game Master.
- The fence is written by EVENT_WRITE_SHD (D3D 82ABA0F8 emits type 3 op
  0x58 to *(device+0x2A90), next value +2); our handler stores it at once
  (or at command buffer completion with ordered fences): not lost. The
  command processor thread is ~2% busy (ring consumed).
- New suspicion: a job counter that never returns to 0 (pending at
  0x82F71910, active at 0x82F71914, which Game Master sums with lwsync),
  e.g. a lost atomic update (lwarx/stwcx under contention on our ARM64
  backend). Next build: debug_watch_guest_addresses (Settings > All
  Options: Logging), a thread sampling the given guest words every 0.1 ms
  and logging min/max/last/% zero every 2 s ("Guest value watch"). Ask the
  owner to set it to 82F71910,82F71914 for a Spider-Man run: if active
  stays >= 1 while every worker is idle, a decrement was lost.

### Build 9654 result (owner, zip "Spidey_WoS-2": 05-43-41, 05-47-29, watch on 82F71910,82F71914)

- Both counters move (min 0, max 2) but sit at 1 essentially always after
  the loading screen starts ("zero 0%" in every 2 s window, ~15,000
  samples): one frame job running and the next already queued. No lost
  update. Before the loading screen they were 0 90-99% of the time.
- Game Master's help step (82963468.., 82963600) may run queued jobs itself
  (class 1 any time for worker slots < 2, else only inside a time window
  read from 0x82F719C0/0x82F719C8 against 82920CC0; class 2 for slots < 5),
  so the jobs are reachable; what is missing is why frame jobs keep being
  queued, i.e. what should end the loading screen.
- Next build: debug_dump_guest_code (Settings > All Options: Logging),
  "start-end" hex ranges, at most 64 KB each, logged once 20 s after
  launch ("Guest code dump at A: 8 words"). Ask for 82962000-82968000
  (job system), 82925000-82931000 (loading-screen frame job),
  82A16000-82A26000 (music system) and 82920000-82921000 (clock).

### Build 9655 result (owner, log 06-20-13 with the code dump; disassembly in the session scratchpad r9655/dis.txt)

Job system (globals at 0x82F718C8 = J): J+0x3C pointer to 32-byte queue
entries, J+0x44 free-list head, J+0x48 / J+0x4C pending / active over all
queues, J+0x50.. per-queue in each entry (+0 pending, +0x1C active),
J+0x50/0x54 = events 0x82F71918 / 0x82F7191C (worker wake), 0x82F71920
quit flag, 0x82F718BC idle-worker count.
- 829622F8 creates a queue (returns its index), 82962368 destroys one
  (waits for all with -1 first), 829623C8 shuts the system down.
- 82962D70(queue, wake_a, wake_b): sets or resets the two wake events, then
  polls the counters (the queue's own if queue != -1, else the global
  ones), calling 82963600 (run a job, else sleep 82B13248) between polls,
  10000 times per round, forever until 0.
- 82962EE8 is a worker's idle wait: while nothing is pending, count itself
  idle and wait 1 ms on the wake event.
- Game Master's frame (8292FF08) calls 82962D70(*(0x82F6F404), 1, 0) every
  frame; 0x82F6F404 is the render queue, created by 829285E0. The
  loading-screen frame job (82925778) takes D3D ownership (82928540 /
  82928598), waits for the previous frame's fence (829258C0), inserts a
  new fence, draws, and ends with 82928598 and 825A88C8(*(0x82CE2158)).
- The watch (9654) was of the global counters, which include every queue.
  Next: watch 82F6F404 (which queue Game Master waits for: an index or -1),
  82F71904 (the queue array), plus 82F71910,82F71914; then that queue's
  own counters (array + 32 * index, +0 and +0x1C). Not yet known: who
  queues frame jobs (82925778 is only reached by function pointer, from
  code outside the dumped ranges).

### Spider-Man watch runs on 9655 (logs 06-32-58, 06-39-50)

- 0x82F6F404 (Game Master's sync queue) = 0x2D (45); queue array at
  0x00091080, so queue 45's counters are at 0x91620 (pending) and 0x9163C
  (active).
- Queue 45: pending 0 for 99% of samples, active 1 for ~100% after the
  loading screen starts (min 0, so it does end now and then, and something
  starts it again at once). So the loading-screen work on queue 45 is
  never finished when Game Master's frame syncs on it.
- Suspect: the frame job (82925778) ends with 825A88C8(*(0x82CE2158)),
  which may queue the next frame unless a stop condition holds. Next: dump
  825A8000-825A9800 (and 82939000-8293A000 for 82939360) to read it; no new
  build needed (debug_dump_guest_code).

### Spider-Man: code dump 825A8000.. (9655, log 06-48-33) and the fix tried next

- 825A88C8 only frees a list's nodes; the frame job does not requeue
  itself. The job runner (82963600..) calls job->vtable[0] (8296369C),
  then decrements the global and the queue's active counts; the worker
  then takes the next frame job. So the loading screen runs normally and
  Game Master is probably not frozen in its per-frame sync either; it only
  spends most of each frame there. The open question is the game logic
  that should end "Please Wait".
- The game's notification listeners: mask 0x20 (music player) first, mask
  1 (system), another 0x20 later. The system one got the sign-in
  notifications; the music player ones never got anything. xenia-edge and
  XeniOS HEAD give the first music player listener its state
  (XmpStateChanged) and controller (XmpPlaybackControllerChanged) at
  registration, unsignaled; this branch sent nothing. The music worker
  (F8000064) and F8000058 (waiting on an event never set) belong to that
  system.
- Next build: those two startup notifications ported
  (KernelState::RegisterNotifyListener, has_notified_xmp_startup_;
  XNotifyListener::EnqueueNotification gets xenia-edge's `signal`
  parameter, default true). To test: Spider-Man past "Please Wait".
  xenia-edge's XMP app also returns the media player's real controller
  and lock state from XMPGetPlaybackController (ours writes 0, 0), not
  ported yet.

### Spider-Man: Web of Shadows solved by comparison with PC (owner, 2026-11-20)

- Owner's xenia-edge PC log (edge@f159897b9): same packs in the same order
  as on the phone, then XamUserGetName, XGIUserSetContext x2 and, one
  frame later, XamShowDeviceSelectorUI (answered automatically, no visible
  prompt), XamContentGetDeviceData, content enumeration, main menu. The PC
  had a profile signed in at boot (logged_profile_slot_0_xuid); the phone
  had none, so the game showed XamShowSigninUI after Press Start, and after
  it made the same profile reads and XGI calls but never the device
  selector. (The next screen is the main menu, not a storage prompt.)
- Confirmed (owner, 9658, log 08-29-45): with logged_profile_slot_0_xuid
  set to the phone profile (E03000000B54CE77) the game reaches the main
  menu.
- Cause: on iOS, XamShowSigninUI ran the native prompt inside the call
  (xeXamDispatchHeadless with no overlapped): the guest thread was blocked
  until a profile was picked, and XN_SYS_UI 1, XN_SYS_SIGNINCHANGED and
  XN_SYS_UI 0 were all queued before the call returned. On the console and
  in xenia-edge the call returns at once and the game waits for them.
- Fix (next build): the iOS path broadcasts UI shown, counts the dialog
  (IsUIActive), returns X_ERROR_SUCCESS, and runs the prompt (or the
  fallback LoginMultiple) on its own thread, then UI closed 100 ms later.
  To test: Spider-Man with logged_profile_slot_0_xuid empty, signing in at
  the prompt; and the other games' sign-in prompts.
- The job-system, music and fence investigations above were not the cause;
  their diagnostics (debug watch, code dump, report code dumps) stay.

### Batman: Arkham City first look (owner, zip "Batman_Arkham_City", build 9658, logs 08-33-33, 08-37-04, 08-39-09 + 6 traces)

- Menus and intros: 33.3 ms (30 FPS cap). Gameplay: 45-65 ms (15-22 FPS),
  spikes of 100-1800 ms.
- Hitches (scripted camera moments): shader compiles, 50-60 per 120 frames
  at ~300-400 ms of compile time each (20.8 s per window), draws waiting
  for them 91-109 times (~1.2-1.5 s per window, async compile with
  metal_async_compile_wait_ms = 16), and 1.1-1.7 GB uploaded per window.
- Steady gameplay is GPU bound (busy 90-105%): per frame ~57 ms of GPU,
  draws 29.1 ms in 66 passes (770 draws), rt transfer 8.7 ms, texture load
  6.0, edram dump 5.2 and resolve 4.5 (50 resolves), texture upload blit
  3.7. The main 1280x736 target is drawn in ~16.7 passes per frame, each
  loading and storing it: 785 MB of attachment traffic per frame. Cutting
  pass breaks (most from the resolves) is the lever, for both Batman games.
- Traces not analysed yet (tools/xtr_parse.py + edram_sim.py work on them).

### Batman: Arkham City logs analysed (2026-11-24) and the plan for GPU time and first-run stutters

Same zip as above (build 9658). Runs 1 and 2 (08-33-33, 08-37-04):
msaa_samples_as_pixels off; run 3 (08-39-09): on, with real occlusion
queries, thermal state 2 most of the run.

GPU time per frame, steady gameplay (no compiles):
| | samples as pixels off (runs 1-2) | on (run 3) |
|---|---|---|
| frame | 38-63 ms, GPU busy 102-107% | 39-51 ms, GPU busy 82-105% |
| rt transfer | 14.6-27.5 ms, 50-60 passes | 3.4-5.4 ms, 15 passes |
| draws | 12.9-21.1 ms, 81-92 passes | 13-15 ms, 54-65 passes |
| texture load + upload blit | 7-10 ms, ~30 loads | 6-8 ms |
| edram dump + resolve | 8.4-9.9 ms, ~48 resolves | 6.3-7.2 ms |
- Samples as pixels removes most of the transfers here as in Arkham Asylum
  (the 640x384 4x depth round trips). Recommended per game for both Batman
  games (long-press > Game Settings) until it is on by default.
- What is left is pass structure: the 1280x736 main target is drawn in
  13-19 passes a frame, each loading and storing it (150-230 MB each way),
  because every resolve ends the render pass; and each resolve is two
  compute passes (edram dump, resolve), each texture load two (load, blit).
- About 35 command buffers a frame (4,000-4,500 per 120 frames). Two causes
  in IssueCopy/RequestTextures: a resolve in a submission that has draws
  commits the command buffer (XeniOS HEAD keeps it open: "Later work in the
  submission that reads the output ... is ordered after the resolve on the
  GPU"), and a texture load requested while the draw's render encoder is
  open (IssueDraw opens it before IssueDrawMsl requests textures) goes into
  a command buffer of its own, committed at once. The second is why the
  first exists: a load in its own buffer runs ahead of the open one, so the
  resolve it reads must already be committed. They have to change together
  (XeniOS HEAD: loads into the current buffer after ending the pass, or
  "loads ahead" from a CPU snapshot of guest memory).

First-run stutters (run 3, a new area): up to 54 shaders and 40 pipelines
per 120 frames, compile threads busy 5-20 s per window, draws waiting up
to 1.6 s per window (metal_async_compile_wait_ms = 16 per frame) and
compiles queued up to 3.3 s. Second runs compiled the same shaders in about
1 ms each (Metal's system cache), but that cache is empty after an app
update or a reboot (XeniOS HEAD's note on metal_bake_fetch_state), so every
new build the owner installs is a first run again.
- 670 "Compile timings" lines: per shader, median 239 ms; SPIRV-Cross
  ~10 ms, Metal library from MSL source median 185 ms (MSL median 45 KB,
  up to 180 KB), and newArgumentEncoder 38.8 s in total over the pixel
  shaders (24% of their library time; median 10 ms, p90 214 ms): Metal
  compiles the function to reflect it. Pipelines about 160 ms each.
- XeniOS HEAD avoids the MSL compile with spirv2air (AIR straight from
  SPIR-V, loaded with newLibraryWithData from its shader library cache) and
  draws through vertex-only stand-ins while a pipeline builds. spirv2air
  floors at iOS 18 / AIR 2.7 and goes with the DXIL binding layout (see the
  scoping section below): not a small step.

Plan, in order (each its own build):
1. (Next build) Share argument encoders by layout:
   metal_share_argument_encoders (default on, Settings > All Options:
   Metal). The encoder depends only on the argument buffer struct, so one
   newArgumentEncoder per distinct struct (GetArgumentBufferLayoutKey hashes
   the spvDescriptorSetBuffer members' types and [[id]]s, names left out)
   instead of one per shader. Expect "argument-encoder=0.00ms" on most
   pixel shader Compile timings lines and "MslShader: argument encoder for
   layout ..." info lines (one per layout). To check: pixel shader compile
   time drops ~20% and pipeline creation time does not rise by the same
   (if it does, Metal reused the reflection compile and nothing is won);
   textures right everywhere (a wrong layout would show wrong textures).
   Test on a fresh install of the build (cold cache), same route as run 3.
2. A shader library cache of our own that survives updates: an
   MTLBinaryArchive (iOS 14+) of the pipelines, filled as they are built and
   saved per title, given to newRenderPipelineState. Saves the ~160 ms per
   pipeline after an update or reboot; the MSL compile stays.
3. Commit after resolves only at primary buffer end, with texture loads for
   a draw encoded into the current command buffer: either request textures
   before the draw opens its render encoder, or end the pass and reopen it
   (as XeniOS HEAD). Fewer command buffers, and the same ordering.
4. One compute encoder for a resolve's edram dump and resolve dispatches,
   and one for a draw's texture loads (blits after), to halve those passes.
5. The large one: resolves read from the open pass (XeniOS HEAD's tile
   resolve, a tile dispatch reading the attachments in tile memory), so a
   resolve no longer ends the pass and the main target stops being loaded
   and stored 13-19 times a frame. Needs Apple tile shaders (A11+, iOS 11+).
6. Compile cost itself: a spirv2air probe for iOS 16 (scoping section), or
   cutting MSL size (the 45-180 KB per shader is mostly the translator's
   control flow and helpers).

### Arkham City traces re-checked (2026-11-24, the 6 traces from build 9658)

Tools: scratchpad tools/xtr_parse.py, edram_auto.py, and new
pass_model.py / pass_timeline.py (render passes as the Metal backend opens
them: the shared cache's accumulated targets, a resolve ends the pass).
- Samples as pixels cuts modelled transfers 60-70% in all six traces
  (79-130 -> 27-42 per frame); the rectangle-list, owner-aware rule gives
  the same result as "all MSAA as pixels" in this game.
- Correction to the section above: the main 1280x720 depth target
  (D24FS8 at base 0, pitch 16) is bound in 28-54 passes a frame in the
  model, not 13-19, and about half of all pass breaks are target switches,
  not resolves. Of those, most are a draw adding a colour target to a
  depth-only pass (the per-light loop: depth only, depth + colour, depth
  only, shadow map, resolve), which RenderTargetCache treats as a new pass
  ("Binding a totally new render target - won't keep the existing render
  pass anyway"). Model: 57-103 passes per frame now; 41-71 if adding a
  target kept the pass; 32-59 if resolves also kept it. So plan item 5
  (tile resolve) is worth less than thought (about 10-15% of passes), and
  "start the pass with the colour target the previous pass on this depth
  had" (when it still owns its tiles) is worth about 30%.
- The 24-draw "none" runs are point-list memexport draws (no targets,
  colour mask 0); they keep the pass.
- Model limits: it can't see which colour targets a pixel shader writes,
  so RT1-3 left at RB_COLORn_INFO 0 are dropped, and storage formats 10/12
  are folded into 2/3 as RenderTargetKey does.

### Owner's runs on build 9660 (zip "Logs_of_various_games_for_data_acquisition-1", 2026-11-20..22)

All 33 logs are 9660 (6c7c4319f). Every launch shows the "config values
had invalid types" box for user_country/user_language (see below).

| Game (title id) | Logs | What happens |
|---|---|---|
| Borderlands 2 (5454087C) | 20-22-07, 20-28-33, 08-37-06 | Boots, plays the intro Bink, at ~34 s the main thread waits forever on an event (82E7CE5C <- 82E7E24C, never set); the movie keeps drawing (3-4 draws, 180 MB/window) for 346 s. Same in all 3. |
| AC Revelations (55530879) | 13-29-10 .. 13-38-34 (6) | Frames stop after exactly 10 perf windows (~43 s, all 6 runs); the render thread waits on an event never set (8354D85C), every thread idle. 9648 ran into chapter 1. |
| Dishonored (425307E3) | 08-51-49 | Menu/loading at 33 ms, GPU 9%; from ~75 s the main thread waits forever (82A64824 <- 82A65C14, never set), 56 draws a frame for 156 s. |
| Spider-Man WoS (41560815) | 09-16-01, 09-19-23, 18-58-13 | Sign-in fix works (profile prompt, gameplay, 17 min in one run). 64-95 ms frames: per frame ~50 ms of GPU with SAP (draws 20, rt transfer 13, texture load+blit 9, edram dump+resolve 7) and 52 command buffers; GPU busy 50-60%, JQ workers 50-65% each. |
| GTA IV (545407F2) | 08-46-03 | 37 ms median, later 50-87 ms; GPU bound (100%+): rt transfer 26 ms in 50 passes even with SAP, draws 21, texture load 9 (85 a frame), edram dump+resolve 10; 2.3-5 GB uploaded a window; one guest thread at 82% CPU (hotspot 8216D90C, the XMA code). |
| Arkham Asylum (5343080B) | 09-23-15, 09-27-07 | 39-45 ms median, thermal 2; no hang. |
| Arkham City (57520802) | 15-41-03, 15-43-50, 15-46-05 | 33-44 ms median, spikes 1.6-1.8 s (compiles); mem peak 2.15 GB; no hang. |
| Fallout NV (425307E0) | 08-55-53 | 806 s at the 30 FPS cap, GPU 10%; 791 "AllocFixed ... already reserved range" at boot (harmless so far); face glitches need photos. |
| Spider-Man Edge of Time (415608B2) | 08-44-18 | 28 s of frames at 1120x640, GPU busy 9% (median); the INDIRECT RINGBUFFER errors are at shutdown only. "Presentation broken" needs a photo. |
| 41560816 (UE3, "WGame") | 16-21-54 | 250 s at 3 draws a frame (movie/loading), then gameplay 50-115 ms, mem to 2.4 GB. |
| 41560855 (unidentified, .pak~lk files) | 11-54-34 .. 12-07-42 (5) | Never presents a frame. Two runs: crash dump on kernel thread 01000014 at guest 824A5C58 (write to host address 0) and F800007C jumping to host 0; the others: F80000FC waits on handle 0 and yields in a loop, F800007C spins in NtWaitForSingleObjectEx. |

Common cause found for BL2, AC, Dishonored (and the "UE games have no
audio" report): the notify listener. In each game one thread, in the same
library as the hung wait (XAudio2's music-player handling: it listens to
the XMP area), loops NtWaitForMultipleObjectsEx -> XNotifyGetNext(handle,
0A000003) millions of times (BL2 F8000134, AC F8000244, Dishonored
F80001BC). BL2's report shows its wait list: [listener, F8000120 (set 2176
times by the audio processing thread F8000108), F8000130]. Our listener was
a manual-reset event that stays set while anything is queued; since 9658
(24cc8363e) the first XMP listener starts with XmpStateChanged (0A000001)
queued, which this thread never reads (it only asks for 0A000003), so the
listener stays signaled for good, every wait returns index 0, and
F8000120 (voice callbacks: buffer and stream end) is never serviced. Sound
that waits on those callbacks never finishes, and a thread that waits for
it (stopping voices at a load) hangs. It also burns a core (94% for that
thread in AC). xenia-edge fixed exactly this in a573178da ("a wait +
XNotifyGetNext(id) loop spun while other notifications were pending"): the
listener's event is auto-reset (a wait takes the signal), one entry per
notification (a repeat replaces the data), and a read clears the signal
when it empties that area.
- Fix (next build, owner approved 2026-11-24): that commit ported (git
  show a573178da from refs/remotes/xenia-edge-deep/HEAD; identical files
  except the follow-up 4c222dc85, system-only notifications, which needs
  XamNotifyCreateListener's is_system), with a Checks note in
  docs/ios16_backport_variables.md for QueueKey, RemoveNotification,
  Notifications and its locals. To check: AC past 43 s, BL2 past the intro movie, Dishonored
  past its load, audio in UE3 games; no "XNotifyGetNext; guest lr" repeat
  warnings; Spider-Man WoS still reaches the menu (it was primed by the
  same change).

Other findings, not fixed yet:
- user_language/user_country are defined twice (half-port):
  xboxkrnl/xboxkrnl_xconfig.cc DEFINE_string ("English", "United States",
  read by xam_locale.cc and user_tracker.cc through DECLARE_string) and
  kernel/xconfig.cc DEFINE_int32 (1, 103, read by xeGetLanguage through
  XConfig and achievement_manager.cc). Both upstreams have only the int32
  ones (their xboxkrnl_xconfig.cc is 107 lines of exports). The saved
  config holds the strings, the int cvar flags them, MigrateLegacyCvars
  then maps the names, hence the box every launch. Which definition the
  binary really uses is unclear (same symbol, two types); worth porting
  upstream's xboxkrnl_xconfig.cc and checking the language games get.
- "uploaded" in the perf line is shared memory revalidated after CPU writes
  (zero-copy buffer: no memcpy, but write watches and faults): 60-270 MB/s
  across games (GTA IV highest). A fault counter would say what it costs
  on the CPU.
- Command buffers: 44-53 a frame in AC, Spider-Man, GTA IV (texture loads
  in their own buffers, plan item 3 above).

### Black Ops, Edge of Time, New Vegas (owner, 2026-11-24: zips "Spidey_EoT_and_Fallout" logs 11-36-17 / 11-37-19 + traces, "Edge_of_time_and_fallout" photos 3897-3903)

Title 41560855 is Call of Duty: Black Ops (goal: running into gameplay).

Black Ops (logs 11-54-34 .. 12-07-42, build 9660): never presents a frame.
- Two runs: crash dumps on host-created threads running guest callbacks for
  the first time, the vblank interrupt callback on 01000014 (guest 824A5C58,
  its entry; r3 0 = vblank, r4 the D3D device) and the audio callback on
  0100001C (8251AEA8), both "write at 0x0" with the host PC in the code
  cache; and F800007C jumping to host address 0 (pc 0, retried forever).
- Cause found in the a64 code cache, iOS only: indirection is encoded (a
  slot holds the target's offset in the code cache) and the table is
  reserved readable/writable and zeroed, so a slot no module commit filled
  reads 0 = code cache offset 0, which was the host-to-guest thunk (emitted
  first). A call through such a slot entered that thunk with garbage
  registers: writes to address 0, jumps to 0. XeniOS afbf8cf11 ("Commit the
  indirection table on demand") made 0 mean the resolve thunk by emitting it
  first. Fix (next build): the resolve thunk is emitted first (a warning is
  logged if it does not land at offset 0); ResolveFunction logs the first 16
  targets it can't resolve (guest address, thread, guest lr), and the thunk
  then traps (brk) instead of running garbage.
- Kernel exports Black Ops called that were undefined here (an undefined
  export returns whatever r3 held): XeKeysConsolePrivateKeySign (called once
  at startup; no certificate was written) and IoDismountVolumeByFileHandle
  (twice, cache drive). Ported from xenia-edge (next build):
  XeKeysConsolePrivateKeySign, XeKeysConsoleSignatureVerification (with its
  certificate structs), StfsCreateDevice, StfsControlDevice,
  IoDismountVolume, IoDismountVolumeByFileHandle.
- Also imported and missing, not ported: __C_specific_handler, RtlUnwind,
  RtlCaptureContext (structured exception handling; xenia-edge's
  xboxkrnl_debug.cc is 2035 lines against 198 here, with guest unwinding
  tied to the JIT), IoCompleteRequest/IoCheckShareAccess/... and
  ObIsTitleObject (no upstream implementation either), the NetDll_XNet*
  online calls, XCustomSetAction.
- Crash dumps now print the host PC (offset in the function), LR, x8-x10,
  x19-x21 and the code cache base on ARM64; "Unhandled host exception" lines
  print lr and sp. To check on the next build: no crash dump at launch; if
  frames still don't appear, an "A64 ResolveFunction: no function at guest"
  line or the new registers say where to look, and a Debug log level run
  (kernel calls) would show what the stuck threads wait for (F80000FC polls
  a handle that is still 0, F800007C polls an event with a zero timeout).

Edge of Time (415608B2, log 11-36-17, trace 11-37-06, photos 3897-3899):
the screen is black at 30 FPS with the GPU busy. The frame is rendered
(1120x640, 16_16_16_16 targets, lighting, 35 resolves); its last draws
composite into a 2_10_10_10 target, a full-screen textured quad (tex x
interpolated colour, alpha blended, tf0 a 512x64 DXT5 at the time of the
trace) goes on top, and the frame is resolved to a 2_10_10_10 front buffer
(04E81000, 1120x632, fetch format 54 = 2_10_10_10_AS_16_16_16_16, base format
2_10_10_10, endian 0), so the swap goes through the PWL gamma ramp (the only
game here that does). The PWL ramp in the trace is a real curve (base 0x0240
to 0xFF40, the same for R, G and B). The swap path's code checks out; what
is black is unknown: the front buffer, or the PWL/10 bpc present path.
Owner test (no build needed): metal_presenter_gamma_debug = 2 (copies the
front buffer without gamma), then = 1 (a gradient from the presenter). A
picture with 2 means the PWL step; black with 2 and a gradient with 1 means
the front buffer itself. Tools for this: scratchpad tools/xtr_mem.py (replays
a trace with guest memory from its memory reads: LOAD_ALU_CONSTANT, IM_LOAD;
dumps constants, fetch constants and shader microcode at a draw; the ucode
hash matches the Frame Detail PS hash as xxh3-64 of the big-endian words).

Fallout: New Vegas (425307E0, log 11-37-19, traces 11-37-33 and 11-39-10,
photos 3900-3903): everything renders except faces: the player's face black,
Doc Mitchell's face showing a hair/scalp texture. The 4x MSAA main pass
(predicated tiling, 3 bands) draws characters with small per-character
textures (64x64 8_8_8_8 at 1F7FC000, 32x32 and 64x64 DXT1 at 1FB64000 and
1FC7F000) that no resolve in the frame wrote. Black or another texture's
content at such addresses is what stale data looks like: memory reused for a
new face without the texture cache seeing the write, or (if the game builds
faces on the GPU and reads them back) the CPU reading resolve output before
the GPU wrote it (zero-copy; XeniOS c694076c5 and 237a78443 hold fences and
interrupts behind resolve output; this branch holds only shader-done
fences). Owner test: per game, metal_shared_memory_zero_copy = false. If the
faces are right, it's the zero-copy ordering; if not, the texture cache's
invalidation of those pages.

user_language/user_country (next build): xboxkrnl_xconfig.cc no longer
defines them as strings; GetUserLanguageValue/GetUserCountryValue read
XConfig's int32 cvars (defaults English, United States, as before), and a
saved name ("English") that MigrateLegacyCvars maps is no longer reported as
an invalid type, so the box at every launch goes away.

### Owner's runs on build 9670 (zip "9670_logs-1", 2026-11-24, logs 19-02-27 .. 19-25-28) and the next build

Confirmed fixed on 9670: AC Revelations runs again; Dishonored and BL2 load
into gameplay with audio (notify listener fix). Not yet reported: the config
box at every launch.

Call of Duty: Black Ops (41560855, logs 19-05-37 and 19-05-51): black screen,
no crash dumps (the resolve thunk fix holds).
- Run 1 reached the game's own XamLoaderLaunchTitle ("iOS: Queued title
  relaunch", line 2157) and the relaunch failed with C00000BB: the target is
  the GoD package file, and the callback appended default.xex to it, a path
  inside a file. Fix (next build, XeniOS HEAD's form): a .xex target gets its
  directory + module, a directory gets / module, anything else (a container)
  is launched as is and its launch finds the module from
  cvars::launch_module.
- Run 2 never got that far: F80000FC polls a handle that is still 0,
  F800007C waits on an event with a zero timeout in a loop, the main thread
  sits in a KeDelayExecutionThread loop. The difference from run 1 is the
  InfoCache (Library/Caches/xenia/modules/<sha>/executable_addr_flags.bin),
  written by run 1 and loaded by run 2: its is_return_site flags send calls
  through the longjmp path in ResolveFunction. That path could land on a
  function's first instruction (skipping its prolog: the old test compared
  against machine_code(), which is before the prolog-less body, so it never
  rejected anything). Fix (next build, XeniOS 6e304ab86): never resume on a
  function's first instruction; the first 16 resumes are logged ("A64
  ResolveFunction: longjmp to guest ..."). If it still stalls: per-game
  disable_instruction_infocache = true turns the longjmp path off (no flags),
  which says whether it is involved.
- The owner then launched Dishonored in the same session: C000010A
  (X_STATUS_PROCESS_IS_TERMINATING). TerminateTitle sets the processor's
  title stop request and nothing cleared it, so every later launch in the
  session failed until the app restarted. Fix (next build): EmulatorThread
  clears it (ClearTitleStopRequestIOS) before each launch. Black Ops'
  relaunch depends on this too.
- Still not ported: xenia-edge 02ad00c2b (a longjmp resumes only in its own
  function's frame), the SEH exports, Io share access, NetDll online calls.

Spider-Man: Edge of Time (logs 19-19-27, 19-20-31, 19-23-58): with
metal_presenter_gamma_debug = 2, 2D (logo, save select) shows and 3D stays
black; with 1, the same gradient throughout. So two problems:
- The PWL present: the front buffer is 2_10_10_10 and asks for a red/blue
  swap the source view can't do; the gamma pass wrote the guest output and
  the swap then read and wrote that same texture in one dispatch. Fix (next
  build): in that case gamma writes gamma_output_texture_ and the swap
  writes the guest output.
- The 3D: the game resolves its HDR scene to 16_16_16_16 with number format
  2 (unsigned integer, bits 13:15 of RB_COPY_DEST_INFO) and exponent bias 11,
  and samples it as an integer texture (fetch word 3 bit 0) with exp_adjust
  -11. The console stores round(x * 2048) as the bits and returns them as
  integers; here the resolve packed unorm (saturate, so 1.0 at most) and the
  fetch returned unorm, so the scene came out at most 1/2048 of its value:
  black. Fix (next build): resolve.xesli divides the exp bias factor by 255
  or 65535 for unsigned integer 8- and 16-bit destinations; the SPIR-V fetch
  multiplies integer fetches of 8- and 16-bit-per-component formats by
  2^bits - 1 (unsigned) or 2^(bits-1) - 1 (signed, biased), as XeniOS HEAD's
  GetIntegerScaleBits; constant swizzles and gamma are left alone. Other
  integer formats (10_11_11, 2_10_10_10, 24_8, 32) are not scaled yet.

Fallout: New Vegas (log 19-25-28): faces still wrong with
metal_shared_memory_zero_copy = false, so it is not the zero-copy ordering.
The photos (3901-3903) show Doc Mitchell's face mesh drawn with another
texture's content (hair-like, with the face's UVs): a texture cache entry
that kept old bytes after the CPU rewrote its memory. Cause found in the
write watches, iOS-specific (16 KB host pages, 4 KB guest pages):
PhysicalHeap::EnableAccessCallbacksInner decided whether to arm a 16 KB host
page from its first 4 KB guest page only. In 0xE0000000 (4 KB pages, 4 KB off
the host pages) a texture in the last 4 KB of a host page whose first 4 KB
were free, or read-only, got no watch, while SharedMemory marked its range
valid: CPU writes there were never seen. One of the face textures,
1FC7F000, is in the last 4 KB of its 16 KB block. Also, Release, Decommit
and Protect-to-writable only invalidated ranges that had a watch armed, and
Alloc invalidated nothing, so memory handed out again (a new face texture
where the hair was) could keep the old texture. Fix (next build), the write
watch parts of xenia-edge aed81ca93 and d1541a579 (both in XeniOS HEAD):
arming and unprotecting use the most permissive access of all guest pages
in the host page (PhysicalHeap::SystemPageGuestAccess, as BaseHeap::Protect
already did), and Alloc/AllocFixed/AllocRange, Release, Decommit and
Protect-to-writable invalidate even with no watch armed. Not ported: the
read watches (data providers) in aed81ca93, and its change to report a
fault on an unwatched page the guest can't write as unhandled (iOS retries
unhandled faults anyway). If faces are still wrong: a Frame Detail run in a
dialogue, and FNV's integer fetches of other formats (depth 24_8 among
them), which the fetch scale doesn't cover yet.

Rainbow textures while Dishonored and BL2 first load (owner's photo):
optimisation lead, not a bug yet. UE3 streams textures in, and the colours
look like mips or memory not yet uploaded being sampled; samplers already
clamp the minimum mip level. To look at once the hangs are done.

### Owner's runs on build 9672 (zip "9672logs-1", 2026-11-24, logs 20-18-01 .. 20-47-18, 4 traces) and the next build

Owner: New Vegas faces fixed (write watch port). Dishonored's rainbow on
first load goes away on a relaunch (shaders compiling, draws skipped until
then; also when something is overlaid, e.g. confirming a new game): back
burner. Focus: Black Ops, Edge of Time, AC Revelations sound effects.

Black Ops (logs 20-23-07, 20-24-28, 20-25-07 with Debug level): no relaunch,
two "A64 ResolveFunction: longjmp" lines per run, both on thread F800007C at
825B1768 / 825B17B8 in the function at 825B1718, frame 4 of 11/12. That code
is `li r3,2; bl 82341A90; bl 825FB3A0; cmpwi r3,0`: setjmp on the thread's
abort jmp_buf, the Quake-family Com_Error recovery. So these are real
longjmps, landing correctly; the game raised an error twice. In 9670 the
same point was its XamLoaderLaunchTitle(default.xex): the fatal error path
restarting the title, not a normal first-boot relaunch. The trigger, in the
Debug log: F800007C calls NetDll_recvfrom on its non-blocking VDP socket
(port 1000, remapped to 11000), then NetDll_WSAGetLastError, then the
longjmp. AsioErrorToWSAError returned the host errno, 35 (EWOULDBLOCK on
Darwin) instead of WSAEWOULDBLOCK (10035), so an empty socket read as a
network failure. xenia-edge 250fb40dc maps asio errors to Winsock codes
(its comment: COD4 MP treats an unrecognized recvfrom error as a hard init
failure); ported (next build). After the errors F800007C waits on F800006C
with a zero timeout forever, F80000FC polls handle 0 and the main thread
loops in KeDelayExecutionThread (824DD8FC) re-trying a path: all downstream.
Also seen: the game formats \Device\Harddisk0\Cache1 (a NullDevice here)
twice at every boot and gets C000014F (unrecognized volume) back; xenia-edge
c628a23df keeps the cache across formats (not ported, not a blocker).

Launching a second game in a session (log 20-25-50): Dishonored (425307E3)
was launched after AC Revelations and its module reported title 55530879,
AC Revelations again: MountPath appends the new package to \Device\Cdrom0
behind the old one and resolution finds the old one first, so the old game
restarted on its leftover kernel state (its old main thread still in the
object table) and hung ("last frame, one second of audio, then dead").
XeniOS tears every title down with Emulator::ResetTitle (xenia-edge: stop
GPU/audio/scheduler, terminate threads, Shutdown, Setup again). Fix (next
build): after LaunchPath, EmulatorThread marks the CPU side uninitialized,
so the next launch (library or the title's own XamLoaderLaunchTitle, whose
launch data is in cvars) goes through the Shutdown + Setup the first game
launch uses: a fresh kernel, file system and objects per title. The kernel
call thread report (shim_utils) never drops threads, so a session's later
reports still list the earlier title's threads; noise only.

Edge of Time (log 20-20-16, traces 20-20-47, 20-21-43, 20-22-43): renders,
with a rainbow over it and the scene in three places. 2925 draws failed
"SPIRV-Cross: Failed to prepare pixel shader": PS 126C706122A7280B (the
deferred lighting shader, 16 textures) hit "Compiler encountered an internal
error" in the Metal compiler after 10.6 s. On 9670 it compiled in 747 ms but
was already 108 KB of SPIR-V and 976 KB of MSL; the integer scale (9672)
added a fetch-constant load, format compares and selects to every fetch,
+15% SPIR-V over the game's pixel shaders. Without the lighting pass the
frame shows the G-buffer (likely the rainbow) and leftovers. Fix (next
build): the CPU (MetalCommandProcessor::UpdateSpirvSystemConstantValues)
puts the component width code (0 none, 1 = 8 bits, 2 = 16 bits; 0 with a
constant 1 in the swizzle) in bits 24:27 / 28:31 of texture_swizzles, which
only used bits 0:23; the shader derives 2^bits - 1 and half of it and picks
by sign: no format logic in the shader, an exact 1.0 for every other
texture. The trace also shows what the integers are for: depth packed into
8888 integers (from 16_16_16_16_FLOAT and 16_16_16_16 targets) read back as
24_8_FLOAT integer textures, and normals as 16x4 integers (bias 16) read
back biased with exp_adjust -15. If the three areas remain once lighting
runs, the next step is the composite's EDRAM placement.

AC Revelations sound effects late in cutscenes (voices and music in time):
not found yet. The guest XAudio2 mixes every voice before
XAudioSubmitRenderDriverFrame, so the host queue (apu_max_queued_frames 8,
~43 ms) delays all sound alike; a kick blocks until the XMA worker finished
that context, so the dedicated XMA thread adds no start latency either.
Left: the game's own sound path (its threads run 300%+ of a core in this
log) or XMA decoder correctness: xenia-edge has ~30 XMA commits from
Aug-Nov 2026 we lack (loops, seeks, split frames, a stall where "the rest
came a kick later", plus two ffmpeg updates); a large port to do with data.
Asked the owner for A/B runs (use_dedicated_xma_thread = false; xma_decoder =
"old"). Also ported (next build): NtRemoveIoCompletion keeps the 64-bit
timeout (xenia-edge d287b30be; truncated, a relative timeout waited for
weeks) and returns on an invalid handle instead of dereferencing null;
AC Revelations doesn't import it.

### Owner's runs on build 9673 (zip "9673_logs-1", 2026-11-24, logs 21-39-56 .. 22-49-14, 8 GTA IV/other traces) and the next build

Owner's report: Black Ops reaches menus and gameplay; Edge of Time renders
with lighting (some post-processing missing; vertex explosions at one Time
Vortex chamber angle and on web-construct heavy attacks); relaunching works
(Black Ops -> Edge of Time, ACR -> ACR); ACR sound effects in time with
defaults; Batman AA and AC run with shader-load stutter; AA intro walk
25/30 FPS; Arkham City story stalls waiting for a dialogue line, and after
long play all dialogue and music stop and are skipped; GTA IV cutscenes
12-17 FPS, gameplay pre 70-100+ ms, GPU ~40 ms, run 2 killed the game.

GTA IV run 2 (22-49-14) ended in `XamShowDirtyDiscErrorUI` from the
streaming thread F8000C60, right after the cache copier (F8001014) wrote
bronx_e.img and bronx_w.img. The game copies every .img to cache: on every
boot (89 and 80 files in the two runs) because the times it stamps are lost
(`SetWriteTimestamp` only logged). Found while reading that path:
1. **filesystem_posix.cc OpenExisting OR'd O_RDONLY (0) and O_WRONLY (1)**:
   a read/write handle got a write-only descriptor, so any read through it
   failed (`HostPathFile::ReadSync` -> END_OF_FILE). Every writable device
   (cache, content, saves). Fixed as XeniOS HEAD / xenia-edge 1e7c1f677,
   c9536890c (O_APPEND only for append-only handles), b9e00e6cc.
2. **Entry::Delete freed an entry with open handles**, and Overwrite /
   OverwriteIf opens were delete + recreate, so another handle on the file
   was left with a dangling entry. Ported xenia-edge 494e2ef67's open-file
   count (RetainFile/ReleaseFile, delete-on-close at the last close) and
   in-place overwrite.
3. **File times**: xenia-edge 98fad6f14 (futimens through the title's own
   handle, 2 s units). Expect "Tried to change write timestamp" lines gone
   and, from the second boot on, far fewer .img copies.
4. **X_FILE_BASIC_INFORMATION.attributes** was a 64-bit read of a ULONG +
   padding (xenia-edge 8a179c7af): set-info chmod'ed by the padding's bit 0,
   and basic-info queries returned the attributes in the padding.
Which of 1/2 caused the dirty disc is not proven; both are real.

GTA IV profile (22-49-14, gameplay windows, 120 frames each): frame 58-81
ms, GPU busy 75-100%, threads XThreadC603 ~78%, GPU Commands 75-89%.
GPU per frame: draws 24-26 ms (56 passes), rt transfer 23-24 ms (50
passes), texture load 18-19 ms + upload blit 8 ms (350-430 loads), edram
dump + resolve ~9 ms. "uploaded" 3-5 GB per 120 frames (~35 MB a frame):
with zero copy that is pages made valid again after CPU writes, each a
write-watch fault and re-protect on 16 KB pages.
- Ported XeniOS 0f21056f3: `RequestRangeForGpuRead` for vertex and index
  data (both Metal draw paths and PrimitiveProcessor) skips valid pages and
  the write watch when the buffer is guest memory, and invalidation is not
  widened to 256 KB then. Expect "uploaded" to drop by most of its size,
  texture loads per frame to drop, and CPU time on the game threads to fall.
  Textures, memexport, resolves and the primitive cache keep their watches.
- Kernel call repeat detector: history as a ring (no 62-entry shift per
  kernel call). GTA IV threads make over 10^6 kernel calls a second.
- XMA census (Arkham City): every 10 s an info line "XMA census: N
  contexts allocated, E enabled, 0 stalled", or a warning with the first
  stalled context's fields when enabled contexts holding input did not move
  for 10 s. Theory: UE3 has a fixed voice pool; a voice whose XMA context
  never ends holds the story line, and once the pool is full new sounds are
  skipped. If the census shows stalled contexts climbing, the fix is the
  XMA decoder of XeniOS HEAD = xenia-edge HEAD (identical there; ~20 fixes
  of Nov 6-10: loop jump stall, XMABlockWhileInUse on a disabled context,
  split frames waiting on a late buffer, one-shot looping, ...), ~700 lines
  in xma_context_new.cc plus the context layout (decode_subframe) and the
  audio system pacing. Not ported blind: ACR now works on ours.

Optimization plan (in order, each measured on GTA IV and Arkham City):
1. Check this build's upload/texture-load drop.
2. Texture reloads from 16 KB false sharing: XeniOS d98568820 (hash the
   guest bytes, skip unchanged reloads) needs Metal load-completion tracking
   this branch's texture cache lacks (texture_cache.cc diverged ~800 lines).
3. Batch texture loads (one compute encoder per draw's loads, blits after)
   and resolves (edram dump + resolve in one encoder): 350-430 passes a
   frame in GTA IV.
4. Render target transfers (23 ms in GTA IV, 50 passes): find which
   ownership transfers repeat per frame from a trace.
5. Plan items 3 and 5 of the Arkham City section (command buffers, tile
   resolve).
6. CPU: the always-on diagnostics (stuck-thread reports, call tracker,
   hotspot sampler) behind one switch once hangs are rare; JIT code cache
   persisted per title.

Owner's wishlist (2026-11-24), assessment:
- Multi-disc (install disc + play disc, e.g. GTA V): feasible. Installed
  content already persists; needs disc sets in the library, remounting
  \Device\Cdrom0 at a swap prompt (XamSwapDisc / the game's "insert disc"
  wait) without a title reset.
- Options reorganisation: UI only, easy.
- Customisable touch controls with shareable presets: feasible; XeniOS has
  an editor and preset format; our iOS UI diverged, so port the layout model
  and preset JSON, not its view controllers wholesale.
- Touch input lag: measure first (touch timestamp -> XInputGetState ->
  present). Likely wins: state updated in the touch handler, 120 Hz touch
  on ProMotion, fewer frames queued in the presenter.
- Repeated inputs (QTE mashing): XInput is polled, not interrupt driven; a
  press and release between two polls is lost. Latch presses so each one is
  seen by at least one poll, and bump the packet number per change. Small.
- Toggle to stop tap-to-menu, with a menu button: easy.
- Portrait layout (screen in the top half, controls below): feasible.
- Shader cache: we have async compiles, the MSL disk cache and the pipeline
  binary archive; next is a cache keyed by shader hash + translator version
  (survives app updates), placeholder draws instead of skipped ones
  (rainbows), and import/export per title. spirv2air floors at iOS 18.
- Precompile from the long-press menu: compiling everything a game will
  use without running it is not possible (shaders appear as the game draws);
  "compile everything this title has cached so far" in the background is.
- rexglue-sdk: an ahead-of-time PowerPC -> C++ recompiler (early
  development; CI for Windows/Linux/macOS, no iOS). Per-game output, a
  native binary per title, built off device; not an emulator feature.
  Within reach instead: persist our JIT output per title.
- Achievements: AchievementManager with the GPD backend already records
  them; needs an iOS toast (imgui_guest_notification exists) and a list in
  the profile view.
- Xbox-style system prompts (keyboard, sign-in) with controller navigation:
  feasible with the existing ImGui dialogs plus gamepad navigation, behind a
  toggle.
- In-app .zar compression: XeniOS has zar_converter.cc and the
  long-press action; third_party/zarchive is here; a port.

### Owner's runs on build 9674 (zips "9674_logs-1", "xenia_2026-11-25_06-14-31", "xenia_2026-11-25_06-22-12": GTA IV 05-39-34 and 06-22-12 debug, Arkham City 05-50-46, 06-01-21 and 06-14-31 debug) and the next build

Owner: GTA IV no crash in two runs, but the Metal HUD still shows the CPU
side at 80-100 ms and the GPU at ~44 ms in gameplay. Arkham City: the story
stalled waiting for a dialogue line again (music kept playing this time).
The owner recalls the missing line was a regression introduced later in
upstream, so the fault is likely in code brought from upstream; asked for
our own fix, not an upstream port, and offered diagnostic runs.

GTA IV (05-39-34):
- "uploaded" fell from 3-5 GB to 10-50 MB per 120 frames (zero-copy GPU
  reads); texture loads from 350-430 to ~50 a frame.
- Two areas: frame 50-58 ms with the GPU 100% busy, and 75-97 ms with the
  GPU 45-60% busy and no thread saturated: serialised, not loaded.
- ~42 command buffers a frame (5,000 per 120). Causes in the SPIRV-Cross
  path: IssueCopy commits a draw-containing command buffer at every resolve
  (~43 a frame, the 3 predicated-tiling tiles 1280x256/256/224 at 2x MSAA),
  IssueDrawMsl commits again when a draw samples resolved memory, and a
  texture load requested while the render encoder is open goes into a
  command buffer of its own.
- GPU per frame: draws 25 ms, rt transfers 24 ms (51 passes, mostly the
  tiles' 2x MSAA colour; little shader time, mostly pass boundaries and
  attachment traffic, 1.1 GB a frame), texture load + blit 9 ms, edram
  dump + resolve 8 ms, host depth store 2 ms.
- XThreadD403 (~80%) is the audio thread: XMA decodes on the kicking
  thread (ff_wma*, MDCT, ConvertFrame) plus guest mixing. XThread11027
  spends 20-45% of its samples in syscall_thread_switch (Sleep(0) polling).
Changes:
1. metal_resolve_keep_command_buffer (default on): a draw-containing command
   buffer stays open after a resolve, as copy-only ones already did, and is
   committed at primary buffer end / ring-empty waits / swap. A draw that
   samples a texture to be loaded from memory a resolve wrote this frame ends
   the render pass, loads the textures into the same command buffer (after
   the resolve) and begins the pass again (attachments loaded; a first-bind
   clear is not repeated, the descriptor is rebuilt with Load). Shader-done
   fences are still written when the buffer completes. Off restores the
   commits. Risk: a game reading resolve output on the CPU between the
   resolve and the commit (no read watch here, unlike XeniOS 237a78443).
   Expect "N command buffers" per 120 frames to drop well below 5,000.
2. sleep_zero_backoff_us (default 50): a thread making 64 quick Sleep(0)
   calls in a row sleeps 50 us per call instead of yielding.

Arkham City (06-14-31 debug, 06-01-21):
- The census was wrong: Work() clears the enabled flag at the start of the
  kick, so "enabled" was always 0 and nothing could count as stalled. Now
  counted by state: "XMA census at T s: N allocated, H holding input, S of
  them unchanged for 10 s; context ...".
- XMA contexts are never released in gameplay (ids handed out in order,
  allocated count 110-180 then flat, also in 06-01-21 where sounds later
  resumed): the game caches voices; no pool exhaustion.
- No decoder errors: no error_status 4 exits, no "Decode skipped".
- In both runs new voices stop starting for long stretches while ~7
  long-lived contexts (music, ambience) keep decoding (06-14-31 after line
  2.81M; 06-01-21 lines 1.4M-3.95M, then new ones again).
- Full output ring: when a decode fills the ring exactly (write == read),
  Work() clears output_buffer_valid ("Output ring buffer empty"). The guest
  reads valid 0 with write == read as full (context 6 read 8 blocks after
  one), so no audio is lost there. The same code would invalidate a truly
  empty ring if a context with no input were kicked; never seen.
- StoreContextMerged writes back output_buffer_read_offset (the guest's)
  and input_buffer_read_offset from the snapshot at Work() start, so a guest
  change made during a decode from another thread would be lost. Kicks and
  mixing are on the same thread here (F800021C), so not shown to matter.
- Sleep(0) storms: F8000108 (87M calls), F800029C and the main thread
  (TryEnterCriticalSection / LeaveCriticalSection / Sleep(0)), through
  XAPI SleepEx at 82D3E83C (interval = ms * -10000; Sleep(0) yields).
  Normal for the title, but each pins a host core.
- Next diagnostic runs: (a) the same story point with xma_decoder = "old"
  (the older decoder; if the line plays, the fault is in XmaContextNew);
  (b) default decoder, note the census time stamp when it stalls; (c) which
  upstream build the owner remembers as first bad.

### Owner's runs on build 9675 (zip "9675_logs-1": GTA IV 07-17-47, Arkham City 08-16-05 default and 08-21-56 old decoder, ACR 08-25-35/08-30-33/08-32-46, EoT and Batman AA 08-36-10, 4 traces) and the next build

Owner: GTA IV stutters more, times a little lower. Arkham City: with the
default decoder the line dropped (~08:19:26); with xma_decoder = "old" (set
globally before launch) it played and the story continued. Owner recalls the
regression between upstream 2.0.0 and 2.0.1; fix asked for on our own. ACR:
save-select menu flickers with metal_resolve_keep_command_buffer on (two
runs), fixed with it off. EoT better with it on; still missing lighting
effects and the time chamber's blue glow. Batman AA: compiles faster, pre and
GPU still ~high 40s ms. Signed-in profile not remembered after an app restart;
settings changed after a game was launched and closed reportedly not saved.

Arkham City root cause (XmaContextNew):
- FFmpeg's wmapro/xmaframes decoder sets skip_frame on open and returns no
  samples for the first frame (it keeps only its overlap). XmaContextNew
  reopens the codec whenever a context's sample rate or channel count
  changes (dialogue voices move between 24 and 32 kHz) and then advanced past
  that frame with nothing written, so the stream came out one frame (512
  samples) short. XmaContextOld returned without advancing and decoded the
  frame again, so its count stayed right.
- Evidence (9674 debug logs 06-14-31 / 06-01-21, tools/xma_reinit_reuse.py in
  the scratchpad): every stream whose context was never used again by the
  game had lost its first frame (173 / 115); every stream that kept all its
  frames was followed by reuse (255 / 289). The guest's voice waits for the
  buffer's samples, so a short one never finishes: the dialogue line never
  ends, and voices leak until nothing new plays ("all audio buffers out").
  The census never saw it because those contexts hold no input.
- Fix: a frame FFmpeg gives no samples for (first after open, or a decode
  error) is output as silence; every frame produces its subframes, as on the
  hardware.
- Seen but not changed: loop jumps happen before the loop-end frame is
  decoded (that frame is skipped and the loop-start frame is cut instead);
  error_status 4 (split frame whose next buffer is not valid yet) is never
  cleared and makes every later Work() run one pass. Neither seen in these
  logs.

metal_resolve_keep_command_buffer: default off again. Command buffers fell
from ~5000 to ~500 per 120 frames, but copy_resolve_writes_pending_ stays set
for the whole buffer, so every draw reading resolved memory with changed
texture bindings restarted the render pass: GTA IV's main pass went from ~2
to 24-88 passes a frame, attachment traffic 1.6-4.3 GB a frame (was ~1 GB),
GPU-bound frames 57-68 ms (9674 49-62 ms on the same route). ACR's menu
flicker cause not pinned (trace 08-27-52: 48 resolves a frame into a 4096x1024
atlas sampled right after). Kept as a per-game opt-in for EoT.

Profile sign-in: ProfileManager::UpdateConfig used OVERRIDE_string, which on
this branch sets the per-title value (not written by SaveConfig, cleared at
the next title load), so the slot was always saved empty. Now
OVERRIDE_PERSIST_string, as XeniOS and xenia-edge HEAD. Settings: the iOS
screens write config_value and SaveConfig at once and config is read only at
app start; the logs show changes made in short sessions persisting. Need the
owner's exact steps (which screen, game running or closed, which setting).

sleep_zero_backoff_us stays 50: XThread10E03 in GTA IV now sleeps in it
(__semwait_signal) instead of yielding; the frame-time change is the pass
splitting above. A/B with 0 if stutter remains.

### Spider-Man: Edge of Time frame anatomy (trace 9675 08-38-17, frame 1) and the fast-math A/B

Owner: lighting present, but some lighting effects, post-processing and the
time chamber's blue glow missing; vertex explosions at one chamber angle and
on web-construct heavy attacks. Same on every build so far.

Frame (tools in the scratchpad: xtr_pass.py passes with xxh3 VS/PS hashes,
xtr_regs.py, xtr_bools.py, xtr_find_shader.py, xenos_ops.py opcode counts,
xenos_cf.py control flow; xtr_replay.py is the importable memory replay):
- G-buffer D115 (16_16_16_16_FLOAT + depth D24FS8 at 1120x632), depth resolved
  as raw 32-bit (fast copy path) and read back as 24_8_FLOAT integer fetch.
- Lights D256-D284: stencil-only volume (mode 5) then additive light into a
  16_16_16_16 fixed target (-32..32, emulated via exp bias -5), resolved with
  exp bias 11 to 16x4 unsigned integers, read back with exp_adjust -11.
- Emissive/transparent pass D443 (47 draws, VS ACFEB0B41C1457D7 with an
  aL loop, loop constant L0 = 1 iteration), glow mask D499 (8 additive draws
  into an 8_8_8_8_GAMMA target, resolved as k_8), glow extraction D526 (PS
  859476699225B6B3: HDR scene + depth, 3 IEEE rcp) and blur to 0628C000,
  bloom 288x158 / 160x79 chain (PS BF0D2886..., D8105EC2..., E48E4905...).
- Composite D532 PS 126C706122A7280B: an uber-shader with feature bools
  b128-b140; this frame b129 (glow t5 + depth t4), b130 (bloom t1), b131,
  b133 (t2 mask, 4 taps), b135 (t7). The "No valid binding for fetch 9/10/11"
  warnings are blocks b138-b140, off here. It uses 9 IEEE log and 12 exp
  (pow), lights use IEEE rsq/rcp/log/exp.
- Checked and fine: every draw encoded (no compile skips), 16-bit fixed
  range, integer resolves/fetches, depth resolves raw, gamma output and gamma
  fetch on the SPIR-V path (gamma RTs blend in gamma space, a known
  inaccuracy that brightens, not removes), clip-disabled draws are only
  screen-space quads.
- Lead: Metal fast math (on for all translated shaders, as upstream) lets
  the compiler assume no NaN/infinity, so the translator's SM3 checks
  (0 x anything = 0) and infinities from rcp/rsq/log can be folded away:
  exploding vertices and NaN/zero HDR terms. New metal_shader_fast_math
  (default on) to A/B per game.
- Next data needed: photos of the time chamber and a web-construct attack
  with a GPU trace at that moment, with metal_shader_fast_math on and off.

### Owner's runs on build 9677 (zips "9677_logs" and "EoT-1": EoT 11-30-29 with 5 stream traces and photos 3923-3953, EoT 12-52-40, Arkham City 12-54-11, GTA IV 13-00-39) and the next build

Confirmed by the owner: Arkham City now passes the dialogue line on the
default decoder (the XMA silence fix), and the profile stays signed in across
app restarts.

- Arkham City, new: during the camera sweep to the warehouse the GPU
  Commands thread (01000010) faults in a system library (pc 0x21FEE9290, lr
  0x21FEED8C4, both in the shared cache, probably memmove), address
  0x4292E4000 (page aligned), retried 6 million times = the hang. No
  "Unable to decode MMIO" line, so not an MMIO range. If guest memory was at
  host 0x3092E4000 the address is the first byte past guest physical memory
  (a copy running past 512 MB), but the base was never logged. Metal shared
  memory is zero copy here, so it is not UploadRanges. Diagnostics added: the
  first unhandled fault logs read/write and a named frame-pointer backtrace,
  MMIOHandler logs the guest address of a guest-memory fault that is neither
  MMIO nor a watch, Memory logs the host address of guest memory. Next run
  of the sweep names the caller.
- EoT with metal_shader_fast_math=false per game (the log confirms it was
  applied, and MSL shaders are compiled from source each run, so it took
  effect): no change. Photos: big spikes on the skinned enemies, growing
  with each event. The skinning VS (e.g. 03492900938ABAFD, 0F5D17BB987908DA)
  builds the bone index from an 8_8_8_8 integer fetch times c254 (mulsc) and
  maxas (rounds, writes only a0), reads c[66+a0].. c[68+a0]: exact, and the
  VS constants in all 5 traces are finite (max 1849). Several skinned VS add
  a second stream, fc94 (32_32_32_FLOAT, stride 3, at 0327B000, 13917 words,
  shared by most skinned draws), to the position: a per-vertex offset buffer
  the CPU writes, the likeliest place for an error that grows over time. The
  stream traces hold no vertex data (all zeros): Metal never recorded the
  memory draws read. Ported XeniOS HEAD's WriteMemoryRead per upload range in
  MetalSharedMemory, so the next trace shows fc94 frame by frame. No EoT VS
  uses memexport. The recomp (goliathret/EdgeOfTimeRecomp) is a D3D-level
  reimplementation (not Xenos), with gpu_allow_invalid_fetch_constants,
  execute_unclipped_draw_vs_on_cpu and readback_memexport off in its config;
  its credits say Boma (goldislead) fixed EoT on xenia-edge, and their
  xenia-edge GPU commits include scalar maxas clamped to [0, 255] (we clamp
  to [-256, 255]), out-of-range vertex fetch words read as 0 (9e9d3cdd3),
  and 21-bit rounding of scalar approximations (3a44f20c7, which we have).
- A64 note: RSQRT_V128/F64 here still call the C lane functions with a bare
  blr; XeniOS a3a4cd468 replaced them with emitted helpers because guest
  values in v8-v31 are not preserved across a C call (safe only while the
  callees compile to integer code).
- GTA IV: in town, frames 90-113 ms, GPU 37-43% busy, uploads ~0.6 MB a
  frame, CPU bound: XThreadE913 48-83% (hot 8216D90C, 828C6790, 828C2D78;
  ff_wma decode also there, so likely the audio/stream thread), Main and
  XThreadE003/14F07 share 829CFED0 (hot). The hotspot sampler now keeps
  totals since launch and logs the code around each top function's hottest
  instruction once, so the hot paths can be disassembled offline.

### Owner's runs on build 9679 (zips "9679_logs" and "gpu": Arkham City 21-03-34/21-06-12, EoT 21-25-31/21-29-37/11-15-09 with 3 stream traces, GTA IV 11-20-04..11-45-42 with one trace) and the next build

- Guest memory is mapped at host 0x2A0000000 (virtual) / 0x3A0000000
  (physical) in every run, so it ends at 0x3C0000000. If 9677 used the same
  base, Arkham City's fault at 0x4292E4000 was outside guest memory: a host
  buffer (a memmove past the end of one), not guest memory. It did not recur
  (the owner corrected the place: a camera sweep down a street to the
  courthouse). The backtrace logging stays for the next time.
- Hotspot totals: the top function in all three games is the same Xbox D3D
  library routine, a spin-wait for command ring space (4 x 8 `or
  r31,r31,r31` (db16cyc), then read the read pointer at 0x2B10/0x2A90 of the
  device): EoT 82231CF0 59% (+ its caller 82231310 22%), GTA IV 829CFED0
  11-20%, Arkham City 82247140 7%. db16cyc compiled to `yield`, a no-op on
  Apple cores, so the waiting thread held a P core (EoT XThread15403 99%)
  from the GPU Commands thread it waits on. Ported XeniOS 9ae719d98: two
  db16cyc less than 1 us apart sleep the host thread 60 us
  (release_core_in_spin_waits, default on). Other hot code: Arkham City
  82CFDxxx is an FPU abs/fsel peak scan (audio meter); GTA IV 8216D90C
  (audio thread E913) a VMX vmaddfp filter loop.
- GTA IV and metal_resolve_keep_command_buffer: the owner's "near 30 fps
  then 15-17" is visible: keep-CB on gives ~5 command buffers a frame
  instead of ~40 and frames of 30-40 ms at first (55-65 ms off), but draw
  passes per frame climb 80 -> 150 -> 390 and attachment traffic to ~4.4 GB
  a frame, frames 70-80 ms. Cause: the restart check ran whenever any used
  slot was out of sync (every binding change) and the slot's memory had been
  resolved at any time in the frame (resolved_memory_ranges_ is per frame and
  merges ranges past its cap), even when the texture was already loaded since
  the resolve. Now (keep-CB only) it checks only the slots
  GetUsedTexturePotentialLoadMask reports, which now looks the texture up for
  a changed key: no restart unless a load (new or outdated texture) will
  happen. Off by default still; the ACR save-menu flicker with keep-CB is not
  explained (the swap is safe: RequestSwapTexture runs after the commit;
  texture loads with a render pass open go to a separate earlier command
  buffer, which is why the pass must end). XeniOS HEAD instead guards resolve
  output with a read watch (237a78443), not ported.
- apu_max_queued_frames: runs at 8, 24, 32, 16 (all different routes, so no
  clean A/B in the logs). The value is the audio worker's semaphore depth
  (frames of 256 samples, 5.3 ms each): more lets the guest's audio callback
  run ahead in bursts; latency grows (16 = 85 ms, 32 = 171 ms).
- EoT: GPU bound (GPU busy ~110%, 50 ms frames): draws 28.5 ms, render
  target transfers 25.4 ms (31 passes, 5 ms of it shader time), texture
  loads 5.9, EDRAM dump 4.5, upload blits 3.9, resolves 3.5; 6 MB uploaded a
  frame.
- EoT traces still hold no vertex data: with zero-copy shared memory, vertex
  fetches go through RequestRangeForGpuRead, which returns at once, so the
  upload path never records them (7677 memory records, none for fc94 at
  032FF000 or the vertex streams). IssueDrawMsl now records each vertex
  fetch range with WriteMemoryReadCached when a trace is open, and the
  writer clears that cache at every swap, so each frame records them once.

### Owner's runs on build 9680 (zips "9680_logs" and "gpu.rar": GTA IV 13-18-37 keep-CB on per game and 18-47-45, Arkham City 13-25-00, EoT 18-50-53 and 19-52-59 with ACR from line 4030, traces EoT 19-55-50 (138 MB) and ACR 19-58-58/20-00-01) and the next build

- EoT: no exploding vertices any more in any run (owner). The 9680 trace
  holds vertex data (8896 memory records, 279 MB decoded).
- EoT glow: the glow mask is drawn into an 8_8_8_8_GAMMA target writing
  alpha only (color mask 8, additive), then resolved (D629 in frame 1) to
  k_8 with the red/blue swap bit, and the composite (PS 126C706122A7280B,
  t2 = 061DD000 k_8) reads it. Our 8bpp resolve took red, or blue when
  swapped: blue is black here, so every glow mask was empty (Spidey's
  glow, the time chamber's blue glow from the 9675 trace, same D499/D526
  pattern). xenia-edge 2b3f0cb45: D3D's A8 copy is k_8 + LOW_BLUE, from
  XGCopySurface; the swap then selects alpha. resolve.xesli now does that
  (8888 top byte, 2_10_10_10 2 bit alpha, 64bpp high half of the second
  dword, 1.0 for formats without alpha).
- EoT hypersense copies: frame 1 draws Spidey once (meshes of 8240, 11204
  and 2519 indices, fc95 1223A5C0/12261D20/122310A0) in the shadow, G-buffer
  and a third pass, and one translucent copy: D592 depth only (VS
  E24E6E540EC19D98) then D593 (VS E96C88A78902DD66, PS 66904CF55CAF67B9,
  alpha = c58.w = 0.6, colour from a cube map times c59 = 20) of a 3975
  index skinned mesh at 1258A3D0, depth GEQUAL with write; the positions
  are computed identically in both VS and the position is invariant
  (preserveInvariance on). The 358 index additive draws (VS
  E0B5CC4F85057955, a dynamic vertex ring at 0876xxxx) are trails or
  sparks. No draw is dropped (draw stats other=0). The copy should show;
  the glow fix may be what was missing. Recheck with the owner.
- ACR save menu with keep-CB: still flickers (owner). The 20-00-01 trace
  resolves the finished frame (D106, c0 -> 1CB85000 1280x720) and aliases a
  32_32_FLOAT 320x180 reduction over the shadow map memory (D114 -> 1CF1D000
  read as both). A CPU read of resolve output (e.g. a save thumbnail) sees
  the data only when the command buffer completes, which keep-CB delays to
  the end of the frame; XeniOS HEAD guards this with a read watch
  (237a78443). Not ported; keep-CB stays per game (GTA IV).
- GTA IV keep-CB on (per game) after the restart fix: 70-78 draw passes a
  frame (was 150-390), frames 30-46 ms early (GPU ~100%), 55-80 ms later
  (GPU 50-60%). The db16cyc spin is gone from the hotspots. GPU per frame
  in the GPU-bound stretch: render target transfers 17-25 ms for 50 passes
  (4-6 ms of it shader time; no unchanged copies to skip), draws 13-21 ms,
  texture loads 5-6 ms + upload blits 3-5 ms (about 40 each), resolve 3 ms,
  EDRAM dump 3-4 ms (about 42 each). Transfers are the next GPU target.
- GTA IV audio: the busiest thread (XThread1644F, 80-85% of a core) is the
  game's audio thread: samples ~50% guest code (VMX filter 8216D90C, mixer
  828C2D78/828C5AA8/828C6790), ~38% waits, ~3-5% FFmpeg XMA decode (run on
  that thread: use_dedicated_xma_thread is off on ARM64, and on it the kick
  waits for the worker anyway). FFmpeg ran its C paths: our branch turned
  NEON off on ARM64 ("decode parity"); XeniOS HEAD has no such switch. Now
  NEON is on by default (UPDATE_from_bool so saved configs follow). The
  VMX multiply-add stored three sources to the stack and reloaded one for
  every vmaddfp; with FZ flushing inputs it is now fmla in place with the
  NaN fixup in the tail (a64_vmx_nan_fixup_in_tail).

### Owner's runs on build 9682 (zips "9682_logs", "gpu9682.rar": GTA IV 23-02-33 (keep-CB, apu 24) and 23-09-09 (keep-CB), one log 23-16-11 with EoT (trace 23-18-17, 2 frames), Black Ops twice, Arkham City, ACR) and the next build

- EoT hypersense copies still missing; glow and audio fine (owner). Trace
  frame 1: four copies, each a depth-only pass of Spider-Man's two meshes
  (1275 and 10869 indices, drawn twice, VS AE830F371BFC7C3F) and then the
  two meshes drawn additively (blend 01060106) with VS 5B0F2B2D065D0186 /
  PS 20CC72B645FD9349, both with GEQUAL and depth write (00600466): a copy
  shows only where its colour pass reproduces the pre-pass depth exactly.
  The pre-pass VS accumulates every bone-weighted position component with
  mad; the colour VS computes the c68 row with mul and two adds (its
  product is used twice). The translator emits mad as NoContraction mul and
  add, and SPIRV-Cross implements those with fma(l, r, 0)/fma(1, l, r)
  helpers marked optnone, which this branch (and XeniOS) strips; with fast
  math Metal may then fold them and fuse differently per shader, so the
  colour pass fails GEQUAL. New metal_vertex_shader_fast_math (default
  false): vertex shaders compile without fast math (folding the helpers
  needs no-signed-zeros), pixel shaders keep it.
- ACR: no flicker with keep-CB off (its global value; no per-game value in
  the log), so not yet a sign the keep-CB flicker is fixed.
- GTA IV 23-09-09 (keep-CB per game): a heavier scene than the 9680 runs
  (6100 draws a frame), 65-70 ms frames with the GPU 80-95% busy: render
  target transfers 32 ms for 51 passes (8 ms of it shader time; 9 passes
  into a 1280x256 2x MSAA target cost 9 ms, 1 ms of fragment), draws 31 ms,
  texture loads 6.7, EDRAM dump 5.6, resolve 4.8, upload blits 4.0, host
  depth store 2.2. Audio thread 85%, GPU Commands 65%, main 48%. Routes
  differ between runs, so the builds cannot be compared from these logs.
- Black Ops: 543 Metal compiles in the first run (559 s of compiler time,
  about 1 s each; draws waited up to 466 s in the queue), 371 in the second
  (83 s): MSL sources are cached, compiled libraries are not. MSL size
  median 114 KB, p90 227 KB, max 824 KB. CPU bound on its "Backend" thread
  (90-94%, GPU 40-46%); half of its samples are "host" without a symbol and
  syscall_thread_switch. The hotspot sampler now labels samples in the JIT
  code cache outside guest functions "jit glue" (thunks, call resolution).
- FFmpeg: has207/FFmpeg d1963569 here; XeniOS and xenia-edge HEAD use
  dfc5a3993 with "play XMA frames whose data ends early" and "fix XMA
  output running 192 samples late" (owner's decision; not updated). NEON
  covers the FFT/MDCT (tx_float_neon) and window/vector ops (float_dsp_neon);
  what stays scalar is the entropy decoding (ff_wma_run_level_decode),
  which no SIMD helps.

#### Larger changes: plan (2026-11-27)

1. Resolve read watch, to make metal_resolve_keep_command_buffer safe by
   default (GTA IV ran 30-40 ms frames with it before the pass restarts).
   XeniOS: command_processor_resolve_readwatch.inc (655 lines), Memory
   read-watch API (PhysicalAccess, ProvideReadWatchedPage...), commits
   77f2cca80, 935e03876, 16df25981, b83724656, 34387b31f, 237a78443,
   d5806a7d2. Phases: memory read-watch API; the include; Metal hookup;
   default on after ACR and GTA IV runs.
2. Render target transfers (GTA IV 17-32 ms a frame, EoT 25 ms): first
   data with metal_frame_detail_log (per transfer source/destination and
   tiles), then batching or skipping where the destination is cleared or
   fully drawn over, and cheaper passes (most of the time is not shading).
3. Shader compile cost (Black Ops): reduce MSL size (find what dominates
   with dump_shaders), and cache compiled libraries across runs where iOS
   allows (MTLBinaryArchive holds pipelines, not source compiles).
4. XMA: decode on a dedicated thread like the hardware (kick returns at
   once); small gain on GTA IV's audio thread (3-5% of its samples).
5. Long term: EDRAM through raster order groups (as D3D12's ROV path),
   which removes transfers entirely.

Phase 1 of item 1 (next commit after 63d43e136): the Memory read watch from
XeniOS (0fa163a23, 77f2cca80, 935e03876, 78315ec19, 98b6319c5).
EnablePhysicalMemoryAccessCallbacks with data providers arms a page no-access
(notify_on_read); its first CPU access calls the read callbacks
(RegisterPhysicalMemoryReadCallback) without the global lock
(ProvideReadWatchedPage), then under it (TriggerCallbacks), and the watch is
dropped. A write that drops a read watch calls them with kWrite, or kDiscard
for an allocation, release or file read that replaces the pages. File writes
read physical memory through the unprotected physical view (a syscall reading
a no-access page fails instead of faulting); file reads provide the pages
they only partly cover first. Nothing arms a read watch yet, so behaviour is
unchanged. Kept from ours: a write fault that finds no watch still retries.
For phase 2:
- The MMIO handler's QueryProtect race check runs in the signal handler
  (mach_vm_region is not async-signal-safe); XeniOS skips it on Apple and
  lets TriggerCallbacks handle the race. Read watches make such faults
  common, so drop it with the hookup.
- The Metal side (XeniOS metal_command_processor.cc ResolveReadWatchActive,
  FaultCopyResolveToGuestRam, the read callback thunk, NoteResolveForReadback
  in IssueCopy) waits for a completion counter under a condition variable,
  bounded (kResolveFaultWaitBound), from the faulting thread. Ours has
  zero-copy shared memory, so the wait is the whole copy. Needs a completed
  command buffer counter and the guest-sync submit points
  (SubmitResolvesForGuestSync: scratch register write-back, fences, memory
  writes, interrupts) in our command_processor.cc.
- XeniOS's Metal path also holds guest signals behind submissions
  (held_guest_signals_) and adapts per destination; with keep-CB our fences
  are already written at command buffer completion, so start without those.

### Owner's runs on build 9684 (rars "xenia" = Library/Caches/xenia, "com.UltraAssassin64.xenios.ios" = Metal's caches, "9684_gpu-1", zip "9684_logs") and the next build

- FFmpeg moved to dfc5a3993 (owner's go-ahead; what XeniOS and xenia-edge
  HEAD use): first frame after open no longer skipped (d980192), decode
  errors returned as AVERROR_INVALIDDATA and a flush callback (c4f4407),
  output starts 192 samples earlier, at the point the samples are final, and
  a stream's last frame is no longer held back (3913ff9), cut loop-end frames
  marked by bit 0 of the packet's first byte (9307052; our PreparePacket
  leaves it 0, as our loop handling cuts the loop start instead), frames
  whose data ends early play (dfc5a39). Every file our premake lists exists
  in the new tree, XeniOS's CMake list for it is a subset of ours (plus
  libswresample, which we don't use), and the only config macro the new
  headers add (CONFIG_CBS_LCEVC) is in code we don't build. Our frame loop
  still outputs silence for a frame with no samples; with the new FFmpeg
  that is only an undecodable frame (logged "Error sending packet").
  If Arkham City's dialogue stalls again, compare against XmaContextNew's
  first-frame handling (handoff: "Arkham City root cause").
- EoT hypersense copies still missing on 9684 (vertex fast math off), so
  that was not the cause. Registers of the copies (9682 trace 23-18-17 frame
  1, tools/xtr_regs2.py): depth-only passes bind only the D24FS8 depth at
  0x460, colour passes bind c0 = 16_16_16_16 (fixed, -32...32) at 0 with the
  same depth; no alpha test (RB_COLORCONTROL 87000006), no depth export or
  kill in either pixel shader, same viewport (the Metal viewport doesn't
  depend on the bound target size; only the scissor is clamped). The
  colour pass blends src * src alpha + dst (01060106). With
  snorm16_render_target_full_range (default) the host holds 16_16_16_16 as
  RGBA16Snorm -1...1 and color_exp_bias divides the whole output, alpha
  included, by 32, so the source factor was a/32 and each copy added 1/32 of
  what it should. Spider-Man's glow draws (VS 919AF99E40E818A3 and others in
  the same pass, also 01060106 into c0) the same. The 9684 traces
  (08-07-30, 08-08-05) have no hypersense draws.
- Fix: where only the source factors read the source and the ops are
  add/subtract (GetFixed16SourceFactorPremult), the pixel shader
  pre-multiplies the output by the source factor (the MIN/MAX pre-multiply,
  rt0_blend_*_factor_for_premult), taken from the output times 32
  (kSysFlag_Color0Fixed16Scaled), and the pipeline's source factor is one
  (ApplyFixed16SourceFactorPremult in CreateMslPipelineState, the tess
  pipeline and the archive prewarm). Exact for src * f(src) + dst * g(dst or
  constant). Still wrong: a destination factor that reads the source (plain
  alpha blending 07060706 into these targets keeps a/32 there), and
  factors reading the destination (stored /32). XeniOS 2bc0867b6
  (metal_rt16_fetch_blend) blends these targets in the shader through
  framebuffer fetch with the console's encode, on its gamma fetch blending
  (b22a84286) and its spirv2air compiler; a candidate larger change (would
  need SPIRV-Cross framebuffer fetch here).
- GTA IV frame detail logs (08-08-29, 08-11-43, 08-19-54; tools
  fd_timeline.py and fd_transfers.py in the scratchpad): GPU 50-70 ms a heavy
  frame, of it render target transfers 20-26 ms (only 0.4 vertex + 5-6 ms
  fragment, the rest attachment traffic: 125-130 MB loaded and stored a
  frame) and draws 16-25 ms (430-450 MB loaded and stored). The scene is
  drawn in three predicated tiling bands of 1280x256 (the last 208) at 2x
  MSAA into c0, c1, c2 8888 and D24FS8 (host 1280x1024 x2), each band ending
  with resolves, one clearing colour and depth.
- Bug: the render area a pass descriptor needed was remembered per
  MTL::RenderPassDescriptor pointer. The render target cache makes a new
  descriptor whenever the targets change, and it often got the address of
  the one just freed, so the band passes took over the area of the depth
  clear pass between bands: 736 rows loaded and stored (416 for band 3) for
  256 drawn. Now keyed by the attachment textures
  (render_area_by_attachments_).
- Bug: the copy->draw split (a draw reading a texture a resolve in the open
  command buffer wrote, keep-CB off commits, keep-CB on loads the texture)
  ran after IssueDraw had begun the render pass, so each such draw left an
  empty pass behind, its whole area loaded and stored (one per band: four
  2x MSAA targets). Now checked before the pass begins when the shaders'
  textures are known (DrawReadsPendingResolveOutput /
  SplitForPendingResolveOutput); IssueDrawMsl still checks for shaders
  translated in the draw.
- Next for GTA IV (not done): between bands the game clears depth with one
  rectangle at 4x MSAA (pitch 640, depth ALWAYS with write, no pixel
  shader, mode 5): it is drawn into the samples-as-pixels 1x target, so each
  costs two 512-tile depth transfers (2x -> 1x and back) carrying nothing
  the clear doesn't overwrite. Resolve clears run as their own passes
  clearing the whole 1280x1024 target on load (PerformTransfersAndResolve
  Clears), then the next pass loads it again; deferring the clear to the next
  pass's load action would save a store and a load per clear. 20 of 32
  transfers a frame are depth <-> colour reinterpretations of the same EDRAM
  (12.6 of 28.4 M samples), 11 MSAA changes (13.3 M).
- release_core_in_spin_waits is read when a function is compiled, so
  changing it in game only affects code compiled after; all GTA IV runs
  here launched with it on. A fair A/B needs it set before launch.


### Per-game settings and copy/paste (owner's request, 2026-11-20)

- Long-press a game > "Game Settings" (above Compatibility): the settings
  screen for that title. Rows start from the global values with the game's
  config/<title id>.config.toml overrides on top (the file
  config::LoadGameConfigForFile already loads at launch); rows that differ
  say "Changed for this game"; Save writes only the rows that differ from
  global (removing the others' overrides, keeping overrides of settings not
  on screen), or deletes the file when none differ; "Use Global" deletes it.
  App-level (user default) rows and actions are not per game.
- Text settings' editor: Copy (left) and Paste (right) buttons in the
  field, since its edit menu doesn't show (cause not found in the app; no
  canPerformAction or first-responder overrides).

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
