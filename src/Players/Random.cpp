#include "Random.h"
#include <random>

Move Players::Random::MakeMove(ChessEngine& game) {
	auto moves = game.GetValidMoves();
	if(moves.size() == 0) return {};
	return moves[std::random_device()() % moves.size()];
}
