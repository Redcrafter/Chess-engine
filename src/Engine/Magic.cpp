#include "Magic.h"
#include "Platform.h"

#include <random>

const int BitTable[64] = {
	63, 30,  3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34,
	61, 29,  2, 51, 21, 43, 45, 10, 18, 47,  1, 54,  9, 57,  0, 35,
	62, 31, 40,  4, 49,  5, 52, 26, 60,  6, 23, 44, 46, 27, 56, 16,
	 7, 39, 48, 24, 59, 14, 12, 55, 38, 28, 58, 20, 37, 17, 36,  8
};

// std::random_device rnd;
std::mt19937_64 eng(1234); // use set seed
std::uniform_int_distribution<uint64_t> distr;

uint64_t random_uint64() {
	return distr(eng);
}

uint64_t random_uint64_fewbits() {
	return random_uint64() & random_uint64() & random_uint64();
}

uint64_t rookMask(int sq) {
	uint64_t result = 0;
	int rk = sq / 8, fl = sq % 8, r, f;
	for(r = rk + 1; r <= 6; r++) result |= (1ULL << (fl + r * 8));
	for(r = rk - 1; r >= 1; r--) result |= (1ULL << (fl + r * 8));
	for(f = fl + 1; f <= 6; f++) result |= (1ULL << (f + rk * 8));
	for(f = fl - 1; f >= 1; f--) result |= (1ULL << (f + rk * 8));
	return result;
}

uint64_t bishopMask(int sq) {
	uint64_t result = 0;
	int rk = sq / 8, fl = sq % 8, r, f;
	for(r = rk + 1, f = fl + 1; r <= 6 && f <= 6; r++, f++) result |= (1ULL << (f + r * 8));
	for(r = rk + 1, f = fl - 1; r <= 6 && f >= 1; r++, f--) result |= (1ULL << (f + r * 8));
	for(r = rk - 1, f = fl + 1; r >= 1 && f <= 6; r--, f++) result |= (1ULL << (f + r * 8));
	for(r = rk - 1, f = fl - 1; r >= 1 && f >= 1; r--, f--) result |= (1ULL << (f + r * 8));
	return result;
}

int pop_1st_bit(uint64_t* bb) {
	uint64_t b = *bb ^ (*bb - 1);
	auto fold = (unsigned)((b & 0xffffffff) ^ (b >> 32));
	*bb &= (*bb - 1);
	return BitTable[(fold * 0x783a9b23) >> 26];
}

uint64_t index_to_uint64(int id, int bits, uint64_t m) {
	int i, j;
	uint64_t result = 0ULL;
	for(i = 0; i < bits; i++) {
		j = pop_1st_bit(&m);
		if(id & (1 << i))
			result |= (1ULL << j);
	}
	return result;
}

uint64_t rookAttack(int sq, uint64_t block) {
	uint64_t result = 0;
	int rk = sq / 8, fl = sq % 8, r, f;
	for(r = rk + 1; r <= 7; r++) {
		result |= (1ULL << (fl + r * 8));
		if(block & (1ULL << (fl + r * 8))) break;
	}
	for(r = rk - 1; r >= 0; r--) {
		result |= (1ULL << (fl + r * 8));
		if(block & (1ULL << (fl + r * 8))) break;
	}
	for(f = fl + 1; f <= 7; f++) {
		result |= (1ULL << (f + rk * 8));
		if(block & (1ULL << (f + rk * 8))) break;
	}
	for(f = fl - 1; f >= 0; f--) {
		result |= (1ULL << (f + rk * 8));
		if(block & (1ULL << (f + rk * 8))) break;
	}
	return result;
}

uint64_t bishopAttack(int sq, uint64_t block) {
	uint64_t result = 0;
	int rk = sq / 8, fl = sq % 8, r, f;
	for(r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++) {
		result |= (1ULL << (f + r * 8));
		if(block & (1ULL << (f + r * 8))) break;
	}
	for(r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--) {
		result |= (1ULL << (f + r * 8));
		if(block & (1ULL << (f + r * 8))) break;
	}
	for(r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++) {
		result |= (1ULL << (f + r * 8));
		if(block & (1ULL << (f + r * 8))) break;
	}
	for(r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--) {
		result |= (1ULL << (f + r * 8));
		if(block & (1ULL << (f + r * 8))) break;
	}
	return result;
}

int transform(uint64_t b, uint64_t magic, int bits) {
	return (int)((b * magic) >> (64 - bits));
}

uint64_t bishopAttacks[64][512];
uint64_t rookAttacks[64][4096];
SMagic bishopTbl[64];
SMagic rookTbl[64];

void find_magic(int sq, int m, bool bishop) {
	uint64_t b[4096];
	uint64_t a[4096];
	uint64_t used[4096];

	uint64_t mask = bishop ? bishopMask(sq) : rookMask(sq);
	int n = popcnt64(mask);

	for(int i = 0; i < (1 << n); i++) {
		b[i] = index_to_uint64(i, n, mask);
		a[i] = bishop ? bishopAttack(sq, b[i]) : rookAttack(sq, b[i]);
	}

	while(true) {
		uint64_t magic = random_uint64_fewbits();
		if(popcnt64((mask * magic) & 0xFF00000000000000ULL) < 7)
			continue;

		std::memset(used, 0, sizeof(used));

		bool fail = false;
		for(int i = 0; i < (1 << n); i++) {
			int j = transform(b[i], magic, m);
			if(used[j] == 0) {
				used[j] = a[i];
			} else if(used[j] != a[i]) {
				fail = true;
				break;
			}
		}

		if(fail)
			continue;

		if(bishop) {
			for(int i = 0; i < 512; ++i) {
				bishopAttacks[sq][i] = used[i];
			}
			bishopTbl[sq] = { mask, magic };
		} else {
			for(int i = 0; i < 4096; ++i) {
				rookAttacks[sq][i] = used[i];
			}
			rookTbl[sq] = { mask, magic };
		}

		return;
	}
}

void CalcMagic() {
	#pragma omp parallel for
	for(int square = 0; square < 64; square++) {
		find_magic(square, 12, false);
		find_magic(square, 9, true);
	}
}
