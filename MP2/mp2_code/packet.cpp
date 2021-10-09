#include "imports.h"
#include "log_utils.cpp"
#include "resident.cpp"

struct Packet
{
	Resident* owner;
	string op;
	int src;
	int dest;
	int nextHop;
	string contents;

	Packet(Resident _owner, unsigned char* recvBuf);
	void execute();
};


Packet::Packet(Resident _owner, unsigned char* recvBuf) {
	owner = &_owner;
	string str = string((char*) recvBuf);

	// if(!strncmp(recvBuf, "send", 4))
	if(str.substr(0, 4) == "send")
	{
		op = "send";
		src = myId; // or srcnode = self.id?
		// dest = parseDestNode()
		// contents = parseContents()
	} else if (str.substr(0, 4) == "cost")
	{
		op = "cost";
		src = myId;
		// dest = parseDestNode()
		// contents = parseContents()
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
		addCostUpdateLog(dest, updatedCost);
	} else if (op == "recv")
	{
		addRecvLog(dest, contents);
	} else if (op == "send")
	{

	} else if (op == "forward") {

	}
	// reachable
	// unreachable
}

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