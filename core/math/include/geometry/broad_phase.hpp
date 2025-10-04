#ifndef PYNOVAGE_MATH_GEOMETRY_BROAD_PHASE_HPP
#define PYNOVAGE_MATH_GEOMETRY_BROAD_PHASE_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include "primitives.hpp"
#include "../vector3.hpp"

namespace pynovage {
namespace math {
namespace geometry {

// Forward declarations
struct AABBProxy;

// Represents a pair of potentially colliding AABBs
struct CollisionPair {
    const AABBProxy* a;
    const AABBProxy* b;

    bool operator==(const CollisionPair& other) const {
        return (a == other.a && b == other.b) || (a == other.b && b == other.a);
    }

    size_t hash() const {
        size_t h1 = reinterpret_cast<size_t>(a);
        size_t h2 = reinterpret_cast<size_t>(b);
        if (h1 > h2) std::swap(h1, h2);
        return h1 * 37 + h2;
    }

    void getOrdered(const AABBProxy*& first, const AABBProxy*& second) const {
        size_t id1 = reinterpret_cast<size_t>(a);
        size_t id2 = reinterpret_cast<size_t>(b);
        if (id1 < id2) { first = a; second = b; }
        else { first = b; second = a; }
    }
};

// Broad-phase collision culling using sweep and prune with spatial hashing
class BroadPhase {
public:
    BroadPhase(float cell_size = 10.0f);
    ~BroadPhase();

    AABBProxy* createProxy(const AABB& aabb, bool is_static = false);
    void destroyProxy(AABBProxy* proxy);
    void updateProxy(AABBProxy* proxy, const AABB& aabb);
    void finalizeBroadPhase();

    std::vector<geometry::CollisionPair> findPotentialCollisions(size_t max_pairs = 0);
    void updateTemporalCoherence();

private:
    // Helper struct for fast spatial binning
    struct SpatialBin {
        std::vector<AABBProxy*> staticObjects;  // Static objects in this bin
        std::vector<size_t> gridCells;          // Grid cells overlapping this bin
    };

    // Utility functions for spatial binning
    int getBinIndex(float value) const { return static_cast<int>(value / (mCellSize * 4.0f)); }
    void insertIntoSpatialBin(int binIdx, AABBProxy* proxy);
    std::vector<SpatialBin> mSpatialBins;
    bool mBinsNeedUpdate = true;

    // Flags to track which axes need sorting
    bool mDirtyAxes[3];

    // Morton code computation for spatial coherence
    uint32_t computeMortonCode(const Vector3& position) const;

    // Sweep and prune helpers
    void sortAxisList(int axis);
    bool testOverlap(const AABBProxy* a, const AABBProxy* b) const;
    void insertionSort(std::vector<AABBProxy*>& list, int axis);

    // Spatial hashing
    size_t hashPosition(const Vector3& position) const;
    void insertIntoGrid(AABBProxy* proxy);
    void removeFromGrid(AABBProxy* proxy);

    // Internal state
    float mCellSize;                    // Grid cell size for spatial hashing
    std::vector<AABBProxy*> mProxies;   // All proxies
    std::vector<AABBProxy*> mDynamicProxies[3];  // Sorted lists for each axis (SAP)
    std::unordered_map<size_t, std::vector<AABBProxy*>> mGrid;  // Spatial hash grid

    // Temporal coherence tracking
    struct PreviousState {
        Vector3 center;      // Previous center position
        Vector3 extent;      // Previous half-extents
        float moveThresh;    // How far object can move before needing retest
    };
    std::unordered_map<const AABBProxy*, PreviousState> mPrevStates;
    std::vector<geometry::CollisionPair> mPrevPairs;
};

// Proxy object for an AABB in the broad-phase system
struct AABBProxy {
    AABB aabb;               // The actual AABB
    bool isStatic;           // Whether this is a static object
    size_t gridKey;          // Current grid cell (for static objects)
    Vector3 center;          // Center point for quick distance checks
    uint32_t mortonCode;     // Morton code for spatial coherence
    size_t proxyId;          // Unique identifier

    // SAP data (for dynamic objects)
    float min[3];            // Minimum bounds on each axis
    float max[3];            // Maximum bounds on each axis
    int32_t sortKeys[3];     // Sort keys for each axis
    bool needsResort[3];     // Flags to track which axes need resorting
};

} // namespace geometry
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_BROAD_PHASE_HPP
