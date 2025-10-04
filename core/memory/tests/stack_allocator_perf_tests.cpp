#include <benchmark/benchmark.h>
#include "stack_allocator.hpp"
#include <thread>
#include <vector>
#include <future>

using namespace pynovage::memory;

// Single-threaded temporary allocation pattern
static void BM_StackAllocator_SingleThread(benchmark::State& state) {
    LockFreeStackAllocator allocator(1024 * 1024);  // 1MB
    
    for (auto _ : state) {
        // Get initial marker
        auto start_mark = allocator.getMarker();
        
        // Allocate 100 objects of varying sizes
        std::vector<void*> ptrs;
        ptrs.reserve(100);
        
        for (int i = 0; i < 100; ++i) {
            void* ptr = allocator.allocate(i * 16 + 16);  // 16 to 1600 bytes
            benchmark::DoNotOptimize(ptr);
            ptrs.push_back(ptr);
        }
        
        // Verify we can access all allocations
        for (void* ptr : ptrs) {
            benchmark::DoNotOptimize(ptr);
        }
        
        // Unwind all allocations at once
        allocator.unwind(start_mark);
    }
}
BENCHMARK(BM_StackAllocator_SingleThread);

// Multi-threaded temporary allocation pattern
static void BM_StackAllocator_MultiThread(benchmark::State& state) {
    LockFreeStackAllocator allocator(4 * 1024 * 1024);  // 4MB for multiple threads
    
    for (auto _ : state) {
        state.PauseTiming();
        
        // Create 4 threads that perform temporary allocations
        std::vector<std::future<void>> threads;
        for (int t = 0; t < 4; ++t) {
            threads.push_back(std::async(std::launch::async, [&allocator]() {
                // Each thread gets its own marker
                auto thread_mark = allocator.getMarker();
                
                std::vector<void*> ptrs;
                ptrs.reserve(100);
                
                // Allocate 100 objects
                for (int i = 0; i < 100; ++i) {
                    void* ptr = allocator.allocate(i * 16 + 16);  // 16 to 1600 bytes
                    benchmark::DoNotOptimize(ptr);
                    ptrs.push_back(ptr);
                }
                
                // Use the allocations
                for (void* ptr : ptrs) {
                    benchmark::DoNotOptimize(ptr);
                }
                
                // Unwind thread's allocations
                allocator.unwind(thread_mark);
            }));
        }
        
        state.ResumeTiming();
        
        // Wait for all threads to complete
        for (auto& future : threads) {
            future.wait();
        }
    }
}
BENCHMARK(BM_StackAllocator_MultiThread);

// Frame simulation with render commands
static void BM_StackAllocator_FrameSimulation(benchmark::State& state) {
    LockFreeStackAllocator allocator(16 * 1024 * 1024);  // 16MB for frame data
    
    // Simulate typical frame command sizes
    const std::size_t sizes[] = {
        32,    // Draw call (small)
        64,    // Draw call (medium)
        128,   // Draw call (large)
        256,   // Material update
        512,   // Mesh update
        1024,  // Texture update
        2048   // Buffer update
    };
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (auto _ : state) {
        // Start frame
        auto frame_mark = allocator.getMarker();
        
        // Simulate 1000 render commands per frame
        std::vector<void*> commands;
        commands.reserve(1000);
        
        for (int i = 0; i < 1000; ++i) {
            // Pick a random-ish command size
            std::size_t size = sizes[i % num_sizes];
            void* cmd = allocator.allocate(size);
            benchmark::DoNotOptimize(cmd);
            commands.push_back(cmd);
        }
        
        // Process commands
        for (void* cmd : commands) {
            benchmark::DoNotOptimize(cmd);
        }
        
        // End frame - unwind all frame allocations
        allocator.unwind(frame_mark);
    }
}
BENCHMARK(BM_StackAllocator_FrameSimulation);