#pragma once
#include <cstdint>

struct SMagic {
	uint64_t mask;
	uint64_t magic;
};

void CalcMagic();