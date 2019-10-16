#include "Generous.h"

Move Players::Generous::MakeMove(ChessEngine& game) {
	auto moves = *game.GetMoves();

	Move best;
	int bestScore = -1;

	int mask = game.WhiteMove ? game.White : game.Black;
	const int qCount = popcnt64(mask & game.Q);
	const int rCount = popcnt64(mask & game.R);
	const int bnCount = popcnt64(mask & (game.B | game.N));
	const int pCount = popcnt64(mask & game.P);

	for(Move move : moves) {
		auto cp = game;
		cp.MakeMove(move);

		if(cp.IsValid()) {
			int score = 0;

			for(auto cmove : *cp.GetMoves()) {
				auto cpp = cp;
				cpp.MakeMove(cmove);

				if(qCount - popcnt64(mask & cp.Q)) {
					score += 9;
				} else if(rCount - popcnt64(mask & cp.R)) {
					score += 5;
				} else if(bnCount - popcnt64(mask & (cp.B | cp.N))) {
					score += 3;
				} else if(pCount - popcnt64(mask & cp.P)) {
					score += 1;
				}
			}

			if(score > bestScore) {
				bestScore = score;
				best = move;
			}
		}
	}

	return best;
}
