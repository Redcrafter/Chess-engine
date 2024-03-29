#include "Test.h"
#include "ChessEngine.h"

#include <chrono>
#include <iostream>
#include <unordered_set>
#include <vector>

static Testcase data[] = {
	{"3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 6, 1134888, "Illegal ep move #1"},
	{"8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 6, 1015133, "Illegal ep move #2"},
	{"8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 6, 1440467, "EP Capture Checks Opponent"},
	{"5k2/8/8/8/8/8/8/4K2R w K - 0 1", 6, 661072, "Short Castling Gives Check"},
	{"3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 6, 803711, "Long Castling Gives Check"},
	{"r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 4, 1274206, "Castle Rights"},
	{"r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 4, 1720476, "Castling Prevented"},
	{"2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 6, 3821001, "Promote out of Check"},
	{"8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 5, 1004658, "Discovered Check"},
	{"4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 6, 217342, "Promote to give check"},
	{"8/P1k5/K7/8/8/8/8/8 w - - 0 1", 6, 92683, "Under Promote to give check"},
	{"K1k5/8/P7/8/8/8/8/8 w - - 0 1", 6, 2217, "Self Stalemate"},
	{"8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 7, 567584, "Stalemate & Checkmate"},
	{"8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 4, 23527, "Stalemate & Checkmate"}
};

struct PerftDat {
	uint32_t total = 0;
	uint32_t valid = 0;

	uint32_t endStates = 0;

	void operator +=(const PerftDat& other) {
		total += other.total;
		valid += other.valid;
		endStates += other.endStates;
	}
};

static PerftDat Perft(ChessEngine g, int depth) {
	PerftDat ret{};

	const auto& moves = g.GetMoves();
	ret.total += moves.size();

	if(depth == 1) {
		for(auto& move : moves) {
			auto c = g;
			c.MakeMove(move);

			if(c.IsValid()) {
				ret.valid++;
				ret.endStates++;
			}
		}
	} else {
		for(auto& move : moves) {
			auto c = g;
			c.MakeMove(move);

			if(c.IsValid()) {
				ret.valid++;
				ret += Perft(c, depth - 1);
			}
		}
	}

	return ret;
}

void MoveTest() {
	for(auto testcase : data) {
		auto g = ChessEngine(testcase.fen);

		auto count = Perft(g, testcase.depth);

		if(count.endStates != testcase.count) {
			std::cout << "\033[31m[Failed] ";
		} else {
			std::cout << "\033[32m[Passed] ";
		}
		std::cout << testcase.name << " Acc: " << (count.valid / (float)count.total) * 100 << "%\n";
	}

	std::cout << "\033[0m";
}

void PerformanceTest(int depth) {
	// auto g = ChessEngine("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
	// auto g = ChessEngine("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
	auto g = ChessEngine();
	auto begin = std::chrono::high_resolution_clock::now();

#if false
	PerftDat sum = Perft(g, depth);
#else
	PerftDat sum {};

	std::vector<ChessEngine> boards;

	auto moves = g.GetMoves();
	sum.total += moves.size();
	
	for(auto& move : moves) {
		auto c = g;
		c.MakeMove(move);

		if(!c.IsValid()) 
			continue;

		sum.valid++;

		auto m1 = c.GetMoves();
		sum.total += m1.size();

		for(auto& move1 : m1) {
			auto c1 = c;
			c1.MakeMove(move1);

			if(c1.IsValid()) {
				sum.valid++;
				boards.emplace_back(c1);
			}
		}
	}

#pragma omp parallel for
	for(int i = 0; i < boards.size(); i++) {
		PerftDat sub = Perft(boards[i], depth - 2);

#pragma omp critical
		sum += sub;
	}
#endif

	auto end = std::chrono::high_resolution_clock::now();
	auto passed = std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count();

	std::cout << "Evaluated " << sum.endStates << " moves in " << passed << "s = " << (int)(sum.endStates / passed) << "/s\n";
	// std::cout << "Evaluated " << sum.total << " moves in " << passed << "s = " << (int)(sum.total / passed) << "/s\n";
	std::cout << "Accuracy: " << (sum.valid / (float)sum.total) * 100 << "%" << std::endl;
}
