#include <benchmark/benchmark.h>
#include "../../include/lighting/light_transforms.hpp"
#include "../../include/lighting/directional_light.hpp"
#include "../../include/lighting/point_light.hpp"
#include "../../include/lighting/spot_light.hpp"
#include "../../include/vector3.hpp"
#include <vector>
#include <random>

using namespace pynovage::math;
using namespace pynovage::math::lighting;

// Utility function to generate random points
static std::vector<Vector3> GenerateRandomPoints(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-100.0f, 100.0f);
    
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

// Directional Light Transforms
static void BM_DirectionalLightViewTransform(benchmark::State& state) {
    DirectionalLight light(Vector3(0.0f, -1.0f, 0.0f));
    auto centers = GenerateRandomPoints(1000);
    std::vector<float> radii;
    
    // Generate random radii
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
BENCHMARK(BM_DirectionalLightViewTransform);

static void BM_DirectionalLightProjectionTransform(benchmark::State& state) {
    DirectionalLight light(Vector3(0.0f, -1.0f, 0.0f));
    auto centers = GenerateRandomPoints(1000);
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
BENCHMARK(BM_DirectionalLightProjectionTransform);

// Point Light Transforms
static void BM_PointLightViewTransform(benchmark::State& state) {
    PointLight light(Vector3(0.0f, 5.0f, 0.0f));
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 view = LightSpaceTransform::createPointLightView(
            light,
            index % 6  // Cycle through all 6 cube faces
        );
        benchmark::DoNotOptimize(view);
        index++;
    }
}
BENCHMARK(BM_PointLightViewTransform);

static void BM_PointLightCubemapTransforms(benchmark::State& state) {
    PointLight light(Vector3(0.0f, 5.0f, 0.0f));
    std::vector<Matrix4> faceTransforms(6);
    
    for (auto _ : state) {
        // Generate transforms for all 6 faces
        for (int face = 0; face < 6; ++face) {
            faceTransforms[face] = LightSpaceTransform::createLightSpaceTransform(
                light,
                face,
                0.1f  // near plane
            );
        }
        benchmark::DoNotOptimize(faceTransforms);
    }
}
BENCHMARK(BM_PointLightCubemapTransforms);

// Spot Light Transforms
static void BM_SpotLightViewTransform(benchmark::State& state) {
    SpotLight light(
        Vector3(0.0f, 5.0f, 0.0f),      // position
        Vector3(0.0f, -1.0f, 0.0f)      // direction
    );
    
    for (auto _ : state) {
        Matrix4 view = LightSpaceTransform::createSpotLightView(light);
        benchmark::DoNotOptimize(view);
    }
}
BENCHMARK(BM_SpotLightViewTransform);

static void BM_SpotLightFrustumCalculation(benchmark::State& state) {
    SpotLight light(
        Vector3(0.0f, 5.0f, 0.0f),      // position
        Vector3(0.0f, -1.0f, 0.0f)      // direction
    );
    float fovY, aspect;
    
    for (auto _ : state) {
        LightSpaceTransform::calculateSpotLightFrustum(light, fovY, aspect);
        benchmark::DoNotOptimize(fovY);
        benchmark::DoNotOptimize(aspect);
    }
}
BENCHMARK(BM_SpotLightFrustumCalculation);

// Batch Transform Operations
static void BM_BatchTransformCalculation(benchmark::State& state) {
    const size_t BatchSize = state.range(0);
    std::vector<DirectionalLight> dirLights;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;
    std::vector<Matrix4> transforms;
    
    // Create test data
    auto positions = GenerateRandomPoints(BatchSize);
    auto directions = GenerateRandomVectors(BatchSize);
    
    for (size_t i = 0; i < BatchSize; ++i) {
        dirLights.emplace_back(directions[i]);
        pointLights.emplace_back(positions[i]);
        spotLights.emplace_back(positions[i], directions[i]);
    }
    
    transforms.resize(BatchSize * 8);  // Space for all transforms
    
    for (auto _ : state) {
        size_t transformIndex = 0;
        
        // Directional light transforms
        for (const auto& light : dirLights) {
            transforms[transformIndex++] = LightSpaceTransform::createLightSpaceTransform(
                light,
                Vector3(),  // center
                10.0f,     // radius
                0.1f,      // near
                100.0f     // far
            );
        }
        
        // Point light transforms (6 faces per light)
        for (const auto& light : pointLights) {
            for (int face = 0; face < 6; ++face) {
                transforms[transformIndex++] = LightSpaceTransform::createLightSpaceTransform(
                    light,
                    face,
                    0.1f  // near
                );
            }
        }
        
        // Spot light transforms
        for (const auto& light : spotLights) {
            transforms[transformIndex++] = LightSpaceTransform::createLightSpaceTransform(
                light,
                0.1f  // near
            );
        }
        
        benchmark::DoNotOptimize(transforms);
    }
}
BENCHMARK(BM_BatchTransformCalculation)
    ->Arg(1)    // Single light of each type
    ->Arg(4)    // Few lights
    ->Arg(16)   // Many lights
    ->Arg(64);  // Very many lights

// Bias Matrix Generation
static void BM_BiasMatrixGeneration(benchmark::State& state) {
    // Test both normal and depth bias matrix generation
    DirectionalLight light(Vector3(0.0f, -1.0f, 0.0f));
    
    for (auto _ : state) {
        Matrix4 normalBias = LightSpaceTransform::createNormalBiasMatrix(
            light,
            0.005f  // normal bias factor
        );
        Matrix4 depthBias = LightSpaceTransform::createDepthBiasMatrix(
            0.0001f,  // depth bias
            1.0f      // slope scale
        );
        benchmark::DoNotOptimize(normalBias);
        benchmark::DoNotOptimize(depthBias);
    }
}
BENCHMARK(BM_BiasMatrixGeneration);