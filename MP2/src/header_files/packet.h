#pragma once

#include "imports.h"
#include "node.h"

struct Packet
{
	int src; // who originated the message (can be manager)
	string op;
	short int dest;

	Packet(int src, Node* node, int bytesRecvd, unsigned char packet[]);
};
