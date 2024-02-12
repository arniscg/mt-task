CC=g++ -Wall
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
LIBS=-lpthread

all: service

service: service.o $(OBJECTS)
	$(CC) service.o -o service $(OBJECTS) $(LIBS)

clean:
	rm -f *.o src/*.o service