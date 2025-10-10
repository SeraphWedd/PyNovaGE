#pragma once

#include <cstdint>
#include <array>
#include <cmath>
#include <functional>
#include <vectors/vector2.hpp>
#include <vectors/vector3.hpp>
#include <vectors/vector4.hpp>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

// Voxel world configuration constants
constexpr int CHUNK_SIZE = 16;              // 16x16x16 voxels per chunk
constexpr int CHUNK_HEIGHT = 256;           // World height in blocks
constexpr int CHUNKS_PER_LAYER = CHUNK_HEIGHT / CHUNK_SIZE; // 16 chunks vertically
constexpr int MAX_TEXTURE_LAYERS = 256;     // Maximum texture array layers

// Voxel type enum for compatibility
enum class VoxelType : uint16_t {
    AIR = 0,
    STONE = 1,
    DIRT = 2,
    GRASS = 3,
    WOOD = 4,
    LEAVES = 5
};

// Legacy voxel ID aliases
using VoxelID = uint16_t;
constexpr VoxelID VOXEL_AIR = static_cast<VoxelID>(VoxelType::AIR);
constexpr VoxelID VOXEL_STONE = static_cast<VoxelID>(VoxelType::STONE);
constexpr VoxelID VOXEL_DIRT = static_cast<VoxelID>(VoxelType::DIRT);
constexpr VoxelID VOXEL_GRASS = static_cast<VoxelID>(VoxelType::GRASS);
constexpr VoxelID VOXEL_WOOD = static_cast<VoxelID>(VoxelType::WOOD);
constexpr VoxelID VOXEL_LEAVES = static_cast<VoxelID>(VoxelType::LEAVES);

/**
 * @brief Voxel data structure
 * Contains voxel type and optional metadata
 */
struct Voxel {
    VoxelID id = VOXEL_AIR;
    uint8_t metadata = 0;  // For rotation, damage, etc.
    
    inline bool IsAir() const { return id == VOXEL_AIR; }
    inline bool IsSolid() const { return id != VOXEL_AIR; }
};

// Coordinate types
using ChunkCoord = Vector3i;  // Using Vector3i for 3D chunk coordinates
using Vector2f = Vector2<float>;
using Vector4f = Vector4<float>;

/**
 * @brief Legacy chunk coordinates in world space (2D)
 */
struct ChunkCoord2D {
    int x, z;
    
    ChunkCoord2D() : x(0), z(0) {}
    ChunkCoord2D(int x_, int z_) : x(x_), z(z_) {}
    
    bool operator==(const ChunkCoord2D& other) const {
        return x == other.x && z == other.z;
    }
    
    bool operator!=(const ChunkCoord2D& other) const {
        return !(*this == other);
    }
    
    // Hash function for use in unordered containers
    struct Hash {
        std::size_t operator()(const ChunkCoord2D& coord) const {
            return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.z) << 1);
        }
    };
};

/**
 * @brief World coordinates to chunk coordinates conversion
 */
inline ChunkCoord2D WorldToChunk2D(int world_x, int world_z) {
    return ChunkCoord2D(world_x >> 4, world_z >> 4);  // Divide by 16
}

/**
 * @brief World coordinates to chunk coordinates (3D)
 */
inline ChunkCoord WorldToChunk(const Vector3f& world_pos) {
    return ChunkCoord(
        static_cast<int>(std::floor(world_pos.x / CHUNK_SIZE)), 
        static_cast<int>(std::floor(world_pos.y / CHUNK_SIZE)),
        static_cast<int>(std::floor(world_pos.z / CHUNK_SIZE))
    );
}

/**
 * @brief World coordinates to local chunk coordinates
 */
inline Vector3i WorldToLocal(int world_x, int world_y, int world_z) {
    return Vector3i(world_x & 15, world_y, world_z & 15);  // Modulo 16
}

/**
 * @brief Chunk coordinates to world coordinates
 */
inline Vector3i ChunkToWorld(const ChunkCoord2D& chunk, int local_x, int local_y, int local_z) {
    return Vector3i(
        chunk.x * CHUNK_SIZE + local_x,
        local_y,
        chunk.z * CHUNK_SIZE + local_z
    );
}

/**
 * @brief 3D Chunk coordinates to world coordinates
 */
inline Vector3f ChunkToWorld(const ChunkCoord& chunk) {
    return Vector3f(
        static_cast<float>(chunk.x * CHUNK_SIZE),
        static_cast<float>(chunk.y * CHUNK_SIZE),
        static_cast<float>(chunk.z * CHUNK_SIZE)
    );
}

/**
 * @brief Face directions for voxel rendering
 */
enum class VoxelFace {
    LEFT = 0,   // -X
    RIGHT = 1,  // +X
    BOTTOM = 2, // -Y
    TOP = 3,    // +Y
    BACK = 4,   // -Z
    FRONT = 5   // +Z
};

/**
 * @brief Face direction vectors
 */
constexpr std::array<Vector3i, 6> FACE_DIRECTIONS = {{
    Vector3i(-1, 0, 0),  // LEFT
    Vector3i(1, 0, 0),   // RIGHT
    Vector3i(0, -1, 0),  // BOTTOM
    Vector3i(0, 1, 0),   // TOP
    Vector3i(0, 0, -1),  // BACK
    Vector3i(0, 0, 1)    // FRONT
}};

/**
 * @brief Voxel vertex data for meshing
 */
struct VoxelVertex {
    Vector3f position;
    Vector3f normal;
    Vector2f texcoord;
    float texture_id;     // Texture array layer
    float ambient_occlusion; // AO factor (0.0-1.0)
};

// Additional types for meshing compatibility
using Face = VoxelFace;
using Vertex = VoxelVertex;

/**
 * @brief Voxel material properties
 */
struct VoxelMaterial {
    VoxelID id;
    std::array<uint16_t, 6> texture_ids; // One per face
    bool is_transparent = false;
    bool is_solid = true;
    float hardness = 1.0f;
    
    VoxelMaterial(VoxelID voxel_id = VOXEL_AIR) : id(voxel_id) {
        texture_ids.fill(0);
    }
};

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE