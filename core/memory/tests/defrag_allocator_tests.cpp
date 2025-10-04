#include <gtest/gtest.h>
#include "defrag_allocator.hpp"
#include <vector>
#include <random>

using namespace pynovage::memory;

class DefragAllocatorTest : public ::testing::Test {
protected:
    static constexpr size_t POOL_SIZE = 1024;  // 1KB pool for testing
    std::unique_ptr<DefragmentingAllocator> allocator;

    void SetUp() override {
        allocator = std::make_unique<DefragmentingAllocator>(POOL_SIZE);
    }

    void TearDown() override {
        allocator.reset();
    }

    // Helper to verify an allocation
    void* allocateAndVerify(size_t size, size_t alignment = alignof(std::max_align_t)) {
        void* ptr = allocator->allocate(size, alignment);
        EXPECT_NE(ptr, nullptr) << "Allocation failed";
        if (ptr) {
            // Verify alignment
            EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr) % alignment, 0);
            
            // Get and verify the header
            DefragHeader* header = DefragHeader::getHeader(ptr);
            EXPECT_NE(header, nullptr) << "Failed to get header for payload at " << ptr;
            if (header) {
                EXPECT_TRUE(header->isValid()) << "Invalid header: " << header->debugString();
                EXPECT_FALSE(header->is_free) << "Block marked free: " << header->debugString();
                EXPECT_GE(header->size, size) << "Block too small: " << header->debugString();
                EXPECT_EQ(header->alignment, alignment) << "Wrong alignment: " << header->debugString();
            }
        }
        return ptr;
    }

    // Helper to verify deallocation
    void deallocateAndVerify(void* ptr) {
        if (!ptr) return;
        DefragHeader* header = DefragHeader::getHeader(ptr);
        ASSERT_NE(header, nullptr) << "Failed to get header for payload at " << ptr;
        EXPECT_TRUE(header->isValid()) << "Invalid header before free: " << header->debugString();
        EXPECT_FALSE(header->is_free) << "Block already marked free: " << header->debugString();
        
        allocator->deallocate(ptr);
        
        // Header should now be marked as free
        EXPECT_TRUE(header->is_free) << "Block not marked free after deallocation: " << header->debugString();
        EXPECT_TRUE(header->isValid()) << "Invalid header after free: " << header->debugString();
    }
};

// Basic allocation test
TEST_F(DefragAllocatorTest, BasicAllocation) {
    void* ptr = allocateAndVerify(16);
    ASSERT_NE(ptr, nullptr);
    deallocateAndVerify(ptr);
}

// Test multiple allocations
TEST_F(DefragAllocatorTest, MultipleAllocations) {
    std::vector<void*> ptrs;
    
    // Allocate several blocks
    for (size_t i = 0; i < 5; i++) {
        void* ptr = allocateAndVerify(16);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    
    // Deallocate in order
    for (void* ptr : ptrs) {
        deallocateAndVerify(ptr);
    }
}

// Test merging of adjacent free blocks
TEST_F(DefragAllocatorTest, MergeFreeBlocks) {
    std::vector<void*> ptrs;
    
    // Allocate three blocks
    for (int i = 0; i < 3; i++) {
        void* ptr = allocateAndVerify(16);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    
    // Get the initial size of the middle block
    DefragHeader* middle_header = DefragHeader::getHeader(ptrs[1]);
    size_t initial_size = middle_header->size;
    
    // Free blocks in order - they should merge
    deallocateAndVerify(ptrs[0]);
    deallocateAndVerify(ptrs[1]);
    deallocateAndVerify(ptrs[2]);
    
    // Try to allocate a larger block - should succeed due to merging
    void* large_ptr = allocateAndVerify(initial_size * 2);
    ASSERT_NE(large_ptr, nullptr);
    deallocateAndVerify(large_ptr);
}

// Test alignment requirements
TEST_F(DefragAllocatorTest, Alignment) {
    std::vector<size_t> alignments = {8, 16, 32, 64};
    std::vector<void*> ptrs;
    
    for (size_t align : alignments) {
        void* ptr = allocateAndVerify(16, align);
        ASSERT_NE(ptr, nullptr);
        EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr) % align, 0);
        ptrs.push_back(ptr);
    }
    
    for (void* ptr : ptrs) {
        deallocateAndVerify(ptr);
    }
}

// Test allocation patterns
TEST_F(DefragAllocatorTest, AllocationPattern) {
    std::vector<void*> ptrs;
    std::vector<size_t> sizes = {8, 16, 32, 64};
    
    // First allocation phase
    for (size_t size : sizes) {
        void* ptr = allocateAndVerify(size);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    
    // Free every other block
    for (size_t i = 0; i < ptrs.size(); i += 2) {
        deallocateAndVerify(ptrs[i]);
        ptrs[i] = nullptr;
    }
    
    // Second allocation phase - should fit in gaps
    for (size_t i = 0; i < ptrs.size(); i += 2) {
        void* ptr = allocateAndVerify(sizes[i]);
        ASSERT_NE(ptr, nullptr);
        ptrs[i] = ptr;
    }
    
    // Cleanup
    for (void* ptr : ptrs) {
        if (ptr) deallocateAndVerify(ptr);
    }
}

// Test block splitting
TEST_F(DefragAllocatorTest, BlockSplitting) {
    // Allocate a large block
    void* large_ptr = allocateAndVerify(256);
    ASSERT_NE(large_ptr, nullptr);
    
    // Remember its header
    DefragHeader* large_header = DefragHeader::getHeader(large_ptr);
    size_t original_size = large_header->size;
    
    // Free it
    deallocateAndVerify(large_ptr);
    
    // Allocate a smaller block - should split
    void* small_ptr = allocateAndVerify(64);
    ASSERT_NE(small_ptr, nullptr);
    
    DefragHeader* small_header = DefragHeader::getHeader(small_ptr);
    EXPECT_LT(small_header->size, original_size) << "Split block not smaller: " << small_header->debugString();
    
    // The remaining space should be marked as a free block
    DefragHeader* next = small_header->next;
    ASSERT_NE(next, nullptr) << "No next block after split: " << small_header->debugString();
    EXPECT_TRUE(next->is_free) << "Split block not free: " << next->debugString();
    EXPECT_TRUE(next->isValid()) << "Invalid split block: " << next->debugString();
    
    // Combined size of allocated and next free block should not exceed pool size
    EXPECT_LE(small_header->size + sizeof(DefragHeader) + next->size, allocator->getTotalMemory());
    
    deallocateAndVerify(small_ptr);
}

// Test error conditions
TEST_F(DefragAllocatorTest, ErrorConditions) {
    // Try to allocate more than pool size
    EXPECT_THROW(allocator->allocate(POOL_SIZE * 2), std::bad_alloc);
    
    // Try to deallocate null
    allocator->deallocate(nullptr);  // Should not throw
    
    // Try to deallocate invalid pointer - use uintptr_t
    void* invalid_ptr = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xDEADBEEF));
    EXPECT_THROW(allocator->deallocate(invalid_ptr), std::invalid_argument);
    
    // Allocate and try double-free
    void* ptr = allocateAndVerify(16);
    ASSERT_NE(ptr, nullptr);
    deallocateAndVerify(ptr);
    EXPECT_THROW(allocator->deallocate(ptr), std::runtime_error);
}

// Test the reset functionality
TEST_F(DefragAllocatorTest, Reset) {
    std::vector<void*> ptrs;
    
    // Make some allocations
    for (int i = 0; i < 5; i++) {
        void* ptr = allocateAndVerify(16);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    
    // Reset the allocator
    allocator->reset();
    
    // Verify stats are reset
    EXPECT_EQ(allocator->getUsedMemory(), 0);
    EXPECT_EQ(allocator->getAllocationCount(), 0);
    
    // Should be able to allocate again
    void* ptr = allocateAndVerify(16);
    ASSERT_NE(ptr, nullptr);
    deallocateAndVerify(ptr);
}