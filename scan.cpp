#include <sys/socket.h> // socket
#include <errno.h> // errno
#include <string.h> // strerror
#include <netinet/ip.h> // IPPROTO_UDP
#include <netinet/ip_icmp.h> 
#include <unistd.h> // close
#include <netdb.h> // getaddrinfo
#include <iostream>
#include <arpa/inet.h>
#include "src/socket.hpp"

#define PING_PKT_SIZE 64
#define RECV_TIMEOUT 1

unsigned short checksum(void* b, int len) {
    unsigned short* buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;
 
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

struct ping_pkt {
    struct icmphdr hdr;
    char msg[PING_PKT_SIZE - sizeof(icmphdr)];
};

int main(int argc, char const *argv[]) {
    /**
     * Note: SOCK_RAW requires elevated permissions.
     * Main reason why this has to be executed with "sudo".
    */
    int socketfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (socketfd == -1) {
        throw std::runtime_error("Failed to create a socket, error: " + std::string(strerror(errno)));
    }

    auto sock = Socket(socketfd);

    std::cout << "Created socket " << sock.fd << std::endl;

    int ttl = 60;

    if (setsockopt(sock.fd, SOL_SOCKET, IP_TTL, &ttl, sizeof(ttl)) == -1) {
        throw std::runtime_error("Failed to set socket TTL, error: " + std::string(strerror(errno)));
    }

    struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT;
    tv.tv_usec = 0;
    if (setsockopt(sock.fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof(tv)) == -1) {
        throw std::runtime_error("Failed to set socket timeout, error: " + std::string(strerror(errno)));
    }

    int n = 0;
    while (true) {
        ping_pkt pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.hdr.type = ICMP_ECHO;
        pkt.hdr.un.echo.id = getpid();
        pkt.hdr.un.echo.sequence = 0;

        int i = 0;
        for (i = 0; i < sizeof(pkt.msg) - 1; i++)
            pkt.msg[i] = i + '0';

        pkt.msg[i] = 0;
        pkt.hdr.un.echo.sequence = n++;
        pkt.hdr.checksum = checksum(&pkt, sizeof(pkt));

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr = { .s_addr = htonl(INADDR_ALLHOSTS_GROUP) };

        if (sendto(sock.fd, &pkt, sizeof(pkt), 0, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
            throw std::runtime_error("Failed to send ICMP message, error: " + std::string(strerror(errno)));
        }

        std::cout << "Sent ICMP message" << std::endl;

        while (true) {
            char rbuffer[128];
            sockaddr_in r_addr;
            socklen_t len = sizeof(r_addr);
            if (recvfrom(sock.fd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr *) &r_addr, &len) == -1) {
                break;
                // throw std::runtime_error("Failed to receive ICMP message, error: " + std::string(strerror(errno)));
            }

            char ip[64];
            inet_ntop(AF_INET, &r_addr.sin_addr.s_addr, ip, len); 

            std::cout << "Received ICMP message from " << ip << std::endl;
        }
    }

    usleep(1000000);
}