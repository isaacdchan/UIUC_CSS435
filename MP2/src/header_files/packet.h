#pragma once

#include "imports.h"
#include "node.h"

struct Packet
{
	const static int nodeIdSize = sizeof(short int);
	Node* node;
	string op;
	Resident* dest;
	Resident* src;
	int bytesRecvd;
	char* rawPacket;

	Packet(Node* node, int srcID, int bytesRecvd, char* rawPacket);

	short int extractOrigin();
	Resident* findNewNextHop();
	
	void checkIfPathUpdateAffectsOtherPaths(int dest);

	void handleSendOP();
	void handlePathOP();
	void handleCostOP();
};
