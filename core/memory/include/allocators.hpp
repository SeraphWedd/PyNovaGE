#pragma once

#include "iallocator.hpp"
#include "memory_utils.hpp"

namespace pynovage::memory {

// Forward declarations
template<std::size_t Alignment> class LinearAllocator;
class ThreadLocalPoolAllocator;
class LockFreeStackAllocator;

} // namespace pynovage::memory
