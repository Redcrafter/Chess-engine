#pragma once
#include "Player.h"

namespace Players {
	class OppositeColor: public Player {

	public:
		Move MakeMove(ChessEngine& game) override;
	};
}
