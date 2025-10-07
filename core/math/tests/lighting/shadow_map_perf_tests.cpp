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

// Basic shadow map parameter updates
static void BM_ShadowMap_SetParameters(benchmark::State& state) {
    ShadowMap shadowMap;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> bias_dis(0.0001f, 0.01f);
    std::uniform_real_distribution<float> radius_dis(1.0f, 5.0f);
    std::vector<ShadowMapParameters> params;
    
    // Generate random parameter sets
    for (size_t i = 0; i < 1000; ++i) {
        ShadowMapParameters p;
        p.bias = bias_dis(gen);
        p.normalBias = bias_dis(gen);
        p.pcfRadius = radius_dis(gen);
        params.push_back(p);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        shadowMap = ShadowMap(ShadowMapType::Standard, params[index % params.size()]);
        benchmark::DoNotOptimize(shadowMap.getParameters());
        index++;
    }
}
BENCHMARK(BM_ShadowMap_SetParameters);

// Benchmark creation and initialization of different shadow map types
static void BM_ShadowMap_TypeInitialization(benchmark::State& state) {
    const int mapType = state.range(0); // 0 = Standard, 1 = Cascade, 2 = Cube
    ShadowMapParameters params;
    if (mapType == 1) { // Cascade
        params.resolution = 2048; // Higher resolution for cascades
    }
    
    for (auto _ : state) {
        ShadowMap shadowMap(static_cast<ShadowMapType>(mapType), params);
        benchmark::DoNotOptimize(shadowMap);
    }
}
BENCHMARK(BM_ShadowMap_TypeInitialization)
    ->Arg(0)  // Standard shadow map
    ->Arg(1)  // Cascade shadow map
    ->Arg(2); // Cube shadow map

// Cascade shadow map specific operations
static void BM_ShadowMap_CascadeUpdates(benchmark::State& state) {
    ShadowMap shadowMap(ShadowMapType::Cascade);
    auto positions = GenerateRandomPositions(1000);
    auto directions = GenerateRandomDirections(1000);
    CascadeConfig config;
    
    // Random cascade configurations
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> split_dis(10.0f, 1000.0f);
    std::uniform_real_distribution<float> blend_dis(1.0f, 10.0f);
    
    std::vector<CascadeConfig> configs;
    for (size_t i = 0; i < 100; ++i) {
        CascadeConfig cfg;
        for (uint32_t j = 0; j < cfg.MAX_CASCADES; ++j) {
            cfg.splitDistances[j] = split_dis(gen);
        }
        cfg.cascadeBlendDistance = blend_dis(gen);
        configs.push_back(cfg);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        shadowMap.updateViewMatrix(
            positions[index % positions.size()],
            directions[index % directions.size()]
        );
        
        // Update projection for each cascade level
        const auto& cfg = configs[index % configs.size()];
        for (uint32_t level = 0; level < cfg.numCascades; ++level) {
            float nearPlane = level == 0 ? 0.1f : cfg.splitDistances[level - 1];
            float farPlane = cfg.splitDistances[level];
            shadowMap.updateProjectionMatrix(nearPlane, farPlane);
            benchmark::DoNotOptimize(shadowMap.getProjectionMatrix());
        }
        index++;
    }
}
BENCHMARK(BM_ShadowMap_CascadeUpdates);

// Cubemap face matrix generation and updates
static void BM_ShadowMap_CubemapFaceUpdates(benchmark::State& state) {
    const size_t NumPositions = state.range(0);
    ShadowMap shadowMap(ShadowMapType::Cube);
    auto positions = GenerateRandomPositions(NumPositions);
    std::vector<Matrix4> faceMatrices(6);
    
    size_t index = 0;
    for (auto _ : state) {
        const auto& pos = positions[index % positions.size()];
        shadowMap.updateViewMatrix(pos);
        
        // Get and store all face matrices
        for (uint32_t face = 0; face < 6; ++face) {
            faceMatrices[face] = shadowMap.getCubeFaceViewMatrix(face);
        }
        benchmark::DoNotOptimize(faceMatrices);
        index++;
    }
}
BENCHMARK(BM_ShadowMap_CubemapFaceUpdates)
    ->Arg(1)    // Single position
    ->Arg(10)   // Few positions
    ->Arg(100)  // Many positions
    ->Arg(1000); // Very many positions

// Complete shadow mapping pipeline for each type
static void BM_ShadowMap_CompletePipeline(benchmark::State& state) {
    const int mapType = state.range(0); // 0 = Standard, 1 = Cascade, 2 = Cube
    ShadowMap shadowMap(static_cast<ShadowMapType>(mapType));
    auto positions = GenerateRandomPositions(100);
    auto directions = GenerateRandomDirections(100);
    
    // Initialize parameters
    ShadowMapParameters params;
    params.resolution = 2048;
    params.bias = 0.005f;
    params.normalBias = 0.4f;
    params.bleedReduction = 0.2f;
    params.pcfSamples = 16;
    params.pcfRadius = 3.0f;
    
    CascadeConfig cascadeConfig;
    size_t index = 0;
    
    for (auto _ : state) {
        const auto& pos = positions[index % positions.size()];
        const auto& dir = directions[index % directions.size()];
        
        // Update matrices based on shadow map type
        switch (mapType) {
            case 0: // Standard
                shadowMap.updateViewMatrix(pos, dir);
                shadowMap.updateProjectionMatrix(0.1f, 100.0f);
                benchmark::DoNotOptimize(shadowMap.getViewMatrix());
                benchmark::DoNotOptimize(shadowMap.getProjectionMatrix());
                break;
                
            case 1: // Cascade
                for (uint32_t level = 0; level < cascadeConfig.numCascades; ++level) {
                    float nearPlane = level == 0 ? 0.1f : cascadeConfig.splitDistances[level - 1];
                    float farPlane = cascadeConfig.splitDistances[level];
                    shadowMap.updateViewMatrix(pos, dir);
                    shadowMap.updateProjectionMatrix(nearPlane, farPlane);
                    benchmark::DoNotOptimize(shadowMap.getViewMatrix());
                    benchmark::DoNotOptimize(shadowMap.getProjectionMatrix());
                }
                break;
                
            case 2: // Cube
                shadowMap.updateViewMatrix(pos);
                shadowMap.updateProjectionMatrix(0.1f, 100.0f, 90.0f);
                for (uint32_t face = 0; face < 6; ++face) {
                    benchmark::DoNotOptimize(shadowMap.getCubeFaceViewMatrix(face));
                }
                break;
        }
        
        index++;
    }
}
BENCHMARK(BM_ShadowMap_CompletePipeline)
    ->Arg(0)  // Standard shadow map
    ->Arg(1)  // Cascade shadow map
    ->Arg(2); // Cube shadow map

// Multi-cascaded shadow map batch operations
static void BM_ShadowMap_BatchCascadeOperations(benchmark::State& state) {
    const size_t NumCascades = state.range(0);
    std::vector<ShadowMap> cascadeMaps(NumCascades, ShadowMap(ShadowMapType::Cascade));
    auto positions = GenerateRandomPositions(100);
    auto directions = GenerateRandomDirections(100);
    
    // Cascade configurations
    std::vector<CascadeConfig> configs(NumCascades);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> split_dis(10.0f, 1000.0f);
    
    // Initialize cascade configurations
    for (auto& cfg : configs) {
        for (uint32_t i = 0; i < cfg.MAX_CASCADES; ++i) {
            cfg.splitDistances[i] = split_dis(gen);
        }
    }
    
    size_t index = 0;
    std::vector<Matrix4> viewProjections(NumCascades);
    
    for (auto _ : state) {
        const auto& pos = positions[index % positions.size()];
        const auto& dir = directions[index % directions.size()];
        
        // Update all cascade levels
        for (size_t i = 0; i < NumCascades; ++i) {
            auto& shadowMap = cascadeMaps[i];
            const auto& cfg = configs[i];
            
            shadowMap.updateViewMatrix(pos, dir);
            shadowMap.updateProjectionMatrix(0.1f, cfg.splitDistances[0]);
            
            // Combine view and projection
            viewProjections[i] = shadowMap.getProjectionMatrix() * shadowMap.getViewMatrix();
        }
        
        benchmark::DoNotOptimize(viewProjections);
        index++;
    }
}
BENCHMARK(BM_ShadowMap_BatchCascadeOperations)
    ->Arg(2)    // Two cascades
    ->Arg(4)    // Four cascades
    ->Arg(8)    // Eight cascades
    ->Arg(16);  // Sixteen cascades
