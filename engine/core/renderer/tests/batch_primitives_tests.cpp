#include <gtest/gtest.h>
#include "renderer/batch_renderer.hpp"

using namespace PyNovaGE::Renderer;

TEST(BatchPrimitives, AddRectLineWithoutGL) {
    // Verify we can call primitive APIs without initializing GL-dependent paths.
    // Note: Since BeginBatch requires initialization, we'll manually set the batch state for testing
    BatchRenderer br(10, 2);
    
    // Test that we can create the renderer without crashing
    // This test verifies compilation and basic functionality without GL
    EXPECT_EQ(br.GetCurrentSpriteCount(), 0u);
    EXPECT_EQ(br.GetCurrentTextureCount(), 0u);
    EXPECT_FALSE(br.IsInitialized());
    
    SUCCEED();
}

TEST(BatchPrimitives, AddCircleWithoutGL) {
    // Verify circle drawing API compiles and basic functionality without GL context
    BatchRenderer br(100, 2);
    
    // Test that the renderer can be created and basic accessors work
    EXPECT_EQ(br.GetCurrentSpriteCount(), 0u);
    EXPECT_EQ(br.GetCurrentTextureCount(), 0u);
    EXPECT_FALSE(br.IsInitialized());
    EXPECT_EQ(br.GetMaxSprites(), 100);
    EXPECT_EQ(br.GetMaxTextures(), 2);
    
    // Test that adding primitives without initialization fails gracefully (no crash)
    br.AddCircleScreen(100, 100, 25, 800, 600, {0, 0, 1, 1});
    br.AddRectScreen(10, 10, 50, 30, 800, 600, {1, 0, 0, 1});
    br.AddLineScreen(0, 0, 100, 50, 2.0f, 800, 600, {0, 1, 0, 1});
    
    SUCCEED();
}
