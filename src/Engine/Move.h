﻿#pragma once

#include "Defines.h"
#include <string>
#include <sstream>

enum class MoveType : unsigned char {
	Error,
	Knight,
	Bishop,
	Queen,

	PromotionN,
	PromotionR,
	PromotionB,
	PromotionQ,

	WhitePawn,
	WhiteEnPassant,
	WhiteRook,
	WhiteKing,
	WhiteCastle,

	BlackPawn,
	BlackEnPassant,
	BlackRook,
	BlackKing,
	BlackCastle,
};

struct Move {
	byte X0;
	byte Y0;

	byte X1;
	byte Y1;

	MoveType Type;

	Move() : X0(0), Y0(0), X1(0), Y1(0), Type(MoveType::Error) {
	} ;

	Move(byte x0, byte y0, byte x1, byte y1, MoveType type): X0(x0), Y0(y0), X1(x1), Y1(y1), Type(type) {
	};

	Move(std::string name, MoveType type): Type(type) {
		X0 = name[0] - 'a';
		Y0 = 7 - (name[1] - '1');
		X1 = name[2] - 'a';
		Y1 = 7 - (name[3] - '1');
	}
};

inline bool operator==(const Move &a, const Move& b) {
	return
		a.X0 == b.X0 &&
		a.Y0 == b.Y0 &&
		a.X1 == b.X1 &&
		a.Y1 == b.Y1;
}

inline std::ostream& operator<<(std::ostream& strm, const Move& m) {
	strm << char('a' + m.X0) << (8 - m.Y0) << char('a' + m.X1) << (8 - m.Y1);

	switch(m.Type) {
		case MoveType::PromotionN:
			strm << "N";
		case MoveType::PromotionB:
			strm << "B";
		case MoveType::PromotionQ:
			strm << "Q";
		case MoveType::PromotionR:
			strm << "R";
	}

	return strm;
}
