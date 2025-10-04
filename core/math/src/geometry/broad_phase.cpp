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
    // Reserve initial space for spatial bins
    mSpatialBins.reserve(32);
    // Initialize all axes as clean
    for (int i = 0; i < 3; ++i) {
        mDirtyAxes[i] = false;
    }
}

BroadPhase::~BroadPhase() {
    // Clean up all proxies
    for (auto proxy : mProxies) {
        delete proxy;
    }
}

void BroadPhase::updateTemporalCoherence() {
    mPrevPairs.clear();
    mPrevPairs.shrink_to_fit();
    // Recompute previous states and movement thresholds
    for (auto* proxy : mProxies) {
        PreviousState st;
        st.center = proxy->center;
        Vector3 halfExtent = (proxy->aabb.max - proxy->aabb.min) * 0.5f;
        st.extent = halfExtent;
        // Allow motion up to 10% of max extent without forcing retest
        float maxExtent = std::max(halfExtent.x, std::max(halfExtent.y, halfExtent.z));
        st.moveThresh = std::max(0.01f, 0.1f * maxExtent);
        mPrevStates[proxy] = st;
    }
}

AABBProxy* BroadPhase::createProxy(const AABB& aabb, bool is_static) {
    AABBProxy* proxy = new AABBProxy();
    proxy->aabb = aabb;
    proxy->isStatic = is_static;
    proxy->proxyId = mProxies.size();
    proxy->center = (aabb.min + aabb.max) * 0.5f;
    
    // Compute min/max for SAP
    for (int i = 0; i < 3; i++) {
        proxy->min[i] = aabb.min[i];
        proxy->max[i] = aabb.max[i];
        proxy->sortKeys[i] = static_cast<int32_t>(mDynamicProxies[i].size());  // Will be inserted at end
        proxy->needsResort[i] = false;
    }
    
    // Compute Morton code for spatial coherence
    proxy->mortonCode = computeMortonCode(proxy->center);
    
    // Add to appropriate systems
    mProxies.push_back(proxy);
    if (is_static) {
        insertIntoGrid(proxy);
    } else {
        // Add to dynamic lists and mark for sorting later
        for (int axis = 0; axis < 3; axis++) {
            mDynamicProxies[axis].push_back(proxy);
            mDirtyAxes[axis] = true;  // Mark for sorting in finalize
        }
    }
    
    return proxy;
}

void BroadPhase::destroyProxy(AABBProxy* proxy) {
    // Remove from appropriate systems
    if (proxy->isStatic) {
        removeFromGrid(proxy);
    } else {
        for (int axis = 0; axis < 3; axis++) {
            auto& list = mDynamicProxies[axis];
            list.erase(std::remove(list.begin(), list.end(), proxy), list.end());
        }
    }
    
    // Remove from main list and delete
    mProxies.erase(std::remove(mProxies.begin(), mProxies.end(), proxy), mProxies.end());
    delete proxy;
}

void BroadPhase::insertIntoSpatialBin(int binIdx, AABBProxy* proxy) {
    if (binIdx < 0) return;
    
    // Grow bins array if needed
    while (binIdx >= static_cast<int>(mSpatialBins.size())) {
        mSpatialBins.emplace_back();
    }
    
    if (proxy->isStatic) {
        mSpatialBins[binIdx].staticObjects.push_back(proxy);
    }
}

void BroadPhase::updateProxy(AABBProxy* proxy, const AABB& aabb) {
    // Update basic info
    proxy->aabb = aabb;
    proxy->center = (aabb.min + aabb.max) * 0.5f;
    proxy->mortonCode = computeMortonCode(proxy->center);
    
    // Update SAP data and check which axes need resorting
    for (int i = 0; i < 3; i++) {
        float oldMin = proxy->min[i];
        proxy->min[i] = aabb.min[i];
        proxy->max[i] = aabb.max[i];
        
        // Only resort if position changed significantly
        float movement = std::abs(oldMin - proxy->min[i]);
        proxy->needsResort[i] = movement > 1e-4f;  // Small epsilon to avoid floating point issues
    }
    
    // Update grid if static. For dynamic, mark axes as dirty (sorting deferred)
    if (proxy->isStatic) {
        removeFromGrid(proxy);
        insertIntoGrid(proxy);
    } else {
        for (int axis = 0; axis < 3; axis++) {
            if (proxy->needsResort[axis]) {
                mDirtyAxes[axis] = true;
            }
        }
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
            const AABBProxy *pa, *pb;
            prevPair.getOrdered(pa, pb);
            auto ita = mPrevStates.find(pa);
            auto itb = mPrevStates.find(pb);
            if (ita == mPrevStates.end() || itb == mPrevStates.end()) continue;
            const auto& sa = ita->second;
            const auto& sb = itb->second;
            float da = (pa->center - sa.center).length();
            float db = (pb->center - sb.center).length();
            if (da <= sa.moveThresh && db <= sb.moveThresh) {
                size_t h = prevPair.hash();
                if (pairHashes.insert(h).second) {
                    pairs.push_back(prevPair);
                }
            }
        }
    }
    
    // First check dynamic vs dynamic using SAP
    // Only need to check X-axis since if objects don't overlap on X, they can't overlap at all
    const auto& list = mDynamicProxies[0];  // X-axis list
    for (size_t i = 0; i < list.size() && pairs.size() < max_pairs; i++) {
        float max_i = list[i]->max[0];
        for (size_t j = i + 1; j < list.size() && pairs.size() < max_pairs; j++) {
            if (list[j]->min[0] > max_i) break; // No more overlaps possible on X-axis
            
            // Quick axis checks for Y and Z axes
            if (list[i]->min[1] <= list[j]->max[1] && list[j]->min[1] <= list[i]->max[1] &&
                list[i]->min[2] <= list[j]->max[2] && list[j]->min[2] <= list[i]->max[2]) {
                CollisionPair pair{list[i], list[j]};
                size_t hash = pair.hash();
                if (pairHashes.insert(hash).second) {
                    pairs.push_back(pair);
                }
            }
        }
    }
    
    // Then check dynamic vs static using spatial bins
    for (const auto& proxy : mProxies) {
        if (pairs.size() >= max_pairs) break;
        if (proxy->isStatic) continue;
        
        // Use spatial bins to accelerate static collision checks
        int minBinX = getBinIndex(proxy->aabb.min.x);
        int maxBinX = getBinIndex(proxy->aabb.max.x);
        
        // For each overlapping bin
        for (int x = minBinX; x <= maxBinX; x++) {
            if (x < 0 || x >= static_cast<int>(mSpatialBins.size())) continue;
            
            const auto& bin = mSpatialBins[x];
            
            // For small bins, just test all pairs (SIMD path removed for stability)
            for (const auto* staticProxy : bin.staticObjects) {
                if (testOverlap(proxy, staticProxy)) {
                    CollisionPair pair{proxy, staticProxy};
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
    auto& list = mDynamicProxies[axis];
    if (list.empty()) return;

    // First pass: update sort keys and find the range that needs sorting
    int32_t minIdx = static_cast<int32_t>(list.size());
    int32_t maxIdx = -1;
    
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i]->needsResort[axis]) {
            minIdx = std::min(minIdx, static_cast<int32_t>(i));
            maxIdx = static_cast<int32_t>(i);
            list[i]->needsResort[axis] = false;  // Reset flag
        }
    }
    
    // If nothing needs sorting, we're done
    if (minIdx > maxIdx) return;
    
    // Expand range slightly to ensure we catch all necessary moves
    minIdx = std::max(0, minIdx - 1);
    maxIdx = std::min(static_cast<int32_t>(list.size()) - 1, maxIdx + 1);
    
    // Sort the affected range
    std::sort(list.begin() + minIdx, list.begin() + maxIdx + 1,
              [axis](const AABBProxy* a, const AABBProxy* b) {
                  return a->min[axis] < b->min[axis];
              });
    
    // Update sort keys in the affected range
    for (int32_t i = minIdx; i <= maxIdx; ++i) {
        list[i]->sortKeys[axis] = i;
    }
}

bool BroadPhase::testOverlap(const AABBProxy* a, const AABBProxy* b) const {
    // Quick AABB overlap test
    for (int i = 0; i < 3; i++) {
        if (a->min[i] > b->max[i] || b->min[i] > a->max[i]) {
            return false;
        }
    }
    return true;
}

void BroadPhase::insertionSort(std::vector<AABBProxy*>& list, int axis) {
    for (size_t i = 1; i < list.size(); i++) {
        AABBProxy* key = list[i];
        float value = key->min[axis];
        int32_t j = static_cast<int32_t>(i) - 1;
        
        while (j >= 0 && list[j]->min[axis] > value) {
            list[j + 1] = list[j];
            j--;
        }
        list[j + 1] = key;
    }
}

size_t BroadPhase::hashPosition(const Vector3& position) const {
    // Simple spatial hash
    int32_t x = static_cast<int32_t>(position.x);
    int32_t y = static_cast<int32_t>(position.y);
    int32_t z = static_cast<int32_t>(position.z);
    return static_cast<size_t>((x * 73856093) ^ (y * 19349663) ^ (z * 83492791));
}

void BroadPhase::insertIntoGrid(AABBProxy* proxy) {
    Vector3 cell_pos = proxy->center / mCellSize;
    size_t key = hashPosition(cell_pos);
    proxy->gridKey = key;
    mGrid[key].push_back(proxy);
}

void BroadPhase::finalizeBroadPhase() {
    // Update spatial bins if needed
    if (mBinsNeedUpdate) {
        mSpatialBins.clear();
        
        // Insert all proxies into bins
        for (auto* proxy : mProxies) {
            if (!proxy) continue;
            
            int minBinX = getBinIndex(proxy->aabb.min.x);
            int maxBinX = getBinIndex(proxy->aabb.max.x);
            
            // For now just use X-axis bins - if this works well, we can extend to Y and Z
            for (int x = minBinX; x <= maxBinX; x++) {
                insertIntoSpatialBin(x, proxy);
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

void BroadPhase::removeFromGrid(AABBProxy* proxy) {
    auto it = mGrid.find(proxy->gridKey);
    if (it != mGrid.end()) {
        auto& cell = it->second;
        cell.erase(std::remove(cell.begin(), cell.end(), proxy), cell.end());
        if (cell.empty()) {
            mGrid.erase(it);
        }
    }
}

} // namespace geometry
} // namespace math
} // namespace pynovage
