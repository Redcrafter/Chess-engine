#include <windows.h>
#include <utility>
#include <ctime>

#include "Test.h"
#include "ChessEngine.h"
#include <thread>
#include <future>

static Testcase data[] = {
	Testcase("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 6, 1134888, "Illegal ep move #1"),
	Testcase("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 6, 1015133, "Illegal ep move #2"),
	Testcase("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 6, 1440467, "EP Capture Checks Opponent"),
	Testcase("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 6, 661072, "Short Castling Gives Check"),
	Testcase("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 6, 803711, "Long Castling Gives Check"),
	Testcase("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 4, 1274206, "Castle Rights"),
	Testcase("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 4, 1720476, "Castling Prevented"),
	Testcase("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 6, 3821001, "Promote out of Check"),
	Testcase("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 5, 1004658, "Discovered Check"),
	Testcase("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 6, 217342, "Promote to give check"),
	Testcase("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 6, 92683, "Under Promote to give check"),
	Testcase("K1k5/8/P7/8/8/8/8/8 w - - 0 1", 6, 2217, "Self Stalemate"),
	Testcase("8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 7, 567584, "Stalemate & Checkmate"),
	Testcase("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 4, 23527, "Stalemate & Checkmate")
};

Testcase::Testcase(std::string fen, int depth, int count, std::string name): fen(std::move(fen)), name(std::move(name)), depth(depth), count(count) {
}

static int invalid = 0;
static int Perft(ChessEngine g, int depth) {
	if (!depth) {
		return 1;
	}

	auto sum = 0;
	auto moves = g.GetMoves();
	for (auto move : *moves) {
		auto c = g;
		c.MakeMove(move);

		if (c.IsValid()) {
			sum += Perft(c, depth - 1);
		} else {
			invalid++;
		}
	}

	return sum;
}

void RunTests() {
	MoveTest();
	PerformanceTest();
}

void MoveTest() {
	auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	
	for (auto testcase : data) {
		invalid = 0;
		auto g = ChessEngine(testcase.fen);
		
		auto count = Perft(g, testcase.depth);

		if(count != testcase.count) {
			SetConsoleTextAttribute(hConsole, 4);
			std::cout << "[Failed] ";
		} else {
			SetConsoleTextAttribute(hConsole, 2);
			std::cout << "[Passed] ";
		}
		std::cout << testcase.name << "\nInvalid: " << invalid << "\n";
	}
	SetConsoleTextAttribute(hConsole, 7);
}

void PerformanceTest() {
	auto g = ChessEngine("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");

	auto begin = clock();

	auto sum = 0;
	auto moves = *g.GetMoves();
	std::vector<std::future<int>> threads;
	
	for (auto move : moves) {
		auto c = g;
		c.MakeMove(move);

		if (c.IsValid()) {
			threads.push_back(std::async(Perft, c, 5));
			/*
			auto s = Perft(c, 5);
			sum += s;

			printf("%f\n", sum / 706045033.0);*/
		}
	}

	for(auto &thread : threads) {
		sum += thread.get();
	}
	
	auto end = clock();
	auto passed = double(end - begin) / CLOCKS_PER_SEC;

	printf("%fs\n%fm/s\n%i\n", passed, sum / passed, sum);
}