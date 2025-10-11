#include "renderer/surface.hpp"
#include "renderer/renderer.hpp"
#include "renderer/batch_renderer.hpp"
#include <glad/gl.h>
#include <iostream>

namespace PyNovaGE {
namespace Renderer {

Surface::~Surface() {
    Cleanup();
}

Surface::Surface(Surface&& other) noexcept {
    *this = std::move(other);
}

Surface& Surface::operator=(Surface&& other) noexcept {
    if (this != &other) {
        Cleanup();
        fbo_ = other.fbo_;
        depth_rbo_ = other.depth_rbo_;
        color_tex_ = std::move(other.color_tex_);
        width_ = other.width_;
        height_ = other.height_;
        has_depth_ = other.has_depth_;
        other.fbo_ = 0;
        other.depth_rbo_ = 0;
        other.width_ = 0;
        other.height_ = 0;
        other.has_depth_ = false;
    }
    return *this;
}

bool Surface::Create(int width, int height, TextureFormat format, bool with_depth) {
    Cleanup();
    width_ = width;
    height_ = height;
    has_depth_ = with_depth;

    // Create color texture
    TextureConfig cfg;
    cfg.min_filter = TextureFilter::Linear;
    cfg.mag_filter = TextureFilter::Linear;
    cfg.wrap_s = TextureWrap::ClampToEdge;
    cfg.wrap_t = TextureWrap::ClampToEdge;
    cfg.generate_mipmaps = false;
    if (!color_tex_.CreateEmpty(width, height, format, cfg)) {
        std::cerr << "Surface: failed to create color texture" << std::endl;
        return false;
    }

    // Create FBO
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex_.GetTextureID(), 0);

    if (with_depth) {
        glGenRenderbuffers(1, &depth_rbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_rbo_);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Surface: FBO incomplete (status=" << status << ")" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        Cleanup();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void Surface::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    Renderer::SetViewport(0, 0, width_, height_);
}

void Surface::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Surface::Clear(const Vector4f& color) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    Renderer::Clear(color);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Surface::BlitTo(Surface* target, int dstW, int dstH) {
    // Bind destination
    if (target) {
        target->Bind();
        dstW = target->GetWidth();
        dstH = target->GetHeight();
    } else {
        Surface::Unbind();
        // dstW/dstH provided by caller should match current window framebuffer size
    }

    // Draw full-quad using BatchRenderer in screen pixel space (0..dstW, 0..dstH)
    auto* br = Renderer::GetBatchRenderer();
    if (!br) return;

    br->BeginBatch();
    // Fullscreen textured quad from this surface's color texture
    // Create non-owning shared_ptr to color_tex_
    std::shared_ptr<Texture> texPtr(&color_tex_, [](Texture*){});
    br->AddTexturedQuadScreen(0.0f, 0.0f, static_cast<float>(dstW), static_cast<float>(dstH), dstW, dstH, texPtr);
    br->EndBatch();

    if (target) {
        Surface::Unbind();
    }
}

void Surface::Cleanup() {
    if (depth_rbo_ != 0) {
        glDeleteRenderbuffers(1, &depth_rbo_);
        depth_rbo_ = 0;
    }
    if (fbo_ != 0) {
        glDeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
    }
    width_ = 0;
    height_ = 0;
    has_depth_ = false;
}

} // namespace Renderer
} // namespace PyNovaGE