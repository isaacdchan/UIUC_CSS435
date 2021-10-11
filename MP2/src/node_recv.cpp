#include "node.h"

void Node::monitorResidentsHealth()
{
	bool isHealthy;
	timeval lastHeartbeat, currTime;
	while (1) {
		gettimeofday(&currTime, 0);
		for (int i=0; i<256; i++) {
			isHealthy = dir[1].checkHealth(currTime);
			if (!isHealthy) {
				dir[1].edgeIsActive = false;
				logger.addEdgeExpired(i);
			}
		}
	}
}

void Node::listenForMessages()
{
	char fromAddr[100];
	struct sockaddr_in theirAddr;
	socklen_t theirAddrLen;
	unsigned char recvBuf[1000];

	int bytesRecvd;
	while(1)
	{
		theirAddrLen = sizeof(theirAddr);
		if ((bytesRecvd = recvfrom(udpSocket, recvBuf, 1000 , 0, 
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
			Packet p = Packet(&dir[sender], &logger, recvBuf);
		}
	}
	close(udpSocket);
}
