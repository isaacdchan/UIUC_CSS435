#pragma once

#include "imports.h"
#include "node.h"

struct Packet
{
	int prevHop; // who originated the message (can be manager)
	const static int nodeIdSize = sizeof(short int);
	Node* node;
	string op;
	short int dest;
	int bytesRecvd;
	char* rawPacket;

	Packet(int prevHop, Node* node, int bytesRecvd, char* rawPacket);

	int extractSendSrc();
	int findNewNextHop();
	
	void checkIfPathUpdateAffectsOtherPaths(int dest);

	void handleSendOP();
	void handlePathOP();
	void handleCostOP();
};
