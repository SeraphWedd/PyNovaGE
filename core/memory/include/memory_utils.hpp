#pragma once

#include <cstdint>

namespace pynovage {
namespace memory {

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
