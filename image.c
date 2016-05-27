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
#include <signal.h>

#define PACKETSIZE 1200

typedef char command[22];

struct addrinfo hints, *res;
int rv;
int sockfd;
char packet[PACKETSIZE];
command *commandlist;
int sent;

uint8_t *image;
int pic;

uint16_t xsize, ysize;
uint16_t xoffset = 0;
uint16_t yoffset = 0;

static void cleanup(int signo){
	printf("Got Signal %s, doing cleanup\n", strsignal(signo));
	freeaddrinfo(res); // all done with this structure
	close(sockfd);
	free(image);
	exit(0);
}

void calculate_commands(command *commandlist){
	for (int x = 0; x < xsize; x++){
		for (int y = 0; y < ysize; y++){
			 sprintf(commandlist[x + y*xsize], "PX %u %u %02X%02X%02X\n", x + xoffset, y + yoffset, image[3*(x+y*xsize)], image[3*(x+y*xsize)+1], image[3*(x+y*xsize)+2]);
		}
	}
}

void readimage(char *filename){
	pic = open(filename, O_RDONLY);
	if (pic == -1){
		fprintf(stderr, strerror(errno));
	}

	read(pic, &xsize, 2);
	read(pic, &ysize, 2);
	printf("Image size: %u x %u\n", xsize, ysize);

	image = malloc(xsize*ysize*3);
	commandlist = malloc(xsize * ysize * sizeof(command));

	if ((image == NULL) || (commandlist == NULL)){
		printf("can't allocate memory\n");
		exit(1);
	}

	read(pic, image, xsize*ysize*3);

	calculate_commands(commandlist);
}

void sendpacket(){
	if ((sent = send(sockfd, packet, strlen(packet), MSG_CONFIRM)) == -1) {
		fprintf(stderr, "send: %s\n", strerror(errno));
	}
	
	memset(&packet, 0, sizeof(packet));
}

void sendpixel(uint16_t x, uint16_t y){
	if (strlen(packet) + strlen(commandlist[x + y*xsize]) >= PACKETSIZE){
		sendpacket();
	}
	
	strcat(packet, commandlist[x + y*xsize]);
}


int main(int argc, char *argv[]){
	if (argc != 4 && argc != 6){
		printf("Usage: %s [host] [port] [image] <[xoffset] [yoffset]>\n", argv[0]);
		return 1;
	}

	srand(1);

	if (argc == 6){
		xoffset = atoi(argv[4]);
		yoffset = atoi(argv[5]);
	}

	readimage(argv[3]);

	signal(SIGINT, cleanup);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0){
		fprintf(stderr, "sockfd: %s\n", strerror(errno));
	}

	rv = connect(sockfd, res->ai_addr, res->ai_addrlen);
	if (rv < 0){
		fprintf(stderr, "connect: %s\n", strerror(errno));
	}


	int i, j;

	while(1){
		i = rand()%xsize;
		j = rand()%ysize;
		sendpixel(i, j);
	}

	return 0;
}
