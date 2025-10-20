#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "vectors/vector3.hpp"
#include "foundation/threading/thread_pool.hpp"

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

    explicit SpatialHash(const Config& config = Config{});
    ~SpatialHash() = default;

    /**
     * @brief Insert object into spatial hash
     * @param position World position
     * @param data User data to store
     * @return Handle for future operations
     */
    SpatialHandle Insert(const PyNovaGE::Vector3f& position, const T& data);

    /**
     * @brief Remove object from spatial hash
     * @param handle Handle of object to remove
     * @return true if object was found and removed
     */
    bool Remove(SpatialHandle handle);

    /**
     * @brief Update object position
     * @param handle Handle of object to update
     * @param new_position New world position
     * @return true if object was found and updated
     */
    bool UpdatePosition(SpatialHandle handle, const PyNovaGE::Vector3f& new_position);

    /**
     * @brief Get object data
     * @param handle Handle of object
     * @return Pointer to data, nullptr if not found
     */
    const Entry* GetEntry(SpatialHandle handle) const;
    Entry* GetEntry(SpatialHandle handle);

    /**
     * @brief Query objects within radius of a point
     * @param center Center point for query
     * @param radius Search radius
     * @param results Vector to store results
     * @param max_results Maximum number of results (0 = no limit)
     */
    void QueryRadius(const PyNovaGE::Vector3f& center, float radius, 
                    std::vector<SpatialHandle>& results, size_t max_results = 0) const;

    /**
     * @brief Query objects within axis-aligned bounding box
     * @param min_bounds Minimum corner of AABB
     * @param max_bounds Maximum corner of AABB
     * @param results Vector to store results
     */
    void QueryAABB(const PyNovaGE::Vector3f& min_bounds, 
                   const PyNovaGE::Vector3f& max_bounds,
                   std::vector<SpatialHandle>& results) const;

    /**
     * @brief Get all neighbors within range (optimized for MMO player queries)
     * @param handle Handle of object to find neighbors for
     * @param range Search range
     * @param results Vector to store neighbor handles
     * @param include_self Whether to include the query object in results
     */
    void GetNeighbors(SpatialHandle handle, float range, 
                     std::vector<SpatialHandle>& results, bool include_self = false) const;

    /**
     * @brief Perform bulk position updates (multi-threaded)
     * @param updates Vector of {handle, new_position} pairs
     */
    void BulkUpdate(const std::vector<std::pair<SpatialHandle, PyNovaGE::Vector3f>>& updates);

    /**
     * @brief Execute function for each object in range (optimized iteration)
     * @param center Center point
     * @param radius Search radius
     * @param func Function to execute for each object
     */
    template<typename Func>
    void ForEachInRange(const PyNovaGE::Vector3f& center, float radius, Func func) const;

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
    
    Stats GetStats() const;

    /**
     * @brief Clear all objects
     */
    void Clear();

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
    CellKey GetCellKey(const PyNovaGE::Vector3f& position) const;
    void GetCellsInRange(const PyNovaGE::Vector3f& center, float radius, std::vector<CellKey>& cells) const;
    void GetCellsInAABB(const PyNovaGE::Vector3f& min_bounds, const PyNovaGE::Vector3f& max_bounds, std::vector<CellKey>& cells) const;
    void RemoveFromCell(const CellKey& key, SpatialHandle handle);
    void AddToCell(const CellKey& key, SpatialHandle handle);
    float DistanceSquared(const PyNovaGE::Vector3f& a, const PyNovaGE::Vector3f& b) const;
};

} // namespace Scene
} // namespace PyNovaGE

// Template implementations
#include "scene/spatial_hash_impl.hpp"