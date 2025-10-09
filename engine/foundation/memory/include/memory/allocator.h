#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

namespace PyNovaGE {

/**
 * @brief Base interface for custom allocators
 * 
 * Simple, performance-focused allocator interface for game engine use.
 * All allocators provide aligned memory allocation suitable for SIMD operations.
 */
class Allocator {
public:
    virtual ~Allocator() = default;

    /**
     * @brief Allocate aligned memory
     * @param size Size in bytes to allocate
     * @param alignment Memory alignment (default 16 bytes for SIMD)
     * @return Pointer to allocated memory, or nullptr on failure
     */
    virtual void* allocate(size_t size, size_t alignment = 16) = 0;

    /**
     * @brief Deallocate memory
     * @param ptr Pointer to memory to deallocate
     */
    virtual void deallocate(void* ptr) = 0;

    /**
     * @brief Get total allocated bytes
     */
    virtual size_t getTotalAllocated() const = 0;

    /**
     * @brief Get peak allocated bytes
     */
    virtual size_t getPeakAllocated() const = 0;

    /**
     * @brief Reset allocator statistics
     */
    virtual void resetStats() = 0;
};

/**
 * @brief Default system allocator wrapper
 * 
 * Simple wrapper around standard aligned allocation for baseline performance.
 */
class SystemAllocator : public Allocator {
private:
    size_t total_allocated_ = 0;
    size_t peak_allocated_ = 0;

public:
    void* allocate(size_t size, size_t alignment = 16) override;
    void deallocate(void* ptr) override;
    
    size_t getTotalAllocated() const override { return total_allocated_; }
    size_t getPeakAllocated() const override { return peak_allocated_; }
    void resetStats() override { total_allocated_ = 0; peak_allocated_ = 0; }
};

/**
 * @brief Linear allocator for frame-based allocations
 * 
 * Fast bump allocator that allocates linearly through a buffer.
 * Perfect for temporary allocations that are freed all at once.
 */
class LinearAllocator : public Allocator {
private:
    uint8_t* buffer_;
    size_t buffer_size_;
    size_t current_offset_;
    size_t total_allocated_;
    size_t peak_allocated_;

public:
    explicit LinearAllocator(size_t buffer_size);
    ~LinearAllocator();

    void* allocate(size_t size, size_t alignment = 16) override;
    void deallocate(void* ptr) override { (void)ptr; } // No-op for linear allocator
    
    size_t getTotalAllocated() const override { return total_allocated_; }
    size_t getPeakAllocated() const override { return peak_allocated_; }
    void resetStats() override { total_allocated_ = 0; peak_allocated_ = 0; }

    /**
     * @brief Reset allocator to beginning of buffer
     */
    void reset();

    /**
     * @brief Get current buffer usage
     */
    size_t getCurrentUsage() const { return current_offset_; }
};

} // namespace PyNovaGE