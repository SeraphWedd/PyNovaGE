#pragma once

#include <benchmark/benchmark.h>
#include "defrag_allocator.hpp"
#include <vector>
#include <thread>
#include <atomic>

namespace pynovage {
namespace memory {
namespace benchmarks {

// Thread safety test
static void BM_ThreadSafety(benchmark::State& state) {
    const size_t total_size = 1024 * 1024;  // 1MB
    const int num_threads = static_cast<int>(state.range(0));
    
    for (auto _ : state) {
        state.PauseTiming();
        DefragmentingAllocator allocator(total_size);
        std::vector<std::thread> threads;
        std::atomic<int> errors{0};
        state.ResumeTiming();
        
        // Create threads that perform allocations and deallocations
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&allocator, &errors]() {
                try {
                    std::vector<void*> thread_ptrs;
                    thread_ptrs.reserve(50);
                    
                    for (int i = 0; i < 50 && errors == 0; ++i) {  // Stop if errors occurred
                        void* ptr = nullptr;
                        try {
                            ptr = allocator.allocate(64);
                            if (!ptr) {
                                throw std::runtime_error("Thread allocation failed");
                            }
                            thread_ptrs.push_back(ptr);
                            
                            // Random deallocations
                            if (i % 3 == 0 && !thread_ptrs.empty()) {
                                size_t idx = thread_ptrs.size() - 1;
                                allocator.deallocate(thread_ptrs[idx]);
                                thread_ptrs.pop_back();
                            }
                        } catch (const std::exception& e) {
                            if (ptr) {
                                try {
                                    allocator.deallocate(ptr);
                                } catch (...) {}
                            }
                            throw;  // Re-throw to outer handler
                        }
                    }
                    
                    // Cleanup remaining allocations
                    for (void* ptr : thread_ptrs) {
                        try {
                            allocator.deallocate(ptr);
                        } catch (...) {
                            errors++;
                        }
                    }
                } catch (const std::exception&) {
                    errors++;
                }
            });
        }
        
        // Wait for all threads to complete
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        if (errors > 0) {
            throw std::runtime_error("Thread safety test failed");
        }
        // Ensure clean teardown for this iteration
        allocator.reset();
    }
}

} // namespace benchmarks
} // namespace memory
} // namespace pynovage