#include "src/arp_cache.hpp"
#include <iostream>

int main(int argc, char const *argv[]) {
    std::string ip = "192.168.0.101";
    auto mac = arpMacAddressLookup(ip);
    
    if (mac) {
        std::cout << mac.value() << std::endl;
    } else {
        std::cout << "Not found" << std::endl;
    }

    return 0;
}
