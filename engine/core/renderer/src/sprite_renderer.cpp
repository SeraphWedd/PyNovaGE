#include "renderer/sprite_renderer.hpp"
#include <glad/gl.h>
#include "renderer/renderer.hpp"
#include <iostream>
#include <cmath>

#include <glad/gl.h>

namespace PyNovaGE {
namespace Renderer {

// Default sprite shader source code
static const char* default_vertex_shader_source = R"(
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;

out vec2 v_texCoord;
out vec4 v_color;

uniform vec4 u_color;
uniform mat4 u_transform;

void main() {
    v_texCoord = a_texCoord;
    v_color = u_color;
    
    // Position is already in NDC space after projection scale is applied
    gl_Position = vec4(a_position, 1.0);
}
)";

static const char* default_fragment_shader_source = R"(
#version 330 core

in vec2 v_texCoord;
in vec4 v_color;

out vec4 fragColor;

uniform sampler2D u_texture;
uniform bool u_hasTexture;

void main() {
    if (u_hasTexture) {
        fragColor = texture(u_texture, v_texCoord) * v_color;
    } else {
        fragColor = v_color;
    }
}
)";

SpriteRenderer::SpriteRenderer() {
    // Constructor - initialization is done in Initialize()
}

SpriteRenderer::~SpriteRenderer() {
    Shutdown();
}

bool SpriteRenderer::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // Create default shader
    std::cout << "Creating default shader..." << std::endl;
    default_shader_ = std::make_shared<Shader>();
    if (!default_shader_->LoadFromSource(default_vertex_shader_source, default_fragment_shader_source)) {
        std::cerr << "Failed to create default sprite shader: " << default_shader_->GetErrorLog() << std::endl;
        return false;
    }
    std::cout << "Default shader created successfully" << std::endl;
    
    // Setup quad geometry
    std::cout << "Setting up quad data..." << std::endl;
    SetupQuadData();
    std::cout << "Quad data setup complete" << std::endl;

    initialized_ = true;
    std::cout << "SpriteRenderer initialized successfully" << std::endl;
    return true;
}

void SpriteRenderer::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // Cleanup OpenGL objects
    if (quad_vao_ != 0) {
        glDeleteVertexArrays(1, &quad_vao_);
        quad_vao_ = 0;
    }
    
    if (quad_vbo_ != 0) {
        glDeleteBuffers(1, &quad_vbo_);
        quad_vbo_ = 0;
    }
    
    if (quad_uv_vbo_ != 0) {
        glDeleteBuffers(1, &quad_uv_vbo_);
        quad_uv_vbo_ = 0;
    }
    
    if (quad_ebo_ != 0) {
        glDeleteBuffers(1, &quad_ebo_);
        quad_ebo_ = 0;
    }
    
    default_shader_.reset();
    initialized_ = false;
    
    std::cout << "SpriteRenderer shut down" << std::endl;
}

void SpriteRenderer::RenderSprite(const Sprite& sprite) {
    if (!initialized_) {
        std::cerr << "SpriteRenderer not initialized!" << std::endl;
        return;
    }
    
    std::cout << "Rendering sprite at position: (" << sprite.position.x << ", " << sprite.position.y << ")" << std::endl;
    std::cout << "Sprite size: (" << sprite.size.x << ", " << sprite.size.y << ")" << std::endl;
    std::cout << "Sprite color: (" << sprite.color.x << ", " << sprite.color.y << ", " << sprite.color.z << ", " << sprite.color.w << ")" << std::endl;
    
    // It's okay if there's no texture; we'll render a solid color quad
    
    // Bind shader
    default_shader_->Bind();
    
    // Set uniforms
    default_shader_->SetVector4f("u_color", sprite.color);
    default_shader_->SetInt("u_texture", 0);
    default_shader_->SetInt("u_hasTexture", sprite.texture != nullptr ? 1 : 0);
    
    // Bind texture
    if (sprite.texture) {
        sprite.texture->Bind(0);
    }
    
    // Generate vertex data for this sprite
    float vertices[12]; // 4 vertices * 3 components (x, y, z)
    GenerateVertices(sprite, vertices);
    
    float uvs[8]; // 4 vertices * 2 components (u, v)
    GenerateTextureCoords(sprite, uvs);
    
    // Update vertex buffer with new data
    glBindVertexArray(quad_vao_);
    
    // Update position buffer
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    // Update UV buffer
    glBindBuffer(GL_ARRAY_BUFFER, quad_uv_vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(uvs), uvs);
    
    // Draw the quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // Unbind
    glBindVertexArray(0);
    Shader::Unbind();
    
    if (sprite.texture) {
        Texture::Unbind(0);
    }
    
    // Check for OpenGL errors
    Renderer::CheckGLError("SpriteRenderer::RenderSprite");
}

void SpriteRenderer::SetupQuadData() {
    // Create vertex array
    glGenVertexArrays(1, &quad_vao_);
    glBindVertexArray(quad_vao_);
    
    // Create vertex buffer (positions)
    glGenBuffers(1, &quad_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Create UV buffer
    glGenBuffers(1, &quad_uv_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_uv_vbo_);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // UV attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    
    // Create element buffer
    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };
    
    glGenBuffers(1, &quad_ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    std::cout << "Quad data setup complete" << std::endl;
}

void SpriteRenderer::GenerateVertices(const Sprite& sprite, float* vertices) {
    // Calculate half size for centering
    float half_width = sprite.size.x * 0.5f;
    float half_height = sprite.size.y * 0.5f;
    
    // Calculate origin offset
    float origin_offset_x = (sprite.origin.x - 0.5f) * sprite.size.x;
    float origin_offset_y = (sprite.origin.y - 0.5f) * sprite.size.y;
    
    // Basic quad vertices (before transformation)
    Vector2f quad_vertices[4] = {
        {-half_width - origin_offset_x, -half_height - origin_offset_y}, // Bottom-left
        { half_width - origin_offset_x, -half_height - origin_offset_y}, // Bottom-right
        { half_width - origin_offset_x,  half_height - origin_offset_y}, // Top-right
        {-half_width - origin_offset_x,  half_height - origin_offset_y}  // Top-left
    };
    
    // Apply rotation if needed
    if (sprite.rotation != 0.0f) {
        float cos_r = std::cos(sprite.rotation);
        float sin_r = std::sin(sprite.rotation);
        
        for (int i = 0; i < 4; ++i) {
            float x = quad_vertices[i].x;
            float y = quad_vertices[i].y;
            
            quad_vertices[i].x = x * cos_r - y * sin_r;
            quad_vertices[i].y = x * sin_r + y * cos_r;
        }
    }
    
    // Apply scale and position
    for (int i = 0; i < 4; ++i) {
        // Apply scale
        quad_vertices[i].x *= sprite.scale.x;
        quad_vertices[i].y *= sprite.scale.y;
        
        // Apply position
        quad_vertices[i].x += sprite.position.x;
        quad_vertices[i].y += sprite.position.y;
        
        // Get the current projection scale
        const Vector2f& proj_scale = Renderer::GetProjectionScale();

        // Convert from world coordinates to NDC using projection scale
        // Project to NDC space [-1, 1] with proper aspect ratio correction
        vertices[i * 3 + 0] = quad_vertices[i].x * proj_scale.x;  // X: [-1, 1]
        vertices[i * 3 + 1] = quad_vertices[i].y * proj_scale.y;  // Y: [-1, 1]
        vertices[i * 3 + 2] = 0.0f; // Z
    }
}

void SpriteRenderer::GenerateTextureCoords(const Sprite& sprite, float* uvs) {
    for (int i = 0; i < 4; ++i) {
        uvs[i * 2 + 0] = sprite.texture_coords[i].x;
        uvs[i * 2 + 1] = sprite.texture_coords[i].y;
    }
}

} // namespace Renderer
} // namespace PyNovaGE
