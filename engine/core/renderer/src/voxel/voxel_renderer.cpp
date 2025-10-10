#include "renderer/voxel/voxel_renderer.hpp"
#include <chrono>
#include <algorithm>
#include <cmath>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

VoxelRenderer::VoxelRenderer(const std::string& shader_directory)
    : shader_manager_(shader_directory) {
}

VoxelRenderer::~VoxelRenderer() {
    if (initialized_) {
        Shutdown();
    }
}

bool VoxelRenderer::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // Initialize shader manager
    if (!shader_manager_.Initialize()) {
        return false;
    }
    
    // Load default shader presets
    if (!shader_manager_.LoadShaderPreset(VoxelShaderManager::ShaderPreset::Standard)) {
        return false;
    }
    
    // Configure mesher
    GreedyMesher::Config mesher_config;
    mesher_config.enable_ambient_occlusion = config_.enable_ambient_occlusion;
    mesher_config.enable_face_culling = config_.enable_face_culling;
    mesher_.SetConfig(mesher_config);
    
    // Configure frustum culler
    FrustumCuller::Config culler_config;
    culler_config.enable_frustum_culling = config_.enable_frustum_culling;
    culler_config.enable_distance_culling = config_.enable_distance_culling;
    culler_config.max_render_distance = config_.max_render_distance;
    frustum_culler_.SetConfig(culler_config);
    
    // Start mesh worker threads
    if (config_.enable_multithreaded_meshing && config_.mesh_worker_threads > 0) {
        shutdown_workers_ = false;
        mesh_workers_.reserve(config_.mesh_worker_threads);
        
        for (size_t i = 0; i < config_.mesh_worker_threads; ++i) {
            mesh_workers_.emplace_back(&VoxelRenderer::MeshWorkerThread, this);
        }
    }
    
    initialized_ = true;
    return true;
}

void VoxelRenderer::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // Shutdown worker threads
    shutdown_workers_ = true;
    for (auto& worker : mesh_workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    mesh_workers_.clear();
    
    // Clear queues
    {
        std::lock_guard<std::mutex> lock(mesh_queue_mutex_);
        while (!mesh_queue_.empty()) {
            mesh_queue_.pop();
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(completed_mesh_mutex_);
        while (!completed_meshes_.empty()) {
            completed_meshes_.pop();
        }
    }
    
    // Clear chunk render data
    chunk_render_data_.clear();
    visible_chunks_.clear();
    
    initialized_ = false;
}

void VoxelRenderer::SetConfig(const VoxelRenderConfig& config) {
    config_ = config;
    
    if (initialized_) {
        // Update subsystem configurations
        GreedyMesher::Config mesher_config = mesher_.GetConfig();
        mesher_config.enable_ambient_occlusion = config_.enable_ambient_occlusion;
        mesher_config.enable_face_culling = config_.enable_face_culling;
        mesher_.SetConfig(mesher_config);
        
        FrustumCuller::Config culler_config = frustum_culler_.GetConfig();
        culler_config.enable_frustum_culling = config_.enable_frustum_culling;
        culler_config.enable_distance_culling = config_.enable_distance_culling;
        culler_config.max_render_distance = config_.max_render_distance;
        frustum_culler_.SetConfig(culler_config);
    }
}

void VoxelRenderer::Update([[maybe_unused]] float delta_time, const Camera& camera) {
    if (!initialized_ || !world_) {
        return;
    }
    
    frame_start_time_ = std::chrono::steady_clock::now();
    stats_.Reset();
    current_frame_++;
    
    // Update frustum culler with current camera
    frustum_culler_.UpdateCamera(camera);
    
    // Update chunk render data
    UpdateChunkRenderData(camera);
    
    // Process completed meshes
    UploadMeshesToGPU();
    
    // Process mesh generation queue
    ProcessMeshQueue();
    
    // Update statistics
    UpdateStats();
}

void VoxelRenderer::Render(const Camera& camera) {
    if (!initialized_ || !world_) {
        return;
    }
    
    auto render_start = std::chrono::steady_clock::now();
    
    // Setup rendering state
    SetupRenderState(camera);
    
    // Cull chunks
    visible_chunks_ = CullChunks(camera);
    
    // Render visible chunks
    RenderChunks(visible_chunks_, camera);
    
    // Render debug visualizations
    if (config_.show_chunk_bounds || config_.show_culling_debug) {
        RenderDebug();
    }
    
    // Cleanup rendering state
    CleanupRenderState();
    
    auto render_end = std::chrono::steady_clock::now();
    stats_.render_time_ms = std::chrono::duration<double, std::milli>(
        render_end - render_start).count();
    
    // Call debug callback if set
    if (debug_render_callback_) {
        debug_render_callback_(stats_);
    }
}

void VoxelRenderer::InvalidateChunk(const Vector3f& world_position) {
    size_t key = WorldPositionToKey(world_position);
    auto it = chunk_render_data_.find(key);
    if (it != chunk_render_data_.end()) {
        it->second->needs_remesh = true;
        it->second->last_modified_frame = current_frame_;
    }
}

void VoxelRenderer::InvalidateArea(const Vector3f& center, float radius) {
    float radius_squared = radius * radius;
    
    for (auto& [key, render_data] : chunk_render_data_) {
        Vector3f chunk_center = render_data->world_position + 
            Vector3f(CHUNK_SIZE * 0.5f, CHUNK_SIZE * 0.5f, CHUNK_SIZE * 0.5f);
        Vector3f to_chunk = chunk_center - center;
        
        if (to_chunk.lengthSquared() <= radius_squared) {
            render_data->needs_remesh = true;
            render_data->last_modified_frame = current_frame_;
        }
    }
}

void VoxelRenderer::UpdateChunkRenderData([[maybe_unused]] const Camera& camera) {
    // Get all chunks from world
    auto world_chunks = world_->GetAllChunks();
    stats_.total_chunks = world_chunks.size();
    
    // Update existing chunks and create new ones
    for (const auto& [chunk, world_pos] : world_chunks) {
        ChunkRenderData& render_data = GetOrCreateChunkRenderData(world_pos);
        
        // Check if chunk was modified
        if (world_->WasChunkModified(world_pos, render_data.last_modified_frame)) {
            render_data.needs_remesh = true;
            render_data.last_modified_frame = current_frame_;
        }
    }
    
    // Cleanup unused chunks (chunks no longer in world)
    CleanupUnusedChunks();
}

void VoxelRenderer::ProcessMeshQueue() {
    if (!config_.enable_multithreaded_meshing) {
        return;
    }
    
    size_t processed = 0;
    std::lock_guard<std::mutex> lock(mesh_queue_mutex_);
    
    // Add chunks that need remeshing to queue
    for (auto& [key, render_data] : chunk_render_data_) {
        if (render_data->needs_remesh && !render_data->is_uploading && 
            processed < config_.max_remesh_per_frame) {
            
            const Chunk* chunk = world_->GetChunk(render_data->world_position);
            if (chunk) {
                // Get neighbor chunks for better meshing
                std::array<const Chunk*, 6> neighbors = {};
                // TODO: Get actual neighbors from world
                
                MeshTask task(chunk, render_data->world_position, next_task_id_++);
                task.neighbors = neighbors;
                
                mesh_queue_.push(task);
                render_data->needs_remesh = false;
                processed++;
            }
        }
    }
}

void VoxelRenderer::UploadMeshesToGPU() {
    std::lock_guard<std::mutex> lock(completed_mesh_mutex_);
    
    size_t uploaded = 0;
    while (!completed_meshes_.empty() && uploaded < config_.max_upload_per_frame) {
        auto [task_id, mesh_data] = std::move(completed_meshes_.front());
        completed_meshes_.pop();
        
        // Find the corresponding chunk render data
        // TODO: Match task_id to chunk render data
        // For now, we'll need to store the task_id->world_pos mapping
        
        uploaded++;
        stats_.chunks_remeshed++;
    }
}

std::vector<ChunkRenderData*> VoxelRenderer::CullChunks([[maybe_unused]] const Camera& camera) {
    std::vector<ChunkRenderData*> visible;
    
    // Create culling info for all chunks
    std::vector<ChunkCullInfo> cull_infos;
    cull_infos.reserve(chunk_render_data_.size());
    
    for (auto& [key, render_data] : chunk_render_data_) {
        const Chunk* chunk = world_->GetChunk(render_data->world_position);
        if (chunk && render_data->mesh && !render_data->needs_remesh) {
            cull_infos.emplace_back(chunk, render_data->world_position);
        }
    }
    
    // Perform culling
    auto cull_result = frustum_culler_.CullChunks(cull_infos);
    
    // Extract visible chunks
    for (const auto& cull_info : cull_infos) {
        if (cull_info.is_visible) {
            size_t key = WorldPositionToKey(cull_info.world_position);
            auto it = chunk_render_data_.find(key);
            if (it != chunk_render_data_.end()) {
                visible.push_back(it->second.get());
            }
        }
    }
    
    stats_.visible_chunks = visible.size();
    stats_.culled_chunks = cull_infos.size() - visible.size();
    stats_.culling_ratio = cull_infos.empty() ? 0.0f : 
        static_cast<float>(stats_.culled_chunks) / cull_infos.size();
    
    return visible;
}

void VoxelRenderer::RenderChunks(const std::vector<ChunkRenderData*>& chunks, const Camera& camera) {
    if (chunks.empty()) {
        return;
    }
    
    // Get shader program
    auto* shader = shader_manager_.GetShaderProgram(VoxelShaderManager::ShaderPreset::Standard);
    if (!shader || !shader->IsValid()) {
        return;
    }
    
    shader->Use();
    
    // Set per-frame uniforms
    VoxelShaderManager::CameraMatrices camera_matrices{};
    camera_matrices.view_matrix = camera.GetViewMatrix();
    camera_matrices.projection_matrix = camera.GetProjectionMatrix();
    camera_matrices.view_projection_matrix = camera.GetViewProjectionMatrix();
    camera_matrices.camera_position = camera.GetPosition();
    camera_matrices.near_plane = camera.GetNearPlane();
    camera_matrices.far_plane = camera.GetFarPlane();
    camera_matrices.fov = camera.GetFOV();
    // TODO: Get viewport size from somewhere
    camera_matrices.viewport_size = Vector2f(1920.0f, 1080.0f);
    
    shader_manager_.UpdateCameraMatrices(camera_matrices);
    
    // Set lighting uniforms
    VoxelShaderManager::LightingData lighting{};
    lighting.sun_direction = Vector3f(-0.3f, -0.7f, -0.2f).normalized();
    lighting.sun_color = Vector3f(1.0f, 0.95f, 0.8f);
    lighting.sun_intensity = 1.0f;
    lighting.ambient_color = Vector3f(0.4f, 0.4f, 0.6f);
    lighting.ambient_intensity = 0.3f;
    lighting.gamma = 2.2f;
    lighting.enable_fog = true;
    lighting.fog_color = Vector3f(0.7f, 0.8f, 1.0f);
    lighting.fog_density = 0.02f;
    lighting.fog_start = 100.0f;
    lighting.fog_end = config_.max_render_distance;
    
    shader_manager_.UpdateLightingData(lighting);
    
    // Render each chunk
    for (ChunkRenderData* render_data : chunks) {
        if (!render_data->mesh) continue;
        
        // Set model matrix for chunk position
        Matrix4f model_matrix = Matrix4f::Identity();
        float* m_data = model_matrix.data.data();
        m_data[12] = render_data->world_position.x;
        m_data[13] = render_data->world_position.y;
        m_data[14] = render_data->world_position.z;
        
        shader->SetUniform("u_model_matrix", model_matrix);
        
        // Bind and draw mesh
        render_data->mesh->Bind();
        render_data->mesh->Draw();
        
        stats_.draw_calls++;
        // TODO: Update vertex/index counts from mesh data
    }
    
    stats_.rendered_chunks = chunks.size();
}

void VoxelRenderer::SetupRenderState([[maybe_unused]] const Camera& camera) {
    // TODO: Setup OpenGL state for voxel rendering
    // - Enable depth testing
    // - Set blending mode
    // - Configure face culling
    // - Bind texture arrays
}

void VoxelRenderer::CleanupRenderState() {
    // TODO: Restore previous OpenGL state
    // - Unbind textures
    // - Reset shader program
}

void VoxelRenderer::GenerateMesh(const MeshTask& task) {
    if (!task.chunk) return;
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Generate mesh using greedy mesher
    auto mesh_data = mesher_.GenerateMeshWithNeighbors(*task.chunk, task.neighbors);
    
    auto end_time = std::chrono::steady_clock::now();
    [[maybe_unused]] double generation_time = std::chrono::duration<double, std::milli>(
        end_time - start_time).count();
    
    // Add to completed queue
    {
        std::lock_guard<std::mutex> lock(completed_mesh_mutex_);
        completed_meshes_.emplace(task.task_id, std::move(mesh_data));
    }
}

void VoxelRenderer::MeshWorkerThread() {
    while (!shutdown_workers_) {
        MeshTask task{nullptr, {}, 0};
        
        // Get task from queue
        {
            std::lock_guard<std::mutex> lock(mesh_queue_mutex_);
            if (!mesh_queue_.empty()) {
                task = std::move(mesh_queue_.front());
                mesh_queue_.pop();
            }
        }
        
        if (task.chunk) {
            GenerateMesh(task);
        } else {
            // No work available, sleep briefly
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

ChunkRenderData& VoxelRenderer::GetOrCreateChunkRenderData(const Vector3f& world_position) {
    size_t key = WorldPositionToKey(world_position);
    auto it = chunk_render_data_.find(key);
    
    if (it == chunk_render_data_.end()) {
        auto render_data = std::make_unique<ChunkRenderData>(world_position);
        auto* ptr = render_data.get();
        chunk_render_data_[key] = std::move(render_data);
        return *ptr;
    }
    
    return *it->second;
}

void VoxelRenderer::CleanupUnusedChunks() {
    // TODO: Remove chunk render data for chunks no longer in world
    // This requires tracking which chunks are still active
}

void VoxelRenderer::UpdateStats() {
    auto current_time = std::chrono::steady_clock::now();
    stats_.frame_time_ms = std::chrono::duration<double, std::milli>(
        current_time - frame_start_time_).count();
    
    // Calculate FPS
    if (stats_.frame_time_ms > 0.0) {
        stats_.fps = static_cast<float>(1000.0 / stats_.frame_time_ms);
    }
    
    // Update memory estimates
    stats_.cpu_memory_used = chunk_render_data_.size() * sizeof(ChunkRenderData);
    // TODO: Calculate GPU memory usage from mesh data
}

void VoxelRenderer::RenderDebug() {
    // TODO: Render debug visualizations
    // - Chunk bounding boxes
    // - Frustum wireframe
    // - Culling debug info
}

// SimpleVoxelWorld implementation
SimpleVoxelWorld::SimpleVoxelWorld(int world_size) : world_size_(world_size) {
    GenerateTestWorld();
}

const Chunk* SimpleVoxelWorld::GetChunk(const Vector3f& world_position) const {
    size_t key = hasher_(WorldToChunkPosition(world_position));
    auto it = chunks_.find(key);
    return it != chunks_.end() ? it->second.get() : nullptr;
}

std::vector<std::pair<const Chunk*, Vector3f>> SimpleVoxelWorld::GetAllChunks() const {
    std::vector<std::pair<const Chunk*, Vector3f>> result;
    result.reserve(chunks_.size());
    
    for (const auto& [key, chunk] : chunks_) {
        // Reverse hash to get position (this is a simplification)
        // In a real implementation, you'd store the position separately
        Vector3f pos = chunk->GetWorldPosition(); // Assuming Chunk stores its position
        result.emplace_back(chunk.get(), pos);
    }
    
    return result;
}

std::vector<std::pair<const Chunk*, Vector3f>> SimpleVoxelWorld::GetChunksInRadius(
    const Vector3f& center, float radius) const {
    
    std::vector<std::pair<const Chunk*, Vector3f>> result;
    float radius_squared = radius * radius;
    
    for (const auto& [key, chunk] : chunks_) {
        Vector3f chunk_pos = chunk->GetWorldPosition(); // Assuming Chunk stores its position
        Vector3f chunk_center = chunk_pos + Vector3f(CHUNK_SIZE * 0.5f);
        Vector3f to_chunk = chunk_center - center;
        
        if (to_chunk.lengthSquared() <= radius_squared) {
            result.emplace_back(chunk.get(), chunk_pos);
        }
    }
    
    return result;
}

bool SimpleVoxelWorld::WasChunkModified(const Vector3f& world_position, uint32_t frame) const {
    size_t key = hasher_(WorldToChunkPosition(world_position));
    auto it = chunk_modified_frames_.find(key);
    return it != chunk_modified_frames_.end() && it->second > frame;
}

void SimpleVoxelWorld::GenerateTestWorld() {
    // Generate a simple test world with ground plane and some structures
    for (int x = 0; x < world_size_; ++x) {
        for (int z = 0; z < world_size_; ++z) {
            Vector3f chunk_pos(static_cast<float>(x * CHUNK_SIZE), 0.0f, static_cast<float>(z * CHUNK_SIZE));
            size_t key = hasher_(chunk_pos);
            
            auto chunk = std::make_unique<Chunk>();
            
            // Fill with test pattern
            for (int cy = 0; cy < CHUNK_SIZE; ++cy) {
                for (int cz = 0; cz < CHUNK_SIZE; ++cz) {
                    for (int cx = 0; cx < CHUNK_SIZE; ++cx) {
                        VoxelType voxel = VoxelType::AIR;
                        
                        // Ground layer
                        if (cy < 2) {
                            voxel = VoxelType::STONE;
                        }
                        // Grass layer
                        else if (cy == 2) {
                            voxel = VoxelType::DIRT; // Or grass when available
                        }
                        
                        chunk->SetVoxel({cx, cy, cz}, voxel);
                    }
                }
            }
            
            chunks_[key] = std::move(chunk);
            chunk_modified_frames_[key] = 0;
        }
    }
}

void SimpleVoxelWorld::SetVoxel(const Vector3f& world_pos, VoxelType voxel_type) {
    Vector3f chunk_pos = WorldToChunkPosition(world_pos);
    size_t key = hasher_(chunk_pos);
    
    auto it = chunks_.find(key);
    if (it != chunks_.end()) {
        ChunkCoord local_coord = WorldToLocalCoord(world_pos);
        it->second->SetVoxel(local_coord, voxel_type);
        chunk_modified_frames_[key]++; // Simple modification tracking
    }
}

VoxelType SimpleVoxelWorld::GetVoxel(const Vector3f& world_pos) const {
    Vector3f chunk_pos = WorldToChunkPosition(world_pos);
    size_t key = hasher_(chunk_pos);
    
    auto it = chunks_.find(key);
    if (it != chunks_.end()) {
        ChunkCoord local_coord = WorldToLocalCoord(world_pos);
        return it->second->GetVoxel(local_coord);
    }
    
    return VoxelType::AIR;
}

Vector3f SimpleVoxelWorld::WorldToChunkPosition(const Vector3f& world_pos) const {
    return Vector3f(
        std::floor(world_pos.x / CHUNK_SIZE) * CHUNK_SIZE,
        std::floor(world_pos.y / CHUNK_SIZE) * CHUNK_SIZE,
        std::floor(world_pos.z / CHUNK_SIZE) * CHUNK_SIZE
    );
}

ChunkCoord SimpleVoxelWorld::WorldToLocalCoord(const Vector3f& world_pos) const {
    return ChunkCoord{
        static_cast<int>(world_pos.x) % CHUNK_SIZE,
        static_cast<int>(world_pos.y) % CHUNK_SIZE,
        static_cast<int>(world_pos.z) % CHUNK_SIZE
    };
}

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE