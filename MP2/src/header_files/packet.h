#pragma once

#include "imports.h"
#include "node.h"

struct Packet
{
	Node* node;

	string op;
	Resident* dest;
	Resident* src;
	short int origin;

	int bytesRecvd;
	bool fromManager;
	char* rawPacket;
	string message;

	int sendHeader_size = op_size + nodeID_size;
	int forwardHeader_size = op_size + nodeID_size + nodeID_size;

	Packet(Node* node, int srcID, int bytesRecvd, char* rawPacket);

	void handleSendOP();
	void handlePathOP();
	void handleCostOP();

	// packet_buf_utils
	void extractMessage();
	void extractOrigin();
	char* constructSendPacket();
};
