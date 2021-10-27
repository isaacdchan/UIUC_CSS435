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
				// if (!r->edgeIsActive) { logger->addEdgeRevived(r->id); }
				r->edgeIsActive = true;
			} else
			{
				if (r->edgeIsActive)
				{
					r->edgeIsActive = false;
					// findAltPath(r);
					logger->addEdgeExpired(r->id);
					// should tell N11 to look for N1 instead of N10
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

void Node::findAltPath(Resident* src, Resident* dest)
{
	logger->ss << "Looking for alt path!";
	logger->add();
	Resident* altNextHop = NULL;
	int cheapestAltPathCost = INT_MAX;

	for (Resident* r: dir)
	{
		if (r->nextHop==NULL) { continue; }
		if (!r->edgeIsActive) { continue; }
		if (r->costsToOthers[dest->id] == INT_MAX) { continue; }

		// check the dist from r to dest
		int rPathCostToDest = r->costsToOthers[dest->id];

		logger->ss << "\tPotential Candidate: " << r->id << " | " << r->edgeCost << " | " << rPathCostToDest;
		logger->add();
		if (r->edgeCost + rPathCostToDest < cheapestAltPathCost)
		{
			altNextHop = r;
			cheapestAltPathCost = r->edgeCost + rPathCostToDest;
		}
	}

	if (altNextHop != NULL) {
		updatePath(dest, altNextHop, altNextHop->pathCost + cheapestAltPathCost);
	} else {
		updatePath(dest, NULL, INT_MAX);
	}
}

void Node::updatePath(Resident* dest, Resident* nextHop, int newPathCost)
{
	dest->nextHop = nextHop;
	dest->pathCost = newPathCost;
	if (dest->nextHop != NULL)
	{
		logger->addPathCostUpdate(dest->id, nextHop->id, newPathCost);
	} else 
	{
		// node->broadcastPathCost(dest);
		logger->addUnreachable(dest->id);
	}
}
