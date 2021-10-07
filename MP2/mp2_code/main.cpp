#include "imports.h"
#include "monitor_neighbors.h"
#include "file_utils.cpp"
#include "resident.cpp"


int globalMyID;
int globalSocketUDP;

Resident dir[256];

void broadcastEdgeCosts() {
	stringstream ss;
	string strID = to_string(globalMyID);
	for (int neighbor=0; neighbor<256; neighbor++)
	{
		if (dir[neighbor].edgeIsActive) // if neighbor
		{
			for (int resident=0; resident<256; resident++)
			{
				if (dir[resident].pathCost != -1) // there is a path to resident
				{
					ss << "route|" << globalMyID << "|" << resident << "|" << dir[resident].pathCost << strID;
					dir[neighbor].send(ss.str().c_str(), ss.str().size());
				}
			}
		}
	}

}
 
int main(int argc, char** argv)
{
	if(argc != 4)
	{
		fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
		exit(1);
	}
	
	
	globalMyID = atoi(argv[1]);
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
		if (i == globalMyID)
			dir[i].edgeCost = 1;
		else
			dir[i].edgeCost = 0;
	}
	
	// TODO: read and parse initial costs file. default to cost 1 if no entry for a node. file may be empty.
	vector<tuple<int, int>> initial_costs = parseCostsFile(argv[2]);
	for (tuple<int, int> tup : initial_costs)
	{
		dir[get<0>(tup)].edgeCost = get<1>(tup);
	}

	//socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
	if((globalSocketUDP=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	char myAddr[100];
	struct sockaddr_in bindAddr;
	sprintf(myAddr, "10.1.1.%d", globalMyID);	
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
	
	// constantly checks if a neighbor's most recent heartbeat is older than threshold
	pthread_t announcerThread;
	pthread_t listenerThread;
	pthread_t neighborHealthThread; 
	pthread_create(&announcerThread, 0, broadcastHeartbeat, (void*)0);
	pthread_create(&listenerThread, 0, listenForMessages, (void*)0);
	pthread_create(&neighborHealthThread, 0, monitorResidentsHealth, (void*)0);
}
