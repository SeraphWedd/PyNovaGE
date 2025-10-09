#include <gtest/gtest.h>
#include "memory/allocator.h"

using namespace PyNovaGE;

class AllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// SystemAllocator Tests
TEST_F(AllocatorTest, SystemAllocatorBasicAllocation) {
    SystemAllocator allocator;
    
    void* ptr = allocator.allocate(1024);
    ASSERT_NE(ptr, nullptr);
    EXPECT_GT(allocator.getTotalAllocated(), 0);
    
    // Test alignment
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr) % 16, 0);
    
    allocator.deallocate(ptr);
}

TEST_F(AllocatorTest, SystemAllocatorCustomAlignment) {
    SystemAllocator allocator;
    
    void* ptr = allocator.allocate(1024, 32);
    ASSERT_NE(ptr, nullptr);
    
    // Test 32-byte alignment
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr) % 32, 0);
    
    allocator.deallocate(ptr);
}

TEST_F(AllocatorTest, SystemAllocatorZeroSize) {
    SystemAllocator allocator;
    
    void* ptr = allocator.allocate(0);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(AllocatorTest, SystemAllocatorStatistics) {
    SystemAllocator allocator;
    
    EXPECT_EQ(allocator.getTotalAllocated(), 0);
    EXPECT_EQ(allocator.getPeakAllocated(), 0);
    
    void* ptr1 = allocator.allocate(1024);
    size_t allocated_after_first = allocator.getTotalAllocated();
    EXPECT_GT(allocated_after_first, 0);
    EXPECT_EQ(allocator.getPeakAllocated(), allocated_after_first);
    
    void* ptr2 = allocator.allocate(512);
    EXPECT_GT(allocator.getTotalAllocated(), allocated_after_first);
    EXPECT_GE(allocator.getPeakAllocated(), allocator.getTotalAllocated());
    
    allocator.deallocate(ptr1);
    allocator.deallocate(ptr2);
    
    // Reset statistics
    allocator.resetStats();
    EXPECT_EQ(allocator.getTotalAllocated(), 0);
    EXPECT_EQ(allocator.getPeakAllocated(), 0);
}

// LinearAllocator Tests
TEST_F(AllocatorTest, LinearAllocatorBasicAllocation) {
    LinearAllocator allocator(4096);
    
    void* ptr1 = allocator.allocate(1024);
    ASSERT_NE(ptr1, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr1) % 16, 0);
    
    void* ptr2 = allocator.allocate(512);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_GT(ptr2, ptr1); // Should be allocated after ptr1
    
    EXPECT_EQ(allocator.getCurrentUsage(), 1024 + 512);
}

TEST_F(AllocatorTest, LinearAllocatorAlignment) {
    LinearAllocator allocator(4096);
    
    // Allocate unaligned size
    void* ptr1 = allocator.allocate(17);
    ASSERT_NE(ptr1, nullptr);
    
    // Next allocation should still be aligned
    void* ptr2 = allocator.allocate(32, 32);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr2) % 32, 0);
}

TEST_F(AllocatorTest, LinearAllocatorOutOfMemory) {
    LinearAllocator allocator(1024);
    
    void* ptr1 = allocator.allocate(512);
    ASSERT_NE(ptr1, nullptr);
    
    void* ptr2 = allocator.allocate(512);
    ASSERT_NE(ptr2, nullptr);
    
    // Should fail - not enough space
    void* ptr3 = allocator.allocate(128);
    EXPECT_EQ(ptr3, nullptr);
}

TEST_F(AllocatorTest, LinearAllocatorReset) {
    LinearAllocator allocator(4096);
    
    void* ptr = allocator.allocate(1024);
    ASSERT_NE(ptr, nullptr);
    EXPECT_GT(allocator.getCurrentUsage(), 0);
    
    allocator.reset();
    EXPECT_EQ(allocator.getCurrentUsage(), 0);
    
    // Should be able to allocate from beginning again
    void* ptr2 = allocator.allocate(1024);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_EQ(ptr2, ptr); // Should be same location
}