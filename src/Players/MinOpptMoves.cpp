#include "MinOpptMoves.h"

Move Players::MinOpptMoves::MakeMove(ChessEngine& game) {
	return bestMove(game, [=](const Move& m) {
		auto cp = game;
		cp.MakeMove(m);

		int count = 0;

		for(auto cmove : cp.GetMoves()) {
			auto cpp = cp;
			cpp.MakeMove(cmove);

			if(cpp.IsValid()) {
				count--;
			}
		}

		return count;
	});
}
