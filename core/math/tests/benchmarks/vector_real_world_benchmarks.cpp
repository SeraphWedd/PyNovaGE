#include <benchmark/benchmark.h>
#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
#include <vector>
#include <random>

using namespace pynovage::math;

namespace {

// Utility function to generate random vectors
template<typename Vec>
std::vector<Vec> generateRandomVectors(size_t count, float minVal = -100.0f, float maxVal = 100.0f) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(minVal, maxVal);
    
    std::vector<Vec> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        if constexpr (std::is_same_v<Vec, Vector2>) {
            result.emplace_back(dist(gen), dist(gen));
        } else if constexpr (std::is_same_v<Vec, Vector3>) {
            result.emplace_back(dist(gen), dist(gen), dist(gen));
        } else if constexpr (std::is_same_v<Vec, Vector4>) {
            result.emplace_back(dist(gen), dist(gen), dist(gen), dist(gen));
        }
    }
    return result;
}

// Benchmark batch normalization (common in particle systems)
template<typename Vec>
void BM_BatchNormalize(benchmark::State& state) {
    const size_t count = state.range(0);
    auto vectors = generateRandomVectors<Vec>(count);
    
    for (auto _ : state) {
        for (auto& v : vectors) {
            v.normalize();
        }
        benchmark::DoNotOptimize(vectors);
    }
    
    state.SetItemsProcessed(state.iterations() * count);
}

// Benchmark mixed operations (common in physics)
template<typename Vec>
void BM_MixedOperations(benchmark::State& state) {
    const size_t count = state.range(0);
    auto positions = generateRandomVectors<Vec>(count);
    auto velocities = generateRandomVectors<Vec>(count, -10.0f, 10.0f);
    auto forces = generateRandomVectors<Vec>(count, -50.0f, 50.0f);
    const float dt = 0.016f;  // 60 FPS
    const float mass = 1.0f;
    
    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            // F = ma -> a = F/m
            Vec acceleration = forces[i] / mass;
            velocities[i] += acceleration * dt;
            positions[i] += velocities[i] * dt;
            
            // Add some damping
            velocities[i] *= 0.99f;
        }
        benchmark::DoNotOptimize(positions);
        benchmark::DoNotOptimize(velocities);
    }
    
    state.SetItemsProcessed(state.iterations() * count);
}

// Benchmark collision checks (common in broad phase)
template<typename Vec>
void BM_CollisionChecks(benchmark::State& state) {
    const size_t count = state.range(0);
    auto positions = generateRandomVectors<Vec>(count);
    auto directions = generateRandomVectors<Vec>(count);
    
    // Normalize all directions
    for (auto& dir : directions) {
        dir.normalize();
    }
    
    for (auto _ : state) {
        float totalDots = 0.0f;
        for (size_t i = 0; i < count; ++i) {
            for (size_t j = i + 1; j < count; ++j) {
                Vec delta = positions[j] - positions[i];
                float distSq = delta.lengthSquared();
                if (distSq < 100.0f) {  // Within range
                    float dot = directions[i].dot(directions[j]);
                    totalDots += dot;
                }
            }
        }
        benchmark::DoNotOptimize(totalDots);
    }
    
    state.SetItemsProcessed(state.iterations() * (count * (count - 1) / 2));
}

// Benchmark transform chains (common in scene graphs)
template<typename Vec>
void BM_TransformChain(benchmark::State& state) {
    const size_t count = state.range(0);
    auto positions = generateRandomVectors<Vec>(count);
    auto scales = generateRandomVectors<Vec>(count, 0.5f, 2.0f);
    
    for (auto _ : state) {
        Vec finalPos = positions[0];
        for (size_t i = 1; i < count; ++i) {
            // Apply scale
            finalPos = finalPos.cwiseProduct(scales[i]);
            // Apply translation
            finalPos += positions[i];
        }
        benchmark::DoNotOptimize(finalPos);
    }
    
    state.SetItemsProcessed(state.iterations() * count);
}

} // namespace

// Register Vector2 benchmarks
BENCHMARK_TEMPLATE(BM_BatchNormalize, Vector2)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_MixedOperations, Vector2)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_CollisionChecks, Vector2)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_TransformChain, Vector2)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

// Register Vector3 benchmarks
BENCHMARK_TEMPLATE(BM_BatchNormalize, Vector3)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_MixedOperations, Vector3)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_CollisionChecks, Vector3)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_TransformChain, Vector3)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

// Register Vector4 benchmarks
BENCHMARK_TEMPLATE(BM_BatchNormalize, Vector4)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_MixedOperations, Vector4)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_CollisionChecks, Vector4)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_TransformChain, Vector4)
    ->RangeMultiplier(8)->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond);