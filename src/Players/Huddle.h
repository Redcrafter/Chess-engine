#pragma once
#include "Player.h"

namespace Players {
	class Huddle: public Player {

	public:
		Move MakeMove(ChessEngine& game) override;
	};
}
