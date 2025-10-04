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
    mProxyData.minX.reserve(INITIAL_CAPACITY);
    mProxyData.minY.reserve(INITIAL_CAPACITY);
    mProxyData.minZ.reserve(INITIAL_CAPACITY);
    mProxyData.maxX.reserve(INITIAL_CAPACITY);
    mProxyData.maxY.reserve(INITIAL_CAPACITY);
    mProxyData.maxZ.reserve(INITIAL_CAPACITY);
    
    // Reserve space for dynamic object data
    for (int i = 0; i < 3; ++i) {
        mProxyData.sortKeys[i].reserve(INITIAL_CAPACITY);
        mProxyData.needsResort[i].reserve(INITIAL_CAPACITY);
        mDynamicList[i].reserve(INITIAL_CAPACITY);
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
            (mProxyData.maxX[id] - mProxyData.minX[id]) * 0.5f,
            (mProxyData.maxY[id] - mProxyData.minY[id]) * 0.5f,
            (mProxyData.maxZ[id] - mProxyData.minZ[id]) * 0.5f
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
        mProxyData.minX.resize(new_size);
        mProxyData.minY.resize(new_size);
        mProxyData.minZ.resize(new_size);
        mProxyData.maxX.resize(new_size);
        mProxyData.maxY.resize(new_size);
        mProxyData.maxZ.resize(new_size);
        mProxyData.gridKeys.resize(new_size);
        
        for (int i = 0; i < 3; ++i) {
            mProxyData.sortKeys[i].resize(new_size);
            mProxyData.needsResort[i].resize(new_size);
        }
    }
    
    // Store basic data
    mProxyData.isStatic[id] = is_static;
    mProxyData.aabbs[id] = aabb;
    mProxyData.centers[id] = (aabb.min + aabb.max) * 0.5f;
    
    // Store bounds data
    mProxyData.minX[id] = aabb.min.x;
    mProxyData.minY[id] = aabb.min.y;
    mProxyData.minZ[id] = aabb.min.z;
    mProxyData.maxX[id] = aabb.max.x;
    mProxyData.maxY[id] = aabb.max.y;
    mProxyData.maxZ[id] = aabb.max.z;
    
    // Compute Morton code
    mProxyData.mortonCodes[id] = computeMortonCode(mProxyData.centers[id]);
    
    if (is_static) {
        // Add to static grid
        insertIntoGrid(id);
    } else {
        // Initialize dynamic object data
        for (int axis = 0; axis < 3; ++axis) {
            mProxyData.sortKeys[axis][id] = static_cast<int32_t>(mDynamicList[axis].size());
            mProxyData.needsResort[axis][id] = false;
            mDynamicList[axis].push_back(id);
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
            auto& list = mDynamicList[axis];
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
    float oldMinX = mProxyData.minX[id];
    float oldMinY = mProxyData.minY[id];
    float oldMinZ = mProxyData.minZ[id];
    
    // Update bounds
    mProxyData.minX[id] = aabb.min.x;
    mProxyData.minY[id] = aabb.min.y;
    mProxyData.minZ[id] = aabb.min.z;
    mProxyData.maxX[id] = aabb.max.x;
    mProxyData.maxY[id] = aabb.max.y;
    mProxyData.maxZ[id] = aabb.max.z;
    
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
        
        mProxyData.needsResort[0][id] = needsResortX;
        mProxyData.needsResort[1][id] = needsResortY;
        mProxyData.needsResort[2][id] = needsResortZ;
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
    
    // Check dynamic vs dynamic using SAP with optimized cache access
    const auto& list = mDynamicList[0];  // X-axis list
    const size_t listSize = list.size();

    // Pre-compute and cache min/max bounds for current iteration
    struct BoundsCache {
        float minX, maxX;
        float minY, maxY;
        float minZ, maxZ;
    };
    std::vector<BoundsCache> cache(listSize);
    
    // Build cache - this should be more cache-friendly than random access
    for (size_t i = 0; i < listSize; i++) {
        ProxyId id = list[i];
        cache[i].minX = mProxyData.minX[id];
        cache[i].maxX = mProxyData.maxX[id];
        cache[i].minY = mProxyData.minY[id];
        cache[i].maxY = mProxyData.maxY[id];
        cache[i].minZ = mProxyData.minZ[id];
        cache[i].maxZ = mProxyData.maxZ[id];
    }
    
    // Do sweep with cached bounds
    for (size_t i = 0; i < listSize && pairs.size() < max_pairs; i++) {
        const float max_i = cache[i].maxX;
        const float min_i_y = cache[i].minY;
        const float max_i_y = cache[i].maxY;
        const float min_i_z = cache[i].minZ;
        const float max_i_z = cache[i].maxZ;
        
        for (size_t j = i + 1; j < listSize && pairs.size() < max_pairs; j++) {
            // Quick X-axis check
            if (cache[j].minX > max_i) break;
            
            // Y and Z axes check using cached values
            if (min_i_y <= cache[j].maxY && cache[j].minY <= max_i_y &&
                min_i_z <= cache[j].maxZ && cache[j].minZ <= max_i_z) {
                CollisionPair pair{list[i], list[j]};
                size_t hash = pair.hash();
                if (pairHashes.insert(hash).second) {
                    pairs.push_back(pair);
                }
            }
        }
    }
    
    // Then check dynamic vs static using spatial bins
    for (size_t id = 0; id < mProxyData.aabbs.size(); ++id) {
        // Skip inactive or static proxies
        if (pairs.size() >= max_pairs ||
            std::find(mProxyData.freeIds.begin(), mProxyData.freeIds.end(), id) != mProxyData.freeIds.end() ||
            mProxyData.isStatic[id]) {
            continue;
        }
        
        // Use spatial bins to accelerate static collision checks
        int minBinX = getBinIndex(mProxyData.minX[id]);
        int maxBinX = getBinIndex(mProxyData.maxX[id]);
        
        // For each overlapping bin
        for (int x = minBinX; x <= maxBinX; x++) {
            if (x < 0 || x >= static_cast<int>(mSpatialBins.size())) continue;
            
            const auto& bin = mSpatialBins[x];
            
            // For small bins, just test all pairs
            for (ProxyId staticId : bin.staticObjects) {
                if (testOverlap(id, staticId)) {
                    CollisionPair pair{id, staticId};
                    size_t hash = pair.hash();
                    if (pairHashes.insert(hash).second) {
                        pairs.push_back(pair);
                        if (pairs.size() >= max_pairs) break;
                    }
                }
            }
            if (pairs.size() >= max_pairs) break;
        }
    }
    return pairs;
}

void BroadPhase::sortAxisList(int axis) {
    auto& list = mDynamicList[axis];
    if (list.empty()) return;

    // First pass: update sort keys and find the range that needs sorting
    int32_t minIdx = static_cast<int32_t>(list.size());
    int32_t maxIdx = -1;
    
    for (size_t i = 0; i < list.size(); ++i) {
        ProxyId id = list[i];
        if (mProxyData.needsResort[axis][id]) {
            minIdx = std::min(minIdx, static_cast<int32_t>(i));
            maxIdx = static_cast<int32_t>(i);
            mProxyData.needsResort[axis][id] = false;  // Reset flag
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
                          minA = mProxyData.minX[a];
                          minB = mProxyData.minX[b];
                          break;
                      case 1:
                          minA = mProxyData.minY[a];
                          minB = mProxyData.minY[b];
                          break;
                      case 2:
                          minA = mProxyData.minZ[a];
                          minB = mProxyData.minZ[b];
                          break;
                      default:
                          return false;
                  }
                  return minA < minB;
              });
    
    // Update sort keys in the affected range
    for (int32_t i = minIdx; i <= maxIdx; ++i) {
        ProxyId id = list[i];
        mProxyData.sortKeys[axis][id] = i;
    }
}

bool BroadPhase::testOverlap(ProxyId a, ProxyId b) const {
    // Quick AABB overlap test using SoA layout
    return !(mProxyData.minX[a] > mProxyData.maxX[b] || mProxyData.minX[b] > mProxyData.maxX[a] ||
             mProxyData.minY[a] > mProxyData.maxY[b] || mProxyData.minY[b] > mProxyData.maxY[a] ||
             mProxyData.minZ[a] > mProxyData.maxZ[b] || mProxyData.minZ[b] > mProxyData.maxZ[a]);
}

void BroadPhase::insertionSort(std::vector<ProxyId>& list, int axis) {
    for (size_t i = 1; i < list.size(); i++) {
        ProxyId key = list[i];
        float value;
        switch (axis) {
            case 0: value = mProxyData.minX[key]; break;
            case 1: value = mProxyData.minY[key]; break;
            case 2: value = mProxyData.minZ[key]; break;
            default: value = 0.0f;
        }
        
        int32_t j = static_cast<int32_t>(i) - 1;
        bool needsMove = false;
        
        while (j >= 0) {
            float otherValue;
            switch (axis) {
                case 0: otherValue = mProxyData.minX[list[j]]; break;
                case 1: otherValue = mProxyData.minY[list[j]]; break;
                case 2: otherValue = mProxyData.minZ[list[j]]; break;
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
    mGrid[key].push_back(id);
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
            
            int minBinX = getBinIndex(mProxyData.minX[id]);
            int maxBinX = getBinIndex(mProxyData.maxX[id]);
            
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
    auto it = mGrid.find(mProxyData.gridKeys[id]);
    if (it != mGrid.end()) {
        auto& cell = it->second;
        cell.erase(std::remove(cell.begin(), cell.end(), id), cell.end());
        if (cell.empty()) {
            mGrid.erase(it);
        }
    }
}

} // namespace geometry
} // namespace math
} // namespace pynovage
