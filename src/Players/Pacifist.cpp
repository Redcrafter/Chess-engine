#include "Pacifist.h"

enum Type {
	Checkmate,
	Check,
	CaptureQueen,
	CaptureRook,
	CaptureBN,
	CapturePawn
};

Move Players::Pacifist::MakeMove(ChessEngine& game) {
	auto moves = game.GetMoves();

	Move best;
	int bestScore = -1;

	int mask = game.WhiteMove ? game.Black : game.White;

	const int qCount = popcnt64(mask & game.Q);
	const int rCount = popcnt64(mask & game.R);
	const int bnCount = popcnt64(mask & (game.B | game.N));
	const int pCount = popcnt64(mask & game.P);

	for(Move move : moves) {
		auto cp = game;
		cp.MakeMove(move);

		if(cp.IsValid()) {
			int score = -1;

			if(cp.IsCheck()) {
				if(cp.IsCheckmate()) {
					score = Checkmate;
				} else {
					score = Check;
				}
			} else {
				if(qCount - popcnt64(mask & cp.Q)) {
					score = CaptureQueen;
				} else if(rCount - popcnt64(mask & cp.R)) {
					score = CaptureRook;
				} else if(bnCount - popcnt64(mask & (cp.B | cp.N))) {
					score = CaptureBN;
				} else if(pCount - popcnt64(mask & cp.P)) {
					score = CapturePawn;
				} else {
					return move; // Best move
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
