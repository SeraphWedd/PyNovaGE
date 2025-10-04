#pragma once

#include "allocators.hpp"
#include <cstdint>
#include <cassert>
#include <atomic>

namespace pynovage {
namespace memory {

class LockFreeStackAllocator : public IAllocator {
public:
    // Stack marker for temporary scopes
    class Marker {
        friend class LockFreeStackAllocator;
        std::size_t position_;
        explicit Marker(std::size_t pos) : position_(pos) {}
    public:
        Marker() : position_(0) {}
    };

    explicit LockFreeStackAllocator(std::size_t capacity, std::size_t alignment = alignof(std::max_align_t))
        : capacity_(alignTo(capacity, alignment))
        , alignment_(alignment)
        , allocation_count_(0)
        , total_memory_(capacity_)
        , used_memory_(0) {
        // Allocate aligned memory
        #if defined(_WIN32)
        memory_ = reinterpret_cast<std::uint8_t*>(_aligned_malloc(capacity_, alignment));
        #else
        memory_ = reinterpret_cast<std::uint8_t*>(std::aligned_alloc(alignment, capacity_));
        #endif
        assert(memory_ != nullptr && "Failed to allocate memory");

        // Initialize the top pointer
        top_.store(0, std::memory_order_relaxed);
    }

    ~LockFreeStackAllocator() override {
        #if defined(_WIN32)
        _aligned_free(memory_);
        #else
        std::free(memory_);
        #endif
    }

    // Disable copying
    LockFreeStackAllocator(const LockFreeStackAllocator&) = delete;
    LockFreeStackAllocator& operator=(const LockFreeStackAllocator&) = delete;

    void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) override {
        assert(size > 0 && "Cannot allocate zero bytes");
        assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of 2");
        
        // Calculate total size including header and alignment
        std::size_t header_size = sizeof(AllocationHeader);
        std::size_t aligned_header_size = alignTo(header_size, alignment);
        std::size_t total_size = aligned_header_size + alignTo(size, alignment);

        // Try to allocate using atomic CAS
        while (true) {
            std::size_t current_top = top_.load(std::memory_order_acquire);
            std::size_t new_top = current_top + total_size;

            // Check if we have enough space
            if (new_top > capacity_) {
                return nullptr;  // Out of memory
            }

            // Try to update top pointer
            if (top_.compare_exchange_weak(current_top, new_top,
                                         std::memory_order_release,
                                         std::memory_order_relaxed)) {
                // Success - initialize the allocation header
                auto* header = reinterpret_cast<AllocationHeader*>(memory_ + current_top);
                header->size = size;
                header->alignment = alignment;

                // Update stats
                allocation_count_.fetch_add(1, std::memory_order_relaxed);
                used_memory_.fetch_add(total_size, std::memory_order_relaxed);

                // Return aligned user pointer
                return memory_ + current_top + aligned_header_size;
            }

            // CAS failed - another thread won the race, try again
        }
    }

    void deallocate(void* ptr) override {
        // Stack allocator doesn't support individual deallocation
        // Use reset() or unwind() instead
    }

    // Create a marker for the current stack position
    Marker getMarker() const {
        return Marker(top_.load(std::memory_order_acquire));
    }

    // Unwind the stack to a previous marker
    void unwind(const Marker& marker) {
        std::size_t new_top = marker.position_;
        std::size_t old_top = top_.load(std::memory_order_acquire);
        
        assert(new_top <= old_top && "Invalid marker");
        
        // Update top pointer
        top_.store(new_top, std::memory_order_release);
        
        // Update stats
        used_memory_.fetch_sub(old_top - new_top, std::memory_order_relaxed);
    }

    void reset() override {
        // Reset top pointer to start
        top_.store(0, std::memory_order_release);
        
        // Reset stats
        allocation_count_.store(0, std::memory_order_relaxed);
        used_memory_.store(0, std::memory_order_relaxed);
    }

    std::size_t getUsedMemory() const override { 
        return used_memory_.load(std::memory_order_relaxed); 
    }
    
    std::size_t getTotalMemory() const override { 
        return total_memory_; 
    }
    
    std::size_t getAllocationCount() const override { 
        return allocation_count_.load(std::memory_order_relaxed); 
    }

private:
    std::uint8_t* memory_;           // Raw memory buffer
    std::size_t capacity_;           // Total capacity
    std::size_t alignment_;          // Base alignment
    std::atomic<std::size_t> top_;   // Current allocation position

    // Statistics
    std::atomic<std::size_t> allocation_count_;  // Total number of active allocations
    const std::size_t total_memory_;             // Total memory (fixed)
    std::atomic<std::size_t> used_memory_;       // Currently used memory
};

} // namespace memory
} // namespace pynovage