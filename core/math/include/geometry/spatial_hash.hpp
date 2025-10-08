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
        cells_.reserve(config_.tableSize);
    }
    
    void insert(std::unique_ptr<SpatialObject<T>> object) override {
        const AABB& bounds = object->getBounds();
        std::vector<std::size_t> cellIndices = getCellIndices(bounds);
        
        // Store object
        const SpatialObject<T>* objPtr = object.get();
        objectMap_[objPtr] = std::move(object);
        objectCount_++;
        
        // Add to cells
        for (std::size_t index : cellIndices) {
            cells_[index].objects.push_back(objPtr);
        }
        
        objectCells_[objPtr] = std::move(cellIndices);
    }
    
    void remove(const SpatialObject<T>* object) override {
        auto it = objectCells_.find(object);
        if (it == objectCells_.end()) return;
        
        // Remove from cells
        const auto& cellIndices = it->second;
        for (std::size_t index : cellIndices) {
            auto& cell = cells_[index];
            auto objIt = std::find(cell.objects.begin(), cell.objects.end(), object);
            if (objIt != cell.objects.end()) {
                cell.objects.erase(objIt);
            }
        }
        
        // Remove from maps
        objectCells_.erase(it);
        objectMap_.erase(object);
        objectCount_--;
    }
    
    void update(const SpatialObject<T>* object) override {
        auto it = objectMap_.find(object);
        if (it == objectMap_.end()) return;
        
        // Get new cell indices
        const AABB& bounds = object->getBounds();
        std::vector<std::size_t> newCellIndices = getCellIndices(bounds);
        
        // Get old cell indices
        auto oldIt = objectCells_.find(object);
        if (oldIt != objectCells_.end()) {
            const auto& oldIndices = oldIt->second;
            
            // Remove from old cells
            for (std::size_t index : oldIndices) {
                auto& cell = cells_[index];
                auto objIt = std::find(cell.objects.begin(), cell.objects.end(), object);
                if (objIt != cell.objects.end()) {
                    cell.objects.erase(objIt);
                }
            }
        }
        
        // Add to new cells
        for (std::size_t index : newCellIndices) {
            cells_[index].objects.push_back(object);
        }
        
        objectCells_[object] = std::move(newCellIndices);
    }
    
    void clear() override {
        cells_.clear();
        cells_.reserve(config_.tableSize);
        objectMap_.clear();
        objectCells_.clear();
        objectCount_ = 0;
    }
    
    void query(const SpatialQuery<T>& query,
               std::vector<const SpatialObject<T>*>& results) const override {
        // Convert query bounds to cell range
        AABB queryBounds;
        if (auto* volumeQuery = dynamic_cast<const VolumeQuery<T>*>(&query)) {
            queryBounds = volumeQuery->getBounds();
        } else {
            // Use a point query's position to create a small query bounds
            if (auto* pointQuery = dynamic_cast<const PointQuery<T>*>(&query)) {
            const Vector3& point = pointQuery->getPoint();
            Vector3 halfSize(config_.cellSize * 0.5f, config_.cellSize * 0.5f, config_.cellSize * 0.5f);
            queryBounds = AABB(point - halfSize, point + halfSize);
            } else {
                // For other query types, process all objects
                for (const auto& pair : objectMap_) {
                    if (query.shouldAcceptObject(*pair.second)) {
                        results.push_back(pair.first);
                    }
                }
                return;
            }
        }
        
        // Get cells that overlap with query bounds
        std::vector<std::size_t> cellIndices = getCellIndices(queryBounds);
        std::unordered_set<const SpatialObject<T>*> processed;
        
        // Process objects in overlapping cells
        for (std::size_t index : cellIndices) {
            auto it = cells_.find(index);
            if (it == cells_.end()) continue;
            
            const auto& cell = it->second;
            for (const SpatialObject<T>* obj : cell.objects) {
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
    };
    
    Vector3 cellIndexToPosition(std::size_t index) const {
        // Convert linear index to 3D position
        float x = static_cast<float>(index % config_.tableSize);
        float y = static_cast<float>((index / config_.tableSize) % config_.tableSize);
        float z = static_cast<float>(index / (config_.tableSize * config_.tableSize));
        return Vector3(x, y, z) * config_.cellSize;
    }
    
    std::size_t positionToCellIndex(const Vector3& position) const {
        // Convert 3D position to linear index
        std::size_t x = static_cast<std::size_t>(std::floor(position.x / config_.cellSize));
        std::size_t y = static_cast<std::size_t>(std::floor(position.y / config_.cellSize));
        std::size_t z = static_cast<std::size_t>(std::floor(position.z / config_.cellSize));
        
        // Hash to single dimension
        x = x % config_.tableSize;
        y = y % config_.tableSize;
        z = z % config_.tableSize;
        
        return x + y * config_.tableSize + z * config_.tableSize * config_.tableSize;
    }
    
    std::vector<std::size_t> getCellIndices(const AABB& bounds) const {
        std::vector<std::size_t> indices;
        
        // Get min/max cell coordinates
        Vector3 min = bounds.min;
        Vector3 max = bounds.max;
        
        std::size_t minX = static_cast<std::size_t>(std::floor(min.x / config_.cellSize));
        std::size_t minY = static_cast<std::size_t>(std::floor(min.y / config_.cellSize));
        std::size_t minZ = static_cast<std::size_t>(std::floor(min.z / config_.cellSize));
        std::size_t maxX = static_cast<std::size_t>(std::ceil(max.x / config_.cellSize));
        std::size_t maxY = static_cast<std::size_t>(std::ceil(max.y / config_.cellSize));
        std::size_t maxZ = static_cast<std::size_t>(std::ceil(max.z / config_.cellSize));
        
        // Collect all cell indices that the AABB overlaps
        for (std::size_t z = minZ; z <= maxZ; ++z) {
            for (std::size_t y = minY; y <= maxY; ++y) {
                for (std::size_t x = minX; x <= maxX; ++x) {
                    Vector3 pos(
                        static_cast<float>(x) * config_.cellSize,
                        static_cast<float>(y) * config_.cellSize,
                        static_cast<float>(z) * config_.cellSize
                    );
                    indices.push_back(positionToCellIndex(pos));
                }
            }
        }
        
        return indices;
    }
    
    SpatialConfig config_;
    std::unordered_map<std::size_t, Cell> cells_;
    std::unordered_map<const SpatialObject<T>*, std::unique_ptr<SpatialObject<T>>> objectMap_;
    std::unordered_map<const SpatialObject<T>*, std::vector<std::size_t>> objectCells_;
    std::size_t objectCount_;
};

} // namespace geometry
} // namespace math
} // namespace pynovage