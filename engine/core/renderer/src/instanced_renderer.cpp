#include "renderer/instanced_renderer.hpp"
#include <algorithm>
#include <iostream>
#include <chrono>

namespace PyNovaGE {
namespace Renderer {

InstancedRenderer::InstancedRenderer(const Config& config)
    : config_(config)
    , stats_{}
{
}

InstancedRenderer::~InstancedRenderer() {
    Cleanup();
}

bool InstancedRenderer::Initialize() {
    std::cout << "Initializing InstancedRenderer..." << std::endl;
    
    // Create default shader for instanced rendering
    CreateDefaultShader();
    
    if (!default_shader_ || !default_shader_->IsValid()) {
        std::cerr << "Failed to create default instanced shader!" << std::endl;
        return false;
    }
    
    std::cout << "InstancedRenderer initialized successfully" << std::endl;
    return true;
}

void InstancedRenderer::Cleanup() {
    // Clean up all batches
    for (auto& [id, batch] : batches_) {
        if (batch->vertex_array_object) {
            glDeleteVertexArrays(1, &batch->vertex_array_object);
        }
        if (batch->vertex_buffer) {
            glDeleteBuffers(1, &batch->vertex_buffer);
        }
        if (batch->instance_buffer) {
            glDeleteBuffers(1, &batch->instance_buffer);
        }
        if (batch->index_buffer) {
            glDeleteBuffers(1, &batch->index_buffer);
        }
    }
    batches_.clear();
    default_shader_.reset();
}

void InstancedRenderer::RegisterMeshType(const std::string& batch_id,
                                       const std::vector<float>& vertices,
                                       const std::vector<uint32_t>& indices,
                                       std::shared_ptr<Texture> texture,
                                       std::shared_ptr<Shader> shader) {
    // Safety check
    if (vertices.empty() || indices.empty()) {
        std::cerr << "ERROR: Empty vertices or indices!" << std::endl;
        return;
    }
    
    if (vertices.size() % 8 != 0) {
        std::cerr << "ERROR: Vertex count not divisible by 8 (expected pos3+norm3+uv2)" << std::endl;
        return;
    }
    
    auto batch = std::make_unique<InstanceBatch>();
    
    batch->vertex_count = vertices.size() / 8; // Assuming 8 floats per vertex (pos3 + normal3 + uv2)
    batch->index_count = indices.size();
    batch->texture = texture;
    batch->shader = shader ? shader : default_shader_;
    
    // Generate OpenGL objects
    glGenVertexArrays(1, &batch->vertex_array_object);
    glGenBuffers(1, &batch->vertex_buffer);
    glGenBuffers(1, &batch->instance_buffer);
    glGenBuffers(1, &batch->index_buffer);
    
    glBindVertexArray(batch->vertex_array_object);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, batch->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Set vertex attributes (position, normal, uv)
    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // UV (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    
    // Setup instance buffer (will be updated each frame)
    glBindBuffer(GL_ARRAY_BUFFER, batch->instance_buffer);
    glBufferData(GL_ARRAY_BUFFER, config_.max_instances_per_batch * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
    
    // Instance matrix (location = 3-6, takes 4 attribute slots)
    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 
                             (void*)(i * 4 * sizeof(float)));
        glEnableVertexAttribArray(3 + i);
        glVertexAttribDivisor(3 + i, 1); // Instance data
    }
    
    // Instance color (location = 7)
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 
                         (void*)(16 * sizeof(float))); // After 4x4 matrix
    glEnableVertexAttribArray(7);
    glVertexAttribDivisor(7, 1);
    
    // Instance custom data (location = 8)
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 
                         (void*)(20 * sizeof(float))); // After matrix + color
    glEnableVertexAttribArray(8);
    glVertexAttribDivisor(8, 1);
    
    glBindVertexArray(0);
    
    // Save values before move
    size_t vertex_count = batch->vertex_count;
    size_t index_count = batch->index_count;
    
    batches_[batch_id] = std::move(batch);
    
    std::cout << "Registered mesh type '" << batch_id << "' with " << vertex_count 
              << " vertices, " << index_count << " indices" << std::endl;
}

void InstancedRenderer::AddInstance(const std::string& batch_id,
                                  const PyNovaGE::Matrix4<float>& transform,
                                  const PyNovaGE::Vector4f& color,
                                  const PyNovaGE::Vector4f& custom_data) {
    auto it = batches_.find(batch_id);
    if (it == batches_.end()) {
        std::cerr << "Batch '" << batch_id << "' not found!" << std::endl;
        return;
    }
    
    auto& batch = it->second;
    if (batch->instances.size() >= config_.max_instances_per_batch) {
        return; // Batch full
    }
    
    InstanceData instance;
    instance.transform = transform.transpose(); // Transpose from row-major to column-major for OpenGL
    instance.color = color;
    instance.custom_data = custom_data;
    
    batch->instances.push_back(instance);
    batch->need_upload = true;
}

void InstancedRenderer::ClearInstances() {
    for (auto& [id, batch] : batches_) {
        batch->instances.clear();
        batch->need_upload = true;
    }
    
    // Reset stats
    stats_ = Stats{};
}

void InstancedRenderer::Update(const PyNovaGE::Matrix4<float>& view_matrix,
                             const PyNovaGE::Matrix4<float>& projection_matrix,
                             const PyNovaGE::Vector3f& camera_pos) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Extract frustum planes for culling
    PyNovaGE::Matrix4<float> view_proj = projection_matrix * view_matrix;
    if (config_.enable_frustum_culling) {
        ExtractFrustumPlanes(view_proj);
    }
    
    stats_.total_instances = 0;
    stats_.culled_instances = 0;
    stats_.rendered_instances = 0;
    
    // Process each batch
    for (auto& [id, batch] : batches_) {
        stats_.total_instances += batch->instances.size();
        
        if (batch->instances.empty()) continue;
        
        // Perform frustum culling and LOD selection
        std::vector<InstanceData> visible_instances;
        visible_instances.reserve(batch->instances.size());
        
        for (const auto& instance : batch->instances) {
            // Extract position from transform matrix (column-major after transpose: translation is in indices 12,13,14)
            PyNovaGE::Vector3f position(instance.transform.data[12], 
                                      instance.transform.data[13], 
                                      instance.transform.data[14]);
            
            // Frustum culling
            if (config_.enable_frustum_culling && !IsInstanceVisible(instance, 2.0f)) {
                stats_.culled_instances++;
                continue;
            }
            
            // LOD based on distance
            if (config_.enable_lod) {
                float distance = CalculateDistance(position, camera_pos);
                if (distance > config_.lod_distance_3) {
                    stats_.culled_instances++; // Cull very distant objects
                    continue;
                }
            }
            
            visible_instances.push_back(instance);
        }
        
        // Sort by distance if enabled
        if (config_.sort_by_distance && !visible_instances.empty()) {
            SortInstancesByDistance(visible_instances, camera_pos);
        }
        
        // Update batch instances
        batch->instances = std::move(visible_instances);
        batch->need_upload = true;
        
        stats_.rendered_instances += batch->instances.size();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    stats_.update_time_ms = std::chrono::duration<float, std::milli>(end_time - start_time).count();
}

void InstancedRenderer::Render(const PyNovaGE::Matrix4<float>& view_matrix,
                             const PyNovaGE::Matrix4<float>& projection_matrix) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    stats_.draw_calls = 0;
    stats_.triangles = 0;
    
    for (auto& [id, batch] : batches_) {
        if (batch->instances.empty()) continue;
        
        // Upload instance data if needed
        if (batch->need_upload) {
            UploadInstanceData(*batch);
            batch->need_upload = false;
        }
        
        // Bind shader
        if (batch->shader) {
            batch->shader->Bind();
            
            // Set uniforms
            batch->shader->SetMatrix4f("u_view", view_matrix);
            batch->shader->SetMatrix4f("u_projection", projection_matrix);
        }
        
        // Bind texture if available
        if (batch->texture) {
            // batch->texture->Bind(0);
            // batch->shader->SetInt("u_texture", 0);
        }
        
        // Render instances
        glBindVertexArray(batch->vertex_array_object);
        
        glDrawElementsInstanced(GL_TRIANGLES, 
                               static_cast<GLsizei>(batch->index_count),
                               GL_UNSIGNED_INT, 
                               0, 
                               static_cast<GLsizei>(batch->instances.size()));
        
        stats_.draw_calls++;
        stats_.triangles += (batch->index_count / 3) * batch->instances.size();
        
        glBindVertexArray(0);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    stats_.render_time_ms = std::chrono::duration<float, std::milli>(end_time - start_time).count();
}

void InstancedRenderer::ExtractFrustumPlanes(const PyNovaGE::Matrix4<float>& view_proj_matrix) {
    // Extract frustum planes from combined view-projection matrix
    // Our matrices are row-major, so we need to extract from the transposed matrix
    // for the standard frustum extraction algorithm to work correctly
    auto transposed = view_proj_matrix.transpose();
    const float* m = transposed.data.data();
    
    // Left plane
    frustum_.planes[0] = PyNovaGE::Vector4f(m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]);
    // Right plane  
    frustum_.planes[1] = PyNovaGE::Vector4f(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]);
    // Bottom plane
    frustum_.planes[2] = PyNovaGE::Vector4f(m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]);
    // Top plane
    frustum_.planes[3] = PyNovaGE::Vector4f(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]);
    // Near plane (swapped with far)
    frustum_.planes[4] = PyNovaGE::Vector4f(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]);
    // Far plane (swapped with near)
    frustum_.planes[5] = PyNovaGE::Vector4f(m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]);
    
    // Normalize planes
    for (int i = 0; i < 6; i++) {
        float length = sqrt(frustum_.planes[i].x * frustum_.planes[i].x +
                           frustum_.planes[i].y * frustum_.planes[i].y +
                           frustum_.planes[i].z * frustum_.planes[i].z);
        if (length > 0.0f) {
            frustum_.planes[i].x /= length;
            frustum_.planes[i].y /= length;
            frustum_.planes[i].z /= length;
            frustum_.planes[i].w /= length;
        }
    }
}

bool InstancedRenderer::IsInstanceVisible(const InstanceData& instance, float radius) const {
    // Extract position from transform matrix (column-major after transpose: translation is in indices 12,13,14)
    PyNovaGE::Vector3f position(instance.transform.data[12], 
                              instance.transform.data[13], 
                              instance.transform.data[14]);
    
    // Test against all frustum planes
    for (int i = 0; i < 6; i++) {
        const auto& plane = frustum_.planes[i];
        float distance = plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
        if (distance < -radius) {
            return false; // Outside frustum
        }
    }
    
    return true;
}

float InstancedRenderer::CalculateDistance(const PyNovaGE::Vector3f& position, 
                                         const PyNovaGE::Vector3f& camera_pos) const {
    PyNovaGE::Vector3f diff = position - camera_pos;
    return diff.length();
}

void InstancedRenderer::UploadInstanceData(InstanceBatch& batch) {
    if (batch.instances.empty()) return;
    
    glBindBuffer(GL_ARRAY_BUFFER, batch.instance_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                   batch.instances.size() * sizeof(InstanceData), 
                   batch.instances.data());
}

void InstancedRenderer::CreateDefaultShader() {
    // Simple instanced shader
    std::string vertex_source = R"(
#version 330 core

// Per-vertex attributes
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

// Per-instance attributes
layout(location = 3) in mat4 a_instance_transform;
layout(location = 7) in vec4 a_instance_color;
layout(location = 8) in vec4 a_instance_custom;

// Uniforms
uniform mat4 u_view = mat4(1.0);
uniform mat4 u_projection = mat4(1.0);

// Outputs
out vec3 v_normal;
out vec2 v_uv;
out vec4 v_color;

void main() {
    vec4 world_pos = a_instance_transform * vec4(a_position, 1.0);
    gl_Position = u_projection * u_view * world_pos;
    
    v_normal = normalize(mat3(a_instance_transform) * a_normal);
    v_uv = a_uv;
    v_color = a_instance_color;
}
)";

    std::string fragment_source = R"(
#version 330 core

// Inputs
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;

// Outputs
out vec4 fragment_color;

void main() {
    // Simple lighting
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.3));
    float ndl = max(dot(v_normal, light_dir), 0.1);
    
    fragment_color = v_color * ndl;
}
)";

    default_shader_ = std::make_shared<Shader>(vertex_source, fragment_source);
}

void InstancedRenderer::SortInstancesByDistance(std::vector<InstanceData>& instances, 
                                              const PyNovaGE::Vector3f& camera_pos) {
    std::sort(instances.begin(), instances.end(), 
        [&camera_pos, this](const InstanceData& a, const InstanceData& b) {
            PyNovaGE::Vector3f pos_a(a.transform.data[12], a.transform.data[13], a.transform.data[14]);
            PyNovaGE::Vector3f pos_b(b.transform.data[12], b.transform.data[13], b.transform.data[14]);
            
            float dist_a = CalculateDistance(pos_a, camera_pos);
            float dist_b = CalculateDistance(pos_b, camera_pos);
            
            return dist_a < dist_b; // Sort near to far
        });
}

} // namespace Renderer
} // namespace PyNovaGE