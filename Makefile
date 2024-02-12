CC=g++

all: service

service: service.o src/socket.o src/arp_cache.o src/icmp_scan.o src/logger.o src/utils.o src/neighbor_store.o src/discovery_service.o src/responder_service.o
	$(CC) -o service service.o src/socket.o src/arp_cache.o src/icmp_scan.o src/logger.o src/utils.o src/neighbor_store.o src/discovery_service.o src/responder_service.o

clean:
	rm -f *.o src/*.o service