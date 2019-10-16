#include "Negamax.h"

static int eval(ChessEngine& g) {
	int score =
		9 * (__popcnt64(g.White & g.Q) - __popcnt64(g.Black & g.Q)) +
		5 * (__popcnt64(g.White & g.R) - __popcnt64(g.Black & g.R)) +
		3 * (__popcnt64(g.White & (g.B | g.N)) - __popcnt64(g.Black & (g.B | g.N))) +
		1 * (__popcnt64(g.White & g.P) - __popcnt64(g.Black & g.P));

	if(g.WhiteMove) {
		return score;
	} else {
		return -score;
	}
}

static int alphaBeta(ChessEngine& game, int alpha, int beta, int depth) {
	if(depth == 0) {
		return eval(game); // quiesce(alpha, beta);
	}

	auto moves = *game.GetMoves();
	auto valid = false;
	for(auto& move : moves) {
		auto cp = game;
		cp.MakeMove(move);

		if(!cp.IsValid()) {
			continue;
		}

		valid = true;
		auto score = -alphaBeta(cp, -beta, -alpha, depth - 1);
		if(score >= beta) {
			return beta;
		}
		if(score > alpha) {
			alpha = score;
		}
	}

	if(!valid) {
		if(game.IsCheck()) {
			return -100; // Checkmate
		} else {
			return 0; // Draw
		}
	}

	return alpha;
}

Players::Negamax::Negamax(int depth) : depth(depth - 1) { }

Move Players::Negamax::MakeMove(ChessEngine& game) {
	Move best;

	int alpha = -100000;
	int beta = 100000;

	auto moves = *game.GetMoves();
	auto valid = false;
	for(auto& move : moves) {
		auto cp = game;
		cp.MakeMove(move);

		if(!cp.IsValid()) {
			continue;
		}

		valid = true;
		auto score = -alphaBeta(cp, -beta, -alpha, depth);
		if(score > alpha) {
			alpha = score;
			best = move;
		}
	}

	/* if (!valid && game.IsCheck()) {
		return -200; // Checkmate
	} */

	return best;
}
