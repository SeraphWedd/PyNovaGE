#pragma once

#include <cstddef>
#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace pynovage {
namespace memory {

// Memory tracking utilities for testing and debugging
class MemoryTracking {
public:
    struct AllocationRecord {
        std::size_t size;
        std::string file;
        int line;
        bool is_array;
    };

    static MemoryTracking& instance() {
        static MemoryTracking tracker;
        return tracker;
    }

    void recordAllocation(void* ptr, std::size_t size, const char* file, int line, bool is_array = false) {
        std::lock_guard<std::mutex> lock(mutex_);
        allocations_[ptr] = AllocationRecord{size, file, line, is_array};
        total_allocated_ += size;
        current_usage_ += size;
        if (current_usage_ > peak_usage_) {
            peak_usage_ = current_usage_;
        }
    }

    void recordDeallocation(void* ptr, bool is_array = false) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = allocations_.find(ptr);
        if (it != allocations_.end()) {
            if (it->second.is_array != is_array) {
                // Mismatched allocation/deallocation detected
                allocation_errors_.push_back(
                    "Mismatched allocation/deallocation at " + it->second.file +
                    ":" + std::to_string(it->second.line)
                );
            }
            current_usage_ -= it->second.size;
            allocations_.erase(it);
        } else {
            // Double free or invalid pointer detected
            allocation_errors_.push_back("Invalid deallocation or double free detected");
        }
    }

    // Statistics and debugging
    std::size_t getCurrentUsage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_usage_;
    }

    std::size_t getPeakUsage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return peak_usage_;
    }

    std::size_t getTotalAllocated() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return total_allocated_;
    }

    std::vector<std::string> getErrors() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return allocation_errors_;
    }

    bool hasLeaks() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !allocations_.empty();
    }

    void dumpLeaks(std::ostream& os) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (allocations_.empty()) {
            os << "No memory leaks detected.\n";
            return;
        }

        os << "Memory leaks detected:\n";
        for (const auto& [ptr, record] : allocations_) {
            os << "Leak at " << ptr << " (" << record.size << " bytes) allocated at "
               << record.file << ":" << record.line << "\n";
        }
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        allocations_.clear();
        allocation_errors_.clear();
        current_usage_ = 0;
        peak_usage_ = 0;
        total_allocated_ = 0;
    }

private:
    MemoryTracking() = default;
    MemoryTracking(const MemoryTracking&) = delete;
    MemoryTracking& operator=(const MemoryTracking&) = delete;

    mutable std::mutex mutex_;
    std::unordered_map<void*, AllocationRecord> allocations_;
    std::vector<std::string> allocation_errors_;
    std::size_t current_usage_{0};
    std::size_t peak_usage_{0};
    std::size_t total_allocated_{0};
};

} // namespace memory
} // namespace pynovage