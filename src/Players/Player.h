#pragma once
#include "../Engine/ChessEngine.h"
#include <climits>

template <typename F>
auto bestMove(ChessEngine& game, F&& keyFunc) {
	Move best{};
	int score = INT_MIN;

	auto moves = game.GetMoves();
	
	for(auto move : moves) {
		auto cp = game;
		cp.MakeMove(move);
		if(!cp.IsValid()) continue;

		int res = keyFunc(move);
		if(res > score) {
			score = res;
			best = move;
		}
	}
	
	return best;
}

namespace Players {
	class Player {
	public:
		virtual ~Player() {}
		virtual Move MakeMove(ChessEngine& game) { return {}; };
	};
}
