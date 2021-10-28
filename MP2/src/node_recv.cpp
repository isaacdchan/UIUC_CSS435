#include "header_files/node.h"
#include "header_files/packet.h"

void Node::monitorResidentsHealth() {
	bool isHealthy;
	timeval lastHeartbeat, currTime;
	while (1) {
		gettimeofday(&currTime, 0);
		for (Resident* r: dir) {
			if (r->id == id) continue;
			isHealthy = r->checkHealth(currTime);

			if (isHealthy) {
				// if (!r->edgeIsActive) { logger->addEdgeRevived(r->id); }
				r->edgeIsActive = true;
			} else {
				if (r->edgeIsActive) {
					r->edgeIsActive = false;
					logger->addEdgeExpired(r->id);
				}
			}
		}
	}
}

void Node::listenForMessages() {
	char fromAddr[100];
	struct sockaddr_in theirAddr;
	socklen_t theirAddrLen;
	char recvBuf[1000];

	int bytesRecvd;
	while(1) {
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
		if(strstr(fromAddr, "10.0.0."))	{
			Packet p = Packet(this, -1, bytesRecvd, recvBuf);
		}
		// neighbor
		if(strstr(fromAddr, "10.1.1."))	{
			// extract broadcasting neighbor id
			short int sender = atoi(
					strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
			dir[sender]->recordHeartbeat();
			Packet p = Packet(this, sender, bytesRecvd, recvBuf);
		}
	}
	close(udpSocket);
}

void Node::updatePath(Resident* dest, Resident* nextHop, int newPathCost) {
	dest->nextHop = nextHop;
	dest->pathCost = newPathCost;
	logger->addPathCostUpdate(dest->id, nextHop->id, newPathCost);
	broadcastPathCost(dest);
}

void Node::killPath(Resident* dest) {
	logger->ss << "Killing path to node" << dest->id;
	logger->add();
	dest->nextHop = NULL;
	dest->pathCost = INT_MAX;
	// broadcastPathCost(dest);
}

void Node::findAltPath(Resident* dest) {
	logger->ss << "Looking for alt path to node" << dest->id;
	logger->add();
	killPath(dest);
	Resident* altNextHop = NULL;
	int cheapestAltPathCost = INT_MAX;

	for (Resident* r: dir) {
		if (r->nextHop==NULL) { continue; }
		if (!r->nextHop->edgeIsActive) { continue; }
		if (r->costsToDests[dest->id] == INT_MAX) { continue; }

		// check the dist from r to dest
		int rPathCostToDest = r->costsToDests[dest->id];

		logger->ss << "\t" << r->id << " | " << r->edgeCost << " | " << rPathCostToDest;
		logger->add();
		if (r->edgeCost + rPathCostToDest < cheapestAltPathCost) {
			altNextHop = r;
			cheapestAltPathCost = r->edgeCost + rPathCostToDest;
		}
	}

	if (altNextHop != NULL && cheapestAltPathCost != INT_MAX) {
		logger->ss << "\tAlt path found";
		logger->add();
		updatePath(dest, altNextHop, cheapestAltPathCost);
	} else {
		logger->ss << "\tNo alt path found";
		logger->add();
	}
}
