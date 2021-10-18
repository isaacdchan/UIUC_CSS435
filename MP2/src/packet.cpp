#include "header_files/packet.h"


string extractMessage(int bytesRecvd, unsigned char* rawPacket)
{
	int messageLength = bytesRecvd - 6;
	char message[messageLength];
	memcpy(&message, rawPacket + 6, messageLength);
	message[messageLength] = '\0';

	return string(message);
}

Packet::Packet(int _src, Node* _node, int _bytesRecvd, unsigned char* _rawPacket)
	: src(_src), node(_node), bytesRecvd(_bytesRecvd), rawPacket(_rawPacket)
{
	char char_op[5];
	memcpy(char_op, rawPacket, 4);
	char_op[4] = '\0';
	op = string(char_op);

	memcpy(&dest, rawPacket + 4, 2);
	dest = ntohs(dest);

	// cout << op << " | " << src  << " | " << dest << endl;

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
	int selfToSrcEdgeCost = node->dir[src]->edgeCost; // assume edge is live
	int candidatePathCost = selfToSrcEdgeCost + srcToDestCost;

	// cout << currBestCost << " | " << candidatePathCost << endl;
	if (candidatePathCost < currBestCost)
	{
		node->updatePath(dest, src, candidatePathCost);
	} if (candidatePathCost == currBestCost)  // tiebreak based on lower value
	{
		int currNextHop = node->dir[dest]->nextHop;
		if (src < currNextHop)
		{
			node->updatePath(dest, src, candidatePathCost);
		}
	}
}
void Packet::handleSendOP()
{
	// I AM INTENDED TARGET
	if (dest == node->id)
	{
		int prevHop; // who forwarded it to me
		int src; // original sender (could be same as above)
		string message; // extract message
		node->logger->addRecv(src, message);
	}
	else
	{
		// send4hello
		string message = extractMessage(bytesRecvd, rawPacket);

		Resident* destResident = node->dir[dest];
		int nextHop = destResident->nextHop;
		Resident* nextHopResident = node->dir[nextHop];

		// could have gone down between last path update
		if (!nextHopResident->edgeIsActive)
		{
			// nextHop is by default shortest edge?
			// how to find another nextHop? ask closest neighbor
		} else 
		{
			nextHopResident->send(rawPacket, bytesRecvd);
		}


		// final logging
		// might have to log unreachable
		if (src == -1) //manager
		{
			node->logger->addSend(dest, message);
		} else 
		{
			node->logger->addForward(src, dest, message);
		}
	}
}
