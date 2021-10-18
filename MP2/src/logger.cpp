#include "header_files/logger.h"

Logger::Logger(int _id, string _logFile) 
	: id(_id), logFile(_logFile)
{
	ofstream out;
	out.open(logFile, ofstream::out | ofstream::trunc);
	cout << "init logger id:" << id << endl;
}

void Logger::add()
{
	ofstream out(logFile, ofstream::app);
	out << ss.str() << endl;
	ss.str("");
}

void Logger::addEdgeCostUpdate(int otherNode, int updatedCost)
{
	ss << "edge cost to node" << otherNode << " updated to cost " << updatedCost;
	add();
}

void Logger::addPathCostUpdate(int dest, int nextHop, int updatedCost)
{
	ss << "path cost to node" << dest << " updated to cost " << updatedCost << " through node" << nextHop; 
	add();
}

void Logger::addEdgeExpired(int otherNode)
{
	ss << "edge from node" << otherNode << " expired";
	add();
}

void Logger::addEdgeRevived(int otherNode)
{
	ss << "edge from node" << otherNode << " revived";
	add();
}

void Logger::addSend(int dest, string msg)
{
	ss << "sending msg to node" << dest << ": " << msg;
	add();
}
void Logger::addForward(int prevHop, int dest, string msg)
{
	ss << "forwarding msg from node" << prevHop << " to node" << dest << ": " << msg;
	add();
}
void Logger::addRecv(int src, string msg)
{
	ss << "received msg from node" << src << ": " << msg;
	add();
}