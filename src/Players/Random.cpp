#include "Random.h"

Move Players::Random::MakeMove(ChessEngine& game) {
	auto moves = game.GetMoves();
	return moves[rand() % moves.size()];
}