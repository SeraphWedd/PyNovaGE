#pragma once

#include <vectors/vector2.hpp>
#include <vectors/vector3.hpp>
#include <vectors/vector4.hpp>
#include <memory>
#include <string>
#include "frame_buffer.hpp"
#include "screen_quad.hpp"

namespace PyNovaGE {
namespace Renderer {

/**
 * @brief Renderer configuration options
 */
struct RendererConfig {
    bool enable_vsync = true;
    bool enable_depth_test = true;
    bool enable_blend = true;
    bool enable_culling = true;
    int max_sprites_per_batch = 1000;
    int max_textures_per_batch = 16;
};

/**
 * @brief Core rendering statistics
 */
struct RenderStats {
    size_t draw_calls = 0;
    size_t sprites_rendered = 0;
    size_t vertices_rendered = 0;
    size_t triangles_rendered = 0;
    size_t texture_binds = 0;
    size_t shader_binds = 0;
    double frame_time_ms = 0.0;
    
    void Reset() {
        draw_calls = 0;
        sprites_rendered = 0;
        vertices_rendered = 0;
        triangles_rendered = 0;
        texture_binds = 0;
        shader_binds = 0;
        frame_time_ms = 0.0;
    }
};

/**
 * @brief Rendering API abstraction (currently OpenGL only)
 */
enum class RenderAPI {
    OpenGL
};

// Forward declarations
class Shader;
class Texture;
class SpriteRenderer;
class BatchRenderer;
class FrameBuffer;
class ScreenQuad;

/**
 * @brief Core renderer class
 * 
 * Manages the OpenGL rendering context, state, and provides high-level
 * rendering functionality for 2D sprites and basic voxel rendering.
 */
class Renderer {
public:
    /**
     * @brief Initialize the renderer with given configuration
     * @param config Renderer configuration
     * @return true if successful, false otherwise
     */
    static bool Initialize(const RendererConfig& config = {});
    
    /**
     * @brief Shutdown the renderer and cleanup resources
     */
    static void Shutdown();
    
    /**
     * @brief Check if renderer is initialized
     */
    static bool IsInitialized();
    
    /**
     * @brief Get the current render API
     */
    static RenderAPI GetAPI() { return RenderAPI::OpenGL; }
    
    /**
     * @brief Begin a new frame
     * Clears buffers and resets statistics
     */
    static void BeginFrame();
    
    /**
     * @brief End the current frame
     * Finalizes rendering and updates statistics
     */
    static void EndFrame();
    
    /**
     * @brief Set the viewport size
     * @param x Viewport x offset
     * @param y Viewport y offset  
     * @param width Viewport width
     * @param height Viewport height
     */
    static void SetViewport(int x, int y, int width, int height);

    /**
     * @brief Set the projection scale for 2D rendering
     * @param scale Vector2f containing x and y scaling factors
     */
    static void SetProjectionScale(const Vector2f& scale);

    /**
     * @brief Get the current projection scale
     */
    static const Vector2f& GetProjectionScale();
    
    /**
     * @brief Clear the screen with specified color
     * @param color Clear color (RGBA)
     */
    static void Clear(const Vector4f& color = {0.0f, 0.0f, 0.0f, 1.0f});
    
    /**
     * @brief Set the clear color
     * @param color Clear color (RGBA)
     */
    static void SetClearColor(const Vector4f& color);
    
    /**
     * @brief Enable/disable depth testing
     * @param enabled Whether to enable depth testing
     */
    static void SetDepthTest(bool enabled);
    
    /**
     * @brief Enable/disable blending
     * @param enabled Whether to enable blending
     */
    static void SetBlending(bool enabled);
    
    /**
     * @brief Enable/disable face culling
     * @param enabled Whether to enable face culling
     */
    static void SetCulling(bool enabled);
    
    /**
     * @brief Set wireframe mode
     * @param enabled Whether to enable wireframe mode
     */
    static void SetWireframe(bool enabled);
    
    /**
     * @brief Get current rendering statistics
     */
    static const RenderStats& GetStats();
    
    /**
     * @brief Get the sprite renderer instance
     */
    static SpriteRenderer* GetSpriteRenderer();
    
    /**
     * @brief Get the batch renderer instance
     */
    static BatchRenderer* GetBatchRenderer();
    
    /**
     * @brief Get GPU and driver information
     */
    static std::string GetRendererInfo();
    
    /**
     * @brief Check for OpenGL errors and log them
     * @param operation Description of the operation for error context
     * @return true if no errors, false if errors were found
     */
    static bool CheckGLError(const std::string& operation = "");
    
    /**
     * @brief Read pixels from the framebuffer
     * @param x X coordinate to start reading from
     * @param y Y coordinate to start reading from
     * @param width Width of the region to read
     * @param height Height of the region to read
     * @param data Output buffer for pixel data (RGBA format)
     */
    static void ReadPixels(int x, int y, int width, int height, unsigned char* data);

private:
    Renderer() = delete;
    ~Renderer() = delete;
    
    static bool s_initialized_;
    static RendererConfig s_config_;
    static RenderStats s_stats_;
    static std::unique_ptr<SpriteRenderer> s_sprite_renderer_;
    static std::unique_ptr<BatchRenderer> s_batch_renderer_;
    static Vector2f s_projection_scale_;  // Default (1,1)
};

/**
 * @brief RAII renderer guard for automatic initialization/shutdown
 */
class RendererGuard {
public:
    explicit RendererGuard(const RendererConfig& config = {});
    ~RendererGuard();
    
    RendererGuard(const RendererGuard&) = delete;
    RendererGuard& operator=(const RendererGuard&) = delete;
    RendererGuard(RendererGuard&&) = delete;
    RendererGuard& operator=(RendererGuard&&) = delete;
    
    bool IsInitialized() const { return initialized_; }

private:
    bool initialized_ = false;
};

} // namespace Renderer
} // namespace PyNovaGE