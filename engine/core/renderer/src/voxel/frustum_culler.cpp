#include "renderer/voxel/frustum_culler.hpp"
#include <chrono>
#include <algorithm>
#include <cmath>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

CullingResult FrustumCuller::CullChunks(std::vector<ChunkCullInfo>& chunks) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Reset results
    last_results_ = CullingResult{};
    last_results_.total_chunks = chunks.size();
    
    if (chunks.empty()) {
        return last_results_;
    }
    
    // Calculate distances for all chunks
    for (auto& chunk_info : chunks) {
        Vector3f chunk_center = chunk_info.world_bounds.GetCenter();
        Vector3f to_chunk = chunk_center - camera_position_;
        chunk_info.distance_to_camera = to_chunk.length();
    }
    
    // Perform distance culling first (fastest)
    if (config_.enable_distance_culling) {
        PerformDistanceCulling(chunks);
    }
    
    // Perform frustum culling
    if (config_.enable_frustum_culling) {
        PerformFrustumCulling(chunks);
    }
    
    // Sort by distance for optimal rendering order
    if (config_.sort_by_distance) {
        SortChunksByDistance(chunks);
    }
    
    // Count visible chunks
    for (const auto& chunk_info : chunks) {
        if (chunk_info.is_visible) {
            last_results_.visible_chunks++;
        }
    }
    
    last_results_.culled_chunks = last_results_.total_chunks - last_results_.visible_chunks;
    last_results_.culling_ratio = last_results_.total_chunks > 0 ? 
        static_cast<float>(last_results_.culled_chunks) / last_results_.total_chunks : 0.0f;
    
    auto end_time = std::chrono::high_resolution_clock::now();
    last_results_.culling_time_ms = std::chrono::duration<double, std::milli>(
        end_time - start_time).count();
    
    return last_results_;
}

std::vector<std::pair<const Chunk*, Vector3f>> FrustumCuller::CullChunks(
    const std::vector<std::pair<const Chunk*, Vector3f>>& chunk_positions) {
    
    // Convert to ChunkCullInfo format
    std::vector<ChunkCullInfo> cull_infos;
    cull_infos.reserve(chunk_positions.size());
    
    for (const auto& [chunk, position] : chunk_positions) {
        cull_infos.emplace_back(chunk, position);
    }
    
    // Perform culling
    CullChunks(cull_infos);
    
    // Extract visible chunks
    std::vector<std::pair<const Chunk*, Vector3f>> visible_chunks;
    for (const auto& cull_info : cull_infos) {
        if (cull_info.is_visible) {
            visible_chunks.emplace_back(cull_info.chunk, cull_info.world_position);
        }
    }
    
    return visible_chunks;
}

bool FrustumCuller::IsChunkVisible(const AABB& chunk_world_bounds, float distance_to_camera) const {
    // Distance culling
    if (config_.enable_distance_culling && distance_to_camera > config_.max_render_distance) {
        return false;
    }
    
    // Frustum culling
    if (config_.enable_frustum_culling) {
        // Apply culling margin by expanding the AABB slightly
        AABB expanded_bounds = chunk_world_bounds;
        Vector3f margin(config_.culling_margin);
        expanded_bounds.min -= margin;
        expanded_bounds.max += margin;
        
        return frustum_.IntersectsAABB(expanded_bounds);
    }
    
    return true;
}

void FrustumCuller::PerformDistanceCulling(std::vector<ChunkCullInfo>& chunks) const {
    float max_distance_squared = config_.max_render_distance * config_.max_render_distance;
    
    for (auto& chunk_info : chunks) {
        if (!chunk_info.is_visible) continue; // Already culled
        
        float distance_squared = chunk_info.distance_to_camera * chunk_info.distance_to_camera;
        if (distance_squared > max_distance_squared) {
            chunk_info.is_visible = false;
        }
    }
}

void FrustumCuller::PerformFrustumCulling(std::vector<ChunkCullInfo>& chunks) const {
    for (auto& chunk_info : chunks) {
        if (!chunk_info.is_visible) continue; // Already culled
        
        // Apply culling margin by expanding the AABB slightly
        AABB expanded_bounds = chunk_info.world_bounds;
        Vector3f margin(config_.culling_margin);
        expanded_bounds.min -= margin;
        expanded_bounds.max += margin;
        
        if (!frustum_.IntersectsAABB(expanded_bounds)) {
            chunk_info.is_visible = false;
        }
    }
}

void FrustumCuller::SortChunksByDistance(std::vector<ChunkCullInfo>& chunks) const {
    // Sort visible chunks by distance (front to back for early Z rejection)
    if (config_.enable_early_z_rejection) {
        std::sort(chunks.begin(), chunks.end(), 
            [](const ChunkCullInfo& a, const ChunkCullInfo& b) {
                // Sort visible chunks first, then by distance
                if (a.is_visible != b.is_visible) {
                    return a.is_visible > b.is_visible; // Visible chunks first
                }
                return a.distance_to_camera < b.distance_to_camera; // Closer first
            });
    } else {
        // Sort back to front for transparency/blending
        std::sort(chunks.begin(), chunks.end(), 
            [](const ChunkCullInfo& a, const ChunkCullInfo& b) {
                if (a.is_visible != b.is_visible) {
                    return a.is_visible > b.is_visible;
                }
                return a.distance_to_camera > b.distance_to_camera; // Farther first
            });
    }
}

float FrustumCuller::CalculateDistanceSquared(const AABB& aabb) const {
    Vector3f chunk_center = aabb.GetCenter();
    Vector3f to_chunk = chunk_center - camera_position_;
    return to_chunk.lengthSquared();
}

// CullingUtils implementations
namespace CullingUtils {

void OctreeNode::Insert(ChunkCullInfo* chunk, int max_depth, int current_depth) {
    // Check if chunk fits in this node
    if (!bounds.Contains(chunk->world_bounds.GetCenter())) {
        return; // Chunk doesn't belong in this node
    }
    
    // If we're at max depth or this is a leaf with few chunks, add here
    if (current_depth >= max_depth || (is_leaf && chunks.size() < 8)) {
        chunks.push_back(chunk);
        return;
    }
    
    // Split node if needed
    if (is_leaf) {
        is_leaf = false;
        
        // Create 8 child nodes
        Vector3f center = bounds.GetCenter();
        Vector3f size = bounds.GetSize() * 0.5f;
        
        for (int i = 0; i < 8; ++i) {
            Vector3f child_min = bounds.min;
            Vector3f child_max = center;
            
            if (i & 1) { child_min.x = center.x; child_max.x = bounds.max.x; }
            if (i & 2) { child_min.y = center.y; child_max.y = bounds.max.y; }
            if (i & 4) { child_min.z = center.z; child_max.z = bounds.max.z; }
            
            children[i] = std::make_unique<OctreeNode>(AABB(child_min, child_max));
        }
        
        // Redistribute existing chunks to children
        auto existing_chunks = std::move(chunks);
        chunks.clear();
        
        for (auto* existing_chunk : existing_chunks) {
            Insert(existing_chunk, max_depth, current_depth);
        }
    }
    
    // Insert chunk into appropriate child
    for (auto& child : children) {
        if (child && child->bounds.Contains(chunk->world_bounds.GetCenter())) {
            child->Insert(chunk, max_depth, current_depth + 1);
            return;
        }
    }
    
    // If no child can contain it, keep it at this level
    chunks.push_back(chunk);
}

void OctreeNode::CullChunks(const FrustumCuller& culler, std::vector<ChunkCullInfo*>& visible_chunks) {
    // Test this node's bounds against the frustum
    if (!culler.IsAABBVisible(bounds)) {
        return; // Entire node is outside frustum
    }
    
    // Test chunks at this level
    for (auto* chunk : chunks) {
        if (chunk && culler.IsChunkVisible(chunk->world_bounds, chunk->distance_to_camera)) {
            visible_chunks.push_back(chunk);
        }
    }
    
    // Recurse into children
    if (!is_leaf) {
        for (auto& child : children) {
            if (child) {
                child->CullChunks(culler, visible_chunks);
            }
        }
    }
}

std::unique_ptr<OctreeNode> BuildOctree(
    std::vector<ChunkCullInfo>& chunks, 
    const AABB& world_bounds) {
    
    auto root = std::make_unique<OctreeNode>(world_bounds);
    
    // Insert all chunks into the octree
    for (auto& chunk : chunks) {
        root->Insert(&chunk);
    }
    
    return root;
}

size_t EstimateCullingMemoryUsage(size_t chunk_count) {
    // Estimate memory usage for culling data structures
    size_t chunk_cull_info_size = sizeof(ChunkCullInfo) * chunk_count;
    
    // Octree nodes (rough estimate: 1 node per 8 chunks on average)
    size_t estimated_octree_nodes = chunk_count / 8 + 1;
    size_t octree_memory = sizeof(OctreeNode) * estimated_octree_nodes;
    
    // Frustum and other fixed structures
    size_t fixed_overhead = sizeof(FrustumCuller) + sizeof(Frustum) + 1024; // Extra padding
    
    return chunk_cull_info_size + octree_memory + fixed_overhead;
}

} // namespace CullingUtils

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE