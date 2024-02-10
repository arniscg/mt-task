CC=g++

all: server client

server: server.o src/socket.o
	$(CC) -o server server.o src/socket.o

client: client.o src/socket.o
	$(CC) -o client client.o src/socket.o

clean:
	rm -f *.o src/*.o server client