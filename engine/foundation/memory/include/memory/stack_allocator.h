#pragma once

#include "allocator.h"
#include <cstddef>
#include <cstdint>

namespace PyNovaGE {

/**
 * @brief Stack allocator for scoped allocations
 * 
 * LIFO allocator that allows pushing/popping allocation markers.
 * Perfect for frame-based or scope-based memory management.
 */
class StackAllocator : public Allocator {
private:
    struct Marker {
        size_t offset;
        Marker* prev;
    };

    uint8_t* buffer_;
    size_t buffer_size_;
    size_t current_offset_;
    Marker* current_marker_;
    size_t total_allocated_;
    size_t peak_allocated_;

public:
    explicit StackAllocator(size_t buffer_size);
    ~StackAllocator();

    void* allocate(size_t size, size_t alignment = 16) override;
    void deallocate(void* ptr) override { (void)ptr; } // Use markers instead

    size_t getTotalAllocated() const override { return total_allocated_; }
    size_t getPeakAllocated() const override { return peak_allocated_; }
    void resetStats() override { total_allocated_ = 0; peak_allocated_ = 0; }

    /**
     * @brief Push a new allocation marker
     * @return Marker that can be used to pop back to this point
     */
    size_t pushMarker();

    /**
     * @brief Pop back to a previous marker
     * @param marker Marker returned by pushMarker()
     */
    void popToMarker(size_t marker);

    /**
     * @brief Get current stack usage
     */
    size_t getCurrentUsage() const { return current_offset_; }

    /**
     * @brief Reset entire stack
     */
    void reset();

private:
    static size_t alignUp(size_t value, size_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }
};

} // namespace PyNovaGE