#pragma once

#include "allocator.h"
#include <cstddef>
#include <cstdint>

namespace PyNovaGE {

/**
 * @brief Fixed-size memory pool allocator
 * 
 * Efficient allocator for objects of uniform size.
 * Uses a free list for O(1) allocation and deallocation.
 */
class MemoryPool : public Allocator {
private:
    struct FreeBlock {
        FreeBlock* next;
    };

    uint8_t* buffer_;
    size_t buffer_size_;
    size_t block_size_;
    size_t block_count_;
    FreeBlock* free_list_;
    size_t allocated_blocks_;
    size_t peak_allocated_;

public:
    /**
     * @brief Construct memory pool
     * @param block_size Size of each allocation block (will be aligned)
     * @param block_count Number of blocks in the pool
     */
    MemoryPool(size_t block_size, size_t block_count);
    ~MemoryPool();

    void* allocate(size_t size, size_t alignment = 16) override;
    void deallocate(void* ptr) override;

    size_t getTotalAllocated() const override { 
        return allocated_blocks_ * block_size_; 
    }
    size_t getPeakAllocated() const override { 
        return peak_allocated_ * block_size_; 
    }
    void resetStats() override { 
        allocated_blocks_ = 0; 
        peak_allocated_ = 0; 
    }

    /**
     * @brief Get number of allocated blocks
     */
    size_t getAllocatedBlocks() const { return allocated_blocks_; }

    /**
     * @brief Get number of free blocks
     */
    size_t getFreeBlocks() const { return block_count_ - allocated_blocks_; }

    /**
     * @brief Check if pointer belongs to this pool
     */
    bool ownsPointer(void* ptr) const;

private:
    void initializeFreeList();
};

} // namespace PyNovaGE