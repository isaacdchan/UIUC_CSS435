#include "node.h"

void Node::broadcast(const char* buf, int length)
{
	for(int i=0; i<256; i++) {
		if(i != id) {
			dir[i].send(buf, length);
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

void Node::advertiseEdgeCosts()
{
	stringstream ss;
	string strID = to_string(id);
	for (int neighbor=0; neighbor<256; neighbor++)
	{
		if (dir[neighbor].edgeIsActive) // if neighbor
		{
			for (int resident=0; resident<256; resident++)
			{
				if (dir[resident].pathCost != -1) // there is a path to resident
				{
					ss << "route|" << id << "|" << resident << "|" << dir[resident].pathCost << strID;
					dir[neighbor].send(ss.str().c_str(), ss.str().size());
				}
			}
		}
	}

}
 