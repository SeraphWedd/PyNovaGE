#pragma once

#include <vectors/vector2.hpp>
#include <vectors/vector3.hpp>
#include <vectors/vector4.hpp>
#include <memory>
#include "renderer/texture.hpp"
#include "renderer/shader.hpp"

namespace PyNovaGE {
namespace Renderer {

/**
 * @brief Sprite data structure
 * 
 * Contains all data needed to render a 2D sprite including position,
 * rotation, scale, color, texture coordinates, and texture reference.
 */
class Sprite {
public:
    // Transform properties
    Vector2f position = {0.0f, 0.0f};           ///< World position
    float rotation = 0.0f;                      ///< Rotation in radians
    Vector2f scale = {1.0f, 1.0f};              ///< Scale factor
    Vector2f origin = {0.5f, 0.5f};             ///< Origin point (0.0-1.0 normalized)
    
    // Visual properties
    Vector4f color = {1.0f, 1.0f, 1.0f, 1.0f}; ///< Tint color (RGBA)
    
    // Texture properties
    std::shared_ptr<Texture> texture = nullptr; ///< Texture reference
    Vector2f texture_coords[4] = {              ///< UV coordinates for quad vertices
        {0.0f, 0.0f},  // Bottom-left
        {1.0f, 0.0f},  // Bottom-right
        {1.0f, 1.0f},  // Top-right
        {0.0f, 1.0f}   // Top-left
    };
    
    // Size properties
    Vector2f size = {1.0f, 1.0f};               ///< Sprite size in world units
    
    /**
     * @brief Default constructor
     */
    Sprite() = default;
    
    /**
     * @brief Constructor with position and texture
     * @param pos Sprite position
     * @param tex Texture reference
     */
    Sprite(const Vector2f& pos, std::shared_ptr<Texture> tex)
        : position(pos), texture(tex) {
        if (texture) {
            // Default size to texture dimensions
            size = {static_cast<float>(texture->GetWidth()), 
                   static_cast<float>(texture->GetHeight())};
        }
    }
    
    /**
     * @brief Set texture region (for sprite sheets)
     * @param x X coordinate in texture (pixels)
     * @param y Y coordinate in texture (pixels)
     * @param width Width of region (pixels)
     * @param height Height of region (pixels)
     */
    void SetTextureRegion(float x, float y, float width, float height) {
        if (!texture) return;
        
        float tex_width = static_cast<float>(texture->GetWidth());
        float tex_height = static_cast<float>(texture->GetHeight());
        
        float left = x / tex_width;
        float right = (x + width) / tex_width;
        float bottom = y / tex_height;
        float top = (y + height) / tex_height;
        
        texture_coords[0] = {left, bottom};   // Bottom-left
        texture_coords[1] = {right, bottom};  // Bottom-right
        texture_coords[2] = {right, top};     // Top-right
        texture_coords[3] = {left, top};      // Top-left
        
        // Update sprite size to match region
        size = {width, height};
    }
    
    /**
     * @brief Set texture region using normalized coordinates (0.0-1.0)
     * @param left Left UV coordinate
     * @param bottom Bottom UV coordinate
     * @param right Right UV coordinate
     * @param top Top UV coordinate
     */
    void SetTextureRegionNormalized(float left, float bottom, float right, float top) {
        texture_coords[0] = {left, bottom};   // Bottom-left
        texture_coords[1] = {right, bottom};  // Bottom-right
        texture_coords[2] = {right, top};     // Top-right
        texture_coords[3] = {left, top};      // Top-left
    }
};

/**
 * @brief 2D sprite renderer
 * 
 * Handles individual sprite rendering operations with support for
 * transformation, texturing, and color tinting.
 */
class SpriteRenderer {
public:
    /**
     * @brief Constructor
     */
    SpriteRenderer();
    
    /**
     * @brief Destructor
     */
    ~SpriteRenderer();
    
    // Non-copyable but movable
    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;
    SpriteRenderer(SpriteRenderer&&) = default;
    SpriteRenderer& operator=(SpriteRenderer&&) = default;
    
    /**
     * @brief Initialize the sprite renderer
     * @return true if successful, false otherwise
     */
    bool Initialize();

    
    /**
     * @brief Shutdown and cleanup resources
     */
    void Shutdown();
    
    /**
     * @brief Render a single sprite
     * @param sprite Sprite to render
     */
    void RenderSprite(const Sprite& sprite);
    
    /**
     * @brief Set the projection matrix for rendering
     * @param projection Projection matrix (when matrices are implemented)
     */
    // void SetProjectionMatrix(const Matrix4f& projection);
    
    /**
     * @brief Set the view matrix for rendering
     * @param view View matrix (when matrices are implemented)
     */
    // void SetViewMatrix(const Matrix4f& view);
    
    /**
     * @brief Check if renderer is initialized
     */
    bool IsInitialized() const { return initialized_; }

private:
    /**
     * @brief Setup vertex data for sprite quad
     */
    void SetupQuadData();
    
    /**
     * @brief Generate vertices for a sprite
     * @param sprite Sprite to generate vertices for
     * @param vertices Output vertex array (12 floats: 4 vertices * 3 components)
     */
    void GenerateVertices(const Sprite& sprite, float* vertices);
    
    /**
     * @brief Generate texture coordinates for a sprite
     * @param sprite Sprite to generate UVs for
     * @param uvs Output UV array (8 floats: 4 vertices * 2 components)
     */
    void GenerateTextureCoords(const Sprite& sprite, float* uvs);
    
    bool initialized_ = false;
    
    // OpenGL objects
    unsigned int quad_vao_ = 0;    ///< Vertex Array Object
    unsigned int quad_vbo_ = 0;    ///< Vertex Buffer Object (positions)
    unsigned int quad_uv_vbo_ = 0; ///< UV Buffer Object
    unsigned int quad_ebo_ = 0;    ///< Element Buffer Object
    
    // Default shader for sprite rendering
    std::shared_ptr<Shader> default_shader_ = nullptr;
};

} // namespace Renderer
} // namespace PyNovaGE
