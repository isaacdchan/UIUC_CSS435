#include "header_files/resident.h"

Resident::Resident(int _id, int _udpSocket)
	: id(_id), udpSocket(_udpSocket), pathCost(INT_MAX), edgeCost(1), nextHop(NULL), edgeIsActive(false)
{
	for (int i=0; i < MAX_RESIDENTS; i++) {
		costsToDests[i] = INT_MAX;
	}
}

bool Resident::checkHealth(timeval currTime) {
	int i;
	time_t then, now, diff;

	// time in ms
	then = lastHeartbeat.tv_sec * 1000L + lastHeartbeat.tv_usec / 1000L;
	now = currTime.tv_sec * 1000L + currTime.tv_usec / 1000L;
	diff =  now - then;

	return diff < EXPIRATION_THRESHOLD;
}

void Resident::recordHeartbeat() {
	gettimeofday(&lastHeartbeat, 0);
}

void Resident::send(char* buf, int length) {
	sendto(udpSocket, buf, length, 0,
		(struct sockaddr*)&sockaddr, sizeof(sockaddr));
}
