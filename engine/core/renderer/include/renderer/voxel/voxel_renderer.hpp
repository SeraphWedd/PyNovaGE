#pragma once

#include "camera.hpp"
#include "chunk.hpp"
#include "meshing.hpp"
#include "frustum_culler.hpp"
#include "shader_manager.hpp"
#include "renderer/texture_array.hpp"
#include <vectors/vector3.hpp>
#include <matrices/matrix4.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <chrono>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

/**
 * @brief Chunk render data for GPU rendering
 */
struct ChunkRenderData {
    std::unique_ptr<VoxelMesh> mesh;         // GPU mesh data
    Vector3f world_position;                 // World position of chunk
    bool needs_remesh = true;                // Flag for mesh regeneration
    bool is_uploading = false;               // Currently uploading to GPU
    uint32_t last_modified_frame = 0;        // Frame when chunk was last modified
    GreedyMesher::MeshData cpu_mesh_data;    // CPU mesh data (temp storage)
    std::atomic<bool> mesh_ready{false};     // Thread-safe mesh ready flag

    ChunkRenderData() = default;
    ChunkRenderData(const Vector3f& pos) : world_position(pos) {}
    
    // Move-only type
    ChunkRenderData(ChunkRenderData&& other) noexcept = default;
    ChunkRenderData& operator=(ChunkRenderData&& other) noexcept = default;
    ChunkRenderData(const ChunkRenderData&) = delete;
    ChunkRenderData& operator=(const ChunkRenderData&) = delete;
};

/**
 * @brief Voxel world interface for chunk management
 */
class VoxelWorld {
public:
    virtual ~VoxelWorld() = default;
    
    /**
     * @brief Get chunk at world position
     * @param world_position World position to query
     * @return Pointer to chunk (nullptr if not loaded)
     */
    virtual const Chunk* GetChunk(const Vector3f& world_position) const = 0;
    
    /**
     * @brief Get all loaded chunks
     * @return Vector of (chunk, world_position) pairs
     */
    virtual std::vector<std::pair<const Chunk*, Vector3f>> GetAllChunks() const = 0;
    
    /**
     * @brief Get chunks within a radius of a position
     * @param center Center position
     * @param radius Radius to search
     * @return Vector of (chunk, world_position) pairs
     */
    virtual std::vector<std::pair<const Chunk*, Vector3f>> GetChunksInRadius(
        const Vector3f& center, float radius) const = 0;
    
    /**
     * @brief Check if chunk was modified since frame
     * @param world_position Chunk world position
     * @param frame Frame number to check
     * @return True if chunk was modified after frame
     */
    virtual bool WasChunkModified(const Vector3f& world_position, uint32_t frame) const = 0;
};

/**
 * @brief Render statistics for performance monitoring
 */
struct VoxelRenderStats {
    // Frame statistics
    uint32_t frame_number = 0;
    double frame_time_ms = 0.0;
    double render_time_ms = 0.0;
    
    // Chunk statistics
    size_t total_chunks = 0;
    size_t visible_chunks = 0;
    size_t rendered_chunks = 0;
    size_t culled_chunks = 0;
    
    // Mesh statistics
    size_t chunks_remeshed = 0;
    size_t vertices_rendered = 0;
    size_t indices_rendered = 0;
    size_t draw_calls = 0;
    
    // Memory statistics
    size_t gpu_memory_used = 0;      // Estimated GPU memory usage
    size_t cpu_memory_used = 0;      // CPU memory for chunk data
    
    // Performance metrics
    float culling_ratio = 0.0f;      // Percentage of chunks culled
    float fps = 0.0f;                // Frames per second
    double mesh_generation_time_ms = 0.0; // Time spent generating meshes
    double gpu_upload_time_ms = 0.0; // Time spent uploading to GPU
    
    /**
     * @brief Reset statistics for new frame
     */
    void Reset() {
        *this = VoxelRenderStats{};
        frame_number++;
    }
};

/**
 * @brief Voxel renderer configuration
 */
struct VoxelRenderConfig {
    // Rendering settings
    bool enable_frustum_culling = true;
    bool enable_distance_culling = true;
    bool enable_wireframe = false;
    bool enable_face_culling = true;
    float max_render_distance = 500.0f;
    
    // Quality settings
    bool enable_ambient_occlusion = true;
    float ao_strength = 0.75f;            // AO intensity (0..1)
    bool enable_normal_mapping = false;
    bool enable_texture_arrays = true;
    int anisotropic_filtering = 16;
    
    // Day/Night cycle
    bool enable_day_night = true;
    float day_cycle_seconds = 120.0f; // 2-minute cycle (short sample)
    
    // Performance settings
    bool enable_multithreaded_meshing = true;
    size_t max_remesh_per_frame = 8;        // Max chunks to remesh per frame
    size_t max_upload_per_frame = 4;        // Max chunks to upload to GPU per frame
    size_t mesh_worker_threads = 4;         // Number of meshing threads
    
    // LOD settings
    bool enable_lod = false;                // Level of detail (future)
    float lod_distances[4] = {50.0f, 100.0f, 200.0f, 400.0f};
    
    // Debug settings
    bool show_chunk_bounds = false;
    bool show_culling_debug = false;
    bool collect_detailed_stats = true;
};

/**
 * @brief Mesh generation task for background processing
 */
struct MeshTask {
    const Chunk* chunk;
    Vector3f world_position;
    std::array<const Chunk*, 6> neighbors;
    uint32_t task_id;
    
    MeshTask(const Chunk* c, const Vector3f& pos, uint32_t id)
        : chunk(c), world_position(pos), task_id(id), neighbors{} {}
};

/**
 * @brief Main voxel renderer class
 * 
 * Coordinates all voxel rendering systems including meshing, culling,
 * and GPU rendering. Supports multithreaded mesh generation and 
 * efficient chunk management.
 */
class VoxelRenderer {
public:
    /**
     * @brief Constructor
     * @param shader_directory Directory containing voxel shaders
     */
    explicit VoxelRenderer(const std::string& shader_directory = "shaders/voxel/");
    
    /**
     * @brief Destructor - cleans up resources and threads
     */
    ~VoxelRenderer();
    
    /**
     * @brief Initialize the renderer
     * @return True if initialization succeeded
     */
    bool Initialize();
    
    /**
     * @brief Shutdown the renderer
     */
    void Shutdown();
    
    /**
     * @brief Set voxel world instance
     * @param world Voxel world to render
     */
    void SetWorld(VoxelWorld* world) { world_ = world; }
    
    /**
     * @brief Set render configuration
     * @param config Render configuration
     */
    void SetConfig(const VoxelRenderConfig& config);
    
    /**
     * @brief Get current configuration
     */
    const VoxelRenderConfig& GetConfig() const { return config_; }
    
    /**
     * @brief Update renderer (call once per frame)
     * @param delta_time Time since last frame
     * @param camera Current camera
     */
    void Update(float delta_time, const Camera& camera);
    
    /**
     * @brief Render all visible chunks
     * @param camera Current camera
     */
    void Render(const Camera& camera);
    
    /**
     * @brief Force remesh of specific chunk
     * @param world_position World position of chunk to remesh
     */
    void InvalidateChunk(const Vector3f& world_position);
    
    /**
     * @brief Force remesh of chunks in area
     * @param center Center of area
     * @param radius Radius of area
     */
    void InvalidateArea(const Vector3f& center, float radius);
    
    /**
     * @brief Get render statistics
     */
    const VoxelRenderStats& GetStats() const { return stats_; }
    
    /**
     * @brief Get shader manager
     */
    VoxelShaderManager& GetShaderManager() { return shader_manager_; }
    
    /**
     * @brief Get frustum culler
     */
    FrustumCuller& GetFrustumCuller() { return frustum_culler_; }
    
    /**
     * @brief Set debug visualization callback
     * @param callback Function to render debug visuals
     */
    void SetDebugRenderCallback(std::function<void(const VoxelRenderStats&)> callback) {
        debug_render_callback_ = std::move(callback);
    }

private:
    /**
     * @brief Update chunk render data
     * @param camera Current camera for distance calculations
     */
    void UpdateChunkRenderData(const Camera& camera);
    
    /**
     * @brief Process mesh generation queue
     */
    void ProcessMeshQueue();
    
    /**
     * @brief Upload completed meshes to GPU
     */
    void UploadMeshesToGPU();
    
    /**
     * @brief Perform frustum culling on chunks
     * @param camera Current camera
     * @return Vector of visible chunk render data
     */
    std::vector<ChunkRenderData*> CullChunks(const Camera& camera);
    
    /**
     * @brief Render a batch of chunks
     * @param chunks Chunks to render
     * @param camera Current camera
     */
    void RenderChunks(const std::vector<ChunkRenderData*>& chunks, const Camera& camera);
    
    /**
     * @brief Setup rendering state
     * @param camera Current camera
     */
    void SetupRenderState(const Camera& camera);
    
    /**
     * @brief Cleanup rendering state
     */
    void CleanupRenderState();
    
    /**
     * @brief Render sky (full-screen pass with sun disc)
     */
    void RenderSky(const Camera& camera);
    
    /**
     * @brief Generate mesh for chunk (runs on worker threads)
     * @param task Mesh generation task
     */
    void GenerateMesh(const MeshTask& task);
    
    /**
     * @brief Mesh worker thread function
     */
    void MeshWorkerThread();
    
    /**
     * @brief Get chunk render data, creating if necessary
     * @param world_position Chunk world position
     * @return Reference to chunk render data
     */
    ChunkRenderData& GetOrCreateChunkRenderData(const Vector3f& world_position);
    
    /**
     * @brief Cleanup old/unused chunk render data
     */
    void CleanupUnusedChunks();
    
    /**
     * @brief Update render statistics
     */
    void UpdateStats();
    
    /**
     * @brief Render debug visualizations
     */
    void RenderDebug();

private:
    // Core systems
    VoxelShaderManager shader_manager_;
    GreedyMesher mesher_;
    FrustumCuller frustum_culler_;
    
    // World and configuration
    VoxelWorld* world_ = nullptr;
    VoxelRenderConfig config_;
    
    // Chunk management
    std::unordered_map<size_t, std::unique_ptr<ChunkRenderData>> chunk_render_data_;
    std::vector<ChunkRenderData*> visible_chunks_;
    
    // Multithreading
    std::vector<std::thread> mesh_workers_;
    std::queue<MeshTask> mesh_queue_;
    std::queue<std::pair<uint32_t, GreedyMesher::MeshData>> completed_meshes_;
    std::mutex mesh_queue_mutex_;
    std::mutex completed_mesh_mutex_;
    std::atomic<bool> shutdown_workers_{false};
    std::atomic<uint32_t> next_task_id_{1};
    
    // Statistics and timing
    VoxelRenderStats stats_;
    uint32_t current_frame_ = 0;
    std::chrono::steady_clock::time_point frame_start_time_;
    
    // Day/Night timing
    float time_of_day_seconds_ = 0.0f;
    
    // Debug and callbacks
    std::function<void(const VoxelRenderStats&)> debug_render_callback_;
    
    // Texture array for voxel materials
    std::unique_ptr<PyNovaGE::Renderer::TextureArray> texture_array_;
    
    // Initialization state
    bool initialized_ = false;
    
    // Sky rendering
    uint32_t sky_vao_ = 0;

public:
    // Hash function for world positions (for unordered_map key)
    struct Vector3Hash {
        size_t operator()(const Vector3f& v) const {
            // Simple hash combining x, y, z coordinates
            size_t h1 = std::hash<float>{}(v.x);
            size_t h2 = std::hash<float>{}(v.y);
            size_t h3 = std::hash<float>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

private:
    
    /**
     * @brief Convert world position to hash key
     */
    size_t WorldPositionToKey(const Vector3f& pos) const {
        return Vector3Hash{}(pos);
    }
};

/**
 * @brief Simple voxel world implementation for testing
 */
class SimpleVoxelWorld : public VoxelWorld {
public:
    /**
     * @brief Constructor
     * @param world_size Size of world in chunks
     */
    explicit SimpleVoxelWorld(int world_size = 8);
    
    /**
     * @brief Destructor
     */
    ~SimpleVoxelWorld() override = default;
    
    // VoxelWorld interface
    const Chunk* GetChunk(const Vector3f& world_position) const override;
    std::vector<std::pair<const Chunk*, Vector3f>> GetAllChunks() const override;
    std::vector<std::pair<const Chunk*, Vector3f>> GetChunksInRadius(
        const Vector3f& center, float radius) const override;
    bool WasChunkModified(const Vector3f& world_position, uint32_t frame) const override;
    
    /**
     * @brief Generate test world with various voxel patterns
     */
    void GenerateTestWorld();
    
    /**
     * @brief Set voxel at world position
     * @param world_pos World position
     * @param voxel_type Voxel type to set
     */
    void SetVoxel(const Vector3f& world_pos, VoxelType voxel_type);
    
    /**
     * @brief Get voxel at world position
     * @param world_pos World position
     * @return Voxel type at position
     */
    VoxelType GetVoxel(const Vector3f& world_pos) const;

private:
    /**
     * @brief Convert world position to chunk coordinates
     */
    Vector3f WorldToChunkPosition(const Vector3f& world_pos) const;
    
    /**
     * @brief Convert world position to local voxel coordinates
     */
    ChunkCoord WorldToLocalCoord(const Vector3f& world_pos) const;

private:
    int world_size_;
    std::unordered_map<size_t, std::unique_ptr<Chunk>> chunks_;
    std::unordered_map<size_t, uint32_t> chunk_modified_frames_;
    VoxelRenderer::Vector3Hash hasher_;
};

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE