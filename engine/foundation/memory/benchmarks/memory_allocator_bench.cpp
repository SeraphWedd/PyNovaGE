#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include <memory>
#include <algorithm>
#include <chrono>

#include "memory/allocator.h"
#include "memory/memory_pool.h"
#include "memory/stack_allocator.h"
#include "memory/object_pool.h"

using namespace PyNovaGE;

namespace {

//------------------------------------------------------------------------------
// Real-world Game Engine Data Structures
//------------------------------------------------------------------------------

// Typical game object sizes
constexpr size_t GAME_OBJECT_SIZE = 128;      // Transform + component references
constexpr size_t SPRITE_SIZE = 64;            // Sprite render data
constexpr size_t PARTICLE_SIZE = 32;          // Particle data
constexpr size_t AUDIO_BUFFER_SIZE = 4096;    // Audio sample buffer
constexpr size_t MESH_VERTEX_SIZE = 48;       // Position + normal + UV + tangent

// Realistic batch sizes
constexpr size_t MAX_GAME_OBJECTS = 10000;    // Typical scene object count
constexpr size_t MAX_SPRITES = 50000;         // 2D sprite batch size
constexpr size_t MAX_PARTICLES = 100000;      // Particle system size
constexpr size_t AUDIO_BUFFERS_PER_FRAME = 64; // Audio streaming buffers
constexpr size_t VERTICES_PER_FRAME = 500000; // Typical frame vertex count

// Memory allocator wrapper for standard allocator comparison
class StandardAllocator : public Allocator {
private:
    size_t total_allocated_ = 0;
    size_t peak_allocated_ = 0;

public:
    void* allocate(size_t size, size_t alignment = 16) override {
#ifdef _WIN32
        void* ptr = _aligned_malloc(size, alignment);
#else
        void* ptr = std::aligned_alloc(alignment, size);
#endif
        if (ptr) {
            total_allocated_ += size;
            peak_allocated_ = std::max(peak_allocated_, total_allocated_);
        }
        return ptr;
    }

    void deallocate(void* ptr) override {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        std::free(ptr);
#endif
    }

    size_t getTotalAllocated() const override { return total_allocated_; }
    size_t getPeakAllocated() const override { return peak_allocated_; }
    void resetStats() override { total_allocated_ = 0; peak_allocated_ = 0; }
};

// Test data structures
struct GameObject {
    float transform[16];        // 4x4 matrix
    uint32_t components[16];    // Component IDs
    uint32_t id;
    uint32_t parent_id;
    uint32_t flags;
    uint32_t padding;

    GameObject() : id(0), parent_id(0), flags(0), padding(0) {
        std::fill(std::begin(transform), std::end(transform), 0.0f);
        std::fill(std::begin(components), std::end(components), 0);
    }
};

struct Sprite {
    float position[3];
    float uv[4];               // UV coordinates
    uint32_t texture_id;
    uint32_t layer;
    float tint[4];             // RGBA
    uint32_t flags;
    uint32_t padding[3];

    Sprite() : texture_id(0), layer(0), flags(0) {
        std::fill(std::begin(position), std::end(position), 0.0f);
        std::fill(std::begin(uv), std::end(uv), 0.0f);
        std::fill(std::begin(tint), std::end(tint), 1.0f);
        std::fill(std::begin(padding), std::end(padding), 0);
    }
};

struct Particle {
    float position[3];
    float velocity[3];
    float life_time;
    uint32_t color;

    Particle() : life_time(1.0f), color(0xFFFFFFFF) {
        std::fill(std::begin(position), std::end(position), 0.0f);
        std::fill(std::begin(velocity), std::end(velocity), 0.0f);
    }
};

// Data generator for realistic allocation patterns
class GameDataGenerator {
public:
    mutable std::mt19937 rng;
    mutable std::uniform_int_distribution<size_t> size_dist;
    mutable std::uniform_real_distribution<double> chance_dist;
    GameDataGenerator() : rng(42), size_dist(16, 1024), chance_dist(0.0f, 1.0f) {}
    
    // Simulate variable-sized temporary allocations (UI, strings, temp buffers)
    size_t getRandomTempSize() const {
        return size_dist(rng);
    }
    
    // Simulate object lifetime patterns
    bool shouldDeallocate(float probability = 0.3f) const {
        return chance_dist(rng) < probability;
    }
    
    // Generate realistic frame allocation patterns
    std::vector<size_t> generateFrameAllocationPattern(size_t base_count) const {
        std::vector<size_t> sizes;
        sizes.reserve(base_count * 2); // Allow for spike allocations
        
        // Base allocations
        for (size_t i = 0; i < base_count; ++i) {
            sizes.push_back(getRandomTempSize());
        }
        
        // Spike allocations (30% chance of 2x allocations)
        if (chance_dist(rng) < 0.3f) {
            for (size_t i = 0; i < base_count; ++i) {
                sizes.push_back(getRandomTempSize());
            }
        }
        
        return sizes;
    }
};

} // anonymous namespace

//------------------------------------------------------------------------------
// Basic Allocator Performance Benchmarks
//------------------------------------------------------------------------------

static void BM_SystemAllocator_SmallAllocations(benchmark::State& state) {
    const size_t N = state.range(0);
    SystemAllocator allocator;
    std::vector<void*> pointers(N);
    
    for (auto _ : state) {
        // Allocate
        for (size_t i = 0; i < N; ++i) {
            pointers[i] = allocator.allocate(64, 16);
        }
        
        // Deallocate
        for (size_t i = 0; i < N; ++i) {
            allocator.deallocate(pointers[i]);
        }
        
        benchmark::DoNotOptimize(pointers);
    }
    
    state.SetItemsProcessed(state.iterations() * N * 2); // alloc + dealloc
}

static void BM_LinearAllocator_SmallAllocations(benchmark::State& state) {
    const size_t N = state.range(0);
    LinearAllocator allocator(N * 64 + 4096); // Ensure we have enough space
    std::vector<void*> pointers(N);
    
    for (auto _ : state) {
        allocator.reset();
        
        // Allocate (no deallocation needed for linear allocator)
        for (size_t i = 0; i < N; ++i) {
            pointers[i] = allocator.allocate(64, 16);
        }
        
        benchmark::DoNotOptimize(pointers);
    }
    
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_MemoryPool_SmallAllocations(benchmark::State& state) {
    const size_t N = state.range(0);
    MemoryPool pool(64, N);
    std::vector<void*> pointers(N);
    
    for (auto _ : state) {
        // Allocate
        for (size_t i = 0; i < N; ++i) {
            pointers[i] = pool.allocate(64, 16);
        }
        
        // Deallocate
        for (size_t i = 0; i < N; ++i) {
            pool.deallocate(pointers[i]);
        }
        
        benchmark::DoNotOptimize(pointers);
    }
    
    state.SetItemsProcessed(state.iterations() * N * 2); // alloc + dealloc
}

//------------------------------------------------------------------------------
// Real-World Game Engine Scenarios
//------------------------------------------------------------------------------

static void BM_GameObjectLifecycle_SystemAllocator(benchmark::State& state) {
    const size_t N = state.range(0);
    SystemAllocator allocator;
    GameDataGenerator gen;
    std::vector<GameObject*> active_objects;
    active_objects.reserve(N);
    
    for (auto _ : state) {
        // Simulate a game frame with object creation/destruction
        
        // Create new objects
        size_t new_objects = N / 10; // 10% new objects per frame
        for (size_t i = 0; i < new_objects; ++i) {
            GameObject* obj = static_cast<GameObject*>(
                allocator.allocate(sizeof(GameObject), alignof(GameObject)));
            if (obj) {
                new (obj) GameObject();
                active_objects.push_back(obj);
            }
        }
        
        // Destroy some old objects
        size_t to_remove = std::min(active_objects.size() / 20, active_objects.size()); // 5% destruction
        for (size_t i = 0; i < to_remove; ++i) {
            if (!active_objects.empty()) {
                GameObject* obj = active_objects.back();
                active_objects.pop_back();
                obj->~GameObject();
                allocator.deallocate(obj);
            }
        }
        
        benchmark::DoNotOptimize(active_objects.size());
    }
    
    // Cleanup remaining objects
    for (GameObject* obj : active_objects) {
        obj->~GameObject();
        allocator.deallocate(obj);
    }
    
    state.SetItemsProcessed(state.iterations() * (N / 10 + N / 20));
}

static void BM_GameObjectLifecycle_ObjectPool(benchmark::State& state) {
    const size_t N = state.range(0);
    ObjectPool<GameObject> pool(N);
    std::vector<GameObject*> active_objects;
    active_objects.reserve(N);
    
    for (auto _ : state) {
        // Create new objects
        size_t new_objects = N / 10; // 10% new objects per frame
        for (size_t i = 0; i < new_objects; ++i) {
            GameObject* obj = pool.acquire();
            if (obj) {
                active_objects.push_back(obj);
            }
        }
        
        // Destroy some old objects
        size_t to_remove = std::min(active_objects.size() / 20, active_objects.size()); // 5% destruction
        for (size_t i = 0; i < to_remove; ++i) {
            if (!active_objects.empty()) {
                GameObject* obj = active_objects.back();
                active_objects.pop_back();
                pool.release(obj);
            }
        }
        
        benchmark::DoNotOptimize(active_objects.size());
    }
    
    // Cleanup remaining objects
    for (GameObject* obj : active_objects) {
        pool.release(obj);
    }
    
    state.SetItemsProcessed(state.iterations() * (N / 10 + N / 20));
}

static void BM_ParticleSystem_MemoryPool(benchmark::State& state) {
    const size_t N = state.range(0);
    MemoryPool particle_pool(sizeof(Particle), N);
    GameDataGenerator gen;
    std::vector<Particle*> active_particles;
    active_particles.reserve(N);
    
    for (auto _ : state) {
        // Simulate particle emission and expiration
        
        // Emit new particles
        size_t new_particles = N / 60; // Emit particles every frame
        for (size_t i = 0; i < new_particles; ++i) {
            Particle* particle = static_cast<Particle*>(
                particle_pool.allocate(sizeof(Particle), alignof(Particle)));
            if (particle) {
                new (particle) Particle();
                particle->life_time = static_cast<float>(gen.chance_dist(gen.rng) * 3.0); // 0-3 second lifetime
                active_particles.push_back(particle);
            }
        }
        
        // Age and remove expired particles
        auto it = active_particles.begin();
        while (it != active_particles.end()) {
            (*it)->life_time -= 0.016667f; // 60 FPS delta time
            if ((*it)->life_time <= 0.0f) {
                (*it)->~Particle();
                particle_pool.deallocate(*it);
                it = active_particles.erase(it);
            } else {
                ++it;
            }
        }
        
        benchmark::DoNotOptimize(active_particles.size());
    }
    
    // Cleanup remaining particles
    for (Particle* particle : active_particles) {
        particle->~Particle();
        particle_pool.deallocate(particle);
    }
    
    state.SetItemsProcessed(state.iterations() * (N / 60));
}

static void BM_FrameAllocations_LinearAllocator(benchmark::State& state) {
    const size_t frame_budget = state.range(0);
    LinearAllocator frame_allocator(frame_budget * 2); // Double budget for safety
    GameDataGenerator gen;
    size_t total_allocations = 0;
    
    for (auto _ : state) {
        frame_allocator.reset(); // Start fresh each frame
        
        // Simulate typical frame allocations
        auto allocation_pattern = gen.generateFrameAllocationPattern(frame_budget / 128);
        
        std::vector<void*> temp_pointers;
        temp_pointers.reserve(allocation_pattern.size());
        
        for (size_t alloc_size : allocation_pattern) {
            void* ptr = frame_allocator.allocate(alloc_size, 16);
            if (ptr) {
                temp_pointers.push_back(ptr);
                total_allocations++;
                
                // Simulate some work with the allocated memory
                std::memset(ptr, 0xAB, alloc_size);
            }
        }
        
        // No explicit deallocation needed - frame allocator resets
        benchmark::DoNotOptimize(temp_pointers.size());
    }
    
    state.SetItemsProcessed(state.iterations() * total_allocations);
}

static void BM_FrameAllocations_SystemAllocator(benchmark::State& state) {
    const size_t frame_budget = state.range(0);
    SystemAllocator allocator;
    GameDataGenerator gen;
    size_t total_allocations = 0;
    
    for (auto _ : state) {
        // Simulate typical frame allocations
        auto allocation_pattern = gen.generateFrameAllocationPattern(frame_budget / 128);
        
        std::vector<void*> temp_pointers;
        temp_pointers.reserve(allocation_pattern.size());
        
        for (size_t alloc_size : allocation_pattern) {
            void* ptr = allocator.allocate(alloc_size, 16);
            if (ptr) {
                temp_pointers.push_back(ptr);
                total_allocations++;
                
                // Simulate some work with the allocated memory
                std::memset(ptr, 0xAB, alloc_size);
            }
        }
        
        // Deallocate all temporary allocations
        for (void* ptr : temp_pointers) {
            allocator.deallocate(ptr);
        }
        
        benchmark::DoNotOptimize(temp_pointers.size());
    }
    
    state.SetItemsProcessed(state.iterations() * total_allocations);
}

static void BM_ScopeBasedAllocations_StackAllocator(benchmark::State& state) {
    const size_t N = state.range(0);
    StackAllocator stack_allocator(N * 256); // Generous stack space
    GameDataGenerator gen;
    
    for (auto _ : state) {
        // Simulate nested scope allocations
        auto scope1 = stack_allocator.pushMarker();
        
        // Level 1 allocations (function scope)
        std::vector<void*> level1_ptrs;
        for (size_t i = 0; i < 10; ++i) {
            void* ptr = stack_allocator.allocate(gen.getRandomTempSize(), 16);
            if (ptr) level1_ptrs.push_back(ptr);
        }
        
        // Nested scope
        auto scope2 = stack_allocator.pushMarker();
        
        // Level 2 allocations (inner loop scope)
        std::vector<void*> level2_ptrs;
        for (size_t i = 0; i < 20; ++i) {
            void* ptr = stack_allocator.allocate(gen.getRandomTempSize() / 2, 16);
            if (ptr) level2_ptrs.push_back(ptr);
        }
        
        // Even deeper scope
        auto scope3 = stack_allocator.pushMarker();
        
        // Level 3 allocations (temporary calculations)
        for (size_t i = 0; i < 5; ++i) {
            void* ptr = stack_allocator.allocate(gen.getRandomTempSize() / 4, 16);
            benchmark::DoNotOptimize(ptr);
        }
        
        // Pop scopes in reverse order
        stack_allocator.popToMarker(scope3);
        benchmark::DoNotOptimize(level2_ptrs.size());
        
        stack_allocator.popToMarker(scope2);
        benchmark::DoNotOptimize(level1_ptrs.size());
        
        stack_allocator.popToMarker(scope1);
    }
    
    state.SetItemsProcessed(state.iterations() * 35); // Total allocations per iteration
}

//------------------------------------------------------------------------------
// Real-World Mixed Scenario Benchmarks
//------------------------------------------------------------------------------

static void BM_TypicalFrameScenario_MixedAllocators(benchmark::State& state) {
    const size_t object_count = state.range(0);
    
    // Set up different allocators for different use cases
    ObjectPool<GameObject> object_pool(object_count);
    ObjectPool<Sprite> sprite_pool(object_count * 5);
    MemoryPool particle_pool(sizeof(Particle), object_count * 10);
    LinearAllocator frame_allocator(1024 * 1024); // 1MB frame budget
    
    GameDataGenerator gen;
    
    std::vector<GameObject*> objects;
    std::vector<Sprite*> sprites;
    std::vector<Particle*> particles;
    objects.reserve(object_count);
    sprites.reserve(object_count * 5);
    particles.reserve(object_count * 10);
    
    size_t frame_operations = 0;
    
    for (auto _ : state) {
        frame_allocator.reset(); // New frame
        
        // Game object updates
        if (gen.chance_dist(gen.rng) < 0.1) { // 10% chance to create object
            GameObject* obj = object_pool.acquire();
            if (obj) objects.push_back(obj);
        }
        
        // Sprite batching
        for (size_t i = 0; i < 50; ++i) { // Typical sprite batch
            Sprite* sprite = sprite_pool.acquire();
            if (sprite) {
                sprites.push_back(sprite);
                frame_operations++;
            }
        }
        
        // Particle system update
        for (size_t i = 0; i < 100; ++i) { // Emit particles
            Particle* particle = static_cast<Particle*>(
                particle_pool.allocate(sizeof(Particle), alignof(Particle)));
            if (particle) {
                new (particle) Particle();
                particles.push_back(particle);
                frame_operations++;
            }
        }
        
        // Frame temporary allocations
        std::vector<void*> temp_allocations;
        for (size_t i = 0; i < 20; ++i) { // UI elements, strings, temp buffers
            void* ptr = frame_allocator.allocate(gen.getRandomTempSize(), 16);
            if (ptr) {
                temp_allocations.push_back(ptr);
                std::memset(ptr, i & 0xFF, gen.getRandomTempSize());
                frame_operations++;
            }
        }
        
        // Cleanup some objects (simulate object destruction)
        if (objects.size() > 100) {
            for (size_t i = 0; i < 10; ++i) {
                object_pool.release(objects.back());
                objects.pop_back();
            }
        }
        
        // Cleanup old sprites
        if (sprites.size() > 1000) {
            for (size_t i = 0; i < 50; ++i) {
                sprite_pool.release(sprites.back());
                sprites.pop_back();
            }
        }
        
        // Age particles
        auto it = particles.begin();
        size_t aged = 0;
        while (it != particles.end() && aged < 50) { // Age some particles
            (*it)->life_time -= 0.016667f;
            if ((*it)->life_time <= 0.0f) {
                (*it)->~Particle();
                particle_pool.deallocate(*it);
                it = particles.erase(it);
            } else {
                ++it;
            }
            aged++;
        }
        
        benchmark::DoNotOptimize(frame_operations);
    }
    
    // Cleanup
    for (auto* obj : objects) object_pool.release(obj);
    for (auto* sprite : sprites) sprite_pool.release(sprite);
    for (auto* particle : particles) {
        particle->~Particle();
        particle_pool.deallocate(particle);
    }
    
    state.SetItemsProcessed(state.iterations() * frame_operations);
}

//------------------------------------------------------------------------------
// Benchmark Registration
//------------------------------------------------------------------------------

// Basic allocator performance
BENCHMARK(BM_SystemAllocator_SmallAllocations)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<18)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_LinearAllocator_SmallAllocations)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<18)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_MemoryPool_SmallAllocations)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<18)
    ->Unit(benchmark::kNanosecond);

// Game engine scenarios
BENCHMARK(BM_GameObjectLifecycle_SystemAllocator)
    ->RangeMultiplier(2)
    ->Range(1<<8, 1<<16)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_GameObjectLifecycle_ObjectPool)
    ->RangeMultiplier(2)
    ->Range(1<<8, 1<<16)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_ParticleSystem_MemoryPool)
    ->RangeMultiplier(2)
    ->Range(1<<12, 1<<18)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_FrameAllocations_LinearAllocator)
    ->RangeMultiplier(2)
    ->Range(1<<12, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_FrameAllocations_SystemAllocator)
    ->RangeMultiplier(2)
    ->Range(1<<12, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_ScopeBasedAllocations_StackAllocator)
    ->RangeMultiplier(2)
    ->Range(1<<8, 1<<14)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_TypicalFrameScenario_MixedAllocators)
    ->RangeMultiplier(2)
    ->Range(1<<6, 1<<12)
    ->Unit(benchmark::kNanosecond);