#include "imports.h"
#include "node.h"

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
		exit(1);
	}
	
	Node node = Node(atoi(argv[1]), argv[2]);
	
	// constantly checks if a neighbor's most recent heartbeat is older than threshold
	pthread_t announcerThread;
	pthread_t listenerThread;
	pthread_t neighborHealthThread; 


	thread announcerThread(&Node::broadcastHeartbeat, &node);
	thread listenerThread(&Node::listenForMessages, &node);
	thread neighborHealthThread(&Node::monitorResidentsHealth, &node);
}