#include "header_files/resident.h"

Resident::Resident(int _id, int _udpSocket)
	: id(_id), udpSocket(_udpSocket), pathCost(INT_MAX), edgeCost(1), nextHop(NULL), edgeIsActive(false)
{
	lastHeartbeat = (struct timeval){0};
	for (int i=0; i < MAX_RESIDENTS; i++) {
		costsToDests[i] = INT_MAX;
	}
		
	char tempaddr[100];
	sprintf(tempaddr, "10.1.1.%d", id);
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(7777);
	inet_pton(AF_INET, tempaddr, &sockaddr.sin_addr);
}

bool Resident::checkHealth(timeval currTime) {
	time_t then, now, diff;

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
