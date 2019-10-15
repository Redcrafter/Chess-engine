#pragma once

#include "Defines.h"
#include "Move.h"

#include <intrin.h>
#include <string>
#include <iostream>
#include <vector>

enum Stuff : byte {
	Emtpy,
	CastleWK = 1,
	CastleWQ = 2,
	CastleBK = 4,
	CastleBQ = 8,
	WhiteMove = 16
};

class ChessEngine {
public:
	byte Flags;

	ulong White, Black;
	ulong P, N, R, B, Q, K;
	ulong EP, unsafeForWhite, unsafeForBlack;

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
	std::vector<Move>* _moves{};
	ulong occupied{}, revOccupied{}, empty{};

	void CalcTables();
	ulong UnsafeForBlack() const;
	ulong UnsafeForWhite() const;

	std::vector<Move>* PossibleMoves();
	
	void PossibleWP(std::vector<Move>* moves) const;
	void PossibleBP(std::vector<Move>* moves) const;
	static void PossibleN(std::vector<Move>* moves, ulong notMyPieces, ulong n);
	void PossibleB(std::vector<Move>* moves, ulong notMyPieces, ulong b) const;
	void PossibleR(std::vector<Move>* moves, ulong notMyPieces, ulong r, MoveType type) const;
	void PossibleQ(std::vector<Move>* moves, ulong notMyPieces, ulong q) const;
	static void PossibleK(std::vector<Move>* moves, ulong notMyPieces, ulong k, MoveType type);
	void PossibleWC(std::vector<Move>* moves);
	void PossibleBC(std::vector<Move>* moves);

	ulong StraightMask(int s) const;
	ulong DiagMask(int s) const;
};

void PrintBoard(ulong bitboard);