#include "imports.h"

tuple<int, int> parseLine(string line)
{
	stringstream ss(line);
	string token;
	ss >> token;
	int node = stoi(token);
	ss >> token;
	int cost = stoi(token);

	tuple <int, int> tup(node, cost);
	return tup;
}

vector<tuple<int, int>> parseCostsFile(string filepath)
{
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