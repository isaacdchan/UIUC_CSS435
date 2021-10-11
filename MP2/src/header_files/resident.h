#pragma once

#include "imports.h"
#include "node.h"

#define EXPIRATION_THRESHOLD 100

struct Resident
{
	int id;
	int udpSocket;
	timeval lastHeartbeat;
	sockaddr_in sockaddr;
	string path;
	int pathCost;
	bool edgeIsActive;
	int edgeCost;

	Resident(int _id, int _udpSocket);
	Resident();

	bool checkHealth(timeval currTime);
	void recordHeartbeat();
	void send(const char* buf, int length);
};