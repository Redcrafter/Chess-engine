#pragma once
#include "Player.h"

namespace Players {
	class Generous : public Player {

	public:
		Move MakeMove(ChessEngine& game) override;
	};
}
