#pragma once

#include "imports.h"
#include "node.h"

struct Packet
{
	Node* node;
	string op;
	Resident* dest;
	Resident* src;
	int bytesRecvd;
	char* rawPacket;

	int sendHeader_size = op_size + nodeID_size;
	int forwardHeader_size = op_size + nodeID_size + nodeID_size;

	Packet(Node* node, int srcID, int bytesRecvd, char* rawPacket);

	string extractMessage(int bytesRecvd, char* rawPacket, bool fromManager);
	short int extractOrigin();
	// short int extractOrigin();
	short int extractTTL();
	Resident* findNewNextHop();
	
	void checkIfPathUpdateAffectsOtherPaths(int dest);

	void handleSendOP();
	void handlePathOP();
	void handleCostOP();
};
