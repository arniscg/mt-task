#include "src/socket.hpp"
#include <iostream>

int main(int argc, char const *argv[]) {
    try {
        auto socket = Socket();
        socket.handleRequests();
    } catch (std::runtime_error& error) {
        std::cout << error.what() << std::endl;
        exit(1);
    }

    return 0;
}