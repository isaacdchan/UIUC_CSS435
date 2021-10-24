#include "header_files/node.h"

void Node::broadcast(char* buf, int length)
{
	for(int i=0; i<numResidents; i++) {
		if(i != id)
		{
			dir[i]->send(buf, length);
		}
	}
}

void Node::broadcastHeartbeat()
{
	struct timespec sleepFor;
	sleepFor.tv_sec = 0;
	sleepFor.tv_nsec = 100 * 1000 * 1000; //100 ms
	while(1)
	{
		for (int i=0; i<numResidents; i++)
		{
			if (dir[i]->pathCost != INT_MAX) { broadcastPathCost(i); }
		}
		nanosleep(&sleepFor, 0);
	}
}

void Node::broadcastPathCost(int dest)
{
	if (dir[dest]->pathCost < 0 || dir[dest]-> pathCost > 10000) {
		logger->ss << "OH SHIT: " << dest << " | " << dir[dest]->pathCost;
		logger->add();
	}
	int no_cost = htonl(dir[dest]->pathCost);
	short int no_dest = htons(dest);

	char sendBuf[4+sizeof(short int)+sizeof(int)];

	strcpy(sendBuf, "path");
	memcpy(sendBuf+4, &no_dest, 2);
	memcpy(sendBuf+6, &no_cost, 4);
	
	broadcast(sendBuf, 10);
}
 