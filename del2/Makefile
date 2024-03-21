CC = gcc
CFLAGS = -std=c11 -Wall

all: UDP_server UDP_client

UDP_server: del2/UDP_server.c
	$(CC) $(CFLAGS) -o UDP_server del2/UDP_server.c

UDP_client: del2/UDP_client.c
	$(CC) $(CFLAGS) -o UDP_client del2/UDP_client.c

clean:
	rm -f UDP_server UDP_client
