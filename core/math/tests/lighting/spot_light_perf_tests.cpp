#include <benchmark/benchmark.h>
#include "../../include/lighting/spot_light.hpp"
#include "../../include/lighting/attenuation.hpp"
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

// Basic position and direction updates
static void BM_SpotLight_SetPosition(benchmark::State& state) {
    SpotLight light;
    auto positions = GenerateRandomPoints(1000);
    
    size_t index = 0;
    for (auto _ : state) {
        light.setPosition(positions[index % positions.size()]);
        benchmark::DoNotOptimize(light);
        index++;
    }
}
BENCHMARK(BM_SpotLight_SetPosition);

static void BM_SpotLight_SetDirection(benchmark::State& state) {
    SpotLight light;
    auto directions = GenerateRandomVectors(1000);
    
    size_t index = 0;
    for (auto _ : state) {
        light.setDirection(directions[index % directions.size()]);
        benchmark::DoNotOptimize(light);
        index++;
    }
}
BENCHMARK(BM_SpotLight_SetDirection);

// Cone angle calculations
static void BM_SpotLight_SetAngles(benchmark::State& state) {
    SpotLight light;
    std::vector<std::pair<float, float>> angles;
    
    // Generate random angles (outer between 0-90°, inner slightly smaller)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, pynovage::math::constants::half_pi);
    
    for (size_t i = 0; i < 1000; ++i) {
        float outer = dis(gen);
        float inner = outer * 0.9f;  // Inner is 90% of outer
        angles.emplace_back(outer, inner);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        const auto& angle_pair = angles[index % angles.size()];
        light.setAngles(angle_pair.first, angle_pair.second);
        benchmark::DoNotOptimize(light);
        index++;
    }
}
BENCHMARK(BM_SpotLight_SetAngles);

// Light-to-point intensity calculations
static void BM_SpotLight_IntensityAtPoint(benchmark::State& state) {
    SpotLight light(
        Vector3(0.0f, 5.0f, 0.0f),     // position
        Vector3(0.0f, -1.0f, 0.0f),    // direction
        pynovage::math::constants::quarter_pi // outer angle (45°)
    );
    light.setAngles(pynovage::math::constants::quarter_pi,
                    pynovage::math::constants::quarter_pi * 0.9f);
    auto points = GenerateRandomPoints(1000);
    
    size_t index = 0;
    for (auto _ : state) {
        const Vector3& point = points[index % points.size()];
        
        // Calculate direction to point
        Vector3 to_point = (point - light.position).normalized();
        float cos_angle = light.direction.dot(to_point);
        
        // Check if point is within cone
        float intensity = 0.0f;
        if (cos_angle > std::cos(light.outerAngle)) {
            // Calculate angular attenuation
            float angular_attenuation = 1.0f;
            if (cos_angle < std::cos(light.innerAngle)) {
                float delta = (cos_angle - std::cos(light.outerAngle)) /
                             (std::cos(light.innerAngle) - std::cos(light.outerAngle));
                angular_attenuation = std::max(0.0f, delta);
            }
            
            // Calculate distance attenuation
            float distance_attenuation = calculateAttenuation(
                light.attenuation,
                light.position,
                point,
                light.attenuationModel
            );
            
            intensity = angular_attenuation * distance_attenuation;
        }
        
        benchmark::DoNotOptimize(intensity);
        index++;
    }
}
BENCHMARK(BM_SpotLight_IntensityAtPoint);

// Batch intensity calculations
static void BM_SpotLight_BatchIntensity(benchmark::State& state) {
    const size_t BatchSize = state.range(0);
    SpotLight light(
        Vector3(0.0f, 5.0f, 0.0f),     // position
        Vector3(0.0f, -1.0f, 0.0f),    // direction
        pynovage::math::constants::quarter_pi // outer angle (45°)
    );
    light.setAngles(pynovage::math::constants::quarter_pi,
                    pynovage::math::constants::quarter_pi * 0.9f);
    auto points = GenerateRandomPoints(BatchSize);
    std::vector<float> intensities(BatchSize);
    
    for (auto _ : state) {
        for (size_t i = 0; i < BatchSize; ++i) {
            const Vector3& point = points[i];
            
            // Calculate direction to point
            Vector3 to_point = (point - light.position).normalized();
            float cos_angle = light.direction.dot(to_point);
            
            // Check if point is within cone
            float intensity = 0.0f;
            if (cos_angle > std::cos(light.outerAngle)) {
                // Calculate angular attenuation
                float angular_attenuation = 1.0f;
                if (cos_angle < std::cos(light.innerAngle)) {
                    float delta = (cos_angle - std::cos(light.outerAngle)) /
                                 (std::cos(light.innerAngle) - std::cos(light.outerAngle));
                    angular_attenuation = std::max(0.0f, delta);
                }
                
                // Calculate distance attenuation
                float distance_attenuation = calculateAttenuation(
                    light.attenuation,
                    light.position,
                    point,
                    light.attenuationModel
                );
                
                intensity = angular_attenuation * distance_attenuation;
            }
            
            intensities[i] = intensity;
        }
        benchmark::DoNotOptimize(intensities);
    }
}
BENCHMARK(BM_SpotLight_BatchIntensity)
    ->Arg(4)     // Single SIMD batch
    ->Arg(16)    // Small batch
    ->Arg(1024)  // Large batch
    ->Arg(4096); // Very large batch

// Multiple light interactions
static void BM_SpotLight_MultiLightInteraction(benchmark::State& state) {
    const size_t NumLights = state.range(0);
    auto lightPositions = GenerateRandomPoints(NumLights);
    auto lightDirections = GenerateRandomVectors(NumLights);
    std::vector<SpotLight> lights;
    lights.reserve(NumLights);
    
    // Generate random ranges and angles
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> range_dis(pynovage::math::lighting::constants::MIN_LIGHT_RANGE,
                                                          pynovage::math::lighting::constants::MAX_LIGHT_RANGE);
    std::uniform_real_distribution<float> angle_dis(0.0f, pynovage::math::constants::half_pi);
    
    for (size_t i = 0; i < NumLights; ++i) {
        float outer_angle = angle_dis(gen);
        float inner_angle = outer_angle * 0.9f;
        float range = range_dis(gen);
        lights.emplace_back(
            lightPositions[i],
            lightDirections[i],
            outer_angle,
            inner_angle,
            range,
            LightColor(1.0f, 1.0f, 1.0f, 1.0f)
        );
    }
    
    // Sample points for light calculations
    auto points = GenerateRandomPoints(1000);
    std::vector<float> totalIntensities(points.size(), 0.0f);
    
    for (auto _ : state) {
        // Reset total intensities
        std::fill(totalIntensities.begin(), totalIntensities.end(), 0.0f);
        
        // Accumulate contributions from all lights
        for (const auto& light : lights) {
            for (size_t i = 0; i < points.size(); ++i) {
                const Vector3& point = points[i];
                
                // Calculate direction to point
                Vector3 to_point = (point - light.position).normalized();
                float cos_angle = light.direction.dot(to_point);
                
                // Check if point is within cone
                if (cos_angle > std::cos(light.outerAngle)) {
                    // Calculate angular attenuation
                    float angular_attenuation = 1.0f;
                    if (cos_angle < std::cos(light.innerAngle)) {
                        float delta = (cos_angle - std::cos(light.outerAngle)) /
                                     (std::cos(light.innerAngle) - std::cos(light.outerAngle));
                        angular_attenuation = std::max(0.0f, delta);
                    }
                    
                    // Calculate distance attenuation
                    float distance_attenuation = calculateAttenuation(
                        light.attenuation,
                        light.position,
                        point,
                        light.attenuationModel
                    );
                    
                    totalIntensities[i] += angular_attenuation * distance_attenuation;
                }
            }
        }
        benchmark::DoNotOptimize(totalIntensities);
    }
}
BENCHMARK(BM_SpotLight_MultiLightInteraction)
    ->Arg(1)    // Single light
    ->Arg(4)    // Few lights
    ->Arg(16)   // Many lights
    ->Arg(64);  // Very many lights

// Performance benchmarks will go here
