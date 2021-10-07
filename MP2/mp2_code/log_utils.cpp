#include "imports.h"

// initialize using test + node number

int myId;
stringstream ss;
ofstream logFile;

void initLogFile(int testId, int nodeId)
{
	myId = nodeId;
	string fileName = testId + "_" + myId + string(".log");
	logFile.open(fileName);
}

void addLog()
{
	logFile << ss.str() << "\n";
	ss.clear();
}

void addCostUpdateLog(int otherNode, int updatedCost)
{
	ss << "edge cost to " << otherNode << " updated to " << updatedCost;
	addLog();
}

void addEdgeExpiredLog(int otherNode)
{
	ss << "edge from " << otherNode << " expired";
	addLog();
}

void addRecvLog(int othernode, string msg)
{
	ss << "received msg from node" << othernode << ": " << msg;
	addLog();
}