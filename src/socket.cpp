#include "socket.hpp"
#include <sys/socket.h> // socket
#include <errno.h> // errno
#include <string.h> // strerror
#include <netinet/ip.h> // IPPROTO_UDP
#include <unistd.h> // close
#include <netdb.h> // getaddrinfo
#include <iostream>

#define LISTEN_BACKLOG 100

Socket::Socket() {
    this->createSocket(); // TODO: Call this only when needed, not in constructor
}

Socket::~Socket() {
    this->closeSocket(this->socketfd);
}

void Socket::handleRequests() {
    this->prepareServerSocket();

    sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    while (true) {
        int socket = accept(this->socketfd, (struct sockaddr *) &addr, &addr_len);
        if (socket == -1) {
            std::cout << "Socket accept error: " << strerror(errno) << std::endl;
            continue;
        }

        std::cout << "Accepted new connection, socket: " << socket << std::endl;
        this->closeSocket(socket);
    }
}

void Socket::checkAddress(in_addr_t& inAddr) {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(inAddr);
    addr.sin_port = htons(4123);


    if (connect(this->socketfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        std::cout << "Failed to connect, error: " << strerror(errno) << std::endl;
    }

    /**
     * Whether connection failed or succeeded, close the socket and create a new one
    */
    this->closeSocket(this->socketfd);
    this->createSocket();
}

void Socket::createSocket() {
    this->socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd == -1) {
        throw std::runtime_error("Failed to create a socket, error: " + std::string(strerror(errno)));
    }

    std::cout << "Socket created" << std::endl;
}

void Socket::prepareServerSocket() {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(4123);
    if (bind(this->socketfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        throw std::runtime_error("Failed to bind a socket, error: " + std::string(strerror(errno)));
    }

    std::cout << "Socket bound" << std::endl;

    if (listen(this->socketfd, LISTEN_BACKLOG) == -1) {
        throw std::runtime_error("Failed to start socket listening, error: " + std::string(strerror(errno)));
    }

    std::cout << "Socket listening" << std::endl;
}

void Socket::closeSocket(int socket) {
    if (close(socket) == -1) {
        std::cout << "Failed to close socket << " << socket << ", error: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Socket " << socket << " closed" << std::endl;
    }
}