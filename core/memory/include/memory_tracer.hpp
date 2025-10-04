#pragma once

#include <cstddef>
#include <map>
#include <mutex>

namespace pynovage {
namespace memory {

class IAllocator; // Forward declaration

class MemoryTracer {
public:
    static MemoryTracer& instance() {
        static MemoryTracer tracer;
        return tracer;
    }

    void recordAllocation(void* ptr, std::size_t size, IAllocator* allocator) {
        std::lock_guard<std::mutex> lock(mutex_);
        allocations_[ptr] = AllocationInfo{size, allocator};
    }

    void recordDeallocation(void* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        allocations_.erase(ptr);
    }

    // Add methods to query allocation statistics if needed
    std::size_t getTotalAllocations() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return allocations_.size();
    }

    std::size_t getTotalMemoryUsage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::size_t total = 0;
        for (const auto& [ptr, info] : allocations_) {
            total += info.size;
        }
        return total;
    }

private:
    struct AllocationInfo {
        AllocationInfo() : size(0), allocator(nullptr) {}
        AllocationInfo(std::size_t s, IAllocator* a) : size(s), allocator(a) {}
        std::size_t size;
        IAllocator* allocator;
    };

    mutable std::mutex mutex_;
    std::map<void*, AllocationInfo> allocations_;

    MemoryTracer() = default;
    ~MemoryTracer() = default;
    MemoryTracer(const MemoryTracer&) = delete;
    MemoryTracer& operator=(const MemoryTracer&) = delete;
};

} // namespace memory
} // namespace pynovage
