CC=g++

all: server client scan arp

server: server.o src/socket.o
	$(CC) -o server server.o src/socket.o

client: client.o src/socket.o
	$(CC) -o client client.o src/socket.o

scan: scan.o src/socket.o
	$(CC) -o scan scan.o src/socket.o

arp: arp.o src/socket.o src/arp_cache.o
	$(CC) -o arp arp.o src/socket.o src/arp_cache.o

clean:
	rm -f *.o src/*.o server client scan arp