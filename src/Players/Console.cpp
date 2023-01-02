#include "Console.h"

#include <iostream>

Move Players::Console::MakeMove(ChessEngine& game) {
	auto moves = game.GetValidMoves();
	std::cout << game << std::endl;

	if(moves.empty()) return {};

	while(true) {
		std::cout << "Enter Move: ";

		std::string str;
		std::getline(std::cin, str);

		if(str.length() == 4) {
			const Move m(str, MoveType::Error);

			for(Move& move : moves) {
				if(move == m) {
					return move;
				}
			}
		}

		std::cout << "Invalid move" << std::endl;
	}
}
