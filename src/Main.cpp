#include "Engine/ChessEngine.h"
#include <algorithm>
#include <random>
#include "AllPlayers.h"
#include "Engine/Magic.h"
#include "Engine/Test.h"

int Play(Players::Player* white, Players::Player* black) {
	auto rnd = std::mt19937(std::random_device()());
start:
	auto game = ChessEngine();

	int count = 0;
	while(true) {
		if(count > 200) {
			goto start;
		}

		auto moves = game.GetMoves();
		if(moves->empty()) {
			break;
		}

		std::shuffle(moves->begin(), moves->end(), rnd);

		Move m;
		if(game.WhiteMove) {
			m = white->MakeMove(game);
		} else {
			m = black->MakeMove(game);
		}

		if(m.Type == MoveType::Error) {
			break; // assume no more moves left
		}

		auto cp = game;
		cp.MakeMove(m);

		if(cp.IsValid()) {
			game = cp;
			count++;
		} else {
			moves->pop_back();
		}
	}

	// white->Games++;
	// black->Games++;

	auto ratio = 1.0 / (1 + pow(10, (black->Rating - white->Rating) / 400));

	if(game.IsCheck()) {
		if(game.WhiteMove) {
			// Black wins
			white->Rating -= 32 * ratio;
			black->Rating += 32 * ratio;
			return -1;
		} else {
			// White wins
			white->Rating += 32 * (1 - ratio);
			black->Rating -= 32 * (1 - ratio);
			return 1;
		}
	}

	return 0;
}

static void RunElo() {
	Players::Player* players[] = {
		// new Players::Pacifist(),
		// new Players::Generous(),
		new Players::Huddle(),
		new Players::SameColor(),
		new Players::OppositeColor(),
		new Players::Random(),
		new Players::Swarm(),
		new Players::MinOpptMoves(),
		new Players::Negamax(),
	};

	for(int i = 0; i < 1000; ++i) {
		for(auto white : players) {
			for(auto black : players) {
				Play(white, black);
			}
		}

		for(auto& player : players) {
			// player->Rating = std::max(player->EnemySum / player->Games, 100.0);
			// player->EnemySum = player->Games = 0;

			printf("%f\n", player->Rating);
		}

		printf("Runde: %i\n", i + 1);
	}

	for(auto player : players) {
		printf("%f\n", player->Rating);
		delete player;
	}
}

int main() {
	/* auto a = Players::MinOpptMoves();
	auto b = Players::Negamax();

	int whiteWin = 0, blackWin = 0;
	for(int i = 0; i < 1000; ++i) {
		auto res = Play(&a, &b);

		if(res == 1) {
			whiteWin++;
		} else if(res == -1) {
			blackWin++;
		}

		printf("%i/%i\n", whiteWin, blackWin);
	}
	printf("%i/%i\n", whiteWin, blackWin);*/

	// RunElo();

	// MoveTest();
	// PerformanceTest();
	// CalcMagic();

	auto rnd = std::mt19937(std::random_device()());

	auto white = Players::Console();
	auto black = Players::Negamax();

	auto game = ChessEngine();

	while(true) {
		auto moves = game.GetMoves();
		if(moves->empty()) {
			break; // no more moves avalible
		}

		std::shuffle(moves->begin(), moves->end(), rnd);

		Move m;
		if(game.WhiteMove) {
			m = white.MakeMove(game);
		} else {
			m = black.MakeMove(game);
		}

		if(m.Type == MoveType::Error) {
			break; // assume no more moves left
		}

		auto cp = game;
		cp.MakeMove(m);

		if(cp.IsValid()) {
			game = cp;

			if(!(game.WhiteMove)) {
				std::cout << "Enemy" << m << std::endl;
			}
		} else {
			// remove move from list
			moves->erase(std::remove(moves->begin(), moves->end(), m));

			if(game.WhiteMove) {
				std::cout << "Invalid move" << std::endl;
			}
		}
	}

	if(game.IsCheck()) {
		if(game.WhiteMove) {
			// Black wins
			printf("Enemy wins");
			return -1;
		} else {
			// White wins
			printf("Player wins");
		}
	} else {
		printf("Draw");
	}

	return 0;
}
