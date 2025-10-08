#pragma once

#include "../vector3.hpp"
#include "../matrix4.hpp"
#include "primitives.hpp"
#include "frustum_culling.hpp"
#include "intersection.hpp"
#include <cstdint>
#include <vector>
#include <memory>
#include <functional>

namespace pynovage {
namespace math {
namespace geometry {

// Forward declarations
template<typename T> class SpatialContainer;
template<typename T> class SpatialObject;
template<typename T> class SpatialQuery;

/**
 * Base interface for objects that can be stored in spatial data structures
 */
template<typename T>
class SpatialObject {
public:
    virtual ~SpatialObject() = default;
    
    // Core interface
    virtual AABB getBounds() const = 0;
    virtual bool intersects(const AABB& bounds) const = 0;
    virtual bool contains(const Vector3& point) const = 0;
    
    // Optional interface (default implementations provided)
    virtual bool intersectsRay(const Ray3D& ray, float& t) const { return false; }
    virtual bool intersectsFrustum(const FrustumCulling& frustum) const { return false; }
    
    // Data accessors
    virtual const T& getData() const = 0;
    virtual T& getData() = 0;
    
    // Optimization hints
    virtual bool isStatic() const { return false; }
    virtual std::uint32_t getUpdateFrequency() const { return 1; }
};

/**
 * Query interface for spatial queries
 */
template<typename T>
class SpatialQuery {
public:
    virtual ~SpatialQuery() = default;
    
    // Query types
    virtual bool shouldAcceptObject(const SpatialObject<T>& object) const = 0;
    virtual bool shouldTraverseNode(const AABB& nodeBounds) const = 0;
    
    // Optional early termination
    virtual bool isComplete() const { return false; }
};

/**
 * Common query implementations
 */

// Point query
template<typename T>
class PointQuery : public SpatialQuery<T> {
public:
    explicit PointQuery(const Vector3& point) : point_(point) {}
    
    bool shouldAcceptObject(const SpatialObject<T>& object) const override {
        return object.contains(point_);
    }
    
    bool shouldTraverseNode(const AABB& nodeBounds) const override {
        return nodeBounds.contains(point_);
    }
    
    // Accessors
    const Vector3& getPoint() const { return point_; }

private:
    Vector3 point_;
};

// Ray query
template<typename T>
class RayQuery : public SpatialQuery<T> {
public:
    RayQuery(const Ray3D& ray, float maxDist = std::numeric_limits<float>::max())
        : ray_(ray), maxDistance_(maxDist) {}
    
    bool shouldAcceptObject(const SpatialObject<T>& object) const override {
        float t;
        return object.intersectsRay(ray_, t) && t <= maxDistance_;
    }
    
    bool shouldTraverseNode(const AABB& nodeBounds) const override {
        auto result = rayAABBIntersection(ray_, nodeBounds);
        return result && result->distance <= maxDistance_;
    }
    
private:
    Ray3D ray_;
    float maxDistance_;
};

// Volume query
template<typename T>
class VolumeQuery : public SpatialQuery<T> {
public:
    explicit VolumeQuery(const AABB& bounds) : bounds_(bounds) {}
    
    bool shouldAcceptObject(const SpatialObject<T>& object) const override {
        return object.intersects(bounds_);
    }
    
    bool shouldTraverseNode(const AABB& nodeBounds) const override {
        return aabbAABBIntersection(bounds_, nodeBounds).has_value();
    }
    
    // Accessors
    const AABB& getBounds() const { return bounds_; }

private:
    AABB bounds_;
};

// Frustum query (for view frustum culling)
template<typename T>
class FrustumQuery : public SpatialQuery<T> {
public:
    explicit FrustumQuery(const FrustumCulling& frustum) : frustum_(frustum) {}
    
    bool shouldAcceptObject(const SpatialObject<T>& object) const override {
        return object.intersectsFrustum(frustum_);
    }
    
    bool shouldTraverseNode(const AABB& nodeBounds) const override {
        return frustum_.intersects(nodeBounds);
    }
    
private:
    FrustumCulling frustum_;
};

/**
 * Base interface for spatial partitioning containers
 */
template<typename T>
class SpatialContainer {
public:
    virtual ~SpatialContainer() = default;
    
    // Container operations
    virtual void insert(std::unique_ptr<SpatialObject<T>> object) = 0;
    virtual void remove(const SpatialObject<T>* object) = 0;
    virtual void update(const SpatialObject<T>* object) = 0;
    virtual void clear() = 0;
    
    // Queries
    virtual void query(const SpatialQuery<T>& query,
                      std::vector<const SpatialObject<T>*>& results) const = 0;
    
    // Optimization operations
    virtual void optimize() = 0;
    virtual void rebuild() = 0;
    
    // Statistics
    virtual std::size_t getObjectCount() const = 0;
    virtual std::size_t getNodeCount() const = 0;
    virtual std::size_t getMaxDepth() const = 0;
    virtual float getAverageObjectsPerNode() const = 0;
    
    // Debug visualization (optional)
    virtual void debugDraw(const std::function<void(const AABB&)>& drawAABB) const {}
};

// Configuration parameters for spatial partitioning
struct SpatialConfig {
    std::uint32_t maxObjectsPerNode = 16;
    std::uint32_t maxDepth = 16;
    float minNodeSize = 1.0f;
    float looseness = 1.5f;  // For loose octrees/quadtrees
    float maxObjectSizeRatio = 2.0f;  // For Quadtree/Octree, determines when to keep objects at current level
    
    // BSP specific
    float splitCost = 1.0f;
    float traversalCost = 0.1f;
    std::uint32_t maxTrianglesPerLeaf = 32;
    
    // Hash grid specific
    float cellSize = 10.0f;
    std::uint32_t tableSize = 16384;
    
    // Optimization thresholds
    float rebuildThreshold = 0.7f;
    std::uint32_t updateInterval = 60;
};

} // namespace geometry
} // namespace math
} // namespace pynovage