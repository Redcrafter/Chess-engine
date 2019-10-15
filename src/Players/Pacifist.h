#pragma once
#include "Player.h"

namespace Players {
	class Pacifist : public Player {

	public:
		Move MakeMove(ChessEngine& game) override;
	};
}
