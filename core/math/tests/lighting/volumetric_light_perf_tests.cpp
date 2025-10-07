#include "lighting/volumetric_light.hpp"
#include <benchmark/benchmark.h>
#define _USE_MATH_DEFINES
#include <cmath>

using namespace pynovage::math;
using namespace pynovage::math::lighting;

static void BM_PhaseFunction(benchmark::State& state) {
    float cos_angle = 0.5f;
    float asymmetry = 0.8f;
    
    for (auto _ : state) {
        auto result = CalculatePhaseFunction(cos_angle, asymmetry);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_PhaseFunction);

static void BM_AdaptiveStepSize(benchmark::State& state) {
    Vector3 current_pos(0, 0, 0);
    Vector3 light_pos(10, 0, 0);
    VolumetricMedium medium;
    
    for (auto _ : state) {
        auto result = CalculateAdaptiveStepSize(current_pos, light_pos, medium, 1.0f);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AdaptiveStepSize);

static void BM_VolumetricShadow(benchmark::State& state) {
    const int num_steps = state.range(0);
    Vector3 sample_pos(0, 0, 0);
    Vector3 light_pos(10, 0, 0);
    VolumetricMedium medium;
    VolumeSamplingParams sampling;
    sampling.num_steps = num_steps;
    
    for (auto _ : state) {
        auto result = CalculateVolumetricShadow(sample_pos, light_pos, medium, sampling);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VolumetricShadow)->Range(8, 8<<10);

static void BM_VolumetricScattering_SingleLight(benchmark::State& state) {
    const int num_steps = state.range(0);
    Vector3 ray_origin(0, 0, 0);
    Vector3 ray_direction(1, 0, 0);
    Vector3 light_pos(0, 5, 0);
    Vector3 light_color(1, 1, 1);
    VolumetricMedium medium;
    VolumeSamplingParams sampling;
    sampling.num_steps = num_steps;
    
    for (auto _ : state) {
        auto result = CalculateVolumetricScattering(ray_origin, ray_direction,
            light_pos, light_color, medium, sampling);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VolumetricScattering_SingleLight)->Range(8, 8<<10);

static void BM_VolumetricScattering_AdaptiveSampling(benchmark::State& state) {
    const int num_steps = state.range(0);
    Vector3 ray_origin(0, 0, 0);
    Vector3 ray_direction(1, 0, 0);
    Vector3 light_pos(0, 5, 0);
    Vector3 light_color(1, 1, 1);
    VolumetricMedium medium;
    VolumeSamplingParams sampling;
    sampling.num_steps = num_steps;
    sampling.use_adaptive_sampling = true;
    
    for (auto _ : state) {
        auto result = CalculateVolumetricScattering(ray_origin, ray_direction,
            light_pos, light_color, medium, sampling);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VolumetricScattering_AdaptiveSampling)->Range(8, 8<<10);

static void BM_VolumetricScattering_MultiLight(benchmark::State& state) {
    const int num_lights = state.range(0);
    Vector3 ray_origin(0, 0, 0);
    Vector3 ray_direction(1, 0, 0);
    VolumetricMedium medium;
    VolumeSamplingParams sampling;
    sampling.num_steps = 64;  // Fixed step count for multi-light
    
    // Create light arrays
    Vector3* lights = new Vector3[num_lights];
    Vector3* colors = new Vector3[num_lights];
    
    for (int i = 0; i < num_lights; ++i) {
        float angle = (6.28318530718f * i) / num_lights;  // 2π
        lights[i] = Vector3(5.0f * std::cos(angle), 5.0f, 5.0f * std::sin(angle));
        colors[i] = Vector3(1, 1, 1);
    }
    
    for (auto _ : state) {
        auto result = CalculateMultiLightScattering(ray_origin, ray_direction,
            lights, colors, num_lights, medium, sampling);
        benchmark::DoNotOptimize(result);
    }
    
    delete[] lights;
    delete[] colors;
}
BENCHMARK(BM_VolumetricScattering_MultiLight)->Range(1, 64);

static void BM_VolumetricScattering_DenseMedium(benchmark::State& state) {
    const int num_steps = state.range(0);
    Vector3 ray_origin(0, 0, 0);
    Vector3 ray_direction(1, 0, 0);
    Vector3 light_pos(0, 5, 0);
    Vector3 light_color(1, 1, 1);
    
    VolumetricMedium medium;
    medium.density = 10.0f;
    medium.scattering_coefficient = 0.5f;
    medium.absorption_coefficient = 0.1f;
    
    VolumeSamplingParams sampling;
    sampling.num_steps = num_steps;
    
    for (auto _ : state) {
        auto result = CalculateVolumetricScattering(ray_origin, ray_direction,
            light_pos, light_color, medium, sampling);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VolumetricScattering_DenseMedium)->Range(8, 8<<10);

static void BM_VolumetricScattering_AnisotropicMedium(benchmark::State& state) {
    const int num_steps = state.range(0);
    Vector3 ray_origin(0, 0, 0);
    Vector3 ray_direction(1, 0, 0);
    Vector3 light_pos(0, 5, 0);
    Vector3 light_color(1, 1, 1);
    
    VolumetricMedium medium;
    medium.asymmetry_factor = 0.8f;  // Strong forward scattering
    
    VolumeSamplingParams sampling;
    sampling.num_steps = num_steps;
    
    for (auto _ : state) {
        auto result = CalculateVolumetricScattering(ray_origin, ray_direction,
            light_pos, light_color, medium, sampling);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VolumetricScattering_AnisotropicMedium)->Range(8, 8<<10);