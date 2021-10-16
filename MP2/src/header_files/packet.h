#pragma once

#include "imports.h"
#include "node.h"

struct Packet
{
	int src; // who originated the message (can be manager)
	Node* node;
	string op;
	short int dest;
	int bytesRecvd;
	unsigned char* rawPacket;


	Packet(int src, Node* node, int bytesRecvd, unsigned char* rawPacket);
	void handleSendOP();
	void handlePathOP();
	void handleCostOP();
};
