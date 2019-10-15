#pragma once
#include "Player.h"

namespace Players {
	class Console : public Player {

	public:
		Move MakeMove(ChessEngine& game) override;
	};
}
