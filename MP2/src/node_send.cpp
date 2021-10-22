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
	sleepFor.tv_nsec = 300 * 1000 * 1000; //300 ms
	while(1)
	{
		broadcastUpdatedPath(this->id);
		nanosleep(&sleepFor, 0);
	}
}

void Node::broadcastUpdatedPath(int dest)
{
	int no_newCost = htonl(dir[dest]->edgeCost);
	short int no_dest = htons(dest);

	char sendBuf[4+sizeof(short int)+sizeof(int)];

	strcpy(sendBuf, "path");
	memcpy(sendBuf+4, &no_dest, 2);
	memcpy(sendBuf+6, &no_newCost, 4);
	
	broadcast(sendBuf, 10);
}
 