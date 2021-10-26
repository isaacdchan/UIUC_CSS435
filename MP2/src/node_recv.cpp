#include "header_files/node.h"
#include "header_files/packet.h"

void Node::monitorResidentsHealth()
{
	bool isHealthy;
	timeval lastHeartbeat, currTime;
	while (1)
	{
		gettimeofday(&currTime, 0);
		for (Resident* r: dir)
		{
			if (r->id == id) continue;
			isHealthy = r->checkHealth(currTime);
			if (isHealthy)
			{
				// if (!dir[i]->edgeIsActive) { logger->addEdgeRevived(i); }
				r->edgeIsActive = true;
			} else
			{
				if (r->edgeIsActive)
				{
					r->edgeIsActive = false;
					logger->addEdgeExpired(r->id);
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
		if(strstr(fromAddr, "10.0.0."))
		{
			Packet p = Packet(this, -1, bytesRecvd, recvBuf);
		}
		// neighbor
		if(strstr(fromAddr, "10.1.1."))
		{
			// extract broadcasting neighbor id
			short int sender = atoi(
					strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
			dir[sender]->recordHeartbeat();
			Packet p = Packet(this, sender, bytesRecvd, recvBuf);
		}
	}
	close(udpSocket);
}

void Node::updatePath(Resident* dest, Resident* nextHop, int newPathCost)
{
	dest->pathCost = newPathCost;
	dest->nextHop = nextHop;
	logger->addPathCostUpdate(dest->id, nextHop->id, newPathCost);
}
