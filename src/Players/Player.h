#pragma once
#include "../Engine/Move.h"
#include "../Engine/ChessEngine.h"
#include "../Platform.h"

#include <iostream>

namespace Players {
	class Player {
	public:
		double Rating = 1000;
		// double EnemySum = 0;
		// int Games = 0;
		virtual Move MakeMove(ChessEngine& game) { return {}; };
	};
}
