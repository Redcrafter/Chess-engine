#pragma once

#include "Move.h"

#include <intrin.h>
#include <string>
#include <iostream>
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

	ChessEngine();
	ChessEngine(std::string fen);
	ChessEngine(const ChessEngine& other);
	~ChessEngine();

	void MakeMove(Move m);
	// bool CheckMate();
	void PrintBoard() const;

	bool IsValid() const;
	bool IsCheck() const;
	bool IsCheckmate();
	std::vector<Move>* GetMoves();
private:
	std::vector<Move>* _moves = nullptr;
	uint64_t occupied, revOccupied, empty;

	void CalcTables();
	uint64_t UnsafeForBlack() const;
	uint64_t UnsafeForWhite() const;

	std::vector<Move>* PossibleMoves();
	
	void PossibleWP(std::vector<Move>* moves) const;
	void PossibleBP(std::vector<Move>* moves) const;
	static void PossibleN(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t n);
	void PossibleB(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t b) const;
	void PossibleR(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t r, MoveType type) const;
	void PossibleQ(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t q) const;
	static void PossibleK(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t k, MoveType type);
	void PossibleWC(std::vector<Move>* moves);
	void PossibleBC(std::vector<Move>* moves);

	uint64_t StraightMask(int s) const;
	uint64_t DiagMask(int s) const;
};

void PrintBoard(uint64_t bitboard);