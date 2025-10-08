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
        ProxyId h1 = std::min(a, b);
        ProxyId h2 = std::max(a, b);
        return h1 * 37 + h2;
    }
};

// Simple uniform grid cell for static objects
struct Cell {
    std::vector<ProxyId> static_objects;
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

    std::vector<CollisionPair> findPotentialCollisions(size_t max_pairs = 0);

private:
    // Core data structures with SIMD-friendly SoA layout
    struct alignas(32) ProxyData {
        // SIMD-aligned bounds data
        alignas(32) std::vector<float> minX, minY, minZ;
        alignas(32) std::vector<float> maxX, maxY, maxZ;
        
        // Object state
        std::vector<bool> isStatic;
        std::vector<ProxyId> dynamic_objects;  // Sorted by X-axis
        
        // ID management
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

    // Uniform grid for static objects
    std::unordered_map<uint64_t, Cell> mGrid;
    float mCellSize;
    bool mNeedsSorting;

    // Helper functions
    void insertIntoGrid(ProxyId id);
    void removeFromGrid(ProxyId id);
    uint64_t getGridKey(const Vector3& position) const;
    void sortDynamicObjects();
    bool testOverlap(ProxyId a, ProxyId b) const;

    // Grid helpers
    Vector3 getCellCoords(const Vector3& position) const {
        return Vector3(
            std::floor(position.x / mCellSize),
            std::floor(position.y / mCellSize),
            std::floor(position.z / mCellSize)
        );
    }

    uint64_t packGridKey(int x, int y, int z) const {
        return (static_cast<uint64_t>(x) << 42) |
               (static_cast<uint64_t>(y) << 21) |
               static_cast<uint64_t>(z);
    }
};


} // namespace geometry
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_BROAD_PHASE_HPP
