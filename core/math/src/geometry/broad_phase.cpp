#include "geometry/broad_phase.hpp"
#include "simd_utils.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace pynovage {
namespace math {
namespace geometry {

BroadPhase::BroadPhase(float cell_size) : mCellSize(cell_size), mNeedsSorting(false) {
    // Reserve space for initial capacity
    const size_t INITIAL_CAPACITY = 128;
    mProxyData.isStatic.reserve(INITIAL_CAPACITY);
    mProxyData.dynamic_objects.reserve(INITIAL_CAPACITY);
    
    // Reserve space for bounds data
    mProxyData.minX.reserve(INITIAL_CAPACITY);
    mProxyData.minY.reserve(INITIAL_CAPACITY);
    mProxyData.minZ.reserve(INITIAL_CAPACITY);
    mProxyData.maxX.reserve(INITIAL_CAPACITY);
    mProxyData.maxY.reserve(INITIAL_CAPACITY);
    mProxyData.maxZ.reserve(INITIAL_CAPACITY);
}

BroadPhase::~BroadPhase() {
    // No manual cleanup needed - vectors clean themselves up
}

uint64_t BroadPhase::getGridKey(const Vector3& position) const {
    Vector3 cell = getCellCoords(position);
    return packGridKey(
        static_cast<int>(cell.x),
        static_cast<int>(cell.y),
        static_cast<int>(cell.z)
    );
}

void BroadPhase::insertIntoGrid(ProxyId id) {
    // Get cell coordinates from center point
    Vector3 center(
        (mProxyData.maxX[id] + mProxyData.minX[id]) * 0.5f,
        (mProxyData.maxY[id] + mProxyData.minY[id]) * 0.5f,
        (mProxyData.maxZ[id] + mProxyData.minZ[id]) * 0.5f
    );
    
    uint64_t key = getGridKey(center);
    mGrid[key].static_objects.push_back(id);
}

void BroadPhase::removeFromGrid(ProxyId id) {
    // Find object in all cells and remove
    for (auto& cell_pair : mGrid) {
        auto& objects = cell_pair.second.static_objects;
        auto it = std::find(objects.begin(), objects.end(), id);
        if (it != objects.end()) {
            objects.erase(it);
            if (objects.empty()) {
                mGrid.erase(cell_pair.first);
            }
            break;
        }
    }
}

bool BroadPhase::testOverlap(ProxyId a, ProxyId b) const {
#if PYNOVAGE_MATH_HAS_SSE2
    // Load bounds data for object a
    __m128 minA = _mm_set_ps(0.0f,
                            mProxyData.minZ[a],
                            mProxyData.minY[a],
                            mProxyData.minX[a]);
    __m128 maxA = _mm_set_ps(0.0f,
                            mProxyData.maxZ[a],
                            mProxyData.maxY[a],
                            mProxyData.maxX[a]);

    // Load bounds data for object b
    __m128 minB = _mm_set_ps(0.0f,
                            mProxyData.minZ[b],
                            mProxyData.minY[b],
                            mProxyData.minX[b]);
    __m128 maxB = _mm_set_ps(0.0f,
                            mProxyData.maxZ[b],
                            mProxyData.maxY[b],
                            mProxyData.maxX[b]);

    // Test minB <= maxA and minA <= maxB for all axes simultaneously
    __m128 cmpMinMaxA = _mm_cmple_ps(minB, maxA);
    __m128 cmpMinMaxB = _mm_cmple_ps(minA, maxB);
    __m128 andResult = _mm_and_ps(cmpMinMaxA, cmpMinMaxB);

    // Check if all three axes overlap (ignore w component)
    return (_mm_movemask_ps(andResult) & 0x7) == 0x7;
#else
    // Fallback scalar implementation
    return !(mProxyData.minX[a] > mProxyData.maxX[b] || mProxyData.minX[b] > mProxyData.maxX[a] ||
             mProxyData.minY[a] > mProxyData.maxY[b] || mProxyData.minY[b] > mProxyData.maxY[a] ||
             mProxyData.minZ[a] > mProxyData.maxZ[b] || mProxyData.minZ[b] > mProxyData.maxZ[a]);
#endif
}

void BroadPhase::destroyProxy(ProxyId id) {
    if (id >= mProxyData.isStatic.size()) return;

    if (mProxyData.isStatic[id]) {
        removeFromGrid(id);
    } else {
        auto& list = mProxyData.dynamic_objects;
        list.erase(std::remove(list.begin(), list.end(), id), list.end());
    }

    // Add to free list for reuse
    mProxyData.freeId(id);
}


void BroadPhase::updateProxy(ProxyId id, const AABB& aabb) {
    if (id >= mProxyData.isStatic.size()) return;

    // Store old bounds for comparison
    float oldMinX = mProxyData.minX[id];

    // Update bounds
    mProxyData.minX[id] = aabb.min.x;
    mProxyData.minY[id] = aabb.min.y;
    mProxyData.minZ[id] = aabb.min.z;
    mProxyData.maxX[id] = aabb.max.x;
    mProxyData.maxY[id] = aabb.max.y;
    mProxyData.maxZ[id] = aabb.max.z;

    const float EPSILON = 1e-4f;

    // Update grid if static, otherwise mark for sort if X changed
    if (mProxyData.isStatic[id]) {
        removeFromGrid(id);
        insertIntoGrid(id);
    } else {
        if (std::abs(oldMinX - aabb.min.x) > EPSILON) {
            mNeedsSorting = true;
        }
    }
}

std::vector<CollisionPair> BroadPhase::findPotentialCollisions(size_t max_pairs) {
    std::vector<CollisionPair> pairs;
    std::unordered_set<size_t> pairHashes;  // Track pairs we've already checked
    if (max_pairs == 0) {
        max_pairs = std::numeric_limits<size_t>::max();
    }

    // Ensure dynamic list sorted on X
    if (mNeedsSorting) {
        sortDynamicObjects();
        mNeedsSorting = false;
    }

    const auto& list = mProxyData.dynamic_objects;  // X-sorted
    const size_t listSize = list.size();

    // SIMD working arrays
    const size_t SIMD_WIDTH = 4;
    alignas(32) float mins[12];  // 4 sets of x,y,z
    alignas(32) float maxs[12];  // 4 sets of x,y,z
    alignas(32) int results[4];

    // Sweep and Prune: dynamic vs dynamic
    for (size_t ii = 0; ii < listSize && pairs.size() < max_pairs; ++ii) {
        const ProxyId i = list[ii];
        const float max_i_x = mProxyData.maxX[i];

        size_t jj = ii + 1;
        while (jj < listSize && pairs.size() < max_pairs) {
            const ProxyId j = list[jj];
            if (mProxyData.minX[j] > max_i_x) break; // early out on X

            // Process up to 4 candidates at once
            size_t batch = std::min(SIMD_WIDTH, listSize - jj);

            // Pack target AABB (i)
            float minA[3] = {mProxyData.minX[i], mProxyData.minY[i], mProxyData.minZ[i]};
            float maxA[3] = {mProxyData.maxX[i], mProxyData.maxY[i], mProxyData.maxZ[i]};

            for (size_t k = 0; k < batch; ++k) {
                ProxyId cand = list[jj + k];
                mins[k]     = mProxyData.minX[cand];
                mins[k + 4] = mProxyData.minY[cand];
                mins[k + 8] = mProxyData.minZ[cand];
                maxs[k]     = mProxyData.maxX[cand];
                maxs[k + 4] = mProxyData.maxY[cand];
                maxs[k + 8] = mProxyData.maxZ[cand];
            }

            SimdUtils::TestAABBOverlap4f(minA, maxA, mins, maxs, results);

            for (size_t k = 0; k < batch; ++k) {
                if (results[k]) {
                    CollisionPair pair{i, list[jj + k]};
                    size_t hash = pair.hash();
                    if (pairHashes.insert(hash).second) {
                        pairs.push_back(pair);
                        if (pairs.size() >= max_pairs) break;
                    }
                }
            }

            jj += batch;
        }
    }

    // Dynamic vs static using grid
    for (ProxyId id : list) {
        // Center of dynamic object
        Vector3 center(
            (mProxyData.maxX[id] + mProxyData.minX[id]) * 0.5f,
            (mProxyData.maxY[id] + mProxyData.minY[id]) * 0.5f,
            (mProxyData.maxZ[id] + mProxyData.minZ[id]) * 0.5f
        );

        // Query 3x1x1 neighborhood along X to be safe
        Vector3 cell = getCellCoords(center);
        for (int dx = -1; dx <= 1 && pairs.size() < max_pairs; ++dx) {
            uint64_t key = packGridKey(static_cast<int>(cell.x) + dx, static_cast<int>(cell.y), static_cast<int>(cell.z));
            auto it = mGrid.find(key);
            if (it == mGrid.end()) continue;
            const auto& statics = it->second.static_objects;

            // SIMD batch against statics
            for (size_t s = 0; s < statics.size() && pairs.size() < max_pairs; s += SIMD_WIDTH) {
                size_t batch = std::min(SIMD_WIDTH, statics.size() - s);

                float minA[3] = {mProxyData.minX[id], mProxyData.minY[id], mProxyData.minZ[id]};
                float maxA[3] = {mProxyData.maxX[id], mProxyData.maxY[id], mProxyData.maxZ[id]};

                for (size_t k = 0; k < batch; ++k) {
                    ProxyId sid = statics[s + k];
                    mins[k]     = mProxyData.minX[sid];
                    mins[k + 4] = mProxyData.minY[sid];
                    mins[k + 8] = mProxyData.minZ[sid];
                    maxs[k]     = mProxyData.maxX[sid];
                    maxs[k + 4] = mProxyData.maxY[sid];
                    maxs[k + 8] = mProxyData.maxZ[sid];
                }

                SimdUtils::TestAABBOverlap4f(minA, maxA, mins, maxs, results);

                for (size_t k = 0; k < batch; ++k) {
                    if (results[k]) {
                        CollisionPair pair{id, statics[s + k]};
                        size_t hash = pair.hash();
                        if (pairHashes.insert(hash).second) {
                            pairs.push_back(pair);
                            if (pairs.size() >= max_pairs) break;
                        }
                    }
                }
            }
        }
    }

    return pairs;
}

void BroadPhase::sortDynamicObjects() {
    if (mProxyData.dynamic_objects.empty()) return;
    
    // Sort all dynamic objects by X-axis
    std::sort(mProxyData.dynamic_objects.begin(), mProxyData.dynamic_objects.end(),
              [this](ProxyId a, ProxyId b) {
                  return mProxyData.minX[a] < mProxyData.minX[b];
              });
}

ProxyId BroadPhase::createProxy(const AABB& aabb, bool is_static) {
    // Allocate new proxy ID
    ProxyId id = mProxyData.allocateId();

    // Ensure vectors have space
    if (id >= mProxyData.isStatic.size()) {
        size_t new_size = id + 1;
        mProxyData.isStatic.resize(new_size);
        mProxyData.minX.resize(new_size);
        mProxyData.minY.resize(new_size);
        mProxyData.minZ.resize(new_size);
        mProxyData.maxX.resize(new_size);
        mProxyData.maxY.resize(new_size);
        mProxyData.maxZ.resize(new_size);
    }

    // Store bounds data
    mProxyData.isStatic[id] = is_static;
    mProxyData.minX[id] = aabb.min.x;
    mProxyData.minY[id] = aabb.min.y;
    mProxyData.minZ[id] = aabb.min.z;
    mProxyData.maxX[id] = aabb.max.x;
    mProxyData.maxY[id] = aabb.max.y;
    mProxyData.maxZ[id] = aabb.max.z;

    if (is_static) {
        insertIntoGrid(id);
    } else {
        mProxyData.dynamic_objects.push_back(id);
        mNeedsSorting = true;
    }

    return id;
}

void BroadPhase::finalizeBroadPhase() {
    if (mNeedsSorting) {
        sortDynamicObjects();
        mNeedsSorting = false;
    }
}

} // namespace geometry
} // namespace math
} // namespace pynovage
