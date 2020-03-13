#pragma once

#include "Move.h"

#include <string>
#include <vector>

class ChessEngine {
public:
	bool CastleWK;
	bool CastleWQ;
	bool CastleBK;
	bool CastleBQ;
	bool WhiteMove;

	uint64_t White, Black;
	uint64_t P, N, R, B, Q, K;
	uint64_t EP, unsafeForWhite, unsafeForBlack;

	uint64_t occupied, revOccupied, empty;
public:
	ChessEngine();
	ChessEngine(std::string fen);

	void MakeMove(Move m);

	bool IsValid() const;
	bool IsCheck() const;
	bool IsCheckmate();
	std::vector<Move> GetMoves();

	friend std::ostream& operator<<(std::ostream& strema, const ChessEngine& game);
private:
	void CalcTables();
	uint64_t UnsafeForBlack() const;
	uint64_t UnsafeForWhite() const;

	void PossibleWP(std::vector<Move>& moves) const;
	void PossibleBP(std::vector<Move>& moves) const;
	void PossibleN(std::vector<Move>& moves, uint64_t notMyPieces, uint64_t n) const;
	void PossibleB(std::vector<Move>& moves, uint64_t notMyPieces, uint64_t b) const;
	void PossibleR(std::vector<Move>& moves, uint64_t notMyPieces, uint64_t r, MoveType type) const;
	void PossibleQ(std::vector<Move>& moves, uint64_t notMyPieces, uint64_t q) const;
	void PossibleK(std::vector<Move>& moves, uint64_t notMyPieces, uint64_t k, MoveType type) const;
	void PossibleWC(std::vector<Move>& moves);
	void PossibleBC(std::vector<Move>& moves);
};

void PrintBoard(uint64_t bitboard);