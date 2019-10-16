#include "Swarm.h"

Move Players::Swarm::MakeMove(ChessEngine& game) {
	auto moves = *game.GetMoves();

	int bestDistDiff = INT32_MIN;
	Move best;

	auto kingBit = game.K & (game.WhiteMove ? game.Black : game.White);
	int pos = NumberOfTrailingZeros(kingBit);
	auto kingX = 7 - pos % 8;
	auto kingY = 7 - pos / 8;
	
	for(Move move : moves) {
		auto lastDist = abs(move.X0 - kingX) + abs(move.Y0 - kingY);
		auto nowDist = abs(move.X1 - kingX) + abs(move.Y1 - kingY);

		auto distDiff = lastDist - nowDist;
		if (distDiff > bestDistDiff) {
			bestDistDiff = distDiff;
			best = move;
		}
	}

	return best;
}
