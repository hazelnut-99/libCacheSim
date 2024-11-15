//
//  cacheCluster.cpp
//  CDNSimulator
//
//  Created by Juncheng Yang on 7/13/17.
//  Copyright © 2017 Juncheng. All rights reserved.
//

#include "include/cacheCluster.hpp"
#include <random>
#include "include/consistentHash.h"
// #include <iomanip>

namespace CDNSimulator {

class CacheCluster {
public:
    CacheCluster()
        : gen(std::random_device{}()),
          dist(0, _cache_servers_vec.size() - 1) {} // Initialize RNG and distribution

    ~CacheCluster() {
        if (_ring != NULL) {
            ch_ring_destroy_ring(_ring);
        }
    }

    bool get_random(request_t *req);
    bool get_consistent_hash(request_t *req);

private:
    std::vector<CacheServer> _cache_servers_vec;
    ch_ring_t *_ring = nullptr;
    std::mt19937 gen; // Random number generator
    std::uniform_int_distribution<size_t> dist; // Uniform distribution
};

bool CacheCluster::get_random(request_t *req) {
    // Generate a random index using pre-initialized generator and distribution
    uint64_t idx = dist(gen);

    // Find the server and process the request
    CacheServer &server = this->_cache_servers_vec.at(idx);
    return server.get(req);
}

bool CacheCluster::get_consistent_hash(request_t *req) {
    uint64_t idx;
    // Use consistent hashing to find the server index
    idx = ch_ring_get_server_from_uint64(req->obj_id, this->_ring);

    // Find the server and process the request
    CacheServer &server = this->_cache_servers_vec.at(idx);
    return server.get(req);
}

}  // namespace CDNSimulator
