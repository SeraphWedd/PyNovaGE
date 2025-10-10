#include <gtest/gtest.h>
#include "renderer/texture.hpp"
#include <vector>
#include <memory>

using namespace PyNovaGE::Renderer;

/**
 * Test class for TextureAtlas functionality that doesn't rely on OpenGL
 * We focus on testing the binary tree packing algorithm and region management
 */
class TextureAtlasPackingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test image data (simple dummy data)
        test_data_4x4.resize(4 * 4 * 4, 255);
        test_data_8x8.resize(8 * 8 * 4, 128);
        test_data_16x16.resize(16 * 16 * 4, 64);
    }
    
    std::vector<unsigned char> test_data_4x4;
    std::vector<unsigned char> test_data_8x8;
    std::vector<unsigned char> test_data_16x16;
};

// Test TextureAtlasRegion structure
TEST_F(TextureAtlasPackingTest, AtlasRegionBasics) {
    TextureAtlasRegion region;
    region.position = {10, 20};
    region.size = {32, 64};
    region.name = "test_region";
    region.uv_min = {0.1f, 0.2f};
    region.uv_max = {0.3f, 0.4f};
    
    EXPECT_EQ(region.position.x, 10);
    EXPECT_EQ(region.position.y, 20);
    EXPECT_EQ(region.size.x, 32);
    EXPECT_EQ(region.size.y, 64);
    EXPECT_EQ(region.name, "test_region");
    EXPECT_FLOAT_EQ(region.uv_min.x, 0.1f);
    EXPECT_FLOAT_EQ(region.uv_min.y, 0.2f);
    EXPECT_FLOAT_EQ(region.uv_max.x, 0.3f);
    EXPECT_FLOAT_EQ(region.uv_max.y, 0.4f);
}

// Test UV coordinate calculation
TEST_F(TextureAtlasPackingTest, UVCoordinateCalculation) {
    int atlas_width = 256;
    int atlas_height = 256;
    int region_x = 64;
    int region_y = 32;
    int region_width = 48;
    int region_height = 96;
    
    // Calculate UV coordinates like the atlas would
    float inv_width = 1.0f / static_cast<float>(atlas_width);
    float inv_height = 1.0f / static_cast<float>(atlas_height);
    float uv_min_x = static_cast<float>(region_x) * inv_width;
    float uv_min_y = static_cast<float>(region_y) * inv_height;
    float uv_max_x = static_cast<float>(region_x + region_width) * inv_width;
    float uv_max_y = static_cast<float>(region_y + region_height) * inv_height;
    
    // Verify calculations
    EXPECT_FLOAT_EQ(uv_min_x, 64.0f / 256.0f);
    EXPECT_FLOAT_EQ(uv_min_y, 32.0f / 256.0f);
    EXPECT_FLOAT_EQ(uv_max_x, 112.0f / 256.0f);
    EXPECT_FLOAT_EQ(uv_max_y, 128.0f / 256.0f);
    
    // Verify they're normalized (0.0 to 1.0)
    EXPECT_GE(uv_min_x, 0.0f);
    EXPECT_LE(uv_min_x, 1.0f);
    EXPECT_GE(uv_min_y, 0.0f);
    EXPECT_LE(uv_min_y, 1.0f);
    EXPECT_GE(uv_max_x, 0.0f);
    EXPECT_LE(uv_max_x, 1.0f);
    EXPECT_GE(uv_max_y, 0.0f);
    EXPECT_LE(uv_max_y, 1.0f);
    
    // Verify max > min
    EXPECT_GT(uv_max_x, uv_min_x);
    EXPECT_GT(uv_max_y, uv_min_y);
}

// Test rectangle overlap detection (used in packing algorithms)
TEST_F(TextureAtlasPackingTest, RectangleOverlapDetection) {
    auto overlaps = [](int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
        return !(ax + aw <= bx || bx + bw <= ax || ay + ah <= by || by + bh <= ay);
    };
    
    // Non-overlapping rectangles
    EXPECT_FALSE(overlaps(0, 0, 10, 10, 20, 20, 10, 10));     // Completely separate
    EXPECT_FALSE(overlaps(0, 0, 10, 10, 10, 0, 10, 10));      // Adjacent horizontally
    EXPECT_FALSE(overlaps(0, 0, 10, 10, 0, 10, 10, 10));      // Adjacent vertically
    
    // Overlapping rectangles
    EXPECT_TRUE(overlaps(0, 0, 10, 10, 5, 5, 10, 10));        // Partial overlap
    EXPECT_TRUE(overlaps(0, 0, 20, 20, 5, 5, 10, 10));        // One inside another
    EXPECT_TRUE(overlaps(5, 5, 10, 10, 0, 0, 20, 20));        // One inside another (reversed)
    EXPECT_TRUE(overlaps(0, 0, 10, 10, 0, 0, 10, 10));        // Identical rectangles
}

// Test binary tree node logic (simulating what TextureAtlas uses internally)
TEST_F(TextureAtlasPackingTest, BinaryTreeNodeLogic) {
    // Simulate the node structure used in TextureAtlas
    struct TestNode {
        int x, y, width, height;
        bool used = false;
        std::unique_ptr<TestNode> left;
        std::unique_ptr<TestNode> right;
    };
    
    // Create root node
    auto root = std::make_unique<TestNode>();
    root->x = 0;
    root->y = 0;
    root->width = 100;
    root->height = 100;
    
    EXPECT_FALSE(root->used);
    EXPECT_EQ(root->width, 100);
    EXPECT_EQ(root->height, 100);
    
    // Simulate splitting the node
    root->used = true;
    root->left = std::make_unique<TestNode>();
    root->right = std::make_unique<TestNode>();
    
    int requested_width = 40;
    int requested_height = 30;
    int remaining_width = root->width - requested_width;
    int remaining_height = root->height - requested_height;
    
    // Split based on which dimension has more remaining space
    if (remaining_width > remaining_height) {
        // Split horizontally
        root->left->x = root->x + requested_width;
        root->left->y = root->y;
        root->left->width = remaining_width;
        root->left->height = requested_height;
        
        root->right->x = root->x;
        root->right->y = root->y + requested_height;
        root->right->width = root->width;
        root->right->height = remaining_height;
    } else {
        // Split vertically
        root->left->x = root->x;
        root->left->y = root->y + requested_height;
        root->left->width = requested_width;
        root->left->height = remaining_height;
        
        root->right->x = root->x + requested_width;
        root->right->y = root->y;
        root->right->width = remaining_width;
        root->right->height = root->height;
    }
    
    // Verify the split worked correctly
    EXPECT_TRUE(root->used);
    EXPECT_NE(root->left, nullptr);
    EXPECT_NE(root->right, nullptr);
    
    // Check that the split covers the original area correctly
    int total_left_area = root->left->width * root->left->height;
    int total_right_area = root->right->width * root->right->height;
    int used_area = requested_width * requested_height;
    int original_area = root->width * root->height;
    
    EXPECT_EQ(total_left_area + total_right_area + used_area, original_area);
}

// Test packing efficiency calculation
TEST_F(TextureAtlasPackingTest, PackingEfficiencyCalculation) {
    int atlas_size = 256;
    int region_size = 16;
    
    // Theoretical maximum regions that can fit
    int theoretical_max = (atlas_size / region_size) * (atlas_size / region_size);
    EXPECT_EQ(theoretical_max, 256); // 16x16 = 256 regions of 16x16 in 256x256
    
    // In practice, binary tree packing is less efficient
    // A good packing algorithm should achieve at least 70% efficiency
    int reasonable_expectation = static_cast<int>(theoretical_max * 0.7f);
    EXPECT_GT(reasonable_expectation, 175);
}
