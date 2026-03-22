/**
 ******************************************************************************
 * Optimized A64 sequences for Apple Silicon
 ******************************************************************************
 */

#include "xenia/cpu/backend/a64/a64_sequences.h"
#include "xenia/cpu/backend/a64/a64_emitter.h"

namespace xe {
namespace cpu {
namespace backend {
namespace a64 {

void EmitAdd(A64Emitter& e, const Reg& dst, const Reg& src1, const Reg& src2) {
  if (dst.id == src1.id) {
    e.ADD(dst, dst, src2);
  } else if (dst.id == src2.id) {
    e.ADD(dst, dst, src1);
  } else {
    e.ADD(dst, src1, src2);
  }
}

void EmitSub(A64Emitter& e, const Reg& dst, const Reg& src1, const Reg& src2) {
  if (dst.id == src1.id) {
    e.SUB(dst, dst, src2);
  } else {
    e.SUB(dst, src1, src2);
  }
}

void EmitMul(A64Emitter& e, const Reg& dst, const Reg& src1, const Reg& src2) {
  e.MUL(dst, src1, src2);
}

void EmitAnd(A64Emitter& e, const Reg& dst, const Reg& src1, const Reg& src2) {
  e.AND(dst, src1, src2);
}

void EmitOr(A64Emitter& e, const Reg& dst, const Reg& src1, const Reg& src2) {
  e.ORR(dst, src1, src2);
}

void EmitXor(A64Emitter& e, const Reg& dst, const Reg& src1, const Reg& src2) {
  e.EOR(dst, src1, src2);
}

void EmitShiftLeft(A64Emitter& e, const Reg& dst, const Reg& src, uint32_t shift) {
  e.LSL(dst, src, shift);
}

void EmitShiftRight(A64Emitter& e, const Reg& dst, const Reg& src, uint32_t shift) {
  e.LSR(dst, src, shift);
}

void EmitArithmeticShiftRight(A64Emitter& e, const Reg& dst, const Reg& src,
                              uint32_t shift) {
  e.ASR(dst, src, shift);
}

void EmitLoad(A64Emitter& e, const Reg& dst, const Reg& base, int32_t offset) {
  e.LDR(dst, MemOperand(base, offset));
}

void EmitStore(A64Emitter& e, const Reg& src, const Reg& base, int32_t offset) {
  e.STR(src, MemOperand(base, offset));
}

void EmitBranch(A64Emitter& e, void* target) {
  e.B(target);
}

void EmitCompare(A64Emitter& e, const Reg& a, const Reg& b) {
  e.CMP(a, b);
}

void EmitMove(A64Emitter& e, const Reg& dst, const Reg& src) {
  if (dst.id != src.id) {
    e.MOV(dst, src);
  }
}

}  // namespace a64
}  // namespace backend
}  // namespace cpu
}  // namespace xe