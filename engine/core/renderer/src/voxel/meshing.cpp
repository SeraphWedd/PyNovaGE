#include "renderer/voxel/meshing.hpp"
#include <vectors/vector2.hpp>
#include <chrono>
#include <algorithm>
#include <iostream>

#ifndef PVG_VOXEL_DEBUG_LOGS
#define PVG_VOXEL_DEBUG_LOGS 0
#endif

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

// Static face data initialization
const std::array<Vector3f, 6> GreedyMesher::FACE_NORMALS = {{
    {-1.0f,  0.0f,  0.0f},  // LEFT
    { 1.0f,  0.0f,  0.0f},  // RIGHT  
    { 0.0f, -1.0f,  0.0f},  // BOTTOM
    { 0.0f,  1.0f,  0.0f},  // TOP
    { 0.0f,  0.0f, -1.0f},  // BACK
    { 0.0f,  0.0f,  1.0f}   // FRONT
}};

const std::array<ChunkCoord, 6> GreedyMesher::FACE_OFFSETS = {{
    {-1,  0,  0},  // LEFT
    { 1,  0,  0},  // RIGHT
    { 0, -1,  0},  // BOTTOM
    { 0,  1,  0},  // TOP
    { 0,  0, -1},  // BACK
    { 0,  0,  1}   // FRONT
}};

const std::array<std::array<ChunkCoord, 4>, 6> GreedyMesher::FACE_CORNERS = {{
    // LEFT face (looking from inside cube)
    {{ {0, 0, 1}, {0, 1, 1}, {0, 1, 0}, {0, 0, 0} }},
    // RIGHT face
    {{ {0, 0, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1} }},
    // BOTTOM face
    {{ {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1} }},
    // TOP face
    {{ {0, 0, 1}, {1, 0, 1}, {1, 0, 0}, {0, 0, 0} }},
    // BACK face
    {{ {1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0} }},
    // FRONT face
    {{ {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0} }}
}};

GreedyMesher::MeshData GreedyMesher::GenerateMesh(const Chunk& chunk) {
    return GenerateMeshWithNeighbors(chunk, {});
}

GreedyMesher::MeshData GreedyMesher::GenerateMeshWithNeighbors(
    const Chunk& chunk, 
    const std::array<const Chunk*, 6>& neighbors) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Reset stats
    last_stats_ = Stats{};
    last_stats_.voxels_processed = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    
    std::vector<Quad> all_quads;
    
    // Debug: Count solid voxels
    int solid_count = 0;
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                if (chunk.GetVoxel(x, y, z) != VoxelType::AIR) {
                    solid_count++;
                }
            }
        }
    }
#if PVG_VOXEL_DEBUG_LOGS
    std::cout << "  Meshing chunk with " << solid_count << " solid voxels" << std::endl;
#endif
    
    // Generate quads for each face direction
    for (int face_idx = 0; face_idx < 6; ++face_idx) {
        Face face = static_cast<Face>(face_idx);
        auto face_quads = GenerateQuadsForFace(chunk, face, &neighbors);
#if PVG_VOXEL_DEBUG_LOGS
        std::cout << "    Face " << face_idx << " generated " << face_quads.size() << " quads" << std::endl;
#endif
        all_quads.insert(all_quads.end(), face_quads.begin(), face_quads.end());
    }
    
    // Convert quads to mesh data
    MeshData mesh_data = QuadsToMesh(all_quads, chunk, &neighbors);
    
    // Update stats
    last_stats_.quads_generated = all_quads.size();
    last_stats_.faces_generated = mesh_data.face_count;
    last_stats_.vertices_generated = mesh_data.vertices.size();
    last_stats_.indices_generated = mesh_data.indices.size();
    
    // Calculate compression ratio
    size_t potential_faces = last_stats_.voxels_processed * 6; // Max possible faces
    last_stats_.compression_ratio = potential_faces > 0 ? 
        static_cast<float>(last_stats_.quads_generated) / potential_faces : 0.0f;
    
    auto end_time = std::chrono::high_resolution_clock::now();
    last_stats_.meshing_time_ms = std::chrono::duration<double, std::milli>(
        end_time - start_time).count();
    
    return mesh_data;
}

bool GreedyMesher::ShouldRenderFace(const Chunk& chunk, 
                                   ChunkCoord pos, 
                                   Face face,
                                   const std::array<const Chunk*, 6>* neighbors) const {
    // Current voxel must be solid
    VoxelType current_voxel = chunk.GetVoxel(pos);
    if (current_voxel == VoxelType::AIR) {
        return false;
    }
    
    if (!config_.enable_face_culling) {
        return true;
    }
    
    // Check adjacent voxel
    ChunkCoord adjacent_pos = pos + GetFaceOffset(face);
    VoxelType adjacent_voxel = GetVoxelAt(chunk, adjacent_pos, neighbors);
    
    // Render face if adjacent voxel is air or transparent
    return adjacent_voxel == VoxelType::AIR;
    // TODO: Add transparency support when more voxel types are implemented
}

VoxelType GreedyMesher::GetVoxelAt(const Chunk& chunk,
                                  ChunkCoord pos,
                                  const std::array<const Chunk*, 6>* neighbors) const {
    // Check if position is within current chunk
    if (pos.x >= 0 && pos.x < CHUNK_SIZE &&
        pos.y >= 0 && pos.y < CHUNK_SIZE &&
        pos.z >= 0 && pos.z < CHUNK_SIZE) {
        return chunk.GetVoxel(pos);
    }
    
    // Position is outside chunk, check neighbors if available
    if (!neighbors) {
        return VoxelType::AIR; // Assume air if no neighbor info
    }
    
    // Determine which neighbor chunk to check
    ChunkCoord neighbor_pos = pos;
    const Chunk* neighbor_chunk = nullptr;
    
    if (pos.x < 0) {
        neighbor_chunk = (*neighbors)[static_cast<size_t>(Face::LEFT)];
        neighbor_pos.x = CHUNK_SIZE - 1;
    } else if (pos.x >= CHUNK_SIZE) {
        neighbor_chunk = (*neighbors)[static_cast<size_t>(Face::RIGHT)];
        neighbor_pos.x = 0;
    } else if (pos.y < 0) {
        neighbor_chunk = (*neighbors)[static_cast<size_t>(Face::BOTTOM)];
        neighbor_pos.y = CHUNK_SIZE - 1;
    } else if (pos.y >= CHUNK_SIZE) {
        neighbor_chunk = (*neighbors)[static_cast<size_t>(Face::TOP)];
        neighbor_pos.y = 0;
    } else if (pos.z < 0) {
        neighbor_chunk = (*neighbors)[static_cast<size_t>(Face::BACK)];
        neighbor_pos.z = CHUNK_SIZE - 1;
    } else if (pos.z >= CHUNK_SIZE) {
        neighbor_chunk = (*neighbors)[static_cast<size_t>(Face::FRONT)];
        neighbor_pos.z = 0;
    }
    
    return neighbor_chunk ? neighbor_chunk->GetVoxel(neighbor_pos) : VoxelType::AIR;
}

std::vector<GreedyMesher::Quad> GreedyMesher::GenerateQuadsForFace(
    const Chunk& chunk,
    Face face,
    const std::array<const Chunk*, 6>* neighbors) {
    
    std::vector<Quad> quads;
    
    // Create a 2D mask for the face slice
    bool mask[CHUNK_SIZE][CHUNK_SIZE];
    VoxelType voxel_mask[CHUNK_SIZE][CHUNK_SIZE];
    
    // Determine axis and slice direction
    int axis1, axis2, axis3; // axis3 is the normal direction
    bool reverse = false;
    
    switch (face) {
        case Face::LEFT:
        case Face::RIGHT:
            axis1 = 1; axis2 = 2; axis3 = 0; // Y-Z plane
            reverse = (face == Face::LEFT);
            break;
        case Face::BOTTOM:
        case Face::TOP:
            axis1 = 0; axis2 = 2; axis3 = 1; // X-Z plane  
            reverse = (face == Face::BOTTOM);
            break;
        case Face::BACK:
        case Face::FRONT:
            axis1 = 0; axis2 = 1; axis3 = 2; // X-Y plane
            reverse = (face == Face::BACK);
            break;
        default:
            // Should never happen with valid Face enum values
            axis1 = 0; axis2 = 1; axis3 = 2;
            reverse = false;
            break;
    }
    
    // Process each slice along the normal axis
    for (int d = 0; d < CHUNK_SIZE; ++d) {
        int slice_pos = reverse ? (CHUNK_SIZE - 1 - d) : d;
        
        // Clear the mask
        std::fill_n(&mask[0][0], CHUNK_SIZE * CHUNK_SIZE, false);
        std::fill_n(&voxel_mask[0][0], CHUNK_SIZE * CHUNK_SIZE, VoxelType::AIR);
        
        // Fill the mask for this slice
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            for (int i = 0; i < CHUNK_SIZE; ++i) {
                // Convert 2D slice coordinates to 3D chunk coordinates
                ChunkCoord pos;
                pos.data[axis1] = i;
                pos.data[axis2] = j;
                pos.data[axis3] = slice_pos;
                
                if (ShouldRenderFace(chunk, pos, face, neighbors)) {
                    mask[j][i] = true;
                    voxel_mask[j][i] = chunk.GetVoxel(pos);
                }
            }
        }
        
        // Generate quads from the mask using greedy algorithm
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            for (int i = 0; i < CHUNK_SIZE;) {
                if (!mask[j][i]) {
                    ++i;
                    continue;
                }
                
                VoxelType quad_voxel = voxel_mask[j][i];
                
                // Find width (extend in i direction)
                int width = 1;
                while (i + width < CHUNK_SIZE && 
                       mask[j][i + width] && 
                       voxel_mask[j][i + width] == quad_voxel &&
                       width < config_.max_quad_size) {
                    ++width;
                }
                
                // Find height (extend in j direction)
                int height = 1;
                bool can_extend_height = true;
                
                while (j + height < CHUNK_SIZE && 
                       can_extend_height && 
                       height < config_.max_quad_size) {
                    
                    // Check if entire width can be extended
                    for (int k = 0; k < width; ++k) {
                        if (!mask[j + height][i + k] || 
                            voxel_mask[j + height][i + k] != quad_voxel) {
                            can_extend_height = false;
                            break;
                        }
                    }
                    
                    if (can_extend_height) {
                        ++height;
                    }
                }
                
                // Create the quad
                ChunkCoord quad_pos;
                quad_pos.data[axis1] = i;
                quad_pos.data[axis2] = j;
                quad_pos.data[axis3] = slice_pos;
                
                quads.emplace_back(quad_pos, static_cast<uint8_t>(width), static_cast<uint8_t>(height), quad_voxel, face);
                
                // Clear the mask for the area covered by this quad
                for (int h = 0; h < height; ++h) {
                    for (int w = 0; w < width; ++w) {
                        mask[j + h][i + w] = false;
                    }
                }
                
                i += width;
            }
        }
    }
    
    return quads;
}

GreedyMesher::MeshData GreedyMesher::QuadsToMesh(const std::vector<Quad>& quads,
                                                const Chunk& chunk,
                                                const std::array<const Chunk*, 6>* neighbors) {
    MeshData mesh_data;
    mesh_data.quad_count = quads.size();
    mesh_data.face_count = quads.size();
    
    // Reserve space for vertices and indices
    mesh_data.vertices.reserve(quads.size() * 4);
    mesh_data.indices.reserve(quads.size() * 6);
    
    uint32_t vertex_offset = 0;
    
    for (const auto& quad : quads) {
        // Generate 4 vertices for the quad
        auto quad_vertices = GenerateQuadVertices(quad, chunk, neighbors);
        
        // Add vertices to mesh
        for (const auto& vertex : quad_vertices) {
            mesh_data.vertices.push_back(vertex);
        }
        
        // Add indices (two triangles per quad)
        if (config_.ao_flip_triangles) {
            // Choose diagonal with higher summed AO to reduce visible seams
            float d02 = quad_vertices[0].ambient_occlusion + quad_vertices[2].ambient_occlusion;
            float d13 = quad_vertices[1].ambient_occlusion + quad_vertices[3].ambient_occlusion;
            if (d02 >= d13) {
                // Diagonal 0-2
                mesh_data.indices.insert(mesh_data.indices.end(), {
                    vertex_offset + 0, vertex_offset + 2, vertex_offset + 1,
                    vertex_offset + 0, vertex_offset + 3, vertex_offset + 2
                });
            } else {
                // Diagonal 1-3
                mesh_data.indices.insert(mesh_data.indices.end(), {
                    vertex_offset + 0, vertex_offset + 1, vertex_offset + 3,
                    vertex_offset + 3, vertex_offset + 1, vertex_offset + 2
                });
            }
        } else {
            // Default: fixed diagonal 0-2
            mesh_data.indices.insert(mesh_data.indices.end(), {
                vertex_offset + 0, vertex_offset + 2, vertex_offset + 1,
                vertex_offset + 0, vertex_offset + 3, vertex_offset + 2
            });
        }
        
        vertex_offset += 4;
    }
    
    return mesh_data;
}

std::array<Vertex, 4> GreedyMesher::GenerateQuadVertices(const Quad& quad,
                                                       const Chunk& chunk,
                                                       const std::array<const Chunk*, 6>* neighbors) {
    std::array<Vertex, 4> vertices;

    Vector3f face_normal = GetFaceNormal(quad.face);
    auto tex_coords = GetTextureCoordinates(quad.voxel_type, quad.face);

    // Compute four unique corners per face using a clean axis-based approach.
    // width extends along the first in-plane axis, height along the second in-plane axis.
    const float x = static_cast<float>(quad.position.x);
    const float y = static_cast<float>(quad.position.y);
    const float z = static_cast<float>(quad.position.z);
    const float w = static_cast<float>(quad.width);
    const float h = static_cast<float>(quad.height);

    std::array<Vector3f, 4> world_corners;

    switch (quad.face) {
        case Face::LEFT:  // -X, plane at x (width along Y, height along Z)
            world_corners = {
                Vector3f(x,     y,     z),
                Vector3f(x,     y+w,   z),
                Vector3f(x,     y+w,   z+h),
                Vector3f(x,     y,     z+h)
            };
            break;
        case Face::RIGHT: // +X, plane at x+1 (width along Y, height along Z)
            world_corners = {
                Vector3f(x+1.0f, y,     z),
                Vector3f(x+1.0f, y+w,   z),
                Vector3f(x+1.0f, y+w,   z+h),
                Vector3f(x+1.0f, y,     z+h)
            };
            break;
        case Face::BOTTOM: // -Y, plane at y
            world_corners = {
                Vector3f(x,     y, z),
                Vector3f(x+w,   y, z),
                Vector3f(x+w,   y, z+h),
                Vector3f(x,     y, z+h)
            };
            break;
        case Face::TOP: // +Y, plane at y+1
            world_corners = {
                Vector3f(x,     y+1.0f, z),
                Vector3f(x+w,   y+1.0f, z),
                Vector3f(x+w,   y+1.0f, z+h),
                Vector3f(x,     y+1.0f, z+h)
            };
            break;
        case Face::BACK: // -Z, plane at z
            world_corners = {
                Vector3f(x,     y,     z),
                Vector3f(x+w,   y,     z),
                Vector3f(x+w,   y+h,   z),
                Vector3f(x,     y+h,   z)
            };
            break;
        case Face::FRONT: // +Z, plane at z+1
            world_corners = {
                Vector3f(x,     y,     z+1.0f),
                Vector3f(x+w,   y,     z+1.0f),
                Vector3f(x+w,   y+h,   z+1.0f),
                Vector3f(x,     y+h,   z+1.0f)
            };
            break;
    }

    // Fill vertex data
    // Map voxel type (1..5) to texture array layer index (0..4)
    int layer_index = static_cast<int>(quad.voxel_type) - 1;
    if (layer_index < 0) layer_index = 0;

    for (int i = 0; i < 4; ++i) {
        vertices[i].position = world_corners[i];
        vertices[i].normal = config_.generate_normals ? face_normal : Vector3f(0, 1, 0);
        vertices[i].texcoord = config_.generate_uvs ? tex_coords[i] : Vector2f(0, 0);
        // Store texture array layer index in texture_id channel
        vertices[i].texture_id = static_cast<float>(layer_index);
        // Calculate ambient occlusion for each corner
        vertices[i].ambient_occlusion = CalculateAmbientOcclusion(
            chunk, quad.position, quad.face, i, neighbors);
    }

    return vertices;
}

float GreedyMesher::CalculateAmbientOcclusion(const Chunk& chunk,
                                           ChunkCoord pos,
                                           Face face,
                                           int corner,
                                           const std::array<const Chunk*, 6>* neighbors) const {
    if (!config_.enable_ambient_occlusion) {
        return 1.0f;
    }

    // Get face normal and corner delta vectors
    Vector3f normal = GetFaceNormal(face);
    ChunkCoord corner_pos = pos + FACE_CORNERS[static_cast<size_t>(face)][corner];

    // Get two perpendicular vectors along the face
    Vector3f tangent, bitangent;
    if (face == Face::TOP || face == Face::BOTTOM) {
        tangent = Vector3f(1, 0, 0);
        bitangent = Vector3f(0, 0, 1);
    } else if (face == Face::LEFT || face == Face::RIGHT) {
        tangent = Vector3f(0, 1, 0);
        bitangent = Vector3f(0, 0, 1);
    } else { // FRONT/BACK
        tangent = Vector3f(1, 0, 0);
        bitangent = Vector3f(0, 1, 0);
    }

    // Sample points around the corner
    ChunkCoord side1 = corner_pos + ChunkCoord(static_cast<int>(tangent.x),
                                              static_cast<int>(tangent.y),
                                              static_cast<int>(tangent.z));
    ChunkCoord side2 = corner_pos + ChunkCoord(static_cast<int>(bitangent.x),
                                              static_cast<int>(bitangent.y),
                                              static_cast<int>(bitangent.z));
    ChunkCoord diagonal = corner_pos + ChunkCoord(static_cast<int>(tangent.x + bitangent.x),
                                                 static_cast<int>(tangent.y + bitangent.y),
                                                 static_cast<int>(tangent.z + bitangent.z));

    // Check if sample points are solid
    bool side1_solid = GetVoxelAt(chunk, side1, neighbors) != VoxelType::AIR;
    bool side2_solid = GetVoxelAt(chunk, side2, neighbors) != VoxelType::AIR;
    bool diagonal_solid = GetVoxelAt(chunk, diagonal, neighbors) != VoxelType::AIR;

    // Calculate AO value based on number of solid neighbors
    float ao_value = 1.0f;

    if (side1_solid && side2_solid) {
        ao_value = 0.25f; // Maximum occlusion when both sides are blocked
    } else {
        int solid_count = (side1_solid ? 1 : 0) + (side2_solid ? 1 : 0) + (diagonal_solid ? 1 : 0);
        switch (solid_count) {
            case 3: ao_value = 0.25f; break;  // All three points solid
            case 2: ao_value = 0.5f; break;   // Two points solid
            case 1: ao_value = 0.75f; break;  // One point solid
            default: ao_value = 1.0f;         // No solid neighbors
        }
    }

    // Apply AO strength mix
    float s = std::clamp(config_.ao_strength, 0.0f, 1.0f);
    return 1.0f + (ao_value - 1.0f) * s;
}

std::array<Vector2f, 4> GreedyMesher::GetTextureCoordinates([[maybe_unused]] VoxelType voxel_type, [[maybe_unused]] Face face) const {
    // For now, return simple UV coordinates
    // TODO: Implement texture atlas mapping based on voxel type and face
    return {{
        {0.0f, 0.0f},  // Bottom-left
        {1.0f, 0.0f},  // Bottom-right  
        {1.0f, 1.0f},  // Top-right
        {0.0f, 1.0f}   // Top-left
    }};
}

bool GreedyMesher::CanMergeQuads(const Quad& quad1, const Quad& quad2) const {
    // Check if quads can be merged (same face, voxel type, and adjacent)
    return quad1.face == quad2.face && 
           quad1.voxel_type == quad2.voxel_type &&
           quad1.light_level == quad2.light_level;
}

bool GreedyMesher::MergeQuads([[maybe_unused]] Quad& quad1, [[maybe_unused]] const Quad& quad2) const {
    // TODO: Implement quad merging logic
    return false;
}

Vector3f GreedyMesher::GetFaceNormal(Face face) const {
    return FACE_NORMALS[static_cast<size_t>(face)];
}

ChunkCoord GreedyMesher::GetFaceOffset(Face face) const {
    return FACE_OFFSETS[static_cast<size_t>(face)];
}

size_t GreedyMesher::FaceToNeighborIndex(Face face) const {
    return static_cast<size_t>(face);
}

// MeshingUtils implementations
namespace MeshingUtils {

Complexity AnalyzeChunkComplexity(const Chunk& chunk) {
    Complexity complexity{};
    complexity.total_voxels = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                ChunkCoord pos{x, y, z};
                VoxelType voxel = chunk.GetVoxel(pos);
                
                if (voxel != VoxelType::AIR) {
                    complexity.solid_voxels++;
                    
                    // Count exposed faces
                    for (int face_idx = 0; face_idx < 6; ++face_idx) {
                        // Use static array directly since it's in the same translation unit
                        static const std::array<ChunkCoord, 6> face_offsets = {{
                            {-1,  0,  0},  // LEFT
                            { 1,  0,  0},  // RIGHT
                            { 0, -1,  0},  // BOTTOM
                            { 0,  1,  0},  // TOP
                            { 0,  0, -1},  // BACK
                            { 0,  0,  1}   // FRONT
                        }};
                        ChunkCoord neighbor_pos = pos + face_offsets[face_idx];
                        VoxelType neighbor = VoxelType::AIR;
                        
                        if (neighbor_pos.x >= 0 && neighbor_pos.x < CHUNK_SIZE &&
                            neighbor_pos.y >= 0 && neighbor_pos.y < CHUNK_SIZE &&
                            neighbor_pos.z >= 0 && neighbor_pos.z < CHUNK_SIZE) {
                            neighbor = chunk.GetVoxel(neighbor_pos);
                        }
                        
                        if (neighbor == VoxelType::AIR) {
                            complexity.exposed_faces++;
                        }
                    }
                }
            }
        }
    }
    
    complexity.potential_quads = complexity.exposed_faces;
    complexity.density = static_cast<float>(complexity.solid_voxels) / complexity.total_voxels;
    complexity.exposure = complexity.solid_voxels > 0 ? 
        static_cast<float>(complexity.exposed_faces) / (complexity.solid_voxels * 6.0f) : 0.0f;
    
    return complexity;
}

MemoryEstimate EstimateMeshMemory(const Complexity& complexity) {
    MemoryEstimate estimate{};
    
    // Estimate based on exposed faces (worst case: each face becomes a quad)
    size_t estimated_vertices = complexity.exposed_faces * 4;
    size_t estimated_indices = complexity.exposed_faces * 6;
    
    estimate.vertex_bytes = estimated_vertices * sizeof(Vertex);
    estimate.index_bytes = estimated_indices * sizeof(uint32_t);
    estimate.total_bytes = estimate.vertex_bytes + estimate.index_bytes;
    
    return estimate;
}

std::vector<GreedyMesher::MeshData> BatchGenerateMeshes(
    const std::vector<const Chunk*>& chunks,
    GreedyMesher& mesher) {
    
    std::vector<GreedyMesher::MeshData> results;
    results.reserve(chunks.size());
    
    for (const auto* chunk : chunks) {
        if (chunk) {
            results.push_back(mesher.GenerateMesh(*chunk));
        } else {
            results.emplace_back(); // Empty mesh data for null chunks
        }
    }
    
    return results;
}

} // namespace MeshingUtils

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE