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

// Basic allocation/deallocation benchmark
static void BM_BasicOperations(benchmark::State& state) {
    const size_t total_size = 1024 * 1024;  // 1MB
    const size_t alloc_size = state.range(0);
    
    for (auto _ : state) {
        state.PauseTiming();
        DefragmentingAllocator allocator(total_size);
        std::vector<void*> ptrs;
        ptrs.reserve(100);
        state.ResumeTiming();
        
        // Allocate and deallocate in sequence
        for (int i = 0; i < 100; ++i) {
            void* ptr = allocator.allocate(alloc_size);
            if (!ptr) {
                throw std::runtime_error("Allocation failed");
            }
            ptrs.push_back(ptr);
        }
        
        for (void* ptr : ptrs) {
            allocator.deallocate(ptr);
        }
        // Ensure clean teardown for this iteration
        allocator.reset();
    }
}

// Fragmentation stress test
static void BM_FragmentationStress(benchmark::State& state) {
    const size_t total_size = 1024 * 1024;  // 1MB
    const size_t num_allocs = state.range(0);
    std::vector<size_t> sizes = {16, 32, 64, 128, 256};  // Various sizes
    
    for (auto _ : state) {
        state.PauseTiming();
        DefragmentingAllocator allocator(total_size);
        std::vector<void*> ptrs(num_allocs);
        std::mt19937 rng(std::random_device{}());
        state.ResumeTiming();
        
        // First phase: Allocate
        for (size_t i = 0; i < num_allocs; ++i) {
            size_t size = sizes[i % sizes.size()];
            void* ptr = allocator.allocate(size);
            if (!ptr) {
                throw std::runtime_error("Allocation failed");
            }
            ptrs[i] = ptr;
        }
        
        // Second phase: Random deallocations
        std::vector<size_t> indices(num_allocs);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), rng);
        
        for (size_t i = 0; i < num_allocs / 2; ++i) {
            allocator.deallocate(ptrs[indices[i]]);
            ptrs[indices[i]] = nullptr;
        }
        
        // Third phase: Allocate again
        for (size_t i = 0; i < num_allocs / 2; ++i) {
            if (!ptrs[indices[i]]) {
                size_t size = sizes[i % sizes.size()];
                void* ptr = allocator.allocate(size);
                if (!ptr) {
                    throw std::runtime_error("Reallocation failed");
                }
                ptrs[indices[i]] = ptr;
            }
        }
        
        // Cleanup
        for (void* ptr : ptrs) {
            if (ptr) allocator.deallocate(ptr);
        }
        // Ensure clean teardown for this iteration
        allocator.reset();
    }
}

// Alignment test
static void BM_AlignmentTest(benchmark::State& state) {
    const size_t total_size = 1024 * 1024;  // 1MB
    const size_t align = state.range(0);
    
    for (auto _ : state) {
        state.PauseTiming();
        DefragmentingAllocator allocator(total_size);
        std::vector<void*> ptrs;
        ptrs.reserve(100);
        state.ResumeTiming();
        
        for (int i = 0; i < 100; ++i) {
            void* ptr = allocator.allocate(64, align);
            if (!ptr) {
                throw std::runtime_error("Allocation failed");
            }
            // Verify alignment
            if (reinterpret_cast<std::uintptr_t>(ptr) % align != 0) {
                throw std::runtime_error("Alignment requirement not met");
            }
            ptrs.push_back(ptr);
        }
        
        for (void* ptr : ptrs) {
            allocator.deallocate(ptr);
        }
        // Ensure clean teardown for this iteration
        allocator.reset();
    }
}

} // namespace benchmarks
} // namespace memory
} // namespace pynovage