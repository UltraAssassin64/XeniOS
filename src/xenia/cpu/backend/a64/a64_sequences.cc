/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project
 ******************************************************************************
 */

#include "xenia/cpu/backend/a64/a64_sequences.h"
#include "xenia/cpu/backend/a64/a64_emitter.h"

#include <oaknut/oaknut.hpp>

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

using oaknut::XReg;

void EmitAdd(A64Emitter& e, const XReg& dst, const XReg& src1, const XReg& src2) {
  e.ADD(dst, src1, src2);
}

void EmitSub(A64Emitter& e, const XReg& dst, const XReg& src1, const XReg& src2) {
  e.SUB(dst, src1, src2);
}

void EmitMul(A64Emitter& e, const XReg& dst, const XReg& src1, const XReg& src2) {
  e.MUL(dst, src1, src2);
}

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe