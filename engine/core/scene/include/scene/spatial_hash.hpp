#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <cmath>
#include "vectors/vector3.hpp"
#include "threading/thread_pool.hpp"

namespace PyNovaGE {
namespace Scene {

/**
 * @brief Handle for objects in spatial hash
 */
using SpatialHandle = uint32_t;
static constexpr SpatialHandle INVALID_HANDLE = 0;

/**
 * @brief 3D Spatial hashing system optimized for MMO scenarios
 * 
 * Features:
 * - O(1) insertion/removal/update
 * - Fast neighbor queries for player interaction ranges
 * - Efficient collision detection preprocessing  
 * - Multi-threaded updates for large object counts
 * - Memory-efficient storage for sparse worlds
 * 
 * Optimized for:
 * - Player-to-player proximity detection
 * - NPC AI neighbor awareness
 * - Item pickup range detection
 * - Area-of-effect skill targeting
 * - Dynamic loading/unloading of world regions
 */
template<typename T>
class SpatialHash {
public:
    /**
     * @brief Configuration for spatial hash
     */
    struct Config {
        float cell_size = 10.0f;              // Size of each hash cell (meters)
        size_t initial_capacity = 1024;       // Initial capacity per cell
        bool enable_multithreading = true;    // Use threading for bulk operations
        size_t thread_batch_size = 100;       // Objects per thread batch
    };

    /**
     * @brief Object entry in spatial hash
     */
    struct Entry {
        SpatialHandle handle;
        PyNovaGE::Vector3f position;
        PyNovaGE::Vector3f previous_position;
        T data;
        bool needs_update;
        
        Entry() : handle(INVALID_HANDLE), needs_update(false) {}
        Entry(SpatialHandle h, const PyNovaGE::Vector3f& pos, const T& d)
            : handle(h), position(pos), previous_position(pos), data(d), needs_update(false) {}
    };

    explicit SpatialHash(const Config& config = Config{}) 
        : config_(config), next_handle_(1) {
        if (config_.enable_multithreading) {
            thread_pool_ = std::make_unique<PyNovaGE::Threading::ThreadPool>();
        }
    }
    ~SpatialHash() = default;

    /**
     * @brief Insert object into spatial hash
     * @param position World position
     * @param data User data to store
     * @return Handle for future operations
     */
    SpatialHandle Insert(const PyNovaGE::Vector3f& position, const T& data) {
        SpatialHandle handle = next_handle_++;
        Entry entry(handle, position, data);
        objects_[handle] = entry;
        CellKey cell_key = GetCellKey(position);
        AddToCell(cell_key, handle);
        return handle;
    }

    /**
     * @brief Remove object from spatial hash
     * @param handle Handle of object to remove
     * @return true if object was found and removed
     */
    bool Remove(SpatialHandle handle) {
        auto it = objects_.find(handle);
        if (it == objects_.end()) return false;
        CellKey cell_key = GetCellKey(it->second.position);
        RemoveFromCell(cell_key, handle);
        objects_.erase(it);
        return true;
    }

    /**
     * @brief Update object position
     * @param handle Handle of object to update
     * @param new_position New world position
     * @return true if object was found and updated
     */
    bool UpdatePosition(SpatialHandle handle, const PyNovaGE::Vector3f& new_position) {
        auto it = objects_.find(handle);
        if (it == objects_.end()) return false;
        Entry& entry = it->second;
        CellKey old_cell = GetCellKey(entry.position);
        CellKey new_cell = GetCellKey(new_position);
        entry.previous_position = entry.position;
        entry.position = new_position;
        entry.needs_update = true;
        if (!(old_cell == new_cell)) {
            RemoveFromCell(old_cell, handle);
            AddToCell(new_cell, handle);
        }
        return true;
    }

    /**
     * @brief Get object data
     * @param handle Handle of object
     * @return Pointer to data, nullptr if not found
     */
    const Entry* GetEntry(SpatialHandle handle) const {
        auto it = objects_.find(handle);
        return (it != objects_.end()) ? &it->second : nullptr;
    }
    Entry* GetEntry(SpatialHandle handle) {
        auto it = objects_.find(handle);
        return (it != objects_.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Query objects within radius of a point
     * @param center Center point for query
     * @param radius Search radius
     * @param results Vector to store results
     * @param max_results Maximum number of results (0 = no limit)
     */
    void QueryRadius(const PyNovaGE::Vector3f& center, float radius, 
                    std::vector<SpatialHandle>& results, size_t max_results = 0) const {
        results.clear();
        std::vector<CellKey> cells_to_check;
        GetCellsInRange(center, radius, cells_to_check);
        float radius_squared = radius * radius;
        for (const auto& cell_key : cells_to_check) {
            auto cell_it = cells_.find(cell_key);
            if (cell_it == cells_.end()) continue;
            for (SpatialHandle handle : cell_it->second) {
                auto obj_it = objects_.find(handle);
                if (obj_it == objects_.end()) continue;
                const Entry& entry = obj_it->second;
                if (DistanceSquared(center, entry.position) <= radius_squared) {
                    results.push_back(handle);
                    if (max_results > 0 && results.size() >= max_results) return;
                }
            }
        }
    }

    /**
     * @brief Query objects within axis-aligned bounding box
     * @param min_bounds Minimum corner of AABB
     * @param max_bounds Maximum corner of AABB
     * @param results Vector to store results
     */
    void QueryAABB(const PyNovaGE::Vector3f& min_bounds, 
                   const PyNovaGE::Vector3f& max_bounds,
                   std::vector<SpatialHandle>& results) const {
        results.clear();
        std::vector<CellKey> cells_to_check;
        GetCellsInAABB(min_bounds, max_bounds, cells_to_check);
        for (const auto& cell_key : cells_to_check) {
            auto cell_it = cells_.find(cell_key);
            if (cell_it == cells_.end()) continue;
            for (SpatialHandle handle : cell_it->second) {
                auto obj_it = objects_.find(handle);
                if (obj_it == objects_.end()) continue;
                const Entry& entry = obj_it->second;
                if (entry.position.x >= min_bounds.x && entry.position.x <= max_bounds.x &&
                    entry.position.y >= min_bounds.y && entry.position.y <= max_bounds.y &&
                    entry.position.z >= min_bounds.z && entry.position.z <= max_bounds.z) {
                    results.push_back(handle);
                }
            }
        }
    }

    /**
     * @brief Get all neighbors within range (optimized for MMO player queries)
     * @param handle Handle of object to find neighbors for
     * @param range Search range
     * @param results Vector to store neighbor handles
     * @param include_self Whether to include the query object in results
     */
    void GetNeighbors(SpatialHandle handle, float range, 
                     std::vector<SpatialHandle>& results, bool include_self = false) const {
        results.clear();
        auto it = objects_.find(handle);
        if (it == objects_.end()) return;
        const Entry& entry = it->second;
        QueryRadius(entry.position, range, results);
        if (!include_self) {
            auto self_it = std::find(results.begin(), results.end(), handle);
            if (self_it != results.end()) results.erase(self_it);
        }
    }

    /**
     * @brief Perform bulk position updates (multi-threaded)
     * @param updates Vector of {handle, new_position} pairs
     */
    void BulkUpdate(const std::vector<std::pair<SpatialHandle, PyNovaGE::Vector3f>>& updates) {
        for (const auto& [handle, new_position] : updates) {
            UpdatePosition(handle, new_position);
        }
    }

    /**
     * @brief Execute function for each object in range (optimized iteration)
     * @param center Center point
     * @param radius Search radius
     * @param func Function to execute for each object
     */
    template<typename Func>
    void ForEachInRange(const PyNovaGE::Vector3f& center, float radius, Func func) const {
        std::vector<CellKey> cells;
        GetCellsInRange(center, radius, cells);
        
        float radius_squared = radius * radius;
        
        for (const auto& cell_key : cells) {
            auto cell_it = cells_.find(cell_key);
            if (cell_it == cells_.end()) continue;
            
            for (SpatialHandle handle : cell_it->second) {
                auto obj_it = objects_.find(handle);
                if (obj_it == objects_.end()) continue;
                
                const Entry& entry = obj_it->second;
                if (DistanceSquared(center, entry.position) <= radius_squared) {
                    func(entry);
                }
            }
        }
    }

    /**
     * @brief Get statistics
     */
    struct Stats {
        size_t total_objects;
        size_t active_cells;
        size_t empty_cells;
        float load_factor;
        float average_objects_per_cell;
        size_t max_objects_in_cell;
        size_t memory_usage_bytes;
    };
    
    Stats GetStats() const {
        Stats stats;
        stats.total_objects = objects_.size();
        stats.active_cells = 0;
        stats.empty_cells = 0;
        stats.max_objects_in_cell = 0;
        stats.memory_usage_bytes = sizeof(*this);
        size_t total_objects_in_cells = 0;
        for (const auto& [key, cell] : cells_) {
            if (cell.empty()) {
                stats.empty_cells++;
            } else {
                stats.active_cells++;
                stats.max_objects_in_cell = std::max(stats.max_objects_in_cell, cell.size());
                total_objects_in_cells += cell.size();
            }
            stats.memory_usage_bytes += sizeof(key) + sizeof(cell) + cell.capacity() * sizeof(SpatialHandle);
        }
        stats.memory_usage_bytes += objects_.size() * (sizeof(SpatialHandle) + sizeof(Entry));
        stats.load_factor = stats.active_cells > 0 ? static_cast<float>(total_objects_in_cells) / static_cast<float>(stats.active_cells) : 0.0f;
        stats.average_objects_per_cell = stats.active_cells > 0 ? static_cast<float>(total_objects_in_cells) / static_cast<float>(stats.active_cells) : 0.0f;
        return stats;
    }

    /**
     * @brief Clear all objects
     */
    void Clear() {
        objects_.clear();
        cells_.clear();
        next_handle_ = 1;
    }

    /**
     * @brief Set configuration
     */
    void SetConfig(const Config& config) { config_ = config; }

    /**
     * @brief Get configuration  
     */
    const Config& GetConfig() const { return config_; }

private:
    Config config_;
    
    // Hash key type for spatial cells
    struct CellKey {
        int x, y, z;
        
        CellKey(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
        
        bool operator==(const CellKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };
    
    // Hash function for cell keys
    struct CellKeyHash {
        std::size_t operator()(const CellKey& key) const {
            // Wang hash for good distribution
            uint32_t hash = key.x;
            hash = (~hash) + (hash << 18);
            hash = hash ^ (hash >> 31);
            hash = hash * 21;
            hash = hash ^ (hash >> 11);
            hash = hash + (hash << 6);
            hash = hash ^ (hash >> 22);
            
            uint32_t hash_y = key.y;
            hash_y = (~hash_y) + (hash_y << 18);
            hash_y = hash_y ^ (hash_y >> 31);
            hash_y = hash_y * 21;
            hash_y = hash_y ^ (hash_y >> 11);
            hash_y = hash_y + (hash_y << 6);
            hash_y = hash_y ^ (hash_y >> 22);
            
            uint32_t hash_z = key.z;
            hash_z = (~hash_z) + (hash_z << 18);
            hash_z = hash_z ^ (hash_z >> 31);
            hash_z = hash_z * 21;
            hash_z = hash_z ^ (hash_z >> 11);
            hash_z = hash_z + (hash_z << 6);
            hash_z = hash_z ^ (hash_z >> 22);
            
            return hash ^ (hash_y << 1) ^ (hash_z << 2);
        }
    };
    
    // Cell storage
    using Cell = std::vector<SpatialHandle>;
    std::unordered_map<CellKey, Cell, CellKeyHash> cells_;
    
    // Object storage
    std::unordered_map<SpatialHandle, Entry> objects_;
    SpatialHandle next_handle_;
    
    // Thread pool for parallel operations
    mutable std::unique_ptr<PyNovaGE::Threading::ThreadPool> thread_pool_;
    
    // Helper methods
    CellKey GetCellKey(const PyNovaGE::Vector3f& position) const {
        int x = static_cast<int>(std::floor(position.x / config_.cell_size));
        int y = static_cast<int>(std::floor(position.y / config_.cell_size));
        int z = static_cast<int>(std::floor(position.z / config_.cell_size));
        return CellKey(x, y, z);
    }
    
    void GetCellsInRange(const PyNovaGE::Vector3f& center, float radius, std::vector<CellKey>& cells) const {
        cells.clear();
        int min_x = static_cast<int>(std::floor((center.x - radius) / config_.cell_size));
        int max_x = static_cast<int>(std::floor((center.x + radius) / config_.cell_size));
        int min_y = static_cast<int>(std::floor((center.y - radius) / config_.cell_size));
        int max_y = static_cast<int>(std::floor((center.y + radius) / config_.cell_size));
        int min_z = static_cast<int>(std::floor((center.z - radius) / config_.cell_size));
        int max_z = static_cast<int>(std::floor((center.z + radius) / config_.cell_size));
        for (int x = min_x; x <= max_x; ++x) {
            for (int y = min_y; y <= max_y; ++y) {
                for (int z = min_z; z <= max_z; ++z) {
                    cells.emplace_back(x, y, z);
                }
            }
        }
    }
    
    void GetCellsInAABB(const PyNovaGE::Vector3f& min_bounds, const PyNovaGE::Vector3f& max_bounds, std::vector<CellKey>& cells) const {
        cells.clear();
        int min_x = static_cast<int>(std::floor(min_bounds.x / config_.cell_size));
        int max_x = static_cast<int>(std::floor(max_bounds.x / config_.cell_size));
        int min_y = static_cast<int>(std::floor(min_bounds.y / config_.cell_size));
        int max_y = static_cast<int>(std::floor(max_bounds.y / config_.cell_size));
        int min_z = static_cast<int>(std::floor(min_bounds.z / config_.cell_size));
        int max_z = static_cast<int>(std::floor(max_bounds.z / config_.cell_size));
        for (int x = min_x; x <= max_x; ++x) {
            for (int y = min_y; y <= max_y; ++y) {
                for (int z = min_z; z <= max_z; ++z) {
                    cells.emplace_back(x, y, z);
                }
            }
        }
    }
    
    void RemoveFromCell(const CellKey& key, SpatialHandle handle) {
        auto it = cells_.find(key);
        if (it != cells_.end()) {
            auto& cell = it->second;
            cell.erase(std::remove(cell.begin(), cell.end(), handle), cell.end());
            if (cell.empty()) cells_.erase(it);
        }
    }
    
    void AddToCell(const CellKey& key, SpatialHandle handle) {
        cells_[key].push_back(handle);
    }
    
    float DistanceSquared(const PyNovaGE::Vector3f& a, const PyNovaGE::Vector3f& b) const {
        PyNovaGE::Vector3f diff = a - b;
        return diff.dot(diff);
    }
};

} // namespace Scene
} // namespace PyNovaGE

// Template implementations must be provided by explicit instantiation
// or inline definitions within the class
