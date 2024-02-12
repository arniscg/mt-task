#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <errno.h>
#include <string.h>

std::vector<std::string> getGatewayAddresses() {
    /**
     * Use /proc/net/route to find gateway addresses.
     * Expected format:
     * Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask            MTU     Window  IRTT                                                       
     * enp34s0 00000000        0100A8C0        0003    0       0       100     00000000        0       0       0                                                                          
     * enp34s0 0000FEA9        00000000        0001    0       0       1000    0000FFFF        0       0       0
    */
    std::vector<std::string> ips;
    std::ifstream stream("/proc/net/route", std::ios::in);

    std::string line;
    std::getline(stream, line); // Skip header line
    while(std::getline(stream, line)) {
        auto values = splitString(line);

        if (values.size() != 11) {
            throw std::runtime_error("Failed to parse /proc/net/route, expected 11 values, got " + std::to_string(values.size()));
        }

        // Gateway IP is sotred in little endian hex format
        // Transform into decimal point string
        std::string gatewayIpHex = values[2];
        long addr = strtol(gatewayIpHex.c_str(), NULL, 16);
        char buff[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &addr, buff, sizeof(buff)) == 0) {
            throw std::runtime_error("Invalid IP address");
            std::cerr << "Failed to parse IP: " << gatewayIpHex << std::endl;
        } else {
            ips.push_back(std::string(buff));
        }
    }

    return ips;
}

std::vector<std::string> getLocalAddresses() {
    /**
     * Implementation is based on https://man7.org/linux/man-pages/man3/getifaddrs.3.html
     * Only minor changes done to get only what I need (the IPs)
    */
    std::vector<std::string> ips;
    ifaddrs *ifaddr;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        freeifaddrs(ifaddr);
        throw std::runtime_error("Failed to getifaddrs(), error: " + std::string(strerror(errno)));
    }

    /* Walk through linked list, maintaining head pointer so we
        can free list later. */
    for (ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                            sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST,
                    NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                freeifaddrs(ifaddr);
                throw std::runtime_error("etnameinfo() failed: " + std::string(gai_strerror(s)));
            }

            ips.push_back(std::string(host));
        }
    }

    freeifaddrs(ifaddr);

    return ips;
}

std::vector<std::string> splitString(std::string str) {
    std::vector<std::string> words;
    bool readingValue = true;
    std::string word = "";
    for (const auto& c: str) {
        if (isspace(c)) {
            if (readingValue) {
                readingValue = false;
                words.push_back(word);
                word = "";
            }
            continue;
        }

        readingValue = true;
        word += c;
    }

    return words;
}