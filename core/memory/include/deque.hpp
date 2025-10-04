#pragma once

#include "allocators.hpp"
#include <cstddef>
#include <type_traits>
#include <memory>

namespace pynovage {
namespace memory {

template<typename T, typename Allocator = IAllocator*>
class Deque {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    // Default constructor
    Deque() noexcept : allocator_(nullptr) {}
    
    // Constructor with allocator
    explicit Deque(Allocator alloc) noexcept : allocator_(alloc) {}

    // Destructor
    ~Deque() = default;

    // Disable copying for now
    Deque(const Deque&) = delete;
    Deque& operator=(const Deque&) = delete;

    // Basic capacity operations
    bool empty() const noexcept { return true; }
    size_type size() const noexcept { return 0; }

private:
    Allocator allocator_;
};

} // namespace memory
} // namespace pynovage