#pragma once
#include "Player.h"

namespace Players {
	class Swarm: public Player {

	public:
		Move MakeMove(ChessEngine& game) override;
	};
}
