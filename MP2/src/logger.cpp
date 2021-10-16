#include "header_files/logger.h"

Logger::Logger(int id, string logFile)
{
	id = id;
	file.open(logFile);
}
Logger::Logger()
{
	id = -1;
}

void Logger::add()
{
	file << ss.str() << "\n";
	ss.clear();
}

void Logger::addEdgeCostUpdate(int otherNode, int updatedCost)
{
	ss << "edge cost to " << otherNode << " updated to " << updatedCost;
	add();
}

void Logger::addPathCostUpdate(int dest, int nextHop, int updatedCost)
{
	ss << "path cost to " << dest << " updated to " << updatedCost << " through " << nextHop; 
	add();
}

void Logger::addEdgeExpired(int otherNode)
{
	ss << "edge from " << otherNode << " expired";
	add();
}

void Logger::addRecv(int othernode, string msg)
{
	ss << "received msg from node" << othernode << ": " << msg;
	add();
}