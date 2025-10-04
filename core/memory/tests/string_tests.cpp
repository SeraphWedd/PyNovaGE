#include "string.hpp"
#include "linear_allocator.hpp"
#include <gtest/gtest.h>
#include <memory>

using namespace pynovage::memory;

// Test fixture for String tests
class StringTest : public ::testing::Test {
protected:
    static constexpr std::size_t kDefaultAlignment = 16;
    std::unique_ptr<LinearAllocator<kDefaultAlignment>> allocator_;
    
    void SetUp() override {
        allocator_ = std::make_unique<LinearAllocator<kDefaultAlignment>>(1024 * 1024);
    }
    
    void TearDown() override {
        allocator_.reset();
    }
};

TEST_F(StringTest, DefaultConstructor) {
    string s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
    EXPECT_STREQ(s.c_str(), "");
}

TEST_F(StringTest, SmallStringOptimization) {
    string s(allocator_.get());
    s = "small";
    
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "small");
    EXPECT_TRUE(s.capacity() >= 5);
    
    // Force heap allocation
    string large(allocator_.get());
    const char* kLong = "This is a much longer string that won't fit in the small buffer optimization";
    large = kLong;
    
    const auto kLongLen = static_cast<size_t>(std::char_traits<char>::length(kLong));
    EXPECT_EQ(large.size(), kLongLen);
    EXPECT_STREQ(large.c_str(), kLong);
    EXPECT_TRUE(large.capacity() >= kLongLen);
}

TEST_F(StringTest, CopyConstructor) {
    string s(allocator_.get());
    s = "test string";
    
    string copy(s);
    EXPECT_EQ(copy.size(), s.size());
    EXPECT_STREQ(copy.c_str(), s.c_str());
}

TEST_F(StringTest, MoveConstructor) {
    string s(allocator_.get());
    s = "test string";
    
    string moved(std::move(s));
    EXPECT_EQ(moved.size(), 11);
    EXPECT_STREQ(moved.c_str(), "test string");
    EXPECT_TRUE(s.empty());
}

TEST_F(StringTest, Assignment) {
    string s1(allocator_.get());
    string s2(allocator_.get());
    
    s1 = "first";
    s2 = "second";
    
    s1 = s2;
    EXPECT_EQ(s1.size(), 6);
    EXPECT_STREQ(s1.c_str(), "second");
}

TEST_F(StringTest, MoveAssignment) {
    string s1(allocator_.get());
    string s2(allocator_.get());
    
    s1 = "first";
    s2 = "second";
    
    s1 = std::move(s2);
    EXPECT_EQ(s1.size(), 6);
    EXPECT_STREQ(s1.c_str(), "second");
    EXPECT_TRUE(s2.empty());
}

TEST_F(StringTest, ElementAccess) {
    string s(allocator_.get());
    s = "test";
    
    EXPECT_EQ(s[0], 't');
    EXPECT_EQ(s[1], 'e');
    EXPECT_EQ(s[2], 's');
    EXPECT_EQ(s[3], 't');
    
    EXPECT_EQ(s.at(0), 't');
    EXPECT_THROW(s.at(4), std::out_of_range);
    
    EXPECT_EQ(s.front(), 't');
    EXPECT_EQ(s.back(), 't');
}

TEST_F(StringTest, Append) {
    string s(allocator_.get());
    s = "Hello";
    
    s.append(" World");
    EXPECT_EQ(s.size(), 11);
    EXPECT_STREQ(s.c_str(), "Hello World");
    
    s += "!";
    EXPECT_EQ(s.size(), 12);
    EXPECT_STREQ(s.c_str(), "Hello World!");
}

TEST_F(StringTest, Insert) {
    string s(allocator_.get());
    s = "Hello World";
    
    s.insert(6, "Beautiful ");
    const char* kInserted = "Hello Beautiful World";
    EXPECT_EQ(s.size(), static_cast<size_t>(std::char_traits<char>::length(kInserted)));
    EXPECT_STREQ(s.c_str(), kInserted);
    
    EXPECT_THROW(s.insert(100, "!", 1), std::out_of_range);
}

TEST_F(StringTest, Substr) {
    string s(allocator_.get());
    s = "Hello World";
    
    string sub = s.substr(6, 5);
    EXPECT_EQ(sub.size(), 5);
    EXPECT_STREQ(sub.c_str(), "World");
    
    EXPECT_THROW(s.substr(12), std::out_of_range);
}

TEST_F(StringTest, Comparison) {
    string s1(allocator_.get());
    string s2(allocator_.get());
    
    s1 = "abc";
    s2 = "abc";
    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 != s2);
    EXPECT_FALSE(s1 < s2);
    
    s2 = "abd";
    EXPECT_TRUE(s1 != s2);
    EXPECT_TRUE(s1 < s2);
    EXPECT_FALSE(s1 > s2);
}

TEST_F(StringTest, Clear) {
    string s(allocator_.get());
    s = "test";
    
    s.clear();
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
    EXPECT_STREQ(s.c_str(), "");
}

TEST_F(StringTest, Reserve) {
    string s(allocator_.get());
    
    s.reserve(100);
    EXPECT_GE(s.capacity(), 100);
    EXPECT_EQ(s.size(), 0);
    
    s = "test";
    EXPECT_EQ(s.size(), 4);
    EXPECT_STREQ(s.c_str(), "test");
}

TEST_F(StringTest, ShrinkToFit) {
    string s(allocator_.get());
    s = "test string that will cause heap allocation";
    
    auto cap_before = s.capacity();
    s = "small";
    s.shrink_to_fit();
    
    EXPECT_LT(s.capacity(), cap_before);
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "small");
}

TEST_F(StringTest, SwapSmall) {
    string s1(allocator_.get());
    string s2(allocator_.get());
    
    s1 = "first";
    s2 = "second";
    
    s1.swap(s2);
    EXPECT_STREQ(s1.c_str(), "second");
    EXPECT_STREQ(s2.c_str(), "first");
}

TEST_F(StringTest, SwapLarge) {
    string s1(allocator_.get());
    string s2(allocator_.get());
    
    s1 = "This is a longer string that will use heap allocation";
    s2 = "Another heap-allocated string that's quite long";
    
    s1.swap(s2);
    EXPECT_STREQ(s1.c_str(), "Another heap-allocated string that's quite long");
    EXPECT_STREQ(s2.c_str(), "This is a longer string that will use heap allocation");
}

TEST_F(StringTest, SwapMixed) {
    string s1(allocator_.get());
    string s2(allocator_.get());
    
    s1 = "small";
    s2 = "This is a longer string that will use heap allocation";
    
    s1.swap(s2);
    EXPECT_STREQ(s1.c_str(), "This is a longer string that will use heap allocation");
    EXPECT_STREQ(s2.c_str(), "small");
}

TEST_F(StringTest, StringView) {
    string s(allocator_.get());
    s = "test string";
    
    std::string_view view = s;
    EXPECT_EQ(view.size(), s.size());
    EXPECT_EQ(view, "test string");
}

TEST_F(StringTest, Unicode) {
    u16string s16(allocator_.get());
    const char16_t* kU16 = u"Hello 世界";
    s16 = kU16;
    
    EXPECT_EQ(s16.size(), static_cast<size_t>(std::char_traits<char16_t>::length(kU16)));
    
    u32string s32(allocator_.get());
    const char32_t* kU32 = U"Hello 🌍";
    s32 = kU32;
    
    EXPECT_EQ(s32.size(), static_cast<size_t>(std::char_traits<char32_t>::length(kU32)));
}

TEST_F(StringTest, MultipleAllocations) {
    string s(allocator_.get());
    
    // Start small
    s = "small";
    EXPECT_EQ(s.size(), 5);
    
    // Grow larger
    const char* kGrow = "This string is longer and will need heap allocation";
    s = kGrow;
    EXPECT_EQ(s.size(), static_cast<size_t>(std::char_traits<char>::length(kGrow)));
    
    // Shrink back to small
    s = "tiny";
    EXPECT_EQ(s.size(), 4);
    
    // Verify content is preserved through all operations
    EXPECT_STREQ(s.c_str(), "tiny");
}