#include "logger.h"
#include "packet.h"


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

Packet::Packet(Resident* _owner, Logger* _logger, unsigned char* recvBuf)
{
	owner = _owner;
	logger = _logger;
	string str = string((char*) recvBuf);

	// if(!strncmp(recvBuf, "send", 4))
	if(str.substr(0, 4) == "send")
	{
		op = "send";
		src = owner->id;
		// dest = extractDestNode(str)
		// contents = extractContents()
	} else if (str.substr(0, 4) == "cost")
	{
		op = "cost";
		src = owner->id;
		// dest = extractDestNode(str)
		// contents = extractContents()
	} else
	{
		vector<string> tokens = tokenize(str, "|");
		op = tokens[0];
		src = stoi(tokens[1]);
		dest = stoi(tokens[2]);
		nextHop = stoi(tokens[3]);
		contents = stoi(tokens[4]);
	}
}

void Packet::execute() {
	if (op == "cost")
	{
		int updatedCost = stoi(contents);
		owner->edgeCost = updatedCost;
		logger->addCostUpdate(dest, updatedCost);
	} else if (op == "recv")
	{
		logger->addRecv(dest, contents);
	} else if (op == "send")
	{

	} else if (op == "forward") {

	}
	// handle reachable vs. unreachable use case
}
