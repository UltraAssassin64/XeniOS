#!/usr/bin/env python3
"""Runs the CPU test binaries one test at a time and writes a report.

Each HIR test case (xenia-cpu-tests) and each PowerPC instruction test file
(xenia-cpu-ppc-tests) runs in its own process, so a crash in generated code is
recorded against that test instead of ending the whole run. The script always
exits 0: its purpose is the report, not a pass/fail gate.
"""

import argparse
import concurrent.futures
import json
import os
import re
import signal
import subprocess
import sys


def run(cmd, timeout):
    """Returns (status, output) where status is pass, fail, crash or timeout."""
    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, timeout=timeout)
    except subprocess.TimeoutExpired as e:
        return "timeout", (e.stdout or b"").decode("utf-8", "replace")
    output = proc.stdout.decode("utf-8", "replace")
    if proc.returncode == 0:
        return "pass", output
    if proc.returncode < 0:
        try:
            name = signal.Signals(-proc.returncode).name
        except ValueError:
            name = str(-proc.returncode)
        return "crash", f"{output}\n[terminated by {name}]"
    return "fail", output


def tail(text, lines):
    kept = [l for l in text.splitlines() if l.strip()]
    return "\n".join(kept[-lines:])


def escape_catch_name(name):
    # Catch reads ',' as a separator and '[' as the start of a tag.
    return re.sub(r'([\\,\[\]"])', r"\\\1", name)


def run_hir_tests(binary, timeout):
    listing = subprocess.run([binary, "--list-test-names-only"],
                             stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    names = [l.strip() for l in listing.stdout.decode("utf-8", "replace")
             .splitlines() if l.strip()]
    results = []
    for name in names:
        status, output = run([binary, escape_catch_name(name)], timeout)
        results.append({"name": name, "status": status,
                        "output": "" if status == "pass" else tail(output, 25)})
        print(f"[hir] {status:7} {name}", flush=True)
    return results


def run_ppc_suite(binary, suite, timeout):
    status, output = run([binary, suite], timeout)
    passed = re.search(r"^Passed: (\d+)", output, re.M)
    failed = re.search(r"^Failed: (\d+)", output, re.M)
    failed_cases = re.findall(r"^\s*\[(\S+)\] FAILED", output, re.M)
    details = [l for l in output.splitlines()
               if "assert failed" in l or "Expected:" in l or "Actual:" in l]
    return {
        "name": suite,
        "status": status,
        "passed": int(passed.group(1)) if passed else 0,
        "failed": int(failed.group(1)) if failed else 0,
        "failed_cases": failed_cases,
        "details": details[:30],
        "output": tail(output, 12) if status in ("crash", "timeout") else "",
    }


def run_ppc_tests(binary, test_dir, timeout, jobs):
    suites = sorted(f[:-2] for f in os.listdir(test_dir)
                    if f.startswith("instr_") and f.endswith(".s"))
    results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        for result in pool.map(
                lambda s: run_ppc_suite(binary, s, timeout), suites):
            results.append(result)
            print(f"[ppc] {result['status']:7} {result['name']} "
                  f"passed={result['passed']} failed={result['failed']}",
                  flush=True)
    return results


def write_report(path, hir, ppc):
    lines = ["# CPU test report", ""]
    if hir is not None:
        counts = {}
        for r in hir:
            counts[r["status"]] = counts.get(r["status"], 0) + 1
        lines += ["## HIR tests (xenia-cpu-tests)", "",
                  f"{len(hir)} test cases: " + ", ".join(
                      f"{v} {k}" for k, v in sorted(counts.items())), ""]
        bad = [r for r in hir if r["status"] != "pass"]
        if bad:
            lines += ["| Test | Result |", "|---|---|"]
            lines += [f"| `{r['name']}` | {r['status']} |" for r in bad]
            lines.append("")
            for r in bad:
                lines += [f"### {r['name']} ({r['status']})", "", "```",
                          r["output"], "```", ""]
    if ppc is not None:
        files_bad = [r for r in ppc
                     if r["status"] != "pass" or r["failed"]]
        lines += ["## PowerPC instruction tests (xenia-cpu-ppc-tests)", "",
                  f"{len(ppc)} test files, "
                  f"{sum(r['passed'] for r in ppc)} cases passed, "
                  f"{sum(r['failed'] for r in ppc)} cases failed, "
                  f"{sum(1 for r in ppc if r['status'] == 'crash')} files "
                  f"crashed, "
                  f"{sum(1 for r in ppc if r['status'] == 'timeout')} files "
                  f"timed out.", ""]
        if files_bad:
            lines += ["| Test file | Result | Passed | Failed |",
                      "|---|---|---|---|"]
            lines += [f"| `{r['name']}` | {r['status']} | {r['passed']} | "
                      f"{r['failed']} |" for r in files_bad]
            lines.append("")
            for r in files_bad:
                lines += [f"### {r['name']} ({r['status']})", ""]
                if r["failed_cases"]:
                    lines += ["Failed cases: " + ", ".join(
                        f"`{c}`" for c in r["failed_cases"][:40]), ""]
                body = r["details"] or r["output"].splitlines()
                if body:
                    lines += ["```"] + body + ["```", ""]
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    return lines


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--hir-binary")
    parser.add_argument("--ppc-binary")
    parser.add_argument("--ppc-test-dir",
                        default=os.path.join("src", "xenia", "cpu", "ppc",
                                             "testing"))
    parser.add_argument("--out-dir", default="cpu-test-results")
    parser.add_argument("--timeout", type=int, default=180)
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 2)
    args = parser.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)
    hir = ppc = None
    if args.hir_binary and os.path.exists(args.hir_binary):
        hir = run_hir_tests(args.hir_binary, args.timeout)
    if args.ppc_binary and os.path.exists(args.ppc_binary):
        ppc = run_ppc_tests(args.ppc_binary, args.ppc_test_dir, args.timeout,
                            args.jobs)

    with open(os.path.join(args.out_dir, "results.json"), "w") as f:
        json.dump({"hir": hir, "ppc": ppc}, f, indent=1)
    lines = write_report(os.path.join(args.out_dir, "report.md"), hir, ppc)

    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary_path:
        # The step summary has a size limit; keep the tables, drop the details.
        short = [l for l in lines[:400]]
        with open(summary_path, "a", encoding="utf-8") as f:
            f.write("\n".join(short) + "\n")
    if hir is None and ppc is None:
        print("No test binary was found.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
