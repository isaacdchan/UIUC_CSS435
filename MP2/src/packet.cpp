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
	if (srcToDestCost == INT_MAX && dest->nextHop == src) 
	{
		// node11 just broadcasted that it no longer has a path to node77
		// if your path to node77 was through node11, you need to find altPath
		node->logger->ss << "Received news that node " << src->id << " cannot reach node" << dest->id;
		node->logger->add();
		node->updatePath(dest, NULL, INT_MAX);
		return;
	}
	src->costsToOthers[dest->id] = srcToDestCost;

	int currPathCost = dest->pathCost;
	int selfToSrcEdgeCost = src->edgeCost; // assume edge is live
	int candidatePathCost = selfToSrcEdgeCost + srcToDestCost;

	if (candidatePathCost < currPathCost ||
		candidatePathCost == currPathCost && src->id < dest->nextHop->id)  // tiebreak based on lower value
	{
		node->updatePath(dest, src, candidatePathCost);
	}
}

// from manager: send<dest><message> ex. send4hello
// from  node: send<dest><src><MAX_TTL><message> send4216hello
void Packet::handleSendOP()
{
	// not always correct
	extractOrigin();
	fromManager = (origin == -1) ? true : false;
	string message = extractMessage();

	if (dest->id == node->id)
	{
		node->logger->addRecv(origin, message);
		return;
	}
	extractTTL();
	if ( !fromManager ) { TTL-=1; }

	// abstract into expired
	if (TTL == 0)
	{
		node->updatePath(dest, NULL, INT_MAX);
		node->broadcastPathCost(dest);

		node->logger->ss << "FOUND EXPIRED PACKET";
		node->logger->add();
		node->findAltPath(src, dest);
		TTL = MAX_TTL;
		char* newRawPacket = constructSendPacket();
		node->logger->addForward(src->id, dest->nextHop->id, dest->id, message);
		dest->nextHop->send(newRawPacket, bytesRecvd);
		return;
	}

	if (dest->nextHop == NULL || !dest->nextHop->edgeIsActive) { node->findAltPath(src, dest); }
	if (dest->nextHop == NULL)
	{
		// node->logger->addUnreachable(dest->id)
	}

	char* newRawPacket = constructSendPacket();
	if (fromManager)
	{
		node->logger->addSend(dest->nextHop->id, dest->id, message);
		dest->nextHop->send(newRawPacket, bytesRecvd+nodeID_size+TTL_size);
	} else 
	{
		node->logger->addForward(src->id, dest->nextHop->id, dest->id, message);
		dest->nextHop->send(newRawPacket, bytesRecvd);
	}
	delete newRawPacket;
}