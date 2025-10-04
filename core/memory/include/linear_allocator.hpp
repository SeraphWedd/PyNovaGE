#pragma once

#include "allocators.hpp"
#include "memory_utils.hpp"
#include <cstdint>
#include <cassert>

#if defined(_WIN32)
    #include <malloc.h>
#endif

namespace pynovage {
namespace memory {

template<std::size_t DefaultAlignment>
class LinearAllocator : public IAllocator {
    static_assert(DefaultAlignment >= alignof(std::max_align_t), "DefaultAlignment must be at least alignof(std::max_align_t)");
    static_assert((DefaultAlignment & (DefaultAlignment - 1)) == 0, "DefaultAlignment must be a power of 2");

public:
    explicit LinearAllocator(std::size_t capacity) 
        : capacity_(alignTo(capacity, DefaultAlignment))
        , used_(0)
        , allocation_count_(0) {
        // Allocate aligned memory
        #if defined(_WIN32)
        start_ = reinterpret_cast<std::uint8_t*>(_aligned_malloc(capacity_, DefaultAlignment));
        #else
        start_ = reinterpret_cast<std::uint8_t*>(std::aligned_alloc(DefaultAlignment, capacity_));
        #endif
        current_ = start_;
        assert(start_ != nullptr && "Failed to allocate memory");
    }

    ~LinearAllocator() override {
        #if defined(_WIN32)
        _aligned_free(start_);
        #else
        std::free(start_);
        #endif
    }

    // Disable copying
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;

    // Allow moving
    LinearAllocator(LinearAllocator&& other) noexcept 
        : start_(other.start_)
        , current_(other.current_)
        , capacity_(other.capacity_)
        , used_(other.used_)
        , allocation_count_(other.allocation_count_) {
        other.start_ = nullptr;
        other.current_ = nullptr;
        other.capacity_ = 0;
        other.used_ = 0;
        other.allocation_count_ = 0;
    }

    void reset() {
        current_ = start_;
        used_ = 0;
        allocation_count_ = 0;
    }

    LinearAllocator& operator=(LinearAllocator&& other) noexcept {
        if (this != &other) {
            std::free(start_);
            start_ = other.start_;
            current_ = other.current_;
            capacity_ = other.capacity_;
            used_ = other.used_;
            allocation_count_ = other.allocation_count_;
            other.start_ = nullptr;
            other.current_ = nullptr;
            other.capacity_ = 0;
            other.used_ = 0;
            other.allocation_count_ = 0;
        }
        return *this;
    }

protected:
    void* allocateImpl(std::size_t size, std::size_t alignment) override {
        assert(size > 0 && "Cannot allocate zero bytes");
        assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of 2");
        
        // Calculate aligned address
        std::uint8_t* aligned_addr = alignPointer(current_, alignment);
        std::size_t padding = aligned_addr - current_;
        std::size_t total_size = size + padding;

        // Check if we have enough space
        if (used_ + total_size > capacity_) {
            return nullptr;  // Out of memory
        }

        // Update allocator state
        current_ = aligned_addr + size;
        used_ += total_size;
        ++allocation_count_;

        return aligned_addr;
    }

    void deallocateImpl(void*) override {
        // Linear allocator doesn't support individual deallocation
    }

public:
    std::size_t getUsedMemory() const override { return used_; }
    std::size_t getTotalMemory() const override { return capacity_; }
    std::size_t getAllocationCount() const override { return allocation_count_; }

private:
    std::uint8_t* start_;        // Start of the memory block
    std::uint8_t* current_;      // Current allocation position
    std::size_t capacity_;       // Total capacity
    std::size_t used_;          // Used memory (including padding)
    std::size_t allocation_count_; // Number of successful allocations
};

} // namespace memory
} // namespace pynovage