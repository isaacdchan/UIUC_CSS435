#include "header_files/node.h"

void Node::broadcast(const char* buf, int length)
{
	for(int i=0; i<numResidents; i++) {
		if(i != id) {
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
		Node::broadcast("BABUMP", 7);
		nanosleep(&sleepFor, 0);
	}
}

void Node::advertisePathCosts()
{
	stringstream ss;
	string strID = to_string(id);
	short int no_dest;
	int no_newCost;
	// for each neighbor, advertise your edge costs to all other neighbors
	// advertise distance to self is 0
	for (int i=0; i<numResidents; i++)
	{
		short int no_dest = htons(i);
		no_newCost = htonl(dir[i]->edgeCost);
		char sendBuf[4+sizeof(short int)+sizeof(int)];

		strcpy(sendBuf, "path");
		memcpy(sendBuf+4, &no_dest, 2);
		memcpy(sendBuf+6, &no_newCost, 4);
		dir[i]->send(sendBuf, 10);
	}
}
 