#include "src/socket.hpp"
#include <iostream>
#include <string.h>

int main(int argc, char const *argv[]) {
    try {
        auto socket = Socket();
        in_addr_t addr = INADDR_LOOPBACK;
        socket.checkAddress(addr);
    } catch (std::runtime_error& error) {
        std::cout << error.what() << std::endl;
        exit(1);
    }

    return 0;
}