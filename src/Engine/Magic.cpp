#include "Magic.h"

#include <cstdio>
#include <intrin.h>
#include <random>
#include <fstream>

const int BitTable[64] = {
	63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
	51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
	26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
	58, 20, 37, 17, 36, 8
};

const int RBits[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

const int BBits[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

std::random_device rnd;
std::mt19937_64 eng(rnd());
std::uniform_int_distribution<ulong> distr;

ulong random_ulong() {
	return distr(eng);
}

ulong random_ulong_fewbits() {
	return random_ulong() & random_ulong() & random_ulong();
}

static ulong rookMask(int sq) {
	ulong result = 0ULL;
	int rk = sq / 8, fl = sq % 8, r, f;
	for(r = rk + 1; r <= 6; r++) result |= (1ULL << (fl + r * 8));
	for(r = rk - 1; r >= 1; r--) result |= (1ULL << (fl + r * 8));
	for(f = fl + 1; f <= 6; f++) result |= (1ULL << (f + rk * 8));
	for(f = fl - 1; f >= 1; f--) result |= (1ULL << (f + rk * 8));
	return result;
}

ulong bishopMask(int sq) {
	ulong result = 0ULL;
	int rk = sq / 8, fl = sq % 8, r, f;
	for(r = rk + 1, f = fl + 1; r <= 6 && f <= 6; r++, f++) result |= (1ULL << (f + r * 8));
	for(r = rk + 1, f = fl - 1; r <= 6 && f >= 1; r++, f--) result |= (1ULL << (f + r * 8));
	for(r = rk - 1, f = fl + 1; r >= 1 && f <= 6; r--, f++) result |= (1ULL << (f + r * 8));
	for(r = rk - 1, f = fl - 1; r >= 1 && f >= 1; r--, f--) result |= (1ULL << (f + r * 8));
	return result;
}

int pop_1st_bit(ulong* bb) {
	ulong b = *bb ^ (*bb - 1);
	auto fold = (unsigned)((b & 0xffffffff) ^ (b >> 32));
	*bb &= (*bb - 1);
	return BitTable[(fold * 0x783a9b23) >> 26];
}

ulong index_to_ulong(int id, int bits, ulong m) {
	int i, j;
	ulong result = 0ULL;
	for(i = 0; i < bits; i++) {
		j = pop_1st_bit(&m);
		if(id & (1 << i))
			result |= (1ULL << j);
	}
	return result;
}

ulong rookAttack(int sq, ulong block) {
	ulong result = 0ULL;
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

ulong bishopAttack(int sq, ulong block) {
	ulong result = 0ULL;
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

int transform(ulong b, ulong magic, int bits) {
	return (int)((b * magic) >> (64 - bits));
}

static ulong bishopAttacks[64][512];
static ulong rookAttacks[64][4096];
SMagic bishopTbl[64];
SMagic rookTbl[64];

void find_magic(int sq, int m, bool bishop) {
	ulong b[4096], a[4096], used[4096];

	ulong mask = bishop ? bishopMask(sq) : rookMask(sq);
	int n = __popcnt64(mask);

	for(int i = 0; i < (1 << n); i++) {
		b[i] = index_to_ulong(i, n, mask);
		a[i] = bishop ? bishopAttack(sq, b[i]) : rookAttack(sq, b[i]);
	}

	while(true) {
		ulong magic = random_ulong_fewbits();
		if(__popcnt64((mask * magic) & 0xFF00000000000000ULL) < 7)
			continue;

		for(int i = 0; i < 4096; i++)
			used[i] = 0ULL;

		bool fail = false;
		for(int i = 0; i < (1 << n); i++) {
			int j = transform(b[i], magic, m);
			if(used[j] == 0ULL) {
				used[j] = a[i];
			} else if(used[j] != a[i]) {
				fail = true;
				break;
			}
		}

		if(!fail) {
			if(bishop) {
				for(int i = 0; i < 512; ++i) {
					bishopAttacks[sq][i] = used[i];
				}
				bishopTbl[sq] = SMagic(mask, magic);
			} else {
				for(int i = 0; i < 4096; ++i) {
					rookAttacks[sq][i] = used[i];
				}
				rookTbl[sq] = SMagic(mask, magic);
			}

			return;
		}
	}
	printf("***Failed***\n");
}

void CalcMagic() {
	for(int square = 0; square < 64; square++) {
		
		find_magic(square, BBits[square], true);
		find_magic(square, RBits[square], false);

		printf("%i\n", square);
	}

	std::ofstream out("massive.h");
	out.setf(std::ios::hex, std::ios::basefield);
	out.setf(std::ios::showbase);

	out << "#pragma once\n";
	out << "const ulong rookAttacks[64][4096] = {\n";
	for(int i = 0; i < 64; ++i) {
		out << "    {";
		for(int j = 0; j < 4096; ++j) {
			out << rookAttacks[i][j] << ",";
		}
		out << "},\n";
	}
	out << "};\n\n";

	out << "const ulong bishopAttacks[64][512] = {\n";
	for(int i = 0; i < 64; ++i) {
		out << "    {";
		for(int j = 0; j < 512; ++j) {
			out << bishopAttacks[i][j] << ",";
		}
		out << "},\n";
	}
	out << "};\n\n";

	out << "const SMagic rookTbl[64] = {\n";
	for(int i = 0; i < 64; ++i) {
		out << "    SMagic(" << rookTbl[i].mask << "," << rookTbl[i].magic << "),\n";
	}
	out << "};\n\n";

	out << "const SMagic bishopTbl[64] = {\n";
	for(int i = 0; i < 64; ++i) {
		out << "    SMagic(" << bishopTbl[i].mask << "," << bishopTbl[i].magic << "),\n";
	}
	out << "};\n\n";

	out.close();
}