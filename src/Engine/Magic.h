#pragma once
#include "ChessConstants.h"

struct SMagic {
	ulong mask;
	ulong magic;

	SMagic() : mask(0), magic(0) {
	};

	SMagic(ulong mask, ulong magic) : mask(mask), magic(magic) {
	};
};

void CalcMagic();