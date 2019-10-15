#include "MinOpptMoves.h"

Move Players::MinOpptMoves::MakeMove(ChessEngine& game) {
	Move best;
	int bestScore = -1;

	for(Move move : *game.GetMoves()) {
		auto cp = game;
		cp.MakeMove(move);

		if(!cp.IsValid()) 
			continue;

		int count = 0;

		for(auto cmove : *cp.GetMoves()) {
			auto cpp = cp;
			cpp.MakeMove(cmove);

			if(cpp.IsValid()) {
				count++;
			}
		}

		if(count > bestScore) {
			bestScore = count;
			best = move;
		}
	}

	return best;
}
