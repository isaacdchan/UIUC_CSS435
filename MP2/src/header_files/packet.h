#pragma once

#include "imports.h"
#include "resident.h"
#include "logger.h"

struct Packet
{
	Resident* owner;
	Logger* logger;
	string op;
	int src;
	int dest;
	int nextHop;
	string contents;

	Packet(Resident* _owner, Logger* _logger, unsigned char* recvBuf);
	Packet();
	void execute();
};
