#include "header_files/packet.h"


Packet::Packet(Node* _node, int srcID, int _bytesRecvd, char* _rawPacket)
	: node(_node), bytesRecvd(_bytesRecvd), rawPacket(_rawPacket)
{
	char char_op[op_size];
	memcpy(char_op, rawPacket, op_size);
	char_op[op_size] = '\0';
	op = string(char_op);

	short int destID;
	memcpy(&destID, rawPacket + op_size, nodeID_size);
	destID = ntohs(destID);
	dest = node->dir[destID];
	if (srcID == -1)
	{
		src = NULL;
	} else
	{
		src = node->dir[srcID];
	}

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

// from manager: send<dest><message> ex. send4hello
// from  node: send<dest><src><TTL><message> send4216hello
void Packet::handleSendOP()
{
	short int origin = extractOrigin();
	bool fromManager = (origin == -1) ? true : false;
	string message = extractMessage(bytesRecvd, rawPacket, fromManager);

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
		char newPacket[bytesRecvd+nodeID_size];
		short int no_origin = htons(node->id);

		int messageLength = bytesRecvd - sendHeader_size;

		// send<dest>
		memcpy(&newPacket, rawPacket, sendHeader_size);
		// send<dest><src>
		memcpy(newPacket + sendHeader_size, &no_origin, nodeID_size);
		// send<dest><src><msg>
		memcpy(newPacket + sendHeader_size + nodeID_size, rawPacket + sendHeader_size, messageLength);

		node->logger->addSend(nextHop->id, dest->id, message);
		nextHop->send(newPacket, bytesRecvd+nodeID_size);
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