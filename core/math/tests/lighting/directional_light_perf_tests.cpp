#include <benchmark/benchmark.h>
#include "../../include/lighting/directional_light.hpp"
#include "../../include/lighting/light_transforms.hpp"
#include "../../include/vector3.hpp"
#include <vector>
#include <random>

using namespace pynovage::math;
using namespace pynovage::math::lighting;

// Utility function to generate random vectors
static std::vector<Vector3> GenerateRandomVectors(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    std::vector<Vector3> vectors;
    vectors.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        Vector3 vec(dis(gen), dis(gen), dis(gen));
        vectors.push_back(vec.normalized());
    }
    return vectors;
}

// Basic direction vector calculations
static void BM_DirectionalLight_SetDirection(benchmark::State& state) {
    DirectionalLight light;
    auto directions = GenerateRandomVectors(1000);
    
    size_t index = 0;
    for (auto _ : state) {
        light.setDirection(directions[index % directions.size()]);
        benchmark::DoNotOptimize(light);
        index++;
    }
}
BENCHMARK(BM_DirectionalLight_SetDirection);

// Shadow bounds calculation
static void BM_DirectionalLight_ShadowBounds(benchmark::State& state) {
    DirectionalLight light(Vector3(0.0f, -1.0f, 0.0f));
    auto centers = GenerateRandomVectors(1000);
    std::vector<float> radii;
    
    // Generate random radii between 1 and 100
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0f, 100.0f);
    for (size_t i = 0; i < 1000; ++i) {
        radii.push_back(dis(gen));
    }
    
    Vector3 min, max;
    size_t index = 0;
    for (auto _ : state) {
        light.computeShadowBounds(
            centers[index % centers.size()],
            radii[index % radii.size()],
            min,
            max
        );
        benchmark::DoNotOptimize(min);
        benchmark::DoNotOptimize(max);
        index++;
    }
}
BENCHMARK(BM_DirectionalLight_ShadowBounds);

// Light space transform calculations
static void BM_DirectionalLight_ViewTransform(benchmark::State& state) {
    DirectionalLight light(Vector3(0.0f, -1.0f, 0.0f));
    auto centers = GenerateRandomVectors(1000);
    std::vector<float> radii;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0f, 100.0f);
    for (size_t i = 0; i < 1000; ++i) {
        radii.push_back(dis(gen));
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 view = LightSpaceTransform::createDirectionalLightView(
            light,
            centers[index % centers.size()],
            radii[index % radii.size()]
        );
        benchmark::DoNotOptimize(view);
        index++;
    }
}
BENCHMARK(BM_DirectionalLight_ViewTransform);

// Projection matrix calculations
static void BM_DirectionalLight_ProjectionTransform(benchmark::State& state) {
    DirectionalLight light(Vector3(0.0f, -1.0f, 0.0f));
    auto centers = GenerateRandomVectors(1000);
    std::vector<float> radii;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0f, 100.0f);
    for (size_t i = 0; i < 1000; ++i) {
        radii.push_back(dis(gen));
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 proj = LightSpaceTransform::createDirectionalLightProjection(
            light,
            centers[index % centers.size()],
            radii[index % radii.size()],
            0.1f,  // near plane
            100.0f // far plane
        );
        benchmark::DoNotOptimize(proj);
        index++;
    }
}
BENCHMARK(BM_DirectionalLight_ProjectionTransform);

// Combined view-projection transform
static void BM_DirectionalLight_ViewProjectionTransform(benchmark::State& state) {
    DirectionalLight light(Vector3(0.0f, -1.0f, 0.0f));
    auto centers = GenerateRandomVectors(1000);
    std::vector<float> radii;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0f, 100.0f);
    for (size_t i = 0; i < 1000; ++i) {
        radii.push_back(dis(gen));
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 transform = LightSpaceTransform::createLightSpaceTransform(
            light,
            centers[index % centers.size()],
            radii[index % radii.size()],
            0.1f,  // near plane
            100.0f // far plane
        );
        benchmark::DoNotOptimize(transform);
        index++;
    }
}
BENCHMARK(BM_DirectionalLight_ViewProjectionTransform);

// Batch light calculations
static void BM_DirectionalLight_BatchProcessing(benchmark::State& state) {
    const size_t BatchSize = state.range(0);
    std::vector<DirectionalLight> lights;
    std::vector<Vector3> centers;
    std::vector<float> radii;
    std::vector<Matrix4> transforms;
    
    // Generate test data
    auto directions = GenerateRandomVectors(BatchSize);
    centers = GenerateRandomVectors(BatchSize);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0f, 100.0f);
    
    for (size_t i = 0; i < BatchSize; ++i) {
        lights.emplace_back(directions[i]);
        radii.push_back(dis(gen));
    }
    transforms.resize(BatchSize);
    
    for (auto _ : state) {
        for (size_t i = 0; i < BatchSize; ++i) {
            transforms[i] = LightSpaceTransform::createLightSpaceTransform(
                lights[i],
                centers[i],
                radii[i],
                0.1f,
                100.0f
            );
        }
        benchmark::DoNotOptimize(transforms);
    }
}
BENCHMARK(BM_DirectionalLight_BatchProcessing)
    ->Arg(1)     // Single light
    ->Arg(4)     // Small batch
    ->Arg(16)    // Medium batch
    ->Arg(64)    // Large batch
    ->Arg(256);  // Very large batch
