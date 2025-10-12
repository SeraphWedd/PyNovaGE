#include "renderer/screen_quad.hpp"
#include "renderer/shader.hpp"
#include <glad/gl.h>
#include <iostream>

namespace PyNovaGE {
namespace Renderer {

static const char* screen_quad_vertex_shader = 
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_position;\n"
    "layout(location = 1) in vec2 a_texCoord;\n\n"
    "out vec2 v_texCoord;\n\n"
    "void main() {\n"
    "    v_texCoord = a_texCoord;\n"
    "    gl_Position = vec4(a_position, 1.0);\n"
    "}"
;

static const char* screen_quad_fragment_shader = 
    "#version 330 core\n"
    "in vec2 v_texCoord;\n"
    "out vec4 fragColor;\n\n"
    "uniform sampler2D u_texture;\n\n"
    "void main() {\n"
    "    fragColor = texture(u_texture, v_texCoord);\n"
    "}"
;

ScreenQuad::ScreenQuad()
    : vao_(0)
    , vbo_(0)
    , initialized_(false)
{
}

ScreenQuad::~ScreenQuad() {
    Cleanup();
}

void ScreenQuad::Initialize() {
    if (initialized_) {
        return;
    }

    // Create and compile shader
    shader_ = std::make_shared<PyNovaGE::Renderer::Shader>();
    if (!shader_->LoadFromSource(screen_quad_vertex_shader, screen_quad_fragment_shader)) {
        std::cerr << "Failed to create screen quad shader: " << shader_->GetErrorLog() << std::endl;
        return;
    }

    // Quad vertices (NDC coordinates with texture coords)
    float vertices[] = {
        // positions      // texture coords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,   // top left
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,   // bottom left
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,   // bottom right
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,   // top left
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,   // bottom right
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f    // top right
    };

    // Create buffers
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    initialized_ = true;
}

void ScreenQuad::Cleanup() {
    if (!initialized_) {
        return;
    }

    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }

    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }

    shader_.reset();
    initialized_ = false;
}

void ScreenQuad::Render(GLuint texture_handle) {
    if (!initialized_) {
        return;
    }

    shader_->Bind();
    shader_->SetInt("u_texture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    Shader::Unbind();
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Renderer
} // namespace PyNovaGE
