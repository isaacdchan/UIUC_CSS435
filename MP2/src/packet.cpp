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
	memcpy(&newEdgeCost, rawPacket + op_size + nodeID_size, edgeCost_size);
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
// from  node: send<dest><src><MAX_TTL><message> send4216hello
void Packet::handleSendOP()
{
	short int origin = extractOrigin();
	fromManager = (origin == -1) ? true : false;
	string message = extractMessage();

	if (dest->id == node->id)
	{
		node->logger->addRecv(origin, message);
		return;
	}
	extractTTL();
	if ( !fromManager ) { TTL-=1; }

	node->logger->ss << "TTL: " << TTL;
	node->logger->add();

	// if (TTL == 0)
	// {
	// 	node->logger->addUnreachable(origin, dest->id)
	// }

	Resident* nextHop = dest->nextHop;

	if (nextHop == NULL || !nextHop->edgeIsActive)
	{
		findAltPath(dest);
		nextHop = dest->nextHop;
	}
	if (nextHop == NULL)
	{
		// node->logger->addUnreachable(origin, dest->id)
	}

	char* newRawPacket = constructSendPacket();
	if (fromManager)
	{
		node->logger->addSend(nextHop->id, dest->id, message);
		nextHop->send(newRawPacket, bytesRecvd+nodeID_size+TTL_size);
	} else 
	{
		node->logger->addForward(src->id, nextHop->id, dest->id, message);
		nextHop->send(newRawPacket, bytesRecvd);
	}
	delete newRawPacket;
}

void Packet::findAltPath(Resident* dest)
{
	Resident* cheapestAltNextHop = NULL;
	int cheapestAltPathCost = INT_MAX;
	for (Resident* r: node->dir)
	{
		if (r == src || r->nextHop==NULL) { continue; }
		if (!r->nextHop->edgeIsActive) { continue; }

		int rPathToDestCost = r->costsToOthers[dest->id];
		if (rPathToDestCost < cheapestAltPathCost)
		{
			cheapestAltNextHop = r;
			cheapestAltPathCost = rPathToDestCost;
		}
	}

	node->logger->ss << "CANDIDATE Path to Dest + newNextHop + newCost " << dest->id << " | " << cheapestAltNextHop->id << " | " << cheapestAltPathCost;
	node->logger->add();
	if (cheapestAltNextHop != NULL)
	{
		dest->nextHop = cheapestAltNextHop;
		dest->pathCost = cheapestAltNextHop->pathCost + cheapestAltPathCost;
	} else // no nextHop was found
	{
		// infinite loops
	}
}