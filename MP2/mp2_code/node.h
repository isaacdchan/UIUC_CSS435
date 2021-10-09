#include "imports.h"
#include "resident.cpp"
#include "packet.cpp"

#include "file_utils.cpp"
#include "log_utils.cpp"


struct Node
{
	int id;
	Resident dir[256];

	// init
	Node(int id, string costsFile);
	void initDir();
	void initCosts(string costsFile);
	void initLogFile(string logFile);

	// send
	void broadcast(const char* buf, int length);
	void broadcastHeartbeat();
	void advertiseEdgeCosts();

	// recv
	void monitorResidentsHealth();
	void listenForMessages();
};