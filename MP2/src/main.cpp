#include "node.h"

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
		exit(1);
	}
	
	Node node = Node(atoi(argv[1]), argv[2], argv[3]);

	thread announcerThread(&Node::broadcastHeartbeat, &node);
	thread listenerThread(&Node::listenForMessages, &node);
	thread neighborHealthThread(&Node::monitorResidentsHealth, &node);
}