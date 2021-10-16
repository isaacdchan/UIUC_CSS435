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

Packet::Packet(int _src, Node* node, int bytesRecvd, unsigned char packet[])
{
	src = _src;

	char char_op[5];
	memcpy(char_op, packet, 4);
	char_op[4] = '\0';
	string op = string(char_op);

	memcpy(&dest, packet + 4, 2);
	dest = ntohs(dest);

	if (op == "cost") 
	{
		int selfToDestEdgeCost;
		memcpy(&selfToDestEdgeCost, packet + 6, 4);
		selfToDestEdgeCost = ntohl(selfToDestEdgeCost);
		cout << op << " | " << dest << " | " << selfToDestEdgeCost << endl;

		node->dir[dest]->edgeCost = selfToDestEdgeCost;
		node->logger->addEdgeCostUpdate(dest, selfToDestEdgeCost);

		// update best path
	} else if (op == "path")
	{
		int srcToDestCost;
		memcpy(&srcToDestCost, packet + 6, 4);
		srcToDestCost = ntohl(srcToDestCost);

		int currBestCost = node->dir[dest]->pathCost;
		int selfToSrcEdgeCost = node->dir[src]->edgeCost; // assume edge is live
		int candidatePathCost = selfToSrcEdgeCost + srcToDestCost;

		// handle reachable vs. unreachable use case
		if (candidatePathCost > currBestCost)
		{
			// ignore
		} else if (candidatePathCost < currBestCost)
		{
			node->dir[dest]->pathCost = candidatePathCost;
			node->dir[dest]->nextHop = src;
			node->logger->addPathCostUpdate(dest, src, candidatePathCost);

			// broadcast to neighbors that they have a new cheapest path from self to dest
		} else  // tiebreak based on lower value
		{
			int currNextHop = node->dir[dest]->nextHop;
			if (src < currNextHop)
			{
				node->dir[dest]->pathCost = candidatePathCost;
				node->dir[dest]->nextHop = src;
				node->logger->addPathCostUpdate(dest, src, candidatePathCost);
				// broadcast to neighbors that they have a new cheapest path from self to dest
			}
		}
	} else if (op == "send")
	{
			int messageLength = bytesRecvd - 6;
			char message[messageLength];
			memcpy(&message, packet + 6, messageLength);
			message[messageLength] = '\0';

			Resident* destResident = node->dir[dest];
			int nextHop = destResident->nextHop;
			Resident* nextHopResident = node->dir[nextHop];

			// check whether the edge is live
			// could have gone down between last path update
	}
}