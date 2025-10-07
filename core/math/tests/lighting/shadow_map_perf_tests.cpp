#include <benchmark/benchmark.h>
#include "../../include/lighting/shadow_map.hpp"
#include "../../include/vector3.hpp"
#include <random>

using namespace pynovage::math;
using namespace pynovage::math::lighting;

// Utility function to generate random positions
static std::vector<Vector3> GenerateRandomPositions(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-100.0f, 100.0f);
    
    std::vector<Vector3> positions;
    positions.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        positions.emplace_back(
            dis(gen),
            dis(gen),
            dis(gen)
        );
    }
    return positions;
}

// Utility function to generate random directions
static std::vector<Vector3> GenerateRandomDirections(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    std::vector<Vector3> directions;
    directions.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        Vector3 dir(dis(gen), dis(gen), dis(gen));
        directions.push_back(dir.normalized());
    }
    return directions;
}

// Benchmark creation of different shadow map types
static void BM_ShadowMapCreation(benchmark::State& state) {
    for (auto _ : state) {
        ShadowMap standardMap(ShadowMapType::Standard);
        benchmark::DoNotOptimize(standardMap);
        
        ShadowMap cascadeMap(ShadowMapType::Cascade);
        benchmark::DoNotOptimize(cascadeMap);
        
        ShadowMap cubeMap(ShadowMapType::Cube);
        benchmark::DoNotOptimize(cubeMap);
    }
}
BENCHMARK(BM_ShadowMapCreation);

// Benchmark standard shadow map view matrix updates
static void BM_StandardShadowMapViewUpdate(benchmark::State& state) {
    ShadowMap shadowMap(ShadowMapType::Standard);
    auto positions = GenerateRandomPositions(1000);
    auto directions = GenerateRandomDirections(1000);
    
    size_t index = 0;
    for (auto _ : state) {
        shadowMap.updateViewMatrix(
            positions[index % positions.size()],
            directions[index % directions.size()]
        );
        benchmark::DoNotOptimize(shadowMap.getViewMatrix());
        index++;
    }
}
BENCHMARK(BM_StandardShadowMapViewUpdate);

// Benchmark cube shadow map view matrix updates
static void BM_CubeShadowMapViewUpdate(benchmark::State& state) {
    ShadowMap shadowMap(ShadowMapType::Cube);
    auto positions = GenerateRandomPositions(1000);
    
    size_t index = 0;
    for (auto _ : state) {
        shadowMap.updateViewMatrix(positions[index % positions.size()]);
        for (uint32_t face = 0; face < 6; ++face) {
            benchmark::DoNotOptimize(shadowMap.getCubeFaceViewMatrix(face));
        }
        index++;
    }
}
BENCHMARK(BM_CubeShadowMapViewUpdate);

// Benchmark projection matrix updates for different shadow map types
static void BM_ProjectionMatrixUpdate(benchmark::State& state) {
    const int mapType = state.range(0); // 0 = Standard, 1 = Cascade, 2 = Cube
    ShadowMap shadowMap(static_cast<ShadowMapType>(mapType));
    
    // Random near/far values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> near_dis(0.1f, 1.0f);
    std::uniform_real_distribution<float> far_dis(50.0f, 200.0f);
    std::uniform_real_distribution<float> fov_dis(30.0f, 120.0f);
    
    for (auto _ : state) {
        float near = near_dis(gen);
        float far = far_dis(gen);
        float fov = fov_dis(gen);
        
        shadowMap.updateProjectionMatrix(near, far, fov);
        benchmark::DoNotOptimize(shadowMap.getProjectionMatrix());
    }
}
BENCHMARK(BM_ProjectionMatrixUpdate)
    ->Arg(0)  // Standard shadow map
    ->Arg(1)  // Cascade shadow map
    ->Arg(2); // Cube shadow map

// Benchmark complete shadow map matrix chain computation
static void BM_CompleteMatrixChainUpdate(benchmark::State& state) {
    const int mapType = state.range(0); // 0 = Standard, 1 = Cascade, 2 = Cube
    ShadowMap shadowMap(static_cast<ShadowMapType>(mapType));
    auto positions = GenerateRandomPositions(100);
    auto directions = GenerateRandomDirections(100);
    
    size_t index = 0;
    for (auto _ : state) {
        // Update both view and projection matrices
        shadowMap.updateViewMatrix(
            positions[index % positions.size()],
            directions[index % directions.size()]
        );
        shadowMap.updateProjectionMatrix(0.1f, 100.0f, 90.0f);
        
        // Get final matrices
        benchmark::DoNotOptimize(shadowMap.getViewMatrix());
        benchmark::DoNotOptimize(shadowMap.getProjectionMatrix());
        
        if (mapType == 2) { // Cube map
            for (uint32_t face = 0; face < 6; ++face) {
                benchmark::DoNotOptimize(shadowMap.getCubeFaceViewMatrix(face));
            }
        }
        
        index++;
    }
}
BENCHMARK(BM_CompleteMatrixChainUpdate)
    ->Arg(0)  // Standard shadow map
    ->Arg(1)  // Cascade shadow map
    ->Arg(2); // Cube shadow map