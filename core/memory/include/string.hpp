#pragma once

#include "allocators.hpp"
#include "string_view.hpp"
#include <cstddef>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace pynovage {
namespace memory {

template<typename CharT, typename Allocator = IAllocator*>
class basic_string {
public:
    static_assert(std::is_same_v<CharT, char> || 
                 std::is_same_v<CharT, char16_t> || 
                 std::is_same_v<CharT, char32_t> ||
                 std::is_same_v<CharT, wchar_t>,
                 "Character type must be a char type");

    using value_type = CharT;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;

private:
    // Short string optimization: Use local buffer for strings up to 23 bytes (on 64-bit)
    static constexpr size_type sso_capacity = (24 / sizeof(CharT)) - 1;
    static constexpr size_type sso_mask = 0x80;
    
    union {
        struct {
            CharT data_[sso_capacity + 1];
            std::uint8_t size_; // Size in highest bit indicates SSO active
        } small_;
        
        struct {
            CharT* ptr_;
            size_type size_;
            size_type capacity_;
        } large_;
    };
    
#if __cplusplus >= 202002L
    [[no_unique_address]]
#endif
    Allocator allocator_;

public:
    // Constructors
    basic_string() noexcept
        : allocator_(nullptr) {
        small_.data_[0] = CharT();
        small_.size_ = 0;
    }

    explicit basic_string(Allocator alloc) noexcept
        : allocator_(alloc) {
        small_.data_[0] = CharT();
        small_.size_ = 0;
    }

    basic_string(const CharT* str, Allocator alloc = nullptr)
        : allocator_(alloc) {
        if (!str) {
            small_.data_[0] = CharT();
            small_.size_ = 0;
            return;
        }
        
        const size_type len = std::char_traits<CharT>::length(str);
        if (len <= sso_capacity) {
            std::char_traits<CharT>::copy(small_.data_, str, len);
            small_.data_[len] = CharT();
            small_.size_ = static_cast<std::uint8_t>(len);
        } else {
            large_.size_ = len;
            large_.capacity_ = next_capacity(len);
            large_.ptr_ = allocate(large_.capacity_);
            std::char_traits<CharT>::copy(large_.ptr_, str, len);
            large_.ptr_[len] = CharT();
            small_.size_ |= sso_mask; // Mark as heap allocated
        }
    }

    basic_string(const CharT* str, size_type count, Allocator alloc = nullptr)
        : allocator_(alloc) {
        if (!str || count == 0) {
            small_.data_[0] = CharT();
            small_.size_ = 0;
            return;
        }

        if (count <= sso_capacity) {
            std::char_traits<CharT>::copy(small_.data_, str, count);
            small_.data_[count] = CharT();
            small_.size_ = static_cast<std::uint8_t>(count);
        } else {
            large_.size_ = count;
            large_.capacity_ = next_capacity(count);
            large_.ptr_ = allocate(large_.capacity_);
            std::char_traits<CharT>::copy(large_.ptr_, str, count);
            large_.ptr_[count] = CharT();
            small_.size_ |= sso_mask; // Mark as heap allocated
        }
    }

    basic_string(const basic_string& other)
        : allocator_(other.allocator_) {
        if (other.is_small()) {
            std::char_traits<CharT>::copy(small_.data_, other.small_.data_, other.small_.size_);
            small_.data_[other.small_.size_] = CharT();
            small_.size_ = other.small_.size_;
        } else {
            large_.size_ = other.large_.size_;
            large_.capacity_ = next_capacity(other.large_.size_);
            large_.ptr_ = allocate(large_.capacity_);
            std::char_traits<CharT>::copy(large_.ptr_, other.large_.ptr_, other.large_.size_ + 1);
            small_.size_ |= sso_mask;
        }
    }

    basic_string(basic_string&& other) noexcept {
        if (other.is_small()) {
            std::char_traits<CharT>::copy(small_.data_, other.small_.data_, other.small_.size_);
            small_.data_[other.small_.size_] = CharT();
            small_.size_ = other.small_.size_;
            // Leave other empty
            other.small_.data_[0] = CharT();
            other.small_.size_ = 0;
        } else {
            large_.ptr_ = other.large_.ptr_;
            large_.size_ = other.large_.size_;
            large_.capacity_ = other.large_.capacity_;
            small_.size_ |= sso_mask;
            
            other.small_.data_[0] = CharT();
            other.small_.size_ = 0;
        }
        allocator_ = std::exchange(other.allocator_, nullptr);
    }

    ~basic_string() {
        if (!is_small()) {
            deallocate(large_.ptr_, large_.capacity_);
        }
    }

    // Assignment
    basic_string& operator=(const basic_string& other) {
        if (this != &other) {
            basic_string tmp(other);
            swap(tmp);
        }
        return *this;
    }

    basic_string& operator=(basic_string&& other) noexcept {
        if (this != &other) {
            clear();
            if (!is_small()) {
                deallocate(large_.ptr_, large_.capacity_);
            }

            if (other.is_small()) {
                std::char_traits<CharT>::copy(small_.data_, other.small_.data_, other.small_.size_);
                small_.data_[other.small_.size_] = CharT();
                small_.size_ = other.small_.size_;
                // Leave other empty
                other.small_.data_[0] = CharT();
                other.small_.size_ = 0;
            } else {
                large_.ptr_ = other.large_.ptr_;
                large_.size_ = other.large_.size_;
                large_.capacity_ = other.large_.capacity_;
                small_.size_ |= sso_mask;
                
                other.small_.data_[0] = CharT();
                other.small_.size_ = 0;
            }
            allocator_ = std::exchange(other.allocator_, nullptr);
        }
        return *this;
    }

    // Assign from C-string (preserve allocator)
    basic_string& operator=(const CharT* str) {
        if (!str) {
            clear();
            return *this;
        }
        const size_type len = std::char_traits<CharT>::length(str);
        assign_from_ptr_count(str, len);
        return *this;
    }

    // Assign from string_view (preserve allocator)
    basic_string& operator=(std::basic_string_view<CharT> sv) {
        assign_from_ptr_count(sv.data(), sv.size());
        return *this;
    }

    // Element access
    reference operator[](size_type pos) noexcept {
        return is_small() ? small_.data_[pos] : large_.ptr_[pos];
    }

    const_reference operator[](size_type pos) const noexcept {
        return is_small() ? small_.data_[pos] : large_.ptr_[pos];
    }

    reference at(size_type pos) {
        if (pos >= size()) {
            throw std::out_of_range("String index out of range");
        }
        return operator[](pos);
    }

    const_reference at(size_type pos) const {
        if (pos >= size()) {
            throw std::out_of_range("String index out of range");
        }
        return operator[](pos);
    }

    reference front() noexcept {
        return operator[](0);
    }

    const_reference front() const noexcept {
        return operator[](0);
    }

    reference back() noexcept {
        return operator[](size() - 1);
    }

    const_reference back() const noexcept {
        return operator[](size() - 1);
    }

    const CharT* c_str() const noexcept {
        return data();
    }

    const CharT* data() const noexcept {
        return is_small() ? small_.data_ : large_.ptr_;
    }

    CharT* data() noexcept {
        return is_small() ? small_.data_ : large_.ptr_;
    }

    operator std::basic_string_view<CharT>() const noexcept {
        return std::basic_string_view<CharT>(data(), size());
    }

    // Iterators
    iterator begin() noexcept {
        return data();
    }

    const_iterator begin() const noexcept {
        return data();
    }

    const_iterator cbegin() const noexcept {
        return data();
    }

    iterator end() noexcept {
        return data() + size();
    }

    const_iterator end() const noexcept {
        return data() + size();
    }

    const_iterator cend() const noexcept {
        return data() + size();
    }

    // Capacity
    bool empty() const noexcept {
        return size() == 0;
    }

    size_type size() const noexcept {
        return is_small() ? small_.size_ : large_.size_;
    }

    size_type length() const noexcept {
        return size();
    }

    size_type capacity() const noexcept {
        return is_small() ? sso_capacity : large_.capacity_;
    }

    void reserve(size_type new_cap) {
        if (new_cap <= capacity()) {
            return;
        }

        new_cap = next_capacity(new_cap);
        if (is_small()) {
            auto* new_ptr = allocate(new_cap);
            std::char_traits<CharT>::copy(new_ptr, small_.data_, small_.size_ + 1);
            large_.ptr_ = new_ptr;
            large_.size_ = small_.size_;
            large_.capacity_ = new_cap;
            small_.size_ |= sso_mask;
        } else {
            auto* new_ptr = allocate(new_cap);
            std::char_traits<CharT>::copy(new_ptr, large_.ptr_, large_.size_ + 1);
            deallocate(large_.ptr_, large_.capacity_);
            large_.ptr_ = new_ptr;
            large_.capacity_ = new_cap;
        }
    }

    void shrink_to_fit() {
        if (is_small() || large_.size_ == large_.capacity_) {
            return;
        }

        if (large_.size_ <= sso_capacity) {
            // Switch to small string
            CharT tmp[sso_capacity + 1];
            std::char_traits<CharT>::copy(tmp, large_.ptr_, large_.size_ + 1);
            deallocate(large_.ptr_, large_.capacity_);
            std::char_traits<CharT>::copy(small_.data_, tmp, large_.size_ + 1);
            small_.size_ = static_cast<std::uint8_t>(large_.size_);
        } else {
            // Reallocate with exact size
            auto* new_ptr = allocate(large_.size_ + 1);
            std::char_traits<CharT>::copy(new_ptr, large_.ptr_, large_.size_ + 1);
            deallocate(large_.ptr_, large_.capacity_);
            large_.ptr_ = new_ptr;
            large_.capacity_ = large_.size_ + 1;
        }
    }

    // Operations
    void clear() noexcept {
        if (is_small()) {
            small_.data_[0] = CharT();
            small_.size_ = 0;
        } else {
            large_.ptr_[0] = CharT();
            large_.size_ = 0;
        }
    }

    basic_string& insert(size_type pos, const CharT* str, size_type count) {
        if (pos > size()) {
            throw std::out_of_range("String position out of range");
        }
        
        if (count == 0) {
            return *this;
        }

        const size_type old_size = size();
        const size_type new_size = old_size + count;

        if (new_size <= capacity()) {
            if (is_small()) {
                if (pos < old_size) {
                    std::char_traits<CharT>::move(small_.data_ + pos + count,
                                                small_.data_ + pos,
                                                old_size - pos + 1);
                }
                std::char_traits<CharT>::copy(small_.data_ + pos, str, count);
                small_.size_ = static_cast<std::uint8_t>(new_size);
                small_.data_[new_size] = CharT();
            } else {
                if (pos < old_size) {
                    std::char_traits<CharT>::move(large_.ptr_ + pos + count,
                                                large_.ptr_ + pos,
                                                old_size - pos + 1);
                }
                std::char_traits<CharT>::copy(large_.ptr_ + pos, str, count);
                large_.size_ = new_size;
                large_.ptr_[new_size] = CharT();
            }
        } else {
            const size_type new_capacity = next_capacity(new_size);
            auto* new_ptr = allocate(new_capacity);
            
            if (pos > 0) {
                std::char_traits<CharT>::copy(new_ptr, data(), pos);
            }
            std::char_traits<CharT>::copy(new_ptr + pos, str, count);
            if (pos < old_size) {
                std::char_traits<CharT>::copy(new_ptr + pos + count,
                                            data() + pos,
                                            old_size - pos + 1);
            } else {
                new_ptr[new_size] = CharT();
            }

            if (!is_small()) {
                deallocate(large_.ptr_, large_.capacity_);
            }
            large_.ptr_ = new_ptr;
            large_.size_ = new_size;
            large_.capacity_ = new_capacity;
            small_.size_ |= sso_mask;
        }
        return *this;
    }

    basic_string& insert(size_type pos, const basic_string& str) {
        return insert(pos, str.data(), str.size());
    }

    basic_string& append(const CharT* str, size_type count) {
        return insert(size(), str, count);
    }

    basic_string& append(const basic_string& str) {
        return append(str.data(), str.size());
    }

    basic_string& operator+=(const basic_string& str) {
        return append(str);
    }

    basic_string& operator+=(const CharT* str) {
        return append(str, std::char_traits<CharT>::length(str));
    }

    basic_string& operator+=(CharT ch) {
        const size_type old_size = size();
        const size_type new_size = old_size + 1;

        if (new_size <= capacity()) {
            if (is_small()) {
                small_.data_[old_size] = ch;
                small_.data_[new_size] = CharT();
                small_.size_ = static_cast<std::uint8_t>(new_size);
            } else {
                large_.ptr_[old_size] = ch;
                large_.ptr_[new_size] = CharT();
                large_.size_ = new_size;
            }
        } else {
            const size_type new_capacity = next_capacity(new_size);
            auto* new_ptr = allocate(new_capacity);
            std::char_traits<CharT>::copy(new_ptr, data(), old_size);
            new_ptr[old_size] = ch;
            new_ptr[new_size] = CharT();

            if (!is_small()) {
                deallocate(large_.ptr_, large_.capacity_);
            }
            large_.ptr_ = new_ptr;
            large_.size_ = new_size;
            large_.capacity_ = new_capacity;
            small_.size_ |= sso_mask;
        }
        return *this;
    }

    int compare(const basic_string& str) const noexcept {
        const size_type lhs_sz = size();
        const size_type rhs_sz = str.size();
        const int result = std::char_traits<CharT>::compare(data(),
                                                          str.data(),
                                                          std::min(lhs_sz, rhs_sz));
        if (result != 0) {
            return result;
        }
        if (lhs_sz < rhs_sz) {
            return -1;
        }
        if (lhs_sz > rhs_sz) {
            return 1;
        }
        return 0;
    }

    basic_string substr(size_type pos = 0, size_type count = npos) const {
        if (pos > size()) {
            throw std::out_of_range("String position out of range");
        }
        count = std::min(count, size() - pos);
        return basic_string(data() + pos, count, allocator_);
    }

    void swap(basic_string& other) noexcept {
        if (this == &other) {
            return;
        }

        if (is_small() && other.is_small()) {
            // Swap small strings
            CharT tmp_data[sso_capacity + 1];
            std::char_traits<CharT>::copy(tmp_data, small_.data_, small_.size_ + 1);
            std::char_traits<CharT>::copy(small_.data_, other.small_.data_, other.small_.size_ + 1);
            std::char_traits<CharT>::copy(other.small_.data_, tmp_data, small_.size_ + 1);
            std::swap(small_.size_, other.small_.size_);
        } else if (!is_small() && !other.is_small()) {
            // Swap large strings
            std::swap(large_.ptr_, other.large_.ptr_);
            std::swap(large_.size_, other.large_.size_);
            std::swap(large_.capacity_, other.large_.capacity_);
        } else {
            // One small, one large
            CharT tmp_data[sso_capacity + 1];
            size_type tmp_size;
            
            if (is_small()) {
                std::char_traits<CharT>::copy(tmp_data, small_.data_, small_.size_ + 1);
                tmp_size = small_.size_;
                
                large_.ptr_ = other.large_.ptr_;
                large_.size_ = other.large_.size_;
                large_.capacity_ = other.large_.capacity_;
                small_.size_ |= sso_mask;
                
                std::char_traits<CharT>::copy(other.small_.data_, tmp_data, tmp_size + 1);
                other.small_.size_ = static_cast<std::uint8_t>(tmp_size);
            } else {
                std::char_traits<CharT>::copy(tmp_data, other.small_.data_, other.small_.size_ + 1);
                tmp_size = other.small_.size_;
                
                other.large_.ptr_ = large_.ptr_;
                other.large_.size_ = large_.size_;
                other.large_.capacity_ = large_.capacity_;
                other.small_.size_ |= sso_mask;
                
                std::char_traits<CharT>::copy(small_.data_, tmp_data, tmp_size + 1);
                small_.size_ = static_cast<std::uint8_t>(tmp_size);
            }
        }
        std::swap(allocator_, other.allocator_);
    }

    static constexpr size_type npos = static_cast<size_type>(-1);

private:
    bool is_small() const noexcept {
        return (small_.size_ & sso_mask) == 0;
    }

    pointer allocate(size_type n) {
        if (!allocator_) {
            throw std::runtime_error("No allocator provided");
        }
        return static_cast<pointer>(
            allocator_->allocate(n * sizeof(CharT), alignof(CharT))
        );
    }

    void deallocate(pointer p, size_type n) {
        if (p && allocator_) {
            allocator_->deallocate(p);
        }
    }

    static size_type next_capacity(size_type n) {
        constexpr size_type min_capacity = 16;
        n = std::max(n + 1, min_capacity);  // +1 for null terminator
        return (n + n/2);  // Grow by 1.5x
    }

    void assign_from_ptr_count(const CharT* src, size_type count) {
        if (count <= sso_capacity) {
            // Switch to small if necessary
            if (!is_small()) {
                deallocate(large_.ptr_, large_.capacity_);
            }
            std::char_traits<CharT>::copy(small_.data_, src, count);
            small_.data_[count] = CharT();
            small_.size_ = static_cast<std::uint8_t>(count);
        } else {
            size_type needed_cap = next_capacity(count);
            if (is_small()) {
                large_.ptr_ = allocate(needed_cap);
                large_.capacity_ = needed_cap;
            } else if (needed_cap > large_.capacity_) {
                auto* new_ptr = allocate(needed_cap);
                deallocate(large_.ptr_, large_.capacity_);
                large_.ptr_ = new_ptr;
                large_.capacity_ = needed_cap;
            }
            std::char_traits<CharT>::copy(large_.ptr_, src, count);
            large_.ptr_[count] = CharT();
            large_.size_ = count;
            small_.size_ |= sso_mask;
        }
    }
};

template<typename CharT, typename Allocator>
bool operator==(const basic_string<CharT, Allocator>& lhs,
                const basic_string<CharT, Allocator>& rhs) noexcept {
    return lhs.compare(rhs) == 0;
}

template<typename CharT, typename Allocator>
bool operator!=(const basic_string<CharT, Allocator>& lhs,
                const basic_string<CharT, Allocator>& rhs) noexcept {
    return !(lhs == rhs);
}

template<typename CharT, typename Allocator>
bool operator<(const basic_string<CharT, Allocator>& lhs,
               const basic_string<CharT, Allocator>& rhs) noexcept {
    return lhs.compare(rhs) < 0;
}

template<typename CharT, typename Allocator>
bool operator<=(const basic_string<CharT, Allocator>& lhs,
                const basic_string<CharT, Allocator>& rhs) noexcept {
    return lhs.compare(rhs) <= 0;
}

template<typename CharT, typename Allocator>
bool operator>(const basic_string<CharT, Allocator>& lhs,
               const basic_string<CharT, Allocator>& rhs) noexcept {
    return lhs.compare(rhs) > 0;
}

template<typename CharT, typename Allocator>
bool operator>=(const basic_string<CharT, Allocator>& lhs,
                const basic_string<CharT, Allocator>& rhs) noexcept {
    return lhs.compare(rhs) >= 0;
}

// Type aliases
using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

} // namespace memory
} // namespace pynovage