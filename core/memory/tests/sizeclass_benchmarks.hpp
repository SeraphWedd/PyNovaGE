#pragma once

#include <benchmark/benchmark.h>
#include "defrag_allocator.hpp"
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>

namespace pynovage {
namespace memory {
namespace benchmarks {

// Size class optimization test
static void BM_SizeClassAllocation(benchmark::State& state) {
    const size_t total_size = 1024 * 1024;  // 1MB
    const size_t size_class_index = state.range(0);
    
    // Get actual size for this size class
    const size_t alloc_size = SizeClassManager::getSizeForClass(size_class_index);
    
    for (auto _ : state) {
        state.PauseTiming();
        DefragmentingAllocator allocator(total_size);
        std::vector<void*> ptrs;
        ptrs.reserve(100);
        state.ResumeTiming();
        
        // Perform allocations that should hit the size class system
        for (int i = 0; i < 100; ++i) {
            void* ptr = allocator.allocate(alloc_size);
            if (!ptr) {
                throw std::runtime_error("Size class allocation failed");
            }
            ptrs.push_back(ptr);
        }
        
        // Random deallocations to test free list
        std::vector<size_t> indices(ptrs.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::mt19937 rng(std::random_device{}());
        std::shuffle(indices.begin(), indices.end(), rng);
        
        for (size_t i = 0; i < indices.size() / 2; ++i) {
            allocator.deallocate(ptrs[indices[i]]);
            ptrs[indices[i]] = nullptr;
        }
        
        // Reallocate to test free list reuse
        for (size_t i = 0; i < indices.size() / 2; ++i) {
            if (!ptrs[indices[i]]) {
                void* ptr = allocator.allocate(alloc_size);
                if (!ptr) {
                    throw std::runtime_error("Size class reallocation failed");
                }
                ptrs[indices[i]] = ptr;
            }
        }
        
        // Cleanup
        for (void* ptr : ptrs) {
            if (ptr) allocator.deallocate(ptr);
        }
        
        // Get hit rate stats before reset
        auto stats = allocator.getStats();
        state.counters["HitRate"] = stats.size_class_stats.getHitRate(size_class_index) * 100.0;
        state.counters["TotalAllocs"] = stats.total_allocations;
        state.counters["TotalDeallocs"] = stats.total_deallocations;
        
        // Ensure clean teardown for this iteration
        allocator.reset();
    }
}

// Mixed workload test
static void BM_MixedSizeWorkload(benchmark::State& state) {
    const size_t total_size = 1024 * 1024;  // 1MB
    const size_t num_allocs = state.range(0);
    
    // Mix of size classes and non-size class allocations
    std::vector<size_t> sizes = {
        8,    // Small size class
        64,   // Small size class
        256,  // Small size class
        1024, // Medium size class
        3000, // Non-size class
        6000  // Non-size class
    };
    
    for (auto _ : state) {
        state.PauseTiming();
        DefragmentingAllocator allocator(total_size);
        std::vector<void*> ptrs;
        ptrs.reserve(num_allocs);
        state.ResumeTiming();
        
        // Allocate mix of sizes
        for (size_t i = 0; i < num_allocs; ++i) {
            size_t size = sizes[i % sizes.size()];
            void* ptr = allocator.allocate(size);
            if (!ptr) {
                throw std::runtime_error("Mixed workload allocation failed");
            }
            ptrs.push_back(ptr);
        }
        
        // Random deallocations
        std::vector<size_t> indices(ptrs.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::mt19937 rng(std::random_device{}());
        std::shuffle(indices.begin(), indices.end(), rng);
        
        for (size_t i = 0; i < indices.size() / 2; ++i) {
            allocator.deallocate(ptrs[indices[i]]);
            ptrs[indices[i]] = nullptr;
        }
        
        // Reallocate
        for (size_t i = 0; i < indices.size() / 2; ++i) {
            if (!ptrs[indices[i]]) {
                size_t size = sizes[i % sizes.size()];
                void* ptr = allocator.allocate(size);
                if (!ptr) {
                    throw std::runtime_error("Mixed workload reallocation failed");
                }
                ptrs[indices[i]] = ptr;
            }
        }
        
        // Cleanup
        for (void* ptr : ptrs) {
            if (ptr) allocator.deallocate(ptr);
        }
        
        // Get stats before reset
        auto stats = allocator.getStats();
        double total_hit_rate = 0.0;
        size_t size_class_count = 0;
        for (size_t i = 0; i < SizeClassManager::TOTAL_SIZE_CLASSES; ++i) {
            if (stats.size_class_stats.allocations[i] > 0) {
                total_hit_rate += stats.size_class_stats.getHitRate(i);
                size_class_count++;
            }
        }
        state.counters["AvgHitRate"] = size_class_count > 0 ? (total_hit_rate / size_class_count) * 100.0 : 0.0;
        state.counters["FragCycles"] = stats.total_fragmentation_cycles;
        state.counters["BlocksMerged"] = stats.total_blocks_merged;
        
        // Ensure clean teardown for this iteration
        allocator.reset();
    }
}

} // namespace benchmarks
} // namespace memory
} // namespace pynovage