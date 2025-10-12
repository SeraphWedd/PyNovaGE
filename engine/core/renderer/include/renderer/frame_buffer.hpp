#pragma once
#include <cstdint>
typedef unsigned int GLuint;

namespace PyNovaGE {
namespace Renderer {

class FrameBuffer {
public:
    FrameBuffer(int width, int height);
    ~FrameBuffer();

    void Bind();
    void Unbind();
    void Resize(int width, int height);
    GLuint GetTextureHandle() const { return color_texture_; }
    
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

private:
    void Initialize();
    void Cleanup();

    GLuint fbo_;
    GLuint color_texture_;
    GLuint depth_rbo_;
    int width_;
    int height_;
bool initialized_;
};

} // namespace Renderer
} // namespace PyNovaGE
