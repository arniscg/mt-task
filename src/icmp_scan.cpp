#include "include/icmp_scan.hpp"
#include "include/socket.hpp"
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>
#include <string>

std::vector<std::string> scanLocalIps() {
    /**
     * Note: SOCK_RAW requires elevated permissions.
     * Main reason why this has to be executed with "sudo".
    */
    auto sock = Socket(SOCK_RAW, IPPROTO_ICMP);

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

    PingPacket packet;
    memset(&packet, 0, sizeof(packet));
    packet.hdr.type = ICMP_ECHO;
    packet.hdr.un.echo.id = getpid();

    int i = 0;
    for (i = 0; i < sizeof(packet.msg) - 1; i++)
        packet.msg[i] = i + '0';

    packet.msg[i] = 0;
    packet.hdr.checksum = checksum(&packet, sizeof(packet));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = { .s_addr = htonl(INADDR_ALLHOSTS_GROUP) };

    if (sendto(sock.fd, &packet, sizeof(packet), 0, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        throw std::runtime_error("Failed to send ICMP message, error: " + std::string(strerror(errno)));
    }

    std::vector<std::string> ipAddresses;
    while (true) {
        char responseBuffer[128];
        sockaddr_in responseAddr;
        socklen_t len = sizeof(responseAddr);
        if (recvfrom(sock.fd, responseBuffer, sizeof(responseBuffer), 0, (struct sockaddr *) &responseAddr, &len) == -1) {
            // Stop when no messages can be retrieved anymore
            break;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &responseAddr.sin_addr.s_addr, ip, len); 

        std::string ipStr(ip);
        // if (ipStr == "192.168.0.1") continue;

        ipAddresses.push_back(ipStr);
    }

    return ipAddresses;
}

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