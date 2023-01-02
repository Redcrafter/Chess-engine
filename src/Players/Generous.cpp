#include "Generous.h"
#include "Platform.h"

Move Players::Generous::MakeMove(ChessEngine& game) {
	auto mask = game.WhiteMove ? game.White : game.Black;
	const int qCount = popcnt64(mask & game.Q);
	const int rCount = popcnt64(mask & game.R);
	const int bnCount = popcnt64(mask & (game.B | game.N));
	const int pCount = popcnt64(mask & game.P);

	return bestMove(game, [=](const Move& m) {
		auto cp = game;
		cp.MakeMove(m);

		int score = 0;
		for(auto& cmove : cp.GetMoves()) {
			auto cpp = cp;
			cpp.MakeMove(cmove);

			if(!cpp.IsValid()) continue;

			score += (qCount > popcnt64(mask & cp.Q)) * 9;
			score += (rCount > popcnt64(mask & cp.R)) * 5;
			score += (bnCount > popcnt64(mask & (cp.B | cp.N))) * 3;
			score += (pCount > popcnt64(mask & cp.P)) * 1;
		}

		return score;
	});
}
