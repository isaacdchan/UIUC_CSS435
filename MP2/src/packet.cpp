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
	if (srcID == -1) {
		src = NULL;
	} else {
		src = node->dir[srcID];
	}

	if (op == "cost") { handleCostOP();	}
	if (op == "path") { handlePathOP(); }
	if (op == "send") { handleSendOP(); }
}

void Packet::handleCostOP() {
	int newEdgeCost;
	memcpy(&newEdgeCost, rawPacket + op_size + nodeID_size, edgeCost_size);
	newEdgeCost = ntohl(newEdgeCost);

	int oldEdgeCost = dest->edgeCost;
	int edgeDiff = oldEdgeCost - newEdgeCost;
	dest->edgeCost = newEdgeCost;
	node->logger->addEdgeCostUpdate(dest->id, newEdgeCost);

	for (Resident* r: node->dir) {
		// if the path from node to r went through dest
		// the shortest path cost must be adjusted (and potentially changed)
		if (r->nextHop == dest)	{
			int oldPathCost = r->pathCost;
			int newPathCost = oldPathCost - edgeDiff;
			// nextHop remains the same for now
			node->updatePath(r, dest, newPathCost);
		}
	}
}

void Packet::handlePathOP() {
	int srcToDestCost;
	memcpy(&srcToDestCost, rawPacket + 6, 4);
	srcToDestCost = ntohl(srcToDestCost);
	src->costsToOthers[dest->id] = srcToDestCost;

	// nodeX just broadcasted that it no longer has a path to nodeY
	// if your path to nodeX was through nodeY, you need to find altPath
	if (srcToDestCost == INT_MAX) {
		if (dest->nextHop == src) {
			handleUnreachablePath();
		}
		return;
	}

	int currPathCost = dest->pathCost;
	int candidatePathCost = src->edgeCost + srcToDestCost;

	// tiebreak based on lower value
	if (candidatePathCost < currPathCost ||
		candidatePathCost == currPathCost && src->id < dest->nextHop->id) {
		node->updatePath(dest, src, candidatePathCost);
	}
}

// from manager: send<dest><message> ex. send4hello
// from  node: send<dest><src><MAX_TTL><message> send4216hello
void Packet::handleSendOP() {
	extractOrigin();
	fromManager = (origin == -1) ? true : false;
	extractMessage();

	if (dest->id == node->id) {
		node->logger->addRecv(origin, message);
		return;
	}

	extractTTL();
	if ( !fromManager ) { TTL-=1; }

	if (TTL == 0) {
		handleZeroTTL();
		return;
	}

	// currNextHop is missing or dead
	if (dest->nextHop == NULL || !dest->nextHop->edgeIsActive) {

		node->logger->ss << "Received send. No nextHop" << dest->id;
		node->logger->add();
		node->findAltPath(src, dest, true);
	}
	if (dest->nextHop == NULL) {
		node->logger->addUnreachable(dest->id);
		return;
	}

	char* newRawPacket = constructSendPacket();
	if (fromManager) {
		node->logger->addSend(dest->nextHop->id, dest->id, message);
		dest->nextHop->send(newRawPacket, bytesRecvd+nodeID_size+TTL_size);
	} else  {
		node->logger->addForward(src->id, dest->nextHop->id, dest->id, message);
		dest->nextHop->send(newRawPacket, bytesRecvd);
	}
	delete newRawPacket;
}

void Packet::handleUnreachablePath() {
	node->logger->ss << "Received news that node" << src->id << " cannot reach node" << dest->id;
	node->logger->add();
	node->findAltPath(src, dest, false);
}

void Packet::handleZeroTTL() {
	node->logger->ss << "Received expired packet from node" << src->id;
	node->logger->add();
	node->findAltPath(src, dest, true);

	TTL = MAX_TTL;
	char* newRawPacket = constructSendPacket();

	Resident* nextHop;
	if (dest->nextHop == NULL) {
		// send it back
		node->logger->ss << "\tReturning to sender" << src->id;
		node->logger->add();
		node->killPath(dest);
		nextHop = src;
	} else {
		nextHop = dest->nextHop;
		node->logger->addForward(src->id, dest->nextHop->id, dest->id, message);
		dest->nextHop->send(newRawPacket, bytesRecvd);
	}
	node->logger->addForward(src->id, nextHop->id, dest->id, message);
	nextHop->send(newRawPacket, bytesRecvd);
}