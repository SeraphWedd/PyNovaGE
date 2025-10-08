#include "geometry/broad_phase.hpp"
#include "simd_utils.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace pynovage {
namespace math {
namespace geometry {

namespace {
    // Simple Morton encoding helpers (expand 10-bit values). For now, use a basic hash fallback.
}

uint32_t BroadPhase::computeMortonCode(const Vector3& position) const {
    // Basic 3D hash as a stand-in for Morton code to satisfy linker and keep behavior deterministic
    int32_t xi = static_cast<int32_t>(position.x * 10.0f);
    int32_t yi = static_cast<int32_t>(position.y * 10.0f);
    int32_t zi = static_cast<int32_t>(position.z * 10.0f);
    uint32_t hx = static_cast<uint32_t>(xi * 73856093);
    uint32_t hy = static_cast<uint32_t>(yi * 19349663);
    uint32_t hz = static_cast<uint32_t>(zi * 83492791);
    return hx ^ (hy << 1) ^ (hz << 2);
}

BroadPhase::BroadPhase(float cell_size) : mCellSize(cell_size) {
    // Reserve space for initial capacity
    const size_t INITIAL_CAPACITY = 128;
    mProxyData.isStatic.reserve(INITIAL_CAPACITY);
    mProxyData.centers.reserve(INITIAL_CAPACITY);
    mProxyData.mortonCodes.reserve(INITIAL_CAPACITY);
    mProxyData.aabbs.reserve(INITIAL_CAPACITY);
    
    // Reserve space for bounds data
    mProxyData.bounds.minX.reserve(INITIAL_CAPACITY);
    mProxyData.bounds.minY.reserve(INITIAL_CAPACITY);
    mProxyData.bounds.minZ.reserve(INITIAL_CAPACITY);
    mProxyData.bounds.maxX.reserve(INITIAL_CAPACITY);
    mProxyData.bounds.maxY.reserve(INITIAL_CAPACITY);
    mProxyData.bounds.maxZ.reserve(INITIAL_CAPACITY);
    
    // Reserve space for dynamic object data
    for (int i = 0; i < 3; ++i) {
        mProxyData.dynamicAxes[i].sortKeys.reserve(INITIAL_CAPACITY);
        mProxyData.dynamicAxes[i].needsResort.reserve(INITIAL_CAPACITY);
        mProxyData.dynamicAxes[i].objects.reserve(INITIAL_CAPACITY);
        mDirtyAxes[i] = false;
    }
    
    // Reserve space for static object data
    mProxyData.gridKeys.reserve(INITIAL_CAPACITY);
    mSpatialBins.reserve(32);
}

BroadPhase::~BroadPhase() {
    // No manual cleanup needed - vectors clean themselves up
}

void BroadPhase::updateTemporalCoherence() {
    mPrevPairs.clear();
    mPrevPairs.shrink_to_fit();
    mPrevStates.clear();
    
    // Recompute previous states and movement thresholds for all active proxies
    size_t maxId = mProxyData.aabbs.size();
    for (ProxyId id = 0; id < maxId; ++id) {
        // Skip inactive proxies
        if (std::find(mProxyData.freeIds.begin(), mProxyData.freeIds.end(), id) != mProxyData.freeIds.end()) {
            continue;
        }
        
        PreviousState st;
        st.center = mProxyData.centers[id];
    Vector3 halfExtent = Vector3(
            (mProxyData.bounds.maxX[id] - mProxyData.bounds.minX[id]) * 0.5f,
            (mProxyData.bounds.maxY[id] - mProxyData.bounds.minY[id]) * 0.5f,
            (mProxyData.bounds.maxZ[id] - mProxyData.bounds.minZ[id]) * 0.5f
        );
        st.extent = halfExtent;
        
        // Allow motion up to 10% of max extent without forcing retest
        float maxExtent = std::max(halfExtent.x, std::max(halfExtent.y, halfExtent.z));
        st.moveThresh = std::max(0.01f, 0.1f * maxExtent);
        mPrevStates[id] = st;
    }
}

ProxyId BroadPhase::createProxy(const AABB& aabb, bool is_static) {
    // Allocate new proxy ID
    ProxyId id = mProxyData.allocateId();
    
    // Ensure vectors have space
    if (id >= mProxyData.isStatic.size()) {
        size_t new_size = id + 1;
        mProxyData.isStatic.resize(new_size);
        mProxyData.centers.resize(new_size);
        mProxyData.mortonCodes.resize(new_size);
        mProxyData.aabbs.resize(new_size);
        mProxyData.bounds.minX.resize(new_size);
        mProxyData.bounds.minY.resize(new_size);
        mProxyData.bounds.minZ.resize(new_size);
        mProxyData.bounds.maxX.resize(new_size);
        mProxyData.bounds.maxY.resize(new_size);
        mProxyData.bounds.maxZ.resize(new_size);
        mProxyData.gridKeys.resize(new_size);
        
        for (int i = 0; i < 3; ++i) {
            mProxyData.dynamicAxes[i].sortKeys.resize(new_size);
            mProxyData.dynamicAxes[i].needsResort.resize(new_size);
        }
    }
    
    // Store basic data
    mProxyData.isStatic[id] = is_static;
    mProxyData.aabbs[id] = aabb;
    mProxyData.centers[id] = (aabb.min + aabb.max) * 0.5f;
    
    // Store bounds data
    mProxyData.bounds.minX[id] = aabb.min.x;
    mProxyData.bounds.minY[id] = aabb.min.y;
    mProxyData.bounds.minZ[id] = aabb.min.z;
    mProxyData.bounds.maxX[id] = aabb.max.x;
    mProxyData.bounds.maxY[id] = aabb.max.y;
    mProxyData.bounds.maxZ[id] = aabb.max.z;
    
    // Compute Morton code
    mProxyData.mortonCodes[id] = computeMortonCode(mProxyData.centers[id]);
    
    if (is_static) {
        // Add to static grid
        insertIntoGrid(id);
    } else {
        // Initialize dynamic object data
        for (int axis = 0; axis < 3; ++axis) {
            mProxyData.dynamicAxes[axis].sortKeys[id] = static_cast<int32_t>(mProxyData.dynamicAxes[axis].objects.size());
            mProxyData.dynamicAxes[axis].needsResort[id] = false;
            mProxyData.dynamicAxes[axis].objects.push_back(id);
            mDirtyAxes[axis] = true;
        }
    }
    
    return id;
}

void BroadPhase::destroyProxy(ProxyId id) {
    if (id >= mProxyData.isStatic.size()) return;
    
    // Remove from appropriate systems
    if (mProxyData.isStatic[id]) {
        removeFromGrid(id);
    } else {
        for (int axis = 0; axis < 3; axis++) {
            auto& list = mProxyData.dynamicAxes[axis].objects;
            list.erase(std::remove(list.begin(), list.end(), id), list.end());
        }
    }
    
    // Add to free list for reuse
    mProxyData.freeId(id);
}

void BroadPhase::insertIntoSpatialBin(int binIdx, ProxyId id) {
    if (binIdx < 0) return;
    
    // Grow bins array if needed
    while (binIdx >= static_cast<int>(mSpatialBins.size())) {
        mSpatialBins.emplace_back();
    }
    
    if (mProxyData.isStatic[id]) {
        mSpatialBins[binIdx].staticObjects.push_back(id);
    }
}

void BroadPhase::updateProxy(ProxyId id, const AABB& aabb) {
    if (id >= mProxyData.aabbs.size()) return;
    
    // Update basic info
    mProxyData.aabbs[id] = aabb;
    mProxyData.centers[id] = (aabb.min + aabb.max) * 0.5f;
    mProxyData.mortonCodes[id] = computeMortonCode(mProxyData.centers[id]);
    
    // Store old bounds for comparison
    float oldMinX = mProxyData.bounds.minX[id];
    float oldMinY = mProxyData.bounds.minY[id];
    float oldMinZ = mProxyData.bounds.minZ[id];
    
    // Update bounds
    mProxyData.bounds.minX[id] = aabb.min.x;
    mProxyData.bounds.minY[id] = aabb.min.y;
    mProxyData.bounds.minZ[id] = aabb.min.z;
    mProxyData.bounds.maxX[id] = aabb.max.x;
    mProxyData.bounds.maxY[id] = aabb.max.y;
    mProxyData.bounds.maxZ[id] = aabb.max.z;
    
    const float EPSILON = 1e-4f;
    
    // Check which axes need resorting
    bool needsResortX = std::abs(oldMinX - aabb.min.x) > EPSILON;
    bool needsResortY = std::abs(oldMinY - aabb.min.y) > EPSILON;
    bool needsResortZ = std::abs(oldMinZ - aabb.min.z) > EPSILON;
    
    // Update grid if static, otherwise mark axes as dirty
    if (mProxyData.isStatic[id]) {
        removeFromGrid(id);
        insertIntoGrid(id);
    } else {
        if (needsResortX) mDirtyAxes[0] = true;
        if (needsResortY) mDirtyAxes[1] = true;
        if (needsResortZ) mDirtyAxes[2] = true;
        
        mProxyData.dynamicAxes[0].needsResort[id] = needsResortX;
        mProxyData.dynamicAxes[1].needsResort[id] = needsResortY;
        mProxyData.dynamicAxes[2].needsResort[id] = needsResortZ;
    }
}

std::vector<CollisionPair> BroadPhase::findPotentialCollisions(size_t max_pairs) {
    std::vector<CollisionPair> pairs;
    std::unordered_set<size_t> pairHashes;  // Track pairs we've already checked
    if (max_pairs == 0) {
        max_pairs = std::numeric_limits<size_t>::max();
    }

    // Reuse previous pairs if movement since last frame is small (temporal coherence)
    if (!mPrevPairs.empty() && !mPrevStates.empty()) {
        for (const auto& prevPair : mPrevPairs) {
            if (pairs.size() >= max_pairs) break;
            ProxyId idA, idB;
            prevPair.getOrdered(idA, idB);
            auto ita = mPrevStates.find(idA);
            auto itb = mPrevStates.find(idB);
            if (ita == mPrevStates.end() || itb == mPrevStates.end()) continue;
            const auto& sa = ita->second;
            const auto& sb = itb->second;
            float da = (mProxyData.centers[idA] - sa.center).length();
            float db = (mProxyData.centers[idB] - sb.center).length();
            if (da <= sa.moveThresh && db <= sb.moveThresh) {
                size_t h = prevPair.hash();
                if (pairHashes.insert(h).second) {
                    pairs.push_back(prevPair);
                }
            }
        }
    }
    
    // Check dynamic vs dynamic using SAP with optimized cache access and SIMD
    const auto& list = mProxyData.dynamicAxes[0].objects;  // X-axis list
    const size_t listSize = list.size();

    // Pre-compute and cache bounds in SoA format for SIMD processing
    struct alignas(32) BoundsCache {
        std::vector<float> minX, maxX;
        std::vector<float> minY, maxY;
        std::vector<float> minZ, maxZ;
    };
    BoundsCache cache;
    
    // Pre-allocate vectors
    cache.minX.resize(listSize);
    cache.maxX.resize(listSize);
    cache.minY.resize(listSize);
    cache.maxY.resize(listSize);
    cache.minZ.resize(listSize);
    cache.maxZ.resize(listSize);
    
    // Build SoA cache
    for (size_t i = 0; i < listSize; i++) {
        ProxyId id = list[i];
        cache.minX[i] = mProxyData.bounds.minX[id];
        cache.maxX[i] = mProxyData.bounds.maxX[id];
        cache.minY[i] = mProxyData.bounds.minY[id];
        cache.maxY[i] = mProxyData.bounds.maxY[id];
        cache.minZ[i] = mProxyData.bounds.minZ[id];
        cache.maxZ[i] = mProxyData.bounds.maxZ[id];
    }
    
    // Allocate SIMD working arrays
    const size_t SIMD_WIDTH = 4;
    alignas(32) float mins[12];  // 4 sets of x,y,z
    alignas(32) float maxs[12];  // 4 sets of x,y,z
    alignas(32) int results[4];
    
    // Do sweep with SIMD-accelerated bounds testing
    for (size_t i = 0; i < listSize && pairs.size() < max_pairs; i++) {
        const float max_i = cache.maxX[i];
        const float min_i_y = cache.minY[i];
        const float max_i_y = cache.maxY[i];
        const float min_i_z = cache.minZ[i];
        const float max_i_z = cache.maxZ[i];
        
        size_t j = i + 1;
        while (j < listSize && pairs.size() < max_pairs) {
            // Quick X-axis check
            if (cache.minX[j] > max_i) break;
            
            // Process up to 4 candidates at once
            size_t batchSize = std::min(SIMD_WIDTH, listSize - j);
            
            // Load target AABB (i)
            float minA[3] = {cache.minX[i], min_i_y, min_i_z};
            float maxA[3] = {max_i, max_i_y, max_i_z};
            
            // Load batch of AABBs (j, j+1, j+2, j+3)
            for (size_t k = 0; k < batchSize; k++) {
                mins[k] = cache.minX[j + k];
                mins[k + 4] = cache.minY[j + k];
                mins[k + 8] = cache.minZ[j + k];
                maxs[k] = cache.maxX[j + k];
                maxs[k + 4] = cache.maxY[j + k];
                maxs[k + 8] = cache.maxZ[j + k];
            }
            
            // Test batch with SIMD
            SimdUtils::TestAABBOverlap4f(minA, maxA, mins, maxs, results);
            
            // Process results
            for (size_t k = 0; k < batchSize; k++) {
                if (results[k]) {
                    CollisionPair pair{list[i], list[j + k]};
                    size_t hash = pair.hash();
                    if (pairHashes.insert(hash).second) {
                        pairs.push_back(pair);
                        if (pairs.size() >= max_pairs) break;
                    }
                }
            }
            
            j += batchSize;
        }
    }
    
    // Then check dynamic vs static using spatial bins with SIMD
    for (size_t id = 0; id < mProxyData.aabbs.size(); ++id) {
        // Skip inactive or static proxies
        if (pairs.size() >= max_pairs ||
            std::find(mProxyData.freeIds.begin(), mProxyData.freeIds.end(), id) != mProxyData.freeIds.end() ||
            mProxyData.isStatic[id]) {
            continue;
        }
        
        // Use spatial bins to accelerate static collision checks
        int minBinX = getBinIndex(mProxyData.bounds.minX[id]);
        int maxBinX = getBinIndex(mProxyData.bounds.maxX[id]);
        
        // Load dynamic object bounds once
        float minA[3] = {
            mProxyData.bounds.minX[id],
            mProxyData.bounds.minY[id],
            mProxyData.bounds.minZ[id]
        };
        float maxA[3] = {
            mProxyData.bounds.maxX[id],
            mProxyData.bounds.maxY[id],
            mProxyData.bounds.maxZ[id]
        };
        
        // SIMD working arrays
        const size_t SIMD_WIDTH = 4;
        alignas(32) float mins[12];  // 4 sets of x,y,z
        alignas(32) float maxs[12];  // 4 sets of x,y,z
        alignas(32) int results[4];
        
        // For each overlapping bin
        for (int x = minBinX; x <= maxBinX && pairs.size() < max_pairs; x++) {
            if (x < 0 || x >= static_cast<int>(mSpatialBins.size())) continue;
            
            const auto& bin = mSpatialBins[x];
            const auto& staticObjects = bin.staticObjects;
            size_t numStatic = staticObjects.size();
            
            // Process static objects in batches of 4
            for (size_t i = 0; i < numStatic && pairs.size() < max_pairs; i += SIMD_WIDTH) {
                size_t batchSize = std::min(SIMD_WIDTH, numStatic - i);
                
                // Load batch of static AABBs
                for (size_t k = 0; k < batchSize; k++) {
                    ProxyId staticId = staticObjects[i + k];
                    mins[k] = mProxyData.bounds.minX[staticId];
                    mins[k + 4] = mProxyData.bounds.minY[staticId];
                    mins[k + 8] = mProxyData.bounds.minZ[staticId];
                    maxs[k] = mProxyData.bounds.maxX[staticId];
                    maxs[k + 4] = mProxyData.bounds.maxY[staticId];
                    maxs[k + 8] = mProxyData.bounds.maxZ[staticId];
                }
                
                // Test batch with SIMD
                SimdUtils::TestAABBOverlap4f(minA, maxA, mins, maxs, results);
                
                // Process results
                for (size_t k = 0; k < batchSize; k++) {
                    if (results[k]) {
                        CollisionPair pair{id, staticObjects[i + k]};
                        size_t hash = pair.hash();
                        if (pairHashes.insert(hash).second) {
                            pairs.push_back(pair);
                            if (pairs.size() >= max_pairs) break;
                        }
                    }
                }
            }
            
            if (pairs.size() >= max_pairs) break;
        }
    }
    return pairs;
}

void BroadPhase::sortAxisList(int axis) {
    auto& list = mProxyData.dynamicAxes[axis].objects;
    if (list.empty()) return;

    // First pass: update sort keys and find the range that needs sorting
    int32_t minIdx = static_cast<int32_t>(list.size());
    int32_t maxIdx = -1;
    
    for (size_t i = 0; i < list.size(); ++i) {
        ProxyId id = list[i];
        if (mProxyData.dynamicAxes[axis].needsResort[id]) {
            minIdx = std::min(minIdx, static_cast<int32_t>(i));
            maxIdx = static_cast<int32_t>(i);
            mProxyData.dynamicAxes[axis].needsResort[id] = false;  // Reset flag
        }
    }
    
    // If nothing needs sorting, we're done
    if (minIdx > maxIdx) return;
    
    // Expand range slightly to ensure we catch all necessary moves
    minIdx = std::max(0, minIdx - 1);
    maxIdx = std::min(static_cast<int32_t>(list.size()) - 1, maxIdx + 1);
    
    // Sort the affected range based on the axis
    std::sort(list.begin() + minIdx, list.begin() + maxIdx + 1,
              [this, axis](ProxyId a, ProxyId b) {
                  float minA, minB;
                  switch (axis) {
                      case 0:
                          minA = mProxyData.bounds.minX[a];
                          minB = mProxyData.bounds.minX[b];
                          break;
                      case 1:
                          minA = mProxyData.bounds.minY[a];
                          minB = mProxyData.bounds.minY[b];
                          break;
                      case 2:
                          minA = mProxyData.bounds.minZ[a];
                          minB = mProxyData.bounds.minZ[b];
                          break;
                      default:
                          return false;
                  }
                  return minA < minB;
              });
    
    // Update sort keys in the affected range
    for (int32_t i = minIdx; i <= maxIdx; ++i) {
        ProxyId id = list[i];
        mProxyData.dynamicAxes[axis].sortKeys[id] = i;
    }
}

bool BroadPhase::testOverlap(ProxyId a, ProxyId b) const {
#if PYNOVAGE_MATH_HAS_SSE2
    // Load bounds data for object a
    __m128 minA = _mm_set_ps(0.0f,
                            mProxyData.bounds.minZ[a],
                            mProxyData.bounds.minY[a],
                            mProxyData.bounds.minX[a]);
    __m128 maxA = _mm_set_ps(0.0f,
                            mProxyData.bounds.maxZ[a],
                            mProxyData.bounds.maxY[a],
                            mProxyData.bounds.maxX[a]);

    // Load bounds data for object b
    __m128 minB = _mm_set_ps(0.0f,
                            mProxyData.bounds.minZ[b],
                            mProxyData.bounds.minY[b],
                            mProxyData.bounds.minX[b]);
    __m128 maxB = _mm_set_ps(0.0f,
                            mProxyData.bounds.maxZ[b],
                            mProxyData.bounds.maxY[b],
                            mProxyData.bounds.maxX[b]);

    // Test minB <= maxA and minA <= maxB for all axes simultaneously
    __m128 cmpMinMaxA = _mm_cmple_ps(minB, maxA);
    __m128 cmpMinMaxB = _mm_cmple_ps(minA, maxB);
    __m128 andResult = _mm_and_ps(cmpMinMaxA, cmpMinMaxB);

    // Check if all three axes overlap (ignore w component)
    int mask = _mm_movemask_ps(andResult) & 0x7;
    return mask == 0x7;
#else
    // Fallback scalar implementation
    return !(mProxyData.bounds.minX[a] > mProxyData.bounds.maxX[b] || mProxyData.bounds.minX[b] > mProxyData.bounds.maxX[a] ||
             mProxyData.bounds.minY[a] > mProxyData.bounds.maxY[b] || mProxyData.bounds.minY[b] > mProxyData.bounds.maxY[a] ||
             mProxyData.bounds.minZ[a] > mProxyData.bounds.maxZ[b] || mProxyData.bounds.minZ[b] > mProxyData.bounds.maxZ[a]);
#endif
}

void BroadPhase::insertionSort(std::vector<ProxyId>& list, int axis) {
    for (size_t i = 1; i < list.size(); i++) {
        ProxyId key = list[i];
        float value;
        switch (axis) {
            case 0: value = mProxyData.bounds.minX[key]; break;
            case 1: value = mProxyData.bounds.minY[key]; break;
            case 2: value = mProxyData.bounds.minZ[key]; break;
            default: value = 0.0f;
        }
        
        int32_t j = static_cast<int32_t>(i) - 1;
        bool needsMove = false;
        
        while (j >= 0) {
            float otherValue;
            switch (axis) {
                case 0: otherValue = mProxyData.bounds.minX[list[j]]; break;
                case 1: otherValue = mProxyData.bounds.minY[list[j]]; break;
                case 2: otherValue = mProxyData.bounds.minZ[list[j]]; break;
                default: otherValue = 0.0f;
            }
            
            if (otherValue <= value) break;
            list[j + 1] = list[j];
            needsMove = true;
            j--;
        }
        
        if (needsMove) {
            list[j + 1] = key;
        }
    }
}

size_t BroadPhase::hashPosition(const Vector3& position) const {
    // Simple spatial hash
    int32_t x = static_cast<int32_t>(position.x);
    int32_t y = static_cast<int32_t>(position.y);
    int32_t z = static_cast<int32_t>(position.z);
    return static_cast<size_t>((x * 73856093) ^ (y * 19349663) ^ (z * 83492791));
}

void BroadPhase::insertIntoGrid(ProxyId id) {
    Vector3 cell_pos = mProxyData.centers[id] / mCellSize;
    size_t key = hashPosition(cell_pos);
    mProxyData.gridKeys[id] = key;
    mProxyData.grid[key].push_back(id);
}

void BroadPhase::finalizeBroadPhase() {
    // Update spatial bins if needed
    if (mBinsNeedUpdate) {
        mSpatialBins.clear();
        
        // Insert all active proxies into bins
        for (size_t id = 0; id < mProxyData.aabbs.size(); ++id) {
            // Skip inactive proxies or dynamic objects
            if (std::find(mProxyData.freeIds.begin(), mProxyData.freeIds.end(), id) != mProxyData.freeIds.end() ||
                !mProxyData.isStatic[id]) {
                continue;
            }
            
            int minBinX = getBinIndex(mProxyData.bounds.minX[id]);
            int maxBinX = getBinIndex(mProxyData.bounds.maxX[id]);
            
            // For now just use X-axis bins - if this works well, we can extend to Y and Z
            for (int x = minBinX; x <= maxBinX; x++) {
                insertIntoSpatialBin(x, id);
            }
        }
        
        mBinsNeedUpdate = false;
    }
    
    // Sort only the axes that were marked as dirty
    for (int axis = 0; axis < 3; ++axis) {
        if (mDirtyAxes[axis]) {
            sortAxisList(axis);
            mDirtyAxes[axis] = false;
        }
    }
}

void BroadPhase::removeFromGrid(ProxyId id) {
    auto it = mProxyData.grid.find(mProxyData.gridKeys[id]);
    if (it != mProxyData.grid.end()) {
        auto& cell = it->second;
        cell.remove(id);
        if (cell.empty()) {
            mProxyData.grid.erase(it);
        }
    }
}

} // namespace geometry
} // namespace math
} // namespace pynovage
