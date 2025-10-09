#include <gtest/gtest.h>
#include "memory/stack_allocator.h"

using namespace PyNovaGE;

TEST(StackAllocatorTest, BasicAllocation) {
    StackAllocator allocator(4096);
    
    void* ptr1 = allocator.allocate(1024);
    ASSERT_NE(ptr1, nullptr);
    EXPECT_GT(allocator.getCurrentUsage(), 0);
    
    void* ptr2 = allocator.allocate(512);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_GT(ptr2, ptr1);
}

TEST(StackAllocatorTest, Markers) {
    StackAllocator allocator(4096);
    
    void* ptr1 = allocator.allocate(1024);
    (void)ptr1; // Suppress unused variable warning
    size_t marker = allocator.pushMarker();
    EXPECT_NE(marker, SIZE_MAX);
    
    void* ptr2 = allocator.allocate(512);
    (void)ptr2; // Suppress unused variable warning
    size_t usage_before_pop = allocator.getCurrentUsage();
    
    allocator.popToMarker(marker);
    EXPECT_LT(allocator.getCurrentUsage(), usage_before_pop);
    
    // Should be able to allocate again from marker point
    void* ptr3 = allocator.allocate(256);
    ASSERT_NE(ptr3, nullptr);
}

TEST(StackAllocatorTest, Reset) {
    StackAllocator allocator(4096);
    
    allocator.allocate(1024);
    allocator.pushMarker();
    allocator.allocate(512);
    
    EXPECT_GT(allocator.getCurrentUsage(), 0);
    
    allocator.reset();
    EXPECT_EQ(allocator.getCurrentUsage(), 0);
}