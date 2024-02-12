CC=g++

all: service

service: service.o src/socket.o src/arp_cache.o src/icmp_scan.o src/logger.o src/utils.o
	$(CC) -o service service.o src/socket.o src/arp_cache.o src/icmp_scan.o src/logger.o src/utils.o

clean:
	rm -f *.o src/*.o service