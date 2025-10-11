#pragma once

#include "camera.hpp"
#include "chunk.hpp"
#include <vectors/vector3.hpp>
#include <vectors/vector4.hpp>
#include <matrices/matrix4.hpp>
#include <array>
#include <cmath>
#include <algorithm>
#include <unordered_set>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

/**
 * @brief Axis-aligned bounding box for culling calculations
 */
struct AABB {
    Vector3f min;    // Minimum corner
    Vector3f max;    // Maximum corner

    /**
     * @brief Default constructor (empty AABB)
     */
    AABB() : min(0.0f), max(0.0f) {}

    /**
     * @brief Construct AABB from min/max points
     */
    AABB(const Vector3f& min_point, const Vector3f& max_point) 
        : min(min_point), max(max_point) {}

    /**
     * @brief Get center point of AABB
     */
    Vector3f GetCenter() const { return (min + max) * 0.5f; }

    /**
     * @brief Get size (extent) of AABB
     */
    Vector3f GetSize() const { return max - min; }

    /**
     * @brief Get radius of bounding sphere
     */
    float GetRadius() const { return GetSize().length() * 0.5f; }

    /**
     * @brief Check if point is inside AABB
     */
    bool Contains(const Vector3f& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    /**
     * @brief Check if this AABB intersects with another
     */
    bool Intersects(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }

    /**
     * @brief Expand AABB to include a point
     */
    void ExpandToInclude(const Vector3f& point) {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    /**
     * @brief Expand AABB to include another AABB
     */
    void ExpandToInclude(const AABB& other) {
        ExpandToInclude(other.min);
        ExpandToInclude(other.max);
    }

    /**
     * @brief Transform AABB by a matrix
     */
    AABB Transform(const Matrix4f& transform) const {
        // Transform all 8 corners and find new min/max
        std::array<Vector3f, 8> corners = {
            Vector3f(min.x, min.y, min.z),
            Vector3f(max.x, min.y, min.z),
            Vector3f(min.x, max.y, min.z),
            Vector3f(max.x, max.y, min.z),
            Vector3f(min.x, min.y, max.z),
            Vector3f(max.x, min.y, max.z),
            Vector3f(min.x, max.y, max.z),
            Vector3f(max.x, max.y, max.z)
        };

        AABB result;
        bool first = true;

        for (const auto& corner : corners) {
            Vector4f transformed_corner = transform * Vector4f(corner, 1.0f);
            Vector3f world_corner(transformed_corner.x, transformed_corner.y, transformed_corner.z);

            if (first) {
                result.min = result.max = world_corner;
                first = false;
            } else {
                result.ExpandToInclude(world_corner);
            }
        }

        return result;
    }
};

/**
 * @brief Frustum culling planes extracted from view-projection matrix
 */
struct Frustum {
    std::array<Vector4f, 6> planes;  // Left, Right, Bottom, Top, Near, Far

    /**
     * @brief Plane indices for clarity
     */
    enum PlaneIndex {
        LEFT = 0,
        RIGHT = 1,
        BOTTOM = 2,
        TOP = 3,
        NEAR = 4,
        FAR = 5
    };

    /**
     * @brief Default constructor
     */
    Frustum() = default;

    /**
     * @brief Extract frustum planes from view-projection matrix
     * @param view_projection_matrix Combined view-projection matrix
     */
    void ExtractPlanes(const Matrix4f& view_projection_matrix) {
        // NOTE: Matrix4f is stored in row-major order. Extract planes using row-wise indices.
        // Rows: r0=[m0..m3], r1=[m4..m7], r2=[m8..m11], r3=[m12..m15]
        const float* m = view_projection_matrix.data.data();

        // Left plane = row3 + row0
        {
            Vector4f p(
                m[12] + m[0],
                m[13] + m[4],
                m[14] + m[8],
                m[15] + m[3]
            );
            float nlen = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            if (nlen > 0.0f) { p.x /= nlen; p.y /= nlen; p.z /= nlen; p.w /= nlen; }
            planes[LEFT] = p;
        }

        // Right plane = row3 - row0
        {
            Vector4f p(
                m[12] - m[0],
                m[13] - m[4],
                m[14] - m[8],
                m[15] - m[3]
            );
            float nlen = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            if (nlen > 0.0f) { p.x /= nlen; p.y /= nlen; p.z /= nlen; p.w /= nlen; }
            planes[RIGHT] = p;
        }

        // Bottom plane = row3 + row1
        {
            Vector4f p(
                m[12] + m[4],
                m[13] + m[5],
                m[14] + m[6],
                m[15] + m[7]
            );
            float nlen = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            if (nlen > 0.0f) { p.x /= nlen; p.y /= nlen; p.z /= nlen; p.w /= nlen; }
            planes[BOTTOM] = p;
        }

        // Top plane = row3 - row1
        {
            Vector4f p(
                m[12] - m[4],
                m[13] - m[5],
                m[14] - m[6],
                m[15] - m[7]
            );
            float nlen = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            if (nlen > 0.0f) { p.x /= nlen; p.y /= nlen; p.z /= nlen; p.w /= nlen; }
            planes[TOP] = p;
        }

        // Near plane = row3 + row2
        {
            Vector4f p(
                m[12] + m[8],
                m[13] + m[9],
                m[14] + m[10],
                m[15] + m[11]
            );
            float nlen = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            if (nlen > 0.0f) { p.x /= nlen; p.y /= nlen; p.z /= nlen; p.w /= nlen; }
            planes[NEAR] = p;
        }

        // Far plane = row3 - row2
        {
            Vector4f p(
                m[12] - m[8],
                m[13] - m[9],
                m[14] - m[10],
                m[15] - m[11]
            );
            float nlen = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            if (nlen > 0.0f) { p.x /= nlen; p.y /= nlen; p.z /= nlen; p.w /= nlen; }
            planes[FAR] = p;
        }
    }

    /**
     * @brief Test if point is inside frustum
     * @param point Point to test
     * @return True if point is inside frustum
     */
    bool ContainsPoint(const Vector3f& point) const {
        for (const auto& plane : planes) {
            float distance = plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w;
            if (distance < 0.0f) {
                return false;  // Point is outside this plane
            }
        }
        return true;
    }

    /**
     * @brief Test if sphere is inside or intersecting frustum
     * @param center Sphere center
     * @param radius Sphere radius
     * @return True if sphere intersects frustum
     */
    bool IntersectsSphere(const Vector3f& center, float radius) const {
        for (const auto& plane : planes) {
            float distance = plane.x * center.x + plane.y * center.y + plane.z * center.z + plane.w;
            if (distance < -radius) {
                return false;  // Sphere is completely outside this plane
            }
        }
        return true;
    }

    /**
     * @brief Test if AABB is inside or intersecting frustum
     * @param aabb AABB to test
     * @return True if AABB intersects frustum
     */
    bool IntersectsAABB(const AABB& aabb) const {
        // Robust plane-AABB test using both positive and negative support vertices
        // See: Akenine-Moller, Real-Time Rendering
        constexpr float epsilon = 1e-4f;
        for (const auto& plane : planes) {
            // Positive vertex (farthest along plane normal)
            Vector3f p;
            p.x = (plane.x >= 0.0f) ? aabb.max.x : aabb.min.x;
            p.y = (plane.y >= 0.0f) ? aabb.max.y : aabb.min.y;
            p.z = (plane.z >= 0.0f) ? aabb.max.z : aabb.min.z;

            float d_p = plane.x * p.x + plane.y * p.y + plane.z * p.z + plane.w;
            if (d_p < -epsilon) {
                // Entire box is outside this plane
                return false;
            }

            // Negative vertex (closest along plane normal)
            Vector3f n;
            n.x = (plane.x >= 0.0f) ? aabb.min.x : aabb.max.x;
            n.y = (plane.y >= 0.0f) ? aabb.min.y : aabb.max.y;
            n.z = (plane.z >= 0.0f) ? aabb.min.z : aabb.max.z;

            // Compute d_n (not strictly needed beyond documentation); keep to aid future classification
            // float d_n = plane.x * n.x + plane.y * n.y + plane.z * n.z + plane.w;
            // If d_n < -epsilon the box intersects the plane but is not fully outside,
            // so we continue testing other planes.
        }
        return true; // Not outside any plane
    }
};

/**
 * @brief Frustum culling result information
 */
struct CullingResult {
    size_t total_chunks = 0;        // Total chunks tested
    size_t visible_chunks = 0;      // Chunks that passed culling
    size_t culled_chunks = 0;       // Chunks that were culled
    double culling_time_ms = 0.0;   // Time spent culling (milliseconds)
    float culling_ratio = 0.0f;     // Ratio of culled to total chunks
};

/**
 * @brief Chunk culling information for efficient processing
 */
struct ChunkCullInfo {
    const Chunk* chunk;             // Pointer to chunk
    AABB world_bounds;              // World-space bounding box
    Vector3f world_position;        // World position
    float distance_to_camera;       // Distance from camera (for sorting)
    bool is_visible;                // Culling result

    ChunkCullInfo(const Chunk* c, const Vector3f& world_pos) 
        : chunk(c), world_position(world_pos), distance_to_camera(0.0f), is_visible(true) {
        
        // Calculate world-space AABB for chunk
        Vector3f chunk_min = world_pos;
        Vector3f chunk_max = world_pos + Vector3f(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);
        world_bounds = AABB(chunk_min, chunk_max);
    }
};

/**
 * @brief Advanced frustum culler with multiple culling techniques
 */
class FrustumCuller {
public:
    /**
     * @brief Culling configuration options
     */
    struct Config {
        bool enable_frustum_culling = true;      // Enable frustum culling
        bool enable_distance_culling = true;     // Enable distance-based culling
        bool enable_occlusion_culling = false;   // Enable occlusion culling (future)
        float max_render_distance = 500.0f;      // Maximum render distance
        float lod_distance_thresholds[4] = {50.0f, 100.0f, 200.0f, 400.0f}; // LOD distances
        bool enable_early_z_rejection = true;    // Use early Z testing
        bool sort_by_distance = true;            // Sort chunks by distance for rendering
        float culling_margin = 2.0f;             // Extra margin for frustum culling (expanded to avoid corner pop)
        float frustum_near_bias = 2.0f;          // Shift near plane backwards by this many world units
        float frustum_guard_band = 16.0f;        // Expand all frustum planes outward by this many world units
    };

    /**
     * @brief Constructor with default configuration
     */
    FrustumCuller() = default;

    /**
     * @brief Constructor with custom configuration
     */
    explicit FrustumCuller(const Config& config) : config_(config) {}

    /**
     * @brief Set culling configuration
     */
    void SetConfig(const Config& config) { config_ = config; }

    /**
     * @brief Get current configuration
     */
    const Config& GetConfig() const { return config_; }

    /**
     * @brief Update culler with current camera state
     * @param camera Current camera
     */
    void UpdateCamera(const Camera& camera) {
        camera_position_ = camera.GetPosition();
        view_projection_matrix_ = camera.GetViewProjectionMatrix();
        frustum_.ExtractPlanes(view_projection_matrix_);
        camera_forward_ = camera.GetForward();
        // Apply requested near-plane bias (shift near plane backwards to avoid popping large chunks near camera)
        frustum_.planes[Frustum::NEAR].w += config_.frustum_near_bias;
        // Apply a uniform world-space guard band to all planes to prevent near-edge popping with large chunks
        for (auto& plane : frustum_.planes) {
            plane.w += config_.frustum_guard_band;
        }
    }

    /**
     * @brief Perform frustum culling on a list of chunks
     * @param chunks List of chunk cull info to test
     * @return Culling results and statistics
     */
    CullingResult CullChunks(std::vector<ChunkCullInfo>& chunks);

    /**
     * @brief Perform frustum culling on chunks with world positions
     * @param chunk_positions Vector of (chunk, world_position) pairs
     * @return Vector of visible chunks with their world positions
     */
    std::vector<std::pair<const Chunk*, Vector3f>> CullChunks(
        const std::vector<std::pair<const Chunk*, Vector3f>>& chunk_positions);

    /**
     * @brief Test if a single chunk is visible
     * @param chunk_world_bounds World-space AABB of chunk
     * @param distance_to_camera Distance from camera to chunk center
     * @return True if chunk should be rendered
     */
    bool IsChunkVisible(const AABB& chunk_world_bounds, float distance_to_camera) const;

    /**
     * @brief Test if a point is visible in the frustum
     * @param point World-space point
     * @return True if point is visible
     */
    bool IsPointVisible(const Vector3f& point) const {
        return frustum_.ContainsPoint(point);
    }

    /**
     * @brief Test if a sphere is visible in the frustum
     * @param center Sphere center
     * @param radius Sphere radius
     * @return True if sphere is visible
     */
    bool IsSphereVisible(const Vector3f& center, float radius) const {
        return frustum_.IntersectsSphere(center, radius);
    }

    /**
     * @brief Test if an AABB is visible in the frustum
     * @param aabb World-space AABB
     * @return True if AABB is visible
     */
    bool IsAABBVisible(const AABB& aabb) const {
        return frustum_.IntersectsAABB(aabb);
    }

    /**
     * @brief Calculate level of detail for a chunk based on distance
     * @param distance Distance to chunk
     * @return LOD level (0 = highest detail, higher = lower detail)
     */
    int CalculateLOD(float distance) const {
        for (int i = 0; i < 4; ++i) {
            if (distance < config_.lod_distance_thresholds[i]) {
                return i;
            }
        }
        return 4; // Lowest detail
    }

    /**
     * @brief Get last culling results
     */
    const CullingResult& GetLastResults() const { return last_results_; }

    /**
     * @brief Get current frustum
     */
    const Frustum& GetFrustum() const { return frustum_; }

    /**
     * @brief Get current camera position
     */
    const Vector3f& GetCameraPosition() const { return camera_position_; }

private:
    /**
     * @brief Perform distance-based culling
     * @param chunks Chunks to test
     */
    void PerformDistanceCulling(std::vector<ChunkCullInfo>& chunks) const;

    /**
     * @brief Perform frustum culling
     * @param chunks Chunks to test
     */
    void PerformFrustumCulling(std::vector<ChunkCullInfo>& chunks) const;

    /**
     * @brief Sort chunks by distance for optimal rendering
     * @param chunks Chunks to sort
     */
    void SortChunksByDistance(std::vector<ChunkCullInfo>& chunks) const;

    /**
     * @brief Calculate squared distance from camera to AABB
     * @param aabb AABB to test
     * @return Squared distance
     */
    float CalculateDistanceSquared(const AABB& aabb) const;

private:
    Config config_;
    Frustum frustum_;
    Vector3f camera_position_;
    Vector3f camera_forward_;
    Matrix4f view_projection_matrix_;
    CullingResult last_results_;
};

/**
 * @brief Hierarchical culling utilities for large worlds
 */
namespace CullingUtils {
    /**
     * @brief Octree node for hierarchical culling
     */
    struct OctreeNode {
        AABB bounds;
        std::array<std::unique_ptr<OctreeNode>, 8> children;
        std::vector<ChunkCullInfo*> chunks;
        bool is_leaf = true;
        
        OctreeNode(const AABB& node_bounds) : bounds(node_bounds) {}
        
        /**
         * @brief Insert chunk into octree
         */
        void Insert(ChunkCullInfo* chunk, int max_depth = 5, int current_depth = 0);
        
        /**
         * @brief Cull chunks in this node and children
         */
        void CullChunks(const FrustumCuller& culler, std::vector<ChunkCullInfo*>& visible_chunks);
    };

    /**
     * @brief Build octree from chunk list
     * @param chunks List of chunks to organize
     * @param world_bounds Overall world bounds
     * @return Root octree node
     */
    std::unique_ptr<OctreeNode> BuildOctree(
        std::vector<ChunkCullInfo>& chunks, 
        const AABB& world_bounds);

    /**
     * @brief Estimate memory usage for culling data structures
     * @param chunk_count Number of chunks
     * @return Estimated memory usage in bytes
     */
    size_t EstimateCullingMemoryUsage(size_t chunk_count);

} // namespace CullingUtils

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE