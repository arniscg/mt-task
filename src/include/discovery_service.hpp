#pragma once

#include "neighbor_store.hpp"
#include "logger.hpp"

/**
 * A service responsible for discovering devices in local network
 *  and checking if these devices are running our service.
*/
class DiscoveryService {
    public:
        DiscoveryService(NeighborStore& neighbors);

        void start();

    private:
        NeighborStore& neighbors;
        std::unique_ptr<Logger> logger;
};