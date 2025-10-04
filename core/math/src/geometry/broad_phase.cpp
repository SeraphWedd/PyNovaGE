#include "geometry/broad_phase.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace pynovage {
namespace math {
namespace geometry {

namespace {
    // Morton code look-up tables for fast computation
    const uint32_t gMorton256_x[256] = {
        0x00000000, 0x00000001, 0x00000008, 0x00000009, 0x00000040, 0x00000041, 0x00000048, 0x00000049,
        // ... (rest of the table would be here, omitted for brevity)
    };
    const uint32_t gMorton256_y[256] = {
        0x00000000, 0x00000002, 0x00000010, 0x00000012, 0x00000080, 0x00000082, 0x00000090, 0x00000092,
        // ...
    };
    const uint32_t gMorton256_z[256] = {
        0x00000000, 0x00000004, 0x00000020, 0x00000024, 0x00000100, 0x00000104, 0x00000120, 0x00000124,
        // ...
    };
}

BroadPhase::BroadPhase(float cell_size) : mCellSize(cell_size) {}

BroadPhase::~BroadPhase() {
    // Clean up all proxies
    for (auto proxy : mProxies) {
        delete proxy;
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
        proxy->sortKeys[i] = 0;
    }
    
    // Compute Morton code for spatial coherence
    proxy->mortonCode = computeMortonCode(proxy->center);
    
    // Add to appropriate systems
    mProxies.push_back(proxy);
    if (is_static) {
        insertIntoGrid(proxy);
    } else {
        for (int axis = 0; axis < 3; axis++) {
            mDynamicProxies[axis].push_back(proxy);
            sortAxisList(axis);
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

void BroadPhase::updateProxy(AABBProxy* proxy, const AABB& aabb) {
    // Update basic info
    proxy->aabb = aabb;
    proxy->center = (aabb.min + aabb.max) * 0.5f;
    proxy->mortonCode = computeMortonCode(proxy->center);
    
    // Update SAP data
    for (int i = 0; i < 3; i++) {
        proxy->min[i] = aabb.min[i];
        proxy->max[i] = aabb.max[i];
    }
    
    // Update grid if static, resort if dynamic
    if (proxy->isStatic) {
        removeFromGrid(proxy);
        insertIntoGrid(proxy);
    } else {
        for (int axis = 0; axis < 3; axis++) {
            sortAxisList(axis);
        }
    }
}

std::vector<BroadPhase::CollisionPair> BroadPhase::findPotentialCollisions(size_t max_pairs) {
    std::vector<CollisionPair> pairs;
    if (max_pairs == 0) {
        max_pairs = std::numeric_limits<size_t>::max();
    }
    
    // First check dynamic vs dynamic using SAP
    for (int axis = 0; axis < 3; axis++) {
        const auto& list = mDynamicProxies[axis];
        for (size_t i = 0; i < list.size() && pairs.size() < max_pairs; i++) {
            float max_i = list[i]->max[axis];
            for (size_t j = i + 1; j < list.size() && pairs.size() < max_pairs; j++) {
                if (list[j]->min[axis] > max_i) break; // No more overlaps possible
                
                if (testOverlap(list[i], list[j])) {
                    CollisionPair pair{list[i], list[j]};
                    // Check if this pair is already added
                    if (std::find(pairs.begin(), pairs.end(), pair) == pairs.end()) {
                        pairs.push_back(pair);
                    }
                }
            }
        }
    }
    
    // Then check dynamic vs static using grid
    for (const auto& proxy : mProxies) {
        if (pairs.size() >= max_pairs) break;
        if (proxy->isStatic) continue;
        
        // Get grid cells that this dynamic object overlaps
        Vector3 min_cell = proxy->aabb.min / mCellSize;
        Vector3 max_cell = proxy->aabb.max / mCellSize;
        
        for (int x = static_cast<int>(min_cell.x); x <= static_cast<int>(max_cell.x); x++) {
            for (int y = static_cast<int>(min_cell.y); y <= static_cast<int>(max_cell.y); y++) {
                for (int z = static_cast<int>(min_cell.z); z <= static_cast<int>(max_cell.z); z++) {
                    Vector3 cell_pos(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                    size_t key = hashPosition(cell_pos);
                    
                    auto it = mGrid.find(key);
                    if (it != mGrid.end()) {
                        for (const auto& static_proxy : it->second) {
                            if (testOverlap(proxy, static_proxy)) {
                                CollisionPair pair{proxy, static_proxy};
                                if (std::find(pairs.begin(), pairs.end(), pair) == pairs.end()) {
                                    pairs.push_back(pair);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return pairs;
}

uint32_t BroadPhase::computeMortonCode(const Vector3& position) const {
    // Scale position to [0,255] range for lookup tables
    Vector3 offset(1000.0f, 1000.0f, 1000.0f);
    Vector3 scaled = (position + offset) * (255.0f / 2000.0f);
    uint32_t x = static_cast<uint32_t>(std::min(255.0f, std::max(0.0f, scaled.x)));
    uint32_t y = static_cast<uint32_t>(std::min(255.0f, std::max(0.0f, scaled.y)));
    uint32_t z = static_cast<uint32_t>(std::min(255.0f, std::max(0.0f, scaled.z)));
    
    // Use lookup tables for each 8 bits
    return gMorton256_x[x] | gMorton256_y[y] | gMorton256_z[z];
}

void BroadPhase::sortAxisList(int axis) {
    insertionSort(mDynamicProxies[axis], axis);
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