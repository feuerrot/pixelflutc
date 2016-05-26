#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <arpa/inet.h>

int sockfd;
char command[22];
int sent;

uint8_t *image;
int pic;

uint16_t xsize, ysize;

void readimage(char *filename){
	pic = open(filename, O_RDONLY);
	if (pic == -1){
		fprintf(stderr, strerror(errno));
	}

	read(pic, &xsize, 2);
	read(pic, &ysize, 2);
	printf("Image size: %u x %u\n", xsize, ysize);

	image = malloc(xsize*ysize*3);

	read(pic, image, xsize*ysize*3);
}

void sendpixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b){
	memset(&command, 0, sizeof(command));
	sprintf(command, "PX %u %u %02X%02X%02X\n", x, y, r, g, b);

	if ((sent = send(sockfd, command, strlen(command), MSG_CONFIRM)) == -1) {
		fprintf(stderr, "send: %s\n", strerror(errno));
	}
}


int main(int argc, char *argv[]){
	struct addrinfo hints, *res;
	int rv;

	if (argc != 4){
		printf("Usage: %s [host] [port] [image]\n", argv[0]);
		return 1;
	}

	srand(1);

	readimage(argv[3]);

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
		int i = rand()%xsize;
		int j = rand()%ysize;

		sendpixel(i, j, image[3*(i+j*xsize)], image[3*(i+j*xsize)+1], image[3*(i+j*xsize)+2]);
	}

	freeaddrinfo(res); // all done with this structure

	close(sockfd);

	return 0;
}
