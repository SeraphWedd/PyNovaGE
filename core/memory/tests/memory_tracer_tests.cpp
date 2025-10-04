#include <gtest/gtest.h>
#include "memory_tracer.hpp"
#include "pool_allocator.hpp"
#include <thread>
#include <chrono>
#include <vector>

using namespace pynovage::memory;

class MemoryTracerTest : public ::testing::Test {
protected:
    void SetUp() override {
        MemoryTracer::instance().enable(true);
    }

    void TearDown() override {
        MemoryTracer::instance().enable(false);
    }
};

TEST_F(MemoryTracerTest, BasicAllocationTracking) {
    // Create a pool allocator for testing
    std::vector<ThreadLocalPoolAllocator::SizeClass> size_classes = {
        {64, 256, 8}  // 64-byte blocks
    };
    ThreadLocalPoolAllocator allocator(size_classes);

    // Allocate and track some memory
    void* ptr1 = allocator.allocate(64);
    void* ptr2 = allocator.allocate(64);

    // Check allocation history
    {
        auto history = MemoryTracer::instance().getAllocationHistory();
        EXPECT_EQ(history.size(), 0);  // Nothing deallocated yet
    }

    // Deallocate one pointer
    allocator.deallocate(ptr1);

    // Check history again
    {
        auto history = MemoryTracer::instance().getAllocationHistory();
        EXPECT_EQ(history.size(), 1);  // One deallocation
    }

    // Cleanup
    allocator.deallocate(ptr2);
}

TEST_F(MemoryTracerTest, HotColdAnalysis) {
    std::vector<ThreadLocalPoolAllocator::SizeClass> size_classes = {
        {64, 256, 8}
    };
    ThreadLocalPoolAllocator allocator(size_classes);

    // Allocate memory
    void* hot_ptr = allocator.allocate(64);
    void* cold_ptr = allocator.allocate(64);

    // Access hot_ptr many times
    for (int i = 0; i < 2000; ++i) {
        MemoryTracer::instance().recordAccess(hot_ptr, 64);
    }

    // Access cold_ptr few times
    for (int i = 0; i < 10; ++i) {
        MemoryTracer::instance().recordAccess(cold_ptr, 64);
    }

    // Check hot allocations
    auto hot_allocs = MemoryTracer::instance().getHotAllocations();
    EXPECT_GT(hot_allocs.size(), 0);
    if (!hot_allocs.empty()) {
        EXPECT_EQ(hot_allocs[0].address, hot_ptr);
    }

    // Cleanup
    allocator.deallocate(hot_ptr);
    allocator.deallocate(cold_ptr);
}

TEST_F(MemoryTracerTest, MultiThreadedTracking) {
    std::vector<ThreadLocalPoolAllocator::SizeClass> size_classes = {
        {64, 256, 8}
    };
    ThreadLocalPoolAllocator allocator(size_classes);

    constexpr int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    std::vector<void*> pointers(NUM_THREADS);

    // Create threads that allocate and access memory
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&allocator, i, &pointers]() {
            pointers[i] = allocator.allocate(64);
            
            // Simulate some memory access
            for (int j = 0; j < 500; ++j) {
                MemoryTracer::instance().recordAccess(pointers[i], 64);
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }

    // Wait for threads to finish
    for (auto& t : threads) {
        t.join();
    }

    // Cleanup
    for (void* ptr : pointers) {
        allocator.deallocate(ptr);
    }

    // Check history
    auto history = MemoryTracer::instance().getAllocationHistory();
    EXPECT_EQ(history.size(), NUM_THREADS);  // All allocations should be in history
}

TEST_F(MemoryTracerTest, StackTraceCapture) {
    std::vector<ThreadLocalPoolAllocator::SizeClass> size_classes = {
        {64, 256, 8}
    };
    ThreadLocalPoolAllocator allocator(size_classes);

    // Allocate and immediately check stack trace
    void* ptr = allocator.allocate(64);
    
    // Get the allocation event
    auto hot_allocs = MemoryTracer::instance().getHotAllocations();
    auto history = MemoryTracer::instance().getAllocationHistory();
    
    bool found_stack_trace = false;
    if (!hot_allocs.empty() && !hot_allocs[0].stack_trace.empty()) {
        found_stack_trace = true;
    }
    
    EXPECT_TRUE(found_stack_trace);

    // Cleanup
    allocator.deallocate(ptr);
}

// Utility to print stack traces for debugging
void printStackTrace(const std::vector<void*>& trace) {
    for (std::size_t i = 0; i < trace.size(); ++i) {
        printf("#%zu: %p\n", i, trace[i]);
    }
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}