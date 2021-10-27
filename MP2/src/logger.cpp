#include "header_files/logger.h"

Logger::Logger(int testID, string _logFile, bool _verbose) 
	: testID(testID), logFile(_logFile), verbose(_verbose)
{
	ofstream out;
	out.open(logFile, ofstream::out | ofstream::trunc);
}

void Logger::add() {
	ofstream out(logFile, ofstream::app);
	out << ss.str() << endl;
	ss.str("");
}

void Logger::addEdgeCostUpdate(int otherNode, int updatedCost) {
	if (verbose) {
		ss << "edge cost to node" << otherNode << " updated to cost " << updatedCost;
		add();
	}
}

void Logger::addPathCostUpdate(int dest, int nextHop, int updatedCost) {
	if (verbose) {
		// ss << "path cost to node" << dest << " updated to cost " << updatedCost << " through node" << nextHop; 
		ss << "PATH | dest:" << dest << " | nextHop:" << nextHop <<  " | cost: " << updatedCost;
		add();
	}
}

void Logger::addEdgeExpired(int otherNode) {
	if (verbose) {
		ss << "edge from node" << otherNode << " expired";
		add();
	}
}

void Logger::addEdgeRevived(int otherNode) {
	if (verbose) {
		ss << "edge from node" << otherNode << " revived";
		add();
	}
}

void Logger::addSend(int nextHop, int dest, string msg) {
	if (verbose) {
		// ss << "sending msg to node " << dest << " via nexthop " << nextHop << " message " << msg;
		ss << "SEND | dest:" << dest << " | nextHop:" << nextHop <<  " | msg: " << msg;
	} else {
		ss << "sending packet dest " << dest << " nexthop " << nextHop << " message " << msg;
	}
	add();
}

void Logger::addForward(int src, int nextHop, int dest, string msg) {
	if (verbose) {
		// ss << "forwarding msg from node" << src << " to node" << dest << " via nextHop " << nextHop <<  ": " << msg;
		ss << "FORWARD | src:" << src << " | nextHop:" << nextHop << " | dest:" << dest <<  " | msg: " << msg;
	} else {
		ss << "forward packet dest " << dest << " nexthop " << nextHop << " message " << msg;
	}

	add();
}

void Logger::addRecv(int src, string msg) {
	if (verbose) {
		ss << "received msg from node" << src << ": " << msg;
	} else {
		ss << "receive packet message " << msg;
	}
	add();
}

void Logger::addUnreachable(int dest) {
	if (verbose) {
		ss << "CURRENTLY UNREACHABLE: " << dest;
	} else {
		ss << "unreachable dest " << dest;
	}
	add();
}
