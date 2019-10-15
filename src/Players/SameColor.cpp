#include "SameColor.h"

Move Players::SameColor::MakeMove(ChessEngine& game) {
	auto moves = *game.GetMoves();

	for(Move move : moves) {
		auto fromWhite = move.X0 % 2 == move.Y0 % 2;
		auto toWhite = move.X1 % 2 == move.Y1 % 2;

		if(game.Flags & WhiteMove) {
			if(!fromWhite && toWhite) {
				return move;
			}
		} else {
			if(fromWhite && !toWhite) {
				return move;
			}
		}
	}

	return moves[rand() % moves.size()];
}
