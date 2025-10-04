#include "geometry/broad_phase.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

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
    std::unordered_set<size_t> pairHashes;  // Track pairs we've already checked
    if (max_pairs == 0) {
        max_pairs = std::numeric_limits<size_t>::max();
    }
    
    // First check dynamic vs dynamic using SAP
    // Only need to check X-axis since if objects don't overlap on X, they can't overlap at all
    const auto& list = mDynamicProxies[0];  // X-axis list
    for (size_t i = 0; i < list.size() && pairs.size() < max_pairs; i++) {
        float max_i = list[i]->max[0];
        for (size_t j = i + 1; j < list.size() && pairs.size() < max_pairs; j++) {
            if (list[j]->min[0] > max_i) break; // No more overlaps possible on X-axis
            
            // Quick SIMD check for Y and Z axes
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
    
    // Then check dynamic vs static using grid, with optimizations
    for (const auto& proxy : mProxies) {
        if (pairs.size() >= max_pairs) break;
        if (proxy->isStatic) continue;
        
        // Get grid cells that this dynamic object overlaps
        Vector3 min_cell = proxy->aabb.min / mCellSize;
        Vector3 max_cell = proxy->aabb.max / mCellSize;
        
        // Pre-compute cell range to avoid excessive divisions
        int min_x = static_cast<int>(min_cell.x);
        int max_x = static_cast<int>(max_cell.x);
        int min_y = static_cast<int>(min_cell.y);
        int max_y = static_cast<int>(max_cell.y);
        int min_z = static_cast<int>(min_cell.z);
        int max_z = static_cast<int>(max_cell.z);
        
        // Early exit if object spans too many cells (indicating it's too large)
        int cell_span = (max_x - min_x + 1) * (max_y - min_y + 1) * (max_z - min_z + 1);
        if (cell_span > 27) {  // More than 3x3x3 grid? Use SAP instead
            for (const auto& other : mProxies) {
                if (!other->isStatic) continue;
                
                if (testOverlap(proxy, other)) {
                    CollisionPair pair{proxy, other};
                    size_t hash = pair.hash();
                    if (pairHashes.insert(hash).second) {
                        pairs.push_back(pair);
                    }
                }
            }
            continue;
        }
        
        // Process grid cells
        for (int x = min_x; x <= max_x; x++) {
            for (int y = min_y; y <= max_y; y++) {
                for (int z = min_z; z <= max_z; z++) {
                    Vector3 cell_pos(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                    size_t key = hashPosition(cell_pos);
                    
                    auto it = mGrid.find(key);
                    if (it != mGrid.end()) {
                        const auto& cell = it->second;
                        // If cell has too many objects, use SAP for this cell
                        if (cell.size() > 16) {
                            float max_proxy = proxy->max[0];
                            for (const auto& static_proxy : cell) {
                                if (static_proxy->min[0] > max_proxy) continue;
                                
                                if (proxy->min[1] <= static_proxy->max[1] && static_proxy->min[1] <= proxy->max[1] &&
                                    proxy->min[2] <= static_proxy->max[2] && static_proxy->min[2] <= proxy->max[2]) {
                                    CollisionPair pair{proxy, static_proxy};
                                    size_t hash = pair.hash();
                                    if (pairHashes.insert(hash).second) {
                                        pairs.push_back(pair);
                                    }
                                }
                            }
                        } else {
                            // For small cells, just test all pairs
                            for (const auto& static_proxy : cell) {
                                if (testOverlap(proxy, static_proxy)) {
                                    CollisionPair pair{proxy, static_proxy};
                                    size_t hash = pair.hash();
                                    if (pairHashes.insert(hash).second) {
                                        pairs.push_back(pair);
                                    }
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