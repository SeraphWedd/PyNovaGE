#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "rigid_body.hpp"
#include "math/vector.hpp"
#include "simd/geometry_ops.hpp"

namespace PyNovaGE {
namespace Physics {

//------------------------------------------------------------------------------
// Spatial Grid for Broad Phase Collision Detection
// Divides 2D space into uniform grid cells for O(n) collision detection
//------------------------------------------------------------------------------

class SpatialGrid {
public:
    struct GridCell {
        std::vector<std::shared_ptr<RigidBody>> bodies;
        
        void clear() {
            bodies.clear();
        }
        
        void reserve(size_t capacity) {
            bodies.reserve(capacity);
        }
    };
    
    struct CollisionPair {
        std::shared_ptr<RigidBody> bodyA;
        std::shared_ptr<RigidBody> bodyB;
        
        CollisionPair(std::shared_ptr<RigidBody> a, std::shared_ptr<RigidBody> b) 
            : bodyA(a), bodyB(b) {}
        
        bool operator==(const CollisionPair& other) const {
            return (bodyA == other.bodyA && bodyB == other.bodyB) ||
                   (bodyA == other.bodyB && bodyB == other.bodyA);
        }
    };

private:
    float cell_size_;
    Vector2<float> world_min_;
    Vector2<float> world_max_;
    int grid_width_;
    int grid_height_;
    
    std::vector<GridCell> grid_;
    std::vector<CollisionPair> potential_pairs_;
    
    // Hash function for collision pair deduplication
    struct PairHash {
        std::size_t operator()(const CollisionPair& pair) const {
            auto h1 = std::hash<void*>{}(pair.bodyA.get());
            auto h2 = std::hash<void*>{}(pair.bodyB.get());
            // Ensure consistent hash regardless of order
            return (h1 < h2) ? h1 ^ (h2 << 1) : h2 ^ (h1 << 1);
        }
    };
    
    std::unordered_set<CollisionPair, PairHash> unique_pairs_;

public:
    SpatialGrid(float cell_size, const Vector2<float>& world_min, const Vector2<float>& world_max)
        : cell_size_(cell_size), world_min_(world_min), world_max_(world_max) {
        
        Vector2<float> world_size = world_max - world_min;
        grid_width_ = static_cast<int>(std::ceil(world_size.x / cell_size_)) + 1;
        grid_height_ = static_cast<int>(std::ceil(world_size.y / cell_size_)) + 1;
        
        grid_.resize(grid_width_ * grid_height_);
        
        // Pre-allocate space for common use cases
        potential_pairs_.reserve(1024);
        unique_pairs_.reserve(1024);
        
        // Reserve space in each cell for better performance
        for (auto& cell : grid_) {
            cell.reserve(8); // Expect average of 8 objects per cell
        }
    }
    
    // Get grid cell index from world position
    int getCellIndex(const Vector2<float>& position) const {
        Vector2<float> local_pos = position - world_min_;
        int x = static_cast<int>(local_pos.x / cell_size_);
        int y = static_cast<int>(local_pos.y / cell_size_);
        
        // Clamp to valid range
        x = std::max(0, std::min(x, grid_width_ - 1));
        y = std::max(0, std::min(y, grid_height_ - 1));
        
        return y * grid_width_ + x;
    }
    
    // Get all grid cell indices that an AABB overlaps
    std::vector<int> getCellIndices(const SIMD::AABB<float>& aabb) const {
        std::vector<int> indices;
        
        Vector2<float> min_local = Vector2<float>(aabb.min[0], aabb.min[1]) - world_min_;
        Vector2<float> max_local = Vector2<float>(aabb.max[0], aabb.max[1]) - world_min_;
        
        int min_x = std::max(0, static_cast<int>(min_local.x / cell_size_));
        int min_y = std::max(0, static_cast<int>(min_local.y / cell_size_));
        int max_x = std::min(grid_width_ - 1, static_cast<int>(max_local.x / cell_size_));
        int max_y = std::min(grid_height_ - 1, static_cast<int>(max_local.y / cell_size_));
        
        indices.reserve((max_x - min_x + 1) * (max_y - min_y + 1));
        
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                indices.push_back(y * grid_width_ + x);
            }
        }
        
        return indices;
    }
    
    // Clear all grid cells
    void clear() {
        for (auto& cell : grid_) {
            cell.clear();
        }
        potential_pairs_.clear();
        unique_pairs_.clear();
    }
    
    // Insert rigid body into appropriate grid cells
    void insert(std::shared_ptr<RigidBody> body) {
        auto aabb = body->getWorldBounds();
        auto cell_indices = getCellIndices(aabb);
        
        for (int index : cell_indices) {
            grid_[index].bodies.push_back(body);
        }
    }
    
    // Generate potential collision pairs using spatial grid
    const std::vector<CollisionPair>& generatePotentialPairs() {
        potential_pairs_.clear();
        unique_pairs_.clear();
        
        // Process each grid cell
        for (const auto& cell : grid_) {
            if (cell.bodies.size() < 2) continue;
            
            // Check all pairs within the cell
            for (size_t i = 0; i < cell.bodies.size(); ++i) {
                for (size_t j = i + 1; j < cell.bodies.size(); ++j) {
                    CollisionPair pair(cell.bodies[i], cell.bodies[j]);
                    
                    // Only add if we haven't seen this pair before
                    if (unique_pairs_.find(pair) == unique_pairs_.end()) {
                        unique_pairs_.insert(pair);
                        potential_pairs_.push_back(pair);
                    }
                }
            }
        }
        
        return potential_pairs_;
    }
    
    // Update grid with new set of rigid bodies
    void update(const std::vector<std::shared_ptr<RigidBody>>& bodies) {
        clear();
        
        for (const auto& body : bodies) {
            insert(body);
        }
    }
    
    // Get statistics about grid usage
    struct GridStats {
        int total_cells;
        int occupied_cells;
        int max_objects_per_cell;
        double average_objects_per_occupied_cell;
        size_t total_objects;
        size_t potential_pairs;
    };
    
    GridStats getStats() const {
        GridStats stats = {};
        stats.total_cells = static_cast<int>(grid_.size());
        stats.potential_pairs = potential_pairs_.size();
        
        int max_objects = 0;
        size_t total_objects = 0;
        int occupied = 0;
        
        for (const auto& cell : grid_) {
            if (!cell.bodies.empty()) {
                occupied++;
                max_objects = std::max(max_objects, static_cast<int>(cell.bodies.size()));
                total_objects += cell.bodies.size();
            }
        }
        
        stats.occupied_cells = occupied;
        stats.max_objects_per_cell = max_objects;
        stats.total_objects = total_objects;
        stats.average_objects_per_occupied_cell = occupied > 0 ? 
            static_cast<double>(total_objects) / occupied : 0.0;
        
        return stats;
    }
    
    // Resize the grid (useful for dynamic worlds)
    void resize(float new_cell_size, const Vector2<float>& new_world_min, const Vector2<float>& new_world_max) {
        cell_size_ = new_cell_size;
        world_min_ = new_world_min;
        world_max_ = new_world_max;
        
        Vector2<float> world_size = world_max_ - world_min_;
        grid_width_ = static_cast<int>(std::ceil(world_size.x / cell_size_)) + 1;
        grid_height_ = static_cast<int>(std::ceil(world_size.y / cell_size_)) + 1;
        
        grid_.clear();
        grid_.resize(grid_width_ * grid_height_);
        
        for (auto& cell : grid_) {
            cell.reserve(8);
        }
    }
    
    // Getters
    float getCellSize() const { return cell_size_; }
    Vector2<float> getWorldMin() const { return world_min_; }
    Vector2<float> getWorldMax() const { return world_max_; }
    int getGridWidth() const { return grid_width_; }
    int getGridHeight() const { return grid_height_; }
};

} // namespace Physics
} // namespace PyNovaGE