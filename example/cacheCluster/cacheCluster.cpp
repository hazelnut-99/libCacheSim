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

bool CacheCluster::get(request_t *req, const std::string& lb) {
    uint64_t idx;

    if (lb == "consistent_hash") {
        // Use consistent hashing to find the server index
        idx = ch_ring_get_server_from_uint64(req->obj_id, this->_ring);
    } else if (lb == "random") {
        // Use a random index within the bounds of _cache_servers_vec
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, this->_cache_servers_vec.size() - 1);
        idx = dist(gen);
    } else {
        throw std::invalid_argument("Unknown load balancing strategy: " + lb);
    }

    // Find the server and process the request
    CacheServer &server = this->_cache_servers_vec.at(idx);
    bool hit = server.get(req);

    return hit;
}

CacheCluster::~CacheCluster() {
  if (_ring != NULL) {
    ch_ring_destroy_ring(_ring);
  }
}
}  // namespace CDNSimulator
