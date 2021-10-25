#pragma once
#include "imports.h"

struct Logger {
	int testID;
	bool verbose;
	string logFile;
	stringstream ss;
	

	Logger(int testID, string logFile, bool verbose);
	void add();
	void addEdgeCostUpdate(int otherNode, int updatedCost);
	void addPathCostUpdate(int dest, int nextHop, int updatedCost);
	void addEdgeRevived(int otherNode);
	void addEdgeExpired(int otherNode);
	void addSend(int nextHop, int dest, string msg);
	void addForward(int src, int nextHop, int dest, string msg);
	void addRecv(int dest, string msg);
	void addUnreachable(int src, int dest);
};