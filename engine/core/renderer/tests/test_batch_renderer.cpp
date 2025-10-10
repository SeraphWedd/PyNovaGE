#include <gtest/gtest.h>
#include "renderer/batch_renderer.hpp"
#include "renderer/sprite_renderer.hpp"
#include <vectors/vector2.hpp>
#include <vectors/vector4.hpp>
#include <vector>

using namespace PyNovaGE::Renderer;
using namespace PyNovaGE;

class BatchRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Tests that don't require OpenGL context
    }
    
    void TearDown() override {
        // Clean up any allocated resources
    }
    
    BatchRenderer batch_renderer_;
};

// Test basic BatchRenderer creation
TEST_F(BatchRendererTest, BatchRendererCreation) {
    // Should be able to create a BatchRenderer instance
    EXPECT_NO_THROW(BatchRenderer renderer);
    
    // BatchRenderer should not be initialized by default
    EXPECT_FALSE(batch_renderer_.IsInitialized());
}


// Test BatchRenderer shutdown interface
TEST_F(BatchRendererTest, BatchRendererShutdown) {
    // Should be able to call Shutdown safely even if not initialized
    EXPECT_NO_THROW(batch_renderer_.Shutdown());
}

// Test batch statistics interface
TEST_F(BatchRendererTest, BatchStatistics) {
    auto stats = batch_renderer_.GetStats();
    
    // Initial stats should be zero
    EXPECT_EQ(stats.draw_calls, 0u);
    EXPECT_EQ(stats.sprites_batched, 0u);
    EXPECT_EQ(stats.batches_flushed, 0u);
    EXPECT_EQ(stats.texture_binds, 0u);
    EXPECT_EQ(stats.avg_sprites_per_batch, 0.0f);
}

// Test batch operations interface (without OpenGL context)
TEST_F(BatchRendererTest, BatchOperationsInterface) {
    // Should be able to call batch operations without crashing
    // (They will likely do nothing or fail gracefully without OpenGL context)
    EXPECT_NO_THROW(batch_renderer_.BeginBatch());
    EXPECT_NO_THROW(batch_renderer_.EndBatch());
    EXPECT_NO_THROW(batch_renderer_.FlushBatch());
    EXPECT_NO_THROW(batch_renderer_.ResetStats());
}

// Test sprite addition interface
TEST_F(BatchRendererTest, SpriteAdditionInterface) {
    // Create a test sprite
    Sprite sprite;
    sprite.position = Vector2f(100.0f, 200.0f);
    sprite.size = Vector2f(32.0f, 32.0f);
    sprite.color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
    sprite.rotation = 0.0f;
    // Note: texture_region is now texture_coords array in new Sprite struct
    sprite.texture = nullptr; // Default texture
    
    // Should be able to call AddSprite without crashing
    EXPECT_NO_THROW(batch_renderer_.AddSprite(sprite));
}

// Test multiple sprite additions
TEST_F(BatchRendererTest, MultipleSpriteAdditions) {
    std::vector<Sprite> sprites;
    
    // Create multiple test sprites
    for (int i = 0; i < 10; ++i) {
        Sprite sprite;
        sprite.position = Vector2f(static_cast<float>(i * 50), static_cast<float>(i * 50));
        sprite.size = Vector2f(32.0f, 32.0f);
        sprite.color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
        sprite.rotation = 0.0f;
        sprite.texture = nullptr; // Default texture (no texture cycling for now)
        sprites.push_back(sprite);
    }
    
    // Should be able to add all sprites without crashing
    for (const auto& sprite : sprites) {
        EXPECT_NO_THROW(batch_renderer_.AddSprite(sprite));
    }
}

// Test batch lifecycle
TEST_F(BatchRendererTest, BatchLifecycle) {
    // Test a complete batch lifecycle
    EXPECT_NO_THROW({
        batch_renderer_.BeginBatch();
        
        // Add some sprites
        Sprite sprite1;
        sprite1.position = Vector2f(0.0f, 0.0f);
        sprite1.size = Vector2f(32.0f, 32.0f);
        sprite1.color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
        sprite1.texture = nullptr;
        
        Sprite sprite2;
        sprite2.position = Vector2f(50.0f, 50.0f);
        sprite2.size = Vector2f(32.0f, 32.0f);
        sprite2.color = Vector4f(0.0f, 1.0f, 0.0f, 1.0f);
        sprite2.texture = nullptr;
        
        batch_renderer_.AddSprite(sprite1);
        batch_renderer_.AddSprite(sprite2);
        
        batch_renderer_.EndBatch();
    });
}

// Test reset functionality
TEST_F(BatchRendererTest, ResetFunctionality) {
    // Add some sprites
    Sprite sprite;
    sprite.position = Vector2f(0.0f, 0.0f);
    sprite.size = Vector2f(32.0f, 32.0f);
    sprite.color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
    sprite.texture = nullptr;
    
    EXPECT_NO_THROW({
        batch_renderer_.BeginBatch();
        batch_renderer_.AddSprite(sprite);
        batch_renderer_.EndBatch(); // This will flush and reset the batch
    });
}

// Test different sprite properties
TEST_F(BatchRendererTest, SpritePropertiesHandling) {
    // Test sprite with different properties
    Sprite sprite;
    sprite.position = Vector2f(-100.0f, 300.0f);
    sprite.size = Vector2f(64.0f, 128.0f);
    sprite.color = Vector4f(0.5f, 0.8f, 0.2f, 0.9f);
    sprite.rotation = 45.0f; // Rotated sprite
    sprite.SetTextureRegionNormalized(0.25f, 0.25f, 0.75f, 0.75f); // Partial texture region
    sprite.texture = nullptr;
    
    EXPECT_NO_THROW(batch_renderer_.AddSprite(sprite));
}

// Test edge cases
TEST_F(BatchRendererTest, EdgeCases) {
    // Test zero-sized sprite
    Sprite zero_sprite;
    zero_sprite.position = Vector2f(0.0f, 0.0f);
    zero_sprite.size = Vector2f(0.0f, 0.0f);
    zero_sprite.color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
    zero_sprite.texture = nullptr;
    
    EXPECT_NO_THROW(batch_renderer_.AddSprite(zero_sprite));
    
    // Test sprite with negative size
    Sprite negative_sprite;
    negative_sprite.position = Vector2f(0.0f, 0.0f);
    negative_sprite.size = Vector2f(-32.0f, -32.0f);
    negative_sprite.color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
    negative_sprite.texture = nullptr;
    
    EXPECT_NO_THROW(batch_renderer_.AddSprite(negative_sprite));
    
    // Test sprite with extreme rotation
    Sprite rotated_sprite;
    rotated_sprite.position = Vector2f(0.0f, 0.0f);
    rotated_sprite.size = Vector2f(32.0f, 32.0f);
    rotated_sprite.color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
    rotated_sprite.rotation = 720.0f; // Two full rotations
    rotated_sprite.texture = nullptr;
    
    EXPECT_NO_THROW(batch_renderer_.AddSprite(rotated_sprite));
}

// Test statistics reset
TEST_F(BatchRendererTest, StatisticsReset) {
    // Get initial stats
    auto initial_stats = batch_renderer_.GetStats();
    
    // Stats should start at zero
    EXPECT_EQ(initial_stats.draw_calls, 0u);
    EXPECT_EQ(initial_stats.sprites_batched, 0u);
    EXPECT_EQ(initial_stats.batches_flushed, 0u);
    EXPECT_EQ(initial_stats.texture_binds, 0u);
    EXPECT_EQ(initial_stats.avg_sprites_per_batch, 0.0f);
    
    // After reset, stats should still be zero
    batch_renderer_.ResetStats();
    auto reset_stats = batch_renderer_.GetStats();
    
    EXPECT_EQ(reset_stats.draw_calls, 0u);
    EXPECT_EQ(reset_stats.sprites_batched, 0u);
    EXPECT_EQ(reset_stats.batches_flushed, 0u);
    EXPECT_EQ(reset_stats.texture_binds, 0u);
    EXPECT_EQ(reset_stats.avg_sprites_per_batch, 0.0f);
}