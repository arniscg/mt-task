#include "src/socket.hpp"
#include <iostream>

int main(int argc, char const *argv[]) {
    try {
        bool success = checkIpAddress("192.168.0.111");
        // bool success = checkIpAddress("loopback");

        if (success) {
            std::cout << "This is our service" << std::endl;
        } else {
            std::cout << "This is not our service" << std::endl;
        }
    } catch (std::runtime_error& error) {
        std::cout << error.what() << std::endl;
        exit(1);
    }

    return 0;
}