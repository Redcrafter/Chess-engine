#include "Console.h"

Move Players::Console::MakeMove(ChessEngine& game) {
	auto moves = game.GetMoves();
	std::cout << game << std::endl;

	while(true) {
		std::cout << "Enter Move: ";

		std::string str;
		std::getline(std::cin, str);

		if(str.length() == 4) {
			const Move m{str, MoveType::Error};

			for(Move& move : moves) {
				if(move.X0 == m.X0 && move.Y0 == m.Y0 && move.X1 == m.X1 && move.Y1 == m.Y1) {
					return move;
				}
			}
		}

		std::cout << "Invalid move" << std::endl;
	}
}
