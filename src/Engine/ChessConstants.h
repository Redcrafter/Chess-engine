#pragma once
#include <array>

const uint64_t FileA = 0x8080808080808080u,
			   FileH = 0x0101010101010101u,
			   Rank1 = 0xFFu,
			   Rank4 = 0xFF000000u,
			   Rank5 = 0xFF00000000u,
			   Rank8 = 0xFF00000000000000u;

const uint64_t DiagMasks[] = {
	// from bottom right to top left
	0x0000000000000001u, 0x0000000000000102u, 0x0000000000010204u,
	0x0000000001020408u, 0x0000000102040810u, 0x0000010204081020u,
	0x0001020408102040u, 0x0102040810204080u, 0x0204081020408000u,
	0x0408102040800000u, 0x0810204080000000u, 0x1020408000000000u,
	0x2040800000000000u, 0x4080000000000000u, 0x8000000000000000u,
};

const uint64_t AntiDiagMasks[] = {
	// from bottom left to top right
	0x0000000000000080u, 0x0000000000008040u, 0x0000000000804020u,
	0x0000000080402010u, 0x0000008040201008u, 0x0000804020100804u,
	0x0080402010080402u, 0x8040201008040201u, 0x4020100804020100u,
	0x2010080402010000u, 0x1008040201000000u, 0x0804020100000000u,
	0x0402010000000000u, 0x0201000000000000u, 0x0100000000000000u
};

constexpr uint64_t FileMask(int file) {
	return FileH << file;
}

constexpr std::array<uint64_t, 64> KnightBoard() {
	const auto KnightMoves = 0x0000000A1100110AULL;
	std::array<uint64_t, 64> moves {};

	for (size_t location = 0; location < 64; location++) {
		uint64_t possibility = 0;
		if(location > 18) {
			possibility = KnightMoves << (location - 18);
		} else {
			possibility = KnightMoves >> (18 - location);
		}

		if(location % 8 < 4) {
			possibility &= ~(FileMask(7) | FileMask(6));
		} else {
			possibility &= ~(FileMask(1) | FileMask(0));
		}

		moves[location] = possibility;
	}

	return moves;
}

constexpr std::array<uint64_t, 64> KingBoard() {
	const auto KingMoves = 0x070507ULL;
	std::array<uint64_t, 64> moves {};

	for (size_t location = 0; location < 64; location++) {
		uint64_t possibility = 0;
		if(location > 9) {
			possibility = KingMoves << (location - 9);
		} else {
			possibility = KingMoves >> (9 - location);
		}

		if(location % 8 < 4) {
			possibility &= ~FileA;
		} else {
			possibility &= ~FileH;
		}

		moves[location] = possibility;
	}

	return moves;
}

const std::array<uint64_t, 64> KnightMoves = KnightBoard();
const std::array<uint64_t, 64> KingMoves = KingBoard();
