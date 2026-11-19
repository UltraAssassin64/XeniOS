/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2026 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

// With vector context promotion (context_promote_vec128), a VMX register
// written with a constant and read again in the same block reaches the next
// op as an HIR constant: the op is folded, or its emitter loads the constant
// into a scratch register. Each case here requires that result to match the
// same op on register operands.

#include "xenia/cpu/testing/util.h"

using namespace xe;
using namespace xe::cpu;
using namespace xe::cpu::hir;
using namespace xe::cpu::testing;
using xe::cpu::ppc::PPCContext;

namespace {
using Ops = std::vector<Value*>;
}  // namespace

TEST_CASE("DOT_PRODUCT_3_CONSTANT_OPERANDS", "[vector]") {
  RequireConstantOperandsMatchRegisters(
      vec128f(1.5f, -2.0f, 3.25f, 4.0f), vec128f(0.5f, 6.0f, -1.0f, 2.5f),
      [](HIRBuilder& b, Value* x, Value* y) { return b.DotProduct3(x, y); });
}

TEST_CASE("DOT_PRODUCT_4_CONSTANT_OPERANDS", "[vector]") {
  RequireConstantOperandsMatchRegisters(
      vec128f(1.5f, -2.0f, 3.25f, 4.0f), vec128f(0.5f, 6.0f, -1.0f, 2.5f),
      [](HIRBuilder& b, Value* x, Value* y) { return b.DotProduct4(x, y); });
}

TEST_CASE("VECTOR_ROTATE_LEFT_CONSTANT_OPERANDS", "[vector]") {
  const vec128_t values =
      vec128i(0x80412203, 0x01F0E0D7, 0x7FFF8001, 0x12345678);
  const vec128_t amounts =
      vec128i(0x00010203, 0x04050607, 0x0809100F, 0x1F1E1D1C);
  for (TypeName type : {INT8_TYPE, INT16_TYPE, INT32_TYPE}) {
    RequireConstantOperandsMatchRegisters(
        values, amounts, [type](HIRBuilder& b, Value* x, Value* y) {
          return b.VectorRotateLeft(x, y, type);
        });
  }
}

TEST_CASE("VECTOR_DENORMFLUSH_CONSTANT_OPERAND", "[vector]") {
  RequireVectorFoldMatchesBackend(
      {vec128i(0x00000001, 0x80000010, 0x3F800000, 0x007FFFFF)},
      [](HIRBuilder& b, const Ops& ops) {
        return b.VectorDenormFlush(ops[0]);
      });
}

TEST_CASE("SHL_V128_CONSTANT_OPERAND", "[vector]") {
  const vec128_t value =
      vec128i(0x80000001, 0xF0F0F0F0, 0x12345678, 0xFEDCBA98);
  for (int8_t shift = 1; shift < 8; ++shift) {
    RequireVectorFoldMatchesBackend({value},
                                    [shift](HIRBuilder& b, const Ops& ops) {
                                      return b.Shl(ops[0], shift);
                                    });
  }
  // Shift count from a register.
  RequireVectorFoldMatchesBackend(
      {value},
      [](HIRBuilder& b, const Ops& ops) {
        return b.Shl(ops[0], b.Truncate(LoadGPR(b, 5), INT8_TYPE));
      },
      {3});
}

TEST_CASE("SHR_V128_CONSTANT_OPERAND", "[vector]") {
  const vec128_t value =
      vec128i(0x80000001, 0xF0F0F0F0, 0x12345678, 0xFEDCBA98);
  for (int8_t shift = 1; shift < 8; ++shift) {
    RequireVectorFoldMatchesBackend({value},
                                    [shift](HIRBuilder& b, const Ops& ops) {
                                      return b.Shr(ops[0], shift);
                                    });
  }
  RequireVectorFoldMatchesBackend(
      {value},
      [](HIRBuilder& b, const Ops& ops) {
        return b.Shr(ops[0], b.Truncate(LoadGPR(b, 5), INT8_TYPE));
      },
      {5});
}

TEST_CASE("EXTRACT_FOLD_MATCHES_BACKEND", "[vector]") {
  const vec128_t vec =
      vec128b(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  for (int i = 0; i < 16; ++i) {
    RequireScalarFoldMatchesBackend({vec}, [i](HIRBuilder& b, const Ops& ops) {
      return b.Extract(ops[0], b.LoadConstantUint8(uint8_t(i)), INT8_TYPE);
    });
  }
  for (int i = 0; i < 8; ++i) {
    RequireScalarFoldMatchesBackend({vec}, [i](HIRBuilder& b, const Ops& ops) {
      return b.Extract(ops[0], b.LoadConstantUint8(uint8_t(i)), INT16_TYPE);
    });
  }
  for (int i = 0; i < 4; ++i) {
    RequireScalarFoldMatchesBackend({vec}, [i](HIRBuilder& b, const Ops& ops) {
      return b.Extract(ops[0], b.LoadConstantUint8(uint8_t(i)), INT32_TYPE);
    });
  }
}

TEST_CASE("VECTOR_COMPARE_EQ_F32_FOLD_MATCHES_BACKEND", "[vector]") {
  auto build = [](HIRBuilder& b, const Ops& ops) {
    return b.VectorCompareEQ(ops[0], ops[1], FLOAT32_TYPE);
  };
  // +0 and -0 are equal; a NaN equals nothing, itself included.
  RequireVectorFoldMatchesBackend(
      {vec128f(1.0f, -1.0f, 0.0f, 2.0f), vec128f(1.0f, 1.0f, -0.0f, 1.0f)},
      build);
  RequireVectorFoldMatchesBackend(
      {vec128i(0x7FC00000, 0x3F800000, 0x7F800000, 0xFFC00000),
       vec128i(0x7FC00000, 0x3F800000, 0x7F800000, 0x00000000)},
      build);
}

TEST_CASE("SELECT_V128_CONSTANT_OPERANDS", "[vector]") {
  const vec128_t mask = vec128i(0xFFFFFFFF, 0x00000000, 0xFF00FF00, 0x0F0F0F0F);
  const vec128_t value1 =
      vec128i(0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD);
  const vec128_t value2 =
      vec128i(0x11111111, 0x22222222, 0x33333333, 0x44444444);
  // Each operand constant in turn, the others from v16 and v17.
  RequireVectorFoldMatchesBackend({mask},
                                  [](HIRBuilder& b, const Ops& ops) {
                                    return b.Select(ops[0], LoadVR(b, 16),
                                                    LoadVR(b, 17));
                                  },
                                  {}, {value1, value2});
  RequireVectorFoldMatchesBackend({value1},
                                  [](HIRBuilder& b, const Ops& ops) {
                                    return b.Select(LoadVR(b, 16), ops[0],
                                                    LoadVR(b, 17));
                                  },
                                  {}, {mask, value2});
  RequireVectorFoldMatchesBackend({value2},
                                  [](HIRBuilder& b, const Ops& ops) {
                                    return b.Select(LoadVR(b, 16),
                                                    LoadVR(b, 17), ops[0]);
                                  },
                                  {}, {mask, value1});
}
