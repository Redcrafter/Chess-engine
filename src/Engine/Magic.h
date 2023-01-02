#pragma once
#include <cstdint>

struct SMagic {
	uint64_t mask;
	uint64_t magic;
};

extern uint64_t bishopAttacks[64][512];
extern uint64_t rookAttacks[64][4096];
extern SMagic bishopTbl[64];
extern SMagic rookTbl[64];

static const uint64_t StraightMask(const int s, const uint64_t occupied) {
	const auto mag = rookTbl[s];
	return rookAttacks[s][((occupied & mag.mask) * mag.magic) >> (64 - 12)];
}

static const uint64_t DiagMask(const int s, const uint64_t occupied) {
	const auto mag = bishopTbl[s];
	return bishopAttacks[s][((occupied & mag.mask) * mag.magic) >> (64 - 9)];
}

void CalcMagic();
