#pragma once
#include "imports.h"

struct Logger {
	int id;
	stringstream ss;
	ofstream file;

	Logger(int testId, string logFile);
	Logger();
	void add();
	void addCostUpdate(int otherNode, int updatedCost);
	void addEdgeExpired(int otherNode);
	void addRecv(int othernode, string msg);
};