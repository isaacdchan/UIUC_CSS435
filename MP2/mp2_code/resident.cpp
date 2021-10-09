#include "imports.h"
#include "log_utils.cpp"

#define EXPIRATION_THRESHOLD 100
extern int globalSocketUDP;

struct Resident
{
	int id;
	timeval lastHeartbeat;
	sockaddr_in sockaddr;
	string path;
	int pathCost;
	bool edgeIsActive;
	int edgeCost;

	void checkHealth(timeval currTime);
	void recordHeartbeat();
	void send(const char* buf, int length);
};

void Resident::checkHealth(timeval currTime)
{
	int i;
	timeval lastHeartbeat;
	time_t then, now, diff;

	then = lastHeartbeat.tv_sec * 1000L + lastHeartbeat.tv_usec / 1000L;
	now = currTime.tv_sec * 1000L + currTime.tv_usec / 1000L;
	diff =  now - then;

	if (diff > EXPIRATION_THRESHOLD) {
		edgeIsActive = false;
		addEdgeExpiredLog(i);
	}
}

void Resident::recordHeartbeat()
{
	gettimeofday(&lastHeartbeat, 0);
	edgeIsActive = true;
}

void Resident::send(const char* buf, int length)
{
	sendto(globalSocketUDP, buf, length, 0,
	(struct sockaddr*)&sockaddr, sizeof(sockaddr));
}
