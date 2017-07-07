#pragma once

#include <stdint.h>

typedef uint64_t daf_largest_uint;
typedef double daf_largest_float;

//TODO: Magic
#define USIZE_BIT_COUNT 64
#define ISIZE_BIT_COUNT 64
#define CHAR_BIT_COUNT 8

enum class LiteralKind:int {
	U8=1, I8=-1, U16=2, I16=-2, U32=4, I32=-4, U64=8, I64=-8, BOOL=100, USIZE, ISIZE, CHAR, F32=200, F64
};

LiteralKind getIntegerLiteralKind(bool isSigned, int bitSize);
