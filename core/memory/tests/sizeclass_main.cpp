#include <benchmark/benchmark.h>
#include "sizeclass_benchmarks.hpp"

using namespace pynovage::memory::benchmarks;

// Register size class benchmarks
BENCHMARK(BM_SizeClassAllocation)->DenseRange(0, pynovage::memory::SizeClassManager::NUM_SMALL_CLASSES - 1);
BENCHMARK(BM_MixedSizeWorkload)->Range(64, 512);

BENCHMARK_MAIN();