#pragma once

#include "Move.h"

#include <string>
#include <array>
#include <optional>

#if true

class Test {
private:
	std::array<Move, 128> moves;
	int pos = 0;

public:
	void emplace_back(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, MoveType type) {
		if(pos >= moves.size()) {
			throw std::logic_error("ehh");
		}

		auto& m = moves[pos];
		m.X0 = x0;
		m.Y0 = y0;
		m.X1 = x1;
		m.Y1 = y1;
		m.Type = type;

		pos++;
	}

	void push(const Move m) {
		if(pos >= moves.size()) {
			throw std::logic_error("ehh");
		}
		moves[pos] = m;
		pos++;
	}

	void pop_back() {
		pos--;
	}

	void remove(const Move m) {
		int i = 0;
		while(true) {
			if(i == moves.size()) return;
			if(moves[i] == m) break;
			i++;
		}

		for(int j = i + 1; j < pos; j++) {
			moves[j - 1] = moves[j];
		}
		pos--;
	}
	void removeAt(int pos) {
		for(int j = pos + 1; j < this->pos; j++) {
			moves[j - 1] = moves[j];
		}
		this->pos--;
	}
	
	auto size() const {
		return pos;
	}
	auto empty() const {
		return pos == 0;
	}

	auto begin() const { return moves.begin(); }
	auto end() const { return moves.begin() + pos; }

	auto begin() { return moves.begin(); }
	auto end() { return moves.begin() + pos; }

	auto operator[](int pos) {
		return moves[pos];
	}
};

#else
class Test {
	std::vector<Move> moves;

public:
	Test() {
		moves.reserve(64);
	}

	void push(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, MoveType type) {
		moves.emplace_back(x0, y0, x1, y1, type);
	}

	auto pop_back() {
		moves.pop_back();
	}

	auto size() const {
		return moves.size();
	}
	auto empty() const {
		return moves.empty();
	}

	auto begin() const { return moves.begin(); }
	auto end() const { return moves.end(); }

	auto begin() { return moves.begin(); }
	auto end() { return moves.end(); }

	auto operator[](int pos) {
		return moves[pos];
	}
};
#endif

enum class Piece {
	WhitePawn,
	BlackPawn,
	WhiteKnight,
	BlackKnight,
	WhiteBishop,
	BlackBishop,
	WhiteRook,
	BlackRook,
	WhiteQueen,
	BlackQueen,
	WhiteKing,
	BlackKing,
	Empty
};

class ChessEngine {
public:
	bool CastleWK = false;
	bool CastleWQ = false;
	bool CastleBK = false;
	bool CastleBQ = false;
	bool WhiteMove = false;

	uint64_t White, Black;
	uint64_t P, N, R, B, Q, K;
	uint64_t EP;

	// Temporary vars
	uint64_t unsafeForWhite, unsafeForBlack;
	uint64_t occupied, revOccupied, empty;
public:
	ChessEngine();
	ChessEngine(std::string fen);

	void MakeMove(Move m);

	bool IsValid() const;
	bool IsCheck() const;
	bool IsCheckmate();

	Test GetMoves();
	Test GetValidMoves();

	Piece GetPiece(int position) const;
	Piece GetPiece(int column, int row) const;

	void print() const;

	friend std::ostream& operator<<(std::ostream& stream, const ChessEngine& game);
private:
	void CalcTables();
	uint64_t UnsafeForBlack() const;
	uint64_t UnsafeForWhite() const;

	void PossibleWP(Test& moves) const;
	void PossibleBP(Test& moves) const;
	void PossibleN(Test& moves, uint64_t notMyPieces, uint64_t n) const;
	void PossibleB(Test& moves, uint64_t notMyPieces, uint64_t b) const;
	void PossibleR(Test& moves, uint64_t notMyPieces, uint64_t r, MoveType type) const;
	void PossibleQ(Test& moves, uint64_t notMyPieces, uint64_t q) const;
	void PossibleK(Test& moves, uint64_t notMyPieces, uint64_t k, MoveType type) const;
	void PossibleWC(Test& moves);
	void PossibleBC(Test& moves);
};

void PrintBoard(uint64_t bitboard);
