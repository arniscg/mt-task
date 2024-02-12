#pragma once

#include "neighbor_store.hpp"
#include "logger.hpp"

/**
 * A service responsible for listening for incomming messages and responding
 *  with a simple message indicating that it is indeed our service.
 * Also updates neighbor store when a message is received.
*/
class ResponderService {
    public:
        ResponderService(NeighborStore& neighbors);

        void start();

    private:
        NeighborStore& neighbors;
        std::unique_ptr<Logger> logger;
};