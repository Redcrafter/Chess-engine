#pragma once
#include "Player.h"

namespace Players {
	class Random : public Player {
	public:
		Move MakeMove(ChessEngine& game) override;
	};
}
