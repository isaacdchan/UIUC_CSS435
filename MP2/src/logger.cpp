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
	ss << "edge from node " << otherNode << " expired";
	add();
}

void Logger::addSend(int dest, string msg)
{
	ss << "sending msg to node " << dest << ": " << msg;
	add();
}
void Logger::addForward(int prevHop, int dest, string msg)
{
	ss << "forwarding msg from node" << prevHop << " to node " << dest << ": " << msg;
	add();
}
void Logger::addRecv(int src, string msg)
{
	ss << "received msg from node" << src << ": " << msg;
	add();
}