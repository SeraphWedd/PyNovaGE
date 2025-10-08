#pragma once

#include "spatial_partitioning.hpp"
#include "../vector3.hpp"
#include "../matrix4.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <cmath>

namespace pynovage {
namespace math {
namespace geometry {

template<typename T>
class SpatialHash : public SpatialContainer<T> {
public:
    explicit SpatialHash(const SpatialConfig& config = SpatialConfig())
        : config_(config), objectCount_(0), totalBounds_(Vector3(0,0,0), Vector3(0,0,0)) {
        // Set reasonable grid size based on cell size
        cellSizeInv_ = 1.0f / config_.cellSize;
        gridSize_ = 32;  // Default to 32³ grid (32,768 cells)
        gridSizeSq_ = gridSize_ * gridSize_;
        
        // Pre-allocate initial cell capacity
        cells_.reserve(64);
        
        // Initialize object density tracking
        objectsPerCell_ = 0;
        maxObjectOverlap_ = 1;
    }
    
    void insert(std::unique_ptr<SpatialObject<T>> object) override {
        // Reuse thread-local storage for cell indices to avoid allocation
        storage_.cellIndices.clear();
        
        // Throttle grid adjustment: only on powers of two to avoid per-insert cost
        std::size_t oldGridSize = gridSize_;
        if (objectCount_ == 0 || ((objectCount_ & (objectCount_ - 1)) == 0)) {
            adjustGridSize();
        }
        
        // Update total bounds
        const AABB& objBounds = object->getBounds();
        if (objectCount_ == 0) {
            totalBounds_ = objBounds;
        } else {
            totalBounds_.min = Vector3(
                std::min(totalBounds_.min.x, objBounds.min.x),
                std::min(totalBounds_.min.y, objBounds.min.y),
                std::min(totalBounds_.min.z, objBounds.min.z)
            );
            totalBounds_.max = Vector3(
                std::max(totalBounds_.max.x, objBounds.max.x),
                std::max(totalBounds_.max.y, objBounds.max.y),
                std::max(totalBounds_.max.z, objBounds.max.z)
            );
        }
        
        // If grid size changed, rebuild
        if (oldGridSize != gridSize_ && objectCount_ > 0) {
            // Store object for after rebuild
            std::vector<std::unique_ptr<SpatialObject<T>>> objects;
            objects.reserve(objectCount_ + 1);
            objects.push_back(std::move(object));
            
            // Collect existing objects
            for (auto& pair : objectMap_) {
                objects.push_back(std::move(pair.second));
            }
            
            // Clear and rebuild
            clear();
            for (auto& obj : objects) {
                const SpatialObject<T>* objPtr = obj.get();
                std::size_t id = reinterpret_cast<std::size_t>(objPtr);
                objectMap_[id] = std::move(obj);
                objectCount_++;
                
        // Calculate cell indices without padding for insert (fewer cells touched)
        std::vector<std::size_t> cellIndices;
        cellIndices.reserve(8);
        getCellIndicesNoPad(objPtr->getBounds(), cellIndices);
                
                // Add to cells
                for (std::size_t index : cellIndices) {
                    cells_[index].objects.push_back(objPtr);
                }
                
                objectCells_[id] = std::move(cellIndices);
            }
            return;
        }
        
        // Normal insertion path
        const SpatialObject<T>* objPtr = object.get();
        std::size_t id = reinterpret_cast<std::size_t>(objPtr);
        objectMap_[id] = std::move(object);
        objectCount_++;
        
        // Calculate cell indices without padding (reusing thread-local storage)
        getCellIndicesNoPad(objBounds, storage_.cellIndices);
        
        // Add to cells
        for (std::size_t index : storage_.cellIndices) {
            auto it = cells_.try_emplace(index).first;
            it->second.objects.push_back(objPtr);
        }
        
        // Copy indices for tracking (need a copy since storage_ will be reused)
        objectCells_[id] = storage_.cellIndices;
    }
    
    void remove(const SpatialObject<T>* object) override {
        std::size_t id = reinterpret_cast<std::size_t>(object);
        auto it = objectCells_.find(id);
        if (it == objectCells_.end()) return;
        
        // Remove from cells
        const auto& cellIndices = it->second;
        for (std::size_t index : cellIndices) {
            auto& cell = cells_[index];
            // Fast remove since order doesn't matter
            for (std::size_t i = 0; i < cell.objects.size(); ++i) {
                if (cell.objects[i] == object) {
                    if (i < cell.objects.size() - 1) {
                        cell.objects[i] = cell.objects.back();
                    }
                    cell.objects.pop_back();
                    break;
                }
            }
        }
        
        // Remove from maps
        objectCells_.erase(it);
        objectMap_.erase(id);
        objectCount_--;
    }
    
    // Helper to round up to next power of 2 (C++17 compatible)
    static std::size_t nextPowerOfTwo(std::size_t v) {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }
    
    void update(const SpatialObject<T>* object) override {
        std::size_t id = reinterpret_cast<std::size_t>(object);
        auto it = objectMap_.find(id);
        if (it == objectMap_.end()) return;
        
        // Get new cell indices
        const AABB& bounds = object->getBounds();
        std::vector<std::size_t> newCellIndices;
        newCellIndices.reserve(8);  // Most objects overlap with <= 8 cells
        getCellIndices(bounds, newCellIndices);
        
        // Get old cell indices
        auto oldIt = objectCells_.find(id);
        if (oldIt != objectCells_.end()) {
            const auto& oldIndices = oldIt->second;
            
            // Remove from old cells with fast removal
            for (std::size_t index : oldIndices) {
                auto& cell = cells_[index];
                for (std::size_t i = 0; i < cell.objects.size(); ++i) {
                    if (cell.objects[i] == object) {
                        if (i < cell.objects.size() - 1) {
                            cell.objects[i] = cell.objects.back();
                        }
                        cell.objects.pop_back();
                        break;
                    }
                }
            }
        }
        
        // Add to new cells
        for (std::size_t index : newCellIndices) {
            cells_[index].objects.push_back(object);
        }
        
        objectCells_[id] = std::move(newCellIndices);
    }
    
    void clear() override {
        cells_.clear();
        cells_.reserve(64);
        objectMap_.clear();
        objectCells_.clear();
        objectCount_ = 0;
    }
    
    void reserveObjects(std::size_t count) override {
        // Reserve internal containers to reduce rehashing during bulk inserts
        objectMap_.reserve(count);
        objectCells_.reserve(count);
        // Heuristic: expect ~4 cells per object on average
        std::size_t expectedCells = count * 4;
        if (expectedCells > cells_.bucket_count()) {
            cells_.reserve(expectedCells);
        }
    }
    
    void query(const SpatialQuery<T>& query,
               std::vector<const SpatialObject<T>*>& results) const override {
        // Fast path for VolumeQuery which is most common
        if (auto* volumeQuery = dynamic_cast<const VolumeQuery<T>*>(&query)) {
            queryVolume(volumeQuery->getBounds(), query, results);
            return;
        }
        
        // Handle point queries
        if (auto* pointQuery = dynamic_cast<const PointQuery<T>*>(&query)) {
            const Vector3& point = pointQuery->getPoint();
            
            // Check if point is in total bounds
            if (!totalBounds_.contains(point)) return;
            
            // Get potential cells around point
            std::size_t baseIndex = positionToCellIndex(point);
            static const int offsets[8][3] = {
                {0,0,0}, {1,0,0}, {0,1,0}, {1,1,0},
                {0,0,1}, {1,0,1}, {0,1,1}, {1,1,1}
            };
            
            static thread_local std::unordered_set<const SpatialObject<T>*> processed;
            processed.clear();
            processed.reserve(32);
            
            // Check neighboring cells for objects that might contain the point
            std::size_t x = baseIndex % gridSize_;
            std::size_t y = (baseIndex / gridSize_) % gridSize_;
            std::size_t z = baseIndex / gridSizeSq_;
            
            for (const auto& offset : offsets) {
                std::size_t newX = (x + offset[0]) % gridSize_;
                std::size_t newY = (y + offset[1]) % gridSize_;
                std::size_t newZ = (z + offset[2]) % gridSize_;
                std::size_t index = newX + newY * gridSize_ + newZ * gridSizeSq_;
                
                auto it = cells_.find(index);
                if (it == cells_.end()) continue;
                
                for (const auto* obj : it->second.objects) {
                    if (processed.insert(obj).second && 
                        obj->contains(point) && 
                        query.shouldAcceptObject(*obj)) {
                        results.push_back(obj);
                    }
                }
            }
            return;
        }
        
        // Handle ray queries without internal ray accessors
        if (dynamic_cast<const RayQuery<T>*>(&query)) {
            // Fall back to scanning potentially overlapping cells using the query's traversal predicate
            // Build a coarse AABB using total bounds; if empty just scan all objects
            if (objectCount_ == 0) return;
            
            // Conservative approach: iterate non-empty cells and test objects via shouldAcceptObject
            for (const auto& pair : cells_) {
                const auto& cell = pair.second;
                if (cell.objects.empty()) continue;
                for (const auto* obj : cell.objects) {
                    if (query.shouldAcceptObject(*obj)) {
                        results.push_back(obj);
                    }
                }
            }
            return;
        }
        
        // For other query types, process all objects
        for (const auto& pair : objectMap_) {
            if (query.shouldAcceptObject(*pair.second)) {
                results.push_back(pair.second.get());
            }
        }
    }
    
    void queryVolume(const AABB& bounds, const SpatialQuery<T>& query,
                    std::vector<const SpatialObject<T>*>& results) const {
        // Early out if no intersection with total bounds
        if (!aabbAABBIntersection(bounds, totalBounds_).has_value()) return;
        
        // Get overlapping cell indices with bounds padding
        std::vector<std::size_t> cellIndices;
        cellIndices.reserve(27);  // 3x3x3 grid around target
        getCellIndices(bounds, cellIndices);
        
        // Use a small hash set for duplicate checking
        static thread_local std::unordered_set<const SpatialObject<T>*> processed;
        processed.clear();
        processed.reserve(maxObjectOverlap_ * 2);  // Account for overlap
        
        // Process objects in overlapping cells
        for (std::size_t index : cellIndices) {
            auto it = cells_.find(index);
            if (it == cells_.end()) continue;
            
            const auto& cell = it->second;
            // Skip empty cells or if cell is too far
            if (cell.objects.empty()) continue;
            
            // Process objects in this cell
            for (const auto* obj : cell.objects) {
                if (processed.insert(obj).second) {
                    // Extra bounds check since we might have over-estimated cells
                    if (aabbAABBIntersection(bounds, obj->getBounds()).has_value() &&
                        query.shouldAcceptObject(*obj)) {
                        results.push_back(obj);
                    }
                }
            }
        }
    }
    
    void optimize() override {
        // Spatial hashing doesn't require optimization
    }
    
    void rebuild() override {
        // Collect all objects
        std::vector<std::unique_ptr<SpatialObject<T>>> objects;
        objects.reserve(objectCount_);
        for (auto& pair : objectMap_) {
            objects.push_back(std::move(pair.second));
        }
        
        // Clear and rebuild
        clear();
        for (auto& obj : objects) {
            insert(std::move(obj));
        }
    }
    
    std::size_t getObjectCount() const override { return objectCount_; }
    std::size_t getNodeCount() const override { return cells_.size(); }
    std::size_t getMaxDepth() const override { return 1; }  // Flat structure
    float getAverageObjectsPerNode() const override {
        if (cells_.empty()) return 0.0f;
        return static_cast<float>(objectCount_) / cells_.size();
    }
    
    void debugDraw(const std::function<void(const AABB&)>& drawAABB) const override {
        for (const auto& pair : cells_) {
            if (pair.second.objects.empty()) continue;
            Vector3 min = cellIndexToPosition(pair.first);
            AABB cellBounds(
                min + Vector3(config_.cellSize * 0.5f, config_.cellSize * 0.5f, config_.cellSize * 0.5f),
                Vector3(config_.cellSize * 0.5f, config_.cellSize * 0.5f, config_.cellSize * 0.5f)
            );
            drawAABB(cellBounds);
        }
    }

private:
    struct Cell {
        std::vector<const SpatialObject<T>*> objects;
        Cell() { objects.reserve(8); }  // Most cells contain few objects
    };
    
    // Thread-local per-object reusable storage to avoid allocations
    struct LocalStorage {
        std::vector<std::size_t> cellIndices;
        LocalStorage() { cellIndices.reserve(8); }
    };
    inline static thread_local LocalStorage storage_;
    
    std::size_t adjustGridSize() {
        // Cheap heuristic based only on object count to avoid scanning cells_ each insert
        std::size_t oldSize = gridSize_;
        if (objectCount_ < 100) gridSize_ = 16;
        else if (objectCount_ < 1000) gridSize_ = 32;
        else if (objectCount_ < 10000) gridSize_ = 64;
        else gridSize_ = 128;
        
        gridSizeSq_ = gridSize_ * gridSize_;
        return oldSize;
    }
    
    Vector3 cellIndexToPosition(std::size_t index) const {
        // Convert linear index to 3D position
        float x = static_cast<float>(index % gridSize_);
        float y = static_cast<float>((index / gridSize_) % gridSize_);
        float z = static_cast<float>(index / gridSizeSq_);
        return Vector3(x, y, z) * config_.cellSize;
    }
    
    std::size_t positionToCellIndex(const Vector3& position) const {
        // Fast cell index calculation
        std::size_t x = static_cast<std::size_t>(std::floor(position.x * cellSizeInv_)) % gridSize_;
        std::size_t y = static_cast<std::size_t>(std::floor(position.y * cellSizeInv_)) % gridSize_;
        std::size_t z = static_cast<std::size_t>(std::floor(position.z * cellSizeInv_)) % gridSize_;
        return x + y * gridSize_ + z * gridSizeSq_;
    }
    
    void getCellIndices(const AABB& bounds, std::vector<std::size_t>& indices) const {
        // Add padding to catch edge cases (used for queries)
        const float padding = config_.cellSize * 0.1f;  // 10% padding
        Vector3 padMin = bounds.min - Vector3(padding, padding, padding);
        Vector3 padMax = bounds.max + Vector3(padding, padding, padding);
        
        // Calculate cell coordinates with padding
        std::size_t minX = static_cast<std::size_t>(std::floor(padMin.x * cellSizeInv_)) % gridSize_;
        std::size_t minY = static_cast<std::size_t>(std::floor(padMin.y * cellSizeInv_)) % gridSize_;
        std::size_t minZ = static_cast<std::size_t>(std::floor(padMin.z * cellSizeInv_)) % gridSize_;
        std::size_t maxX = static_cast<std::size_t>(std::floor(padMax.x * cellSizeInv_)) % gridSize_;
        std::size_t maxY = static_cast<std::size_t>(std::floor(padMax.y * cellSizeInv_)) % gridSize_;
        std::size_t maxZ = static_cast<std::size_t>(std::floor(padMax.z * cellSizeInv_)) % gridSize_;
        
        // Handle cases where max < min after modulo
        if (maxX < minX) maxX += gridSize_;
        if (maxY < minY) maxY += gridSize_;
        if (maxZ < minZ) maxZ += gridSize_;
        
        // Calculate expected number of cells
        std::size_t numX = maxX - minX + 1;
        std::size_t numY = maxY - minY + 1;
        std::size_t numZ = maxZ - minZ + 1;
        indices.reserve(numX * numY * numZ);
        
        // Collect overlapping cell indices
        for (std::size_t z = minZ; z <= maxZ; ++z) {
            std::size_t zBase = (z % gridSize_) * gridSizeSq_;
            for (std::size_t y = minY; y <= maxY; ++y) {
                std::size_t zyBase = zBase + (y % gridSize_) * gridSize_;
                for (std::size_t x = minX; x <= maxX; ++x) {
                    indices.push_back(zyBase + (x % gridSize_));
                }
            }
        }
    }
    
    void getCellIndicesNoPad(const AABB& bounds, std::vector<std::size_t>& indices) const {
        // Fast path for objects that likely fit in one cell (avoid modulo)
        float minX = std::floor(bounds.min.x * cellSizeInv_);
        float minY = std::floor(bounds.min.y * cellSizeInv_);
        float minZ = std::floor(bounds.min.z * cellSizeInv_);
        float maxX = std::floor(bounds.max.x * cellSizeInv_);
        float maxY = std::floor(bounds.max.y * cellSizeInv_);
        float maxZ = std::floor(bounds.max.z * cellSizeInv_);
        
        if (maxX == minX && maxY == minY && maxZ == minZ) {
            // Object fits in one cell
            std::size_t x = static_cast<std::size_t>(minX) % gridSize_;
            std::size_t y = static_cast<std::size_t>(minY) % gridSize_;
            std::size_t z = static_cast<std::size_t>(minZ) % gridSize_;
            indices.push_back(x + y * gridSize_ + z * gridSizeSq_);
            return;
        }
        
        // Multi-cell case
        std::size_t gminX = static_cast<std::size_t>(minX) % gridSize_;
        std::size_t gminY = static_cast<std::size_t>(minY) % gridSize_;
        std::size_t gminZ = static_cast<std::size_t>(minZ) % gridSize_;
        std::size_t gmaxX = static_cast<std::size_t>(maxX) % gridSize_;
        std::size_t gmaxY = static_cast<std::size_t>(maxY) % gridSize_;
        std::size_t gmaxZ = static_cast<std::size_t>(maxZ) % gridSize_;
        
        if (gmaxX < gminX) gmaxX += gridSize_;
        if (gmaxY < gminY) gmaxY += gridSize_;
        if (gmaxZ < gminZ) gmaxZ += gridSize_;
        
        for (std::size_t z = gminZ; z <= gmaxZ; ++z) {
            std::size_t zBase = (z % gridSize_) * gridSizeSq_;
            for (std::size_t y = gminY; y <= gmaxY; ++y) {
                std::size_t zyBase = zBase + (y % gridSize_) * gridSize_;
                for (std::size_t x = gminX; x <= gmaxX; ++x) {
                    indices.push_back(zyBase + (x % gridSize_));
                }
            }
        }
    }
    
    SpatialConfig config_;
    std::unordered_map<std::size_t, Cell> cells_;  // Sparse cell storage
    std::unordered_map<std::size_t, std::unique_ptr<SpatialObject<T>>> objectMap_;  // ID -> Object
    std::unordered_map<std::size_t, std::vector<std::size_t>> objectCells_;  // ID -> Cell indices
    std::size_t objectCount_;
    
    // Cached values for faster computation
    float cellSizeInv_;  // 1.0f / cellSize
    std::size_t gridSize_;  // Current grid size (adjusted based on object count)
    std::size_t gridSizeSq_;  // gridSize_ * gridSize_
    
    // Statistical tracking for adaptive resizing
    float objectsPerCell_;  // Average objects per non-empty cell
    std::size_t maxObjectOverlap_;  // Maximum objects in any cell
    AABB totalBounds_;  // Total bounds of all objects
};

} // namespace geometry
} // namespace math
} // namespace pynovage
