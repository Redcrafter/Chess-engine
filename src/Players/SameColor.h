#pragma once
#include "Player.h"

namespace Players {
	class SameColor: public Player {

	public:
		Move MakeMove(ChessEngine& game) override;
	};
}
