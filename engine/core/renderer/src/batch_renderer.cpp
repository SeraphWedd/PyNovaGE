#include "renderer/batch_renderer.hpp"
#include "renderer/renderer.hpp"
#include "renderer/texture.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

#include <glad/gl.h>

namespace PyNovaGE {
namespace Renderer {

// Batch rendering shader source
static const char* batch_vertex_shader_source = R"(
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoords;
layout(location = 2) in vec4 a_color;
layout(location = 3) in float a_textureIndex;

out vec2 v_texCoords;
out vec4 v_color;
out float v_textureIndex;

void main() {
    v_texCoords = a_texCoords;
    v_color = a_color;
    v_textureIndex = a_textureIndex;
    
    // Simple orthographic projection for now
    // Will be improved when matrices are fully implemented
    gl_Position = vec4(a_position, 1.0);
}
)";

static const char* batch_fragment_shader_source = R"(
#version 330 core

in vec2 v_texCoords;
in vec4 v_color;
in float v_textureIndex;

out vec4 fragColor;

uniform sampler2D u_textures[16];

void main() {
    int texIndex = int(v_textureIndex);
    vec4 texColor = vec4(1.0); // Default white
    
    // Sample from the appropriate texture
    if (texIndex == 0) texColor = texture(u_textures[0], v_texCoords);
    else if (texIndex == 1) texColor = texture(u_textures[1], v_texCoords);
    else if (texIndex == 2) texColor = texture(u_textures[2], v_texCoords);
    else if (texIndex == 3) texColor = texture(u_textures[3], v_texCoords);
    else if (texIndex == 4) texColor = texture(u_textures[4], v_texCoords);
    else if (texIndex == 5) texColor = texture(u_textures[5], v_texCoords);
    else if (texIndex == 6) texColor = texture(u_textures[6], v_texCoords);
    else if (texIndex == 7) texColor = texture(u_textures[7], v_texCoords);
    else if (texIndex == 8) texColor = texture(u_textures[8], v_texCoords);
    else if (texIndex == 9) texColor = texture(u_textures[9], v_texCoords);
    else if (texIndex == 10) texColor = texture(u_textures[10], v_texCoords);
    else if (texIndex == 11) texColor = texture(u_textures[11], v_texCoords);
    else if (texIndex == 12) texColor = texture(u_textures[12], v_texCoords);
    else if (texIndex == 13) texColor = texture(u_textures[13], v_texCoords);
    else if (texIndex == 14) texColor = texture(u_textures[14], v_texCoords);
    else if (texIndex == 15) texColor = texture(u_textures[15], v_texCoords);
    
    fragColor = texColor * v_color;
}
)";

BatchRenderer::BatchRenderer(int max_sprites, int max_textures) 
    : max_sprites_(max_sprites), max_textures_(max_textures) {
    // Reserve space for vertices and indices
    vertices_.reserve(max_sprites_ * 4); // 4 vertices per sprite
    indices_.reserve(max_sprites_ * 6);  // 6 indices per sprite (2 triangles)
    batch_textures_.reserve(max_textures_);
    
    std::cout << "BatchRenderer created - Max sprites: " << max_sprites_ 
              << ", Max textures: " << max_textures_ << std::endl;
}

BatchRenderer::~BatchRenderer() {
    Shutdown();
}

bool BatchRenderer::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // Create batch shader
    batch_shader_ = std::make_shared<Shader>();
    if (!batch_shader_->LoadFromSource(batch_vertex_shader_source, batch_fragment_shader_source)) {
        std::cerr << "Failed to create batch shader: " << batch_shader_->GetErrorLog() << std::endl;
        return false;
    }
    
    // Setup vertex buffer
    SetupVertexBuffer();
    
    // Setup texture uniform array
    batch_shader_->Bind();
    for (int i = 0; i < max_textures_; ++i) {
        batch_shader_->SetInt("u_textures[" + std::to_string(i) + "]", i);
    }
    Shader::Unbind();
    
    initialized_ = true;
    std::cout << "BatchRenderer initialized successfully" << std::endl;
    return true;
}

void BatchRenderer::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // Cleanup OpenGL objects
    if (batch_vao_ != 0) {
        glDeleteVertexArrays(1, &batch_vao_);
        batch_vao_ = 0;
    }
    
    if (batch_vbo_ != 0) {
        glDeleteBuffers(1, &batch_vbo_);
        batch_vbo_ = 0;
    }
    
    if (batch_ebo_ != 0) {
        glDeleteBuffers(1, &batch_ebo_);
        batch_ebo_ = 0;
    }
    
    batch_shader_.reset();
    initialized_ = false;
    
    std::cout << "BatchRenderer shut down" << std::endl;
}

void BatchRenderer::BeginBatch() {
    if (!initialized_) {
        std::cerr << "BatchRenderer not initialized!" << std::endl;
        return;
    }
    
    if (batch_started_) {
        std::cerr << "Batch already started! Call EndBatch() first." << std::endl;
        return;
    }
    
    ResetBatch();
    batch_started_ = true;
}

bool BatchRenderer::AddSprite(const Sprite& sprite) {
    if (!batch_started_) {
        std::cerr << "Batch not started! Call BeginBatch() first." << std::endl;
        return false;
    }
    
    if (!sprite.texture) {
        std::cerr << "Sprite has no texture!" << std::endl;
        return false;
    }
    
    // Check if batch is full
    if (current_sprite_count_ >= static_cast<size_t>(max_sprites_)) {
        return false; // Batch is full
    }
    
    // Find or add texture to batch
    int texture_index = FindOrAddTexture(sprite.texture);
    if (texture_index == -1) {
        return false; // No more texture slots available
    }
    
    // Add sprite vertices to batch
    AddSpriteVertices(sprite, static_cast<float>(texture_index));
    
    current_sprite_count_++;
    stats_.sprites_batched++;
    
    return true;
}

void BatchRenderer::FlushBatch() {
    if (!batch_started_ || current_sprite_count_ == 0) {
        return; // Nothing to flush
    }
    
    // Bind shader
    batch_shader_->Bind();
    
    // Bind all textures
    for (size_t i = 0; i < batch_textures_.size(); ++i) {
        batch_textures_[i]->Bind(static_cast<unsigned int>(i));
    }
    stats_.texture_binds += batch_textures_.size();
    
    // Update vertex buffer
    glBindVertexArray(batch_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, batch_vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_.size() * sizeof(BatchVertex), vertices_.data());
    
    // Update index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch_ebo_);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices_.size() * sizeof(unsigned int), indices_.data());
    
    // Draw batch
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, 0);
    
    // Unbind resources
    glBindVertexArray(0);
    Shader::Unbind();
    
    // Unbind textures
    for (size_t i = 0; i < batch_textures_.size(); ++i) {
        Texture::Unbind(static_cast<unsigned int>(i));
    }
    
    // Update statistics
    stats_.draw_calls++;
    stats_.batches_flushed++;
    stats_.UpdateAverage();
    
    // Check for OpenGL errors
    Renderer::CheckGLError("BatchRenderer::FlushBatch");
    
    // Reset for next batch
    ResetBatch();
}

void BatchRenderer::EndBatch() {
    if (!batch_started_) {
        return;
    }
    
    // Flush any remaining sprites
    FlushBatch();
    
    batch_started_ = false;
}

void BatchRenderer::RenderSprites(const Sprite* sprites, size_t count) {
    if (!sprites || count == 0) {
        return;
    }
    
    BeginBatch();
    
    for (size_t i = 0; i < count; ++i) {
        if (!AddSprite(sprites[i])) {
            // Batch is full, flush and start new batch
            FlushBatch();
            if (!AddSprite(sprites[i])) {
                std::cerr << "Failed to add sprite to batch even after flush!" << std::endl;
                break;
            }
        }
    }
    
    EndBatch();
}

void BatchRenderer::RenderSprites(const std::vector<Sprite>& sprites) {
    if (sprites.empty()) {
        return;
    }
    
    RenderSprites(sprites.data(), sprites.size());
}

void BatchRenderer::SetupVertexBuffer() {
    // Generate vertex array
    glGenVertexArrays(1, &batch_vao_);
    glBindVertexArray(batch_vao_);
    
    // Generate and setup vertex buffer
    glGenBuffers(1, &batch_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, batch_vbo_);
    glBufferData(GL_ARRAY_BUFFER, max_sprites_ * 4 * sizeof(BatchVertex), nullptr, GL_DYNAMIC_DRAW);
    
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), 
                         reinterpret_cast<void*>(offsetof(BatchVertex, position)));
    glEnableVertexAttribArray(0);
    
    // Texture coordinates attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), 
                         reinterpret_cast<void*>(offsetof(BatchVertex, texCoords)));
    glEnableVertexAttribArray(1);
    
    // Color attribute (location = 2)
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), 
                         reinterpret_cast<void*>(offsetof(BatchVertex, color)));
    glEnableVertexAttribArray(2);
    
    // Texture index attribute (location = 3)
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), 
                         reinterpret_cast<void*>(offsetof(BatchVertex, textureIndex)));
    glEnableVertexAttribArray(3);
    
    // Generate and setup element buffer
    glGenBuffers(1, &batch_ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch_ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_sprites_ * 6 * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
    
    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    std::cout << "Batch vertex buffer setup complete" << std::endl;
}

void BatchRenderer::AddSpriteVertices(const Sprite& sprite, float texture_index) {
    // Generate transformed vertices
    std::array<Vector3f, 4> positions;
    GenerateTransformedVertices(sprite, positions);
    
    // Base vertex index for this sprite
    unsigned int base_index = static_cast<unsigned int>(vertices_.size());
    
    // Add vertices
    vertices_.emplace_back(positions[0], sprite.texture_coords[0], sprite.color, texture_index); // Bottom-left
    vertices_.emplace_back(positions[1], sprite.texture_coords[1], sprite.color, texture_index); // Bottom-right
    vertices_.emplace_back(positions[2], sprite.texture_coords[2], sprite.color, texture_index); // Top-right
    vertices_.emplace_back(positions[3], sprite.texture_coords[3], sprite.color, texture_index); // Top-left
    
    // Add indices for two triangles
    indices_.push_back(base_index + 0); // Bottom-left
    indices_.push_back(base_index + 1); // Bottom-right
    indices_.push_back(base_index + 2); // Top-right
    
    indices_.push_back(base_index + 2); // Top-right
    indices_.push_back(base_index + 3); // Top-left
    indices_.push_back(base_index + 0); // Bottom-left
}

int BatchRenderer::FindOrAddTexture(std::shared_ptr<Texture> texture) {
    // Look for existing texture
    for (size_t i = 0; i < batch_textures_.size(); ++i) {
        if (batch_textures_[i] == texture) {
            return static_cast<int>(i);
        }
    }
    
    // Check if we have room for a new texture
    if (batch_textures_.size() >= static_cast<size_t>(max_textures_)) {
        return -1; // No more texture slots
    }
    
    // Add new texture
    batch_textures_.push_back(texture);
    current_texture_count_ = batch_textures_.size();
    
    return static_cast<int>(batch_textures_.size() - 1);
}

void BatchRenderer::ResetBatch() {
    vertices_.clear();
    indices_.clear();
    batch_textures_.clear();
    current_sprite_count_ = 0;
    current_texture_count_ = 0;
}

void BatchRenderer::GenerateTransformedVertices(const Sprite& sprite, std::array<Vector3f, 4>& vertices) {
    // Calculate half size for centering
    float half_width = sprite.size.x * 0.5f;
    float half_height = sprite.size.y * 0.5f;
    
    // Calculate origin offset
    float origin_offset_x = (sprite.origin.x - 0.5f) * sprite.size.x;
    float origin_offset_y = (sprite.origin.y - 0.5f) * sprite.size.y;
    
    // Basic quad vertices (before transformation)
    std::array<Vector2f, 4> local_vertices = {
        Vector2f{-half_width - origin_offset_x, -half_height - origin_offset_y}, // Bottom-left
        Vector2f{ half_width - origin_offset_x, -half_height - origin_offset_y}, // Bottom-right
        Vector2f{ half_width - origin_offset_x,  half_height - origin_offset_y}, // Top-right
        Vector2f{-half_width - origin_offset_x,  half_height - origin_offset_y}  // Top-left
    };
    
    // Apply rotation if needed
    if (sprite.rotation != 0.0f) {
        float cos_r = std::cos(sprite.rotation);
        float sin_r = std::sin(sprite.rotation);
        
        for (auto& vertex : local_vertices) {
            float x = vertex.x;
            float y = vertex.y;
            
            vertex.x = x * cos_r - y * sin_r;
            vertex.y = x * sin_r + y * cos_r;
        }
    }
    
    // Apply scale and position, convert to 3D
    for (size_t i = 0; i < 4; ++i) {
        // Apply scale
        local_vertices[i].x *= sprite.scale.x;
        local_vertices[i].y *= sprite.scale.y;
        
        // Apply position
        local_vertices[i].x += sprite.position.x;
        local_vertices[i].y += sprite.position.y;
        
        // Convert to normalized device coordinates (simple conversion for now)
        // This will be improved when proper matrices are implemented
        vertices[i] = Vector3f(
            (local_vertices[i].x / 400.0f) - 1.0f, // X: [-1, 1]
            (local_vertices[i].y / 300.0f) - 1.0f, // Y: [-1, 1]
            0.0f                                    // Z
        );
    }
}

} // namespace Renderer
} // namespace PyNovaGE
