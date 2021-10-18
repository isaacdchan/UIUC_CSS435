#pragma once

#include "imports.h"

#define EXPIRATION_THRESHOLD 600

struct Resident
{
	int id;
	int udpSocket;
	timeval lastHeartbeat;
	sockaddr_in sockaddr;
	// string path; 
	int pathCost;
	int nextHop;
	bool edgeIsActive;
	int edgeCost;

	Resident(int _id, int _udpSocket);
	Resident();

	bool checkHealth(timeval currTime);
	void recordHeartbeat();
	void send(unsigned char* buf, int length);
};