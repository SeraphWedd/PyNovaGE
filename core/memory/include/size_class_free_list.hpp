#pragma once

#include "size_classes.hpp"
#include <array>
#include <mutex>
#include <atomic>
#include <cstring>
#include <iostream>
#include <iomanip>

namespace pynovage {
namespace memory {

// Free list optimized for size classes with atomic operations
class SizeClassFreeList {
public:
    struct FreeBlock {
        FreeBlock* next;
    };

private:
    // Array of free lists, one per size class
    std::array<std::atomic<FreeBlock*>, SizeClassManager::TOTAL_SIZE_CLASSES> free_lists_{};
    
    // Statistics
    SizeClassManager::Stats stats_;
    mutable std::mutex stats_mutex_;  // Stats don't need to be atomic, mutex is fine

public:
    SizeClassFreeList() {
        for (auto& list : free_lists_) {
            list.store(nullptr, std::memory_order_relaxed);
        }
    }

// Try to allocate from a specific size class
    void* tryAllocate(size_t size_class) {
        assert(size_class < SizeClassManager::TOTAL_SIZE_CLASSES);
        
        FreeBlock* next = nullptr;
        FreeBlock* current;
        
        do {
            // Load current head
            current = free_lists_[size_class].load(std::memory_order_acquire);
            if (!current) {
                // List is empty
                std::lock_guard<std::mutex> lock(stats_mutex_);
                stats_.misses[size_class]++;
                stats_.allocations[size_class]++;
                return nullptr;
            }
            
            // Try to use next as the new head
            next = current->next;
            
        } while (!free_lists_[size_class].compare_exchange_weak(
            current, next,
            std::memory_order_acquire,
            std::memory_order_relaxed));
        
        
        // Update stats
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.allocations[size_class]++;
        }
        
        // Zero out the block's memory for safety
        size_t block_size = SizeClassManager::getSizeForClass(size_class);
        std::memset(static_cast<void*>(current), 0, block_size);
        
        return current;
    }

    // Add a block to the free list
void addToFreeList(void* ptr, size_t size_class) {
        assert(size_class < SizeClassManager::TOTAL_SIZE_CLASSES);
        assert(ptr != nullptr);

        // Verify alignment is suitable for size class usage
        if (reinterpret_cast<std::uintptr_t>(ptr) % alignof(FreeBlock) != 0) {
            throw std::runtime_error("Block not properly aligned for size class free list");
        }
        
        // Initialize the block to a known state
        FreeBlock* block = static_cast<FreeBlock*>(ptr);
        
        // Zero remainder for security if block larger than node
        size_t block_size = SizeClassManager::getSizeForClass(size_class);
        if (block_size > sizeof(FreeBlock)) {
            std::memset(static_cast<char*>(ptr) + sizeof(FreeBlock), 0, 
                      block_size - sizeof(FreeBlock));
        }
        
        // Fix ABA problem by moving next pointer update outside CAS loop
        FreeBlock* old_head;
        do {
            old_head = free_lists_[size_class].load(std::memory_order_acquire);
            block->next = old_head;
            // Memory barrier ensures block->next is visible before CAS
            std::atomic_thread_fence(std::memory_order_release);
        } while (!free_lists_[size_class].compare_exchange_strong(
            old_head, block,
            std::memory_order_acq_rel,
            std::memory_order_acquire));
        
        // Update stats
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.deallocations[size_class]++;
        }
    }

    // Get statistics
    SizeClassManager::Stats getStats() const {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        return stats_;
    }

    // Clear statistics
    void clearStats() {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.clear();
    }

    // Check if a size class's free list is empty
    bool isEmpty(size_t size_class) const {
        assert(size_class < SizeClassManager::TOTAL_SIZE_CLASSES);
        return free_lists_[size_class].load(std::memory_order_relaxed) == nullptr;
    }

    // Get the current free list length for a size class
    size_t getFreeCount(size_t size_class) const {
        assert(size_class < SizeClassManager::TOTAL_SIZE_CLASSES);
        
        size_t count = 0;
        FreeBlock* current = free_lists_[size_class].load(std::memory_order_relaxed);
        
        while (current) {
            count++;
            current = current->next;
        }
        
        return count;
    }
    
    // Reset the free lists to empty state
    void reset() {
        for (auto& list : free_lists_) {
            list.store(nullptr, std::memory_order_relaxed);
        }
        clearStats();
    }
};

} // namespace memory
} // namespace pynovage