#include <benchmark/benchmark.h>
#include "linear_allocator.hpp"
#include <vector>

using namespace pynovage::memory;

static void BM_LinearAllocator_SmallAllocations(benchmark::State& state) {
    // Use 16-byte alignment for SIMD
    LinearAllocator<16> allocator(1024 * 1024);  // 1MB capacity
    
    for (auto _ : state) {
        allocator.reset();
        for (int i = 0; i < 100; ++i) {
            void* ptr = allocator.allocate(16);
            benchmark::DoNotOptimize(ptr);
        }
    }
}
BENCHMARK(BM_LinearAllocator_SmallAllocations);

static void BM_LinearAllocator_MixedAllocations(benchmark::State& state) {
    // Use 32-byte alignment for AVX
    LinearAllocator<32> allocator(1024 * 1024);  // 1MB capacity
    
    for (auto _ : state) {
        allocator.reset();
        for (int i = 0; i < 50; ++i) {
            const int size = (i == 0 ? 1 : i) * 16;  // Avoid zero-size allocations
            void* ptr = allocator.allocate(size);
            benchmark::DoNotOptimize(ptr);
        }
    }
}
BENCHMARK(BM_LinearAllocator_MixedAllocations);

static void BM_LinearAllocator_LargeAllocations(benchmark::State& state) {
    // Use 64-byte alignment for cache line
    LinearAllocator<64> allocator(16 * 1024 * 1024);  // 16MB capacity
    
    for (auto _ : state) {
        allocator.reset();
        for (int i = 0; i < 100; ++i) {
            void* ptr = allocator.allocate(64 * 1024);  // 64KB blocks
            benchmark::DoNotOptimize(ptr);
        }
    }
}
BENCHMARK(BM_LinearAllocator_LargeAllocations);

static void BM_LinearAllocator_FragmentationTest(benchmark::State& state) {
    LinearAllocator<16> allocator(1024 * 1024);  // 1MB capacity
    std::vector<void*> ptrs;
    ptrs.reserve(100);
    
    for (auto _ : state) {
        allocator.reset();
        ptrs.clear();
        
        // Allocate mixed sizes
        for (int i = 0; i < 100; ++i) {
            void* ptr = allocator.allocate((i % 16 + 1) * 16);  // 16 to 256 bytes
            benchmark::DoNotOptimize(ptr);
            ptrs.push_back(ptr);
        }
        
        // Verify allocations are properly aligned
        for (void* ptr : ptrs) {
            benchmark::DoNotOptimize(ptr);
            assert(reinterpret_cast<std::uintptr_t>(ptr) % 16 == 0);
        }
    }
}
BENCHMARK(BM_LinearAllocator_FragmentationTest);
