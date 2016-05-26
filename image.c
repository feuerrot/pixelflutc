#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

int sockfd;
char command[22];
int sent;

void sendpixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b){
	memset(&command, 0, sizeof(command));
	sprintf(command, "PX %u %u %02X%02X%02X\n", x, y, r, g, b);

	if ((sent = send(sockfd, command, strlen(command), MSG_CONFIRM)) == -1) {
		fprintf(stderr, "send: %s\n", strerror(errno));
	}
	
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
	int numbytes;  
	struct addrinfo hints, *res, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 4){
		printf("Usage: %s [host] [port] [image]\n", argv[0]);
		return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	connect(sockfd, res->ai_addr, res->ai_addrlen);

	while(1){
		for (int i=0; i<1280; i++){
			for (int j=0; j<800; j++){
				sendpixel(i, j, (i*2-j)%255, (i-j)%255, (i*j)%255);
			}
		}
	}

	freeaddrinfo(res); // all done with this structure

	close(sockfd);

	return 0;
}
