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
    bool empty() const noexcept { return size_ == 0; }
    size_type size() const noexcept { return size_; }

    // Get first/last elements
    reference front() { 
        if (empty()) throw std::out_of_range("Deque is empty");
        return first_block_->data()[front_index_];
    }
    
    const_reference front() const { 
        if (empty()) throw std::out_of_range("Deque is empty");
        return first_block_->data()[front_index_];
    }
    
    reference back() {
        if (empty()) throw std::out_of_range("Deque is empty");
        return last_block_->data()[(back_index_ - 1 + BLOCK_CAPACITY) % BLOCK_CAPACITY];
    }
    
    const_reference back() const {
        if (empty()) throw std::out_of_range("Deque is empty");
        return last_block_->data()[(back_index_ - 1 + BLOCK_CAPACITY) % BLOCK_CAPACITY];
    }

    // Element manipulation
    void push_front(const T& value) {
        if (empty()) {
            if (!first_block_) {
                first_block_ = allocate_block();
                new (first_block_) Block();
                last_block_ = first_block_;
            }
            front_index_ = back_index_ = 0;
            new(first_block_->data() + front_index_) T(value);
            back_index_++;
        } else {
            if (front_index_ == 0) {
                // Need new block at front
                first_block_ = create_block_before(first_block_);
                front_index_ = BLOCK_CAPACITY - 1;
                new(first_block_->data() + front_index_) T(value);
            } else {
                // Space in current front block
                front_index_--;
                new(first_block_->data() + front_index_) T(value);
            }
        }
        size_++;
    }

    void push_back(const T& value) {
        if (empty()) {
            if (!first_block_) {
                first_block_ = allocate_block();
                new (first_block_) Block();
                last_block_ = first_block_;
            }
            front_index_ = back_index_ = 0;
            new(last_block_->data() + back_index_) T(value);
            back_index_++;
        } else {
            if (back_index_ >= BLOCK_CAPACITY) {
                // Need new block at back
                auto new_block = create_block_after(last_block_);
                back_index_ = 0;
                last_block_ = new_block;
                new(last_block_->data()) T(value);
                back_index_++;
            } else {
                // Space in current back block
                new(last_block_->data() + back_index_) T(value);
                back_index_++;
            }
        }
        size_++;
    }

    void pop_front() {
        if (empty()) throw std::out_of_range("Deque is empty");

        // Destroy front element
        first_block_->data()[front_index_].~T();

        // Update pointers and size
        if (size_ == 1) {
            front_index_ = back_index_ = 0;
        } else {
            front_index_++;
            if (front_index_ >= BLOCK_CAPACITY && first_block_->next) {
                // Move to next block if it exists
                Block* old_block = first_block_;
                first_block_ = first_block_->next;
                first_block_->prev = nullptr;
                front_index_ = 0;
                destroy_block(old_block);
            }
        }
        size_--;
    }

    void pop_back() {
        if (empty()) throw std::out_of_range("Deque is empty");

        // Update pointers and size
        if (size_ == 1) {
            // Last element - just destroy it and reset indices
            last_block_->data()[back_index_ - 1].~T();
            front_index_ = back_index_ = 0;
        } else {
            if (back_index_ == 0 && last_block_->prev) {
                // Move to previous block if it exists
                Block* old_block = last_block_;
                last_block_ = last_block_->prev;
                last_block_->next = nullptr;
                back_index_ = BLOCK_CAPACITY;
                destroy_block(old_block);
            }
            back_index_--;
            last_block_->data()[back_index_].~T();
        }
        size_--;
    }

    template<typename... Args>
    void emplace_front(Args&&... args) {
        if (empty()) {
            if (!first_block_) {
                first_block_ = allocate_block();
                new (first_block_) Block();
                last_block_ = first_block_;
            }
            front_index_ = back_index_ = 0;
            new(first_block_->data() + front_index_) T(std::forward<Args>(args)...);
            back_index_++;
        } else {
            if (front_index_ == 0) {
                // Need new block at front
                first_block_ = create_block_before(first_block_);
                front_index_ = BLOCK_CAPACITY - 1;
                new(first_block_->data() + front_index_) T(std::forward<Args>(args)...);
            } else {
                // Space in current front block
                front_index_--;
                new(first_block_->data() + front_index_) T(std::forward<Args>(args)...);
            }
        }
        size_++;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (empty()) {
            if (!first_block_) {
                first_block_ = allocate_block();
                new (first_block_) Block();
                last_block_ = first_block_;
            }
            front_index_ = back_index_ = 0;
            new(last_block_->data() + back_index_) T(std::forward<Args>(args)...);
            back_index_++;
        } else {
            if (back_index_ >= BLOCK_CAPACITY) {
                // Need new block at back
                auto new_block = create_block_after(last_block_);
                back_index_ = 0;
                last_block_ = new_block;
                new(last_block_->data()) T(std::forward<Args>(args)...);
                back_index_++;
            } else {
                // Space in current back block
                new(last_block_->data() + back_index_) T(std::forward<Args>(args)...);
                back_index_++;
            }
        }
        size_++;
    }

private:
    // Block structure for deque storage
    static constexpr size_type BLOCK_SIZE = 512;  // Bytes per block
    static constexpr size_type BLOCK_CAPACITY = BLOCK_SIZE / sizeof(T);
    
    struct Block {
        alignas(alignof(T)) std::byte storage[BLOCK_SIZE];
        Block* next = nullptr;
        Block* prev = nullptr;

        T* data() noexcept { return reinterpret_cast<T*>(storage); }
        const T* data() const noexcept { return reinterpret_cast<const T*>(storage); }
    };

    Block* first_block_ = nullptr;      // First block in the chain
    Block* last_block_ = nullptr;       // Last block in the chain
    size_type front_index_ = 0;         // Index of first element in first block
    size_type back_index_ = 0;          // Index of last element + 1 in last block
    size_type size_ = 0;                // Total number of elements
    Allocator allocator_;               // Memory allocator

    // Block management helpers
    Block* allocate_block() {
        if (!allocator_) {
            throw std::runtime_error("No allocator provided");
        }
        return static_cast<Block*>(
            allocator_->allocate(sizeof(Block), alignof(Block))
        );
    }

    void deallocate_block(Block* block) {
        if (block && allocator_) {
            allocator_->deallocate(block);
        }
    }

    // Creates and links a new block before the given block
    Block* create_block_before(Block* block) {
        Block* new_block = allocate_block();
        new (new_block) Block();

        if (!block) {
            // First block in the deque
            first_block_ = last_block_ = new_block;
        } else {
            new_block->next = block;
            new_block->prev = block->prev;
            if (block->prev) {
                block->prev->next = new_block;
            } else {
                first_block_ = new_block;
            }
            block->prev = new_block;
        }

        return new_block;
    }

    // Creates and links a new block after the given block
    Block* create_block_after(Block* block) {
        Block* new_block = allocate_block();
        new (new_block) Block();

        if (!block) {
            // First block in the deque
            first_block_ = last_block_ = new_block;
        } else {
            new_block->prev = block;
            new_block->next = block->next;
            if (block->next) {
                block->next->prev = new_block;
            } else {
                last_block_ = new_block;
            }
            block->next = new_block;
        }

        return new_block;
    }

    // Destroys and removes a block from the chain
    void destroy_block(Block* block) {
        if (!block) return;

        // Unlink from chain
        if (block->prev) {
            block->prev->next = block->next;
        } else {
            first_block_ = block->next;
        }

        if (block->next) {
            block->next->prev = block->prev;
        } else {
            last_block_ = block->prev;
        }

        // Destroy and deallocate
        block->~Block();
        deallocate_block(block);
    }
};

} // namespace memory
} // namespace pynovage