#include "geometry/high_speed_collision.hpp"
#include <benchmark/benchmark.h>
#include <random>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

static void generateRandomSphere(Sphere& sphere, std::mt19937& rng, float radius_range = 5.0f, float pos_range = 100.0f) {
    std::uniform_real_distribution<float> radius_dist(0.1f, radius_range);
    std::uniform_real_distribution<float> pos_dist(-pos_range, pos_range);
    
    sphere.center = Vector3(pos_dist(rng), pos_dist(rng), pos_dist(rng));
    sphere.radius = radius_dist(rng);
}

static void generateRandomPenetrationParams(PenetrationTestParams& params, std::mt19937& rng, 
    float velocity_range = 1000.0f, float radius_range = 1.0f) {
    std::uniform_real_distribution<float> vel_dist(100.0f, velocity_range);
    std::uniform_real_distribution<float> radius_dist(0.05f, radius_range);
    std::uniform_real_distribution<float> pos_dist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> dir_dist(-1.0f, 1.0f);
    
    params.ray_origin = Vector3(pos_dist(rng), pos_dist(rng), pos_dist(rng));
    
    // Generate random direction and normalize it
    Vector3 dir(dir_dist(rng), dir_dist(rng), dir_dist(rng));
    dir.normalize();
    params.ray_direction = dir;
    
    params.velocity = vel_dist(rng);
    params.projectile_radius = radius_dist(rng);
}

// Benchmark sphere penetration tests with varying target counts
static void BM_SpherePenetrationTest(benchmark::State& state) {
    const int num_targets = state.range(0);
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    
    std::vector<Sphere> spheres(num_targets);
    PenetrationTestParams params;
    
    // Generate random spheres
    for (auto& sphere : spheres) {
        generateRandomSphere(sphere, rng);
    }
    
    for (auto _ : state) {
        state.PauseTiming();
        generateRandomPenetrationParams(params, rng);
        state.ResumeTiming();
        
        for (const auto& sphere : spheres) {
            auto result = TestSpherePenetration(sphere, params);
            benchmark::DoNotOptimize(result);
        }
    }
}
BENCHMARK(BM_SpherePenetrationTest)->Range(8, 8<<10);

// Benchmark AABB penetration tests
static void BM_AABBPenetrationTest(benchmark::State& state) {
    const int num_targets = state.range(0);
    std::mt19937 rng(42);
    
    std::vector<AABB> boxes(num_targets);
    PenetrationTestParams params;
    
    // Generate random AABBs
    for (auto& box : boxes) {
        std::uniform_real_distribution<float> size_dist(0.1f, 5.0f);
        std::uniform_real_distribution<float> pos_dist(-100.0f, 100.0f);
        
        Vector3 center(pos_dist(rng), pos_dist(rng), pos_dist(rng));
        Vector3 half_size(size_dist(rng), size_dist(rng), size_dist(rng));
        box.min = center - half_size;
        box.max = center + half_size;
    }
    
    for (auto _ : state) {
        state.PauseTiming();
        generateRandomPenetrationParams(params, rng);
        state.ResumeTiming();
        
        for (const auto& box : boxes) {
            auto result = TestAABBPenetration(box, params);
            benchmark::DoNotOptimize(result);
        }
    }
}
BENCHMARK(BM_AABBPenetrationTest)->Range(8, 8<<10);

// Benchmark worst-case scenario with many overlapping targets
static void BM_HighSpeedCollisionWorstCase(benchmark::State& state) {
    const int num_targets = state.range(0);
    std::mt19937 rng(42);
    
    std::vector<Sphere> spheres(num_targets);
    PenetrationTestParams params;
    
    // Create overlapping spheres in a small volume
    for (int i = 0; i < num_targets; i++) {
        spheres[i].center = Vector3(
            0.1f * static_cast<float>(i % 10),
            0.1f * static_cast<float>((i / 10) % 10),
            0.1f * static_cast<float>(i / 100)
        );
        spheres[i].radius = 1.0f;  // Large enough to guarantee overlaps
    }
    
    for (auto _ : state) {
        state.PauseTiming();
        generateRandomPenetrationParams(params, rng);
        // Ensure ray passes through the volume containing all spheres
        params.ray_origin = Vector3(-10.0f, -10.0f, -10.0f);
        params.ray_direction = Vector3(1.0f, 1.0f, 1.0f).normalized();
        state.ResumeTiming();
        
        for (const auto& sphere : spheres) {
            auto result = TestSpherePenetration(sphere, params);
            benchmark::DoNotOptimize(result);
        }
    }
}
BENCHMARK(BM_HighSpeedCollisionWorstCase)->Range(8, 8<<10);

// Benchmark mixed sphere and AABB penetration tests
static void BM_MixedPenetrationTest(benchmark::State& state) {
    const int num_targets = state.range(0);
    std::mt19937 rng(42);
    
    // Create a mix of spheres and AABBs
    std::vector<Sphere> spheres(num_targets / 2);
    std::vector<AABB> boxes(num_targets - num_targets / 2);
    PenetrationTestParams params;
    
    // Generate random objects
    for (auto& sphere : spheres) {
        generateRandomSphere(sphere, rng);
    }
    
    for (auto& box : boxes) {
        std::uniform_real_distribution<float> size_dist(0.1f, 5.0f);
        std::uniform_real_distribution<float> pos_dist(-100.0f, 100.0f);
        
        Vector3 center(pos_dist(rng), pos_dist(rng), pos_dist(rng));
        Vector3 half_size(size_dist(rng), size_dist(rng), size_dist(rng));
        box.min = center - half_size;
        box.max = center + half_size;
    }
    
    for (auto _ : state) {
        state.PauseTiming();
        generateRandomPenetrationParams(params, rng);
        state.ResumeTiming();
        
        // Test both types of objects
        for (const auto& sphere : spheres) {
            auto result = TestSpherePenetration(sphere, params);
            benchmark::DoNotOptimize(result);
        }
        
        for (const auto& box : boxes) {
            auto result = TestAABBPenetration(box, params);
            benchmark::DoNotOptimize(result);
        }
    }
}
BENCHMARK(BM_MixedPenetrationTest)->Range(8, 8<<10);