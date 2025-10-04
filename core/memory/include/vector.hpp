#pragma once

#include "allocators.hpp"
#include <cstddef>
#include <type_traits>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <new>

namespace pynovage {
namespace memory {

template<typename T, std::size_t N = 16, typename Allocator = IAllocator*>
class Vector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;

private:
    static constexpr bool uses_sbo = (sizeof(T) * N) <= 64 && std::is_trivially_copyable_v<T>; // Small buffer optimization threshold
    static constexpr size_type sbo_capacity = N;

    alignas(alignof(T)) std::byte small_buffer_[uses_sbo ? sizeof(T) * N : 1];
    pointer data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;
    [[no_unique_address]] Allocator allocator_ = nullptr;

public:
    Vector() noexcept {
        if constexpr (uses_sbo) {
            data_ = reinterpret_cast<pointer>(small_buffer_);
            capacity_ = N;
        }
    }

    explicit Vector(Allocator alloc) noexcept 
        : allocator_(alloc) {
        if constexpr (uses_sbo) {
            data_ = reinterpret_cast<pointer>(small_buffer_);
            capacity_ = N;
        }
    }

    explicit Vector(size_type count) {
        assign_default(count);
    }

    Vector(size_type count, const T& value) {
        assign_fill(count, value);
    }

    Vector(const Vector& other) {
        if (other.size_ == 0) return;
        
        if (other.using_small_buffer()) {
            if constexpr (uses_sbo) {
                data_ = reinterpret_cast<pointer>(small_buffer_);
                capacity_ = N;
                uninitialized_copy(other.data_, other.data_ + other.size_, data_);
                size_ = other.size_;
            } else {
                reserve(other.size_);
                uninitialized_copy(other.data_, other.data_ + other.size_, data_);
                size_ = other.size_;
            }
        } else {
            reserve(other.size_);
            uninitialized_copy(other.data_, other.data_ + other.size_, data_);
            size_ = other.size_;
        }
    }

    Vector(Vector&& other) noexcept 
        : allocator_(other.allocator_) {
        if (other.using_small_buffer()) {
            if constexpr (uses_sbo) {
                data_ = reinterpret_cast<pointer>(small_buffer_);
                capacity_ = N;
                if constexpr (std::is_trivially_copyable_v<T>) {
                    std::memcpy(data_, other.data_, other.size_ * sizeof(T));
                } else {
                    uninitialized_move(other.data_, other.data_ + other.size_, data_);
                    destroy(other.data_, other.data_ + other.size_);
                }
                size_ = other.size_;
                other.size_ = 0;
            } else {
                data_ = allocate(other.size_);
                uninitialized_move(other.data_, other.data_ + other.size_, data_);
                size_ = capacity_ = other.size_;
                other.size_ = 0;
                other.capacity_ = N;
            }
        } else {
            // Take ownership of other's buffer
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            
            // Reset other to small buffer state
            other.data_ = other.using_small_buffer() ? 
                reinterpret_cast<pointer>(other.small_buffer_) : nullptr;
            other.size_ = 0;
            other.capacity_ = uses_sbo ? N : 0;
            other.allocator_ = nullptr;
        }
    }

    ~Vector() {
        clear();
        if (!using_small_buffer() && data_) {
            deallocate(data_, capacity_);
        }
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            assign_copy(other);
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            clear();
            if (!using_small_buffer()) {
                deallocate(data_, capacity_);
            }

            if (other.using_small_buffer()) {
                if constexpr (uses_sbo) {
                    data_ = reinterpret_cast<pointer>(small_buffer_);
                    capacity_ = N;
                    if constexpr (std::is_trivially_copyable_v<T>) {
                        std::memcpy(data_, other.data_, other.size_ * sizeof(T));
                    } else {
                        uninitialized_move(other.data_, other.data_ + other.size_, data_);
                        destroy(other.data_, other.data_ + other.size_);
                    }
                } else {
                    data_ = allocate(other.size_);
                    capacity_ = other.size_;
                    uninitialized_move(other.data_, other.data_ + other.size_, data_);
                    destroy(other.data_, other.data_ + other.size_);
                }
            } else {
                data_ = other.data_;
                capacity_ = other.capacity_;
                allocator_ = other.allocator_;
                other.data_ = other.using_small_buffer() ? 
                    reinterpret_cast<pointer>(other.small_buffer_) : nullptr;
                other.capacity_ = other.using_small_buffer() ? N : 0;
                other.allocator_ = nullptr;
            }
            size_ = other.size_;
            other.size_ = 0;
        }
        return *this;
    }

    // Element access
    reference at(size_type pos) {
        if (pos >= size_) throw std::out_of_range("Vector index out of range");
        return data_[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= size_) throw std::out_of_range("Vector index out of range");
        return data_[pos];
    }

    reference operator[](size_type pos) noexcept {
        return data_[pos];
    }

    const_reference operator[](size_type pos) const noexcept {
        return data_[pos];
    }

    reference front() noexcept {
        return data_[0];
    }

    const_reference front() const noexcept {
        return data_[0];
    }

    reference back() noexcept {
        return data_[size_ - 1];
    }

    const_reference back() const noexcept {
        return data_[size_ - 1];
    }

    T* data() noexcept {
        return data_;
    }

    const T* data() const noexcept {
        return data_;
    }

    // Iterators
    iterator begin() noexcept {
        return data_;
    }

    const_iterator begin() const noexcept {
        return data_;
    }

    const_iterator cbegin() const noexcept {
        return data_;
    }

    iterator end() noexcept {
        return data_ + size_;
    }

    const_iterator end() const noexcept {
        return data_ + size_;
    }

    const_iterator cend() const noexcept {
        return data_ + size_;
    }

    // Capacity
    bool empty() const noexcept {
        return size_ == 0;
    }

    size_type size() const noexcept {
        return size_;
    }

    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    void reserve(size_type new_cap) {
        if (new_cap <= capacity_) return;

        pointer new_data;
        if (using_small_buffer() && new_cap <= N) {
            return;
        } else {
            new_data = allocate(new_cap);
        }

        if (size_ > 0) {
            uninitialized_move(data_, data_ + size_, new_data);
            destroy(data_, data_ + size_);
        }

        if (!using_small_buffer()) {
            deallocate(data_, capacity_);
        }

        data_ = new_data;
        capacity_ = new_cap;
    }

    size_type capacity() const noexcept {
        return capacity_;
    }

    void shrink_to_fit() {
        if (size_ < capacity_) {
            if (size_ == 0) {
                if (!using_small_buffer()) {
                    deallocate(data_, capacity_);
                    data_ = nullptr;
                    capacity_ = 0;
                }
            } else {
                pointer new_data;
                if (size_ <= N && uses_sbo) {
                    new_data = reinterpret_cast<pointer>(small_buffer_);
                } else {
                    new_data = allocate(size_);
                }
                uninitialized_move(data_, data_ + size_, new_data);
                destroy(data_, data_ + size_);
                if (!using_small_buffer()) {
                    deallocate(data_, capacity_);
                }
                data_ = new_data;
                capacity_ = size_;
            }
        }
    }

    // Modifiers
    void clear() noexcept {
        destroy(data_, data_ + size_);
        size_ = 0;
    }

    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    iterator insert(const_iterator pos, size_type count, const T& value) {
        auto index = pos - cbegin();
        if (size_ + count > capacity_) {
            grow(size_ + count);
        }

        if (pos != end()) {
            // Move existing elements
            for (auto i = size_ - 1; i >= index && i < size_; --i) {
                construct(data_ + i + count, std::move(data_[i]));
                destroy(data_ + i);
            }
        }

        for (size_type i = 0; i < count; ++i) {
            construct(data_ + index + i, value);
        }

        size_ += count;
        return begin() + index;
    }

    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        auto index = pos - cbegin();
        if (size_ + 1 > capacity_) {
            grow(size_ + 1);
        }

        if (pos != end()) {
            // Move existing elements
            construct(data_ + size_, std::move(data_[size_ - 1]));
            for (auto i = size_ - 1; i > index; --i) {
                data_[i] = std::move(data_[i - 1]);
            }
            destroy(data_ + index);
        }

        construct(data_ + index, std::forward<Args>(args)...);
        ++size_;
        return begin() + index;
    }

    iterator erase(const_iterator pos) {
        auto index = pos - cbegin();
        destroy(data_ + index);
        std::move(begin() + index + 1, end(), begin() + index);
        --size_;
        return begin() + index;
    }

    iterator erase(const_iterator first, const_iterator last) {
        auto start = first - cbegin();
        auto count = last - first;
        destroy(data_ + start, data_ + start + count);
        std::move(begin() + start + count, end(), begin() + start);
        size_ -= count;
        return begin() + start;
    }

    void push_back(const T& value) {
        emplace_back(value);
    }

    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    template<typename... Args>
    reference emplace_back(Args&&... args) {
        if (size_ >= capacity_) {
            grow(size_ + 1);
        }
        construct(data_ + size_, std::forward<Args>(args)...);
        return data_[size_++];
    }

    void pop_back() {
        destroy(data_ + --size_);
    }

    void resize(size_type count) {
        if (count > size_) {
            reserve(count);
            construct_default_fill(data_ + size_, count - size_);
        } else if (count < size_) {
            destroy(data_ + count, data_ + size_);
        }
        size_ = count;
    }

    void resize(size_type count, const value_type& value) {
        if (count > size_) {
            reserve(count);
            construct_fill(data_ + size_, count - size_, value);
        } else if (count < size_) {
            destroy(data_ + count, data_ + size_);
        }
        size_ = count;
    }

    void swap(Vector& other) noexcept {
        if (this == &other) return;

        if (using_small_buffer() && other.using_small_buffer()) {
            // Both using small buffer
            const size_type min_size = std::min(size_, other.size_);
            for (size_type i = 0; i < min_size; ++i) {
                std::swap(data_[i], other.data_[i]);
            }

            if (size_ < other.size_) {
                uninitialized_move(
                    other.data_ + size_,
                    other.data_ + other.size_,
                    data_ + size_
                );
                destroy(other.data_ + size_, other.data_ + other.size_);
            } else if (other.size_ < size_) {
                uninitialized_move(
                    data_ + other.size_,
                    data_ + size_,
                    other.data_ + other.size_
                );
                destroy(data_ + other.size_, data_ + size_);
            }
            std::swap(size_, other.size_);
        } else {
            // At least one using heap
            std::swap(data_, other.data_);
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
            std::swap(allocator_, other.allocator_);
        }
    }

private:
    static constexpr size_type minimum_growth = 16;
    static constexpr float growth_factor = 1.5f;

    bool using_small_buffer() const noexcept {
        return uses_sbo && data_ == reinterpret_cast<pointer>(const_cast<std::byte*>(small_buffer_));
    }

    pointer allocate(size_type n) {
        if (n == 0) return nullptr;
        if (!allocator_) {
            throw std::runtime_error("No allocator provided");
        }
        return static_cast<pointer>(
            allocator_->allocate(n * sizeof(T), alignof(T))
        );
    }

    void deallocate(pointer p, size_type n) {
        if (p && allocator_) {
            allocator_->deallocate(p);
        }
    }

    void grow(size_type min_size) {
        size_type new_cap = std::max(
            capacity_ + (capacity_ / 2), // Growth by factor
            std::max(minimum_growth, min_size) // At least what we need
        );
        reserve(new_cap);
    }

    // Construction/destruction helpers
    template<typename... Args>
    void construct(pointer p, Args&&... args) {
        new (p) T(std::forward<Args>(args)...);
    }

    void destroy(pointer p) noexcept {
        p->~T();
    }

    void destroy(pointer first, pointer last) noexcept {
        for (; first != last; ++first) {
            destroy(first);
        }
    }

    void construct_default_fill(pointer first, size_type n) {
        for (size_type i = 0; i < n; ++i) {
            construct(first + i);
        }
    }

    void construct_fill(pointer first, size_type n, const T& value) {
        for (size_type i = 0; i < n; ++i) {
            construct(first + i, value);
        }
    }

    void uninitialized_copy(const_pointer first, const_pointer last, pointer d_first) {
        pointer current = d_first;
        try {
            for (; first != last; ++first, ++current) {
                construct(current, *first);
            }
        } catch (...) {
            destroy(d_first, current);
            throw;
        }
    }

    void uninitialized_move(pointer first, pointer last, pointer d_first) {
        pointer current = d_first;
        try {
            for (; first != last; ++first, ++current) {
                construct(current, std::move(*first));
            }
        } catch (...) {
            destroy(d_first, current);
            throw;
        }
    }

    void assign_default(size_type count) {
        if (count > capacity_) {
            Vector tmp(nullptr);
            tmp.reserve(count);
            tmp.construct_default_fill(tmp.data_, count);
            tmp.size_ = count;
            *this = std::move(tmp);
        } else {
            construct_default_fill(data_, count);
            size_ = count;
        }
    }

    void assign_fill(size_type count, const T& value) {
        if (count > capacity_) {
            Vector tmp(nullptr);
            tmp.reserve(count);
            tmp.construct_fill(tmp.data_, count, value);
            tmp.size_ = count;
            *this = std::move(tmp);
        } else {
            construct_fill(data_, count, value);
            size_ = count;
        }
    }

    void assign_copy(const Vector& other) {
        if (other.size_ > capacity_) {
            pointer new_data;
            if (other.size_ <= N && uses_sbo) {
                new_data = reinterpret_cast<pointer>(small_buffer_);
            } else {
                new_data = allocate(other.size_);
            }
            uninitialized_copy(other.data_, other.data_ + other.size_, new_data);
            destroy(data_, data_ + size_);
            if (!using_small_buffer()) {
                deallocate(data_, capacity_);
            }
            data_ = new_data;
            size_ = capacity_ = other.size_;
        } else {
            destroy(data_, data_ + size_);
            uninitialized_copy(other.data_, other.data_ + other.size_, data_);
            size_ = other.size_;
        }
    }
};

} // namespace memory
} // namespace pynovage