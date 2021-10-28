#pragma once

#include "imports.h"

struct Resident
{
	int id;
	int udpSocket;
	timeval lastHeartbeat;
	sockaddr_in sockaddr;
	// string path; 
	int pathCost;
	Resident* nextHop;
	bool edgeIsActive;
	int edgeCost;
	int costsToDests[MAX_RESIDENTS];

	Resident(int _id, int _udpSocket);
	Resident();

	bool checkHealth(timeval currTime);
	void recordHeartbeat();
	void send(char* buf, int length);
};