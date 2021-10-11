#pragma once

#include "imports.h"
#include "resident.h"
#include "packet.h"
#include "logger.h"

#include "file_utils.cpp"


struct Node
{
	int id;
	Logger logger;
	int udpSocket;
	Resident dir[256];

	// init
	Node(int id, string costsFile, string logFile);
	void initDir(int udpSocket);
	void initCosts(string costsFile);
	void initLogFile(string logFile);

	// send
	void broadcast(const char* buf, int length);
	void broadcastHeartbeat();
	void advertiseEdgeCosts();

	// recv
	void monitorResidentsHealth();
	void listenForMessages();
};
