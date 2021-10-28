#include "header_files/packet.h"
#include "header_files/imports.h"

void Packet::extractTTL() {
	if (fromManager) {
		TTL = MAX_TTL;
	} else {
		int raw_TTL;
		memcpy(&raw_TTL, rawPacket + sendHeader_size + nodeID_size, TTL_size);
		TTL = ntohl(raw_TTL);
	}
}

void Packet::extractMessage() {
	int bias = fromManager ? sendHeader_size : forwardHeader_size;
	int messageLength = bytesRecvd - bias;
	char message_arr[messageLength];
	memcpy(&message_arr, rawPacket + bias, messageLength);
	message_arr[messageLength] = '\0';

	message =  string(message_arr);
}

void Packet::extractOrigin() {
	if (src == NULL) {
		origin = -1;
	} else {
		memcpy(&origin, rawPacket + sendHeader_size, nodeID_size);
		origin = ntohs(origin);
	}
}

char* Packet::constructSendPacket() {
	char* newPacket = new char[bytesRecvd+nodeID_size+TTL_size];
	int no_TTL = htonl(TTL);

	short int no_origin;
	if (fromManager) {
		no_origin = htons(node->id);
	} else {
		no_origin = htons(origin);
	}

	int bias = fromManager ? sendHeader_size : forwardHeader_size;
	int messageLength = bytesRecvd - bias;

	memcpy(newPacket, rawPacket, sendHeader_size);
	memcpy(newPacket + sendHeader_size, &no_origin, nodeID_size);
	memcpy(newPacket + sendHeader_size + nodeID_size, &no_TTL, TTL_size);
	memcpy(newPacket + sendHeader_size + nodeID_size + TTL_size, rawPacket + bias, messageLength);
	return newPacket;
}