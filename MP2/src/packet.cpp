#include "header_files/packet.h"


string extractMessage(int bytesRecvd, char* rawPacket, bool fromManager)
{
	// bias of -10 to account for op, dest, prevHop
	int bias = fromManager ? 6 : 6 + Packet::nodeIdSize;
	int messageLength = bytesRecvd - bias;
	char message[messageLength];
	memcpy(&message, rawPacket + bias, messageLength);
	message[messageLength] = '\0';

	return string(message);
}

Packet::Packet(int _prevHop, Node* _node, int _bytesRecvd, char* _rawPacket)
	: prevHop(_prevHop), node(_node), bytesRecvd(_bytesRecvd), rawPacket(_rawPacket)
{
	char char_op[5];
	memcpy(char_op, rawPacket, 4);
	char_op[4] = '\0';
	op = string(char_op);

	memcpy(&dest, rawPacket + 4, nodeIdSize);
	dest = ntohs(dest);

	if (op == "cost") { handleCostOP();	}
	if (op == "path") { handlePathOP(); }
	if (op == "send") { handleSendOP(); }
}

void Packet::handleCostOP()
{
	int newEdgeCost;
	memcpy(&newEdgeCost, rawPacket + 6, 4);
	newEdgeCost = ntohl(newEdgeCost);

	Resident* destResident = node->dir[dest];

	int oldEdgeCost = destResident->edgeCost;
	int edgeDiff = oldEdgeCost - newEdgeCost;
	destResident->edgeCost = newEdgeCost;
	node->logger->addEdgeCostUpdate(dest, newEdgeCost);

	for (Resident* r: node->dir)
	{
		// if the path from node to r went through dest
		// the shortest path cost must be adjusted (and potentially changed)
		if (r->nextHop == dest)
		{
			int oldPathCost = r->pathCost;
			int newPathCost = oldPathCost - edgeDiff;
			// nextHop remains the same for now
			node->updatePath(dest, destResident->nextHop, newPathCost);
		}
	}
}
void Packet::handlePathOP()
{
	int srcToDestCost;
	memcpy(&srcToDestCost, rawPacket + 6, 4);
	srcToDestCost = ntohl(srcToDestCost);

	int currPathCost = node->dir[dest]->pathCost;
	// prevHop == src
	int selfToSrcEdgeCost = node->dir[prevHop]->edgeCost; // assume edge is live
	int candidatePathCost = selfToSrcEdgeCost + srcToDestCost;

	bool shouldUpdatePath = false;
	if (candidatePathCost < currPathCost) { shouldUpdatePath = true; }
	if (candidatePathCost == currPathCost)  // tiebreak based on lower value
	{
		int currNextHop = node->dir[dest]->nextHop;
		if (prevHop < currNextHop) { shouldUpdatePath = true; }
	}

	if (shouldUpdatePath)
	{
		// node->logger->ss << selfToSrcEdgeCost << " | " << srcToDestCost << " | " << dest;
		// node->logger->add();
		node->updatePath(dest, prevHop, candidatePathCost);
		// for paths to all other residents
		for (Resident* r: node->dir)
		{
			// for neighbors who's path passed through dest
			// AND are not dest itself (don't rebroadcast)
			// AND we've a heartbeat from this nieghbor already
			if (r->nextHop == dest && r->id != dest && currPathCost != INT_MAX)
			{
				// calculate the difference in cost, now that self -> dest is cheaper
				// ex. n1 -> n2 was 5, updated to 3
				// n1 -> n3 used to cost 8 going through n2
				// must decrease cost to 6
				// int oldPathCostToR = r->pathCost;
				// int newPathCostToDest = node->dir[dest]->pathCost;
				// int costSaved = currPathCost - newPathCostToDest;
				// int newPathCostToR = oldPathCostToR - costSaved;
				/*
				this = node3, r = node1, dest = node2, r.nextHop = node2
				edge from node1 to node2 is 54
				oldCost from node3 to node1 = 55 via node2 (node1 has seen node2. node3 has not seen node2)
				oldCost from node3 to node2 is INT_MAX because we have not seen it yet
				newCost from node3 to node2 is 1 via node2
				newCost from node3 to node1 needs to be 2 via node2
				
				node->logger->ss << r->id << " | " << r->nextHop << " | " << dest << " | " << endl;
				node->logger->ss << currPathCost << " | " << newPathCostToDest << " | " << candidatePathCost << " | " << endl;
				node->logger->ss << oldPathCostToR << " | " << newPathCostToDest << " | " << costSaved << " | " << newPathCostToR;
				node->logger->add();
				*/

				// node->updatePath(r->id, r->nextHop, newPathCostToR);
			}
		}
	}
}

// send4hello | send42hello
void Packet::handleSendOP()
{
	int src = extractSendSrc();
	bool fromManager = (src == -1) ? true : false;
	string message = extractMessage(bytesRecvd, rawPacket, fromManager);

	if (dest == node->id)
	{
		node->logger->addRecv(src, message);
		return;
	}

	Resident* destResident = node->dir[dest];
	Resident* nextHopResident;
	int nextHop = destResident->nextHop;

	bool nextHopActive = (nextHop != -1) ? true : false;
	nextHopResident = node->dir[nextHop];
	nextHopActive = (nextHopResident->edgeIsActive) ? true : false;

	// could have gone down between last path update
	if (!nextHopActive)
	{
		int newNextHop = findNewNextHop();

		if (newNextHop == -1) // no edges active
		{
			node->logger->addUnreachable(src, dest);
			return;
		} else
		{
			// don't think need to broadcast ATM?
			nextHopResident = node->dir[newNextHop];
		}
	}
	if (fromManager)
	{
		// need to modify rawPacket so this->id is included
		char newPacket[bytesRecvd+nodeIdSize];
		short int no_srcId = htons(node->id);

		int messageLength = bytesRecvd - 6;

		// send<dest>
		memcpy(&newPacket, rawPacket, 6);
		// send<dest><src>
		memcpy(newPacket + 6, &no_srcId, nodeIdSize);
		// send<dest><src><msg>
		memcpy(newPacket + 6 + nodeIdSize, rawPacket + 6, messageLength);

		node->logger->addSend(nextHopResident->id, dest, message);
		nextHopResident->send(newPacket, bytesRecvd+nodeIdSize);
	} else 
	{
		node->logger->addForward(src, nextHopResident->id, dest, message);
		nextHopResident->send(rawPacket, bytesRecvd);
	}

}

int Packet::findNewNextHop() {
	int closestNeighbor = -1;
	int cheapestEdge = INT_MAX;
	for (Resident* r: node->dir)
	{
		// since it increments through dir, the smaller edge id will always win
		if (r->edgeIsActive && r->edgeCost < cheapestEdge) 
		{
			closestNeighbor = r->id;
			cheapestEdge = r->edgeCost;
		}
	}

	return closestNeighbor;
}

int Packet::extractSendSrc() {
	int src;
	if (prevHop == -1)
	{
		src = -1;
	} else
	{
		memcpy(&src, rawPacket + 6, nodeIdSize);
		src = ntohs(src);
	}
	return src;
}
