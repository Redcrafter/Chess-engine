#include <algorithm>
#include <iostream>
#include <random>
#include <memory>

#include "AllPlayers.h"
#include "Engine/ChessConstants.h"
#include "Engine/ChessEngine.h"
#include "Engine/Test.h"

#include "Engine/Magic.h"

#if _WIN32 || _WIN64
#include <windows.h>
#endif

const char* engineName = "Dumb Engine";

static std::vector<std::string> split(const std::string& str, const std::string& delim) {
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do {
		pos = str.find(delim, prev);
		if(pos == std::string::npos)
			pos = str.length();

		auto token = str.substr(prev, pos - prev);
		if(!token.empty())
			tokens.push_back(token);

		prev = pos + delim.length();
	} while(pos < str.length() && prev < str.length());
	return tokens;
}

struct EloPlayer {
	std::unique_ptr<Players::Player> Player;

	float Rating = 1000;
	int Wins = 0;
	int Games = 0;
};

template<class T, class... Types>
EloPlayer makeElo(Types&&... args) {
	return EloPlayer{ std::make_unique<T>(std::forward<Types>(args)...) };
}

int Play(EloPlayer& white, EloPlayer& black) {
	auto game = ChessEngine();

	int count = 0;

	while(true) {
		if(count > 200) {
			break;
		}

		auto move = (game.WhiteMove ? white : black).Player->MakeMove(game);
		if(move.Type == MoveType::Error) {
			break; // no moves left
		}
		game.MakeMove(move);
		count++;
	}

	// white->Games++;
	// black->Games++;

	auto chance = 1.0 / (1 + pow(10, (black.Rating - white.Rating) / 400));
	const int K = 32;

	white.Games++;
	black.Games++;

	if(game.IsCheck()) {
		if(game.WhiteMove) {
			// Black wins
			white.Rating -= K * chance;
			black.Rating += K * chance;
			black.Wins++;
			return -1;
		} else {
			// White wins
			white.Rating += K * (1 - chance);
			black.Rating -= K * (1 - chance);
			white.Wins++;
			return 1;
		}
	} else {
		// draw
		white.Rating += 8 * (0.5 - chance);
		black.Rating += 8 * (chance - 0.5);

		return 0;
	}
}

static void RunElo() {
	EloPlayer players[] = {
		makeElo<Players::Pacifist>(),
		makeElo<Players::Generous>(),
		makeElo<Players::Huddle>(),
		makeElo<Players::SameColor>(),
		makeElo<Players::OppositeColor>(),
		makeElo<Players::Random>(),
		makeElo<Players::Swarm>(),

		// makeElo<Players::MinOpptMoves>(),
		// makeElo<Players::Negamax>(3),
	};

	for(int i = 0; i < 1000; ++i) {
		for(auto& white : players) {
			for(auto& black : players) {
				if(&white == &black) continue;

				Play(white, black);
			}
		}

		for(auto& player : players) {
			printf("%5.0f %4.1f\n", player.Rating, (player.Wins / (float)player.Games) * 100);
		}

		printf("Round: %i\n", i + 1);
	}

	for(auto& player : players) {
		printf("%f\n", player.Rating);
	}
}

void uci() {
	std::string line;
	ChessEngine game;

	// TODO: remove
	game.print();

	auto player = Players::Negamax();

	while(true) {
		getline(std::cin, line);
		auto tokens = split(line, " ");

		if(tokens[0] == "uci") {
			std::cout
				<< "id name " << engineName << std::endl
				<< "id author Redcrafter" << std::endl
				<< "uciok" << std::endl;
		} else if(tokens[0] == "isready") {
			std::cout << "readyok" << std::endl;
		} else if(tokens[0] == "setoption") {
			// no options supported yet
		} else if(tokens[0] == "ucinewgame") {
			// nothing to do
		} else if(tokens[0] == "position") {
			if(tokens[1] == "fen") {
				game = ChessEngine(line.substr(13));

			} else if(tokens[1] == "startpos") {
				game = ChessEngine(); // default constructor uses startpos
			} else {
				throw std::logic_error("bad command");
			}

			if(line.find("moves") != std::string::npos) {
				line = line.substr(line.find("moves") + 6);
				auto gameMoves = split(line, " ");

				for(int i = 0; i < gameMoves.size(); ++i) {
					auto moveString = gameMoves[i];

					Move m(moveString, MoveType::Error);

					// find right move type
					for(Move move : game.GetMoves()) {
						if(move.X0 == m.X0 && move.Y0 == m.Y0 && move.X1 == m.X1 && move.Y1 == m.Y1) {
							m = move;
							break;
						}
					}

					if(moveString.length() == 5) {
						switch(moveString[4]) {
							case 'n': m.Type = MoveType::PromotionN; break;
							case 'r': m.Type = MoveType::PromotionR; break;
							case 'b': m.Type = MoveType::PromotionB; break;
							case 'q': m.Type = MoveType::PromotionQ; break;
							default: throw std::logic_error("bad move format");
						}
					}

					game.MakeMove(m);
				}
			}
		} else if(tokens[0] == "go") {
			auto move = player.MakeMove(game);

			std::cout << "bestmove " << move << std::endl;
		} else if(tokens[0] == "quit") {
			return;
		}
	}
}

void PlayConsole() {
	bool playerWhite;

	while(true) {
		std::cout << "Do you want to play as white? (y/n)";
		char c;
		std::cin >> c;
		c = tolower(c);

		if(c == 'y') {
			playerWhite = true;
			break;
		}
		if(c == 'n') {
			playerWhite = false;
			break;
		}
	}

	std::unique_ptr<Players::Player> white = std::make_unique<Players::Console>();
	std::unique_ptr<Players::Player> black = std::make_unique<Players::Negamax>();

	if(!playerWhite) {
		std::swap(white, black);
	}

	auto game = ChessEngine();

	while(true) {
		auto move = (game.WhiteMove ? white : black)->MakeMove(game);
		if(move.Type == MoveType::Error) {
			break; // no more moves available
		}

		if(game.WhiteMove == playerWhite) {
			// __debugbreak();
			std::cout << "Computer: " << move << std::endl;
		}
		
	}

	if(game.IsCheck()) {
		if(game.WhiteMove) {
			// Black wins
			printf("Enemy wins");
		} else {
			// White wins
			printf("Player wins");
		}
	} else {
		printf("Draw");
	}
}

int main(int argc, char* argv[]) {
	#if _WIN32 || _WIN64
	SetConsoleOutputCP(65001);
	#endif

	CalcMagic();

	if(argc > 1) {
		std::string val = argv[1];

		if(val == "uci") {
			uci();
		} else if(val == "test") {
			MoveTest();
		} else if(val == "perf") {
			int count = 6;
			if(argc > 2) {
				count = std::atoi(argv[2]);
			}
			PerformanceTest(count);
		} else if(val == "play") {
			PlayConsole();
		} else {
			std::cout << "Unknown command" << std::endl;
		}
	} else {
		std::cout
			<< "Missing command parameter" << std::endl
			<< "Possible options are" << std::endl
			<< "play:	play normally against the engine" << std::endl
			<< "test:	run engine tests" << std::endl
			<< "perf:	run performance test" << std::endl
			<< "uci:	enter uci mode" << std::endl;
	}

	return 0;
}
