#include "string_view.hpp"
#include "linear_allocator.hpp"
#include "string.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace pynovage::memory;

TEST(StringViewTest, DefaultConstructor) {
    string_view sv;
    EXPECT_TRUE(sv.empty());
    EXPECT_EQ(sv.size(), 0);
    EXPECT_EQ(sv.data(), nullptr);
}

TEST(StringViewTest, CStringConstructor) {
    const char* str = "Hello";
    string_view sv(str);
    EXPECT_EQ(sv.size(), 5);
    EXPECT_EQ(sv.data(), str);
    EXPECT_FALSE(sv.empty());
}

TEST(StringViewTest, SubstringConstructor) {
    const char* str = "Hello World";
    string_view sv(str + 6, 5);  // "World"
    EXPECT_EQ(sv.size(), 5);
    EXPECT_EQ(sv.data(), str + 6);
    EXPECT_FALSE(sv.empty());
}

TEST(StringViewTest, StdStringViewConstructor) {
    std::string_view stdv = "Hello";
    string_view sv(stdv);
    EXPECT_EQ(sv.size(), stdv.size());
    EXPECT_EQ(sv.data(), stdv.data());
}

TEST(StringViewTest, ElementAccess) {
    string_view sv = "Hello";
    
    EXPECT_EQ(sv[0], 'H');
    EXPECT_EQ(sv[4], 'o');
    
    EXPECT_EQ(sv.at(0), 'H');
    EXPECT_THROW(sv.at(5), std::out_of_range);
    
    EXPECT_EQ(sv.front(), 'H');
    EXPECT_EQ(sv.back(), 'o');
}

TEST(StringViewTest, Iteration) {
    string_view sv = "Hello";
    std::string result;
    
    for (char c : sv) {
        result += c;
    }
    
    EXPECT_EQ(result, "Hello");
}

TEST(StringViewTest, RemovePrefix) {
    string_view sv = "Hello World";
    sv.remove_prefix(6);  // "World"
    
    EXPECT_EQ(sv.size(), 5);
    EXPECT_EQ(sv, "World");
}

TEST(StringViewTest, RemoveSuffix) {
    string_view sv = "Hello World";
    sv.remove_suffix(6);  // "Hello"
    
    EXPECT_EQ(sv.size(), 5);
    EXPECT_EQ(sv, "Hello");
}

TEST(StringViewTest, Substr) {
    string_view sv = "Hello World";
    
    auto sub = sv.substr(6, 5);  // "World"
    EXPECT_EQ(sub.size(), 5);
    EXPECT_EQ(sub, "World");
    
    EXPECT_THROW(sv.substr(12), std::out_of_range);
}

TEST(StringViewTest, Compare) {
    string_view sv1 = "Hello";
    string_view sv2 = "Hello";
    string_view sv3 = "World";
    
    EXPECT_EQ(sv1.compare(sv2), 0);
    EXPECT_LT(sv1.compare(sv3), 0);
    EXPECT_GT(sv3.compare(sv1), 0);
}

TEST(StringViewTest, StartsWith) {
    string_view sv = "Hello World";
    
    EXPECT_TRUE(sv.starts_with("Hello"));
    EXPECT_TRUE(sv.starts_with("H"));
    EXPECT_FALSE(sv.starts_with("World"));
}

TEST(StringViewTest, EndsWith) {
    string_view sv = "Hello World";
    
    EXPECT_TRUE(sv.ends_with("World"));
    EXPECT_TRUE(sv.ends_with("d"));
    EXPECT_FALSE(sv.ends_with("Hello"));
}

TEST(StringViewTest, Find) {
    string_view sv = "Hello World";
    
    EXPECT_EQ(sv.find("World"), 6);
    EXPECT_EQ(sv.find("o"), 4);
    EXPECT_EQ(sv.find("xyz"), string_view::npos);
    EXPECT_EQ(sv.find("World", 7), string_view::npos);
}

TEST(StringViewTest, RFind) {
    string_view sv = "Hello World";
    
    EXPECT_EQ(sv.rfind("o"), 7);
    EXPECT_EQ(sv.rfind("l"), 9);
    EXPECT_EQ(sv.rfind("xyz"), string_view::npos);
    EXPECT_EQ(sv.rfind("Hello", 1), 0);
}

TEST(StringViewTest, ComparisonOperators) {
    string_view sv1 = "Hello";
    string_view sv2 = "Hello";
    string_view sv3 = "World";
    
    EXPECT_TRUE(sv1 == sv2);
    EXPECT_FALSE(sv1 != sv2);
    EXPECT_TRUE(sv1 < sv3);
    EXPECT_TRUE(sv1 <= sv2);
    EXPECT_TRUE(sv3 > sv1);
    EXPECT_TRUE(sv2 >= sv1);
}

TEST(StringViewTest, StdStringViewConversion) {
    string_view sv = "Hello";
    std::string_view stdv = sv;
    
    EXPECT_EQ(stdv.size(), sv.size());
    EXPECT_EQ(stdv.data(), sv.data());
}

TEST(StringViewTest, StringViewOnBasicString) {
    // Test with our own basic_string implementation
    string str = "Hello World";
    string_view sv = str;
    
    EXPECT_EQ(sv.size(), str.size());
    EXPECT_EQ(sv.data(), str.data());
}

TEST(StringViewTest, Unicode) {
    const char16_t* kU16 = u"Hello 世界";
    u16string_view sv16 = kU16;
    EXPECT_EQ(sv16.size(), static_cast<size_t>(std::char_traits<char16_t>::length(kU16)));
    
    const char32_t* kU32 = U"Hello 🌍";
    u32string_view sv32 = kU32;
    EXPECT_EQ(sv32.size(), static_cast<size_t>(std::char_traits<char32_t>::length(kU32)));
}

TEST(StringViewTest, EmptyStringBehavior) {
    string_view sv1;  // nullptr
    string_view sv2("");  // empty but not nullptr
    
    EXPECT_TRUE(sv1.empty());
    EXPECT_TRUE(sv2.empty());
    EXPECT_EQ(sv1.size(), 0);
    EXPECT_EQ(sv2.size(), 0);
    EXPECT_EQ(sv1.data(), nullptr);
    EXPECT_NE(sv2.data(), nullptr);
}