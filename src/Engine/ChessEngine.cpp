#include "ChessEngine.h"

#include <cassert>
#include <iostream>

#include "ChessConstants.h"
#include "Magic.h"
#include "../Platform.h"

static uint64_t Reverse(uint64_t i) {
	// HD, Figure 7-1
	i = (i & 0x5555555555555555L) << 1 | ((i >> 1) & 0x5555555555555555L);
	i = (i & 0x3333333333333333L) << 2 | ((i >> 2) & 0x3333333333333333L);
	i = (i & 0x0f0f0f0f0f0f0f0fL) << 4 | ((i >> 4) & 0x0f0f0f0f0f0f0f0fL);
	i = (i & 0x00ff00ff00ff00ffL) << 8 | ((i >> 8) & 0x00ff00ff00ff00ffL);
	i = (i << 48) | ((i & 0xffff0000L) << 16) | ((i >> 16) & 0xffff0000L) | (i >> 48);
	return i;
}

void PrintBoard(uint64_t bitboard) {
	std::stringstream str;

	for(int i = 63; i >= 0; i--) {
		auto bit = (bitboard >> i) & 1;

		if(bit == 1) {
			str << "X|";
		} else {
			str << " |";
		}

		if(i % 8 == 0) {
			str << "\n";
		}
	}

	std::cout << str.str() << '\n';
}

ChessEngine::ChessEngine() : ChessEngine("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}

ChessEngine::ChessEngine(const std::string fen) : White(0), Black(0), P(0), N(0), R(0), B(0), Q(0), K(0), EP(0), unsafeForWhite(0), unsafeForBlack(0) {
	auto mask = 1ULL << 63;

	int i = 0;
	for(; mask; i++) {
		const auto c = fen[i];

		switch(c) {
			case '/': continue;
			case '1':
				mask >>= 1;
				continue;
			case '2':
				mask >>= 2;
				continue;
			case '3':
				mask >>= 3;
				continue;
			case '4':
				mask >>= 4;
				continue;
			case '5':
				mask >>= 5;
				continue;
			case '6':
				mask >>= 6;
				continue;
			case '7':
				mask >>= 7;
				continue;
			case '8':
				mask >>= 8;
				continue;
		}

		if(c > 'a') {
			Black |= mask;
		} else {
			White |= mask;
		}

		switch(c) {
			case 'r':
			case 'R':
				R |= mask;
				break;
			case 'n':
			case 'N':
				N |= mask;
				break;
			case 'b':
			case 'B':
				B |= mask;
				break;
			case 'q':
			case 'Q':
				Q |= mask;
				break;
			case 'k':
			case 'K':
				K |= mask;
				break;
			case 'p':
			case 'P':
				P |= mask;
				break;
		}

		mask >>= 1;
	}

	i++;
	const auto w = fen[i];
	WhiteMove = w == 'w';

	i += 2;

	while(true) {
		const auto c = fen[i];

		if(c == '-') {
			i += 2;
			break;
		}

		if(c == ' ') {
			i++;
			break;
		}

		switch(c) {
			case 'K':
				CastleWK = true;
				break;
			case 'Q':
				CastleWQ = true;
				break;
			case 'k':
				CastleBK = true;
				break;
			case 'q':
				CastleBQ = true;
				break;
		}

		i++;
	}

	const auto ep = fen[i];
	if(ep != '-') {
		const auto p = fen[i + 1];
		if(p == '3') {
			EP = 1ULL << (7 - (ep - 'a') + 3 * 8);
		} else if(p == '6') {
			EP = 1ULL << (7 - (ep - 'a') + 4 * 8);
		} else {
			throw std::logic_error("Invalid en passant position");
		}

		i++;
	}

	// TODO other fen values

	CalcTables();
}

void ChessEngine::MakeMove(Move m) {
	EP = 0;

	auto start = 1ULL << (63 - (m.X0 + (m.Y0 << 3)));
	auto end = 1ULL << (63 - (m.X1 + (m.Y1 << 3)));
	auto moveMask = start | end;
	auto endM = ~end;
	uint64_t tmp = 0;

	switch(m.Type) {
		case MoveType::WhiteEnPassant:
			tmp = 1ULL << (63 - (m.X1 + (m.Y0 << 3)));

			P ^= moveMask | tmp;
			White ^= moveMask;
			Black ^= tmp;
			goto end;
		case MoveType::BlackEnPassant:
			tmp = 1ULL << (63 - (m.X1 + (m.Y0 << 3)));

			P ^= moveMask | tmp;
			Black ^= moveMask;
			White ^= tmp;
			goto end;
		case MoveType::WhiteCastle:
			K ^= moveMask;
			White ^= moveMask;

			if(m.X0 < m.X1) {
				R ^= 0b101;
				White ^= 0b101;
			} else {
				R ^= 0b10010000;
				White ^= 0b10010000;
			}

			CastleWK = false;
			CastleWQ = false;
			goto end;
		case MoveType::BlackCastle:
			K ^= moveMask;
			Black ^= moveMask;

			if(m.X0 < m.X1) {
				R ^= 0b101ULL << 56;
				Black ^= 0b101ULL << 56;
			} else {
				R ^= 0b10010000ULL << 56;
				Black ^= 0b10010000ULL << 56;
			}

			CastleBK = false;
			CastleBQ = false;
			goto end;
	}

	// Remove dest | King can't be killed
	P &= endM;
	N &= endM;
	B &= endM;
	R &= endM;
	Q &= endM;

	if(WhiteMove) {
		Black &= endM;
		White ^= moveMask;
	} else {
		White &= endM;
		Black ^= moveMask;
	}

	switch(m.Type) {
		case MoveType::Knight:
			N ^= moveMask;
			break;
		case MoveType::Bishop:
			B ^= moveMask;
			break;
		case MoveType::Queen:
			Q ^= moveMask;
			break;

			#pragma region White
		case MoveType::WhitePawn:
			P ^= moveMask;

			if(m.Y0 - 2 == m.Y1) {
				EP = FileMask(7 - m.X0);
			}

			break;
		case MoveType::WhiteRook:
			R ^= moveMask;
			switch(start) {
				case 1:
					CastleWK = false;
					break;
				case 0x80:
					CastleWQ = false;
					break;
			}

			break;
		case MoveType::WhiteKing:
			K ^= moveMask;
			CastleWK = false;
			CastleWQ = false;
			break;
			#pragma endregion

			#pragma region Black
		case MoveType::BlackPawn:
			P ^= moveMask;

			if(m.Y0 + 2 == m.Y1) {
				EP = FileMask(7 - m.X0);
			}

			break;
		case MoveType::BlackRook:
			R ^= moveMask;

			switch(start) {
				case 1ULL << 56:
					CastleBK = false;
					break;
				case 0x1ULL << 63:
					CastleBQ = false;
					break;
			}

			break;
		case MoveType::BlackKing:
			K ^= moveMask;

			CastleBK = false;
			CastleBQ = false;
			break;
			#pragma endregion

			#pragma region Promotion
		case MoveType::PromotionN:
			P ^= start;
			N ^= end;
			break;
		case MoveType::PromotionB:
			P ^= start;
			B ^= end;
			break;
		case MoveType::PromotionR:
			P ^= start;
			R ^= end;
			break;
		case MoveType::PromotionQ:
			P ^= start;
			Q ^= end;
			break;
			#pragma endregion

		default:
			throw std::invalid_argument("Invalid move type");
	}

end: // TODO: remove goto
	WhiteMove = !WhiteMove;

	// CalcTables();
	occupied = P | N | B | R | Q | K;
	revOccupied = Reverse(occupied);
	empty = ~occupied;
	if(WhiteMove) {
		unsafeForBlack = UnsafeForBlack();
	} else {
		unsafeForWhite = UnsafeForWhite();
	}
}

std::ostream& operator<<(std::ostream& str, const ChessEngine& game) {
	for(int i = 63; i >= 0; i--) {
		if(i % 8 == 7) {
			str << "   +---+---+---+---+---+---+---+---+\n " << i / 8 + 1;
		}

		str << " | ";
		const auto mask = 1ULL << i;

#if true
		// utf-8 characters
		// TODO: add console param
		if((game.P & mask) != 0) {
			str << ((game.White & mask) != 0 ? "♙" : "♟");
		} else if((game.N & mask) != 0) {
			str << ((game.White & mask) != 0 ? "♘" : "♞");
		} else if((game.R & mask) != 0) {
			str << ((game.White & mask) != 0 ? "♖" : "♜");
		} else if((game.B & mask) != 0) {
			str << ((game.White & mask) != 0 ? "♗" : "♝");
		} else if((game.Q & mask) != 0) {
			str << ((game.White & mask) != 0 ? "♕" : "♛");
		} else if((game.K & mask) != 0) {
			str << ((game.White & mask) != 0 ? "♔" : "♚");
		} else {
			str << ' ';
		}
#else

		if((game.P & mask) != 0) {
			str << ((game.White & mask) != 0 ? 'P' : 'p');
		} else if((game.N & mask) != 0) {
			str << ((game.White & mask) != 0 ? 'N' : 'n');
		} else if((game.R & mask) != 0) {
			str << ((game.White & mask) != 0 ? 'R' : 'r');
		} else if((game.B & mask) != 0) {
			str << ((game.White & mask) != 0 ? 'B' : 'b');
		} else if((game.Q & mask) != 0) {
			str << ((game.White & mask) != 0 ? 'Q' : 'q');
		} else if((game.K & mask) != 0) {
			str << ((game.White & mask) != 0 ? 'K' : 'k');
		} else {
			str << ' ';
		}
#endif

		if(i % 8 == 0) {
			str << " |\n";
		}
	}

	str << "   +---+---+---+---+---+---+---+---+\n";
	str << "     A   B   C   D   E   F   G   H\n";

	return str;
}

void ChessEngine::print() const {
	std::cout << *this << std::endl;
}

void ChessEngine::CalcTables() {
	occupied = P | N | B | R | Q | K;
	revOccupied = Reverse(occupied);
	empty = ~occupied;

	unsafeForWhite = UnsafeForWhite();
	unsafeForBlack = UnsafeForBlack();
}

bool ChessEngine::IsValid() const {
	return WhiteMove ? !(unsafeForBlack & Black & K) : !(unsafeForWhite & White & K);
}

bool ChessEngine::IsCheck() const {
	return WhiteMove ? unsafeForWhite & White & K : unsafeForBlack & Black & K;
}

bool ChessEngine::IsCheckmate() {
	if(!IsCheck()) {
		return false;
	}

	auto moves = GetMoves();
	for(const auto move : moves) {
		auto cp = *this;
		cp.MakeMove(move);

		if(cp.IsValid()) {
			return false;
		}
	}

	return true;
}

Test ChessEngine::GetMoves() {
	Test moves;
	if(WhiteMove) {
		const auto notWhitePieces = ~((occupied & White) | (Black & K)); // added BK to avoid illegal capture
		unsafeForWhite = UnsafeForWhite();
		PossibleWP(moves);
		PossibleN(moves, notWhitePieces, White & N);
		PossibleB(moves, notWhitePieces, White & B);
		PossibleR(moves, notWhitePieces, White & R, MoveType::WhiteRook);
		PossibleQ(moves, notWhitePieces, White & Q);
		PossibleK(moves, notWhitePieces & ~unsafeForWhite, White & K, MoveType::WhiteKing);
		PossibleWC(moves);
	} else {
		const auto notBlackPieces = ~((occupied & Black) | (White & K)); // added WK to avoid illegal capture
		unsafeForBlack = UnsafeForBlack();
		PossibleBP(moves);
		PossibleN(moves, notBlackPieces, Black & N);
		PossibleB(moves, notBlackPieces, Black & B);
		PossibleR(moves, notBlackPieces, Black & R, MoveType::BlackRook);
		PossibleQ(moves, notBlackPieces, Black & Q);
		PossibleK(moves, notBlackPieces & ~unsafeForBlack, Black & K, MoveType::BlackKing);
		PossibleBC(moves);
	}

	return moves;
}

Test ChessEngine::GetValidMoves() {
	auto moves = GetMoves();
	for(int i = 0; i < moves.size(); ++i) {
		auto cp = *this;
		cp.MakeMove(moves[i]);

		if(!cp.IsValid()) {
			moves.removeAt(i);
		}
	}
	return moves;
}

Piece ChessEngine::GetPiece(int position) const {
	auto mask = 1llu << (63 - position);
	int color = 0;
	if(White & mask) color = 0;
	if(Black & mask) color = 1;

	if(P & mask) return (Piece)((int)Piece::WhitePawn + color);
	if(N & mask) return (Piece)((int)Piece::WhiteKnight + color);
	if(B & mask) return (Piece)((int)Piece::WhiteBishop + color);
	if(R & mask) return (Piece)((int)Piece::WhiteRook + color);
	if(Q & mask) return (Piece)((int)Piece::WhiteQueen + color);
	if(K & mask) return (Piece)((int)Piece::WhiteKing + color);

	return Piece::Empty;
}

Piece ChessEngine::GetPiece(int column, int row) const {
	return GetPiece(row * 8 + column);
}

void ChessEngine::PossibleWP(Test& moves) const {
	const auto blackPieces = Black & ~K; // omitted BK to avoid illegal capture
	const auto WP = White & P;
	const auto BP = Black & P;

	// Attack top right
	uint64_t mask = (WP << 7) & blackPieces & ~FileA & ~Rank8;
	uint64_t poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::WhitePawn
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}

	// Attack top left
	mask = (WP << 9) & blackPieces & ~FileH & ~Rank8;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::WhitePawn
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}

	// Move 1 forward
	mask = (WP << 8) & empty & ~Rank8;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::WhitePawn
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}

	// Move 2 forward
	mask = (WP << 16) & empty & (empty << 8) & Rank4;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1,
			(y1 + 2),
			x1,
			y1,
			MoveType::WhitePawn
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}

	// Promote by Move 1 forward
	mask = (WP << 8) & empty & Rank8;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionN
		);
		moves.emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionB
		);
		moves.emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionR
		);
		moves.emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionQ
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}

	// Promote by Attack top right
	mask = (WP << 7) & blackPieces & ~FileA & Rank8;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionN
		);
		moves.emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionB
		);
		moves.emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionR
		);
		moves.emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionQ
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}

	// Promote by Attack top left
	mask = (WP << 9) & blackPieces & ~FileH & Rank8;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionN
		);
		moves.emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionB
		);
		moves.emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionR
		);
		moves.emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionQ
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}

	#pragma region En passant
	// en passant right
	mask = (WP >> 1) & BP & Rank5 & ~FileA & EP;
	if(mask != 0) {
		int i = NumberOfTrailingZeros(mask);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			(x1 - 1),
			y1,
			x1,
			(y1 - 1),
			MoveType::WhiteEnPassant
		);
	}

	// en passant left
	mask = (WP << 1) & BP & Rank5 & ~FileH & EP;
	if(mask != 0) {
		int i = NumberOfTrailingZeros(mask);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			(x1 + 1),
			y1,
			x1,
			(y1 - 1),
			MoveType::WhiteEnPassant
		);
	}
	#pragma endregion
}

void ChessEngine::PossibleBP(Test& moves) const {
	const auto whitePieces = White & ~K; // omitted WK to avoid illegal capture
	const auto BP = Black & P;
	const auto WP = White & P;

	#pragma region Attack top right
	uint64_t mask = (BP >> 7) & whitePieces & ~Rank1 & ~FileH;
	uint64_t poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1 + 1,
			y1 - 1,
			x1,
			y1,
			MoveType::BlackPawn
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}
	#pragma endregion

	#pragma region Attack top left
	mask = (BP >> 9) & whitePieces & ~Rank1 & ~FileA;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1 - 1,
			y1 - 1,
			x1,
			y1,
			MoveType::BlackPawn
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}
	#pragma endregion

	#pragma region Move 1 forward
	mask = (BP >> 8) & empty & ~Rank1;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1,
			y1 - 1,
			x1,
			y1,
			MoveType::BlackPawn
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}
	#pragma endregion

	#pragma region Move 2 forward
	mask = (BP >> 16) & empty & (empty >> 8) & Rank5;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1,
			y1 - 2,
			x1,
			y1,
			MoveType::BlackPawn
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}
	#pragma endregion

	#pragma region Promote by Move 1 forward
	mask = (BP >> 8) & empty & Rank1;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionN
		);
		moves.emplace_back(
			x1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionB
		);
		moves.emplace_back(
			x1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionR
		);
		moves.emplace_back(
			x1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionQ
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}
	#pragma endregion

	#pragma region Promote by Attack top right
	mask = (BP >> 7) & whitePieces & ~FileH & Rank1;
	poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1 + 1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionN
		);
		moves.emplace_back(
			x1 + 1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionB
		);
		moves.emplace_back(
			x1 + 1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionR
		);
		moves.emplace_back(
			x1 + 1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionQ
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}
	#pragma endregion

	#pragma region Promote by Attack top left
	mask = (BP >> 9) & whitePieces & ~FileA & Rank1;
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			(x1 - 1),
			(y1 - 1),
			x1,
			y1,
			MoveType::PromotionN
		);
		moves.emplace_back(
			(x1 - 1),
			(y1 - 1),
			x1,
			y1,
			MoveType::PromotionB
		);
		moves.emplace_back(
			(x1 - 1),
			(y1 - 1),
			x1,
			y1,
			MoveType::PromotionR
		);
		moves.emplace_back(
			(x1 - 1),
			(y1 - 1),
			x1,
			y1,
			MoveType::PromotionQ
		);

		mask &= ~poss;
		poss = mask & ~(mask - 1);
	}
	#pragma endregion

	#pragma region en passant
	// en passant right
	mask = (BP << 1) & WP & Rank4 & ~FileH & EP;
	if(mask != 0) {
		int i = NumberOfTrailingZeros(mask);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			x1 + 1,
			y1,
			x1,
			y1 + 1,
			MoveType::BlackEnPassant
		);
	}

	// en passant left
	mask = (BP >> 1) & WP & Rank4 & ~FileA & EP;
	if(mask != 0) {
		int i = NumberOfTrailingZeros(mask);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves.emplace_back(
			(x1 - 1),
			y1,
			x1,
			(y1 + 1),
			MoveType::BlackEnPassant
		);
	}
	#pragma endregion
}

void ChessEngine::PossibleN(Test& moves, uint64_t notMyPieces, uint64_t n) const {
	auto i = n & ~(n - 1);

	while(i) {
		const int location = NumberOfTrailingZeros(i);
		uint64_t possibility = KnightMoves[location] & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves.emplace_back(
				7 - location % 8,
				7 - location / 8,
				7 - index % 8,
				7 - index / 8,
				MoveType::Knight
			);
			possibility &= ~j;
			j = possibility & ~(possibility - 1);
		}

		n &= ~i;
		i = n & ~(n - 1);
	}
}

void ChessEngine::PossibleB(Test& moves, uint64_t notMyPieces, uint64_t b) const {
	auto i = b & ~(b - 1);

	while(i != 0) {
		const auto location = NumberOfTrailingZeros(i);
		uint64_t possibility = DiagMask(location, occupied) & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves.emplace_back(
				7 - location % 8,
				7 - location / 8,
				7 - index % 8,
				7 - index / 8,
				MoveType::Bishop
			);
			possibility &= ~j;
			j = possibility & ~(possibility - 1);
		}

		b &= ~i;
		i = b & ~(b - 1);
	}
}

void ChessEngine::PossibleR(Test& moves, uint64_t notMyPieces, uint64_t r, MoveType type) const {
	auto i = r & ~(r - 1);

	while(i != 0) {
		int location = NumberOfTrailingZeros(i);
		uint64_t possibility = StraightMask(location, occupied) & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves.emplace_back(
				7 - location % 8,
				7 - location / 8,
				7 - index % 8,
				7 - index / 8,
				type
			);
			possibility &= ~j;
			j = possibility & ~(possibility - 1);
		}

		r &= ~i;
		i = r & ~(r - 1);
	}
}

void ChessEngine::PossibleQ(Test& moves, uint64_t notMyPieces, uint64_t q) const {
	auto i = q & ~(q - 1);

	while(i != 0) {
		int location = NumberOfTrailingZeros(i);
		uint64_t possibility = (StraightMask(location, occupied) | DiagMask(location, occupied)) & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves.emplace_back(
				(7 - location % 8),
				(7 - location / 8),
				(7 - index % 8),
				(7 - index / 8),
				MoveType::Queen
			);
			possibility &= ~j;
			j = possibility & ~(possibility - 1);
		}

		q &= ~i;
		i = q & ~(q - 1);
	}
}

void ChessEngine::PossibleK(Test& moves, uint64_t notMyPieces, uint64_t k, MoveType type) const {
	auto i = k & ~(k - 1);

	while(i != 0) {
		const auto location = NumberOfTrailingZeros(i);
		uint64_t possibility = KingMoves[location] & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves.emplace_back(
				(7 - location % 8),
				(7 - location / 8),
				(7 - index % 8),
				(7 - index / 8),
				type
			);
			possibility &= ~j;
			j = possibility & ~(possibility - 1);
		}

		k &= ~i;
		i = k & ~(k - 1);
	}
}

void ChessEngine::PossibleWC(Test& moves) {
	if(unsafeForWhite & White & K) {
		return; // King is in check
	}

	if(CastleWK) {
		if(!(White & R & 1))
			CastleWK = false; // Rook has been hit
		else if(!((occupied | unsafeForWhite) & 0b110)) {
			moves.emplace_back(4, 7, 6, 7, MoveType::WhiteCastle);
		}
	}

	if(CastleWQ) {
		if(!(White & R & (1ULL << 7)))
			CastleWQ = false; // Rook has been hit
		else if((occupied & 0b01110000) == 0 && (unsafeForWhite & 0b00110000) == 0) {
			moves.emplace_back(4, 7, 2, 7, MoveType::WhiteCastle);
		}
	}
}

void ChessEngine::PossibleBC(Test& moves) {
	if(unsafeForBlack & Black & K) {
		return; // King is in check
	}

	if(CastleBK) {
		if((Black & R & (1ULL << 56)) == 0) {
			CastleBK = false; // Rook has been hit
		} else if(((occupied | unsafeForBlack) & (0b0110ULL << 56)) == 0) {
			moves.emplace_back(4, 0, 6, 0, MoveType::BlackCastle);
		}
	}

	if(CastleBQ) {
		if((Black & R & (1ULL << 63)) == 0) {
			CastleBQ = false; // Rook has been hit
		} else if((occupied & (0b0111ULL << 60)) == 0 && (unsafeForBlack & (0b0011ULL << 60)) == 0)
			moves.emplace_back(4, 0, 2, 0, MoveType::BlackCastle);
	}
}

uint64_t ChessEngine::UnsafeForBlack() const {
	uint64_t res = 0;
	uint64_t i;

	#pragma region Pawn
	res |= ((White & P) << 7) & ~FileA;
	res |= ((White & P) << 9) & ~FileH;
	#pragma endregion

	#pragma region Knight
	{
		auto wn = White & N;
		i = wn & ~(wn - 1);

		while(i != 0) {
			res |= KnightMoves[NumberOfTrailingZeros(i)];;

			wn &= ~i;
			i = wn & ~(wn - 1);
		}
	}
	#pragma endregion

	#pragma region Bishop / Queen
	{
		auto qb = White & (B | Q);
		i = qb & ~(qb - 1);

		while(i != 0) {
			res |= DiagMask(NumberOfTrailingZeros(i), occupied);

			qb &= ~i;
			i = qb & ~(qb - 1);
		}
	}
	#pragma endregion

	#pragma region Rook / Queen
	{
		auto qr = White & (R | Q);
		i = qr & ~(qr - 1);

		while(i != 0) {
			res |= StraightMask(NumberOfTrailingZeros(i), occupied);

			qr &= ~i;
			i = qr & ~(qr - 1);
		}
	}
	#pragma endregion

	#pragma region King
	{
		auto wk = White & K;
		i = wk & ~(wk - 1);

		while(i != 0) {
			res |= KingMoves[NumberOfTrailingZeros(i)];

			wk &= ~i;
			i = wk & ~(wk - 1);
		}
	}
	#pragma endregion

	return res;
}

uint64_t ChessEngine::UnsafeForWhite() const {
	uint64_t res;
	uint64_t i;

	#pragma region Pawn
	res = ((Black & P) >> 7) & ~FileH | ((Black & P) >> 9) & ~FileA;
	#pragma endregion

	#pragma region Knight
	auto bn = Black & N;
	i = bn & ~(bn - 1);

	while(i != 0) {
		res |= KnightMoves[NumberOfTrailingZeros(i)];

		bn &= ~i;
		i = bn & ~(bn - 1);
	}
	#pragma endregion

	#pragma region Bishop / Queen
	auto qb = Black & (B | Q);
	i = qb & ~(qb - 1);

	while(i != 0) {
		res |= DiagMask(NumberOfTrailingZeros(i), occupied);

		qb &= ~i;
		i = qb & ~(qb - 1);
	}
	#pragma endregion

	#pragma region Rook / Queen
	auto qr = Black & (R | Q);
	i = qr & ~(qr - 1);

	while(i != 0) {
		res |= StraightMask(NumberOfTrailingZeros(i), occupied);

		qr &= ~i;
		i = qr & ~(qr - 1);
	}
	#pragma endregion

	#pragma region King
	auto bk = Black & K;
	i = bk & ~(bk - 1);

	while(i != 0) {
		res |= KingMoves[NumberOfTrailingZeros(i)];

		bk &= ~i;
		i = bk & ~(bk - 1);
	}
	#pragma endregion

	return res;
}
