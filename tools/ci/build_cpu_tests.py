#!/usr/bin/env python3
"""Builds the CPU test programs from the CPU and base sources alone.

The regular desktop build links the test programs against the whole emulator
(kernel, GPU, audio, Qt UI). This script compiles only xenia-base, xenia-cpu,
the JIT backend for the host architecture, the guest memory code and the tests
themselves, so the tests can be built where the desktop build cannot.

Outputs build/cpu-tests/xenia-cpu-tests and build/cpu-tests/xenia-cpu-ppc-tests.
"""

import argparse
import concurrent.futures
import glob
import os
import platform
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))


def host():
    system = "mac" if sys.platform == "darwin" else "linux"
    machine = platform.machine().lower()
    arch = "a64" if machine in ("arm64", "aarch64") else "x64"
    return system, arch


def wanted(path, system, arch):
    """Whether a source file belongs in a build for this host."""
    name = os.path.basename(path)
    stem = os.path.splitext(name)[0]
    for suffix in ("_test", "_demo"):
        if stem.endswith(suffix):
            return False
    if stem.startswith("main_") or stem.startswith("console_app_main_"):
        return False
    other_platforms = ["_win", "_android", "_ios", "_x11", "_gtk"]
    if system == "mac":
        other_platforms += ["_linux", "_gnulinux"]
    else:
        other_platforms += ["_mac"]
    for suffix in other_platforms:
        if stem.endswith(suffix):
            return False
    if stem.endswith("_arm64") and arch != "a64":
        return False
    if stem.endswith(("_amd64", "_x64")) and arch == "a64":
        return False
    return True


def collect_sources(system, arch):
    def rel(pattern, recursive=False):
        return sorted(os.path.relpath(p, ROOT) for p in glob.glob(
            os.path.join(ROOT, pattern), recursive=recursive))

    common = []
    common += [p for p in rel("src/xenia/base/*.cc")
               if wanted(p, system, arch)]
    if system == "mac":
        common += rel("src/xenia/base/*_mac.mm")
    cpu = rel("src/xenia/cpu/**/*.cc", recursive=True)
    backend_dir = "backend/a64/" if arch == "a64" else "backend/x64/"
    for p in cpu:
        unix = p.replace(os.sep, "/")
        if "/testing/" in unix:
            continue
        if "/backend/a64/" in unix or "/backend/x64/" in unix:
            if backend_dir not in unix:
                continue
        if not wanted(p, system, arch):
            continue
        # XexModule loads real executables and needs the kernel; the tests
        # only use TestModule and raw binaries.
        if unix.endswith("/xex_module.cc"):
            continue
        common.append(p)
    # Message boxes go through SDL; cpu_tests_support.cc replaces them.
    common = [p for p in common if not p.endswith(
        ("system_gnulinux.cc", "system_mac.cc"))]
    common.append("src/xenia/memory.cc")
    common += ["third_party/fmt/src/format.cc", "third_party/fmt/src/os.cc"]
    common += ["third_party/mspack/logging.cc", "third_party/mspack/lzxd.c",
               "third_party/mspack/system.c"]
    common += rel("third_party/capstone/*.c")
    if arch == "a64":
        common += rel("third_party/capstone/arch/AArch64/*.c")
        common += rel("third_party/xbyak_aarch64/src/*.cpp")
    else:
        common += rel("third_party/capstone/arch/X86/*.c")
    boost = "third_party/boost_context/context/src/asm/%%s_%s_gas.S" % (
        {("mac", "a64"): "arm64_aapcs_macho",
         ("mac", "x64"): "x86_64_sysv_macho",
         ("linux", "a64"): "arm64_aapcs_elf",
         ("linux", "x64"): "x86_64_sysv_elf"}[(system, arch)])
    common += [boost % part for part in ("make", "jump", "ontop")]
    common.append("tools/ci/cpu_tests_support.cc")

    console_main = "src/xenia/base/console_app_main_%s.cc" % (
        "mac" if system == "mac" and os.path.exists(os.path.join(
            ROOT, "src/xenia/base/console_app_main_mac.cc")) else "posix")
    hir = rel("src/xenia/cpu/testing/*_test.cc") + [
        "tools/build/src/test_suite_main.cc", console_main]
    ppc = ["src/xenia/cpu/ppc/testing/ppc_testing_main.cc", console_main]
    return common, hir, ppc


def flags(system, arch):
    defines = [
        "NDEBUG", "_LIB", "USE_CPP17", "HAVE_CONFIG_H",
        "CAPSTONE_USE_SYS_DYN_MEM", "CAPSTONE_DIET_NO",
        "XBYAK_NO_OP_NAMES", "XBYAK_ENABLE_OMITTED_OPERAND",
        'XE_TEST_SUITE_NAME="xenia-cpu-tests"',
    ]
    if arch == "a64":
        defines += ["CAPSTONE_HAS_AARCH64", "CAPSTONE_HAS_ARM64"]
    else:
        defines += ["CAPSTONE_HAS_X86", "CAPSTONE_X86_ATT_DISABLE",
                    "CAPSTONE_X86_REDUCE_NO"]
    includes = [
        ".", "src", "third_party", "build", "tools/build",
        "tools/build/src", "third_party/llvm/include",
        "third_party/capstone", "third_party/capstone/include",
        "third_party/mspack", "third_party/fmt/include",
        "third_party/xbyak", "third_party/xbyak_aarch64",
        "third_party/xbyak_aarch64/xbyak_aarch64",
        "third_party/oaknut/include",
    ]
    common = ["-O2", "-g1", "-w", "-fno-strict-aliasing"]
    if arch == "x64":
        common += ["-mavx2", "-mfma", "-mlzcnt", "-mbmi", "-mbmi2", "-mf16c",
                   "-mmovbe"]
    if system == "mac":
        common += ["-mmacosx-version-min=15.0"]
    common += ["-D" + d for d in defines]
    common += ["-I" + os.path.join(ROOT, i) for i in includes]
    return common


def ensure_version_header():
    """Writes build/version.h, which the regular build generates."""
    path = os.path.join(ROOT, "build", "version.h")
    if os.path.exists(path):
        return

    def git(*cmd):
        try:
            return subprocess.check_output(
                ("git", "-C", ROOT) + cmd,
                stderr=subprocess.DEVNULL).decode().strip()
        except (OSError, subprocess.CalledProcessError):
            return "unknown"

    commit = git("rev-parse", "HEAD")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write("#ifndef GENERATED_VERSION_H_\n"
                "#define GENERATED_VERSION_H_\n"
                '#define XE_BUILD_BRANCH "%s"\n'
                '#define XE_BUILD_COMMIT "%s"\n'
                '#define XE_BUILD_COMMIT_SHORT "%s"\n'
                '#define XE_BUILD_VERSION "0.0.0"\n'
                '#define XE_BUILD_NUMBER "0"\n'
                "#define XE_BUILD_DATE __DATE__\n"
                "#endif  // GENERATED_VERSION_H_\n" % (
                    git("rev-parse", "--abbrev-ref", "HEAD"), commit,
                    commit[:8]))


def compile_one(job):
    source, obj, cmd = job
    if (os.path.exists(obj) and
            os.path.getmtime(obj) >= os.path.getmtime(source)):
        return source, 0, ""
    os.makedirs(os.path.dirname(obj), exist_ok=True)
    proc = subprocess.run(cmd, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT)
    return source, proc.returncode, proc.stdout.decode("utf-8", "replace")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-dir",
                        default=os.path.join(ROOT, "build", "cpu-tests"))
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 2)
    parser.add_argument("--cxx", default=os.environ.get("CXX", "clang++"))
    parser.add_argument("--cc", default=os.environ.get("CC", "clang"))
    args = parser.parse_args()

    system, arch = host()
    ensure_version_header()
    common, hir, ppc = collect_sources(system, arch)
    base_flags = flags(system, arch)
    obj_dir = os.path.join(args.out_dir, "obj")

    jobs = []
    objects = {}
    for source in sorted(set(common + hir + ppc)):
        obj = os.path.join(obj_dir, source + ".o")
        objects[source] = obj
        path = os.path.join(ROOT, source)
        if source.endswith(".S"):
            cmd = [args.cc] + base_flags
        elif source.endswith(".c"):
            cmd = [args.cc, "-std=c17"] + base_flags
        elif source.endswith(".mm"):
            cmd = [args.cxx, "-std=c++20", "-fobjc-arc"] + base_flags
        else:
            cmd = [args.cxx, "-std=c++20"] + base_flags
        if source.replace(os.sep, "/").endswith("cpu/backend/backend.cc"):
            # memory.cc and backend.cc both define these two cvars on this
            # branch. Keep memory.cc's and rename this file's copies.
            cmd += ["-Drecord_mmio_access_exceptions="
                    "cpu_tests_duplicate_record_mmio_access_exceptions",
                    "-Demit_mmio_aware_stores_for_recorded_exception_addresses="
                    "cpu_tests_duplicate_emit_mmio_aware_stores"]
        jobs.append((path, obj, cmd + ["-c", path, "-o", obj]))

    print("Compiling %d files for %s/%s with %d jobs" % (
        len(jobs), system, arch, args.jobs), flush=True)
    failed = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for source, code, output in pool.map(compile_one, jobs):
            if code:
                failed.append(source)
                print("FAILED %s\n%s" % (os.path.relpath(source, ROOT),
                                         output[-4000:]), flush=True)
    if failed:
        print("%d files failed to compile" % len(failed))
        return 1

    link_flags = ["-lpthread", "-ldl"]
    if system == "mac":
        link_flags = ["-mmacosx-version-min=15.0", "-framework", "Foundation",
                      "-framework", "CoreFoundation"]
    else:
        link_flags += ["-lrt"]
    # The shared code goes into an archive so the linker takes only the
    # objects each program refers to, as it does in the regular build.
    archive = os.path.join(args.out_dir, "libxenia-cpu-tests-common.a")
    if os.path.exists(archive):
        os.remove(archive)
    subprocess.check_call(["ar", "rcs", archive] +
                          [objects[s] for s in sorted(set(common))])
    result = 0
    for name, extra in (("xenia-cpu-tests", hir),
                        ("xenia-cpu-ppc-tests", ppc)):
        out = os.path.join(args.out_dir, name)
        cmd = [args.cxx, "-o", out]
        cmd += [objects[s] for s in sorted(set(extra))]
        cmd += [archive] + link_flags
        proc = subprocess.run(cmd, stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT)
        text = proc.stdout.decode("utf-8", "replace")
        if proc.returncode:
            print("LINK FAILED %s\n%s" % (name, text[-12000:]), flush=True)
            result = 1
        else:
            print("Linked %s" % out, flush=True)
    return result


if __name__ == "__main__":
    sys.exit(main())
