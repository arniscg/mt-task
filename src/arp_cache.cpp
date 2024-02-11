#include <fstream>
#include <iostream>
#include <sstream>
#include <array>
#include "arp_cache.hpp"

std::vector<ArpEntry> parseArpCache() {
    std::vector<ArpEntry> result;

    std::ifstream stream("/proc/net/arp", std::ios::in);

    int lineId = 0;
    std::string line;
    while(std::getline(stream, line)) {
        if (lineId == 0) {
            ++lineId;
            continue;
        }

        // There are 6 values in one line
        std::array<std::string,6> values;
        int valueId = 0;
        bool readingValue = true;

        for (const auto& c: line) {
            if (c == ' ') {
                if (readingValue) {
                    readingValue = false;
                    ++valueId;
                }
                continue;
            }

            readingValue = true;

            if (valueId >= 6) {
                throw std::runtime_error("Failed to parse ARP cache, more than 6 values detected");
            }

            values[valueId].push_back(c);
        }

        result.push_back(ArpEntry{
            .ipAddress = values[0],
            .hwAddress = values[3],
            .device = values[5]
        });

        ++lineId;
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