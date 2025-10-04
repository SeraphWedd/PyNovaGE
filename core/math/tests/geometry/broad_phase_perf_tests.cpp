#include "geometry/broad_phase.hpp"
#include <benchmark/benchmark.h>
#include <random>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

static void generateRandomAABB(AABB& aabb, std::mt19937& rng, float size_range = 5.0f, float pos_range = 100.0f) {
    std::uniform_real_distribution<float> size_dist(0.1f, size_range);
    std::uniform_real_distribution<float> pos_dist(-pos_range, pos_range);
    
    Vector3 center(pos_dist(rng), pos_dist(rng), pos_dist(rng));
    Vector3 half_size(size_dist(rng), size_dist(rng), size_dist(rng));
    aabb.min = center - half_size;
    aabb.max = center + half_size;
}

static void BM_BroadPhaseInsertion(benchmark::State& state) {
    const int num_objects = state.range(0);
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    
    for (auto _ : state) {
        state.PauseTiming();
        BroadPhase bp(10.0f);
        std::vector<AABB> aabbs(num_objects);
        for (auto& aabb : aabbs) {
            generateRandomAABB(aabb, rng);
        }
        state.ResumeTiming();
        
        for (const auto& aabb : aabbs) {
            bp.createProxy(aabb, false);
        }
    }
}
BENCHMARK(BM_BroadPhaseInsertion)->Range(8, 8<<10);

// Original non-batched update benchmark for comparison
static void BM_BroadPhaseUpdate_NoBatch(benchmark::State& state) {
    const int num_objects = state.range(0);
    std::mt19937 rng(42);
    
    BroadPhase bp(10.0f);
    std::vector<AABB> aabbs(num_objects);
    std::vector<ProxyId> proxies(num_objects);
    
    // Setup initial state
    for (size_t i = 0; i < num_objects; i++) {
        generateRandomAABB(aabbs[i], rng);
        proxies[i] = bp.createProxy(aabbs[i], false);
    }
    
    for (auto _ : state) {
        state.PauseTiming();
        // Generate new positions
        for (auto& aabb : aabbs) {
            generateRandomAABB(aabb, rng, 5.0f, 100.0f);
        }
        state.ResumeTiming();
        
        // Update all proxies
        for (size_t i = 0; i < num_objects; i++) {
            bp.updateProxy(proxies[i], aabbs[i]);
        }
    }
}
BENCHMARK(BM_BroadPhaseUpdate_NoBatch)->Range(8, 8<<10);

// New batched update benchmark
static void BM_BroadPhaseUpdate(benchmark::State& state) {
    const int num_objects = state.range(0);
    std::mt19937 rng(42);
    
    BroadPhase bp(10.0f);
    std::vector<AABB> aabbs(num_objects);
    std::vector<ProxyId> proxies(num_objects);
    
    // Setup initial state
    for (size_t i = 0; i < num_objects; i++) {
        generateRandomAABB(aabbs[i], rng);
        proxies[i] = bp.createProxy(aabbs[i], false);
    }
    
    for (auto _ : state) {
        state.PauseTiming();
        // Generate new positions
        for (auto& aabb : aabbs) {
            generateRandomAABB(aabb, rng, 5.0f, 100.0f);
        }
        state.ResumeTiming();
        
        // Update all proxies and finalize once
        for (size_t i = 0; i < num_objects; i++) {
            bp.updateProxy(proxies[i], aabbs[i]);
        }
        bp.finalizeBroadPhase();
    }
}
BENCHMARK(BM_BroadPhaseUpdate)->Range(8, 8<<10);

static void BM_BroadPhaseQuery(benchmark::State& state) {
    const int num_objects = state.range(0);
    std::mt19937 rng(42);
    
    BroadPhase bp(10.0f);
    
    // Setup scene with 80% dynamic and 20% static objects
    int num_static = num_objects / 5;
    int num_dynamic = num_objects - num_static;
    
    // Add objects
    for (int i = 0; i < num_static; i++) {
        AABB aabb;
        generateRandomAABB(aabb, rng);
        bp.createProxy(aabb, true);
    }
    
    for (int i = 0; i < num_dynamic; i++) {
        AABB aabb;
        generateRandomAABB(aabb, rng);
        bp.createProxy(aabb, false);
    }
    
    for (auto _ : state) {
        auto pairs = bp.findPotentialCollisions();
        benchmark::DoNotOptimize(pairs);
    }
}
BENCHMARK(BM_BroadPhaseQuery)->Range(8, 8<<10);

static void BM_BroadPhaseWorstCase(benchmark::State& state) {
    const int num_objects = state.range(0);
    
    // Create objects all in the same cell for worst-case scenario
    BroadPhase bp(100.0f);  // Large cell size to ensure all objects are in same cell
    
    for (int i = 0; i < num_objects; i++) {
        // Create overlapping AABBs
        AABB aabb;
        aabb.min = Vector3(-1.0f, -1.0f, -1.0f) + Vector3(0.1f * i, 0.1f * i, 0.1f * i);
        aabb.max = Vector3(1.0f, 1.0f, 1.0f) + Vector3(0.1f * i, 0.1f * i, 0.1f * i);
        bp.createProxy(aabb, false);
    }
    
    for (auto _ : state) {
        auto pairs = bp.findPotentialCollisions();
        benchmark::DoNotOptimize(pairs);
    }
}
BENCHMARK(BM_BroadPhaseWorstCase)->Range(8, 8<<10);

static void BM_BroadPhaseMixed(benchmark::State& state) {
    const int num_objects = state.range(0);
    std::mt19937 rng(42);
    
    BroadPhase bp(10.0f);
    std::vector<AABB> aabbs(num_objects);
    std::vector<ProxyId> proxies(num_objects);
    std::vector<bool> is_static_flags(num_objects);
    
    // Setup with mix of static/dynamic
    for (int i = 0; i < num_objects; i++) {
        generateRandomAABB(aabbs[i], rng);
        bool is_static = (i % 3 == 0);  // Make every third object static
        is_static_flags[i] = is_static;
        proxies[i] = bp.createProxy(aabbs[i], is_static);
    }
    
    std::uniform_real_distribution<float> update_dist(0.0f, 1.0f);
    
    for (auto _ : state) {
        // Update about 20% of dynamic objects
        for (size_t i = 0; i < static_cast<size_t>(num_objects); i++) {
            if (!is_static_flags[i] && update_dist(rng) < 0.2f) {
                state.PauseTiming();
                generateRandomAABB(aabbs[i], rng);
                state.ResumeTiming();
                bp.updateProxy(proxies[i], aabbs[i]);
            }
        }
        
        auto pairs = bp.findPotentialCollisions();
        benchmark::DoNotOptimize(pairs);
    }
}
BENCHMARK(BM_BroadPhaseMixed)->Range(8, 8<<10);
