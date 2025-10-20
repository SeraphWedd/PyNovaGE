#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <glad/gl.h>
#include "vectors/vector3.hpp"
#include "vectors/vector4.hpp"
#include "matrices/matrix4.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"

namespace PyNovaGE {
namespace Renderer {

/**
 * @brief Instance data for a single object
 */
struct InstanceData {
    PyNovaGE::Matrix4<float> transform;      // 4x4 transform matrix (64 bytes)
    PyNovaGE::Vector4f color;          // Color/tint (16 bytes)
    PyNovaGE::Vector4f custom_data;    // Custom per-instance data (16 bytes)
    
    // Total: 96 bytes per instance (GPU cache-friendly)
};

/**
 * @brief Batch of instances sharing the same mesh and material
 */
struct InstanceBatch {
    GLuint vertex_array_object;
    GLuint vertex_buffer;
    GLuint instance_buffer;
    GLuint index_buffer;
    
    size_t vertex_count;
    size_t index_count;
    
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Shader> shader;
    
    std::vector<InstanceData> instances;
    bool need_upload;                   // Flag to indicate buffer needs updating
    
    InstanceBatch() : vertex_array_object(0), vertex_buffer(0), 
                     instance_buffer(0), index_buffer(0),
                     vertex_count(0), index_count(0), need_upload(true) {}
};

/**
 * @brief High-performance instanced renderer for MMO scenarios
 * 
 * Optimized for:
 * - Hundreds of players with similar models
 * - Thousands of NPCs using shared meshes  
 * - Many pickable items (coins, potions, etc.)
 * - Environmental objects (trees, rocks, etc.)
 * 
 * Features:
 * - GPU instancing reduces draw calls from N to 1 per mesh type
 * - Frustum culling on CPU before GPU upload
 * - LOD system based on distance
 * - Batch sorting by material to minimize state changes
 */
class InstancedRenderer {
public:
    /**
     * @brief Configuration for instanced rendering
     */
    struct Config {
        size_t max_instances_per_batch = 10000;    // Max instances in single batch
        float frustum_culling_margin = 5.0f;       // Extra margin for culling
        bool enable_lod = true;                    // Enable level-of-detail
        float lod_distance_1 = 50.0f;              // Distance for LOD level 1
        float lod_distance_2 = 100.0f;             // Distance for LOD level 2
        float lod_distance_3 = 200.0f;             // Distance for LOD level 3
        bool enable_frustum_culling = true;        // Enable frustum culling
        bool sort_by_distance = true;              // Sort instances by distance
    };

    explicit InstancedRenderer(const Config& config = Config{});
    ~InstancedRenderer();

    /**
     * @brief Initialize the renderer
     */
    bool Initialize();

    /**
     * @brief Clean up resources
     */
    void Cleanup();

    /**
     * @brief Register a mesh type for instanced rendering
     * @param batch_id Unique identifier for this batch type
     * @param vertices Vertex data
     * @param indices Index data  
     * @param texture Texture to use
     * @param shader Shader to use (nullptr = use default)
     */
    void RegisterMeshType(const std::string& batch_id,
                         const std::vector<float>& vertices,
                         const std::vector<uint32_t>& indices,
                         std::shared_ptr<Texture> texture,
                         std::shared_ptr<Shader> shader = nullptr);

    /**
     * @brief Add instance to be rendered
     * @param batch_id Batch identifier
     * @param transform World transform matrix
     * @param color Color/tint
     * @param custom_data Custom per-instance data
     */
    void AddInstance(const std::string& batch_id,
                    const PyNovaGE::Matrix4<float>& transform,
                    const PyNovaGE::Vector4f& color = PyNovaGE::Vector4f(1.0f),
                    const PyNovaGE::Vector4f& custom_data = PyNovaGE::Vector4f(0.0f));

    /**
     * @brief Clear all instances (call each frame)
     */
    void ClearInstances();

    /**
     * @brief Update instance buffers and perform culling
     * @param view_matrix View matrix for frustum culling
     * @param projection_matrix Projection matrix for frustum culling
     * @param camera_pos Camera position for LOD calculations
     */
    void Update(const PyNovaGE::Matrix4<float>& view_matrix,
               const PyNovaGE::Matrix4<float>& projection_matrix,
               const PyNovaGE::Vector3f& camera_pos);

    /**
     * @brief Render all batches
     * @param view_matrix View matrix
     * @param projection_matrix Projection matrix
     */
    void Render(const PyNovaGE::Matrix4<float>& view_matrix,
               const PyNovaGE::Matrix4<float>& projection_matrix);

    /**
     * @brief Get rendering statistics
     */
    struct Stats {
        size_t total_instances;
        size_t culled_instances;
        size_t rendered_instances;
        size_t draw_calls;
        size_t triangles;
        float update_time_ms;
        float render_time_ms;
    };
    
    Stats GetStats() const { return stats_; }

    /**
     * @brief Set configuration
     */
    void SetConfig(const Config& config) { config_ = config; }
    
    /**
     * @brief Get configuration
     */
    const Config& GetConfig() const { return config_; }

private:
    Config config_;
    std::unordered_map<std::string, std::unique_ptr<InstanceBatch>> batches_;
    std::shared_ptr<Shader> default_shader_;
    
    // Frustum culling
    struct Frustum {
        PyNovaGE::Vector4f planes[6];  // Left, Right, Bottom, Top, Near, Far
    } frustum_;
    
    Stats stats_;
    
    // Private methods
    void ExtractFrustumPlanes(const PyNovaGE::Matrix4<float>& view_proj_matrix);
    bool IsInstanceVisible(const InstanceData& instance, float radius = 1.0f) const;
    float CalculateDistance(const PyNovaGE::Vector3f& position, const PyNovaGE::Vector3f& camera_pos) const;
    void UploadInstanceData(InstanceBatch& batch);
    void CreateDefaultShader();
    void SortInstancesByDistance(std::vector<InstanceData>& instances, const PyNovaGE::Vector3f& camera_pos);
};

} // namespace Renderer
} // namespace PyNovaGE