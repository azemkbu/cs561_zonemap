#include <fstream>
#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <chrono>
#include "zonemaps.h"
#include "zonemaps.cpp"

// Specify your path of workload file here
std::string kInputDataPath = "./workload.dat";
std::string kPointQueriesPath = "./point_queries.txt";
std::string kRangeQueriesPath = "./range_queries.txt";
const uint32_t kRuns = 3;

static std::mt19937 rng(std::random_device{}()); // C++17 shuffle RNG

void loadPointQueries(const std::string& input_queries_path, std::vector<int>& queries) {
  queries.clear();
  std::ifstream infile(input_queries_path);
  int tmp;
  while (infile >> tmp) queries.push_back(tmp);

  // shuffle indexes
  std::shuffle(queries.begin(), queries.end(), rng);
}

struct RangeQuery {
  int low;
  int high;
};

void loadRangeQueries(const std::string& path, std::vector<RangeQuery>& queries) {
  queries.clear();
  std::ifstream infile(path);
  int low, high;
  while (infile >> low >> high) {
    if (low > high) std::swap(low, high);
    queries.push_back({low, high});
  }

  std::shuffle(queries.begin(), queries.end(), rng);
}

int main(int argc, char **argv) {
  // read data
  std::ifstream ifs;
  std::vector<int> data;

  ifs.open(kInputDataPath, std::ios::binary);
  ifs.seekg(0, std::ios::end);
  size_t filesize = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  data.resize(filesize / sizeof(int));
  ifs.read((char *)data.data(), filesize);

  //1. ----------------------------- initialize zonemap and build -----------------------------
  //build zonemap
  zonemap<int> zones(data, (uint)data.size() / 100);

  //2. ----------------------------- point queries -----------------------------
  std::vector<int> queries;
  loadPointQueries(kPointQueriesPath, queries);

  volatile size_t res = 0; // prevents compiler from optimizing away work

  // ---------------- point queries ----------------
  auto start_pq = std::chrono::high_resolution_clock::now();
  for (size_t r = 0; r < kRuns; r++) {
    size_t local = 0;
    for (int pq : queries) {
      local += zones.query(pq);
    }
    res += local;

    // shuffle indexes
    std::shuffle(queries.begin(), queries.end(), rng);
  }
  auto stop_pq = std::chrono::high_resolution_clock::now();
  auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
  std::cout << "Time taken to perform point queries from zonemap = "
  << (duration_pq.count() * 1.0 / kRuns) << " microseconds\n";

  // ---------------- range queries ----------------
  std::vector<RangeQuery> range_queries;
  loadRangeQueries(kRangeQueriesPath, range_queries);

  auto start_rq = std::chrono::high_resolution_clock::now();
  for (size_t r = 0; r < kRuns; r++) {
    size_t local = 0;
    for (const auto& rq : range_queries) {
      local += zones.range_query_count(rq.low, rq.high);
    }
    res += local;
    std::shuffle(range_queries.begin(), range_queries.end(), rng);
  }
  auto stop_rq = std::chrono::high_resolution_clock::now();
  auto duration_rq = std::chrono::duration_cast<std::chrono::microseconds>(stop_rq - start_rq);
  std::cout << "Time taken to perform range query from zonemap = "
            << (duration_rq.count() * 1.0 / kRuns) << " microseconds\n";
  return 0;
}
