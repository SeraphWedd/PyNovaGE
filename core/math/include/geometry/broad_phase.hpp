#ifndef PYNOVAGE_MATH_GEOMETRY_BROAD_PHASE_HPP
#define PYNOVAGE_MATH_GEOMETRY_BROAD_PHASE_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "primitives.hpp"
#include "../vector3.hpp"

namespace pynovage {
namespace math {
namespace geometry {

// Forward declarations
struct AABBNode;
struct AABBProxy;

/**
 * @brief Broad-phase collision culling using sweep and prune with spatial hashing
 * 
 * Implements a hybrid approach combining:
 * 1. Sweep and Prune (SAP) for dynamic objects
 * 2. Spatial hashing for static objects
 * 3. Morton codes for cache-coherent traversal
 */
class BroadPhase {
public:
    struct CollisionPair {
        AABBProxy* a;
        AABBProxy* b;

        bool operator==(const CollisionPair& other) const {
            return (a == other.a && b == other.b) || (a == other.b && b == other.a);
        }

        size_t hash() const {
            size_t h1 = reinterpret_cast<size_t>(a);
            size_t h2 = reinterpret_cast<size_t>(b);
            // Ensure order doesn't matter
            if (h1 > h2) std::swap(h1, h2);
            return h1 * 37 + h2;
        }

        // Extract canonical ordering (useful for temporal coherence)
        void getOrdered(AABBProxy*& first, AABBProxy*& second) const {
            size_t id1 = reinterpret_cast<size_t>(a);
            size_t id2 = reinterpret_cast<size_t>(b);
            if (id1 < id2) {
                first = a;
                second = b;
            } else {
                first = b;
                second = a;
            }
        }
    };
    BroadPhase(float cell_size = 10.0f);
    ~BroadPhase();

    /**
     * @brief Add an AABB to the broad-phase system
     * @param aabb The AABB to add
     * @param is_static Whether the object is static (for spatial hashing)
     * @return Proxy object for updating the AABB
     */
    AABBProxy* createProxy(const AABB& aabb, bool is_static = false);

    /**
     * @brief Remove an AABB from the broad-phase system
     * @param proxy The proxy to remove
     */
    void destroyProxy(AABBProxy* proxy);

    /**
     * @brief Update an AABB's position/size without immediate sorting
     * @param proxy The proxy to update
     * @param aabb The new AABB
     * 
     * This operation is fast as it only marks axes as needing resort.
     * Call finalizeBroadPhase() after all updates to perform the actual sorting.
     */
    void updateProxy(AABBProxy* proxy, const AABB& aabb);

    /**
     * @brief Finalizes all pending broad phase updates
     * 
     * Call this after performing all updateProxy operations for the frame.
     * This will sort the necessary axis lists for sweep and prune collision detection.
     */
    void finalizeBroadPhase();

    /**
     * @brief Find all potential collisions
     * @param max_pairs Maximum number of pairs to return (0 for unlimited)
     * @return Vector of potentially colliding AABB pairs
     */
    std::vector<CollisionPair> findPotentialCollisions(size_t max_pairs = 0);

    /**
     * @brief Updates temporal coherence data after physics step
     * Call this after all proxies have been updated for the frame
     */
    void updateTemporalCoherence();

    private:
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
        float moveThresh;   // How far object can move before needing retest
    };
    std::unordered_map<AABBProxy*, PreviousState> mPrevStates;
    std::vector<CollisionPair> mPrevPairs;
};

/**
 * @brief Proxy object for an AABB in the broad-phase system
 */
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