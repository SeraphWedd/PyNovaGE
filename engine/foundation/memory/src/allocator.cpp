#include "memory/allocator.h"
#include <cstdlib>
#include <algorithm>

#ifdef _WIN32
    #include <malloc.h>
#endif

namespace PyNovaGE {

// SystemAllocator Implementation
void* SystemAllocator::allocate(size_t size, size_t alignment) {
    if (size == 0) return nullptr;

    void* ptr = nullptr;
    
#ifdef _WIN32
    ptr = _aligned_malloc(size, alignment);
#else
    if (posix_memalign(&ptr, alignment, size) != 0) {
        ptr = nullptr;
    }
#endif

    if (ptr) {
        total_allocated_ += size;
        peak_allocated_ = std::max(peak_allocated_, total_allocated_);
    }

    return ptr;
}

void SystemAllocator::deallocate(void* ptr) {
    if (!ptr) return;

#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

// LinearAllocator Implementation
LinearAllocator::LinearAllocator(size_t buffer_size)
    : buffer_size_(buffer_size)
    , current_offset_(0)
    , total_allocated_(0)
    , peak_allocated_(0) {
    
    // Allocate with larger alignment to support higher alignment requests
#ifdef _WIN32
    buffer_ = static_cast<uint8_t*>(_aligned_malloc(buffer_size_, 64));
#else
    buffer_ = static_cast<uint8_t*>(std::aligned_alloc(64, buffer_size_));
#endif

    if (!buffer_) {
        throw std::bad_alloc();
    }
}

LinearAllocator::~LinearAllocator() {
    if (buffer_) {
#ifdef _WIN32
        _aligned_free(buffer_);
#else
        free(buffer_);
#endif
    }
}

void* LinearAllocator::allocate(size_t size, size_t alignment) {
    if (size == 0) return nullptr;

    // Align current offset
    size_t aligned_offset = (current_offset_ + alignment - 1) & ~(alignment - 1);
    
    // Check if we have enough space
    if (aligned_offset + size > buffer_size_) {
        return nullptr; // Out of memory
    }

    void* ptr = buffer_ + aligned_offset;
    current_offset_ = aligned_offset + size;

    total_allocated_ += size;
    peak_allocated_ = std::max(peak_allocated_, current_offset_);

    return ptr;
}

void LinearAllocator::reset() {
    current_offset_ = 0;
    // Note: We don't reset total_allocated_ on purpose - it tracks lifetime totals
}

} // namespace PyNovaGE