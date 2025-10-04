#include <benchmark/benchmark.h>
#include "threading_benchmarks.hpp"

using namespace pynovage::memory::benchmarks;

// Register threading benchmarks
BENCHMARK(BM_ThreadSafety)->Range(2, 8);

BENCHMARK_MAIN();