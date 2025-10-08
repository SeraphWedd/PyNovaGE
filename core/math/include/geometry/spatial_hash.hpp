#pragma once

#include "spatial_partitioning.hpp"
#include "../vector3.hpp"
#include "../matrix4.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace pynovage {
namespace math {
namespace geometry {

template<typename T>
class SpatialHash : public SpatialContainer<T> {
public:
    explicit SpatialHash(const SpatialConfig& config = SpatialConfig())
        : config_(config), objectCount_(0) {
        // Set reasonable grid size based on cell size
        cellSizeInv_ = 1.0f / config_.cellSize;
        gridSize_ = 32;  // Default to 32³ grid (32,768 cells)
        gridSizeSq_ = gridSize_ * gridSize_;
        
        // Pre-allocate initial cell capacity
        cells_.reserve(64);
    }
    
    void insert(std::unique_ptr<SpatialObject<T>> object) override {
        // Check if we need to adjust grid size
        std::size_t oldGridSize = gridSize_;
        adjustGridSize();
        
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
                
                // Calculate cell indices
                std::vector<std::size_t> cellIndices;
                cellIndices.reserve(8);
                getCellIndices(objPtr->getBounds(), cellIndices);
                
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
        
        // Calculate cell indices
        const AABB& bounds = objPtr->getBounds();
        std::vector<std::size_t> cellIndices;
        cellIndices.reserve(8);  // Most objects overlap with <= 8 cells
        getCellIndices(bounds, cellIndices);
        
        // Add to cells
        for (std::size_t index : cellIndices) {
            cells_[index].objects.push_back(objPtr);
        }
        
        objectCells_[id] = std::move(cellIndices);
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
            std::size_t index = positionToCellIndex(point);
            auto it = cells_.find(index);
            if (it == cells_.end()) return;
            const auto& cell = it->second;
            for (const auto* obj : cell.objects) {
                if (query.shouldAcceptObject(*obj)) {
                    results.push_back(obj);
                }
            }
            return;
        }
        
        // For other query types (ray, etc), process all objects
        for (const auto& pair : objectMap_) {
            if (query.shouldAcceptObject(*pair.second)) {
                results.push_back(pair.second.get());
            }
        }
    }
    
    void queryVolume(const AABB& bounds, const SpatialQuery<T>& query,
                    std::vector<const SpatialObject<T>*>& results) const {
        // Get overlapping cell indices
        std::vector<std::size_t> cellIndices;
        cellIndices.reserve(8);
        getCellIndices(bounds, cellIndices);
        
        // Use a small hash set for duplicate checking
        static thread_local std::unordered_set<const SpatialObject<T>*> processed;
        processed.clear();
        processed.reserve(32);  // Reserve space for typical case
        
        // Process objects in overlapping cells
        for (std::size_t index : cellIndices) {
            auto it = cells_.find(index);
            if (it == cells_.end()) continue;
            const auto& cell = it->second;
            for (const auto* obj : cell.objects) {
                if (processed.insert(obj).second) {
                    if (query.shouldAcceptObject(*obj)) {
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
    
    std::size_t adjustGridSize() {
        // Adjust grid size based on number of objects
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
        // Calculate cell coordinates directly
        std::size_t minX = static_cast<std::size_t>(std::floor(bounds.min.x * cellSizeInv_)) % gridSize_;
        std::size_t minY = static_cast<std::size_t>(std::floor(bounds.min.y * cellSizeInv_)) % gridSize_;
        std::size_t minZ = static_cast<std::size_t>(std::floor(bounds.min.z * cellSizeInv_)) % gridSize_;
        std::size_t maxX = static_cast<std::size_t>(std::floor(bounds.max.x * cellSizeInv_)) % gridSize_;
        std::size_t maxY = static_cast<std::size_t>(std::floor(bounds.max.y * cellSizeInv_)) % gridSize_;
        std::size_t maxZ = static_cast<std::size_t>(std::floor(bounds.max.z * cellSizeInv_)) % gridSize_;
        
        // Handle cases where max < min after modulo
        if (maxX < minX) maxX += gridSize_;
        if (maxY < minY) maxY += gridSize_;
        if (maxZ < minZ) maxZ += gridSize_;
        
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
    
    SpatialConfig config_;
    std::unordered_map<std::size_t, Cell> cells_;  // Sparse cell storage
    std::unordered_map<std::size_t, std::unique_ptr<SpatialObject<T>>> objectMap_;  // ID -> Object
    std::unordered_map<std::size_t, std::vector<std::size_t>> objectCells_;  // ID -> Cell indices
    std::size_t objectCount_;
    
    // Cached values for faster computation
    float cellSizeInv_;  // 1.0f / cellSize
    std::size_t gridSize_;  // Current grid size (adjusted based on object count)
    std::size_t gridSizeSq_;  // gridSize_ * gridSize_
};

} // namespace geometry
} // namespace math
} // namespace pynovage
