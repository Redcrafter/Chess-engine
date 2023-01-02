#include "Swarm.h"
#include "Platform.h"

Move Players::Swarm::MakeMove(ChessEngine& game) {
	auto kingBit = game.K & (game.WhiteMove ? game.Black : game.White);
	int pos = NumberOfTrailingZeros(kingBit);
	auto kingX = 7 - pos % 8;
	auto kingY = 7 - pos / 8;

	return bestMove(game, [=](const Move& m) {
		auto lastDist = abs(m.X0 - kingX) + abs(m.Y0 - kingY);
		auto nowDist = abs(m.X1 - kingX) + abs(m.Y1 - kingY);

		return lastDist - nowDist;
	});
}
