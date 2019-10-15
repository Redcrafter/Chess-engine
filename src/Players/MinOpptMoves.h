#pragma once
#include "Player.h"

namespace Players {
	class MinOpptMoves : public Player {

	public:
		Move MakeMove(ChessEngine& game) override;
	};
}
