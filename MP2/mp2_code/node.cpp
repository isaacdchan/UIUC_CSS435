#include "node.h"

int globalSocketUDP;

void Node::initCosts(string costsFile)
{
	vector<tuple<int, int>> initial_costs = parseCostsFile(costsFile);
	for (tuple<int, int> tup : initial_costs)
	{
		dir[get<0>(tup)].edgeCost = get<1>(tup);
	}
}

void Node::initDir()
{
	for(int i=0; i<256; i++)
	{
		// init last heartbeat to curr time
		gettimeofday(&dir[i].lastHeartbeat, 0);
		
		char tempaddr[100];
		sprintf(tempaddr, "10.1.1.%d", i);
		memset(&dir[i].sockaddr, 0, sizeof(dir[i].sockaddr));
		dir[i].sockaddr.sin_family = AF_INET;
		dir[i].sockaddr.sin_port = htons(7777);
		inet_pton(AF_INET, tempaddr, &dir[i].sockaddr.sin_addr);
		if (i == id)
			dir[i].edgeCost = 1;
		else
			dir[i].edgeCost = 0;
	}
}

Node::Node(int _id, string costsFile)
{
	id = _id;
	initDir();
	initCosts(costsFile);
	//socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
	if((globalSocketUDP=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	char myAddr[100];
	struct sockaddr_in bindAddr;
	sprintf(myAddr, "10.1.1.%d", id);	
	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(7777);
	inet_pton(AF_INET, myAddr, &bindAddr.sin_addr);
	if(bind(globalSocketUDP, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind");
		close(globalSocketUDP);
		exit(1);
	}
}
