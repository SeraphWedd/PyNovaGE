#include <benchmark/benchmark.h>
#include "../../include/lighting/point_light.hpp"
#include "../../include/lighting/attenuation.hpp"
#include "../../include/lighting/light_transforms.hpp"
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

// Basic position calculations
static void BM_PointLight_SetPosition(benchmark::State& state) {
    PointLight light;
    auto positions = GenerateRandomPoints(1000);
    
    size_t index = 0;
    for (auto _ : state) {
        light.setPosition(positions[index % positions.size()]);
        benchmark::DoNotOptimize(light);
        index++;
    }
}
BENCHMARK(BM_PointLight_SetPosition);

// Range-based attenuation setup
static void BM_PointLight_SetRange(benchmark::State& state) {
    PointLight light;
    std::vector<float> ranges;
    
    // Generate random ranges between minimum and maximum
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(MIN_LIGHT_RANGE, MAX_LIGHT_RANGE);
    for (size_t i = 0; i < 1000; ++i) {
        ranges.push_back(dis(gen));
    }
    
    size_t index = 0;
    for (auto _ : state) {
        light.setRange(ranges[index % ranges.size()]);
        benchmark::DoNotOptimize(light);
        index++;
    }
}
BENCHMARK(BM_PointLight_SetRange);

// Light-to-point intensity calculations
static void BM_PointLight_IntensityAtPoint(benchmark::State& state) {
    PointLight light(Vector3(0.0f, 5.0f, 0.0f), 10.0f);
    auto points = GenerateRandomPoints(1000);
    
    size_t index = 0;
    for (auto _ : state) {
        float intensity = calculateAttenuation(
            light.attenuation,
            light.position,
            points[index % points.size()],
            light.attenuationModel
        );
        benchmark::DoNotOptimize(intensity);
        index++;
    }
}
BENCHMARK(BM_PointLight_IntensityAtPoint);

// Batch intensity calculations
static void BM_PointLight_BatchIntensity(benchmark::State& state) {
    const size_t BatchSize = state.range(0);
    PointLight light(Vector3(0.0f, 5.0f, 0.0f), 10.0f);
    auto points = GenerateRandomPoints(BatchSize);
    std::vector<float> intensities(BatchSize);
    
    for (auto _ : state) {
        calculateAttenuationBatch(
            light.attenuation,
            light.position,
            points.data(),
            BatchSize,
            light.attenuationModel,
            intensities.data()
        );
        benchmark::DoNotOptimize(intensities);
    }
}
BENCHMARK(BM_PointLight_BatchIntensity)
    ->Arg(4)     // Single SIMD batch
    ->Arg(16)    // Small batch
    ->Arg(1024)  // Large batch
    ->Arg(4096); // Very large batch

// Shadow cubemap face transforms
static void BM_PointLight_CubemapTransforms(benchmark::State& state) {
    PointLight light(Vector3(0.0f, 5.0f, 0.0f), 10.0f);
    std::vector<Matrix4> faceTransforms(6);  // One for each cube face
    
    for (auto _ : state) {
        for (int face = 0; face < 6; ++face) {
            faceTransforms[face] = LightSpaceTransform::createLightSpaceTransform(
                light,
                face,   // Face index
                0.1f    // Near plane
            );
        }
        benchmark::DoNotOptimize(faceTransforms);
    }
}
BENCHMARK(BM_PointLight_CubemapTransforms);

// Multiple light interactions
static void BM_PointLight_MultiLightInteraction(benchmark::State& state) {
    const size_t NumLights = state.range(0);
    auto lightPositions = GenerateRandomPoints(NumLights);
    std::vector<PointLight> lights;
    lights.reserve(NumLights);
    
    // Generate random ranges for lights
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> range_dis(MIN_LIGHT_RANGE, MAX_LIGHT_RANGE);
    
    for (size_t i = 0; i < NumLights; ++i) {
        lights.emplace_back(lightPositions[i], range_dis(gen));
    }
    
    // Sample points for light calculations
    auto points = GenerateRandomPoints(1000);
    std::vector<float> totalIntensities(points.size(), 0.0f);
    std::vector<float> tempIntensities(points.size());
    
    for (auto _ : state) {
        // Reset total intensities
        std::fill(totalIntensities.begin(), totalIntensities.end(), 0.0f);
        
        // Accumulate contributions from all lights
        for (const auto& light : lights) {
            calculateAttenuationBatch(
                light.attenuation,
                light.position,
                points.data(),
                points.size(),
                light.attenuationModel,
                tempIntensities.data()
            );
            
            // Add to total intensities
            for (size_t i = 0; i < points.size(); ++i) {
                totalIntensities[i] += tempIntensities[i];
            }
        }
        benchmark::DoNotOptimize(totalIntensities);
    }
}
BENCHMARK(BM_PointLight_MultiLightInteraction)
    ->Arg(1)    // Single light
    ->Arg(4)    // Few lights
    ->Arg(16)   // Many lights
    ->Arg(64);  // Very many lights


using namespace pynovage::math::lighting;

// Performance benchmarks will go here
