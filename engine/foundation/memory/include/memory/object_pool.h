#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>

#ifdef _WIN32
    #include <malloc.h>
#endif

namespace PyNovaGE {

/**
 * @brief Type-safe object pool
 * 
 * Template-based object pool for efficient allocation and deallocation
 * of objects of a specific type. Handles construction/destruction automatically.
 */
template<typename T>
class ObjectPool {
private:
    struct FreeNode {
        FreeNode* next;
    };

    union Block {
        T object;
        FreeNode free_node;
        
        Block() {} // Don't initialize - we'll handle it manually
        ~Block() {} // Don't destruct - we'll handle it manually
    };

    Block* buffer_;
    size_t pool_size_;
    FreeNode* free_list_;
    size_t allocated_objects_;
    size_t peak_allocated_;

public:
    /**
     * @brief Construct object pool
     * @param pool_size Number of objects the pool can hold
     */
    explicit ObjectPool(size_t pool_size)
        : pool_size_(pool_size)
        , allocated_objects_(0)
        , peak_allocated_(0) {
        
        // Allocate buffer aligned for both T and pointers
        constexpr size_t alignment = std::max(alignof(T), alignof(void*));
        
#ifdef _WIN32
        buffer_ = reinterpret_cast<Block*>(
            _aligned_malloc(sizeof(Block) * pool_size_, alignment));
#else
        buffer_ = reinterpret_cast<Block*>(
            std::aligned_alloc(alignment, sizeof(Block) * pool_size_));
#endif
        
        if (!buffer_) {
            throw std::bad_alloc();
        }

        initializeFreeList();
    }

    ~ObjectPool() {
        // Ensure all objects are destroyed
        clear();
        
#ifdef _WIN32
        _aligned_free(buffer_);
#else
        std::free(buffer_);
#endif
    }

    // Non-copyable
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // Movable
    ObjectPool(ObjectPool&& other) noexcept
        : buffer_(other.buffer_)
        , pool_size_(other.pool_size_)
        , free_list_(other.free_list_)
        , allocated_objects_(other.allocated_objects_)
        , peak_allocated_(other.peak_allocated_) {
        
        other.buffer_ = nullptr;
        other.free_list_ = nullptr;
        other.allocated_objects_ = 0;
    }

    /**
     * @brief Acquire an object from the pool
     * @param args Arguments to forward to T's constructor
     * @return Pointer to constructed object, or nullptr if pool is full
     */
    template<typename... Args>
    T* acquire(Args&&... args) {
        if (!free_list_) {
            return nullptr; // Pool exhausted
        }

        // Get block from free list
        Block* block = reinterpret_cast<Block*>(free_list_);
        free_list_ = free_list_->next;

        // Construct object in place
        T* object = new(&block->object) T(std::forward<Args>(args)...);
        
        ++allocated_objects_;
        if (allocated_objects_ > peak_allocated_) {
            peak_allocated_ = allocated_objects_;
        }

        return object;
    }

    /**
     * @brief Release an object back to the pool
     * @param object Pointer to object to release
     */
    void release(T* object) {
        if (!object || !ownsPointer(object)) {
            return;
        }

        // Destroy the object
        object->~T();

        // Add block back to free list
        Block* block = reinterpret_cast<Block*>(object);
        block->free_node.next = free_list_;
        free_list_ = &block->free_node;

        --allocated_objects_;
    }

    /**
     * @brief Check if pointer belongs to this pool
     */
    bool ownsPointer(const void* ptr) const {
        const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(ptr);
        const uint8_t* buffer_start = reinterpret_cast<const uint8_t*>(buffer_);
        const uint8_t* buffer_end = buffer_start + (sizeof(Block) * pool_size_);
        
        return byte_ptr >= buffer_start && byte_ptr < buffer_end;
    }

    /**
     * @brief Get number of allocated objects
     */
    size_t getAllocatedCount() const { return allocated_objects_; }

    /**
     * @brief Get number of free objects
     */
    size_t getFreeCount() const { return pool_size_ - allocated_objects_; }

    /**
     * @brief Get peak allocated count
     */
    size_t getPeakAllocated() const { return peak_allocated_; }

    /**
     * @brief Reset statistics
     */
    void resetStats() { peak_allocated_ = 0; }

    /**
     * @brief Clear all objects and reset pool
     */
    void clear() {
        // Destroy all allocated objects
        for (size_t i = 0; i < pool_size_; ++i) {
            // Check if this block is not in the free list
            bool is_free = false;
            for (FreeNode* node = free_list_; node; node = node->next) {
                if (node == &buffer_[i].free_node) {
                    is_free = true;
                    break;
                }
            }
            
            if (!is_free) {
                buffer_[i].object.~T();
            }
        }

        allocated_objects_ = 0;
        initializeFreeList();
    }

private:
    void initializeFreeList() {
        free_list_ = nullptr;
        for (size_t i = pool_size_; i > 0; --i) {
            size_t index = i - 1;
            buffer_[index].free_node.next = free_list_;
            free_list_ = &buffer_[index].free_node;
        }
    }
};

} // namespace PyNovaGE