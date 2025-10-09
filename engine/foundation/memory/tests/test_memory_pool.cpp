#include <gtest/gtest.h>
#include "memory/memory_pool.h"

using namespace PyNovaGE;

TEST(MemoryPoolTest, BasicAllocationAndDeallocation) {
    MemoryPool pool(64, 10);
    
    EXPECT_EQ(pool.getAllocatedBlocks(), 0);
    EXPECT_EQ(pool.getFreeBlocks(), 10);
    
    void* ptr1 = pool.allocate(32);
    ASSERT_NE(ptr1, nullptr);
    EXPECT_EQ(pool.getAllocatedBlocks(), 1);
    EXPECT_EQ(pool.getFreeBlocks(), 9);
    
    void* ptr2 = pool.allocate(64);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_EQ(pool.getAllocatedBlocks(), 2);
    
    pool.deallocate(ptr1);
    EXPECT_EQ(pool.getAllocatedBlocks(), 1);
    EXPECT_EQ(pool.getFreeBlocks(), 9);
    
    pool.deallocate(ptr2);
    EXPECT_EQ(pool.getAllocatedBlocks(), 0);
    EXPECT_EQ(pool.getFreeBlocks(), 10);
}

TEST(MemoryPoolTest, ExhaustsPool) {
    MemoryPool pool(64, 2);
    
    void* ptr1 = pool.allocate(32);
    void* ptr2 = pool.allocate(32);
    void* ptr3 = pool.allocate(32); // Should fail
    
    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_EQ(ptr3, nullptr);
    
    EXPECT_EQ(pool.getAllocatedBlocks(), 2);
    EXPECT_EQ(pool.getFreeBlocks(), 0);
}

TEST(MemoryPoolTest, OwnsPointer) {
    MemoryPool pool(64, 5);
    
    void* ptr = pool.allocate(32);
    EXPECT_TRUE(pool.ownsPointer(ptr));
    
    void* external_ptr = malloc(64);
    EXPECT_FALSE(pool.ownsPointer(external_ptr));
    
    pool.deallocate(ptr);
    free(external_ptr);
}