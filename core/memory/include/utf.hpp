#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <iterator>
#include <array>
#include <bit>
#include <stdexcept>
#include <algorithm>

namespace pynovage {
namespace memory {
namespace unicode {

// Unicode constants
static constexpr std::uint32_t kMaxCodePoint = 0x10FFFF;
static constexpr std::uint32_t kSurrogateStart = 0xD800;
static constexpr std::uint32_t kSurrogateEnd = 0xDFFF;
static constexpr std::uint32_t kBOMCodePoint = 0xFEFF;

// UTF encoding constants
static constexpr std::uint8_t kBOMUtf8[] = {0xEF, 0xBB, 0xBF};
static constexpr std::uint16_t kBOMUtf16LE = 0xFFFE;
static constexpr std::uint16_t kBOMUtf16BE = 0xFEFF;
static constexpr std::uint32_t kBOMUtf32LE = 0xFFFE0000;
static constexpr std::uint32_t kBOMUtf32BE = 0x0000FEFF;

// Unicode error handling
class encoding_error : public std::runtime_error {
public:
    explicit encoding_error(const char* msg) : std::runtime_error(msg) {}
};

// Check if code point is valid
constexpr bool is_valid_code_point(std::uint32_t code_point) noexcept {
    return code_point <= kMaxCodePoint && 
           (code_point < kSurrogateStart || code_point > kSurrogateEnd);
}

// UTF-8 encoding
template<typename OutIter>
OutIter encode_utf8(std::uint32_t code_point, OutIter out) {
    if (!is_valid_code_point(code_point)) {
        throw encoding_error("Invalid Unicode code point");
    }

    if (code_point < 0x80) {
        *out++ = static_cast<std::uint8_t>(code_point);
    } else if (code_point < 0x800) {
        *out++ = static_cast<std::uint8_t>((code_point >> 6) | 0xC0);
        *out++ = static_cast<std::uint8_t>((code_point & 0x3F) | 0x80);
    } else if (code_point < 0x10000) {
        *out++ = static_cast<std::uint8_t>((code_point >> 12) | 0xE0);
        *out++ = static_cast<std::uint8_t>(((code_point >> 6) & 0x3F) | 0x80);
        *out++ = static_cast<std::uint8_t>((code_point & 0x3F) | 0x80);
    } else {
        *out++ = static_cast<std::uint8_t>((code_point >> 18) | 0xF0);
        *out++ = static_cast<std::uint8_t>(((code_point >> 12) & 0x3F) | 0x80);
        *out++ = static_cast<std::uint8_t>(((code_point >> 6) & 0x3F) | 0x80);
        *out++ = static_cast<std::uint8_t>((code_point & 0x3F) | 0x80);
    }
    return out;
}

template<typename InIter>
std::pair<std::uint32_t, InIter> decode_utf8(InIter first, InIter last) {
    if (first == last) {
        throw encoding_error("Incomplete UTF-8 sequence");
    }

    std::uint32_t code_point = 0;
    std::uint8_t byte = static_cast<std::uint8_t>(*first++);
    
    if ((byte & 0x80) == 0) {
        code_point = byte;
    } else if ((byte & 0xE0) == 0xC0) {
        // 2-byte sequence
        if (first == last) {
            throw encoding_error("Incomplete UTF-8 sequence");
        }
        std::uint8_t byte2 = static_cast<std::uint8_t>(*first++);
        if ((byte2 & 0xC0) != 0x80) {
            throw encoding_error("Invalid UTF-8 sequence");
        }
        code_point = ((byte & 0x1F) << 6) | (byte2 & 0x3F);
        if (code_point < 0x80) {
            throw encoding_error("Overlong UTF-8 sequence");
        }
    } else if ((byte & 0xF0) == 0xE0) {
        // 3-byte sequence
        if (std::distance(first, last) < 2) {
            throw encoding_error("Incomplete UTF-8 sequence");
        }
        std::uint8_t byte2 = static_cast<std::uint8_t>(*first++);
        std::uint8_t byte3 = static_cast<std::uint8_t>(*first++);
        if ((byte2 & 0xC0) != 0x80 || (byte3 & 0xC0) != 0x80) {
            throw encoding_error("Invalid UTF-8 sequence");
        }
        code_point = ((byte & 0x0F) << 12) | 
                    ((byte2 & 0x3F) << 6) | 
                    (byte3 & 0x3F);
        if (code_point < 0x800 || 
            (code_point >= kSurrogateStart && code_point <= kSurrogateEnd)) {
            throw encoding_error("Invalid UTF-8 sequence");
        }
    } else if ((byte & 0xF8) == 0xF0) {
        // 4-byte sequence
        if (std::distance(first, last) < 3) {
            throw encoding_error("Incomplete UTF-8 sequence");
        }
        std::uint8_t byte2 = static_cast<std::uint8_t>(*first++);
        std::uint8_t byte3 = static_cast<std::uint8_t>(*first++);
        std::uint8_t byte4 = static_cast<std::uint8_t>(*first++);
        if ((byte2 & 0xC0) != 0x80 || 
            (byte3 & 0xC0) != 0x80 || 
            (byte4 & 0xC0) != 0x80) {
            throw encoding_error("Invalid UTF-8 sequence");
        }
        code_point = ((byte & 0x07) << 18) |
                    ((byte2 & 0x3F) << 12) |
                    ((byte3 & 0x3F) << 6) |
                    (byte4 & 0x3F);
        if (code_point < 0x10000 || code_point > kMaxCodePoint) {
            throw encoding_error("Invalid UTF-8 sequence");
        }
    } else {
        throw encoding_error("Invalid UTF-8 sequence");
    }

    return {code_point, first};
}

// UTF-16 encoding
template<typename OutIter>
OutIter encode_utf16(std::uint32_t code_point, OutIter out) {
    if (!is_valid_code_point(code_point)) {
        throw encoding_error("Invalid Unicode code point");
    }

    if (code_point < 0x10000) {
        *out++ = static_cast<std::uint16_t>(code_point);
    } else {
        code_point -= 0x10000;
        *out++ = static_cast<std::uint16_t>(
            (code_point >> 10) + kSurrogateStart
        );
        *out++ = static_cast<std::uint16_t>(
            (code_point & 0x3FF) + 0xDC00
        );
    }
    return out;
}

template<typename InIter>
std::pair<std::uint32_t, InIter> decode_utf16(InIter first, InIter last) {
    if (first == last) {
        throw encoding_error("Incomplete UTF-16 sequence");
    }

    std::uint32_t code_point = static_cast<std::uint16_t>(*first++);
    if (code_point >= kSurrogateStart && code_point <= kSurrogateEnd) {
        if (code_point >= 0xDC00) {
            throw encoding_error("Invalid UTF-16 sequence");
        }
        if (first == last) {
            throw encoding_error("Incomplete UTF-16 sequence");
        }
        std::uint32_t surrogate = static_cast<std::uint16_t>(*first++);
        if (surrogate < 0xDC00 || surrogate > 0xDFFF) {
            throw encoding_error("Invalid UTF-16 sequence");
        }
        code_point = ((code_point - kSurrogateStart) << 10) +
                    (surrogate - 0xDC00) + 0x10000;
    }

    if (!is_valid_code_point(code_point)) {
        throw encoding_error("Invalid Unicode code point");
    }
    return {code_point, first};
}

// Conversion between encodings
template<typename InContainer, typename OutContainer>
void utf8_to_utf16(const InContainer& utf8, OutContainer& utf16) {
    utf16.clear();
    auto first = std::begin(utf8);
    const auto last = std::end(utf8);
    
    while (first != last) {
        auto [cp, next] = decode_utf8(first, last);
        encode_utf16(cp, std::back_inserter(utf16));
        first = next;
    }
}

template<typename InContainer, typename OutContainer>
void utf16_to_utf8(const InContainer& utf16, OutContainer& utf8) {
    utf8.clear();
    auto first = std::begin(utf16);
    const auto last = std::end(utf16);
    
    while (first != last) {
        auto [cp, next] = decode_utf16(first, last);
        encode_utf8(cp, std::back_inserter(utf8));
        first = next;
    }
}

template<typename InContainer, typename OutContainer>
void utf8_to_utf32(const InContainer& utf8, OutContainer& utf32) {
    utf32.clear();
    auto first = std::begin(utf8);
    const auto last = std::end(utf8);
    
    while (first != last) {
        auto [cp, next] = decode_utf8(first, last);
        utf32.push_back(static_cast<std::uint32_t>(cp));
        first = next;
    }
}

template<typename InContainer, typename OutContainer>
void utf32_to_utf8(const InContainer& utf32, OutContainer& utf8) {
    utf8.clear();
    for (auto cp : utf32) {
        encode_utf8(cp, std::back_inserter(utf8));
    }
}

template<typename InContainer, typename OutContainer>
void utf16_to_utf32(const InContainer& utf16, OutContainer& utf32) {
    utf32.clear();
    auto first = std::begin(utf16);
    const auto last = std::end(utf16);
    
    while (first != last) {
        auto [cp, next] = decode_utf16(first, last);
        utf32.push_back(static_cast<std::uint32_t>(cp));
        first = next;
    }
}

template<typename InContainer, typename OutContainer>
void utf32_to_utf16(const InContainer& utf32, OutContainer& utf16) {
    utf16.clear();
    for (auto cp : utf32) {
        encode_utf16(cp, std::back_inserter(utf16));
    }
}

// String length counting (code points)
template<typename Container>
std::size_t utf8_length(const Container& utf8) {
    std::size_t length = 0;
    auto first = std::begin(utf8);
    const auto last = std::end(utf8);
    
    while (first != last) {
        auto [_, next] = decode_utf8(first, last);
        ++length;
        first = next;
    }
    return length;
}

template<typename Container>
std::size_t utf16_length(const Container& utf16) {
    std::size_t length = 0;
    auto first = std::begin(utf16);
    const auto last = std::end(utf16);
    
    while (first != last) {
        auto [_, next] = decode_utf16(first, last);
        ++length;
        first = next;
    }
    return length;
}

} // namespace unicode
} // namespace memory
} // namespace pynovage