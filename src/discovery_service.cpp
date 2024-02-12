#include "include/discovery_service.hpp"
#include "include/utils.hpp"
#include "include/arp_cache.hpp"
#include "include/icmp_scan.hpp"
#include "include/socket.hpp"
#include "../config.h"
#include <thread>
#include <algorithm>

DiscoveryService::DiscoveryService(NeighborStore& neighbors) : neighbors(neighbors) {
    this->logger = std::make_unique<Logger>("./discovery_service.log");
}

void DiscoveryService::start() {
    while (true) {
        auto ipAddresses = scanLocalIps();
        auto gatewayAddresses = getGatewayAddresses();
        auto localAddresses = getLocalAddresses();

        // Clear the neighbor store
        this->neighbors.clear();

        for (auto& ip : ipAddresses) {
            // Ignore gateway addresses
            if (std::find(gatewayAddresses.begin(), gatewayAddresses.end(), ip) != gatewayAddresses.end()) {
                this->logger->log("Skipping address: " + ip);
                continue;
            }

            // Ignore own addresses
            if (std::find(localAddresses.begin(), localAddresses.end(), ip) != localAddresses.end()) {
                this->logger->log("Skipping address: " + ip);
                continue;
            }

            int port = SERVICE_PORT_MIN;
            bool isService = false;
            for (port; port <= SERVICE_PORT_MAX; ++port) {
                auto sock = Socket();
                // this->logger->log("Created socket " + std::to_string(sock.fd));
                sock.setLogger([this](std::string msg) { this->logger->log(msg); });

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
                        if (response == SERVICE_NAME) {
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
        std::this_thread::sleep_for(std::chrono::seconds(SERVICE_PERIOD));
    }
}