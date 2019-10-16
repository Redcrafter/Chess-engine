#include "ChessEngine.h"
#include "ChessConstants.h"
#include "Tables.h"

#include <sstream>

#define NumberOfTrailingZeros _tzcnt_u64

static uint64_t Reverse(uint64_t i) {
	// HD, Figure 7-1
	i = (i & 0x5555555555555555L) << 1 | (i >> 1) & 0x5555555555555555L;
	i = (i & 0x3333333333333333L) << 2 | (i >> 2) & 0x3333333333333333L;
	i = (i & 0x0f0f0f0f0f0f0f0fL) << 4 | (i >> 4) & 0x0f0f0f0f0f0f0f0fL;
	i = (i & 0x00ff00ff00ff00ffL) << 8 | (i >> 8) & 0x00ff00ff00ff00ffL;
	i = (i << 48) | ((i & 0xffff0000L) << 16) | ((i >> 16) & 0xffff0000L) | (i >> 48);
	_tzcnt_u64(10);
	return i;
}

static uint64_t FileMask(int file) {
	return 0x0101010101010101ULL << file;
}

static uint64_t RevFileMask(int file) {
	return 0x0101010101010101ULL << (7 - file);
}

static uint64_t RankMask(int rank) {
	return 0xFFULL << (rank << 3);
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

ChessEngine::~ChessEngine() {
	delete _moves;
}

ChessEngine::ChessEngine(): ChessEngine("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
}

ChessEngine::ChessEngine(std::string fen) : White(0), Black(0), P(0), N(0), R(0), B(0), Q(0), K(0),
                                            EP(0), unsafeForWhite(0), unsafeForBlack(0) {
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
	if(w == 'w') {
		WhiteMove = true;
	}

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
			throw std::exception("Invalid en passant position");
		}

		i++;
	}

	// TODO other fen values

	CalcTables();
}

ChessEngine::ChessEngine(const ChessEngine& other) {
	memcpy(this, &other, sizeof(ChessEngine));
	this->_moves = nullptr;
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

end:
	WhiteMove = !WhiteMove;
	CalcTables();
}

void ChessEngine::CalcTables() {
	occupied = P | N | B | R | Q | K;
	revOccupied = Reverse(occupied);
	empty = ~occupied;

	unsafeForWhite = UnsafeForWhite();
	unsafeForBlack = UnsafeForBlack();
}

void ChessEngine::PrintBoard() const {
	std::stringstream str;

	for(int i = 63; i >= 0; i--) {
		if(i % 8 == 7) {
			str << "   +---+---+---+---+---+---+---+---+\n " << i / 8 + 1;
		}

		str << " | ";
		const auto mask = 1ULL << i;

		if((P & mask) != 0) {
			str << ((White & mask) != 0 ? 'P' : 'p');
		} else if((N & mask) != 0) {
			str << ((White & mask) != 0 ? 'N' : 'n');
		} else if((R & mask) != 0) {
			str << ((White & mask) != 0 ? 'R' : 'r');
		} else if((B & mask) != 0) {
			str << ((White & mask) != 0 ? 'B' : 'b');
		} else if((Q & mask) != 0) {
			str << ((White & mask) != 0 ? 'Q' : 'q');
		} else if((K & mask) != 0) {
			str << ((White & mask) != 0 ? 'K' : 'k');
		} else {
			str << (' ');
		}

		if(i % 8 == 0) {
			str << " |\n";
		}
	}

	str << ("   +---+---+---+---+---+---+---+---+\n");
	str << ("     A   B   C   D   E   F   G   H\n");

	std::cout << str.str();
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

	auto moves = *GetMoves();
	for(const auto move : moves) {
		auto cp = *this;
		cp.MakeMove(move);

		if(cp.IsValid()) {
			return false;
		}
	}

	return true;
}

std::vector<Move>* ChessEngine::GetMoves() {
	if(_moves == nullptr) {
		_moves = PossibleMoves();
	}

	return _moves;
}

std::vector<Move>* ChessEngine::PossibleMoves() {
	auto moves = new std::vector<Move>();

	if(WhiteMove) {
		const auto notWhitePieces = ~((occupied & White) | (Black & K)); // added BK to avoid illegal capture

		PossibleWP(moves);
		PossibleN(moves, notWhitePieces, White & N);
		PossibleB(moves, notWhitePieces, White & B);
		PossibleR(moves, notWhitePieces, White & R, MoveType::WhiteRook);
		PossibleQ(moves, notWhitePieces, White & Q);
		PossibleK(moves, notWhitePieces & ~unsafeForWhite, White & K, MoveType::WhiteKing);
		PossibleWC(moves);
	} else {
		const auto notBlackPieces = ~((occupied & Black) | (White & K)); // added WK to avoid illegal capture
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

void ChessEngine::PossibleWP(std::vector<Move>* moves) const {
	const auto blackPieces = Black & ~K; // omitted BK to avoid illegal capture
	const auto WP = White & P;
	const auto BP = Black & P;
	// TODO: change loop style

	// Attack top right
	uint64_t mask = (WP << 7) & blackPieces & ~FileA;
	for(int i = 16; i < 56; i++) {
		// Only go through rank 2-7
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::WhitePawn
		);
	}

	// Attack top left
	mask = (WP << 9) & blackPieces & ~FileH;
	for(int i = 16; i < 56; i++) {
		// Only go through rank 2-7
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::WhitePawn
		);
	}

	// Move 1 forward
	mask = (WP << 8) & empty;
	for(int i = 16; i < 56; i++) {
		// Only go through rank 2-7
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::WhitePawn
		);
	}

	// Move 2 forward
	mask = (WP << 16) & empty & (empty << 8);
	for(int i = 24; i < 32; i++) {
		// Only go through rank 4
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			x1,
			(y1 + 2),
			x1,
			y1,
			MoveType::WhitePawn
		);
	}

	// Promote by Move 1 forward
	mask = (WP << 8) & empty;
	for(int i = 56; i < 64; i++) {
		// Only go through rank8
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionN
		);
		moves->emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionB
		);
		moves->emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionR
		);
		moves->emplace_back(
			x1,
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionQ
		);
	}

	// Promote by Attack top right
	mask = (WP << 7) & blackPieces & ~FileA;
	for(int i = 56; i < 64; i++) {
		// Only go through rank8
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionN
		);
		moves->emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionB
		);
		moves->emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionR
		);
		moves->emplace_back(
			(x1 - 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionQ
		);
	}

	// Promote by Attack top left
	mask = (WP << 9) & blackPieces & ~FileH;
	for(int i = 56; i < 64; i++) {
		// Only go through rank8
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionN
		);
		moves->emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionB
		);
		moves->emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionR
		);
		moves->emplace_back(
			(x1 + 1),
			(y1 + 1),
			x1,
			y1,
			MoveType::PromotionQ
		);
	}

	#pragma region En passant
	// en passant right
	mask = (WP >> 1) & BP & Rank5 & ~FileA & EP;
	if(mask != 0) {
		int i = NumberOfTrailingZeros(mask);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
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

		moves->emplace_back(
			(x1 + 1),
			y1,
			x1,
			(y1 - 1),
			MoveType::WhiteEnPassant
		);
	}
	#pragma endregion
}

void ChessEngine::PossibleBP(std::vector<Move>* moves) const {
	const auto whitePieces = White & ~K; // omitted WK to avoid illegal capture
	const auto BP = Black & P;
	const auto WP = White & P;

	#pragma region Attack top right
	uint64_t mask = (BP >> 7) & whitePieces & ~Rank1 & ~FileH;
	uint64_t poss = mask & ~(mask - 1);
	while(poss != 0) {
		int i = NumberOfTrailingZeros(poss);

		const auto x1 = 7 - i % 8;
		const auto y1 = 7 - i / 8;

		moves->emplace_back(
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

		moves->emplace_back(
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

		moves->emplace_back(
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

		moves->emplace_back(
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
	mask = (BP >> 8) & empty;
	// Only go through rank1
	for(int i = 0; i < 8; i++) {
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			x1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionN
		);
		moves->emplace_back(
			x1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionB
		);
		moves->emplace_back(
			x1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionR
		);
		moves->emplace_back(
			x1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionQ
		);
	}
	#pragma endregion

	#pragma region Promote by Attack top right
	mask = (BP >> 7) & whitePieces;
	// Only go through rank1
	for(int i = 1; i < 8; i++) {
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			x1 + 1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionN
		);
		moves->emplace_back(
			x1 + 1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionB
		);
		moves->emplace_back(
			x1 + 1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionR
		);
		moves->emplace_back(
			x1 + 1,
			y1 - 1,
			x1,
			y1,
			MoveType::PromotionQ
		);
	}
	#pragma endregion

	#pragma region Promote by Attack top left
	mask = (BP >> 9) & whitePieces;
	for(int i = 0; i < 7; i++) {
		// Only go through rank8
		if(((mask >> i) & 1) == 0)
			continue;

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
			(x1 - 1),
			(y1 - 1),
			x1,
			y1,
			MoveType::PromotionN
		);
		moves->emplace_back(
			(x1 - 1),
			(y1 - 1),
			x1,
			y1,
			MoveType::PromotionB
		);
		moves->emplace_back(
			(x1 - 1),
			(y1 - 1),
			x1,
			y1,
			MoveType::PromotionR
		);
		moves->emplace_back(
			(x1 - 1),
			(y1 - 1),
			x1,
			y1,
			MoveType::PromotionQ
		);
	}
	#pragma endregion

	#pragma region en passant
	// en passant right
	mask = (BP << 1) & WP & Rank4 & ~FileH & EP;
	if(mask != 0) {
		int i = NumberOfTrailingZeros(mask);

		auto x1 = 7 - i % 8;
		auto y1 = 7 - i / 8;

		moves->emplace_back(
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

		moves->push_back(Move(
				(x1 - 1),
				y1,
				x1,
				(y1 + 1),
				MoveType::BlackEnPassant)
		);
	}
	#pragma endregion
}

void ChessEngine::PossibleN(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t n) {
	auto i = n & ~(n - 1);

	while(i) {
		const int location = NumberOfTrailingZeros(i);
		uint64_t possibility = KnightMoves[location] & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves->emplace_back(
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

void ChessEngine::PossibleB(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t b) const {
	auto i = b & ~(b - 1);

	while(i != 0) {
		const auto location = NumberOfTrailingZeros(i);
		uint64_t possibility = DiagMask(location) & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves->emplace_back(
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

void ChessEngine::PossibleR(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t r, MoveType type) const {
	auto i = r & ~(r - 1);

	while(i != 0) {
		int location = NumberOfTrailingZeros(i);
		uint64_t possibility = StraightMask(location) & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves->emplace_back(
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

void ChessEngine::PossibleQ(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t q) const {
	auto i = q & ~(q - 1);

	while(i != 0) {
		int location = NumberOfTrailingZeros(i);
		uint64_t possibility = (StraightMask(location) | DiagMask(location)) & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves->emplace_back(
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

void ChessEngine::PossibleK(std::vector<Move>* moves, uint64_t notMyPieces, uint64_t k, MoveType type) {
	auto i = k & ~(k - 1);

	while(i != 0) {
		const auto location = NumberOfTrailingZeros(i);
		uint64_t possibility = KingMoves[location] & notMyPieces;
		uint64_t j = possibility & ~(possibility - 1);

		while(j != 0) {
			const auto index = NumberOfTrailingZeros(j);

			moves->emplace_back(
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

void ChessEngine::PossibleWC(std::vector<Move>* moves) {
	if(unsafeForWhite & White & K) {
		return; // King is in check
	}

	if(CastleWK) {
		if(!(White & R & 1))
			CastleWK = false; // Rook has been hit
		else if(!((occupied | unsafeForWhite) & 0b110)) {
			moves->emplace_back(4, 7, 6, 7, MoveType::WhiteCastle);
		}
	}

	if(CastleWQ) {
		if(!(White & R & (1ULL << 7)))
			CastleWQ = false; // Rook has been hit
		else if((occupied & 0b01110000) == 0 && (unsafeForWhite & 0b00110000) == 0) {
			moves->emplace_back(4, 7, 2, 7, MoveType::WhiteCastle);
		}
	}
}

void ChessEngine::PossibleBC(std::vector<Move>* moves) {
	if(unsafeForBlack & Black & K) {
		return; // King is in check
	}

	if(CastleBK) {
		if((Black & R & (1ULL << 56)) == 0) {
			CastleBK = false; // Rook has been hit
		} else if(((occupied | unsafeForBlack) & (0b0110ULL << 56)) == 0) {
			moves->emplace_back(4, 0, 6, 0, MoveType::BlackCastle);
		}
	}

	if(CastleBQ) {
		if((Black & R & (1ULL << 63)) == 0) {
			CastleBQ = false; // Rook has been hit
		} else if((occupied & (0b0111ULL << 60)) == 0 && (unsafeForBlack & (0b0011ULL << 60)) == 0)
			moves->emplace_back(4, 0, 2, 0, MoveType::BlackCastle);
	}
}

unsigned long long ChessEngine::StraightMask(int s) const {
	const auto mag = rookTbl[s];
	return rookAttacks[s][((occupied & mag.mask) * mag.magic) >> (64 - 12)];
}
unsigned long long ChessEngine::DiagMask(int s) const {
	const auto mag = bishopTbl[s];
	return bishopAttacks[s][((occupied & mag.mask) * mag.magic) >> (64 - 9)];
}

unsigned long long ChessEngine::UnsafeForBlack() const {
	uint64_t res;

	#pragma region Pawn
	res = ((White & P) << 7) & ~FileA;
	res |= ((White & P) << 9) & ~FileH;
	#pragma endregion

	#pragma region Knight
	{
		auto wn = White & N;
		auto i = wn & ~(wn - 1);

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
		auto i = qb & ~(qb - 1);

		while(i != 0) {
			res |= DiagMask(NumberOfTrailingZeros(i));

			qb &= ~i;
			i = qb & ~(qb - 1);
		}
	}
	#pragma endregion

	#pragma region Rook / Queen
	{
		auto qr = White & (R | Q);
		auto i = qr & ~(qr - 1);

		while(i != 0) {
			res |= StraightMask(NumberOfTrailingZeros(i));

			qr &= ~i;
			i = qr & ~(qr - 1);
		}
	}
	#pragma endregion

	#pragma region King
	{
		auto wk = White & K;
		auto i = wk & ~(wk - 1);

		while(i != 0) {
			res |= KingMoves[NumberOfTrailingZeros(i)];

			wk &= ~i;
			i = wk & ~(wk - 1);
		}
	}
	#pragma endregion

	return res;
}
unsigned long long ChessEngine::UnsafeForWhite() const {
	uint64_t res;

	#pragma region Pawn
	res = ((Black & P) >> 7) & ~FileH;
	res |= ((Black & P) >> 9) & ~FileA;
	#pragma endregion

	#pragma region Knight
	{
		auto bn = Black & N;
		auto i = bn & ~(bn - 1);

		while(i != 0) {
			res |= KnightMoves[NumberOfTrailingZeros(i)];

			bn &= ~i;
			i = bn & ~(bn - 1);
		}
	}
	#pragma endregion

	#pragma region Bishop / Queen
	{
		auto qb = Black & (B | Q);
		auto i = qb & ~(qb - 1);

		while(i != 0) {
			res |= DiagMask(NumberOfTrailingZeros(i));

			qb &= ~i;
			i = qb & ~(qb - 1);
		}
	}
	#pragma endregion

	#pragma region Rook / Queen
	{
		auto qr = Black & (R | Q);
		auto i = qr & ~(qr - 1);

		while(i != 0) {
			res |= StraightMask(NumberOfTrailingZeros(i));

			qr &= ~i;
			i = qr & ~(qr - 1);
		}
	}
	#pragma endregion

	#pragma region King
	{
		auto bk = Black & K;
		auto i = bk & ~(bk - 1);

		while(i != 0) {
			res |= KingMoves[NumberOfTrailingZeros(i)];

			bk &= ~i;
			i = bk & ~(bk - 1);
		}
	}
	#pragma endregion

	return res;
}
