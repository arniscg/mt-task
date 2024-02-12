#include "include/neighbor_store.hpp"

NeighborStore::NeighborStore(std::mutex& mutex) : mutex(mutex) {
    this->logger = std::make_unique<Logger>("./neighbor_store.log");
}

Neighbor NeighborStore::get(std::string ip) {
    std::lock_guard<std::mutex> guard(this->mutex);

    if (this->neighbors.count(ip)) {
        return this->neighbors.at(ip);
    }

    throw std::runtime_error("IP doesn't exist: " + ip);
}

void NeighborStore::add(std::string ip, std::string mac) {
    std::lock_guard<std::mutex> guard(this->mutex);

    this->neighbors[ip] = {
        .ip = ip,
        .mac = mac
    };

    this->logger->log("Added neighbor with IP: " + ip);
}

void NeighborStore::dump(std::string filePath) {
    std::lock_guard<std::mutex> guard(this->mutex);

    std::ofstream stream(filePath, std::ios::out);

    for (auto& neigh : this->neighbors) {
        stream << neigh.second.ip << "," << neigh.second.mac << std::endl;
    }

    this->logger->log("Wrote neighbors to file");
}

void NeighborStore::clear() {
    std::lock_guard<std::mutex> guard(this->mutex);

    this->neighbors.clear();

    this->logger->log("Neighbors cleared");
}