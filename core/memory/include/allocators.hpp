#pragma once

#include <cstddef>
#include <type_traits>
#include <atomic>

namespace pynovage {
namespace memory {

// Forward declarations
template<std::size_t Alignment> class LinearAllocator;
class ThreadLocalPoolAllocator;
class LockFreeStackAllocator;

// Base allocator interface
class IAllocator {
public:
    virtual ~IAllocator() = default;
    
    // Core allocation interface
    virtual void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) = 0;
    virtual void deallocate(void* ptr) = 0;
    virtual void reset() = 0;

    // Memory statistics
    virtual std::size_t getUsedMemory() const = 0;
    virtual std::size_t getTotalMemory() const = 0;
    virtual std::size_t getAllocationCount() const = 0;
};

// Memory alignment utilities
constexpr std::size_t alignTo(std::size_t size, std::size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

template<typename T>
constexpr T* alignPointer(T* ptr, std::size_t alignment) {
    std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr);
    std::uintptr_t aligned = alignTo(addr, alignment);
    return reinterpret_cast<T*>(aligned);
}

// Memory block header for tracking allocations
struct AllocationHeader {
    std::size_t size;
    std::size_t alignment;
    void* prev;  // For linked list support
    void* next;  // For linked list support
};

} // namespace memory
} // namespace pynovage