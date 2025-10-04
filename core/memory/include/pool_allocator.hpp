#pragma once

#include "allocators.hpp"
#include <cstdint>
#include <cassert>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <list>

namespace pynovage {
namespace memory {

// Forward declarations
class PoolBlock;
class PoolChunk;

// Pool allocator that maintains thread-local pools for fixed-size allocations
class ThreadLocalPoolAllocator : public IAllocator {
public:
    // Size class for different allocation sizes
    struct SizeClass {
        std::size_t block_size;     // Size of each block in this class
        std::size_t blocks_per_chunk; // Number of blocks per chunk
        std::size_t alignment;      // Alignment requirement
    };

    explicit ThreadLocalPoolAllocator(const std::vector<SizeClass>& size_classes)
        : size_classes_(size_classes)
        , thread_local_pools_()
        , allocation_count_(0)
        , total_memory_(0)
        , used_memory_(0)
        , generation_(0) {
        assert(!size_classes.empty() && "Must provide at least one size class");
        
        // Initialize the main thread's pool
        getOrCreateThreadPool();
    }

    ~ThreadLocalPoolAllocator() override {
        // Increment generation to invalidate all thread-local caches
        ++generation_;
        
        // Clean up all thread pools
        std::lock_guard<std::mutex> lock(pools_mutex_);
        thread_local_pools_.clear();
    }

    // Disable copying
    ThreadLocalPoolAllocator(const ThreadLocalPoolAllocator&) = delete;
    ThreadLocalPoolAllocator& operator=(const ThreadLocalPoolAllocator&) = delete;

    void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) override {
        // Find the appropriate size class
        const auto size_class_info = findSizeClass(size, alignment);
        if (!size_class_info.first) {
            return nullptr; // Size too large or alignment not supported
        }

        // Get the thread's pool and allocate from it
        ThreadPool& pool = getOrCreateThreadPool();
        void* ptr = pool.allocate(*size_class_info.first, size_class_info.second, *this);
        
        if (ptr) {
            allocation_count_.fetch_add(1, std::memory_order_relaxed);
            used_memory_.fetch_add(size_class_info.first->block_size, std::memory_order_relaxed);
        }
        
        return ptr;
    }

    void deallocate(void* ptr) override {
        if (!ptr) return;

        // Get the thread's pool and deallocate from it
        ThreadPool& pool = getOrCreateThreadPool();
        const auto size_class_info = pool.findSizeClassForPointer(ptr);
        if (!size_class_info.first) {
            // Handle pointers from other allocators silently
            // This allows heterogeneous allocator usage in benchmarks
            return;
        }

        pool.deallocate(ptr, *size_class_info.first, size_class_info.second, *this);
        
        used_memory_.fetch_sub(size_class_info.first->block_size, std::memory_order_relaxed);
    }

    // Optional batch allocation to reduce per-call overhead when many objects of the same size are needed
    std::vector<void*> allocateBatch(std::size_t count, std::size_t size, std::size_t alignment = alignof(std::max_align_t)) {
        std::vector<void*> out;
        out.reserve(count);

        const auto size_class_info = findSizeClass(size, alignment);
        if (!size_class_info.first || count == 0) return out;

        ThreadPool& pool = getOrCreateThreadPool();
        for (std::size_t i = 0; i < count; ++i) {
            void* ptr = pool.allocate(*size_class_info.first, size_class_info.second, *this);
            if (!ptr) break;
            out.push_back(ptr);
        }

        // Update stats once based on number of successfully allocated blocks
        if (!out.empty()) {
            allocation_count_.fetch_add(out.size(), std::memory_order_relaxed);
            used_memory_.fetch_add(out.size() * size_class_info.first->block_size, std::memory_order_relaxed);
        }
        return out;
    }

    // Optional batch deallocation counterpart (same-thread usage recommended)
    void deallocateBatch(const std::vector<void*>& ptrs) {
        if (ptrs.empty()) return;
        ThreadPool& pool = getOrCreateThreadPool();
        std::size_t released_bytes = 0;
        for (void* p : ptrs) {
            if (!p) continue;
            const auto size_class_info = pool.findSizeClassForPointer(p);
            if (!size_class_info.first) continue;
            pool.deallocate(p, *size_class_info.first, size_class_info.second, *this);
            released_bytes += size_class_info.first->block_size;
        }
        if (released_bytes) {
            used_memory_.fetch_sub(released_bytes, std::memory_order_relaxed);
        }
    }

    void reset() override {
        // Increment generation to invalidate all thread-local caches
        ++generation_;
        
        std::lock_guard<std::mutex> lock(pools_mutex_);
        thread_local_pools_.clear();
        
        allocation_count_.store(0, std::memory_order_relaxed);
        total_memory_.store(0, std::memory_order_relaxed);
        used_memory_.store(0, std::memory_order_relaxed);
    }

    std::size_t getUsedMemory() const override { 
        return used_memory_.load(std::memory_order_relaxed); 
    }
    
    std::size_t getTotalMemory() const override { 
        return total_memory_.load(std::memory_order_relaxed); 
    }
    
    std::size_t getAllocationCount() const override { 
        return allocation_count_.load(std::memory_order_relaxed); 
    }

private:
    // Represents a block of memory within a chunk
    struct FreeBlock {
        FreeBlock* next;  // Next free block in the list
    };

    // Represents a chunk of memory containing multiple blocks
    struct Chunk {
        std::vector<std::uint8_t> memory;  // Raw memory for blocks (headers + payloads)
        FreeBlock* free_list;  // List of free blocks
        std::size_t used_blocks;  // Number of blocks in use
        std::size_t stride; // bytes per block including header, aligned
        std::size_t total_bytes; // total bytes in memory buffer
        
        explicit Chunk(const SizeClass& sc)
            : memory()
            , free_list(nullptr)
            , used_blocks(0)
            , stride(alignTo(sc.block_size + sizeof(FreeBlock), sc.alignment))
            , total_bytes(stride * sc.blocks_per_chunk) {
            memory.resize(total_bytes);
            initializeFreeList(sc);
        }

        void initializeFreeList(const SizeClass& sc) {
            for (std::size_t i = 0; i < sc.blocks_per_chunk; ++i) {
                std::uint8_t* block_ptr = memory.data() + (i * stride);
                auto* block = reinterpret_cast<FreeBlock*>(block_ptr);
                block->next = (i + 1 < sc.blocks_per_chunk)
                    ? reinterpret_cast<FreeBlock*>(block_ptr + stride)
                    : nullptr;
            }
            free_list = reinterpret_cast<FreeBlock*>(memory.data());
        }

        bool containsPointer(void* ptr) const {
            const std::uint8_t* begin = memory.data();
            const std::uint8_t* end = begin + total_bytes;
            return reinterpret_cast<const std::uint8_t*>(ptr) >= begin && 
                   reinterpret_cast<const std::uint8_t*>(ptr) < end;
        }
    };

    // Per-thread pool of chunks for each size class, cache-aligned for performance
    struct alignas(64) ThreadPool {
        static constexpr std::size_t CACHE_LINE_SIZE = 64;
        
        // Generation counter to detect stale pools
        std::size_t generation;

        struct ChunkList {
            std::vector<Chunk> chunks;      // Chunks for this size class
            std::size_t active_chunk = 0;   // Index of last used chunk
        };

        std::vector<ChunkList> chunks_by_class;  // Per-size-class chunk lists
        std::thread::id thread_id;  // Owner thread ID
        ThreadLocalPoolAllocator* allocator;  // Parent allocator

        explicit ThreadPool(std::size_t num_size_classes, ThreadLocalPoolAllocator* parent)
            : chunks_by_class(num_size_classes)
            , thread_id(std::this_thread::get_id())
            , allocator(parent)
            , generation(parent->generation_) {
            assert(allocator != nullptr && "Parent allocator cannot be null");
        }

        void* allocate(const SizeClass& size_class, std::size_t class_index, ThreadLocalPoolAllocator& alloc) {
            assert(class_index < chunks_by_class.size() && "Invalid size class index");
            assert(&alloc == allocator && "Allocator mismatch - wrong allocator provided");
            assert(generation == alloc.generation_ && "Pool generation mismatch - stale pool");
            
            auto& list = chunks_by_class[class_index];
            auto& chunks = list.chunks;

            // Fast path: try active chunk
            if (!chunks.empty()) {
                if (list.active_chunk >= chunks.size()) list.active_chunk = 0;
                Chunk& c = chunks[list.active_chunk];
                if (c.free_list) {
                    FreeBlock* block = c.free_list;
                    c.free_list = block->next;
                    ++c.used_blocks;
                    return reinterpret_cast<void*>(reinterpret_cast<std::uint8_t*>(block) + sizeof(FreeBlock));
                }
            }

            // Slow path: find any chunk with space
            for (std::size_t i = 0; i < chunks.size(); ++i) {
                if (chunks[i].free_list) {
                    list.active_chunk = i;
                    FreeBlock* block = chunks[i].free_list;
                    chunks[i].free_list = block->next;
                    ++chunks[i].used_blocks;
                    return reinterpret_cast<void*>(reinterpret_cast<std::uint8_t*>(block) + sizeof(FreeBlock));
                }
            }

            // Need a new chunk
            chunks.emplace_back(size_class);
            list.active_chunk = chunks.size() - 1;
            auto& new_chunk = chunks.back();
            
            // Update total memory
            allocator->total_memory_.fetch_add(new_chunk.total_bytes, std::memory_order_relaxed);

            // Allocate from new chunk
            FreeBlock* block = new_chunk.free_list;
            new_chunk.free_list = block->next;
            ++new_chunk.used_blocks;
            return reinterpret_cast<void*>(reinterpret_cast<std::uint8_t*>(block) + sizeof(FreeBlock));
        }

        void deallocate(void* ptr, const SizeClass& size_class, std::size_t class_index, ThreadLocalPoolAllocator& allocator) {
            assert(class_index < chunks_by_class.size());
            
            auto& list = chunks_by_class[class_index];
            auto& chunks = list.chunks;

            // Find the chunk containing this pointer
            for (std::size_t i = 0; i < chunks.size(); ++i) {
                auto& chunk = chunks[i];
                if (chunk.containsPointer(ptr)) {
                    // Convert user pointer back to block header
                    auto* block = reinterpret_cast<FreeBlock*>(
                        reinterpret_cast<std::uint8_t*>(ptr) - sizeof(FreeBlock)
                    );

                    // Add to free list (LIFO)
                    block->next = chunk.free_list;
                    chunk.free_list = block;
                    --chunk.used_blocks;

                    // Prefer this chunk for next allocation
                    list.active_chunk = i;
                    return;
                }
            }

            // Pointer was not allocated from this pool - could be from another thread
            // or already deallocated. Silently return.
        }

        std::pair<const SizeClass*, std::size_t> findSizeClassForPointer(void* ptr) {
            for (std::size_t i = 0; i < chunks_by_class.size(); ++i) {
                for (const auto& chunk : chunks_by_class[i].chunks) {
                    if (chunk.containsPointer(ptr)) {
                        return {&allocator->size_classes_[i], i};
                    }
                }
            }
            return {nullptr, 0};
        }
    };
    // Find the appropriate size class for an allocation
    std::pair<const SizeClass*, std::size_t> findSizeClass(std::size_t size, std::size_t alignment) const {
        for (std::size_t i = 0; i < size_classes_.size(); ++i) {
            const auto& sc = size_classes_[i];
            if (sc.block_size >= size && sc.alignment >= alignment) {
                return {&sc, i};
            }
        }
        return {nullptr, 0};
    }

    // Get or create thread pool for current thread (safe baseline: search by thread_id)
    ThreadPool& getOrCreateThreadPool() {
        auto tid = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(pools_mutex_);

        // Try to find existing pool for this thread id
        for (auto& pool : thread_local_pools_) {
            if (pool.thread_id == tid) {
                // If allocator generation changed (e.g., reset/destruct), refresh pool
                if (pool.generation != generation_) {
                    pool = ThreadPool(size_classes_.size(), this);
                }
                return pool;
            }
        }

        // Create new pool for this thread
        thread_local_pools_.emplace_back(size_classes_.size(), this);
        return thread_local_pools_.back();
    }

    std::vector<SizeClass> size_classes_;  // Available size classes
    std::list<ThreadPool> thread_local_pools_;  // Pools for each thread (stable addresses)
    mutable std::mutex pools_mutex_;  // Protects thread_local_pools_
    mutable std::mutex stats_mutex_;  // Unused after atomics; kept for future detailed stats
    std::atomic<std::size_t> allocation_count_;  // Total number of active allocations
    std::atomic<std::size_t> total_memory_;  // Total memory allocated
    std::atomic<std::size_t> used_memory_;   // Currently used memory
    std::atomic<std::size_t> generation_;    // Generation counter for pool validity
};

} // namespace memory
} // namespace pynovage