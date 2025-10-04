#pragma once

#include "allocators.hpp"
#include "memory_utils.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <atomic>
#include <mutex>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <cstring>
#include <vector>
#include <tuple>
#include "size_classes.hpp"
#include "size_class_free_list.hpp"

namespace pynovage {
namespace memory {

// Forward declarations
class DefragmentingAllocator;

// Extended allocation header with additional safety fields
struct alignas(64) DefragHeader {
    static constexpr std::uint32_t MAGIC_ACTIVE = 0xDEFAC4ED;  // Magic number for active blocks
    static constexpr std::uint32_t MAGIC_FREE = 0xDEFAD0ED;    // Magic number for free blocks
    
    std::uint32_t magic;           // Magic number for validation
    std::size_t size;             // Size of the allocation (excluding header)
    std::size_t alignment;        // Original alignment request
    DefragHeader* prev;           // Previous block in memory order
    DefragHeader* next;           // Next block in memory order
    bool is_free;                 // Whether this block is free
    std::size_t checksum;         // Simple checksum for corruption detection
    
    // Calculate checksum for validation
    std::size_t calculateChecksum() const {
        return static_cast<std::size_t>(magic) ^
               size ^
               alignment ^
               reinterpret_cast<std::size_t>(prev) ^
               reinterpret_cast<std::size_t>(next) ^
               static_cast<std::size_t>(is_free);
    }
    
    // Validate header integrity
    bool isValid() const {
        bool magic_ok = (magic == MAGIC_ACTIVE || magic == MAGIC_FREE);
        bool checksum_ok = (checksum == calculateChecksum());
        return magic_ok && checksum_ok;
    }
    
    // Helper for debugging
    std::string debugString() const {
        std::stringstream ss;
        ss << "Header at " << this << ": "
           << "magic=0x" << std::hex << magic
           << ", size=" << std::dec << size
           << ", alignment=" << alignment
           << ", prev=" << prev
           << ", next=" << next
           << ", is_free=" << is_free
           << ", checksum=0x" << std::hex << checksum
           << ", calculated_checksum=0x" << calculateChecksum()
           << ", valid=" << isValid();
        return ss.str();
    }
    
    // Update checksum after modification
    void updateChecksum() {
        checksum = calculateChecksum();
    }
    
    // Get payload pointer
    void* getPayload() {
        return reinterpret_cast<std::byte*>(this) + sizeof(DefragHeader);
    }
    
    // Get header from payload
    static DefragHeader* getHeader(void* payload) {
        if (!payload) return nullptr;
        return reinterpret_cast<DefragHeader*>(
            reinterpret_cast<std::byte*>(payload) - sizeof(DefragHeader)
        );
    }
    
    // Initialize a new header
    // Additional initialization options
    void initializeSize(std::size_t block_size) {
        size = block_size;
        updateChecksum();
    }

    void initializeAlignment(std::size_t block_alignment) {
        alignment = block_alignment;
        updateChecksum();
    }

    void initializeFree(bool is_free_block) {
        is_free = is_free_block;
        magic = is_free_block ? MAGIC_FREE : MAGIC_ACTIVE;
        updateChecksum();
    }

    void initializeLinks(DefragHeader* prev_block, DefragHeader* next_block) {
        prev = prev_block;
        next = next_block;
        updateChecksum();
    }

    void initialize(std::size_t block_size, std::size_t block_alignment, bool is_free_block) {
        // Zero all fields first
        std::memset(this, 0, sizeof(*this));
        
        // Initialize fields in a specific order
        initializeSize(block_size);
        initializeAlignment(block_alignment);
        initializeFree(is_free_block);
        initializeLinks(nullptr, nullptr);
    }
};

class DefragmentingAllocator final : public IAllocator {
public:
    struct Stats {
        SizeClassManager::Stats size_class_stats;
        size_t total_allocations{0};
        size_t total_deallocations{0};
        size_t total_fragmentation_cycles{0};
        size_t total_blocks_merged{0};
        
        void clear() {
            size_class_stats.clear();
            total_allocations = 0;
            total_deallocations = 0;
            total_fragmentation_cycles = 0;
            total_blocks_merged = 0;
        }
    };
    explicit DefragmentingAllocator(std::size_t total_size) {
        total_size_requested_ = total_size;
        pool_size_ = 0;
        used_memory_ = 0;
        allocation_count_ = 0;
        // Ensure minimum size and alignment
        if (total_size < sizeof(DefragHeader)) {
            throw std::invalid_argument("Total size too small for allocator");
        }
        
        // Allocate memory with extra padding for alignment
        memory_ = new std::byte[total_size_requested_ + alignof(std::max_align_t)];
        
        // Set up initial free block
        std::size_t space = total_size_requested_ + alignof(std::max_align_t);
        void* ptr = memory_;
        void* aligned_start = std::align(
            alignof(DefragHeader),
            sizeof(DefragHeader),
            ptr,
            space
        );
        
        if (!aligned_start) {
            delete[] memory_;
            throw std::runtime_error("Failed to align initial memory");
        }
        
        // Available pool size is remaining space after alignment
        pool_size_ = space;
        memory_start_ = static_cast<std::byte*>(aligned_start);
        memory_end_ = memory_start_ + pool_size_;

        first_block_ = static_cast<DefragHeader*>(aligned_start);
        first_block_->initialize(
            pool_size_ - sizeof(DefragHeader),
            alignof(std::max_align_t),
            true  // Initial block is free
        );
    }
    
    ~DefragmentingAllocator() override {
        delete[] memory_;
    }
    
    // Prevent copying
    DefragmentingAllocator(const DefragmentingAllocator&) = delete;
    DefragmentingAllocator& operator=(const DefragmentingAllocator&) = delete;

protected:
    void* allocateImpl(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) override {
        if (size == 0) return nullptr;
        
        // Try size class allocation first if alignment allows
        if (alignment <= alignof(DefragHeader) && SizeClassManager::shouldUseSizeClass(size)) {
            size_t size_class = SizeClassManager::getSizeClass(size);
            void* ptr = size_class_lists_.tryAllocate(size_class);
            if (ptr) {
                // Validate alignment
                if (reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0) {
                    updateStats([&](Stats& s) {
                        s.total_allocations++;
                        s.size_class_stats.allocations[size_class]++;
                    });
                    {
                        std::lock_guard<std::mutex> map_lock(size_class_map_mutex_);
                        size_class_blocks_[ptr] = size_class;
                    }
                    return ptr;
                }
                // If alignment doesn't match, return to free list
                size_class_lists_.addToFreeList(ptr, size_class);
            } else {
                // Miss: try to replenish this size class from the general pool
                const size_t kMinBlocks = 8;
                const size_t cls_size = SizeClassManager::getSizeForClass(size_class);
                const size_t align = alignof(DefragHeader);
                const size_t stride = (cls_size + (align - 1)) & ~(align - 1);
                const size_t total_bytes = stride * kMinBlocks;

                void* first_block = nullptr;
                {
                    std::lock_guard<std::mutex> lk(mutex_);
                    first_block = internalAllocateRaw(total_bytes, align);
                }
                if (first_block) {
                    // Split into blocks
                    std::byte* base = static_cast<std::byte*>(first_block);
                    // Return the first block to the caller
                    void* ret = base;
                    // Zero returned block for security
                    std::memset(ret, 0, cls_size);
                    // Seed the freelist with remaining blocks
                    for (size_t i = 1; i < kMinBlocks; ++i) {
                        void* blk = base + i * stride;
                        size_class_lists_.addToFreeList(blk, size_class);
                        {
                            std::lock_guard<std::mutex> map_lock(size_class_map_mutex_);
                            size_class_blocks_.emplace(blk, size_class);
                        }
                    }
                    updateStats([&](Stats& s) {
                        s.total_allocations++;
                        s.size_class_stats.allocations[size_class]++;
                    });
                    {
                        std::lock_guard<std::mutex> map_lock(size_class_map_mutex_);
                        size_class_blocks_[ret] = size_class;
                    }
                    return ret;
                }
            }
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Adjust size for alignment requirements
        std::size_t aligned_size = alignTo(size, alignment);
        std::size_t total_required = aligned_size + sizeof(DefragHeader);
        
        // Validate size
        if (total_required > pool_size_) {
            throw std::bad_alloc();
        }
        
        // Update allocation stats
        updateStats([&](Stats& s) {
            s.total_allocations++;
        });
        
        // Find suitable block
        DefragHeader* current = first_block_;
        while (current) {
            if (!current->isValid()) {
                throw std::runtime_error("Memory corruption detected");
            }
            
            if (current->is_free && current->size >= aligned_size) {
                // Found a suitable block
                if (current->size >= aligned_size + sizeof(DefragHeader) + alignof(std::max_align_t)) {
                    // Split the block if there's enough space for another allocation
                    splitBlock(current, aligned_size);
                }
                
                // Mark block as used
                current->is_free = false;
                current->magic = DefragHeader::MAGIC_ACTIVE;
                current->size = aligned_size;
                current->alignment = alignment;
                current->updateChecksum();
                
                used_memory_ += aligned_size;
                allocation_count_++;
                
                return current->getPayload();
            }
            current = current->next;
        }
        
        // No suitable block found, try to defragment
        defragment();
        
        // Try allocation again after defragmentation
        current = first_block_;
        while (current) {
            if (current->is_free && current->size >= aligned_size) {
                if (current->size >= aligned_size + sizeof(DefragHeader) + alignof(std::max_align_t)) {
                    splitBlock(current, aligned_size);
                }
                
                // Update block state atomically
                current->initializeSize(aligned_size);
                current->initializeAlignment(alignment);
                current->initializeFree(false);
                
                used_memory_ += aligned_size;
                allocation_count_++;
                
                return current->getPayload();
            }
            current = current->next;
        }
        
        throw std::bad_alloc();
    }

    void deallocateImpl(void* ptr) override {
        if (!ptr) return;
        
        // Check if this is a size class allocation
        {
            size_t cls = 0;
            bool is_size_class_ptr = false;
            {
                std::lock_guard<std::mutex> map_lock(size_class_map_mutex_);
                auto it = size_class_blocks_.find(ptr);
                if (it != size_class_blocks_.end()) {
                    cls = it->second;
                    size_class_blocks_.erase(it);
                    is_size_class_ptr = true;
                }
            }
            if (is_size_class_ptr) {
                size_class_lists_.addToFreeList(ptr, cls);
                updateStats([&](Stats& s) {
                    s.total_deallocations++;
                    s.size_class_stats.deallocations[cls]++;
                });
                return;
            }
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        DefragHeader* header = getHeaderSafe(ptr);
        
        // Validate header
        if (!header) {
            throw std::invalid_argument("Invalid pointer");
        }
        if (!header->isValid() || header->magic != DefragHeader::MAGIC_ACTIVE) {
            throw std::runtime_error("Invalid pointer or double free detected");
        }
        if (header->is_free) {
            throw std::runtime_error("Invalid pointer or double free detected");
        }
        
        // Update block state
        used_memory_ -= header->size;
        allocation_count_--;
        
        header->initializeFree(true);
        
        updateStats([&](Stats& s) {
            s.total_deallocations++;
        });
        
        // Try to merge with adjacent blocks
        mergeFreeBlocks(header);
    }

public:
    void reset() override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Reset to initial state
        first_block_->initialize(
            pool_size_ - sizeof(DefragHeader),
            alignof(std::max_align_t),
            true
        );
        
        // Clear the size class free lists first
        size_class_lists_.reset();
        
        // Clear tracking data
        {
            std::lock_guard<std::mutex> map_lock(size_class_map_mutex_);
            size_class_blocks_.clear();
        }
        pending_size_class_blocks_.clear();
        
        used_memory_ = 0;
        allocation_count_ = 0;
        
        // Clear stats
        updateStats([](Stats& s) {
            s.clear();
        });
    }
    
    // Get current statistics
    Stats getStats() const {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        return stats_;
    }

    std::size_t getUsedMemory() const override {
        return used_memory_;
    }

    std::size_t getTotalMemory() const override {
        return pool_size_;
    }

    std::size_t getAllocationCount() const override {
        return allocation_count_;
    }


private:
    // Allocate a raw payload region of given size and alignment from the general pool.
    // Returns payload pointer or nullptr if no suitable block found. Caller must hold mutex_.
    void* internalAllocateRaw(std::size_t size, std::size_t alignment) {
        std::size_t aligned_size = alignTo(size, alignment);
        std::size_t total_required = aligned_size + sizeof(DefragHeader);
        if (total_required > pool_size_) return nullptr;

        DefragHeader* current = first_block_;
        while (current) {
            if (!current->isValid()) {
                throw std::runtime_error("Memory corruption detected");
            }
            if (current->is_free && current->size >= aligned_size) {
                if (current->size >= aligned_size + sizeof(DefragHeader) + alignof(std::max_align_t)) {
                    splitBlock(current, aligned_size);
                }
                current->initializeSize(aligned_size);
                current->initializeAlignment(alignment);
                current->initializeFree(false);
                used_memory_ += aligned_size;
                allocation_count_++;
                return current->getPayload();
            }
            current = current->next;
        }
        return nullptr;
    }

    // Validate that a payload pointer belongs to this allocator and return its header
    DefragHeader* getHeaderSafe(void* payload) const {
        if (!payload) return nullptr;
        std::byte* p = reinterpret_cast<std::byte*>(payload);
        if (p <= memory_start_ || p > memory_end_) return nullptr;
        std::byte* hdrp = p - sizeof(DefragHeader);
        if (hdrp < memory_start_) return nullptr;
        return reinterpret_cast<DefragHeader*>(hdrp);
    }

    void splitBlock(DefragHeader* block, std::size_t size) {
        std::size_t remaining_size = block->size - size - sizeof(DefragHeader);
        if (remaining_size < sizeof(DefragHeader) + alignof(std::max_align_t)) {
            return;  // Not worth splitting
        }
        
        // Calculate new block position
        // Compute a properly aligned location for the new header
        std::uintptr_t raw_addr = reinterpret_cast<std::uintptr_t>(block) + sizeof(DefragHeader) + size;
        std::size_t align = alignof(DefragHeader);
        std::uintptr_t aligned_addr = (raw_addr + (align - 1)) & ~(align - 1);
        std::size_t adjust = static_cast<std::size_t>(aligned_addr - raw_addr);
        if (adjust > remaining_size) {
            return; // Not enough space after alignment
        }
        remaining_size -= adjust;

        DefragHeader* new_block = reinterpret_cast<DefragHeader*>(aligned_addr);
        
        // Initialize new block
        new_block->initialize(remaining_size, alignof(std::max_align_t), true);
        
        // Update links
        DefragHeader* old_next = block->next;
        new_block->next = old_next;
        new_block->prev = block;
        if (old_next) {
            old_next->prev = new_block;
            old_next->updateChecksum();
        }
        // update new_block after setting links
        new_block->updateChecksum();
        block->next = new_block;
        
        // Update size of original block
        block->size = size;
        block->updateChecksum();
    }

    void mergeFreeBlocks(DefragHeader* block) {
        // Merge with next block if possible
        while (block->next && block->next->is_free) {
            DefragHeader* next = block->next;
            
            // Validate next block
            if (!next->isValid()) {
                throw std::runtime_error("Memory corruption detected in next block");
            }
            
            block->size += next->size + sizeof(DefragHeader);
            block->next = next->next;
            if (next->next) {
                next->next->prev = block;
                next->next->updateChecksum();
            }
            block->updateChecksum();
        }
        
        // Merge with previous block if possible
        while (block->prev && block->prev->is_free) {
            DefragHeader* prev = block->prev;
            
            // Validate previous block
            if (!prev->isValid()) {
                throw std::runtime_error("Memory corruption detected in previous block");
            }
            
            prev->size += block->size + sizeof(DefragHeader);
            prev->next = block->next;
            if (block->next) {
                block->next->prev = prev;
                block->next->updateChecksum();
            }
            prev->updateChecksum();
            block = prev;
        }
    }

    void defragment() {
        pending_size_class_blocks_.clear();
        DefragHeader* current = first_block_;
        bool changes_made;
        size_t blocks_merged = 0;
        size_t size_class_blocks = 0;
        
        do {
            changes_made = false;
            current = first_block_;
            
            while (current && current->next) {
                if (!current->isValid() || !current->next->isValid()) {
                    throw std::runtime_error("Memory corruption detected during defragmentation");
                }
                
                if (current->is_free && current->next->is_free) {
                    // Try to add to size class free list if appropriate
                    size_t total_size = current->size + current->next->size + sizeof(DefragHeader);
                    if (SizeClassManager::shouldUseSizeClass(total_size)) {
                        size_t size_class = SizeClassManager::getSizeClass(total_size);
                        size_t class_size = SizeClassManager::getSizeForClass(size_class);
                        // Check if this block would make a good size class block
                        if (total_size >= class_size && 
                            total_size <= (class_size + sizeof(DefragHeader)) && // Not too big
                            reinterpret_cast<std::uintptr_t>(current->getPayload()) % alignof(DefragHeader) == 0 &&
                            reinterpret_cast<std::uintptr_t>(current->next->getPayload()) % alignof(DefragHeader) == 0) {
                            // First, merge these blocks
                            DefragHeader* combined = current;
                            combined->size = total_size;
                            DefragHeader* next_block = current->next->next;
                            
                            // Update block links
                            combined->next = next_block;
                            if (next_block) {
                                next_block->prev = combined;
                                next_block->updateChecksum();
                            }
                            combined->is_free = true;
                            combined->magic = DefragHeader::MAGIC_FREE;
                            combined->updateChecksum();
                            
                            // Save block info for size class
                            void* block_start = combined->getPayload();
                            pending_size_class_blocks_.push_back({block_start, size_class, size_t(total_size)});
                            
                            // Move to next block
                            current = next_block;
                            blocks_merged += 2;
                            size_class_blocks++;
                            changes_made = true;
                            continue;
                        }
                    }
                    
                    // Regular merge for blocks that don't fit size classes
                    mergeFreeBlocks(current);
                    blocks_merged++;
                    changes_made = true;
                }
                current = current->next;
            }
        } while (changes_made);
        
        if (blocks_merged > 0) {
            updateStats([blocks_merged](Stats& s) {
                s.total_fragmentation_cycles++;
                s.total_blocks_merged += blocks_merged;
            });
        }
        
        // Now add all pending blocks to size classes
        if (!pending_size_class_blocks_.empty()) {
            size_t successful_blocks = 0;
            for (const auto& [block, size_class, block_size] : pending_size_class_blocks_) {
                DefragHeader* header = getHeaderSafe(block);
                if (!header || !header->isValid() || header->size != block_size) {
                    continue; // Skip if block header is no longer valid
                }
                
                // Remove from linked list
                if (header->prev) {
                    header->prev->next = header->next;
                    header->prev->updateChecksum();
                } else {
                    first_block_ = header->next;
                }
                if (header->next) {
                    header->next->prev = header->prev;
                    header->next->updateChecksum();
                }
                
                // Mark header as part of size class
                header->magic = DefragHeader::MAGIC_FREE;
                header->is_free = true;
                header->prev = header->next = nullptr;
                header->updateChecksum();
                
                // Add block to size class free list
                size_class_lists_.addToFreeList(block, size_class);
                successful_blocks++;
            }
            
            // Update stats for successfully moved blocks
            if (successful_blocks > 0) {
                updateStats([successful_blocks](Stats& s) {
                    s.size_class_stats.allocations[0] += successful_blocks;
                    s.size_class_stats.deallocations[0] += successful_blocks;
                });
                
                // Add blocks to tracking set
                for (const auto& [block, size_class, block_size] : pending_size_class_blocks_) {
                    std::lock_guard<std::mutex> map_lock(size_class_map_mutex_);
                    size_class_blocks_.emplace(block, size_class);
                }
            }
        }
    }
    
    // Helper for updating stats safely
    template<typename F>
    void updateStats(F&& func) {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        func(stats_);
    }

    std::byte* memory_;                    // Raw memory buffer (base allocation)
    std::byte* memory_start_;              // Aligned start of usable pool
    std::byte* memory_end_;                // End of usable pool
    DefragHeader* first_block_;            // First block in the chain
    std::size_t total_size_requested_;     // Requested size
    std::size_t pool_size_;                // Usable pool size after alignment
    std::atomic<std::size_t> used_memory_; // Currently used memory
    std::atomic<std::size_t> allocation_count_; // Number of active allocations
    mutable std::mutex mutex_;             // Mutex for thread safety
    
    SizeClassFreeList size_class_lists_;   // Free lists for common sizes
    Stats stats_;                          // Statistics tracking
    mutable std::mutex stats_mutex_;       // Mutex for stats
    std::vector<std::tuple<void*, size_t, size_t>> pending_size_class_blocks_;
    std::unordered_map<void*, size_t> size_class_blocks_; // Map of block->size class
    mutable std::mutex size_class_map_mutex_;            // Mutex for size_class_blocks_ map
};

} // namespace memory
} // namespace pynovage