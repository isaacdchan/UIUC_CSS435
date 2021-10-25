#include "header_files/packet.h"
#include "header_files/imports.h"

string Packet::extractMessage(int bytesRecvd, char* rawPacket, bool fromManager)
{
	// bias of -10 to account for op, dest, src
	int bias = fromManager ? sendHeader_size : forwardHeader_size;
	int messageLength = bytesRecvd - bias;
	char message[messageLength];
	memcpy(&message, rawPacket + bias, messageLength);
	message[messageLength] = '\0';

	return string(message);
}

short int Packet::extractOrigin() {
	short int origin;
	if (src == NULL)
	{
		origin = -1;
	} else
	{
		memcpy(&origin, rawPacket + sendHeader_size, nodeID_size);
		origin = ntohs(origin);
	}
	return origin;
}