#pragma once

#include <cstddef>
#include <type_traits>
#include "memory_tracer.hpp"

namespace pynovage {
namespace memory {

// Base allocator interface
class IAllocator {
public:
    virtual ~IAllocator() = default;
    
    // Core allocation interface
    void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) {
        void* ptr = allocateImpl(size, alignment);
        if (ptr) {
            MemoryTracer::instance().recordAllocation(ptr, size, this);
        }
        return ptr;
    }

    void deallocate(void* ptr) {
        if (ptr) {
            MemoryTracer::instance().recordDeallocation(ptr);
            deallocateImpl(ptr);
        }
    }

    virtual void reset() = 0;

    // Memory statistics
    virtual std::size_t getUsedMemory() const = 0;
    virtual std::size_t getTotalMemory() const = 0;
    virtual std::size_t getAllocationCount() const = 0;

protected:
    virtual void* allocateImpl(std::size_t size, std::size_t alignment) = 0;
    virtual void deallocateImpl(void* ptr) = 0;
};

} // namespace memory
} // namespace pynovage
