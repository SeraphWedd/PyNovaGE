#include <benchmark/benchmark.h>
#include "linear_allocator.hpp"
#include "pool_allocator.hpp"
#include "stack_allocator.hpp"
#include <vector>
#include <memory>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <numeric>

using namespace pynovage::memory;

// ===================== Comparative Benchmarks =====================

static void BM_StandardMalloc(benchmark::State& state) {
    const size_t num_allocs = state.range(0);
    const size_t alloc_size = state.range(1);
    std::vector<void*> ptrs;
    ptrs.reserve(num_allocs);

    for (auto _ : state) {
        for (size_t i = 0; i < num_allocs; ++i) {
            void* ptr = std::malloc(alloc_size);
            benchmark::DoNotOptimize(ptr);
            ptrs.push_back(ptr);
        }

        // Cleanup
        for (void* ptr : ptrs) {
            std::free(ptr);
        }
        ptrs.clear();
    }

    state.SetItemsProcessed(state.iterations() * num_allocs);
    state.SetBytesProcessed(state.iterations() * num_allocs * alloc_size);
}

static void BM_LinearAllocator(benchmark::State& state) {
    const size_t num_allocs = state.range(0);
    const size_t alloc_size = state.range(1);
    const size_t total_size = num_allocs * alloc_size * 2; // Extra space for alignment
    std::vector<void*> ptrs;
    ptrs.reserve(num_allocs);

    for (auto _ : state) {
        LinearAllocator<16> allocator(total_size);
        
        for (size_t i = 0; i < num_allocs; ++i) {
            void* ptr = allocator.allocate(alloc_size);
            benchmark::DoNotOptimize(ptr);
            ptrs.push_back(ptr);
        }

        // Reset
        allocator.reset();
        ptrs.clear();
    }

    state.SetItemsProcessed(state.iterations() * num_allocs);
    state.SetBytesProcessed(state.iterations() * num_allocs * alloc_size);
}

// Test both allocators with different sizes
BENCHMARK(BM_StandardMalloc)->Ranges({{8, 1024}, {8, 1024}});
BENCHMARK(BM_LinearAllocator)->Ranges({{8, 1024}, {8, 1024}});

// ===================== Cache Behavior Benchmarks =====================

static void BM_CacheEfficiency(benchmark::State& state) {
    const size_t cache_line = 64;
    const size_t num_allocations = state.range(0);
    const size_t stride = state.range(1);
    std::vector<char*> ptrs;
    ptrs.reserve(num_allocations);
    
    // Setup allocator outside the benchmark loop
    const size_t total_size = num_allocations * (cache_line + stride + 64); // Extra padding for alignment
    LinearAllocator<64> allocator(total_size);
    std::mt19937 rng(std::random_device{}());
    
    for (auto _ : state) {
        state.PauseTiming();
        ptrs.clear();
        allocator.reset();
        
        // Allocate cache-line sized blocks with stride
        for (size_t i = 0; i < num_allocations; ++i) {
            char* ptr = static_cast<char*>(allocator.allocate(cache_line + stride));
            if (!ptr) {
                state.SkipWithError("Allocation failed");
                return;
            }
            ptrs.push_back(ptr);
        }
        
        // Create and shuffle indices before timing
        std::vector<size_t> indices(ptrs.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), rng);
        state.ResumeTiming();
        
        // Sequential access
        for (char* ptr : ptrs) {
            for (size_t i = 0; i < cache_line; i += 16) {
                ptr[i] = 1;
                benchmark::DoNotOptimize(ptr[i]);
            }
        }
        
        // Random access
        for (size_t idx : indices) {
            char* ptr = ptrs[idx];
            for (size_t i = 0; i < cache_line; i += 16) {
                ptr[i] = 2;
                benchmark::DoNotOptimize(ptr[i]);
            }
        }
    }
    
    state.SetItemsProcessed(state.iterations() * num_allocations * 2); // Sequential + Random
    state.SetBytesProcessed(state.iterations() * num_allocations * cache_line * 2);
}

// Test with different allocation counts and strides
// Test with smaller ranges first to ensure stability
BENCHMARK(BM_CacheEfficiency)->Ranges({{32, 1024}, {0, 32}});

// ===================== Fragmentation Analysis =====================

// Test with smaller ranges first to ensure stability

// ===================== Memory Overhead Analysis =====================

static void BM_MemoryOverhead(benchmark::State& state) {
    const size_t num_allocs = state.range(0);
    const size_t base_size = state.range(1);
    std::vector<void*> ptrs;
    ptrs.reserve(num_allocs);

    struct AllocStats {
        size_t requested;
        size_t actual;
        size_t operations;
    };

    for (auto _ : state) {
        state.PauseTiming();
        // Test different allocator types
        LinearAllocator<16> linear(num_allocs * base_size * 4);
        ThreadLocalPoolAllocator pool({{16, 1024}, {64, 256}, {256, 64}});
        
        std::vector<AllocStats> stats(3); // One for each allocator
        ptrs.clear();
        state.ResumeTiming();

        // Linear allocator test
        for (size_t i = 0; i < num_allocs; ++i) {
            size_t size = base_size * (1 + (i % 4));
            void* ptr = linear.allocate(size);
            benchmark::DoNotOptimize(ptr);
            stats[0].requested += size;
            stats[0].actual += alignTo(size, 16);
            stats[0].operations++;
            ptrs.push_back(ptr);
        }
        ptrs.clear();
        linear.reset();

        // Pool allocator test
        for (size_t i = 0; i < num_allocs; ++i) {
            size_t size = base_size * (1 + (i % 4));
            void* ptr = pool.allocate(size);
            benchmark::DoNotOptimize(ptr);
            stats[1].requested += size;
            // Actual size is determined by the pool's size class
            stats[1].actual += (size <= 16) ? 16 : (size <= 64) ? 64 : 256;
            stats[1].operations++;
            ptrs.push_back(ptr);
        }
        for (void* ptr : ptrs) {
            pool.deallocate(ptr);
        }
        ptrs.clear();


        // Calculate and report overhead percentages
        for (const auto& stat : stats) {
            double overhead = static_cast<double>(stat.actual - stat.requested) / stat.requested * 100.0;
            benchmark::DoNotOptimize(overhead);
        }
    }
}

BENCHMARK(BM_MemoryOverhead)->Ranges({{32, 512}, {16, 128}});

// ===================== Allocation Pattern Analysis =====================

static void BM_AllocationPatterns(benchmark::State& state) {
    const size_t pattern_length = state.range(0);
    const size_t base_size = state.range(1);
    std::vector<void*> ptrs;
    ptrs.reserve(pattern_length);

    enum class AllocPattern {
        LINEAR_GROWTH,
        FIBONACCI,
        POWER_OF_TWO,
        RANDOM
    };

    for (auto _ : state) {
        state.PauseTiming();
        LinearAllocator<16> linear(pattern_length * base_size * 8);
        ThreadLocalPoolAllocator pool({{16, 1024}, {64, 256}, {256, 64}});
        std::mt19937 rng(std::random_device{}());
        ptrs.clear();
        state.ResumeTiming();

        // Test different allocation patterns
        for (int pattern = 0; pattern < 4; ++pattern) {
            for (size_t i = 0; i < pattern_length; ++i) {
                size_t size = base_size;
                switch (static_cast<AllocPattern>(pattern)) {
                    case AllocPattern::LINEAR_GROWTH:
                        size = base_size * (i + 1);
                        break;
                    case AllocPattern::FIBONACCI:
                        size = base_size * (i <= 1 ? 1 : (i - 1) + (i - 2));
                        break;
                    case AllocPattern::POWER_OF_TWO:
                        size = base_size * (1 << (i % 8));
                        break;
                    case AllocPattern::RANDOM:
                        size = base_size * (1 + (rng() % 16));
                        break;
                }

                void* ptr = nullptr;
                switch (i % 3) {
                    case 0:
                        ptr = linear.allocate(size);
                        break;
                    case 1:
                        ptr = pool.allocate(size);
                        break;
                    case 2:
                        continue; // Skip this allocation (previously defrag)
                        break;
                }
                benchmark::DoNotOptimize(ptr);
                ptrs.push_back(ptr);
            }

            // Cleanup
            for (size_t i = 0; i < ptrs.size(); ++i) {
                if (i % 3 == 0) {
                    // Linear allocator cleanup happens in reset
                } else if (i % 3 == 1) {
                    pool.deallocate(ptrs[i]);
                } else {
                    // Previously defrag deallocate
                }
            }
            ptrs.clear();
            linear.reset();
        }
    }
}

BENCHMARK(BM_AllocationPatterns)->Ranges({{32, 512}, {16, 128}});
