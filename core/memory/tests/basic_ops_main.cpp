#include <benchmark/benchmark.h>
#include "basic_ops_benchmarks.hpp"

using namespace pynovage::memory::benchmarks;

// Register basic operations benchmarks
BENCHMARK(BM_BasicOperations)->Range(8, 8<<10);
BENCHMARK(BM_FragmentationStress)->Range(64, 512);
BENCHMARK(BM_AlignmentTest)->Range(8, 64);

BENCHMARK_MAIN();