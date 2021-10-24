#pragma once

#include "imports.h"
#include "resident.h"
#include "logger.h"


struct Node
{
	short int id;
	int udpSocket;
	Logger* logger;
	Resident* dir[MAX_RESIDENTS];

	// init
	Node(short int id, string costsFile, string logFile);
	~Node();
	void initDir(int udpSocket);
	void initCosts(string costsFile);
	void initLogFile(string logFile);

	// send
	void broadcast(char* buf, int length);
	void broadcastHeartbeat();
	void broadcastPathCost(Resident* dest);

	// recv
	void monitorResidentsHealth();
	void listenForMessages();
	void updatePath(Resident* dest, Resident* nextHop, int newPathCost);
	void findAltPath(Resident* dest);
};
