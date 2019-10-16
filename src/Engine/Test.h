#pragma once
#include <string>

struct Testcase {
	std::string fen;
	int depth;
	int count;
	std::string name;
};

void RunTests();
void MoveTest();
void PerformanceTest();
