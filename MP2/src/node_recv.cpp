#include "header_files/node.h"
#include "header_files/packet.h"

void Node::monitorResidentsHealth()
{
	bool isHealthy;
	timeval lastHeartbeat, currTime;
	while (1)
	{
		gettimeofday(&currTime, 0);
		for (int i=0; i<numResidents; i++)
		{
			if (i == id) continue;
			isHealthy = dir[i]->checkHealth(currTime);
			// constantly returning healthy
			if (isHealthy)
			{
				// if (!dir[i]->edgeIsActive)
				// {
				// 	logger->addEdgeRevived(i);
				// }

				// cout << "healthy: " << i << endl;
				dir[i]->edgeIsActive = true;
			} else
			{
				if (dir[i]->edgeIsActive)
				{
					dir[i]->edgeIsActive = false;
					logger->addEdgeExpired(i);
					// don't need to immediately update paths
				}
			}
		}
	}
}

void Node::listenForMessages()
{
	char fromAddr[100];
	struct sockaddr_in theirAddr;
	socklen_t theirAddrLen;
	char recvBuf[1000];

	int bytesRecvd;
	while(1)
	{
		memset(recvBuf, 0, sizeof(recvBuf));
		theirAddrLen = sizeof(theirAddr);
		if ((bytesRecvd = recvfrom(udpSocket, recvBuf, 1000 , 0, 
					(struct sockaddr*)&theirAddr, &theirAddrLen)) == -1)
		{
			perror("connectivity listener: recvfrom failed");
			exit(1);
		}

		inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);

		// manager
		if(strstr(fromAddr, "10.0.0.")) {
			Packet p = Packet(-1, this, bytesRecvd, recvBuf);
		}
		// neighbor
		if(strstr(fromAddr, "10.1.1."))
		{
			// extract broadcasting neighbor id
			short int sender = atoi(
					strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
			dir[sender]->recordHeartbeat();
			Packet p = Packet(sender, this, bytesRecvd, recvBuf);
		}
	}
	close(udpSocket);
}

void Node::updatePath(int dest, int nextHop, int newPathCost)
{
	dir[dest]->pathCost = newPathCost;
	dir[dest]->nextHop = nextHop;
	logger->addPathCostUpdate(dest, nextHop, newPathCost);
	// broadcast to neighbors that they have a new cheapest path from self to dest
	broadcastUpdatedPath(dest);
}
