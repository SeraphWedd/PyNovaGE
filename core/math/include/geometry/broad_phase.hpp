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
using ProxyId = size_t;

// Represents a pair of potentially colliding AABBs
struct CollisionPair {
    ProxyId a;
    ProxyId b;

    bool operator==(const CollisionPair& other) const {
        return (a == other.a && b == other.b) || (a == other.b && b == other.a);
    }

    size_t hash() const {
        ProxyId h1 = a;
        ProxyId h2 = b;
        if (h1 > h2) std::swap(h1, h2);
        return h1 * 37 + h2;
    }

    void getOrdered(ProxyId& first, ProxyId& second) const {
        first = std::min(a, b);
        second = std::max(a, b);
    }
};

// Broad-phase collision culling using sweep and prune with spatial hashing
class BroadPhase {
public:
    BroadPhase(float cell_size = 10.0f);
    ~BroadPhase();

    ProxyId createProxy(const AABB& aabb, bool is_static = false);
    void destroyProxy(ProxyId id);
    void updateProxy(ProxyId id, const AABB& aabb);
    void finalizeBroadPhase();

    std::vector<geometry::CollisionPair> findPotentialCollisions(size_t max_pairs = 0);
    void updateTemporalCoherence();

private:
    // SoA data structures for better cache and SIMD
    struct ProxyData {
        std::vector<bool> isStatic;
        std::vector<Vector3> centers;
        std::vector<uint32_t> mortonCodes;
        std::vector<AABB> aabbs;          // Only used for returning full AABB data

        // Bounds data for efficient SIMD
        std::vector<float> minX, minY, minZ;
        std::vector<float> maxX, maxY, maxZ;

        // Dynamic object data
        std::vector<int32_t> sortKeys[3];    // Sort keys for each axis
        std::vector<bool> needsResort[3];    // Flags for each axis

        // Static object data
        std::vector<size_t> gridKeys;        // Hash grid cell keys

        // Maintain free list for proxy IDs
        std::vector<ProxyId> freeIds;
        ProxyId nextId = 0;

        ProxyId allocateId() {
            if (!freeIds.empty()) {
                ProxyId id = freeIds.back();
                freeIds.pop_back();
                return id;
            }
            return nextId++;
        }

        void freeId(ProxyId id) {
            freeIds.push_back(id);
        }
    } mProxyData;

    // Helper struct for spatial binning
    struct SpatialBin {
        std::vector<ProxyId> staticObjects;  // Static object IDs in this bin
        std::vector<size_t> gridCells;       // Grid cells overlapping this bin
    };

    // Utility functions for spatial binning
    int getBinIndex(float value) const { return static_cast<int>(value / (mCellSize * 4.0f)); }
    void insertIntoSpatialBin(int binIdx, ProxyId id);
    std::vector<SpatialBin> mSpatialBins;
    bool mBinsNeedUpdate = true;

    // Dynamic object lists (sorted by min bounds)
    std::vector<ProxyId> mDynamicList[3];  // One list per axis
    bool mDirtyAxes[3];  // Tracks which axes need sorting

    // Hash grid for static objects
    std::unordered_map<size_t, std::vector<ProxyId>> mGrid;

    // Morton code and hash computation
    uint32_t computeMortonCode(const Vector3& position) const;
    size_t hashPosition(const Vector3& position) const;

    // Helper functions
    void insertIntoGrid(ProxyId id);
    void removeFromGrid(ProxyId id);
    void sortAxisList(int axis);
    bool testOverlap(ProxyId a, ProxyId b) const;
    void insertionSort(std::vector<ProxyId>& list, int axis);

    // Internal state
    float mCellSize;  // Grid cell size for spatial hashing

    // Temporal coherence tracking
    struct PreviousState {
        Vector3 center;      // Previous center position
        Vector3 extent;      // Previous half-extents
        float moveThresh;    // How far object can move before needing retest
    };
    std::unordered_map<ProxyId, PreviousState> mPrevStates;
    std::vector<geometry::CollisionPair> mPrevPairs;
};


} // namespace geometry
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_BROAD_PHASE_HPP
