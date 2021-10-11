#include "resident.h"

Resident::Resident(int _id, int _udpSocket) {
	id = _id;
	udpSocket = _udpSocket;
}

bool Resident::checkHealth(timeval currTime)
{
	int i;
	timeval lastHeartbeat;
	time_t then, now, diff;

	then = lastHeartbeat.tv_sec * 1000L + lastHeartbeat.tv_usec / 1000L;
	now = currTime.tv_sec * 1000L + currTime.tv_usec / 1000L;
	diff =  now - then;

	return diff < EXPIRATION_THRESHOLD;
}

void Resident::recordHeartbeat()
{
	gettimeofday(&lastHeartbeat, 0);
	edgeIsActive = true;
}

void Resident::send(const char* buf, int length)
{
	sendto(udpSocket, buf, length, 0,
	(struct sockaddr*)&sockaddr, sizeof(sockaddr));
}
