#pragma once

#include <cstddef>
#include <string_view>
#include <type_traits>
#include <stdexcept>

namespace pynovage {
namespace memory {

template<typename CharT>
class basic_string_view {
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
    using iterator = const CharT*;
    using const_iterator = const CharT*;

    static constexpr size_type npos = static_cast<size_type>(-1);

    // Constructors
    constexpr basic_string_view() noexcept
        : data_(nullptr), size_(0) {}

    constexpr basic_string_view(const CharT* str) noexcept
        : data_(str), size_(str ? std::char_traits<CharT>::length(str) : 0) {}

    constexpr basic_string_view(const CharT* str, size_type len) noexcept
        : data_(str), size_(len) {}

    constexpr basic_string_view(const std::basic_string_view<CharT>& other) noexcept
        : data_(other.data()), size_(other.size()) {}

    // Assignment
    constexpr basic_string_view& operator=(const basic_string_view& view) noexcept = default;

    // Element access
    constexpr const_reference operator[](size_type pos) const noexcept {
        return data_[pos];
    }

    constexpr const_reference at(size_type pos) const {
        if (pos >= size_) {
            throw std::out_of_range("String_view index out of range");
        }
        return data_[pos];
    }

    constexpr const_reference front() const noexcept {
        return data_[0];
    }

    constexpr const_reference back() const noexcept {
        return data_[size_ - 1];
    }

    constexpr const_pointer data() const noexcept {
        return data_;
    }

    // Iterators
    constexpr const_iterator begin() const noexcept {
        return data_;
    }

    constexpr const_iterator cbegin() const noexcept {
        return begin();
    }

    constexpr const_iterator end() const noexcept {
        return data_ + size_;
    }

    constexpr const_iterator cend() const noexcept {
        return end();
    }

    // Capacity
    constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    constexpr size_type size() const noexcept {
        return size_;
    }

    constexpr size_type length() const noexcept {
        return size_;
    }

    constexpr size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max();
    }

    // Operations
    constexpr void remove_prefix(size_type n) noexcept {
        data_ += n;
        size_ -= n;
    }

    constexpr void remove_suffix(size_type n) noexcept {
        size_ -= n;
    }

    constexpr void swap(basic_string_view& v) noexcept {
        auto tmp_data = data_;
        auto tmp_size = size_;
        data_ = v.data_;
        size_ = v.size_;
        v.data_ = tmp_data;
        v.size_ = tmp_size;
    }

    constexpr basic_string_view substr(size_type pos = 0, size_type count = npos) const {
        if (pos > size_) {
            throw std::out_of_range("String_view::substr");
        }
        const size_type rcount = std::min(count, size_ - pos);
        return basic_string_view(data_ + pos, rcount);
    }

    constexpr int compare(basic_string_view v) const noexcept {
        const size_type rlen = std::min(size_, v.size_);
        const int result = std::char_traits<CharT>::compare(data_, v.data_, rlen);
        if (result != 0) {
            return result;
        }
        if (size_ < v.size_) {
            return -1;
        }
        if (size_ > v.size_) {
            return 1;
        }
        return 0;
    }

    constexpr bool starts_with(basic_string_view v) const noexcept {
        return size_ >= v.size_ && std::char_traits<CharT>::compare(data_, v.data_, v.size_) == 0;
    }

    constexpr bool ends_with(basic_string_view v) const noexcept {
        return size_ >= v.size_ && std::char_traits<CharT>::compare(data_ + (size_ - v.size_), v.data_, v.size_) == 0;
    }

    constexpr size_type find(basic_string_view v, size_type pos = 0) const noexcept {
        if (pos > size_ || v.empty() || v.size_ > size_ - pos) {
            return npos;
        }
        
        for (size_type i = pos; i <= size_ - v.size_; ++i) {
            if (std::char_traits<CharT>::compare(data_ + i, v.data_, v.size_) == 0) {
                return i;
            }
        }
        return npos;
    }

    constexpr size_type rfind(basic_string_view v, size_type pos = npos) const noexcept {
        if (v.empty()) {
            return std::min(pos, size_);
        }
        if (v.size_ > size_) {
            return npos;
        }
        pos = std::min(pos, size_ - v.size_);
        
        for (size_type i = pos + 1; i-- > 0;) {
            if (std::char_traits<CharT>::compare(data_ + i, v.data_, v.size_) == 0) {
                return i;
            }
        }
        return npos;
    }

    // Conversion to std::basic_string_view
    constexpr operator std::basic_string_view<CharT>() const noexcept {
        return std::basic_string_view<CharT>(data_, size_);
    }

private:
    const CharT* data_;
    size_type size_;
};

// Comparison operators
template<typename CharT>
constexpr bool operator==(basic_string_view<CharT> lhs,
                         basic_string_view<CharT> rhs) noexcept {
    return lhs.compare(rhs) == 0;
}

template<typename CharT>
constexpr bool operator==(basic_string_view<CharT> lhs,
                         const CharT* rhs) noexcept {
    return lhs.compare(basic_string_view<CharT>(rhs)) == 0;
}

template<typename CharT>
constexpr bool operator==(const CharT* lhs,
                         basic_string_view<CharT> rhs) noexcept {
    return basic_string_view<CharT>(lhs).compare(rhs) == 0;
}

template<typename CharT>
constexpr bool operator!=(basic_string_view<CharT> lhs,
                         basic_string_view<CharT> rhs) noexcept {
    return !(lhs == rhs);
}

template<typename CharT>
constexpr bool operator!=(basic_string_view<CharT> lhs,
                         const CharT* rhs) noexcept {
    return !(lhs == rhs);
}

template<typename CharT>
constexpr bool operator!=(const CharT* lhs,
                         basic_string_view<CharT> rhs) noexcept {
    return !(lhs == rhs);
}

template<typename CharT>
constexpr bool operator<(basic_string_view<CharT> lhs,
                        basic_string_view<CharT> rhs) noexcept {
    return lhs.compare(rhs) < 0;
}

template<typename CharT>
constexpr bool operator<(basic_string_view<CharT> lhs,
                        const CharT* rhs) noexcept {
    return lhs.compare(basic_string_view<CharT>(rhs)) < 0;
}

template<typename CharT>
constexpr bool operator<(const CharT* lhs,
                        basic_string_view<CharT> rhs) noexcept {
    return basic_string_view<CharT>(lhs).compare(rhs) < 0;
}

template<typename CharT>
constexpr bool operator<=(basic_string_view<CharT> lhs,
                         basic_string_view<CharT> rhs) noexcept {
    return lhs.compare(rhs) <= 0;
}

template<typename CharT>
constexpr bool operator<=(basic_string_view<CharT> lhs,
                         const CharT* rhs) noexcept {
    return lhs.compare(basic_string_view<CharT>(rhs)) <= 0;
}

template<typename CharT>
constexpr bool operator<=(const CharT* lhs,
                         basic_string_view<CharT> rhs) noexcept {
    return basic_string_view<CharT>(lhs).compare(rhs) <= 0;
}

template<typename CharT>
constexpr bool operator>(basic_string_view<CharT> lhs,
                        basic_string_view<CharT> rhs) noexcept {
    return lhs.compare(rhs) > 0;
}

template<typename CharT>
constexpr bool operator>(basic_string_view<CharT> lhs,
                        const CharT* rhs) noexcept {
    return lhs.compare(basic_string_view<CharT>(rhs)) > 0;
}

template<typename CharT>
constexpr bool operator>(const CharT* lhs,
                        basic_string_view<CharT> rhs) noexcept {
    return basic_string_view<CharT>(lhs).compare(rhs) > 0;
}

template<typename CharT>
constexpr bool operator>=(basic_string_view<CharT> lhs,
                         basic_string_view<CharT> rhs) noexcept {
    return lhs.compare(rhs) >= 0;
}

template<typename CharT>
constexpr bool operator>=(basic_string_view<CharT> lhs,
                         const CharT* rhs) noexcept {
    return lhs.compare(basic_string_view<CharT>(rhs)) >= 0;
}

template<typename CharT>
constexpr bool operator>=(const CharT* lhs,
                         basic_string_view<CharT> rhs) noexcept {
    return basic_string_view<CharT>(lhs).compare(rhs) >= 0;
}

// Type aliases
using string_view = basic_string_view<char>;
using wstring_view = basic_string_view<wchar_t>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;

} // namespace memory
} // namespace pynovage