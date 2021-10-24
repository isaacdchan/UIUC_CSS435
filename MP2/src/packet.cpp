#include "header_files/packet.h"


string extractMessage(int bytesRecvd, char* rawPacket, bool fromManager)
{
	// bias of -10 to account for op, dest, src
	int bias = fromManager ? 6 : 6 + Packet::nodeIdSize;
	int messageLength = bytesRecvd - bias;
	char message[messageLength];
	memcpy(&message, rawPacket + bias, messageLength);
	message[messageLength] = '\0';

	return string(message);
}

Packet::Packet(int srcID, Node* _node, int _bytesRecvd, char* _rawPacket)
	: node(_node), bytesRecvd(_bytesRecvd), rawPacket(_rawPacket)
{
	char char_op[5];
	memcpy(char_op, rawPacket, 4);
	char_op[4] = '\0';
	op = string(char_op);

	short int destID;
	memcpy(&destID, rawPacket + 4, nodeIdSize);
	destID = ntohs(destID);
	dest = node->dir[destID];
	src = node->dir[srcID];

	if (op == "cost") { handleCostOP();	}
	if (op == "path") { handlePathOP(); }
	if (op == "send") { handleSendOP(); }
}

void Packet::handleCostOP()
{
	int newEdgeCost;
	memcpy(&newEdgeCost, rawPacket + 6, 4);
	newEdgeCost = ntohl(newEdgeCost);

	int oldEdgeCost = dest->edgeCost;
	int edgeDiff = oldEdgeCost - newEdgeCost;
	dest->edgeCost = newEdgeCost;
	node->logger->addEdgeCostUpdate(dest->id, newEdgeCost);

	for (Resident* r: node->dir)
	{
		// if the path from node to r went through dest
		// the shortest path cost must be adjusted (and potentially changed)
		if (r->nextHop == dest)
		{
			int oldPathCost = r->pathCost;
			int newPathCost = oldPathCost - edgeDiff;
			// nextHop remains the same for now
			node->updatePath(r, dest, newPathCost);
		}
	}
}
void Packet::handlePathOP()
{
	int srcToDestCost;
	memcpy(&srcToDestCost, rawPacket + 6, 4);
	srcToDestCost = ntohl(srcToDestCost);
	src->costsToOthers[dest->id] = srcToDestCost;

	int currPathCost = dest->pathCost;
	int selfToSrcEdgeCost = src->edgeCost; // assume edge is live
	int candidatePathCost = selfToSrcEdgeCost + srcToDestCost;

	bool shouldUpdatePath = false;
	if (candidatePathCost < currPathCost)
	{
		node->updatePath(dest, src, candidatePathCost);
	}
	if (candidatePathCost == currPathCost && src->id < dest->nextHop->id)  // tiebreak based on lower value
	{
		node->updatePath(dest, src, candidatePathCost);
	}
}

// send4hello | send42hello
void Packet::handleSendOP()
{
	short int origin = extractOrigin();
	bool fromManager = (origin == -1) ? true : false;
	string message = extractMessage(bytesRecvd, rawPacket, fromManager);
	node->logger->ss << "Origin: " << origin;
	node->logger->add();

	if (dest->id == node->id)
	{
		node->logger->addRecv(origin, message);
		return;
	}

	Resident* nextHop = dest->nextHop;

	// could have gone down between last path update
	if (nextHop == NULL || !nextHop->edgeIsActive)
	{
		nextHop = findNewNextHop();

		if (nextHop == NULL) // no edges active
		{
			node->logger->addUnreachable(origin, dest->id);
			return;
		}
	}
	if (fromManager)
	{
		// need to add TTL
		char newPacket[bytesRecvd+nodeIdSize];
		short int no_origin = htons(node->id);

		int messageLength = bytesRecvd - 6;

		// send<dest>
		memcpy(&newPacket, rawPacket, 6);
		// send<dest><src>
		memcpy(newPacket + 6, &no_origin, nodeIdSize);
		// send<dest><src><msg>
		memcpy(newPacket + 6 + nodeIdSize, rawPacket + 6, messageLength);

		// node->logger->addSend(nextHopResident->id, dest, message);
		nextHop->send(newPacket, bytesRecvd+nodeIdSize);
	} else 
	{
		node->logger->addForward(origin, nextHop->id, dest->id, message);
		nextHop->send(rawPacket, bytesRecvd);
	}

}

Resident* Packet::findNewNextHop() {
	Resident* closestNeighbor = NULL;
	for (Resident* r: node->dir)
	{
		// since it increments through dir, the smaller edge id will always win
		if (r->edgeIsActive && r->edgeCost < closestNeighbor->edgeCost) 
		{
			closestNeighbor = r;
		}
	}

	return closestNeighbor;
}

short int Packet::extractOrigin() {
	short int origin;
	if (src->id == -1)
	{
		origin = -1;
	} else
	{
		memcpy(&origin, rawPacket + 6, nodeIdSize);
		origin = ntohs(origin);
	}
	return origin;
}
