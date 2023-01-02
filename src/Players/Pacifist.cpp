#include "Pacifist.h"

#include "Platform.h"

enum Type {
	Checkmate,
	Check,
	CaptureQueen,
	CaptureRook,
	CaptureBN,
	CapturePawn
};

Move Players::Pacifist::MakeMove(ChessEngine& game) {
	auto mask = game.WhiteMove ? game.Black : game.White;

	int qCount = popcnt64(mask & game.Q);
	int rCount = popcnt64(mask & game.R);
	int bnCount = popcnt64(mask & (game.B | game.N));
	int pCount = popcnt64(mask & game.P);

	return bestMove(game, [=](const Move& m) {
		auto cp = game;
		cp.MakeMove(m);

		if(cp.IsCheck()) {
			if(cp.IsCheckmate()) {
				return 0;
			}
			return 1;
		}
		if(qCount - popcnt64(mask & cp.Q)) return 2;
		if(rCount - popcnt64(mask & cp.R)) return 3;
		if(bnCount - popcnt64(mask & (cp.B | cp.N))) return 4;
		if(pCount - popcnt64(mask & cp.P)) return 5;
		return 6; // Best move
	});
}
