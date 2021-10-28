#include "header_files/node.h"

void Node::broadcast(char* buf, int length) {
	for (Resident* r: dir) {
		if(r->id != id)	{ r->send(buf, length);	}
	}
}

void Node::broadcastHeartbeat()
{
	struct timespec sleepFor;
	sleepFor.tv_sec = 0;
	sleepFor.tv_nsec = 300 * 1000 * 1000; //300 ms
	while(1) {
		for (Resident* r: dir) {
			if (r->pathCost != INT_MAX) { broadcastPathCost(r); }
		}
		nanosleep(&sleepFor, 0);
	}
}

void Node::broadcastPathCost(Resident* dest) {
	int no_cost = htonl(dest->pathCost);
	short int no_destID = htons(dest->id);

	char sendBuf[4+sizeof(short int)+sizeof(int)];

	strcpy(sendBuf, "path");
	memcpy(sendBuf+4, &no_destID, 2);
	memcpy(sendBuf+6, &no_cost, 4);
	
	broadcast(sendBuf, 10);
}
 