#include <fstream>
#include <iostream>
#include <sstream>
#include <array>
#include "include/arp_cache.hpp"
#include "include/utils.hpp"

std::vector<ArpEntry> parseArpCache() {
    std::vector<ArpEntry> result;

    std::ifstream stream("/proc/net/arp", std::ios::in);

    std::string line;
    std::getline(stream, line); // Skip header line
    while(std::getline(stream, line)) {
        auto values = splitString(line);

        if (values.size() != 5) {
            throw std::runtime_error("Failed to parse ARP cache, expected 5 values, got " + std::to_string(values.size()));
        }

        result.push_back(ArpEntry{
            .ipAddress = values[0],
            .hwAddress = values[3],
            .device = values[5]
        });
    }

    return result;
}

std::string arpMacAddressLookup(std::string ipAddress) {
    auto arpCache = parseArpCache();

    for (auto const& entry : arpCache) {
        if (entry.ipAddress == ipAddress) {
            return entry.hwAddress;
        }
    }

    throw std::runtime_error("ARP cache has no entry for address " + ipAddress);
}