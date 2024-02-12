#include "src/include/responder_service.hpp"
#include "src/include/discovery_service.hpp"
#include <thread>
#include <string>
#include <mutex>

int main(int argc, char const *argv[]) {
    std::mutex neighMutex;
    auto neighStore = NeighborStore(neighMutex);
    auto responderService = ResponderService(neighStore);
    auto discoveryService = DiscoveryService(neighStore);

    /**
     * Run responder service in a separate thread
    */
    auto responderServiceThread = std::thread(&ResponderService::start, &responderService);

    /**
     * Run discovery service in this thread
    */
    discoveryService.start();

    responderServiceThread.join();

    return 0;
}
