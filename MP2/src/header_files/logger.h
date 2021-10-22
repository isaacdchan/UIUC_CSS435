#pragma once
#include "imports.h"

struct Logger {
	int id;
	string logFile;
	stringstream ss;
	

	Logger(int testId, string logFile);
	void add();
	void addEdgeCostUpdate(int otherNode, int updatedCost);
	void addPathCostUpdate(int dest, int nextHop, int updatedCost);
	void addEdgeRevived(int otherNode);
	void addEdgeExpired(int otherNode);
	void addSend(int src, string msg);
	void addForward(int src, int dest, string msg);
	void addRecv(int dest, string msg);
	void addUnreachable(int src, int dest);
};