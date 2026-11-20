/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/kernel_flags.h"

DEFINE_bool(headless, false,
            "Don't display any UI, using defaults for prompts as needed.",
            "UI");
DEFINE_bool(log_high_frequency_kernel_calls, false,
            "Log kernel calls with the kHighFrequency tag.", "Logging");
DEFINE_uint32(
    kernel_call_repeat_threshold, 20000,
    "Log a warning, with the guest caller and stack, when a guest thread "
    "makes the same kernel call, or repeats the same cycle of up to four "
    "kernel calls, this many times in a row. 0 disables it.",
    "Logging");
DEFINE_bool(
    log_guest_thread_hotspots, true,
    "Diagnostics: with the Metal perf lines (every 120 frames), sample every "
    "running guest thread 16 times, 1 ms apart, and log where each one that "
    "isn't only waiting spends its time (guest functions or host functions). "
    "Apple ARM64 only.",
    "Logging");
DEFINE_string(
    debug_watch_guest_addresses, "",
    "Diagnostics: comma-separated guest addresses (hex) of 32-bit values to "
    "sample every 0.1 ms while a game runs; every 2 seconds the log shows "
    "each one's minimum, maximum, last value and how often it was 0. For a "
    "game stuck waiting on a counter or flag. Empty: off.",
    "Logging");
DEFINE_string(
    debug_dump_guest_code, "",
    "Diagnostics: comma-separated guest address ranges (hex, start-end, at "
    "most 64 KB each) whose code is written to the log once, 20 seconds "
    "after launch (\"Guest code dump at\" lines), to disassemble a stuck "
    "game offline. Empty: off.",
    "Logging");
DEFINE_bool(
    guest_scheduler, false,
    "Run guest threads as cooperative fibers driven by an in-kernel scheduler "
    "instead of mapping each to its own host OS thread. Experimental; off by "
    "default. Requires a restart to take effect.",
    "Kernel");
DEFINE_uint32(
    guest_scheduler_cpus, 6,
    "Number of host dispatch threads the cooperative scheduler spreads the "
    "360's 6 logical CPUs across (clamped to 1-6). 6 is one per guest CPU "
    "(full "
    "parallelism), 3 is one per physical core (SMT pairs share a thread), 1 is "
    "all guest threads cooperative on a single thread. Requires a restart.",
    "Kernel");
