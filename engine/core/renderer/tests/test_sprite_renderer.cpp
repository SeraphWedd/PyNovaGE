#include <gtest/gtest.h>
#include "renderer/sprite_renderer.hpp"
#include "renderer/texture.hpp"
#include <vectors/vector2.hpp>
#include <memory>

using namespace PyNovaGE::Renderer;
using namespace PyNovaGE; // For Vector2f, etc.

class SpriteRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup for sprite renderer tests
    }
};

// Tests for Sprite data structure (non-OpenGL dependent)
TEST(SpriteTest, SpriteDefaultConstruction) {
    Sprite sprite;
    
    // Test default values
    EXPECT_EQ(sprite.position.x, 0.0f);
    EXPECT_EQ(sprite.position.y, 0.0f);
    EXPECT_EQ(sprite.rotation, 0.0f);
    EXPECT_EQ(sprite.scale.x, 1.0f);
    EXPECT_EQ(sprite.scale.y, 1.0f);
    EXPECT_EQ(sprite.origin.x, 0.5f);
    EXPECT_EQ(sprite.origin.y, 0.5f);
    EXPECT_EQ(sprite.color.x, 1.0f); // R
    EXPECT_EQ(sprite.color.y, 1.0f); // G
    EXPECT_EQ(sprite.color.z, 1.0f); // B
    EXPECT_EQ(sprite.color.w, 1.0f); // A
    EXPECT_EQ(sprite.size.x, 1.0f);
    EXPECT_EQ(sprite.size.y, 1.0f);
    EXPECT_EQ(sprite.texture, nullptr);
    
    // Test default texture coordinates (full texture)
    EXPECT_EQ(sprite.texture_coords[0].x, 0.0f); // Bottom-left U
    EXPECT_EQ(sprite.texture_coords[0].y, 0.0f); // Bottom-left V
    EXPECT_EQ(sprite.texture_coords[1].x, 1.0f); // Bottom-right U
    EXPECT_EQ(sprite.texture_coords[1].y, 0.0f); // Bottom-right V
    EXPECT_EQ(sprite.texture_coords[2].x, 1.0f); // Top-right U
    EXPECT_EQ(sprite.texture_coords[2].y, 1.0f); // Top-right V
    EXPECT_EQ(sprite.texture_coords[3].x, 0.0f); // Top-left U
    EXPECT_EQ(sprite.texture_coords[3].y, 1.0f); // Top-left V
}

TEST(SpriteTest, SpriteSetTextureRegionNormalized) {
    Sprite sprite;
    
    // Set normalized UV coordinates
    sprite.SetTextureRegionNormalized(0.25f, 0.5f, 0.75f, 1.0f);
    
    EXPECT_FLOAT_EQ(sprite.texture_coords[0].x, 0.25f); // Bottom-left U
    EXPECT_FLOAT_EQ(sprite.texture_coords[0].y, 0.5f);  // Bottom-left V
    EXPECT_FLOAT_EQ(sprite.texture_coords[1].x, 0.75f); // Bottom-right U
    EXPECT_FLOAT_EQ(sprite.texture_coords[1].y, 0.5f);  // Bottom-right V
    EXPECT_FLOAT_EQ(sprite.texture_coords[2].x, 0.75f); // Top-right U
    EXPECT_FLOAT_EQ(sprite.texture_coords[2].y, 1.0f);  // Top-right V
    EXPECT_FLOAT_EQ(sprite.texture_coords[3].x, 0.25f); // Top-left U
    EXPECT_FLOAT_EQ(sprite.texture_coords[3].y, 1.0f);  // Top-left V
}

TEST(SpriteTest, SpriteSetTextureRegionWithoutTexture) {
    Sprite sprite; // No texture
    
    // This should not crash and should do nothing
    sprite.SetTextureRegion(0.0f, 0.0f, 32.0f, 32.0f);
    
    // Texture coordinates should remain default
    EXPECT_EQ(sprite.texture_coords[0].x, 0.0f);
    EXPECT_EQ(sprite.texture_coords[0].y, 0.0f);
}

// Tests for SpriteRenderer
TEST_F(SpriteRendererTest, SpriteRendererCreation) {
    SpriteRenderer renderer;
    
    // Should start uninitialized
    EXPECT_FALSE(renderer.IsInitialized());
}

TEST_F(SpriteRendererTest, SpriteRendererInitialization) {
    SpriteRenderer renderer;
    
    // Note: This test can't actually initialize OpenGL without a context,
    // but we can test the interface
    EXPECT_FALSE(renderer.IsInitialized());
    
    // In a real scenario with OpenGL context:
    // EXPECT_TRUE(renderer.Initialize());
    // EXPECT_TRUE(renderer.IsInitialized());
}

TEST_F(SpriteRendererTest, SpriteRendererShutdown) {
    SpriteRenderer renderer;
    
    // Should be able to call shutdown even if not initialized
    renderer.Shutdown();
    EXPECT_FALSE(renderer.IsInitialized());
}
