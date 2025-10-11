#pragma once

#include "voxel_types.hpp"
#include "chunk.hpp"
#include <vector>
#include <array>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

/**
 * @brief Greedy meshing algorithm for efficient voxel rendering
 * 
 * Combines adjacent voxels of the same type into larger quads to reduce
 * the number of vertices and draw calls. Uses a greedy approach that
 * processes each face direction separately.
 */
class GreedyMesher {
public:
    /**
     * @brief Result of the meshing operation
     */
    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        size_t quad_count = 0;
        size_t face_count = 0;
    };

    /**
     * @brief Quad representation for meshing
     */
    struct Quad {
        ChunkCoord position;    // Bottom-left position of the quad
        uint8_t width;          // Width of the quad
        uint8_t height;         // Height of the quad
        VoxelType voxel_type;   // Type of voxel this quad represents
        Face face;              // Which face this quad belongs to
        uint8_t light_level;    // Lighting information
        
        Quad(ChunkCoord pos, uint8_t w, uint8_t h, VoxelType type, Face f, uint8_t light = 15)
            : position(pos), width(w), height(h), voxel_type(type), face(f), light_level(light) {}
    };

    /**
     * @brief Configuration for meshing algorithm
     */
    struct Config {
        bool enable_ambient_occlusion = true;  // Calculate AO for vertices
        bool enable_face_culling = true;       // Skip faces between solid voxels
        bool merge_same_textures = true;       // Only merge quads with same texture
        uint8_t max_quad_size = 16;            // Maximum size for a single quad
        bool generate_normals = true;          // Generate face normals
        bool generate_uvs = true;              // Generate texture coordinates
    };

    /**
     * @brief Default constructor with default configuration
     */
    GreedyMesher() = default;

    /**
     * @brief Constructor with custom configuration
     */
    explicit GreedyMesher(const Config& config) : config_(config) {}

    /**
     * @brief Generate mesh data for a single chunk
     * @param chunk The chunk to generate mesh for
     * @return Generated mesh data
     */
    MeshData GenerateMesh(const Chunk& chunk);

    /**
     * @brief Generate mesh data for chunk with neighbor information
     * @param chunk The center chunk to generate mesh for
     * @param neighbors Array of 6 neighbor chunks (can be null)
     * @return Generated mesh data
     */
    MeshData GenerateMeshWithNeighbors(const Chunk& chunk, 
                                      const std::array<const Chunk*, 6>& neighbors);

    /**
     * @brief Set meshing configuration
     */
    void SetConfig(const Config& config) { config_ = config; }

    /**
     * @brief Get current configuration
     */
    const Config& GetConfig() const { return config_; }

    /**
     * @brief Get statistics from last meshing operation
     */
    struct Stats {
        size_t voxels_processed = 0;
        size_t faces_generated = 0;
        size_t quads_generated = 0;
        size_t vertices_generated = 0;
        size_t indices_generated = 0;
        float compression_ratio = 0.0f;  // quads / potential_faces
        double meshing_time_ms = 0.0;
    };

    const Stats& GetLastStats() const { return last_stats_; }

private:
    /**
     * @brief Check if a voxel face should be rendered
     * @param chunk Current chunk
     * @param pos Position within chunk
     * @param face Face direction
     * @param neighbors Optional neighbor chunks
     * @return True if face should be rendered
     */
    bool ShouldRenderFace(const Chunk& chunk, 
                         ChunkCoord pos, 
                         Face face,
                         const std::array<const Chunk*, 6>* neighbors = nullptr) const;

    /**
     * @brief Get voxel at position, checking neighbors if needed
     * @param chunk Current chunk
     * @param pos Position (may be outside chunk bounds)
     * @param neighbors Optional neighbor chunks
     * @return Voxel at position, or AIR if outside bounds
     */
    VoxelType GetVoxelAt(const Chunk& chunk,
                        ChunkCoord pos,
                        const std::array<const Chunk*, 6>* neighbors = nullptr) const;

    /**
     * @brief Generate quads for a specific face direction
     * @param chunk The chunk to process
     * @param face Face direction to generate quads for
     * @param neighbors Optional neighbor chunks
     * @return Vector of generated quads
     */
    std::vector<Quad> GenerateQuadsForFace(const Chunk& chunk,
                                          Face face,
                                          const std::array<const Chunk*, 6>* neighbors = nullptr);

    /**
     * @brief Convert quads to vertex/index data
     * @param quads Vector of quads to convert
     * @return Mesh data with vertices and indices
     */
    MeshData QuadsToMesh(const std::vector<Quad>& quads,
                         const Chunk& chunk,
                         const std::array<const Chunk*, 6>* neighbors = nullptr);

    /**
     * @brief Generate vertices for a single quad
     * @param quad The quad to generate vertices for
     * @param chunk The chunk for AO sampling
     * @param neighbors Optional neighbor chunks for AO sampling
     * @return Array of 4 vertices
     */
    std::array<Vertex, 4> GenerateQuadVertices(const Quad& quad,
                                               const Chunk& chunk,
                                               const std::array<const Chunk*, 6>* neighbors = nullptr);

    /**
     * @brief Calculate ambient occlusion for a vertex
     * @param chunk The chunk
     * @param pos Vertex position
     * @param face Face direction
     * @param corner Which corner of the face (0-3)
     * @param neighbors Optional neighbor chunks
     * @return AO value (0.0 = fully occluded, 1.0 = not occluded)
     */
    float CalculateAmbientOcclusion(const Chunk& chunk,
                                   ChunkCoord pos,
                                   Face face,
                                   int corner,
                                   const std::array<const Chunk*, 6>* neighbors = nullptr) const;

    /**
     * @brief Get texture coordinates for a voxel face
     * @param voxel_type Type of voxel
     * @param face Face direction
     * @return UV coordinates for the face
     */
    std::array<Vector2f, 4> GetTextureCoordinates(VoxelType voxel_type, Face face) const;

    /**
     * @brief Check if two quads can be merged
     * @param quad1 First quad
     * @param quad2 Second quad
     * @return True if quads can be merged
     */
    bool CanMergeQuads(const Quad& quad1, const Quad& quad2) const;

    /**
     * @brief Merge two compatible quads
     * @param quad1 First quad (modified in place)
     * @param quad2 Second quad
     * @return True if merge was successful
     */
    bool MergeQuads(Quad& quad1, const Quad& quad2) const;

    /**
     * @brief Get the normal vector for a face
     * @param face Face direction
     * @return Normal vector
     */
    Vector3f GetFaceNormal(Face face) const;

    /**
     * @brief Get face direction offset vector
     * @param face Face direction
     * @return Offset vector for the face
     */
    ChunkCoord GetFaceOffset(Face face) const;

    /**
     * @brief Convert face enum to neighbor index
     * @param face Face direction
     * @return Index into neighbors array (0-5)
     */
    size_t FaceToNeighborIndex(Face face) const;

    // Public static face data for utility functions
    static const std::array<Vector3f, 6> FACE_NORMALS;
    static const std::array<ChunkCoord, 6> FACE_OFFSETS;
    static const std::array<std::array<ChunkCoord, 4>, 6> FACE_CORNERS;

private:
    Config config_;
    mutable Stats last_stats_;
};

/**
 * @brief Optimized meshing utilities
 */
namespace MeshingUtils {
    /**
     * @brief Calculate mesh complexity metrics
     */
    struct Complexity {
        size_t total_voxels;
        size_t solid_voxels;
        size_t exposed_faces;
        size_t potential_quads;
        float density;          // solid_voxels / total_voxels
        float exposure;         // exposed_faces / (solid_voxels * 6)
    };

    /**
     * @brief Analyze chunk complexity for meshing optimization
     */
    Complexity AnalyzeChunkComplexity(const Chunk& chunk);

    /**
     * @brief Estimate memory usage for mesh data
     */
    struct MemoryEstimate {
        size_t vertex_bytes;
        size_t index_bytes;
        size_t total_bytes;
    };

    MemoryEstimate EstimateMeshMemory(const Complexity& complexity);

    /**
     * @brief Batch process multiple chunks for meshing
     * @param chunks Vector of chunks to process
     * @param mesher Configured greedy mesher
     * @return Vector of mesh data (same order as input chunks)
     */
    std::vector<GreedyMesher::MeshData> BatchGenerateMeshes(
        const std::vector<const Chunk*>& chunks,
        GreedyMesher& mesher);

} // namespace MeshingUtils

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE