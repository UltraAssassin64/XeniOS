#!/usr/bin/env python3
"""
XeniOS iOS JIT Broker for LLDB.

Handles the A64CodeCache BRK protocol used by XeniOS when running under
Xcode or TrollStore debugserver.

Commands supported:

  brk #0xf00d
      x16=1  -> prepare JIT region (x0=addr, x1=len)
      x16=0  -> detach broker

  brk #0x69
      legacy prepare region

This script installs a stop-hook that intercepts those breakpoints and
requests RX memory from debugserver so the emulator can generate code.
"""

import lldb

SIGTRAP = 5

BRK_OPCODE = 0xD4200000
BRK_MASK = 0xFFE0001F

UNIVERSAL_BRK = 0xF00D
LEGACY_BRK = 0x69

CMD_DETACH = 0
CMD_PREPARE = 1

PAGE_STRIDE = 0x4000

_hook_installed = False
_hook_id = None


# ---------------------------------------------------------
# Utilities
# ---------------------------------------------------------

def run(debugger, cmd):
    res = lldb.SBCommandReturnObject()
    debugger.GetCommandInterpreter().HandleCommand(cmd, res)
    return res


def read_reg(frame, name):
    reg = frame.FindRegister(name)
    if not reg.IsValid():
        return None
    return reg.GetValueAsUnsigned()


def write_reg(frame, name, val):
    reg = frame.FindRegister(name)
    if not reg.IsValid():
        return
    reg.SetValueFromCString(f"0x{val:x}")


def read_u32(process, addr):
    err = lldb.SBError()
    data = process.ReadMemory(addr, 4, err)
    if not err.Success():
        return None
    return int.from_bytes(data, "little")


# ---------------------------------------------------------
# Memory preparation
# ---------------------------------------------------------

def send_packet(debugger, packet):
    res = run(debugger, f"process plugin packet send {packet}")
    out = (res.GetOutput() or "") + (res.GetError() or "")
    if "response:" not in out:
        return None
    return out.split("response:")[1].strip()


def mark_pages(debugger, addr, length):
    pages = (length + PAGE_STRIDE - 1) // PAGE_STRIDE

    cur = addr
    for _ in range(pages):
        resp = send_packet(debugger, f"M{cur:x},1:69")
        if resp != "OK":
            return False
        cur += PAGE_STRIDE

    return True


def prepare_region(debugger, addr, length):

    if addr == 0:
        resp = send_packet(debugger, f"_M{length:x},rx")
        if not resp:
            return False, 0
        addr = int(resp, 16)

    if not mark_pages(debugger, addr, length):
        return False, addr

    return True, addr


# ---------------------------------------------------------
# Stop hook
# ---------------------------------------------------------

class XeniosJITHook:

    def __init__(self, target, args, internal_dict):
        self.debugger = target.GetDebugger()
        self.detached = False

    def handle_stop(self, exe_ctx, stream):

        if self.detached:
            return True

        thread = exe_ctx.GetThread()
        frame = exe_ctx.GetFrame()
        process = exe_ctx.GetProcess()

        if not frame.IsValid():
            return True

        pc = read_reg(frame, "pc")
        instr = read_u32(process, pc)

        if instr is None:
            return True

        if (instr & BRK_MASK) != BRK_OPCODE:
            return True

        imm = (instr >> 5) & 0xFFFF

        x16 = read_reg(frame, "x16")
        x0 = read_reg(frame, "x0")
        x1 = read_reg(frame, "x1")

        if imm == UNIVERSAL_BRK:

            if x16 == CMD_DETACH:
                self.detached = True
                print("xenios-jit: broker detached")
                return False

            if x16 == CMD_PREPARE:

                ok, addr = prepare_region(self.debugger, x0, x1)

                if ok:
                    write_reg(frame, "x0", addr)
                    print(f"xenios-jit: prepared {hex(addr)} size={hex(x1)}")
                else:
                    print("xenios-jit: prepare failed")

        elif imm == LEGACY_BRK:

            ok, addr = prepare_region(self.debugger, x0, x1)

            if ok:
                write_reg(frame, "x0", addr)
                print(f"xenios-jit: legacy prepare {hex(addr)}")

        else:
            return True

        write_reg(frame, "pc", pc + 4)
        return False


# ---------------------------------------------------------
# Install broker
# ---------------------------------------------------------

def install(debugger, command, exe_ctx, result, internal_dict):
    global _hook_installed

    if _hook_installed:
        result.AppendMessage("xenios-jit broker already installed")
        return

    run(debugger, "process handle -p false -s true -n true SIGTRAP")
    run(debugger, "process handle -p true -s false -n false SIGUSR2")

    hook = run(
        debugger,
        "target stop-hook add -P xenios_ios_lldb_jit_broker.XeniosJITHook -I false",
    )

    _hook_installed = True

    result.AppendMessage("xenios-jit broker installed")


# ---------------------------------------------------------
# LLDB entrypoint
# ---------------------------------------------------------

def __lldb_init_module(debugger, internal_dict):

    debugger.HandleCommand(
        "command script add -f xenios_ios_lldb_jit_broker.install xenios-jit-install"
    )

    print("XeniOS JIT broker loaded")
    print("Run: xenios-jit-install")