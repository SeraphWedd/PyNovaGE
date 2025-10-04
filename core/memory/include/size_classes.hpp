#pragma once

#include <array>
#include <cstddef>
#include <cassert>

namespace pynovage {
namespace memory {

// Size class system based on research of common allocation patterns
// Uses a power-of-two based system with additional size classes for common sizes
class SizeClassManager {
public:
    static constexpr size_t MIN_ALLOCATION = 8;          // Minimum allocation size
    static constexpr size_t MAX_SMALL_SIZE = 256;       // Max size for small object optimization
    static constexpr size_t MAX_MEDIUM_SIZE = 4096;     // Max size for medium object optimization
    static constexpr size_t NUM_SMALL_CLASSES = 8;      // 8, 16, 32, 48, 64, 96, 128, 256
    static constexpr size_t NUM_MEDIUM_CLASSES = 8;     // 512, 1024, 1536, 2048, 2560, 3072, 3584, 4096
    static constexpr size_t TOTAL_SIZE_CLASSES = NUM_SMALL_CLASSES + NUM_MEDIUM_CLASSES;

    // Small size classes are more finely grained
    static constexpr std::array<size_t, NUM_SMALL_CLASSES> SMALL_SIZES = {
        8, 16, 32, 48, 64, 96, 128, 256
    };

    // Medium size classes use larger steps
    static constexpr std::array<size_t, NUM_MEDIUM_CLASSES> MEDIUM_SIZES = {
        512, 1024, 1536, 2048, 2560, 3072, 3584, 4096
    };

    // Get the size class index for a given size
    static size_t getSizeClass(size_t size) {
        if (size <= MAX_SMALL_SIZE) {
            // Binary search for small sizes
            size_t left = 0;
            size_t right = NUM_SMALL_CLASSES - 1;
            
            while (left <= right) {
                size_t mid = (left + right) / 2;
                if (SMALL_SIZES[mid] < size) {
                    left = mid + 1;
                } else if (mid > 0 && SMALL_SIZES[mid - 1] >= size) {
                    right = mid - 1;
                } else {
                    return mid;
                }
            }
            return left;
        }
        
        if (size <= MAX_MEDIUM_SIZE) {
            // Binary search for medium sizes
            size_t left = 0;
            size_t right = NUM_MEDIUM_CLASSES - 1;
            
            while (left <= right) {
                size_t mid = (left + right) / 2;
                if (MEDIUM_SIZES[mid] < size) {
                    left = mid + 1;
                } else if (mid > 0 && MEDIUM_SIZES[mid - 1] >= size) {
                    right = mid - 1;
                } else {
                    return NUM_SMALL_CLASSES + mid;
                }
            }
            return NUM_SMALL_CLASSES + left;
        }
        
        return TOTAL_SIZE_CLASSES;  // Indicates size too large for size classes
    }

    // Get the actual size for a given size class
    static size_t getSizeForClass(size_t size_class) {
        assert(size_class < TOTAL_SIZE_CLASSES && "Invalid size class");
        if (size_class < NUM_SMALL_CLASSES) {
            return SMALL_SIZES[size_class];
        }
        return MEDIUM_SIZES[size_class - NUM_SMALL_CLASSES];
    }

    // Check if a size should use size class optimization
    static bool shouldUseSizeClass(size_t size) {
        return size <= MAX_MEDIUM_SIZE;
    }

    // Get the next size class for a given size
    static size_t getNextSize(size_t size) {
        size_t size_class = getSizeClass(size);
        if (size_class >= TOTAL_SIZE_CLASSES) {
            return size;  // No size class optimization for large sizes
        }
        return getSizeForClass(size_class);
    }

    // Statistics tracking
    struct Stats {
        std::array<size_t, TOTAL_SIZE_CLASSES> allocations{};  // Number of allocations per size class
        std::array<size_t, TOTAL_SIZE_CLASSES> deallocations{};  // Number of deallocations per size class
        std::array<size_t, TOTAL_SIZE_CLASSES> misses{};  // Cache misses per size class
        
        void clear() {
            allocations.fill(0);
            deallocations.fill(0);
            misses.fill(0);
        }
        
        double getHitRate(size_t size_class) const {
            if (allocations[size_class] == 0) return 0.0;
            return 1.0 - (static_cast<double>(misses[size_class]) / allocations[size_class]);
        }
    };
};

} // namespace memory
} // namespace pynovage