#pragma once

#include <stdint.h>

typedef uint64_t daf_largest_uint;
typedef double daf_largest_float;

namespace NumberLiteralConstants {
  enum ConstantRealType {
    F32, F64
  };

  enum ConstantIntegerType {
    U8, I8, U16, I16, U32, I32, U64, I64
  };
}

