#pragma once

#include "imports.h"
#include "resident.h"
#include "packet.h"
#include "logger.h"


struct Node
{
	int id;
	static const int numResidents = 256;
	int udpSocket;
	Logger* logger;
	Resident* dir[numResidents];

	// init
	Node(int id, string costsFile, string logFile);
	void initDir(int udpSocket);
	void initCosts(string costsFile);
	void initLogFile(string logFile);

	// send
	void broadcast(const char* buf, int length);
	void broadcastHeartbeat();
	void broadcastEdges();
	void broadcastUpdatedPath(int dest);

	// recv
	void monitorResidentsHealth();
	void listenForMessages();
	void updatePath(int dest, int nextHop, int candidatePathCost);
};
