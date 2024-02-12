#include "include/responder_service.hpp"
#include "include/socket.hpp"

ResponderService::ResponderService(NeighborStore& neighbors) : neighbors(neighbors) {
    this->logger = std::make_unique<Logger>("./responder_service.log");
}

void ResponderService::start() {
    auto sock = Socket();
    this->logger->log("Created socket " + std::to_string(sock.fd));
    sock.setLogger([this](std::string msg) { this->logger->log(msg); });

    /**
     * Bind the socket using one of the ports.
     * Using a range of ports in case a port is already taken.
    */
    int portMin = 4320;
    int portMax = 4330;
    in_addr_t addr = INADDR_ANY;
    for (int port = portMin; port <= portMax; ++port) {
        try {
            sock.bindSock(addr, port);
            this->logger->log("Socket bound on port " + std::to_string(port));
            break;
        } catch(std::runtime_error& err) {
            this->logger->log("Filed to bind socket on port " + std::to_string(port) + ", error: " + err.what());
        }
    }

    /**
     * Listen for incoming messages
    */
    sock.startListening();
    this->logger->log("Started listening");
    while (true) {
        try {
            auto newSock = sock.waitConnection(); // This is blocking!
            /**
             * Respond with the service ID
            */
            try {
                newSock.writeMessage("my-service-123");
                this->logger->log("Sent a response");
            } catch(std::runtime_error& err) {
                this->logger->log(err.what());
            }
        } catch (std::runtime_error& err) {
            this->logger->log(err.what());
        }
    }
}