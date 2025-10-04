#include "utf.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <string>

using namespace pynovage::memory::unicode;

// Helper functions for test readability
template<typename T>
std::vector<T> encode_utf8_to_vec(std::uint32_t cp) {
    std::vector<T> result;
    encode_utf8(cp, std::back_inserter(result));
    return result;
}

template<typename T>
std::vector<T> encode_utf16_to_vec(std::uint32_t cp) {
    std::vector<T> result;
    encode_utf16(cp, std::back_inserter(result));
    return result;
}

TEST(UnicodeTest, IsValidCodePoint) {
    EXPECT_TRUE(is_valid_code_point(0x0020));  // Space
    EXPECT_TRUE(is_valid_code_point(0x007F));  // DEL
    EXPECT_TRUE(is_valid_code_point(0x0080));  // Padding Character
    EXPECT_TRUE(is_valid_code_point(0x10FFFF)); // Max valid code point
    
    EXPECT_FALSE(is_valid_code_point(0x110000)); // Too large
    EXPECT_FALSE(is_valid_code_point(0xD800));   // Surrogate range
    EXPECT_FALSE(is_valid_code_point(0xDFFF));   // Surrogate range
}

TEST(UnicodeTest, EncodeUtf8) {
    // ASCII
    {
        auto result = encode_utf8_to_vec<std::uint8_t>(0x0024); // $
        std::vector<std::uint8_t> expected = {0x24};
        EXPECT_EQ(result, expected);
    }
    
    // 2-byte sequence
    {
        auto result = encode_utf8_to_vec<std::uint8_t>(0x00A2); // ¢
        std::vector<std::uint8_t> expected = {0xC2, 0xA2};
        EXPECT_EQ(result, expected);
    }
    
    // 3-byte sequence
    {
        auto result = encode_utf8_to_vec<std::uint8_t>(0x20AC); // €
        std::vector<std::uint8_t> expected = {0xE2, 0x82, 0xAC};
        EXPECT_EQ(result, expected);
    }
    
    // 4-byte sequence
    {
        auto result = encode_utf8_to_vec<std::uint8_t>(0x10348); // 𐍈
        std::vector<std::uint8_t> expected = {0xF0, 0x90, 0x8D, 0x88};
        EXPECT_EQ(result, expected);
    }
    
    // Invalid code points
    EXPECT_THROW(encode_utf8_to_vec<std::uint8_t>(0x110000), encoding_error);
    EXPECT_THROW(encode_utf8_to_vec<std::uint8_t>(0xD800), encoding_error);
}

TEST(UnicodeTest, DecodeUtf8) {
    // ASCII
    {
        std::vector<std::uint8_t> input = {0x24}; // $
        auto [cp, next] = decode_utf8(input.begin(), input.end());
        EXPECT_EQ(cp, 0x0024);
        EXPECT_EQ(next, input.end());
    }
    
    // 2-byte sequence
    {
        std::vector<std::uint8_t> input = {0xC2, 0xA2}; // ¢
        auto [cp, next] = decode_utf8(input.begin(), input.end());
        EXPECT_EQ(cp, 0x00A2);
        EXPECT_EQ(next, input.end());
    }
    
    // 3-byte sequence
    {
        std::vector<std::uint8_t> input = {0xE2, 0x82, 0xAC}; // €
        auto [cp, next] = decode_utf8(input.begin(), input.end());
        EXPECT_EQ(cp, 0x20AC);
        EXPECT_EQ(next, input.end());
    }
    
    // 4-byte sequence
    {
        std::vector<std::uint8_t> input = {0xF0, 0x90, 0x8D, 0x88}; // 𐍈
        auto [cp, next] = decode_utf8(input.begin(), input.end());
        EXPECT_EQ(cp, 0x10348);
        EXPECT_EQ(next, input.end());
    }
    
    // Error cases
    std::vector<std::uint8_t> incomplete = {0xE2, 0x82}; // Incomplete €
    EXPECT_THROW(decode_utf8(incomplete.begin(), incomplete.end()), encoding_error);
    
    std::vector<std::uint8_t> invalid = {0xFF}; // Invalid UTF-8 byte
    EXPECT_THROW(decode_utf8(invalid.begin(), invalid.end()), encoding_error);
}

TEST(UnicodeTest, EncodeUtf16) {
    // BMP character
    {
        auto result = encode_utf16_to_vec<std::uint16_t>(0x20AC); // €
        std::vector<std::uint16_t> expected = {0x20AC};
        EXPECT_EQ(result, expected);
    }
    
    // Supplementary plane character
    {
        auto result = encode_utf16_to_vec<std::uint16_t>(0x10348); // 𐍈
        std::vector<std::uint16_t> expected = {0xD800, 0xDF48};
        EXPECT_EQ(result, expected);
    }
    
    // Invalid code points
    EXPECT_THROW(encode_utf16_to_vec<std::uint16_t>(0x110000), encoding_error);
    EXPECT_THROW(encode_utf16_to_vec<std::uint16_t>(0xD800), encoding_error);
}

TEST(UnicodeTest, DecodeUtf16) {
    // BMP character
    {
        std::vector<std::uint16_t> input = {0x20AC}; // €
        auto [cp, next] = decode_utf16(input.begin(), input.end());
        EXPECT_EQ(cp, 0x20AC);
        EXPECT_EQ(next, input.end());
    }
    
    // Supplementary plane character
    {
        std::vector<std::uint16_t> input = {0xD800, 0xDF48}; // 𐍈
        auto [cp, next] = decode_utf16(input.begin(), input.end());
        EXPECT_EQ(cp, 0x10348);
        EXPECT_EQ(next, input.end());
    }
    
    // Error cases
    std::vector<std::uint16_t> incomplete = {0xD800}; // Incomplete surrogate pair
    EXPECT_THROW(decode_utf16(incomplete.begin(), incomplete.end()), encoding_error);
    
    std::vector<std::uint16_t> invalid = {0xDC00}; // Invalid surrogate pair order
    EXPECT_THROW(decode_utf16(invalid.begin(), invalid.end()), encoding_error);
}

TEST(UnicodeTest, Utf8Utf16Conversion) {
    // Test mixed ASCII and Unicode text
    std::vector<std::uint8_t> utf8_input = {
        0x48, 0x65, 0x6C, 0x6C, 0x6F,             // "Hello"
        0x20,                                      // Space
        0xF0, 0x9F, 0x8C, 0x8E,                   // 🌎
        0x20,                                      // Space
        0xE4, 0xB8, 0x96, 0xE7, 0x95, 0x8C       // 世界
    };
    
    std::vector<std::uint16_t> utf16;
    utf8_to_utf16(utf8_input, utf16);
    
    std::vector<std::uint8_t> utf8_output;
    utf16_to_utf8(utf16, utf8_output);
    
    EXPECT_EQ(utf8_input, utf8_output);
}

TEST(UnicodeTest, Utf8Utf32Conversion) {
    // Test mixed ASCII and Unicode text
    std::vector<std::uint8_t> utf8_input = {
        0x48, 0x65, 0x6C, 0x6C, 0x6F,             // "Hello"
        0x20,                                      // Space
        0xF0, 0x9F, 0x8C, 0x8E,                   // 🌎
        0x20,                                      // Space
        0xE4, 0xB8, 0x96, 0xE7, 0x95, 0x8C       // 世界
    };
    
    std::vector<std::uint32_t> utf32;
    utf8_to_utf32(utf8_input, utf32);
    
    std::vector<std::uint8_t> utf8_output;
    utf32_to_utf8(utf32, utf8_output);
    
    EXPECT_EQ(utf8_input, utf8_output);
}

TEST(UnicodeTest, Utf16Utf32Conversion) {
    std::vector<std::uint16_t> utf16_input = {
        0x0048, 0x0065, 0x006C, 0x006C, 0x006F,   // "Hello"
        0x0020,                                     // Space
        0xD83C, 0xDF0E,                            // 🌎
        0x0020,                                     // Space
        0x4E16, 0x754C                             // 世界
    };
    
    std::vector<std::uint32_t> utf32;
    utf16_to_utf32(utf16_input, utf32);
    
    std::vector<std::uint16_t> utf16_output;
    utf32_to_utf16(utf32, utf16_output);
    
    EXPECT_EQ(utf16_input, utf16_output);
}

TEST(UnicodeTest, UtfLengthCounting) {
    // "Hello 🌎 世界"
    std::vector<std::uint8_t> utf8_text = {
        0x48, 0x65, 0x6C, 0x6C, 0x6F,             // "Hello" (5 chars)
        0x20,                                      // Space (1 char)
        0xF0, 0x9F, 0x8C, 0x8E,                   // 🌎 (1 char)
        0x20,                                      // Space (1 char)
        0xE4, 0xB8, 0x96, 0xE7, 0x95, 0x8C       // 世界 (2 chars)
    };
    
    EXPECT_EQ(utf8_length(utf8_text), 10u); // 5 + 1 + 1 + 1 + 2 = 10 characters
    
    std::vector<std::uint16_t> utf16_text;
    utf8_to_utf16(utf8_text, utf16_text);
    EXPECT_EQ(utf16_length(utf16_text), 10u);
}

TEST(UnicodeTest, ErrorHandling) {
    // Test overlong UTF-8 sequences
    std::vector<std::uint8_t> overlong = {0xC0, 0x80}; // Overlong encoding of NUL
    EXPECT_THROW(decode_utf8(overlong.begin(), overlong.end()), encoding_error);
    
    // Test truncated UTF-8 sequences
    std::vector<std::uint8_t> truncated = {0xE2, 0x82}; // Truncated €
    EXPECT_THROW(decode_utf8(truncated.begin(), truncated.end()), encoding_error);
    
    // Test invalid UTF-16 surrogate pairs
    std::vector<std::uint16_t> invalid_surrogate = {0xDC00, 0xD800}; // Wrong order
    EXPECT_THROW(decode_utf16(invalid_surrogate.begin(), invalid_surrogate.end()), encoding_error);
}