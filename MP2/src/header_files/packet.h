#pragma once

#include "imports.h"
#include "node.h"

struct Packet
{
	Node* node;
	string op;
	Resident* dest;
	Resident* src;
	bool fromManager;
	int bytesRecvd;
	short int origin;
	int TTL;
	char* rawPacket;

	int sendHeader_size = op_size + nodeID_size;
	int forwardHeader_size = op_size + nodeID_size + nodeID_size + TTL_size;

	Packet(Node* node, int srcID, int bytesRecvd, char* rawPacket);

	void handleSendOP();
	void handlePathOP();
	void handleCostOP();

	// packet_buf_utils
	string extractMessage();
	void extractOrigin();
	int extractTTL();
	char* constructSendPacket();
};
