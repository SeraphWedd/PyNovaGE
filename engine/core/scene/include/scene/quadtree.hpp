#pragma once

#include <vectors/vectors.hpp>
#include <matrices/matrices.hpp>
#include "scene/entity.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_set>

namespace PyNovaGE {
namespace Scene {

// Type aliases for convenience
using Vector2f = PyNovaGE::Vector2<float>;
using Matrix3f = PyNovaGE::Matrix3<float>;

/**
 * @brief AABB (Axis-Aligned Bounding Box) for spatial partitioning
 * 
 * Simple 2D bounding box used by the quadtree system.
 */
struct AABB2D {
    Vector2f min{0.0f, 0.0f};
    Vector2f max{0.0f, 0.0f};

    AABB2D() = default;
    AABB2D(const Vector2f& min_point, const Vector2f& max_point) : min(min_point), max(max_point) {}
    AABB2D(float x, float y, float width, float height) 
        : min(x, y), max(x + width, y + height) {}

    // Properties
    Vector2f GetCenter() const { return (min + max) * 0.5f; }
    Vector2f GetSize() const { return max - min; }
    float GetWidth() const { return max.x - min.x; }
    float GetHeight() const { return max.y - min.y; }
    float GetArea() const { return GetWidth() * GetHeight(); }

    // Spatial queries
    bool Contains(const Vector2f& point) const;
    bool Contains(const AABB2D& other) const;
    bool Intersects(const AABB2D& other) const;
    bool Intersects(const Vector2f& center, float radius) const; // Circle intersection
    
    // Operations
    void Expand(const Vector2f& point);
    void Expand(const AABB2D& other);
    AABB2D Union(const AABB2D& other) const;
    AABB2D Intersection(const AABB2D& other) const;
    
    // Subdivision (for quadtree)
    std::array<AABB2D, 4> Subdivide() const;
    
    bool IsValid() const { return min.x <= max.x && min.y <= max.y; }
    void Reset() { min = max = Vector2f(0.0f, 0.0f); }
};

/**
 * @brief Spatial object for quadtree storage
 * 
 * Represents an object in the spatial partitioning system.
 * Associates an entity with its spatial bounds.
 */
struct SpatialObject {
    EntityID entity;
    AABB2D bounds;
    void* user_data = nullptr; // Optional user data pointer

    SpatialObject() = default;
    SpatialObject(EntityID id, const AABB2D& aabb, void* data = nullptr) 
        : entity(id), bounds(aabb), user_data(data) {}

    bool IsValid() const { return entity.IsValid() && bounds.IsValid(); }
};

/**
 * @brief 2D Quadtree for spatial partitioning
 * 
 * Hierarchical spatial data structure for efficient culling and spatial queries.
 * Supports dynamic insertion/removal and various query types.
 */
class Quadtree {
public:
    static constexpr size_t MAX_OBJECTS_PER_NODE = 8;
    static constexpr size_t MAX_DEPTH = 8;

    /**
     * @brief Constructor
     * @param bounds The spatial bounds of this quadtree
     * @param max_objects Maximum objects per node before subdivision
     * @param max_depth Maximum tree depth
     */
    explicit Quadtree(const AABB2D& bounds, 
                     size_t max_objects = MAX_OBJECTS_PER_NODE,
                     size_t max_depth = MAX_DEPTH);

    /**
     * @brief Destructor
     */
    ~Quadtree() = default;

    // Object management
    void Insert(const SpatialObject& object);
    void Insert(EntityID entity, const AABB2D& bounds, void* user_data = nullptr);
    bool Remove(EntityID entity);
    bool Update(EntityID entity, const AABB2D& new_bounds);
    void Clear();

    // Spatial queries
    std::vector<SpatialObject> QueryPoint(const Vector2f& point) const;
    std::vector<SpatialObject> QueryAABB(const AABB2D& aabb) const;
    std::vector<SpatialObject> QueryCircle(const Vector2f& center, float radius) const;
    std::vector<SpatialObject> QueryFrustum(const std::vector<Vector2f>& frustum_points) const;

    // Callback-based queries (more efficient for large result sets)
    using QueryCallback = std::function<void(const SpatialObject&)>;
    void QueryPoint(const Vector2f& point, const QueryCallback& callback) const;
    void QueryAABB(const AABB2D& aabb, const QueryCallback& callback) const;
    void QueryCircle(const Vector2f& center, float radius, const QueryCallback& callback) const;

    // Raycasting
    struct RayHit {
        SpatialObject object;
        Vector2f hit_point;
        float distance;
    };
    std::vector<RayHit> Raycast(const Vector2f& origin, const Vector2f& direction, float max_distance = std::numeric_limits<float>::infinity()) const;
    bool RaycastFirst(const Vector2f& origin, const Vector2f& direction, RayHit& hit, float max_distance = std::numeric_limits<float>::infinity()) const;

    // Statistics and debugging
    size_t GetObjectCount() const;
    size_t GetNodeCount() const;
    size_t GetMaxDepth() const;
    void GetStatistics(size_t& total_objects, size_t& total_nodes, size_t& max_depth) const;
    
    // For rebuilding purposes
    const std::vector<SpatialObject>& GetObjectsInNode() const { return objects_; }
    
    const AABB2D& GetBounds() const { return bounds_; }
    bool IsEmpty() const { return objects_.empty() && children_[0] == nullptr; }
    
    // Debug visualization
    void GetAllBounds(std::vector<AABB2D>& node_bounds) const;
    void VisitNodes(const std::function<void(const AABB2D&, size_t, const std::vector<SpatialObject>&)>& visitor) const;

private:
    AABB2D bounds_;
    size_t max_objects_;
    size_t max_depth_;
    size_t depth_;
    
    std::vector<SpatialObject> objects_;
    std::array<std::unique_ptr<Quadtree>, 4> children_;
    
    // Internal methods
    void Subdivide();
    void Merge();
    bool ShouldSubdivide() const;
    bool ShouldMerge() const;
    int GetChildIndex(const AABB2D& bounds) const;
    void GetChildBounds(std::array<AABB2D, 4>& child_bounds) const;
    
    // Query helpers
    void QueryPointRecursive(const Vector2f& point, std::vector<SpatialObject>& results) const;
    void QueryAABBRecursive(const AABB2D& aabb, std::vector<SpatialObject>& results) const;
    void QueryCircleRecursive(const Vector2f& center, float radius, std::vector<SpatialObject>& results) const;
    
    void QueryPointRecursive(const Vector2f& point, const QueryCallback& callback) const;
    void QueryAABBRecursive(const AABB2D& aabb, const QueryCallback& callback) const;
    void QueryCircleRecursive(const Vector2f& center, float radius, const QueryCallback& callback) const;
    
    void RaycastRecursive(const Vector2f& origin, const Vector2f& direction, float max_distance, std::vector<RayHit>& results) const;
    bool RayAABBIntersect(const Vector2f& origin, const Vector2f& direction, const AABB2D& aabb, float& t_min, float& t_max) const;
};

/**
 * @brief Spatial manager for scene-wide spatial queries
 * 
 * High-level interface for managing spatial partitioning in a scene.
 * Automatically manages quadtree updates and provides scene-level queries.
 */
class SpatialManager {
public:
    explicit SpatialManager(const AABB2D& world_bounds);

    // Object management
    void Insert(const SpatialObject& object);
    void Insert(EntityID entity, const AABB2D& bounds, void* user_data = nullptr);
    bool Remove(EntityID entity);
    bool Update(EntityID entity, const AABB2D& bounds);

    // Initialization and cleanup
    void Initialize();
    void Clear();

    // Object management
    void RegisterObject(EntityID entity, const AABB2D& bounds, void* user_data = nullptr);
    void UnregisterObject(EntityID entity);
    void UpdateObject(EntityID entity, const AABB2D& new_bounds);
    void ClearAll();

    // Automatic bounds expansion
    void SetAutoExpand(bool enabled) { auto_expand_ = enabled; }
    bool GetAutoExpand() const { return auto_expand_; }
    void ExpandWorldBounds(const AABB2D& bounds);

    // Spatial queries (delegates to quadtree)
    std::vector<SpatialObject> QueryPoint(const Vector2f& point) const { return quadtree_.QueryPoint(point); }
    std::vector<SpatialObject> QueryAABB(const AABB2D& aabb) const { return quadtree_.QueryAABB(aabb); }
    std::vector<SpatialObject> QueryCircle(const Vector2f& center, float radius) const { return quadtree_.QueryCircle(center, radius); }
    std::vector<Quadtree::RayHit> Raycast(const Vector2f& origin, const Vector2f& direction, float max_distance = std::numeric_limits<float>::infinity()) const {
        return quadtree_.Raycast(origin, direction, max_distance);
    }

    // Callback queries
    void QueryPoint(const Vector2f& point, const Quadtree::QueryCallback& callback) const { quadtree_.QueryPoint(point, callback); }
    void QueryAABB(const AABB2D& aabb, const Quadtree::QueryCallback& callback) const { quadtree_.QueryAABB(aabb, callback); }
    void QueryCircle(const Vector2f& center, float radius, const Quadtree::QueryCallback& callback) const { quadtree_.QueryCircle(center, radius, callback); }

    // Statistics
    size_t GetObjectCount() const { return quadtree_.GetObjectCount(); }
    size_t GetNodeCount() const { return quadtree_.GetNodeCount(); }
    const AABB2D& GetWorldBounds() const { return quadtree_.GetBounds(); }

    // Debug
    void GetDebugBounds(std::vector<AABB2D>& bounds) const { quadtree_.GetAllBounds(bounds); }

private:
    mutable Quadtree quadtree_;
    std::unordered_set<EntityID, EntityID::Hash> registered_objects_;
    bool auto_expand_ = true;

    void RebuildQuadtree(const AABB2D& new_bounds);
};

/**
 * @brief Spatial utility functions
 */
namespace SpatialUtils {
    // AABB construction helpers
    AABB2D CreateAABB(const Vector2f& center, const Vector2f& size);
    AABB2D CreateAABBFromCircle(const Vector2f& center, float radius);
    AABB2D CreateAABBFromPoints(const std::vector<Vector2f>& points);
    
    // Transform AABB by matrix
    AABB2D TransformAABB(const AABB2D& aabb, const Matrix3f& transform);
    
    // Distance calculations
    float PointToAABBDistance(const Vector2f& point, const AABB2D& aabb);
    float AABBToAABBDistance(const AABB2D& a, const AABB2D& b);
    
    // Collision detection
    bool CircleAABBIntersect(const Vector2f& center, float radius, const AABB2D& aabb);
    bool LineAABBIntersect(const Vector2f& start, const Vector2f& end, const AABB2D& aabb);
}

} // namespace Scene
} // namespace PyNovaGE