#include "header_files/packet.h"

vector<string> tokenize(string str, string delim)
{
	vector<string> tokens;
	string token;
	size_t pos = 0;
	while ((pos = str.find(delim)) != string::npos)
	{
		token = str.substr(0, pos);
		tokens.push_back(token);
		str.erase(0, pos + delim.length());
	}

	return tokens;
}

Packet::Packet(int _src, Node* _node, int _bytesRecvd, unsigned char* _rawPacket)
	: src(_src), node(_node), bytesRecvd(_bytesRecvd), rawPacket(_rawPacket)
{
	char char_op[5];
	memcpy(char_op, rawPacket, 4);
	char_op[4] = '\0';
	string op = string(char_op);

	memcpy(&dest, rawPacket + 4, 2);
	dest = ntohs(dest);

	if (op == "cost") { handleCostOP();	}
	else if (op == "path"){ handlePathOP(); }
	else if (op == "send") { handleSendOP(); }
	else { // unknown op }
}

void Packet::handleCostOP()
{
	int selfToDestEdgeCost;
	memcpy(&selfToDestEdgeCost, rawPacket + 6, 4);
	selfToDestEdgeCost = ntohl(selfToDestEdgeCost);
	cout << op << " | " << dest << " | " << selfToDestEdgeCost << endl;

	node->dir[dest]->edgeCost = selfToDestEdgeCost;
	node->logger->addEdgeCostUpdate(dest, selfToDestEdgeCost);

	// update best path
}
void Packet::handlePathOP()
{
	int srcToDestCost;
	memcpy(&srcToDestCost, rawPacket + 6, 4);
	srcToDestCost = ntohl(srcToDestCost);

	int currBestCost = node->dir[dest]->pathCost;
	int selfToSrcEdgeCost = node->dir[src]->edgeCost; // assume edge is live
	int candidatePathCost = selfToSrcEdgeCost + srcToDestCost;

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
void Packet::handleSend()
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
		int messageLength = bytesRecvd - 6;
		char message[messageLength];
		memcpy(&message, rawPacket + 6, messageLength);
		message[messageLength] = '\0';

		Resident* destResident = node->dir[dest];
		int nextHop = destResident->nextHop;
		Resident* nextHopResident = node->dir[nextHop];

		// could have gone down between last path update
		if (!nextHopResident->edgeIsActive)
		{
			// find another nextHop
		}
		if (src == -1) //manager
		{
			node->logger->addSend(dest, message);
		} else 
		{
			node->logger->addForward(src, dest, message);
		}
	}

}