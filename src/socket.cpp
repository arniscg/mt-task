#include "socket.hpp"
#include <sys/socket.h> // socket
#include <errno.h> // errno
#include <string.h> // strerror
#include <netinet/ip.h> // IPPROTO_UDP
#include <unistd.h> // close
#include <netdb.h> // getaddrinfo
#include <iostream>
#include <arpa/inet.h>

#define LISTEN_BACKLOG 100
#define SERVICE_ID "my-service-123"
#define SERVICE_ID_LEN 15

Socket::Socket() {
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1) {
        throw std::runtime_error("Failed to create a socket, error: " + std::string(strerror(errno)));
    }

    std::cout << "Created socket " << this->fd << std::endl;
}

/**
 * Wrap an existing file descriptor.
*/
Socket::Socket(int fd) {
    this->fd = fd;
}

Socket::~Socket() {
    /**
     * Note: When program is killed, the socket wont be closed.
     *    The OS will handle cleaning up the sockets.
     *    Alternatively one could implement signal handling to close sockets before exiting.
    */
    if (close(this->fd) == -1) {
        /**
         * If a socket can't be closed there is nothing else to do, just print a warning.
        */
        std::cout << "Failed to close socket << " << this->fd << ", error: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Socket " << this->fd << " closed" << std::endl;
    }
}

void handleRequests() {
    auto sock = Socket();

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // Receive messages from any IP
    addr.sin_port = htons(4123);
    if (bind(sock.fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        throw std::runtime_error("Failed to bind a socket, error: " + std::string(strerror(errno)));
    }

    std::cout << "Socket bound" << std::endl;

    if (listen(sock.fd, LISTEN_BACKLOG) == -1) {
        throw std::runtime_error("Failed to start socket listening, error: " + std::string(strerror(errno)));
    }

    std::cout << "Socket listening" << std::endl;

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    while (true) {
        int newSockFd = accept(sock.fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (newSockFd == -1) {
            std::cout << "Socket accept error: " << strerror(errno) << std::endl;
            continue;
        }

        auto newSock = Socket(newSockFd);

        std::cout << "Accepted new connection, socket: " << newSock.fd << std::endl;

        /**
         * Now respond with the service ID
        */
        char buffer[SERVICE_ID_LEN];
        memcpy(&buffer, SERVICE_ID, SERVICE_ID_LEN);
        int ret = write(newSock.fd, &buffer, SERVICE_ID_LEN);
        if (ret == -1) {
            std::cout << "Failed to write: " << strerror(errno) << std::endl;
        } else if (ret < SERVICE_ID_LEN) {
            std::cout << "Wrote too little data. Expected " << SERVICE_ID_LEN << ", sent" << ret << std::endl;
        }

        std::cout << "Responded with service ID" << std::endl;
    }
}

bool checkIpAddress(const char* ip) {
    auto sock = Socket();

    /**
     * Add timeout for the second read method.
     * Service is expected to respond withing 1 second to be discovered.
     */
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(sock.fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof(tv)) == -1) {
        throw std::runtime_error("Failed to set socket timeout, error: " + std::string(strerror(errno)));
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4123);
    inet_pton(AF_INET, ip, &(addr.sin_addr));


    if (connect(sock.fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        std::cout << "Failed to connect, error: " << strerror(errno) << std::endl;
        return false;
    }

    /**
     * Now wait for a response
    */
    std::cout << "Waiting for response..." << std::endl;
    char buffer[SERVICE_ID_LEN];
    if (read(sock.fd, &buffer, SERVICE_ID_LEN) == -1) {
        std::cout << "Failed to read data, error: " << strerror(errno) << std::endl;
        return false;
    }
    std::cout << "Received data: " << buffer << std::endl;

    return strcmp(buffer, SERVICE_ID) == 0;
}