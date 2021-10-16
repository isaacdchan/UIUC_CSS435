#pragma once
#include "imports.h"

struct Logger {
	int id;
	stringstream ss;
	ofstream file;

	Logger(int testId, string logFile);
	Logger();
	void add();
	void addEdgeCostUpdate(int otherNode, int updatedCost);
	void addPathCostUpdate(int dest, int nextHop, int updatedCost);
	void addEdgeExpired(int otherNode);
	void addSend(int src, string msg);
	void addForward(int prevHop, int dest, string msg);
	void addRecv(int dest, string msg);
};