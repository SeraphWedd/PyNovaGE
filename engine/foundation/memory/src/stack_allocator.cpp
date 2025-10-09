#include "memory/stack_allocator.h"
#include <cstdlib>
#include <algorithm>

#ifdef _WIN32
    #include <malloc.h>
#endif

namespace PyNovaGE {

StackAllocator::StackAllocator(size_t buffer_size)
    : buffer_size_(buffer_size)
    , current_offset_(0)
    , current_marker_(nullptr)
    , total_allocated_(0)
    , peak_allocated_(0) {
    
#ifdef _WIN32
    buffer_ = static_cast<uint8_t*>(_aligned_malloc(buffer_size_, 16));
#else
    buffer_ = static_cast<uint8_t*>(std::aligned_alloc(16, buffer_size_));
#endif

    if (!buffer_) {
        throw std::bad_alloc();
    }
}

StackAllocator::~StackAllocator() {
    // Clean up any remaining markers
    while (current_marker_) {
        Marker* prev = current_marker_->prev;
        delete current_marker_;
        current_marker_ = prev;
    }

    if (buffer_) {
#ifdef _WIN32
        _aligned_free(buffer_);
#else
        free(buffer_);
#endif
    }
}

void* StackAllocator::allocate(size_t size, size_t alignment) {
    if (size == 0) return nullptr;

    size_t aligned_offset = alignUp(current_offset_, alignment);
    
    if (aligned_offset + size > buffer_size_) {
        return nullptr; // Out of memory
    }

    void* ptr = buffer_ + aligned_offset;
    current_offset_ = aligned_offset + size;

    total_allocated_ += size;
    peak_allocated_ = std::max(peak_allocated_, current_offset_);

    return ptr;
}

size_t StackAllocator::pushMarker() {
    // Allocate space for marker from our own buffer
    size_t marker_offset = alignUp(current_offset_, alignof(Marker));
    
    if (marker_offset + sizeof(Marker) > buffer_size_) {
        return SIZE_MAX; // Out of memory for marker
    }

    Marker* new_marker = reinterpret_cast<Marker*>(buffer_ + marker_offset);
    new_marker->offset = marker_offset; // Store where the marker starts
    new_marker->prev = current_marker_;
    
    current_marker_ = new_marker;
    current_offset_ = marker_offset + sizeof(Marker);

    return marker_offset; // Return marker position as handle
}

void StackAllocator::popToMarker(size_t marker) {
    if (marker == SIZE_MAX || marker >= current_offset_) {
        return; // Invalid marker
    }

    // Find the marker in our linked list
    Marker* target_marker = reinterpret_cast<Marker*>(buffer_ + marker);
    
    // Verify this is actually a valid marker
    Marker* search = current_marker_;
    bool found = false;
    while (search) {
        if (search == target_marker) {
            found = true;
            break;
        }
        search = search->prev;
    }

    if (!found) {
        return; // Invalid marker
    }

    // Pop to the target marker
    current_marker_ = target_marker->prev;
    current_offset_ = target_marker->offset;
}

void StackAllocator::reset() {
    current_offset_ = 0;
    current_marker_ = nullptr;
    // Note: We don't reset total_allocated_ - it tracks lifetime totals
}

} // namespace PyNovaGE