#pragma once

#include <stdint.h>

typedef uint64_t daf_largest_uint;
typedef double daf_largest_float;

namespace NumberLiteralConstants {
  enum ConstantRealType {
    F32, F64
  };

	enum ConstantIntegerType:int {
    U8=8, I8=-8, U16=16, I16=-16, U32=32, I32=-32, U64=64, I64=-64
  };
}

