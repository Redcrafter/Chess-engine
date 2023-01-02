#pragma once
#include "Player.h"

namespace Players {
	class Negamax : public Player {

	public:
		Negamax(int depth = 4) : depth(depth) {}
		Move MakeMove(ChessEngine& game) override;
	private:
		int depth;
	};
}
