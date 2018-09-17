#include "opt_size_vector.hh"

#include <array>
#include <benchmark/benchmark.h>
#include <cstdio>
#include <vector>

struct fat_data { 
  explicit fat_data(size_t i = 0) : s{} {}
  fat_data(fat_data&) = default;
  fat_data(fat_data&&) = default;
  fat_data& operator=(fat_data&) = default;
  fat_data& operator=(const fat_data&) = default;

  std::array<size_t, 100> s;
};

const auto linear = [](int n) -> double {
  return n;
};

using csci4117::opt_size_vector;

using std::vector;

template <class T, class A>
static void BM_fill(benchmark::State& state) {
  size_t n = state.range(0);
  for (auto _ : state) {
    A arr;
    for (size_t i = 0; i < n; i++) {
      arr.push_back(T(i));
    }
  }

  state.SetComplexityN(n);
}

template <class T, class A>
static void BM_fill_and_read(benchmark::State& state) {
  size_t n = state.range(0);
  for (auto _ : state) {
    A arr;
    for (size_t i = 0; i < n; i++) {
      arr.push_back(T(i));
    }

    for (size_t i = 0; i < n; i++) {
      benchmark::DoNotOptimize(arr[i]);
    }
  }

  state.SetComplexityN(n);
}

BENCHMARK_TEMPLATE(BM_fill, int, vector<int>)->Range(1, 1U << 28U)->Complexity(linear);
BENCHMARK_TEMPLATE(BM_fill, int, opt_size_vector<int>)->Range(1, 1U << 28U)->Complexity(linear);
BENCHMARK_TEMPLATE(BM_fill_and_read, int, vector<int>)->Range(1, 1U << 28U)->Complexity(linear);
BENCHMARK_TEMPLATE(BM_fill_and_read, int, opt_size_vector<int>)->Range(1, 1U << 28U)->Complexity(linear);

BENCHMARK_TEMPLATE(BM_fill, fat_data, vector<fat_data>)->Range(1, 1U << 22U)->Complexity(linear);
BENCHMARK_TEMPLATE(BM_fill, fat_data, opt_size_vector<fat_data>)->Range(1, 1U << 22U)->Complexity(linear);
BENCHMARK_TEMPLATE(BM_fill_and_read, fat_data, vector<fat_data>)->Range(1, 1U << 22U)->Complexity(linear);
BENCHMARK_TEMPLATE(BM_fill_and_read, fat_data, opt_size_vector<fat_data>)->Range(1, 1U << 22U)->Complexity(linear);

BENCHMARK_MAIN();
