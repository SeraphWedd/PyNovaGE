#pragma once

#include "renderer/texture.hpp"
#include <vectors/vector4.hpp>

namespace PyNovaGE {
namespace Renderer {

// Simple offscreen surface (FBO + texture) for 2D rendering
class Surface {
public:
    Surface() = default;
    ~Surface();

    // Non-copyable but movable
    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;
    Surface(Surface&& other) noexcept;
    Surface& operator=(Surface&& other) noexcept;

    // Create an offscreen surface of given size and format
    bool Create(int width, int height, TextureFormat format = TextureFormat::RGBA, bool with_depth = false);

    // Bind as current render target (sets viewport to surface size)
    void Bind();
    // Unbind (bind default framebuffer)
    static void Unbind();

    // Clear surface with color (also clears depth if present)
    void Clear(const Vector4f& color);

    // Blit to another target surface (or screen if target is null) using a textured quad in screen space
    // dstW/H define target viewport size in pixels
    void BlitTo(Surface* target, int dstW, int dstH);

    // Accessors
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    Texture& GetTexture() { return color_tex_; }
    const Texture& GetTexture() const { return color_tex_; }

private:
    void Cleanup();

    unsigned int fbo_ = 0;
    unsigned int depth_rbo_ = 0;
    Texture color_tex_;
    int width_ = 0;
    int height_ = 0;
    bool has_depth_ = false;
};

} // namespace Renderer
} // namespace PyNovaGE