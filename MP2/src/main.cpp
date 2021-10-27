#include "header_files/node.h"

int main(int argc, char** argv)
{
	if(argc != 4) {
		fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
		exit(1);
	}
	
	Node* node = new Node(atoi(argv[1]), argv[2], argv[3]);

	thread announcerThread(&Node::broadcastHeartbeat, node);
	thread listenerThread(&Node::listenForMessages, node);
	thread neighborHealthThread(&Node::monitorResidentsHealth, node);

	announcerThread.join();
	listenerThread.join();
	neighborHealthThread.join();

	// delete node;
}