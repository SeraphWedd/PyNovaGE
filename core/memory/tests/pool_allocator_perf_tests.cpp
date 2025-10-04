#include <benchmark/benchmark.h>
#include "pool_allocator.hpp"
#include <thread>
#include <vector>
#include <future>

using namespace pynovage::memory;

// Test fixed-size allocations in a single thread
static void BM_PoolAllocator_SingleThread(benchmark::State& state) {
    // Size classes for game-typical allocations
    std::vector<ThreadLocalPoolAllocator::SizeClass> size_classes = {
        {16, 1024, 16},    // Small objects (e.g., particle data)
        {64, 512, 16},     // Medium objects (e.g., transform data)
        {256, 128, 16},    // Large objects (e.g., entity data)
    };
    
    ThreadLocalPoolAllocator allocator(size_classes);
    
    for (auto _ : state) {
        std::vector<void*> ptrs;
        ptrs.reserve(100);
        
        // Allocate 100 objects of each size class
        for (int i = 0; i < 100; ++i) {
            void* p1 = allocator.allocate(16);
            void* p2 = allocator.allocate(64);
            void* p3 = allocator.allocate(256);
            benchmark::DoNotOptimize(p1);
            benchmark::DoNotOptimize(p2);
            benchmark::DoNotOptimize(p3);
            ptrs.push_back(p1);
            ptrs.push_back(p2);
            ptrs.push_back(p3);
        }
        
        // Deallocate all objects
        for (void* ptr : ptrs) {
            allocator.deallocate(ptr);
        }
    }
}
BENCHMARK(BM_PoolAllocator_SingleThread);

// Test mixed-size allocations with high contention
static void BM_PoolAllocator_MultiThread(benchmark::State& state) {
    std::vector<ThreadLocalPoolAllocator::SizeClass> size_classes = {
        {16, 1024, 16},    // Small objects
        {64, 512, 16},     // Medium objects
        {256, 128, 16},    // Large objects
    };
    
    ThreadLocalPoolAllocator allocator(size_classes);
    
    for (auto _ : state) {
        state.PauseTiming();
        
        // Create 4 threads that allocate and deallocate simultaneously
        std::vector<std::future<void>> threads;
        for (int t = 0; t < 4; ++t) {
            threads.push_back(std::async(std::launch::async, [&allocator]() {
                std::vector<void*> ptrs;
                ptrs.reserve(100);
                
                // Allocate 100 objects per thread
                for (int i = 0; i < 100; ++i) {
                    void* p1 = allocator.allocate(16);
                    void* p2 = allocator.allocate(64);
                    void* p3 = allocator.allocate(256);
                    benchmark::DoNotOptimize(p1);
                    benchmark::DoNotOptimize(p2);
                    benchmark::DoNotOptimize(p3);
                    ptrs.push_back(p1);
                    ptrs.push_back(p2);
                    ptrs.push_back(p3);
                }
                
                // Deallocate all objects
                for (void* ptr : ptrs) {
                    allocator.deallocate(ptr);
                }
            }));
        }
        
        state.ResumeTiming();
        
        // Wait for all threads to complete
        for (auto& future : threads) {
            future.wait();
        }
    }
}
BENCHMARK(BM_PoolAllocator_MultiThread);

// Test allocation pattern similar to game entity creation/destruction
static void BM_PoolAllocator_GameScenario(benchmark::State& state) {
    std::vector<ThreadLocalPoolAllocator::SizeClass> size_classes = {
        {16, 1024, 16},    // Transform component
        {32, 512, 16},     // Physics component
        {48, 512, 16},     // Renderer component
        {64, 256, 16},     // Animation component
        {128, 128, 16},    // AI component
        {256, 64, 16},     // Inventory component
    };
    
    ThreadLocalPoolAllocator allocator(size_classes);
    
    struct Entity {
        void* transform;
        void* physics;
        void* renderer;
        void* animation;
        void* ai;
        void* inventory;
    };
    
    for (auto _ : state) {
        std::vector<Entity> entities;
        entities.reserve(1000);
        
        // Simulate creating 1000 entities with different component combinations
        for (int i = 0; i < 1000; ++i) {
            Entity entity;
            
            // All entities have transform and renderer
            entity.transform = allocator.allocate(16);
            entity.renderer = allocator.allocate(48);
            
            // 80% have physics
            if (i % 10 < 8) {
                entity.physics = allocator.allocate(32);
            } else {
                entity.physics = nullptr;
            }
            
            // 50% have animation
            if (i % 2 == 0) {
                entity.animation = allocator.allocate(64);
            } else {
                entity.animation = nullptr;
            }
            
            // 30% have AI
            if (i % 10 < 3) {
                entity.ai = allocator.allocate(128);
            } else {
                entity.ai = nullptr;
            }
            
            // 20% have inventory
            if (i % 10 < 2) {
                entity.inventory = allocator.allocate(256);
            } else {
                entity.inventory = nullptr;
            }
            
            benchmark::DoNotOptimize(entity);
            entities.push_back(entity);
        }
        
        // Simulate entity destruction
        for (const auto& entity : entities) {
            allocator.deallocate(entity.transform);
            allocator.deallocate(entity.renderer);
            if (entity.physics) allocator.deallocate(entity.physics);
            if (entity.animation) allocator.deallocate(entity.animation);
            if (entity.ai) allocator.deallocate(entity.ai);
            if (entity.inventory) allocator.deallocate(entity.inventory);
        }
    }
}
BENCHMARK(BM_PoolAllocator_GameScenario);
