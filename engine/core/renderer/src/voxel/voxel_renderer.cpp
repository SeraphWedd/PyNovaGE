#include "renderer/voxel/voxel_renderer.hpp"
#include <chrono>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <glad/gl.h>
#include "renderer/texture_array.hpp"

#ifndef PVG_VOXEL_DEBUG_LOGS
#define PVG_VOXEL_DEBUG_LOGS 0
#endif

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
    // Load sky shader program (uses same shaders directory)
    if (!shader_manager_.LoadShaderProgram("sky", "sky.vert", "sky.frag")) {
        std::cerr << "Failed to load sky shader" << std::endl;
        // Not fatal; continue without sky
    }
    // Load shadow shader preset (depth-only)
    shader_manager_.LoadShaderPreset(VoxelShaderManager::ShaderPreset::Shadow);

    // Create a minimal VAO for full-screen triangle
    glGenVertexArrays(1, &sky_vao_);

    // Create shadow map resources
    glGenFramebuffers(1, &shadow_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_);
    glGenTextures(1, &shadow_depth_tex_);
    glBindTexture(GL_TEXTURE_2D, shadow_depth_tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadow_map_size_, shadow_map_size_, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_depth_tex_, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create point shadow cubemap resources (pool of slots)
    for (int s = 0; s < MAX_POINT_SHADOW_SLOTS; ++s) {
        glGenFramebuffers(1, &point_shadow_fbos_[s]);
        glGenTextures(1, &point_shadow_depth_cubes_[s]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_depth_cubes_[s]);
        for (int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
                         point_shadow_size_, point_shadow_size_, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    // Create and load block textures
    texture_array_ = std::make_unique<TextureArray>();
    const int texW = 16, texH = 16, layers = 5; // STONE, DIRT, GRASS, WOOD, LEAVES
    if (!texture_array_->Create(texW, texH, layers, TextureFormat::RGBA, true)) {
        std::cerr << "Failed to create block texture array" << std::endl;
        return false;
    }

    // Configure texture array
    texture_array_->SetFilter(
        TextureFilter::LinearMipmapLinear,  // min filter (trilinear)
        TextureFilter::Linear               // mag filter
    );
    texture_array_->SetWrap(TextureWrap::Repeat, TextureWrap::Repeat);

    // Create procedural textures since we don't have PNG files yet
    auto makeSolid = [&](unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        std::vector<unsigned char> data(texW * texH * 4);
        for (int i = 0; i < texW * texH; ++i) {
            data[i*4+0] = r; data[i*4+1] = g; data[i*4+2] = b; data[i*4+3] = a;
        }
        return data;
    };

    // Simple procedural textures
    auto stone = makeSolid(140, 140, 150, 255);
    auto dirt  = makeSolid(115, 77,  46,  255);
    auto grass = makeSolid(60,  150, 70,  255);

    // Wood: two-tone stripes
    std::vector<unsigned char> wood(texW * texH * 4);
    for (int y = 0; y < texH; ++y) {
        for (int x = 0; x < texW; ++x) {
            bool stripe = ((x / 2) % 2) == 0;
            unsigned char r = stripe ? 120 : 100;
            unsigned char g = stripe ? 85  : 70;
            unsigned char b = stripe ? 50  : 40;
            int idx = (y * texW + x) * 4;
            wood[idx+0] = r; wood[idx+1] = g; wood[idx+2] = b; wood[idx+3] = 255;
        }
    }

    // Leaves: green with alpha cutouts (checkerboard holes)
    std::vector<unsigned char> leaves(texW * texH * 4);
    for (int y = 0; y < texH; ++y) {
        for (int x = 0; x < texW; ++x) {
            bool hole = ((x + y) % 4) == 0; // sparse holes
            int idx = (y * texW + x) * 4;
            leaves[idx+0] = 50;  // R
            leaves[idx+1] = 140; // G
            leaves[idx+2] = 60;  // B
            leaves[idx+3] = hole ? 0 : 255;
        }
    }

    // Upload procedural textures
    texture_array_->SetLayerData(0, TextureFormat::RGBA, TextureDataType::UnsignedByte, stone.data());
    texture_array_->SetLayerData(1, TextureFormat::RGBA, TextureDataType::UnsignedByte, dirt.data());
    texture_array_->SetLayerData(2, TextureFormat::RGBA, TextureDataType::UnsignedByte, grass.data());
    texture_array_->SetLayerData(3, TextureFormat::RGBA, TextureDataType::UnsignedByte, wood.data());
    texture_array_->SetLayerData(4, TextureFormat::RGBA, TextureDataType::UnsignedByte, leaves.data());
    
    // Configure mesher
    {
        GreedyMesher::Config mesher_config;
        mesher_config.enable_ambient_occlusion = config_.enable_ambient_occlusion;
        mesher_config.ao_strength = config_.ao_strength;
        mesher_config.enable_face_culling = config_.enable_face_culling;
        mesher_.SetConfig(mesher_config);
    }
    
    // Configure frustum culler
    {
        FrustumCuller::Config culler_config;
        culler_config.enable_frustum_culling = config_.enable_frustum_culling;
        culler_config.enable_distance_culling = config_.enable_distance_culling;
        culler_config.max_render_distance = config_.max_render_distance;
        frustum_culler_.SetConfig(culler_config);
    }
    
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
    // Delete point shadow resources
    for (int s = 0; s < MAX_POINT_SHADOW_SLOTS; ++s) {
        if (point_shadow_depth_cubes_[s] != 0) {
            glDeleteTextures(1, &point_shadow_depth_cubes_[s]);
            point_shadow_depth_cubes_[s] = 0;
        }
        if (point_shadow_fbos_[s] != 0) {
            glDeleteFramebuffers(1, &point_shadow_fbos_[s]);
            point_shadow_fbos_[s] = 0;
        }
    }
    // Delete shadow resources
    if (shadow_depth_tex_ != 0) {
        glDeleteTextures(1, &shadow_depth_tex_);
        shadow_depth_tex_ = 0;
    }
    if (shadow_fbo_ != 0) {
        glDeleteFramebuffers(1, &shadow_fbo_);
        shadow_fbo_ = 0;
    }
    // Delete sky VAO
    if (sky_vao_ != 0) {
        glDeleteVertexArrays(1, &sky_vao_);
        sky_vao_ = 0;
    }
    if (!initialized_) {
        return;
    }

    // Clean up texture array
    texture_array_.reset();
    
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
        mesher_config.ao_strength = config_.ao_strength;
        mesher_config.enable_face_culling = config_.enable_face_culling;
        mesher_.SetConfig(mesher_config);
        
        FrustumCuller::Config culler_config = frustum_culler_.GetConfig();
        culler_config.enable_frustum_culling = config_.enable_frustum_culling;
        culler_config.enable_distance_culling = config_.enable_distance_culling;
        culler_config.max_render_distance = config_.max_render_distance;
        frustum_culler_.SetConfig(culler_config);
    }
}

void VoxelRenderer::GatherPointLights() {
    frame_point_lights_.clear();
    frame_point_lights_.reserve(MAX_POINT_LIGHTS);

    if (world_) {
        auto all_chunks = world_->GetAllChunks();
        for (const auto& pair : all_chunks) {
            const Chunk* c = pair.first;
            Vector3f chunk_world = pair.second;
            if (!c) continue;

            int topY[CHUNK_SIZE][CHUNK_SIZE];
            for (int z = 0; z < CHUNK_SIZE; ++z)
                for (int x = 0; x < CHUNK_SIZE; ++x)
                    topY[x][z] = -1;

            for (int y = 0; y < CHUNK_SIZE; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    for (int x = 0; x < CHUNK_SIZE; ++x) {
                        if (c->GetVoxel(x, y, z) == VoxelType::WOOD) {
                            if (y > topY[x][z]) topY[x][z] = y;
                        }
                    }
                }
            }

            for (int z = 0; z < CHUNK_SIZE; ++z) {
                for (int x = 0; x < CHUNK_SIZE; ++x) {
                    int yTop = topY[x][z];
                    if (yTop < 0) continue;

                    float sideOffsetX = ((x + z) % 2 == 0) ? 1.2f : -0.2f;
                    float sideOffsetZ = 0.5f;

                    Vector3f pos = chunk_world + Vector3f(static_cast<float>(x) + sideOffsetX,
                                                          static_cast<float>(yTop) + 1.0f,
                                                          static_cast<float>(z) + sideOffsetZ);
                    frame_point_lights_.push_back({ pos, Vector3f(1.0f, 0.85f, 0.6f), 2.0f, 18.0f });
                    if (frame_point_lights_.size() >= MAX_POINT_LIGHTS) break;
                }
                if (frame_point_lights_.size() >= MAX_POINT_LIGHTS) break;
            }
            if (frame_point_lights_.size() >= MAX_POINT_LIGHTS) break;
        }
    }

    if (frame_point_lights_.empty()) {
        frame_point_lights_.push_back({ Vector3f(8.0f, 6.0f, 8.0f), Vector3f(1.0f, 0.85f, 0.6f), 2.5f, 20.0f });
        frame_point_lights_.push_back({ Vector3f(24.0f, 5.0f, 10.0f), Vector3f(0.6f, 0.7f, 1.0f), 2.0f, 18.0f });
    }
}

void VoxelRenderer::Update([[maybe_unused]] float delta_time, const Camera& camera) {
    if (!initialized_ || !world_) {
        return;
    }
    
    frame_start_time_ = std::chrono::steady_clock::now();
    stats_.Reset();
    current_frame_++;

    // Advance day/night time
    if (config_.enable_day_night && config_.day_cycle_seconds > 0.0f) {
        time_of_day_seconds_ += delta_time;
        if (time_of_day_seconds_ >= config_.day_cycle_seconds) {
            // Wrap around to keep values small
            time_of_day_seconds_ = std::fmod(time_of_day_seconds_, config_.day_cycle_seconds);
        }
    }
    
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
    
    // 1) Shadow map passes
    GatherPointLights();
    RenderShadowMap(camera);      // sun
    RenderPointShadowMap(camera); // point lights (multi)

    // 2) Sky pass
    RenderSky(camera);

    // 3) Setup rendering state for voxels
    SetupRenderState(camera);
    
    // Cull chunks
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
    
    // Track existing chunks to identify new ones
    std::set<size_t> existing_chunk_keys;
    for (const auto& [key, _] : chunk_render_data_) {
        existing_chunk_keys.insert(key);
    }
    
    // Update existing chunks and create new ones
    for (const auto& [chunk, world_pos] : world_chunks) {
        size_t key = WorldPositionToKey(world_pos);
        bool is_new_chunk = existing_chunk_keys.find(key) == existing_chunk_keys.end();
        
        ChunkRenderData& render_data = GetOrCreateChunkRenderData(world_pos);
        
        // Mark new chunks for remeshing immediately
        if (is_new_chunk) {
            render_data.needs_remesh = true;
            render_data.last_modified_frame = current_frame_;
#if PVG_VOXEL_DEBUG_LOGS
            std::cout << "New chunk at (" << world_pos.x << ", " << world_pos.y << ", " << world_pos.z << ") marked for meshing" << std::endl;
#endif
        }
        
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
    size_t processed = 0;
    
    if (!config_.enable_multithreaded_meshing) {
        // Handle immediate meshing when multithreading is disabled
        for (auto& [key, render_data] : chunk_render_data_) {
            if (render_data->needs_remesh && !render_data->is_uploading && 
                processed < config_.max_remesh_per_frame) {
                
                const Chunk* chunk = world_->GetChunk(render_data->world_position);
                if (chunk) {
#if PVG_VOXEL_DEBUG_LOGS
                    std::cout << "Processing immediate mesh for chunk at (" 
                              << render_data->world_position.x << ", " 
                              << render_data->world_position.y << ", " 
                              << render_data->world_position.z << ")" << std::endl;
#endif
                    
                    // Get neighbor chunks for better meshing
                    std::array<const Chunk*, 6> neighbors = {};
                    const Vector3f& wp = render_data->world_position;
                    neighbors[static_cast<size_t>(Face::LEFT)]   = world_->GetChunk(Vector3f(wp.x - CHUNK_SIZE, wp.y, wp.z));
                    neighbors[static_cast<size_t>(Face::RIGHT)]  = world_->GetChunk(Vector3f(wp.x + CHUNK_SIZE, wp.y, wp.z));
                    neighbors[static_cast<size_t>(Face::BOTTOM)] = world_->GetChunk(Vector3f(wp.x, wp.y - CHUNK_SIZE, wp.z));
                    neighbors[static_cast<size_t>(Face::TOP)]    = world_->GetChunk(Vector3f(wp.x, wp.y + CHUNK_SIZE, wp.z));
                    neighbors[static_cast<size_t>(Face::BACK)]   = world_->GetChunk(Vector3f(wp.x, wp.y, wp.z - CHUNK_SIZE));
                    neighbors[static_cast<size_t>(Face::FRONT)]  = world_->GetChunk(Vector3f(wp.x, wp.y, wp.z + CHUNK_SIZE));
                    
                    // Generate mesh immediately
                    auto mesh_data = mesher_.GenerateMeshWithNeighbors(*chunk, neighbors);
                    
                    // Create VoxelMesh and upload immediately
                    if (!mesh_data.vertices.empty()) {
                        auto mesh = std::make_unique<VoxelMesh>();
                        
                        // Convert Vertex to VoxelVertex if needed
                        std::vector<VoxelVertex> voxel_vertices;
                        voxel_vertices.reserve(mesh_data.vertices.size());
                        for (const auto& v : mesh_data.vertices) {
                            voxel_vertices.push_back(v);
                        }
                        
                        // Upload to GPU
                        mesh->UploadData(voxel_vertices, mesh_data.indices);
                        
                        // Assign to render data
                        render_data->mesh = std::move(mesh);
                        
#if PVG_VOXEL_DEBUG_LOGS
                        std::cout << "Immediate mesh completed: " << voxel_vertices.size() 
                                  << " vertices, " << mesh_data.indices.size() << " indices" << std::endl;
#endif
                    }
                    
                    render_data->needs_remesh = false;
                    processed++;
                    stats_.chunks_remeshed++;
                }
            }
        }
        return;
    }
    
    // Multithreaded meshing path
    std::lock_guard<std::mutex> lock(mesh_queue_mutex_);
    
    // Add chunks that need remeshing to queue
    for (auto& [key, render_data] : chunk_render_data_) {
        if (render_data->needs_remesh && !render_data->is_uploading && 
            processed < config_.max_remesh_per_frame) {
            
            const Chunk* chunk = world_->GetChunk(render_data->world_position);
            if (chunk) {
            // Get neighbor chunks for better meshing
            std::array<const Chunk*, 6> neighbors = {};
            const Vector3f& wp = render_data->world_position;
            neighbors[static_cast<size_t>(Face::LEFT)]   = world_->GetChunk(Vector3f(wp.x - CHUNK_SIZE, wp.y, wp.z));
            neighbors[static_cast<size_t>(Face::RIGHT)]  = world_->GetChunk(Vector3f(wp.x + CHUNK_SIZE, wp.y, wp.z));
            neighbors[static_cast<size_t>(Face::BOTTOM)] = world_->GetChunk(Vector3f(wp.x, wp.y - CHUNK_SIZE, wp.z));
            neighbors[static_cast<size_t>(Face::TOP)]    = world_->GetChunk(Vector3f(wp.x, wp.y + CHUNK_SIZE, wp.z));
            neighbors[static_cast<size_t>(Face::BACK)]   = world_->GetChunk(Vector3f(wp.x, wp.y, wp.z - CHUNK_SIZE));
            neighbors[static_cast<size_t>(Face::FRONT)]  = world_->GetChunk(Vector3f(wp.x, wp.y, wp.z + CHUNK_SIZE));
            
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
        // Look for chunks that have no mesh (regardless of needs_remesh flag)
        for (auto& [key, render_data] : chunk_render_data_) {
            if (!render_data->mesh) {
                // Create VoxelMesh from mesh_data
                if (!mesh_data.vertices.empty()) {
                    auto mesh = std::make_unique<VoxelMesh>();
                    
                    // Convert Vertex to VoxelVertex if needed
                    std::vector<VoxelVertex> voxel_vertices;
                    voxel_vertices.reserve(mesh_data.vertices.size());
                    for (const auto& v : mesh_data.vertices) {
                        voxel_vertices.push_back(v);
                    }
                    
                    // Upload to GPU
                    mesh->UploadData(voxel_vertices, mesh_data.indices);
                    
                    // Assign to render data
                    render_data->mesh = std::move(mesh);
                    render_data->needs_remesh = false;
                }
                break; // Only process one chunk per mesh
            }
        }
        
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
#if PVG_VOXEL_DEBUG_LOGS
        std::cout << "RenderChunks: No chunks to render!" << std::endl;
#endif
        return;
    }
    
#if PVG_VOXEL_DEBUG_LOGS
    std::cout << "RenderChunks: Rendering " << chunks.size() << " chunks" << std::endl;
#endif
    
    // Get shader program
    auto* shader = shader_manager_.GetShaderProgram(VoxelShaderManager::ShaderPreset::Standard);
    if (!shader || !shader->IsValid()) {
#if PVG_VOXEL_DEBUG_LOGS
        std::cout << "RenderChunks: Shader program is invalid!" << std::endl;
#endif
        return;
    }
    
    shader->Use();
#if PVG_VOXEL_DEBUG_LOGS
    std::cout << "RenderChunks: Shader activated" << std::endl;
#endif
    
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
    
    // Debug: Print camera settings once
    static bool first_frame_debug = true;
#if !PVG_VOXEL_DEBUG_LOGS
    (void)first_frame_debug;
#endif
#if PVG_VOXEL_DEBUG_LOGS
    if (first_frame_debug) {
        std::cout << "=== CAMERA DEBUG INFO ===" << std::endl;
        std::cout << "Camera position: (" << camera_matrices.camera_position.x << ", "
                  << camera_matrices.camera_position.y << ", " << camera_matrices.camera_position.z << ")" << std::endl;
        std::cout << "Camera FOV: " << camera_matrices.fov << " degrees" << std::endl;
        std::cout << "Near/Far planes: " << camera_matrices.near_plane << " / " << camera_matrices.far_plane << std::endl;
        
        // Debug: Print projection matrix
        const float* proj = camera_matrices.projection_matrix.data.data();
        std::cout << "Projection matrix:" << std::endl;
        for (int row = 0; row < 4; ++row) {
            std::cout << "  [" << proj[row*4+0] << ", " << proj[row*4+1] << ", " << proj[row*4+2] << ", " << proj[row*4+3] << "]" << std::endl;
        }
        
        // Debug: Print view matrix
        const float* view = camera_matrices.view_matrix.data.data();
        std::cout << "View matrix:" << std::endl;
        for (int row = 0; row < 4; ++row) {
            std::cout << "  [" << view[row*4+0] << ", " << view[row*4+1] << ", " << view[row*4+2] << ", " << view[row*4+3] << "]" << std::endl;
        }
        
        first_frame_debug = false;
    }
#endif
    
    // Set camera matrix uniforms individually
    shader->SetUniform("u_view_matrix", camera_matrices.view_matrix);
    shader->SetUniform("u_projection_matrix", camera_matrices.projection_matrix);
    shader->SetUniform("u_view_projection_matrix", camera_matrices.view_projection_matrix);
    shader->SetUniform("u_camera_position", camera_matrices.camera_position);
    shader->SetUniform("u_near_plane", camera_matrices.near_plane);
    shader->SetUniform("u_far_plane", camera_matrices.far_plane);
    shader->SetUniform("u_fov", camera_matrices.fov);
    shader->SetUniform("u_viewport_size", camera_matrices.viewport_size);
    
    // Set lighting uniforms individually
    Vector3f sun_direction;
    Vector3f sun_color;
    float sun_intensity;
    Vector3f ambient_color = Vector3f(0.4f, 0.4f, 0.6f);
    float ambient_intensity;

    if (config_.enable_day_night && config_.day_cycle_seconds > 0.0f) {
        float t = time_of_day_seconds_ / config_.day_cycle_seconds; // 0..1
        float angle = t * 6.2831853f; // 0..2π
        // Sun moves in a vertical arc; negative Y means above horizon for our lighting convention
        sun_direction = Vector3f(std::cos(angle), -std::sin(angle), 0.2f).normalized();
        float sun_elevation = std::clamp(-sun_direction.y, 0.0f, 1.0f); // 0 night, 1 noon
        // Stronger sun; dimmer night
        sun_intensity = 0.15f + 1.35f * sun_elevation;   // up to ~1.5
        ambient_intensity = 0.05f + 0.40f * sun_elevation;
        // Warmer sunlight curve
        sun_color = Vector3f(1.0f, 0.88f + 0.10f * sun_elevation, 0.70f + 0.20f * sun_elevation);
    } else {
        sun_direction = Vector3f(-0.3f, -0.7f, -0.2f).normalized();
        sun_color = Vector3f(1.0f, 0.95f, 0.8f);
        sun_intensity = 1.0f;
        ambient_intensity = 0.3f;
    }
    float gamma = 2.2f;
    bool enable_fog = true;
    // Fog color shifts with time of day
    float sun_elevation_for_fog = std::clamp(-sun_direction.y, 0.0f, 1.0f);
    Vector3f fog_day = Vector3f(0.7f, 0.8f, 1.0f);
    Vector3f fog_night = Vector3f(0.1f, 0.12f, 0.2f);
    Vector3f fog_color = fog_night * (1.0f - sun_elevation_for_fog) + fog_day * sun_elevation_for_fog;
    float fog_density = 0.02f;
    float fog_start = 100.0f;
    float fog_end = config_.max_render_distance;
    
    shader->SetUniform("u_sun_direction", sun_direction);
    shader->SetUniform("u_sun_color", sun_color);
    shader->SetUniform("u_sun_intensity", sun_intensity);
    shader->SetUniform("u_ambient_color", ambient_color);
    shader->SetUniform("u_ambient_intensity", ambient_intensity);
    shader->SetUniform("u_gamma", gamma);
    shader->SetUniform("u_enable_fog", enable_fog);
    shader->SetUniform("u_fog_color", fog_color);
    shader->SetUniform("u_fog_density", fog_density);
    shader->SetUniform("u_fog_start", fog_start);
    shader->SetUniform("u_fog_end", fog_end);
    
    // Set per-scene uniforms (once per frame)
    shader->SetUniform("u_use_texture_arrays", true);
    shader->SetUniform("u_texture_blend_factor", 1.0f);
    shader->SetUniform("u_texture_array", 0);  // Texture unit 0
    shader->SetUniform("u_enable_normal_mapping", false);
    shader->SetUniform("u_normal_strength", 1.0f);
    shader->SetUniform("u_material_roughness", 0.8f);
    shader->SetUniform("u_material_metallic", 0.0f);
    shader->SetUniform("u_material_emission", 0.0f);
    shader->SetUniform("u_material_emission_color", Vector3f(1.0f, 1.0f, 1.0f));
    shader->SetUniform("u_texture_scale", 1.0f);
    shader->SetUniform("u_time", 0.0f);
    shader->SetUniform("u_enable_lighting", true);
    shader->SetUniform("u_enable_shadows", false);
    shader->SetUniform("u_wireframe_mode", false);  // Disable wireframe mode - use solid rendering
    shader->SetUniform("u_show_wireframe", false);  // Disable wireframe overlay
    shader->SetUniform("u_show_normals", false);
    shader->SetUniform("u_show_ao", false);
    shader->SetUniform("u_show_light_levels", false);
    shader->SetUniform("u_wireframe_color", Vector3f(1.0f, 1.0f, 0.0f)); // Bright yellow wireframe

    // Upload per-slot pos/far once per frame
    for (int s = 0; s < MAX_POINT_SHADOW_SLOTS; ++s) {
        shader->SetUniform("u_point_shadow_pos_slot[" + std::to_string(s) + "]", point_shadow_pos_slot_[s]);
        shader->SetUniform("u_point_shadow_far_slot[" + std::to_string(s) + "]", point_shadow_far_slot_[s]);
    }
    
    // Debug: Log wireframe mode
#if PVG_VOXEL_DEBUG_LOGS
    if (first_frame_debug) {
        std::cout << "Shader wireframe mode: DISABLED (solid rendering)" << std::endl;
    }
#endif
    
    // Bind voxel texture array (before drawing chunks)
    if (texture_array_) {
        texture_array_->Bind(0);
    }
    // Bind shadow map (unit 1)
    if (shadow_depth_tex_ != 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadow_depth_tex_);
        glActiveTexture(GL_TEXTURE0);
    }
    // Bind point shadow cubes (units 2..)
    for (int s = 0; s < MAX_POINT_SHADOW_SLOTS; ++s) {
        if (point_shadow_depth_cubes_[s] != 0) {
            glActiveTexture(GL_TEXTURE2 + s);
            glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_depth_cubes_[s]);
        }
    }
    glActiveTexture(GL_TEXTURE0);

    // Render each chunk
    static int debug_chunk_prints = 0;
    for (ChunkRenderData* render_data : chunks) {
        if (!render_data->mesh) continue;
        
        // Set model matrix for chunk position
        // Build model matrix with correct row-major translation
        Matrix4f model_matrix = Matrix4f::Translation(
            render_data->world_position.x,
            render_data->world_position.y,
            render_data->world_position.z);
        
        if (debug_chunk_prints < 5) {
            std::cout << "Rendering chunk at world pos: (" << render_data->world_position.x << ", "
                      << render_data->world_position.y << ", " << render_data->world_position.z << ")" << std::endl;
            std::cout << "  Model matrix translation: [0,3]="  << model_matrix.at(0,3)
                      << ", [1,3]=" << model_matrix.at(1,3)
                      << ", [2,3]=" << model_matrix.at(2,3) << std::endl;
            
            // Debug: Calculate where a test vertex should end up in NDC space
            // Let's test a vertex at the center of this chunk
            Vector3f test_vertex = render_data->world_position + Vector3f(8.0f, 8.0f, 8.0f); // Center of 16x16x16 chunk
            
            // Transform to clip space
            Matrix4f mvp = camera_matrices.view_projection_matrix * model_matrix;
            const float* mvp_data = mvp.data.data();
            
            // Manual matrix-vector multiply for row-major storage
            float clip_x = mvp_data[0]*test_vertex.x + mvp_data[1]*test_vertex.y + mvp_data[2]*test_vertex.z + mvp_data[3];
            float clip_y = mvp_data[4]*test_vertex.x + mvp_data[5]*test_vertex.y + mvp_data[6]*test_vertex.z + mvp_data[7];
            float clip_z = mvp_data[8]*test_vertex.x + mvp_data[9]*test_vertex.y + mvp_data[10]*test_vertex.z + mvp_data[11];
            float clip_w = mvp_data[12]*test_vertex.x + mvp_data[13]*test_vertex.y + mvp_data[14]*test_vertex.z + mvp_data[15];
            
            // NDC coords
            float ndc_x = clip_w != 0.0f ? clip_x / clip_w : clip_x;
            float ndc_y = clip_w != 0.0f ? clip_y / clip_w : clip_y;
            float ndc_z = clip_w != 0.0f ? clip_z / clip_w : clip_z;
            
            std::cout << "  Test vertex world pos: (" << test_vertex.x << ", " << test_vertex.y << ", " << test_vertex.z << ")" << std::endl;
            std::cout << "  Clip coords: (" << clip_x << ", " << clip_y << ", " << clip_z << ", " << clip_w << ")" << std::endl;
            std::cout << "  NDC coords: (" << ndc_x << ", " << ndc_y << ", " << ndc_z << ")" << std::endl;
            if (ndc_x >= -1.0f && ndc_x <= 1.0f && ndc_y >= -1.0f && ndc_y <= 1.0f && ndc_z >= -1.0f && ndc_z <= 1.0f) {
                std::cout << "  -> VERTEX SHOULD BE VISIBLE" << std::endl;
            } else {
                std::cout << "  -> VERTEX IS OUTSIDE VIEW FRUSTUM" << std::endl;
            }
            
            debug_chunk_prints++;
        }
        
        // Use cached per-chunk lights (compute if stale)
        if (render_data->lights_last_frame != current_frame_) {
            ComputeChunkLights(*render_data);
        }
        int budget = render_data->cached_light_count;
        shader->SetUniform("u_num_point_lights", budget);
        for (int i = 0; i < budget; ++i) {
            int li = render_data->cached_light_indices[i];
            const auto& L = frame_point_lights_[li];
            shader->SetUniform("u_point_light_pos[" + std::to_string(i) + "]", L.pos);
            shader->SetUniform("u_point_light_color[" + std::to_string(i) + "]", L.color);
            shader->SetUniform("u_point_light_intensity[" + std::to_string(i) + "]", L.intensity);
            shader->SetUniform("u_point_light_radius[" + std::to_string(i) + "]", L.radius);
            shader->SetUniform("u_light_shadow_slot[" + std::to_string(i) + "]", render_data->cached_light_shadow_slot[i]);
        }

        shader->SetUniform("u_model_matrix", model_matrix);
        
        // Set per-chunk uniforms
        shader->SetUniform("u_voxel_type", 1);  // Default to stone type

    // Shadow uniforms (per-frame)
    shader->SetUniform("u_shadow_matrix", shadow_matrix_);
    shader->SetUniform("u_shadow_map", 1);
    shader->SetUniform("u_shadow_bias", 0.0015f);
    shader->SetUniform("u_shadow_texel", Vector2f(1.0f / shadow_map_size_, 1.0f / shadow_map_size_));

    // Point shadow uniforms
    for (int s = 0; s < MAX_POINT_SHADOW_SLOTS; ++s) {
        shader->SetUniform("u_point_shadow_map" + std::to_string(s), 2 + s);
    }
    shader->SetUniform("u_point_shadow_bias", 0.05f);

    // Bind and draw mesh
        if (debug_chunk_prints < 5) {
            std::cout << "  Binding and drawing mesh..." << std::endl;
        }
        render_data->mesh->Bind();
        
        // Check for OpenGL errors before draw
        GLenum error = glGetError();
        if (error != GL_NO_ERROR && debug_chunk_prints < 5) {
            std::cout << "  OpenGL error before draw: " << error << std::endl;
        }
        
        render_data->mesh->Draw();
        
        // Check for OpenGL errors after draw
        error = glGetError();
        if (error != GL_NO_ERROR && debug_chunk_prints < 5) {
            std::cout << "  OpenGL error after draw: " << error << std::endl;
        }
        
        if (debug_chunk_prints < 5 && debug_chunk_prints >= 0) {
            std::cout << "  Mesh draw complete" << std::endl;
        }
        
        stats_.draw_calls++;
        // TODO: Update vertex/index counts from mesh data
    }
    
    stats_.rendered_chunks = chunks.size();
    
}

void VoxelRenderer::SetupRenderState([[maybe_unused]] const Camera& camera) {
    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
#if PVG_VOXEL_DEBUG_LOGS
    std::cout << "SetupRenderState: Depth testing ENABLED" << std::endl;
#endif
    
    // TEMPORARILY disable face culling to check if geometry is being culled incorrectly
    glDisable(GL_CULL_FACE);
    
    // Ensure OpenGL front face is set to counter-clockwise (default)
    glFrontFace(GL_CCW);
    
    // Debug: Check current OpenGL state
#if PVG_VOXEL_DEBUG_LOGS
    GLboolean culling_enabled = glIsEnabled(GL_CULL_FACE);
    GLint front_face;
    glGetIntegerv(GL_FRONT_FACE, &front_face);
    std::cout << "SetupRenderState: Face culling " << (culling_enabled ? "ENABLED" : "DISABLED") << " for debugging" << std::endl;
    std::cout << "SetupRenderState: Front face = " << (front_face == GL_CCW ? "GL_CCW" : "GL_CW") << std::endl;
#endif
    
    // Disable blending for solid voxels (enable for transparent ones later)
    glDisable(GL_BLEND);
    
    // Enable polygon offset to avoid z-fighting
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
    
    // Ensure proper viewport and scissor settings
    // Note: Viewport should be set by the main renderer, but we'll ensure it's not clipped
    glDisable(GL_SCISSOR_TEST);
    
    // Debug: Print viewport information
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
#if PVG_VOXEL_DEBUG_LOGS
    std::cout << "SetupRenderState: Viewport = (" << viewport[0] << ", " << viewport[1] 
              << ", " << viewport[2] << ", " << viewport[3] << ")" << std::endl;
#endif
    
    // Store previous OpenGL state if needed for restoration
    // (For now, we'll just set what we need)
}

void VoxelRenderer::RenderPointShadowMap(const Camera& camera) {
    auto* prog = shader_manager_.GetShaderProgram("point_shadow");
    if (!prog) {
        shader_manager_.LoadShaderProgram("point_shadow", "voxel_point_shadow.vert", "voxel_point_shadow.frag");
        prog = shader_manager_.GetShaderProgram("point_shadow");
    }
    if (!prog || !prog->IsValid()) {
        return;
    }

    // Select up to MAX_POINT_SHADOW_SLOTS most influential lights (closest to camera)
    struct Candidate { int index; float score; };
    std::vector<Candidate> candidates;
    candidates.reserve(frame_point_lights_.size());
    Vector3f camPos = camera.GetPosition();
    for (int i = 0; i < static_cast<int>(frame_point_lights_.size()); ++i) {
        float d = (frame_point_lights_[i].pos - camPos).length();
        float score = 1.0f / std::max(d, 0.1f);
        candidates.push_back({i, score});
    }
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b){ return a.score > b.score; });

    // Reset mapping
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) frame_light_shadow_slot_[i] = -1;

    int assigned = 0;
    for (const auto& c : candidates) {
        if (assigned >= MAX_POINT_SHADOW_SLOTS) break;
        int slot = assigned;
        frame_light_shadow_slot_[c.index] = slot;
        point_shadow_pos_slot_[slot] = frame_point_lights_[c.index].pos;
        point_shadow_far_slot_[slot] = std::max(frame_point_lights_[c.index].radius, 1.0f);
        assigned++;
    }

    if (assigned == 0) return;

    // Common cube transform
    Vector3f dirs[6] = {
        { 1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0,-1, 0}, {0, 0, 1}, {0, 0,-1}
    };
    Vector3f ups[6] = {
        {0,-1, 0}, {0,-1, 0}, {0, 0, 1}, {0, 0,-1}, {0,-1, 0}, {0,-1, 0}
    };

    // Save and set viewport
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    glViewport(0, 0, point_shadow_size_, point_shadow_size_);

    prog->Use();

    for (int s = 0; s < assigned; ++s) {
        // Update this slot only on its cadence to save GPU time
        if (point_shadow_update_divisor_ > 1) {
            if (((current_frame_ + s) % point_shadow_update_divisor_) != 0) continue;
        }
        float nearP = 0.1f;
        float farP = point_shadow_far_slot_[s];
        Matrix4f proj = Matrix4f::Perspective((float)M_PI/2.0f, 1.0f, nearP, farP);

        glBindFramebuffer(GL_FRAMEBUFFER, point_shadow_fbos_[s]);
        for (int face = 0; face < 6; ++face) {
            Matrix4f view = Matrix4f::LookAt(point_shadow_pos_slot_[s], point_shadow_pos_slot_[s] + dirs[face], ups[face]);
            Matrix4f vp = proj * view;
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, point_shadow_depth_cubes_[s], 0);
            glClearDepth(1.0);
            glClear(GL_DEPTH_BUFFER_BIT);

            prog->SetUniform("u_cube_view_proj", vp);

            for (auto& [key, render_data] : chunk_render_data_) {
                if (!render_data->mesh) continue;
                Matrix4f model = Matrix4f::Translation(render_data->world_position.x, render_data->world_position.y, render_data->world_position.z);
                prog->SetUniform("u_model_matrix", model);
                render_data->mesh->Bind();
                render_data->mesh->Draw();
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

void VoxelRenderer::RenderShadowMap(const Camera& camera) {
    if (shadow_fbo_ == 0 || shadow_depth_tex_ == 0) return;

    // Recompute sun direction (same as main pass)
    Vector3f sun_direction;
    if (config_.enable_day_night && config_.day_cycle_seconds > 0.0f) {
        float t = time_of_day_seconds_ / config_.day_cycle_seconds;
        float angle = t * 6.2831853f;
        sun_direction = Vector3f(std::cos(angle), -std::sin(angle), 0.2f).normalized();
    } else {
        sun_direction = Vector3f(-0.3f, -0.7f, -0.2f).normalized();
    }

    // Build orthographic light camera around the current camera position
    Vector3f center = camera.GetPosition();
    float extent = 80.0f; // covers visible area generously
    Matrix4f light_proj = Matrix4f::Orthographic(-extent, extent, -extent, extent, -100.0f, 200.0f);
    Vector3f light_pos = center - sun_direction * 60.0f;
    Matrix4f light_view = Matrix4f::LookAt(light_pos, center, Vector3f(0.0f, 1.0f, 0.0f));
    shadow_matrix_ = light_proj * light_view; // row-major; consistent with shader uploads

    // Setup FBO and viewport (preserve current viewport)
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    glViewport(0, 0, shadow_map_size_, shadow_map_size_);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Use shadow shader
    auto* shadow_prog = shader_manager_.GetShaderProgram(VoxelShaderManager::ShaderPreset::Shadow);
    if (!shadow_prog || !shadow_prog->IsValid()) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    shadow_prog->Use();
    shadow_prog->SetUniform("u_light_view_proj", shadow_matrix_);

    // Render chunks to depth
    for (auto& [key, render_data] : chunk_render_data_) {
        if (!render_data->mesh) continue;
        Matrix4f model = Matrix4f::Translation(
            render_data->world_position.x,
            render_data->world_position.y,
            render_data->world_position.z);
        shadow_prog->SetUniform("u_model_matrix", model);
        render_data->mesh->Bind();
        render_data->mesh->Draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Restore previous viewport
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

void VoxelRenderer::RenderSky(const Camera& camera) {
    auto* sky = shader_manager_.GetShaderProgram("sky");
    if (!sky || !sky->IsValid()) return;

    // Build same lighting params used for voxels
    Vector3f sun_direction;
    Vector3f sun_color;
    float sun_intensity;
    float ambient_intensity; // not used here directly

    if (config_.enable_day_night && config_.day_cycle_seconds > 0.0f) {
        float t = time_of_day_seconds_ / config_.day_cycle_seconds;
        float angle = t * 6.2831853f;
        sun_direction = Vector3f(std::cos(angle), -std::sin(angle), 0.2f).normalized();
        float sun_elevation = std::clamp(-sun_direction.y, 0.0f, 1.0f);
        sun_intensity = 0.15f + 1.35f * sun_elevation;
        sun_color = Vector3f(1.0f, 0.88f + 0.10f * sun_elevation, 0.70f + 0.20f * sun_elevation);
        ambient_intensity = 0.05f + 0.40f * sun_elevation;
    } else {
        sun_direction = Vector3f(-0.3f, -0.7f, -0.2f).normalized();
        sun_color = Vector3f(1.0f, 0.95f, 0.8f);
        sun_intensity = 1.0f;
        ambient_intensity = 0.3f;
    }

    // Set camera matrices
    sky->Use();
    sky->SetUniform("u_view_matrix", camera.GetViewMatrix());
    sky->SetUniform("u_projection_matrix", camera.GetProjectionMatrix());
    sky->SetUniform("u_sun_direction", sun_direction);
    sky->SetUniform("u_sun_color", sun_color);
    sky->SetUniform("u_sun_intensity", sun_intensity);
    sky->SetUniform("u_time", static_cast<float>(time_of_day_seconds_));
    // Provide camera-independent sun elevation to sky shader
    float sun_elevation_uniform = std::clamp(-sun_direction.y, 0.0f, 1.0f);
    sky->SetUniform("u_sun_elevation", sun_elevation_uniform);

    // Disable depth and draw full-screen triangle
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(sky_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void VoxelRenderer::CleanupRenderState() {
    // Unbind shader program
    glUseProgram(0);
    
    // Unbind VAO
    glBindVertexArray(0);
    
    // Unbind any textures (when we add texture support)
    // glBindTexture(GL_TEXTURE_2D, 0);
    
    // Restore default OpenGL state if needed
    // For now, leave depth testing enabled as it's commonly needed
    // glDisable(GL_DEPTH_TEST);
    // glDisable(GL_CULL_FACE);
    // glDisable(GL_POLYGON_OFFSET_FILL);
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
            
            // Create chunk with proper coordinates
            ChunkCoord2D chunk_coord(x, z);
            auto chunk = std::make_unique<Chunk>(chunk_coord);
            
            // Fill with test pattern
            int solid_voxels = 0;
            for (int cy = 0; cy < CHUNK_SIZE; ++cy) {
                for (int cz = 0; cz < CHUNK_SIZE; ++cz) {
                    for (int cx = 0; cx < CHUNK_SIZE; ++cx) {
                        VoxelType voxel = VoxelType::AIR;
                        
                        // Ground layer - stone
                        if (cy < 2) {
                            voxel = VoxelType::STONE;
                            solid_voxels++;
                        }
                        // Surface layer - grass
                        else if (cy == 2) {
                            voxel = VoxelType::GRASS;
                            solid_voxels++;
                        }
                        // Random trees
                        else if (cy > 2 && cy < 7 && 
                                 (((cx + x * CHUNK_SIZE) % 7 == 0 && (cz + z * CHUNK_SIZE) % 7 == 0))) {
                            // Tree trunk
                            if (cx % 2 == 0 && cz % 2 == 0) {
                                voxel = VoxelType::WOOD;
                                solid_voxels++;
                            }
                            // Leaves around the top
                            else if (cy > 4) {
                                voxel = VoxelType::LEAVES;
                                solid_voxels++;
                            }
                        }
                        // Random scattered dirt
                        else if (cy == 3 && ((cx + cz) % 5 == 0)) {
                            voxel = VoxelType::DIRT;
                            solid_voxels++;
                        }
                        
                        chunk->SetVoxel({cx, cy, cz}, voxel);
                    }
                }
            }
            
            // Debug: Print voxel count for first chunk
            if (x == 0 && z == 0) {
                std::cout << "  Chunk (0,0) has " << solid_voxels << " solid voxels out of " 
                          << (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) << " total" << std::endl;
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

void VoxelRenderer::ComputeChunkLights(ChunkRenderData& crd) {
    // Select nearest lights to chunk center and cache indices and slots
    Vector3f chunk_center = crd.world_position + Vector3f(CHUNK_SIZE * 0.5f, CHUNK_SIZE * 0.5f, CHUNK_SIZE * 0.5f);
    struct LightRef { int index; float dist2; };
    std::vector<LightRef> lr;
    lr.reserve(frame_point_lights_.size());
    for (int i = 0; i < static_cast<int>(frame_point_lights_.size()); ++i) {
        Vector3f d = frame_point_lights_[i].pos - chunk_center;
        lr.push_back({ i, d.lengthSquared() });
    }
    int budget = std::min<int>(MAX_LIGHTS_PER_CHUNK, static_cast<int>(lr.size()));
    if (budget > 0) {
        std::nth_element(lr.begin(), lr.begin() + budget, lr.end(), [](const LightRef&a, const LightRef&b){ return a.dist2 < b.dist2; });
    }
    crd.cached_light_count = budget;
    for (int i = 0; i < budget; ++i) {
        int li = lr[i].index;
        crd.cached_light_indices[i] = li;
        int slot = (li < MAX_POINT_LIGHTS) ? frame_light_shadow_slot_[li] : -1;
        crd.cached_light_shadow_slot[i] = slot;
    }
    crd.lights_last_frame = current_frame_;
}

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE
