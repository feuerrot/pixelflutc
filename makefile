CC = gcc
CFLAGS = -std=gnu99 -Os -Wall -Wextra -Wpedantic


image: image.c
	$(CC) $(CFLAGS) -o image image.c
