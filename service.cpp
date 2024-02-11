#include "src/socket.hpp"
#include "src/arp_cache.hpp"
#include "src/icmp_scan.hpp"
#include "src/logger.hpp"
#include <thread>
#include <string>
#include <unordered_map>
#include <mutex>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <chrono>
#include <memory>

struct Neighbor {
    std::string ip;
    std::string mac;
};

/**
 * A thread-safe store for neighbors (protected by std::mutex).
 * Uses IP strings as keys
*/
class NeighborStore {
    public:
        NeighborStore(std::mutex& mutex) : mutex(mutex) {
            this->logger = std::make_unique<Logger>("./neighbor_store.log");
        }

        Neighbor get(std::string ip) {
            std::lock_guard<std::mutex> guard(this->mutex);

            if (this->neighbors.count(ip)) {
                return this->neighbors.at(ip);
            }

            throw std::runtime_error("IP doesn't exist: " + ip);
        }

        void add(std::string ip, std::string mac) {
            std::lock_guard<std::mutex> guard(this->mutex);

            neighbors[ip] = {
                .ip = ip,
                .mac = mac
            };

            this->logger->log("Added neighbor with IP: " + ip);
        }

        void dump(std::string filePath) {
            std::lock_guard<std::mutex> guard(this->mutex);

            std::ofstream stream(filePath, std::ios::out);

            for (auto& neigh : this->neighbors) {
                stream << neigh.second.ip << "," << neigh.second.mac << std::endl;
            }

            this->logger->log("Wrote neighbors to file");
        }

    private:
        std::unordered_map<std::string, Neighbor> neighbors;
        std::mutex& mutex;
        std::unique_ptr<Logger> logger;
};

/**
 * A service responsible for listening for incomming messages and responding
 *  with a simple message indicating that it is indeed our service.
 * Also updates neighbor store when a message is received.
*/
class ResponderService {
    public:
        ResponderService(NeighborStore& neighbors) : neighbors(neighbors) {
            this->logger = std::make_unique<Logger>("./responder_service.log");
        }

        void start() {
            auto sock = Socket();
            this->logger->log("Created socket " + std::to_string(sock.fd));

            /**
             * Bind the socket using one of the ports.
             * Using a range of ports in case a port is already used.
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
                    auto connection = sock.waitConnection(); // This is blocking!
                    auto& newSock = std::get<0>(connection);
                    auto& ipAddress = std::get<1>(connection);
                    this->logger->log("Received connection from IP: " + ipAddress);
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

    private:
        NeighborStore& neighbors;
        std::unique_ptr<Logger> logger;
};

/**
 * A service responsible for discovering devices in local network
 *  and checking if these devices are running our service.
*/
class DiscoveryService {
    public:
        DiscoveryService(NeighborStore& neighbors) : neighbors(neighbors) {
            this->logger = std::make_unique<Logger>("./discovery_service.log");
        }

        void start() {
            while (true) {
                auto ipAddresses = scanLocalIps();

                for (auto& ip : ipAddresses) {
                    int portMin = 4320;
                    int portMax = 4330;
                    int port = portMin;
                    bool isService = false;
                    for (port = portMin; port <= portMax; ++port) {
                        auto sock = Socket();

                        /** 
                         * Add timeout.
                         * Service is expected to respond withing 1 second to be discovered.
                         */
                        sock.setReadTimeout(1);
                        sock.setSendTimeout(1);
                    
                        try {
                            this->logger->log("Starting to check IP: " + ip + ":" + std::to_string(port));
                            sock.connectTo(ip, port);

                            /**
                             * Wait for response
                            */
                            try {
                                auto response = sock.waitMessage();
                                this->logger->log("Received a message: " + response);
                                if (response == "my-service-123") {
                                    isService = true;
                                    try {
                                        auto mac = arpMacAddressLookup(ip);
                                        this->neighbors.add(ip, mac);
                                    } catch (std::runtime_error& err) {
                                        this->logger->log(err.what());
                                        this->neighbors.add(ip, "unknown");
                                    }
                                    break;
                                }
                            } catch (std::runtime_error& err) {
                                this->logger->log(err.what());
                            }

                            if (isService) break;
                        } catch (std::runtime_error& err) {
                            this->logger->log(err.what());
                        }
                    }

                    if (isService) {
                        this->logger->log("It's our service");
                    } else {
                        this->logger->log("It's not our service");
                    }
                }

                this->neighbors.dump("./neighbors.txt");

                // Repeat after 30s
                std::this_thread::sleep_for(std::chrono::seconds(30));
            }
        }

    private:
        NeighborStore& neighbors;
        std::unique_ptr<Logger> logger;
};

int main(int argc, char const *argv[]) {
    std::mutex neighMutex;
    auto neighStore = NeighborStore(neighMutex);
    auto responderService = ResponderService(neighStore);
    auto discoveryService = DiscoveryService(neighStore);

    auto responderServiceThread = std::thread(&ResponderService::start, &responderService);
    auto discoveryServiceThread = std::thread(&DiscoveryService::start, &discoveryService);

    responderServiceThread.join();
    discoveryServiceThread.join();

    return 0;
}
