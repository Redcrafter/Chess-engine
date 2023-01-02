#include "SameColor.h"

Move Players::SameColor::MakeMove(ChessEngine& game) {
	return bestMove(game, [=](const Move& m) {
		auto fromWhite = m.X0 % 2 == m.Y0 % 2;
		auto toWhite = m.X1 % 2 == m.Y1 % 2;

		if(game.WhiteMove) {
			return !fromWhite + toWhite * 2;
		} else {
			return fromWhite + !toWhite * 2;
		}
	});
}
