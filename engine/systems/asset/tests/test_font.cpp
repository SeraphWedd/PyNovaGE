#include <gtest/gtest.h>
#include "asset/font.hpp"

using namespace PyNovaGE::Asset;

class FontTest : public ::testing::Test {
protected:
    void SetUp() override {
        // We can't easily create a valid TTF file for testing
        // So we'll test the interface without actually loading a font file
    }
};

TEST_F(FontTest, FontConstruction) {
    Font font("test.ttf", 16.0f);
    
    EXPECT_EQ(font.GetSize(), 16.0f);
    EXPECT_EQ(font.GetPath(), "test.ttf");
    EXPECT_EQ(font.GetType(), AssetType::Font);
    EXPECT_FALSE(font.IsLoaded());
    
    // Test default metrics for unloaded font
    EXPECT_EQ(font.GetAscent(), 0);
    EXPECT_EQ(font.GetDescent(), 0);
    EXPECT_EQ(font.GetLineGap(), 0);
}

TEST_F(FontTest, TextMeasurementWithoutFont) {
    Font font("test.ttf", 16.0f);
    
    // Should return zero for unloaded font
    auto size = font.MeasureText("Hello World");
    EXPECT_EQ(size.x, 0);
    EXPECT_EQ(size.y, 0);
    
    // Should return null for unloaded font
    auto glyph = font.GetGlyph('A');
    EXPECT_EQ(glyph, nullptr);
    
    // Should return zero kerning for unloaded font
    auto kerning = font.GetKerning('A', 'V');
    EXPECT_EQ(kerning, 0);
}

TEST_F(FontTest, FontLoadingError) {
    Font font("nonexistent.ttf", 24.0f);
    
    // Should fail to load non-existent file
    bool loaded = font.LoadFromFile("nonexistent.ttf");
    EXPECT_FALSE(loaded);
    EXPECT_FALSE(font.IsLoaded());
}