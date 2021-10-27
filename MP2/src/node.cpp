#include "header_files/node.h"

tuple<int, int> parseLine(string line) {
	stringstream ss(line);
	string token;
	ss >> token;
	int node = stoi(token);
	ss >> token;
	int cost = stoi(token);

	tuple <int, int> tup(node, cost);
	return tup;
}

vector<tuple<int, int>> parseCostsFile(string filepath) {
	string line;
	vector<tuple<int, int>> costs;

	ifstream infile;
	infile.open(filepath);

	while (getline(infile, line))
	{
		tuple<int, int> cost = parseLine(line);
		costs.push_back(cost);
	}

	return costs;
}	

void Node::initCosts(string costsFile) {
	vector<tuple<int, int>> initial_costs = parseCostsFile(costsFile);
	for (tuple<int, int> tup : initial_costs)
	{
		dir[get<0>(tup)]->edgeCost = get<1>(tup);
	}
}

void Node::initDir(int udpSocket) {
	for(int i=0; i<MAX_RESIDENTS; i++) {
		dir[i] = new Resident(i, udpSocket);

		if (i == id) {
			dir[i]->pathCost = 0;
			dir[i]->edgeCost = 0;
			dir[i]->nextHop = dir[i];
		}
		
		char tempaddr[100];
		sprintf(tempaddr, "10.1.1.%d", i);
		memset(&dir[i]->sockaddr, 0, sizeof(dir[i]->sockaddr));
		dir[i]->sockaddr.sin_family = AF_INET;
		dir[i]->sockaddr.sin_port = htons(7777);
		inet_pton(AF_INET, tempaddr, &dir[i]->sockaddr.sin_addr);
	}
}

Node::Node(short int _id, string costsFile, string logFile)
	: id(_id), logger(new Logger(id, logFile, VERBOSE))
{
	if((udpSocket=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	initDir(udpSocket);
	initCosts(costsFile);

	char myAddr[100];
	struct sockaddr_in bindAddr;
	sprintf(myAddr, "10.1.1.%d", id);	
	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(7777);
	inet_pton(AF_INET, myAddr, &bindAddr.sin_addr);
	if(bind(udpSocket, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		close(udpSocket);
		exit(1);
	}
}

Node::~Node() {
	delete logger;
	for (Resident* r: dir) { delete r; }
}
