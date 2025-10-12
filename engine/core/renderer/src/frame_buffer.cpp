#include "renderer/frame_buffer.hpp"
#include <glad/gl.h>
#include <iostream>

namespace PyNovaGE {
namespace Renderer {

FrameBuffer::FrameBuffer(int width, int height)
    : width_(width)
    , height_(height)
    , fbo_(0)
    , color_texture_(0)
    , depth_rbo_(0)
    , initialized_(false)
{
    Initialize();
}

FrameBuffer::~FrameBuffer() {
    Cleanup();
}

void FrameBuffer::Initialize() {
    if (initialized_) {
        return;
    }

    // Create framebuffer
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    // Create color attachment texture
    glGenTextures(1, &color_texture_);
    glBindTexture(GL_TEXTURE_2D, color_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_, 0);

    // Create renderbuffer for depth
    glGenRenderbuffers(1, &depth_rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_rbo_);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        Cleanup();
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    initialized_ = true;
}

void FrameBuffer::Cleanup() {
    if (!initialized_) {
        return;
    }

    if (depth_rbo_ != 0) {
        glDeleteRenderbuffers(1, &depth_rbo_);
        depth_rbo_ = 0;
    }

    if (color_texture_ != 0) {
        glDeleteTextures(1, &color_texture_);
        color_texture_ = 0;
    }

    if (fbo_ != 0) {
        glDeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
    }

    initialized_ = false;
}

void FrameBuffer::Bind() {
    if (!initialized_) {
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, width_, height_);
}

void FrameBuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Resize(int width, int height) {
    if (width_ == width && height_ == height) {
        return;
    }

    width_ = width;
    height_ = height;

    Cleanup();
    Initialize();
}
} // namespace Renderer
} // namespace PyNovaGE
