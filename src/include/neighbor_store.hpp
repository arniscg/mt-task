#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <memory>
#include "logger.hpp"

struct Neighbor {
    std::string ip;
    std::string mac;
};

/**
 * A thread-safe store for neighbors (protected by std::mutex).
 * Uses IP strings as keys.
 * 
 * Thread-safe because originally the idea was that the ResponderService,
 *  which runs is separate thread would also access it, but in the end it was not neccessary.
*/
class NeighborStore {
    public:
        NeighborStore(std::mutex& mutex);

        Neighbor get(std::string ip);

        void add(std::string ip, std::string mac);

        /**
         * Write the store to file
        */
        void dump(std::string filePath);

        void clear();

    private:
        std::unordered_map<std::string, Neighbor> neighbors;
        std::mutex& mutex;
        std::unique_ptr<Logger> logger;
};