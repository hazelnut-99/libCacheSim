#include <inttypes.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include "include/cache.hpp"
#include "include/cacheCluster.hpp"
#include "include/cacheServer.hpp"
#include "libCacheSim.h"

namespace CDNSimulator {

void simulate(int argc, char *argv[]) {
  const char *data_path = "../../../data/twitter_cluster52.csv";
  uint64_t n_server = 10;
  uint64_t server_dram_cache_size = 1 * MiB;
  uint64_t server_disk_cache_size = 10 * MiB;
  std::string algo = "lru";

  
  // Parse command-line arguments
  if (argc > 1) {
    data_path = argv[1];
  } else {
    printf("No trace path provided\n");
    exit(1);
  }

  if (argc > 2) {
    n_server = std::strtoull(argv[2], nullptr, 10);
  }
  if (argc > 3) {
    server_dram_cache_size = std::strtoull(argv[3], nullptr, 10) * MiB;
  }
  if (argc > 4) {
    server_disk_cache_size = std::strtoull(argv[4], nullptr, 10) * MiB;
  }
  if (argc > 5) {
    algo = argv[5];
  }

  // Verify data path
  if (access(data_path, F_OK) == -1) {
    printf("Data file %s does not exist.\n", data_path);
    exit(1);
  }

  // Open trace
  reader_t *reader = setup_reader(data_path, ORACLE_GENERAL_TRACE, NULL);
  
  // print_reader(reader);
  // printf("file size: %lu\n", reader->file_size);
  // printf("n_total_req: %lu\n", reader->n_total_req);
  
  
  const uint32_t hashpower = 20;
  printf(
      "Setting up a cluster of %lu servers, each server has %lu MB DRAM cache "
      "and %lu MB disk cache, using %s as cache replacement algorithm\n",
      (unsigned long)n_server, (unsigned long)(server_dram_cache_size / MiB),
      (unsigned long)(server_disk_cache_size / MiB), algo.c_str());

  CacheCluster cluster(0);

  for (uint64_t i = 0; i < n_server; i++) {
    CacheServer server(i);
    server.add_cache(std::move(Cache(server_dram_cache_size, algo, hashpower)));
    server.add_cache(std::move(Cache(server_disk_cache_size, algo, hashpower)));
    cluster.add_server(std::move(server));
  }

  // Read trace
  request_t *req = new_request();

  uint64_t n_miss = 0, n_miss_byte = 0;
  uint64_t n_req = 0, n_req_byte = 0;

  while (read_trace(reader, req) == 0) {
    bool hit = cluster.get(req);
    if (!hit) {
      n_miss += 1;
      n_miss_byte += req->obj_size;
    }
    n_req += 1;
    n_req_byte += req->obj_size;
  }

  std::cout << n_req << " requests, " << n_miss << " misses, miss ratio: " << (double)n_miss / n_req
            << ", byte miss ratio: " << (double)n_miss_byte / n_req_byte << std::endl;

  close_trace(reader);
}

}  // namespace CDNSimulator

int main(int argc, char *argv[]) {
  try {
    CDNSimulator::simulate(argc, argv);
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    print_stack_trace();
  }

  return 0;
}
