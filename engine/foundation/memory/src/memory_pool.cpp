#include "memory/memory_pool.h"
#include <cstdlib>
#include <algorithm>

#ifdef _WIN32
    #include <malloc.h>
#endif

namespace PyNovaGE {

MemoryPool::MemoryPool(size_t block_size, size_t block_count)
    : block_count_(block_count)
    , allocated_blocks_(0)
    , peak_allocated_(0) {
    
    // Align block size to at least pointer size for free list
    block_size_ = std::max(block_size, sizeof(FreeBlock*));
    block_size_ = (block_size_ + 15) & ~15; // 16-byte align

    buffer_size_ = block_size_ * block_count_;

#ifdef _WIN32
    buffer_ = static_cast<uint8_t*>(_aligned_malloc(buffer_size_, 16));
#else
    buffer_ = static_cast<uint8_t*>(std::aligned_alloc(16, buffer_size_));
#endif

    if (!buffer_) {
        throw std::bad_alloc();
    }

    initializeFreeList();
}

MemoryPool::~MemoryPool() {
    if (buffer_) {
#ifdef _WIN32
        _aligned_free(buffer_);
#else
        free(buffer_);
#endif
    }
}

void* MemoryPool::allocate(size_t size, size_t alignment) {
    (void)alignment; // Unused - memory pool has fixed alignment
    // Memory pool only supports allocations up to block size
    if (size > block_size_ || !free_list_) {
        return nullptr;
    }

    // Get block from free list
    FreeBlock* block = free_list_;
    free_list_ = free_list_->next;

    ++allocated_blocks_;
    if (allocated_blocks_ > peak_allocated_) {
        peak_allocated_ = allocated_blocks_;
    }

    return block;
}

void MemoryPool::deallocate(void* ptr) {
    if (!ptr || !ownsPointer(ptr)) {
        return;
    }

    // Add block back to free list
    FreeBlock* block = static_cast<FreeBlock*>(ptr);
    block->next = free_list_;
    free_list_ = block;

    --allocated_blocks_;
}

bool MemoryPool::ownsPointer(void* ptr) const {
    if (!ptr || !buffer_) return false;

    uint8_t* byte_ptr = static_cast<uint8_t*>(ptr);
    uint8_t* buffer_end = buffer_ + buffer_size_;

    // Check if pointer is within buffer bounds and properly aligned
    if (byte_ptr < buffer_ || byte_ptr >= buffer_end) {
        return false;
    }

    // Check if pointer is at a valid block boundary
    size_t offset = byte_ptr - buffer_;
    return (offset % block_size_) == 0;
}

void MemoryPool::initializeFreeList() {
    free_list_ = nullptr;
    
    // Link all blocks in reverse order (so first block is allocated first)
    for (size_t i = block_count_; i > 0; --i) {
        size_t index = i - 1;
        FreeBlock* block = reinterpret_cast<FreeBlock*>(buffer_ + (index * block_size_));
        block->next = free_list_;
        free_list_ = block;
    }
}

} // namespace PyNovaGE