#pragma once

#include <stdint.h>

typedef int daf_int;
typedef long daf_long;
typedef unsigned int daf_uint;
typedef unsigned long daf_ulong;
typedef char daf_char;
typedef float daf_float;
typedef double daf_double;

#if CHAR_BIT <= 8
typedef uint8_t daf_uint8;
typedef int8_t daf_int8;
#define DAF_HAS_INT8
#endif

typedef uint16_t daf_uint16;
typedef int16_t daf_int16;
typedef uint32_t daf_uint32;
typedef int32_t daf_int32;
typedef uint64_t daf_uint64;
typedef int64_t daf_int64;
