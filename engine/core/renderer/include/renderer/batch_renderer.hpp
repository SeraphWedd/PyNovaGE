#pragma once

#include <vectors/vector2.hpp>
#include <vectors/vector3.hpp>
#include <vectors/vector4.hpp>
#include "renderer/sprite_renderer.hpp"
#include "renderer/shader.hpp"
#include <memory>
#include <vector>
#include <array>

namespace PyNovaGE {
namespace Renderer {

// Forward declarations
class Texture;

/**
 * @brief Vertex data for batch rendering
 * 
 * Contains all data needed to render a sprite vertex in a batch.
 */
struct BatchVertex {
    Vector3f position;      ///< Vertex position (x, y, z)
    Vector2f texCoords;     ///< Texture coordinates (u, v)
    Vector4f color;         ///< Vertex color (r, g, b, a)
    float textureIndex;     ///< Texture slot index (0-31)
    
    BatchVertex() = default;
    
    BatchVertex(const Vector3f& pos, const Vector2f& uv, const Vector4f& col, float texIdx)
        : position(pos), texCoords(uv), color(col), textureIndex(texIdx) {}
};

/**
 * @brief Batch statistics for performance monitoring
 */
struct BatchStats {
    size_t draw_calls = 0;           ///< Number of draw calls made
    size_t sprites_batched = 0;      ///< Total sprites processed
    size_t batches_flushed = 0;      ///< Number of batches flushed
    size_t texture_binds = 0;        ///< Number of texture binds
    float avg_sprites_per_batch = 0.0f; ///< Average sprites per batch
    
    void Reset() {
        draw_calls = 0;
        sprites_batched = 0;
        batches_flushed = 0;
        texture_binds = 0;
        avg_sprites_per_batch = 0.0f;
    }
    
    void UpdateAverage() {
        if (batches_flushed > 0) {
            avg_sprites_per_batch = static_cast<float>(sprites_batched) / static_cast<float>(batches_flushed);
        }
    }
};

/**
 * @brief Batch renderer for efficient sprite rendering
 * 
 * Batches multiple sprites into fewer draw calls for better performance.
 * Uses a single vertex buffer and multiple texture slots to minimize state changes.
 */
class BatchRenderer {
public:
    /**
     * @brief Constructor
     * @param max_sprites Maximum sprites per batch (default: 1000)
     * @param max_textures Maximum textures per batch (default: 16)
     */
    BatchRenderer(int max_sprites = 1000, int max_textures = 16);
    
    /**
     * @brief Destructor
     */
    ~BatchRenderer();
    
    // Non-copyable but movable
    BatchRenderer(const BatchRenderer&) = delete;
    BatchRenderer& operator=(const BatchRenderer&) = delete;
    BatchRenderer(BatchRenderer&&) = default;
    BatchRenderer& operator=(BatchRenderer&&) = default;
    
    /**
     * @brief Initialize the batch renderer
     * @return true if successful, false otherwise
     */
    bool Initialize();
    
    /**
     * @brief Shutdown and cleanup resources
     */
    void Shutdown();
    
    /**
     * @brief Check if renderer is initialized
     */
    bool IsInitialized() const { return initialized_; }
    
    /**
     * @brief Begin a new batch
     * Must be called before adding sprites to batch
     */
    void BeginBatch();
    
    /**
     * @brief Add a sprite to the current batch
     * @param sprite Sprite to add to batch
     * @return true if added successfully, false if batch is full
     */
    bool AddSprite(const Sprite& sprite);
    
    /**
     * @brief Flush the current batch to GPU
     * Renders all sprites in the current batch and resets for next batch
     */
    void FlushBatch();
    
    /**
     * @brief End the current batch
     * Automatically flushes any remaining sprites
     */
    void EndBatch();
    
    /**
     * @brief Render multiple sprites in batches
     * @param sprites Array of sprites to render
     * @param count Number of sprites in array
     */
    void RenderSprites(const Sprite* sprites, size_t count);
    
    /**
     * @brief Render a vector of sprites in batches
     * @param sprites Vector of sprites to render
     */
    void RenderSprites(const std::vector<Sprite>& sprites);
    
    /**
     * @brief Get batch statistics
     */
    const BatchStats& GetStats() const { return stats_; }
    
    /**
     * @brief Reset batch statistics
     */
    void ResetStats() { stats_.Reset(); }
    
    /**
     * @brief Get maximum sprites per batch
     */
    int GetMaxSprites() const { return max_sprites_; }
    
    /**
     * @brief Get maximum textures per batch
     */
    int GetMaxTextures() const { return max_textures_; }
    
    /**
     * @brief Get current sprite count in batch
     */
    size_t GetCurrentSpriteCount() const { return current_sprite_count_; }
    
    /**
     * @brief Get current texture count in batch
     */
    size_t GetCurrentTextureCount() const { return current_texture_count_; }

private:
    /**
     * @brief Setup vertex buffer and vertex array
     */
    void SetupVertexBuffer();
    
    /**
     * @brief Generate vertices for a sprite and add to batch
     * @param sprite Sprite to generate vertices for
     * @param texture_index Index of texture in current batch
     */
    void AddSpriteVertices(const Sprite& sprite, float texture_index);
    
    /**
     * @brief Find or add texture to current batch
     * @param texture Texture to find/add
     * @return Texture index in batch, or -1 if batch is full
     */
    int FindOrAddTexture(std::shared_ptr<Texture> texture);
    
    /**
     * @brief Reset current batch data
     */
    void ResetBatch();
    
    /**
     * @brief Generate vertices for sprite transformation
     * @param sprite Sprite to transform
     * @param vertices Output array of 4 vertices
     */
    void GenerateTransformedVertices(const Sprite& sprite, std::array<Vector3f, 4>& vertices);
    
    // Configuration
    int max_sprites_;           ///< Maximum sprites per batch
    int max_textures_;          ///< Maximum textures per batch
    bool initialized_ = false;  ///< Initialization state
    
    // OpenGL objects
    unsigned int batch_vao_ = 0;    ///< Vertex Array Object
    unsigned int batch_vbo_ = 0;    ///< Vertex Buffer Object
    unsigned int batch_ebo_ = 0;    ///< Element Buffer Object
    
    // Batch data
    std::vector<BatchVertex> vertices_;                     ///< Vertex buffer data
    std::vector<unsigned int> indices_;                     ///< Index buffer data
    std::vector<std::shared_ptr<Texture>> batch_textures_;  ///< Textures in current batch
    
    // Batch state
    size_t current_sprite_count_ = 0;   ///< Current sprites in batch
    size_t current_texture_count_ = 0;  ///< Current textures in batch
    bool batch_started_ = false;        ///< Whether batch is active
    
    // Rendering resources
    std::shared_ptr<Shader> batch_shader_;  ///< Shader for batch rendering
    
    // Statistics
    BatchStats stats_;
};

} // namespace Renderer
} // namespace PyNovaGE
