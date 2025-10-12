#pragma once
#include <memory>
typedef unsigned int GLuint;

namespace PyNovaGE {
namespace Renderer {

class Shader;

class ScreenQuad {
public:
    ScreenQuad();
    ~ScreenQuad();

    void Initialize();
    void Render(GLuint texture_handle);

private:
    void Cleanup();

    GLuint vao_;
    GLuint vbo_;
    std::shared_ptr<PyNovaGE::Renderer::Shader> shader_;
bool initialized_;
};

} // namespace Renderer
} // namespace PyNovaGE
