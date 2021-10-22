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

	int currBestCost = node->dir[dest]->pathCost;
	int selfToSrcEdgeCost = node->dir[prevHop]->edgeCost; // assume edge is live
	int candidatePathCost = selfToSrcEdgeCost + srcToDestCost;

	// cout << currBestCost << " | " << candidatePathCost << endl;
	if (candidatePathCost < currBestCost)
	{
		node->updatePath(dest, prevHop, candidatePathCost);
	} if (candidatePathCost == currBestCost)  // tiebreak based on lower value
	{
		int currNextHop = node->dir[dest]->nextHop;
		if (prevHop < currNextHop)
		{
			node->updatePath(dest, prevHop, candidatePathCost);
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

		// string message2 = extractMessage(bytesRecvd+nodeIdSize, newPacket, 0);
		// cout << "MESSAGE2: " << message2 << endl;

		node->logger->addSend(nextHopResident->id, dest, message);
		nextHopResident->send(newPacket, bytesRecvd+nodeIdSize);
	} else 
	{
		node->logger->addForward(node->id, nextHopResident->id, dest, message);
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
