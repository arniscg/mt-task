#include "include/socket.hpp"
#include "../config.h"
#include <sys/socket.h> // socket
#include <errno.h> // errno
#include <string.h> // strerror
#include <netinet/ip.h> // IPPROTO_UDP
#include <unistd.h> // close
#include <netdb.h> // getaddrinfo
#include <iostream>
#include <arpa/inet.h>
#include <string>

#define LISTEN_BACKLOG 100

Socket::Socket() {
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1) {
        throw std::runtime_error("Failed to create a socket, error: " + std::string(strerror(errno)));
    }

    // this->log("Created socket << " + std::to_string(this->fd));
}

/**
 * Wrap an existing file descriptor.
*/
Socket::Socket(int fd) {
    this->fd = fd;
}

Socket::Socket(int type, int protocol) {
    this->fd = socket(AF_INET, type, protocol);
    if (this->fd == -1) {
        throw std::runtime_error("Failed to create a socket, error: " + std::string(strerror(errno)));
    }
}

Socket::~Socket() {
    /**
     * Note: When program is killed, the socket wont be closed.
     *    The OS will handle cleaning up the sockets.
     *    Alternatively one could implement signal handling to close sockets before exiting.
    */
    if (close(this->fd) == -1) {
        /**
         * If a socket can't be closed there is nothing else to do
        */
        // std::cerr << "Failed to close socket " << this->fd << ", error: " << strerror(errno) << std::endl;
        this->log("Failed to close socket " + std::to_string(this->fd) + ", error: " + std::string(strerror(errno)));
    }

    // this->log("Closed socket " + std::to_string(this->fd));
}

void Socket::setLogger(std::function<void(std::string)> log) {
    this->log = log;
}

void Socket::bindSock(in_addr_t &ipAddr, int port)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(ipAddr);
    addr.sin_port = htons(port);
    if (bind(this->fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        throw std::runtime_error("Failed to bind a socket, error: " + std::string(strerror(errno)));
    }
}

void Socket::startListening() {
    if (listen(this->fd, LISTEN_BACKLOG) == -1) {
        throw std::runtime_error("Failed to start socket listening, error: " + std::string(strerror(errno)));
    }
}

void Socket::setReadTimeout(int timeout) {
    timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if (setsockopt(this->fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof(tv)) == -1) {
        throw std::runtime_error("Failed to set socket timeout, error: " + std::string(strerror(errno)));
    }
}

void Socket::setSendTimeout(int timeout) {
    timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if (setsockopt(this->fd, SOL_SOCKET, SO_SNDTIMEO, (const char*) &tv, sizeof(tv)) == -1) {
        throw std::runtime_error("Failed to set socket timeout, error: " + std::string(strerror(errno)));
    }
}

void Socket::connectTo(std::string ip, int port) {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr));

    if (connect(this->fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        throw std::runtime_error("Failed to connect, error: " + std::string(strerror(errno)));
    }
}

void Socket::writeMessage(std::string msg) {
    int ret = write(this->fd, msg.c_str(), msg.size());
    if (ret == -1) {
        throw std::runtime_error("Failed to write: " + std::string(strerror(errno)) + ", socket: " + std::to_string(this->fd));
    } else if (ret < msg.size()) {
        std::cerr << "Wrote too little data. Expected " << msg.size() << ", sent" << ret << std::endl;
    }
}

std::string Socket::waitMessage() {
    char buffer[sizeof(SERVICE_NAME)] = {0};
    if (read(this->fd, &buffer, sizeof(buffer)) == -1) {
        throw std::runtime_error("Failed to read data, error: " + std::string(strerror(errno)));
    }
    return std::string(buffer);
}

Socket Socket::waitConnection() {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int newSockFd = accept(this->fd, (struct sockaddr *) &clientAddr, &clientAddrLen);
    if (newSockFd == -1) {
        throw std::runtime_error("Failed to accept connection, error: " + std::string(strerror(errno)));
    }
    this->log("Accepted connection");

    char buff[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &clientAddr.sin_addr, buff, sizeof(buff)) == 0) {
        throw std::runtime_error("Invalid IP address");
    }

    auto newSocket = Socket(newSockFd);
    newSocket.setLogger(this->log);

    return newSocket;
}