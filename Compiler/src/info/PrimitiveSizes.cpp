#include "info/PrimitiveSizes.hpp"
#include <cassert>

LiteralKind getIntegerLiteralKind(bool isSigned, int bitSize) {
    bitSize >>= 3; //8 turns to 1, 64 turns to 8
	assert((bitSize > 0 && bitSize < 3) || bitSize == 4 || bitSize == 8);
	return static_cast<LiteralKind>(isSigned ? -bitSize : bitSize);
}
