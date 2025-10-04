#pragma once

#include "allocators.hpp"
#include <cstddef>
#include <type_traits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <iterator>

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

    struct Block; // forward declaration for iterators and helpers

    // Forward iterator traversing elements across blocks
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator() = default;
        iterator(Deque* owner, void* blk, size_type idx, size_type global_index)
            : owner_(owner), blk_(static_cast<typename Deque::Block*>(blk)), idx_(idx), global_index_(global_index) {}

        reference operator*() const { return blk_->data()[idx_]; }
        pointer operator->() const { return &blk_->data()[idx_]; }

        iterator& operator++() { advance(); return *this; }
        iterator operator++(int) { iterator tmp = *this; advance(); return tmp; }

        bool operator==(const iterator& other) const {
            return owner_ == other.owner_ && global_index_ == other.global_index_;
        }
        bool operator!=(const iterator& other) const { return !(*this == other); }

    private:
        Deque* owner_ = nullptr;
        typename Deque::Block* blk_ = nullptr;
        size_type idx_ = 0;
        size_type global_index_ = 0; // 0..size_

        void advance() {
            if (!owner_ || global_index_ >= owner_->size_) return; // end-safe
            // Move to next element position
            ++global_index_;
            if (global_index_ == owner_->size_) {
                // set to end sentinel
                blk_ = nullptr; idx_ = 0; return;
            }
            // compute next local position from global offset
            const size_type offset = owner_->front_index_ + global_index_;
            const size_type block_jumps = offset / owner_->BLOCK_CAPACITY;
            typename Deque::Block* b = owner_->first_block_;
            for (size_type j = 0; j < block_jumps && b; ++j) b = b->next;
            blk_ = b;
            idx_ = offset % owner_->BLOCK_CAPACITY;
        }

        friend class Deque;
    };

    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() = default;
        const_iterator(const iterator& it)
            : owner_(it.owner_), blk_(it.blk_), idx_(it.idx_), global_index_(it.global_index_) {}
        const_iterator(const Deque* owner, const void* blk, size_type idx, size_type global_index)
            : owner_(owner), blk_(static_cast<const typename Deque::Block*>(blk)), idx_(idx), global_index_(global_index) {}

        reference operator*() const { return blk_->data()[idx_]; }
        pointer operator->() const { return &blk_->data()[idx_]; }

        const_iterator& operator++() { advance(); return *this; }
        const_iterator operator++(int) { const_iterator tmp = *this; advance(); return tmp; }

        bool operator==(const const_iterator& other) const {
            return owner_ == other.owner_ && global_index_ == other.global_index_;
        }
        bool operator!=(const const_iterator& other) const { return !(*this == other); }

    private:
        const Deque* owner_ = nullptr;
        const typename Deque::Block* blk_ = nullptr;
        size_type idx_ = 0;
        size_type global_index_ = 0; // 0..size_

        void advance() {
            if (!owner_ || global_index_ >= owner_->size_) return;
            ++global_index_;
            if (global_index_ == owner_->size_) { blk_ = nullptr; idx_ = 0; return; }
            const size_type offset = owner_->front_index_ + global_index_;
            const size_type block_jumps = offset / owner_->BLOCK_CAPACITY;
            const typename Deque::Block* b = owner_->first_block_;
            for (size_type j = 0; j < block_jumps && b; ++j) b = b->next;
            blk_ = b;
            idx_ = offset % owner_->BLOCK_CAPACITY;
        }

        friend class Deque;
    };

    // Default constructor
    Deque() noexcept : allocator_(nullptr) {}
    
    // Constructor with allocator
    explicit Deque(Allocator alloc) noexcept : allocator_(alloc) {}

    // Destructor
    ~Deque() { clear(); }

    // Copy/move operations
    Deque(const Deque& other)
        : allocator_(other.allocator_) {
        // Deep copy elements in order
        for (const auto& v : other) {
            push_back(v);
        }
    }

    Deque& operator=(const Deque& other) {
        if (this == &other) return *this;
        clear();
        allocator_ = other.allocator_;
        for (const auto& v : other) {
            push_back(v);
        }
        return *this;
    }

    Deque(Deque&& other) noexcept
        : first_block_(other.first_block_),
          last_block_(other.last_block_),
          front_index_(other.front_index_),
          back_index_(other.back_index_),
          size_(other.size_),
          allocator_(other.allocator_) {
        other.first_block_ = nullptr;
        other.last_block_ = nullptr;
        other.front_index_ = 0;
        other.back_index_ = 0;
        other.size_ = 0;
        other.allocator_ = nullptr;
    }

    Deque& operator=(Deque&& other) noexcept {
        if (this == &other) return *this;
        clear();
        first_block_ = other.first_block_;
        last_block_ = other.last_block_;
        front_index_ = other.front_index_;
        back_index_ = other.back_index_;
        size_ = other.size_;
        allocator_ = other.allocator_;
        other.first_block_ = nullptr;
        other.last_block_ = nullptr;
        other.front_index_ = 0;
        other.back_index_ = 0;
        other.size_ = 0;
        other.allocator_ = nullptr;
        return *this;
    }

// Basic capacity operations
    bool empty() const noexcept { return size_ == 0; }
    size_type size() const noexcept { return size_; }

    // Random access
    reference operator[](size_type pos) {
        auto [blk, idx] = block_index_for_pos(pos);
        return blk->data()[idx];
    }
    const_reference operator[](size_type pos) const {
        auto [blk, idx] = block_index_for_pos(pos);
        return blk->data()[idx];
    }
    reference at(size_type pos) {
        if (pos >= size_) throw std::out_of_range("Deque index out of range");
        return (*this)[pos];
    }
    const_reference at(size_type pos) const {
        if (pos >= size_) throw std::out_of_range("Deque index out of range");
        return (*this)[pos];
    }

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

    // Resize operations
    void resize(size_type count) {
        if (count < size_) {
            while (size_ > count) pop_back();
        } else if (count > size_) {
            while (size_ < count) emplace_back();
        }
    }

    void resize(size_type count, const T& value) {
        if (count < size_) {
            while (size_ > count) pop_back();
        } else if (count > size_) {
            while (size_ < count) push_back(value);
        }
    }

    // Iterators
    iterator begin() {
        if (empty()) return end();
        return iterator(this, first_block_, front_index_, 0);
    }
    iterator end() { return iterator(this, nullptr, 0, size_); }
    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }
    const_iterator cbegin() const {
        if (empty()) return cend();
        return const_iterator(this, first_block_, front_index_, 0);
    }
    const_iterator cend() const { return const_iterator(this, nullptr, 0, size_); }

    // Clear contents
    void clear() {
        if (empty()) return;
        // Destroy elements in first block from front_index_
        Block* b = first_block_;
        size_type count = size_;
        size_type idx = front_index_;
        while (b && count > 0) {
            const size_type destroy_from = (b == first_block_) ? idx : 0;
            const bool is_last_block = (b == last_block_);
            size_type destroy_to = is_last_block ? back_index_ : BLOCK_CAPACITY;
            if (is_last_block && destroy_to == 0 && count > 0) {
                // last block completely full
                destroy_to = BLOCK_CAPACITY;
            }
            // destroy elements in [destroy_from, destroy_to)
            for (size_type i = destroy_from; i < destroy_to && count > 0; ++i) {
                b->data()[i].~T();
                --count;
            }
            Block* next = b->next;
            destroy_block(b);
            b = next;
        }
        first_block_ = last_block_ = nullptr;
        front_index_ = back_index_ = 0;
        size_ = 0;
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
    // Helpers
    std::pair<Block*, size_type> block_index_for_pos(size_type pos) {
        if (pos >= size_) throw std::out_of_range("Deque index out of range");
        const size_type offset = front_index_ + pos;
        const size_type block_jumps = offset / BLOCK_CAPACITY;
        Block* b = first_block_;
        for (size_type j = 0; j < block_jumps && b; ++j) b = b->next;
        return {b, offset % BLOCK_CAPACITY};
    }
    std::pair<const Block*, size_type> block_index_for_pos(size_type pos) const {
        if (pos >= size_) throw std::out_of_range("Deque index out of range");
        const size_type offset = front_index_ + pos;
        const size_type block_jumps = offset / BLOCK_CAPACITY;
        const Block* b = first_block_;
        for (size_type j = 0; j < block_jumps && b; ++j) b = b->next;
        return {b, offset % BLOCK_CAPACITY};
    }

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