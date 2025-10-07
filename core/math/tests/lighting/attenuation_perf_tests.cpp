#include <benchmark/benchmark.h>
#include "../../include/lighting/attenuation.hpp"
#include "../../include/lighting/light_types.hpp"
#include "../../include/vector3.hpp"
#include <vector>
#include <random>

using namespace pynovage::math;
using namespace pynovage::math::lighting;
using namespace pynovage::math::lighting::constants;

// Utility function to generate random points
static std::vector<Vector3> GenerateRandomPoints(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-10.0f, 10.0f);
    
    std::vector<Vector3> points;
    points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        points.emplace_back(
            dis(gen),
            dis(gen),
            dis(gen)
        );
    }
    return points;
}

// Basic attenuation calculation benchmark
static void BM_AttenuationCalculation(benchmark::State& state) {
    const Vector3 lightPos(0.0f, 5.0f, 0.0f);
    auto points = GenerateRandomPoints(1000);
    AttenuationParams params = AttenuationParams::ForRange(10.0f);
    
    size_t index = 0;
    for (auto _ : state) {
        float attenuation = calculateAttenuation(
            params,
            lightPos,
            points[index % points.size()],
            AttenuationModel::Smooth
        );
        benchmark::DoNotOptimize(attenuation);
        index++;
    }
}
BENCHMARK(BM_AttenuationCalculation);

// Compare different attenuation models
static void BM_AttenuationModels(benchmark::State& state) {
    const Vector3 lightPos(0.0f, 5.0f, 0.0f);
    const Vector3 point(1.0f, 0.0f, 0.0f);
    AttenuationParams params = AttenuationParams::ForRange(10.0f);
    AttenuationModel model = static_cast<AttenuationModel>(state.range(0));
    
    for (auto _ : state) {
        float attenuation = calculateAttenuation(params, lightPos, point, model);
        benchmark::DoNotOptimize(attenuation);
    }
}
BENCHMARK(BM_AttenuationModels)
    ->Arg(static_cast<int>(AttenuationModel::Linear))
    ->Arg(static_cast<int>(AttenuationModel::InverseSquare))
    ->Arg(static_cast<int>(AttenuationModel::Smooth))
    ->Arg(static_cast<int>(AttenuationModel::None));

// SIMD batch processing benchmark
static void BM_AttenuationBatch(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const Vector3 lightPos(0.0f, 5.0f, 0.0f);
    auto points = GenerateRandomPoints(numPoints);
    std::vector<float> results(numPoints);
    AttenuationParams params = AttenuationParams::ForRange(10.0f);
    
    for (auto _ : state) {
        calculateAttenuationBatch(
            params,
            lightPos,
            points.data(),
            numPoints,
            AttenuationModel::Smooth,
            results.data()
        );
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_AttenuationBatch)
    ->Arg(4)     // Single SIMD batch
    ->Arg(16)    // Small batch
    ->Arg(1024)  // Large batch
    ->Arg(4096); // Very large batch

// Edge case benchmarks
static void BM_AttenuationEdgeCases(benchmark::State& state) {
    const Vector3 lightPos(0.0f, 0.0f, 0.0f);
    AttenuationParams params = AttenuationParams::ForRange(10.0f);
    
    // Test different distances based on state.range(0)
    Vector3 point;
    switch (state.range(0)) {
        case 0: // Very close to light
            point = Vector3(0.001f, 0.0f, 0.0f);
            break;
        case 1: // At range boundary
            point = Vector3(10.0f, 0.0f, 0.0f);
            break;
        case 2: // Just beyond range
            point = Vector3(10.1f, 0.0f, 0.0f);
            break;
        case 3: // Far beyond range
            point = Vector3(100.0f, 0.0f, 0.0f);
            break;
    }
    
    for (auto _ : state) {
        float attenuation = calculateAttenuation(
            params,
            lightPos,
            point,
            AttenuationModel::Smooth
        );
        benchmark::DoNotOptimize(attenuation);
    }
}
BENCHMARK(BM_AttenuationEdgeCases)
    ->Arg(0)  // Very close
    ->Arg(1)  // At range
    ->Arg(2)  // Just beyond
    ->Arg(3); // Far beyond

// Range parameter impact benchmark
static void BM_AttenuationRanges(benchmark::State& state) {
    const Vector3 lightPos(0.0f, 0.0f, 0.0f);
    const float range = static_cast<float>(state.range(0));
    AttenuationParams params = AttenuationParams::ForRange(range);
    
    // Test points at different relative distances
    auto points = std::vector<Vector3>{
        Vector3(range * 0.25f, 0.0f, 0.0f),  // 25% of range
        Vector3(range * 0.5f, 0.0f, 0.0f),   // 50% of range
        Vector3(range * 0.75f, 0.0f, 0.0f),  // 75% of range
        Vector3(range * 0.9f, 0.0f, 0.0f)    // 90% of range
    };
    
    size_t index = 0;
    for (auto _ : state) {
        float attenuation = calculateAttenuation(
            params,
            lightPos,
            points[index % points.size()],
            AttenuationModel::Smooth
        );
        benchmark::DoNotOptimize(attenuation);
        index++;
    }
}
BENCHMARK(BM_AttenuationRanges)
    ->Arg(5)    // Short range
    ->Arg(20)   // Medium range
    ->Arg(50)   // Long range
    ->Arg(100); // Very long range