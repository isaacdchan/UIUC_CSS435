#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <string>
using namespace std;

#define OUTPUT_FILE "output"
#define MAX_PROTOCOL_SIZE 100
#define MAX_DATA_SIZE 500000 // max number of bytes we can get at once 
#define MAX_PATH_SIZE 500
#define MAX_IP_SIZE 100
#define MAX_PORT_SIZE 4


int extract_status_code(char line[]) {
	char* token = strtok(line, " ");
	char** tokens = (char**) malloc(sizeof(char*) * 10);

	int i = 0;
	while (token != NULL) {
		tokens[i++] = token;
        token = strtok(NULL, " ");
	}
	int status_code = atoi(tokens[1]);

	free(tokens);
	return status_code;
}

char** parse_response_header(char response[]) {
	char* line = strtok(response, "\n");
	int num_lines = 7;
	char** lines = (char**) malloc(sizeof(char*) * num_lines);

	int i = 0;
	while (i < num_lines) {
		lines[i++] = line;
        line = strtok(NULL, "\n");
	}

	return lines;
}

char* strip_header(char* original) {
	char* copy = (char*) malloc(MAX_DATA_SIZE);

	char* header = strstr(original, "Last-Modified");
	int idx = (int)(header - original); 
	memcpy(copy, &original[idx+48], MAX_DATA_SIZE);

	return copy;
}

void handle_response(int sockfd) {
	char response[MAX_DATA_SIZE];
	char response_copy[MAX_DATA_SIZE];

	int bytes_received;
	int first_response = 1;
	int is_binary;
	FILE* fp = fopen(OUTPUT_FILE, "w+b");

	while (1) {
		memset(response, 0, MAX_DATA_SIZE);
		bytes_received = recv(sockfd, response, MAX_DATA_SIZE, 0);
		printf("Bytes Received: %d\n", bytes_received);

		if (bytes_received == -1) {
			perror("recv");
			exit(1);
		} else if (bytes_received == 0) {
			break;
		}

		if (first_response) {
			first_response = 0;

			memcpy(response_copy, response, sizeof(response));
			char** headers = parse_response_header(response_copy);
			int status_code = extract_status_code(headers[0]);

			if (status_code == 404) {
				fprintf(fp, "FILENOTFOUND");
				return;
			}

			headers[3][strcspn(headers[3], "\r\n")] = 0;
			is_binary = strcmp(headers[3], "Content-type: application/octet-stream") != 0;
			if (!is_binary) { response[bytes_received] = '\0'; }
			free(headers);

			char* new_response = strip_header(response);
			// memcpy(response, new_response, sizeof(new_response));
			if (is_binary) {
				fwrite(new_response, 1, bytes_received, fp);
			} else {
				fprintf(fp, "%s", new_response);
			}

			free(new_response);
		} else {
			// if (is_binary) {
			// 	fwrite(response, 1, bytes_received, fp);
			// } else {
			// 	fprintf(fp, "%s", response);
			// }
		}

	}

	fclose(fp);
	close(sockfd);
}

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char *argv[]) {
	int sockfd, bytes_received;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	char protocol[MAX_PROTOCOL_SIZE];
    char ip[MAX_IP_SIZE];
	char port[MAX_IP_SIZE];
    int int_port;
    char path[MAX_PATH_SIZE];
    sscanf(argv[1], "%[^:]%*[:/]%[^:]:%d%s", protocol, ip, &int_port, path);
	sprintf(port, "%d", int_port);

    for(int i=0; i<strlen(protocol); i++){
        protocol[i] = toupper(protocol[i]);
    }

	if (strcmp(protocol, "HTTP") != 0 && strcmp(protocol, "HTTPS") != 0) {
		FILE* fp = fopen(OUTPUT_FILE, "w");
		fprintf(fp, "INVALIDPROTOCOL");
		fclose(fp);
		return 0;
	}

	if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
	// if ((rv = getaddrinfo("localhost/test_folder/", port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		FILE* fp = fopen(OUTPUT_FILE, "w");
		fprintf(fp, "NOCONNECTION");
		fclose(fp);
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);

	freeaddrinfo(servinfo); // all done with this structure

	char request[700];
	sprintf(request, "GET %s %s/1.0\n\n", path, protocol);
	// sprintf(request, "GET /test_folder/example_file HTTP/1.0\n\n");

	size_t bytes_to_send = strlen(request);
	if (send(sockfd, request, bytes_to_send, 0) == -1)
		perror("send");

	handle_response(sockfd);
	return 0;
}
