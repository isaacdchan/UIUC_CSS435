#include "imports.h"
#include "log_utils.cpp"
#include "packet.cpp"
#include "resident.cpp"

extern int globalMyID;
extern int globalSocketUDP;
extern Resident dir[256];


void broadcast(const char* buf, int length)
{
	for(int i=0; i<256; i++) {
		if(i != globalMyID) {
			dir[i].send(buf, length);
		}
	}
}

void* broadcastHeartbeat(void* unusedParam)
{
	struct timespec sleepFor;
	sleepFor.tv_sec = 0;
	sleepFor.tv_nsec = 300 * 1000 * 1000; //300 ms
	while(1)
	{
		broadcast("BABUMP", 7);
		nanosleep(&sleepFor, 0);
	}
}

void* monitorResidentsHealth(void* unusedParam)
{
	timeval lastHeartbeat, currTime;
	while (1) {
		gettimeofday(&currTime, 0);
		for (int i=0; i<256; i++)
			dir[1].checkHealth(currTime);
	}
}

void* listenForMessages(void* unusedParam)
{
	char fromAddr[100];
	struct sockaddr_in theirAddr;
	socklen_t theirAddrLen;
	unsigned char recvBuf[1000];

	int bytesRecvd;
	while(1)
	{
		theirAddrLen = sizeof(theirAddr);
		if ((bytesRecvd = recvfrom(globalSocketUDP, recvBuf, 1000 , 0, 
					(struct sockaddr*)&theirAddr, &theirAddrLen)) == -1)
		{
			perror("connectivity listener: recvfrom failed");
			exit(1);
		}

		inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);
		
		short int sender = -1;
		// check if the fromAddr contains the prefix we're looking for
		if(strstr(fromAddr, "10.1.1."))
		{
			// extract broadcasting neighbor id
			sender = atoi(
					strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
			dir[sender].recordHeartbeat();
			Packet p = Packet(dir[sender], recvBuf);
		}


	close(globalSocketUDP);
}
