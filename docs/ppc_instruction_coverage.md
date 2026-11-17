# PowerPC instruction coverage on this branch

A static audit of what the CPU emulation on `Only-the-Trollstore-jit-imp` can
and cannot translate. It was made by reading the sources; nothing here was
measured by running code. The CPU test job (`.github/workflows/cpu-tests.yml`)
is what checks behaviour.

## Which versions are in the tree

| Part | Directory | Matches upstream release |
|---|---|---|
| ARM64 JIT backend | `src/xenia/cpu/backend/a64` | `release-v2.0.1-build-9730` (only include paths differ) |
| Front end (PowerPC to HIR) | `src/xenia/cpu/ppc` | `release-v1.0.1-build-9288` |
| HIR and compiler passes | `src/xenia/cpu/hir`, `src/xenia/cpu/compiler` | `release-v1.0.1-build-9288` |

So the backend is one major release newer than the front end that feeds it.
The differences between the two releases in the front end are small (17
files, about 660 lines, most of it a regenerated disassembler table):

| Difference | In this branch | In 9730 | Matters? |
|---|---|---|---|
| Context promotion of vector registers | was enabled | disabled (worked around instability in games) | **Yes.** Aligned to 9730 in commit "[CPU] Stop promoting vector registers across instructions". |
| `AND` of a `NOT` folded into `AND_NOT` | not done | done | Speed only. |
| `vupkhpx`, `vupklpx` | implemented | stubs | This branch is ahead. |
| `OPCODE_ATOMIC_EXCHANGE` | defined, no ARM64 sequence | not defined | No. Nothing in the front end emits it. |
| Constant folding of 16-bit `Permute` | selects like the x64 code | selects the other source | Only for a permute of two constants. Left as is; the test job covers `Permute`. |

## The decode table

`src/xenia/cpu/ppc/ppc_opcode.h` lists 455 instructions. Compared with
reference lists of the Xbox 360 CPU's instruction set (base 64-bit PowerPC,
AltiVec, and the VMX128 extension; the VMX128 list was checked against
`third_party/binutils/binutils-2.24-vmx128.patch`, the other two were written
from memory of the architecture manuals):

| Group | Reference count | Not in the decode table |
|---|---|---|
| Base PowerPC | 226 | `rfid`, `slbia`, `slbie`, `slbmfee`, `slbmfev`, `slbmte`, `tlbie`, `tlbiel`, `tlbia`, `tlbsync`, `eciwx`, `ecowx` |
| AltiVec | 170 | `dss`, `dssall`, `dst`, `dstst`, `dststt`, `dstt` |
| VMX128 | 77 | `vupkhsh128`, `vupklsh128` |

- The missing base instructions are supervisor-only (interrupt return, page
  table and segment management, external control). Games do not execute them.
- `dst*` / `dss*` are data-stream prefetch hints. They have no visible effect
  and could be decoded as no-ops. A game that used one would log
  `Invalid instruction` and the function would fail to translate.
- `vupkhsh128` / `vupklsh128` are real instructions that upstream added to
  its table later (present in `release-v2.3.3-build-11940`). **These are the
  only user-mode instructions known to be missing from the table.**
- `mfocrf` / `mtocrf` share encodings with `mfcr` / `mtcrf` and are handled
  inside those two emitters.

An instruction missing from the table is reported at run time as
`Invalid instruction <address> <code>`.

## Instructions in the table that are not translated

A stub logs `Unimplemented instruction: InstrEmit_<name>` and the function
containing it fails to translate.

### Stubs (28)

| Group | Instructions | Upstream 11940 |
|---|---|---|
| String load/store | `lswi`, `lswx`, `stswi`, `stswx` | still stubs |
| FPSCR bit set/clear | `mtfsb0x`, `mtfsb1x` | implemented |
| AltiVec multiply-add (integer) | `vmhaddshs`, `vmhraddshs`, `vmladduhm` | still stubs |
| AltiVec multiply-sum | `vmsummbm`, `vmsumshm`, `vmsumshs`, `vmsumubm`, `vmsumuhm`, `vmsumuhs` | still stubs |
| AltiVec even/odd multiply | `vmulesb`, `vmulesh`, `vmuleub`, `vmuleuh`, `vmulosb`, `vmulosh`, `vmuloub`, `vmulouh` | still stubs |
| AltiVec sum across | `vsum2sws`, `vsum4sbs`, `vsum4shs`, `vsum4ubs`, `vsumsws` | still stubs |

Also `dcbi` is in the table but has no emitter registered (it is a
supervisor cache instruction).

All of the AltiVec stubs are integer instructions, and upstream has not
written them either.

### Partially translated (22)

| Instructions | What is missing | Upstream 11940 |
|---|---|---|
| `addx`, `addcx`, `addmex`, `addzex`, `subfx`, `subfcx`, `subfex`, `subfmex`, `subfzex` | the overflow-recording forms (`OE=1`, mnemonics ending in `o`) | implemented |
| `mulldx`, `mullwx`, `divdx`, `divdux`, `divwx`, `divwux` | the same `OE=1` forms | implemented |
| `mulhdx`, `mulhdux`, `mulhwx`, `mulhwux` | a reserved `OE=1` encoding | unchanged |
| `mfspr` | any register other than XER, LR, CTR, VRSAVE, TB (268/269) and PVR | unchanged |
| `mtspr` | any register other than XER, LR, CTR, VRSAVE | unchanged |
| `sc` | levels other than 0 and 2 | unchanged |

`addex` already handles `OE=1`.

The overflow-recording forms are rare in compiled game code.

## The ARM64 backend

The HIR has 121 opcodes. The ARM64 backend has a sequence for 120 of them;
the one without is `OPCODE_ATOMIC_EXCHANGE`, which nothing emits.

Type variants the x64 backend accepts and the ARM64 backend does not:

| Opcode | Missing variants | Emitted by the front end? |
|---|---|---|
| `MUL`, `DIV` | 8-bit and 16-bit | no |
| `MUL_HI` | 8-, 16- and 32-bit | no (only used with 64-bit values) |
| `SELECT` | vector result chosen by an 8-bit condition | no (vector selects use a vector mask) |
| `TRAP_TRUE`, `RETURN_TRUE` | float conditions | no |

A missing sequence is reported as `A64: Unable to process HIR opcode <name>`.

`DID_SATURATE` always returns 0 on both backends, so the AltiVec saturation
flag (VSCR.SAT) is never set.

## What the device logs say

None of the logs from Batman: Arkham Asylum (intro, title screen, new game)
contain `Invalid instruction`, `Unimplemented instruction` or
`Unable to process HIR opcode`. The title-screen fault (a NaN camera
position) is therefore an instruction that is translated but computes the
wrong result, not a missing one.

## Test coverage

`src/xenia/cpu/ppc/testing` holds 571 assembly test files. They are not
assembled in the repository; the test job assembles them with the patched
binutils in `third_party/binutils`.

70 table entries have no test file of their own:

`bcctrx bclrx bcx bx dcbi ldarx ldbrx lfdu lfdux lfsu lfsux lswi lswx lvebx
lvehx lvewx lvewx128 lvlxl lvrxl lwarx lwaux mfcr mtfsb0x mtfsb1x mtvscr sc
stbu stbux stdbrx stdcx stdu stdux stfdu stfdux stfsu stfsux sthu sthux stswi
stswx stvlxl stvrxl stwu stwux vcmpbfp128 vcmpeqfp128 vmhaddshs vmhraddshs
vmladduhm vmsummbm vmsumshm vmsumshs vmsumubm vmsumuhm vmsumuhs vmulesb
vmulesh vmuleub vmuleuh vmulosb vmulosh vmuloub vmulouh vsum2sws vsum4sbs
vsum4shs vsum4ubs vsumsws vupkhpx vupklpx`

Some of these are covered by combined files (`vcmpxxfp128`, `lvexx`,
`lxarx`, `lxbrx`) and the branch instructions are exercised by every test.
`skip.txt` lists 28,713 generated cases that are skipped, almost all of them
floating-point rounding and flag corner cases.
