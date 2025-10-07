#include "lighting/area_light.hpp"
#include <benchmark/benchmark.h>

using namespace pynovage::math;
using namespace pynovage::math::lighting;

static void BM_RectFormFactor(benchmark::State& state) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    
    RectAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.up = Vector3(0, 0, 1);
    light.width = 2.0f;
    light.height = 2.0f;
    
    for (auto _ : state) {
        auto result = CalculateRectFormFactor(surface_point, surface_normal, light);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RectFormFactor);

static void BM_DiskFormFactor(benchmark::State& state) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    
    DiskAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.radius = 1.0f;
    
    for (auto _ : state) {
        auto result = CalculateDiskFormFactor(surface_point, surface_normal, light);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_DiskFormFactor);

static void BM_RectLightSampling(benchmark::State& state) {
    const int num_samples = state.range(0);
    RectAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.up = Vector3(0, 0, 1);
    light.width = 2.0f;
    light.height = 2.0f;
    
    AreaSamplingParams sampling;
    sampling.num_samples = num_samples;
    sampling.stratified_sampling = true;
    
    Vector3* samples = new Vector3[num_samples];
    
    for (auto _ : state) {
        GenerateRectLightSamples(light, sampling, samples, num_samples);
        benchmark::DoNotOptimize(samples[0]);
    }
    
    delete[] samples;
}
BENCHMARK(BM_RectLightSampling)->Range(8, 8<<10);

static void BM_DiskLightSampling(benchmark::State& state) {
    const int num_samples = state.range(0);
    DiskAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.radius = 1.0f;
    
    AreaSamplingParams sampling;
    sampling.num_samples = num_samples;
    sampling.stratified_sampling = true;
    
    Vector3* samples = new Vector3[num_samples];
    
    for (auto _ : state) {
        GenerateDiskLightSamples(light, sampling, samples, num_samples);
        benchmark::DoNotOptimize(samples[0]);
    }
    
    delete[] samples;
}
BENCHMARK(BM_DiskLightSampling)->Range(8, 8<<10);

static void BM_RectAreaLighting_SinglePoint(benchmark::State& state) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    Vector3 view_direction(0, 1, 0);
    float material_roughness = 0.5f;
    
    RectAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.up = Vector3(0, 0, 1);
    light.width = 2.0f;
    light.height = 2.0f;
    light.color = Vector3(1, 1, 1);
    light.intensity = 1.0f;
    
    AreaSamplingParams sampling;
    sampling.num_samples = state.range(0);
    
    for (auto _ : state) {
        auto result = CalculateRectAreaLight(surface_point, surface_normal,
            view_direction, material_roughness, light, sampling);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RectAreaLighting_SinglePoint)->Range(8, 8<<10);

static void BM_DiskAreaLighting_SinglePoint(benchmark::State& state) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    Vector3 view_direction(0, 1, 0);
    float material_roughness = 0.5f;
    
    DiskAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.radius = 1.0f;
    light.color = Vector3(1, 1, 1);
    light.intensity = 1.0f;
    
    AreaSamplingParams sampling;
    sampling.num_samples = state.range(0);
    
    for (auto _ : state) {
        auto result = CalculateDiskAreaLight(surface_point, surface_normal,
            view_direction, material_roughness, light, sampling);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_DiskAreaLighting_SinglePoint)->Range(8, 8<<10);

static void BM_CustomAreaLighting(benchmark::State& state) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    Vector3 view_direction(0, 1, 0);
    float material_roughness = 0.5f;
    
    // Create a simple triangular light
    Vector3 vertices[] = {
        Vector3(-1, 5, -1),
        Vector3(1, 5, -1),
        Vector3(0, 5, 1)
    };
    Vector3 normals[] = {
        Vector3(0, -1, 0),
        Vector3(0, -1, 0),
        Vector3(0, -1, 0)
    };
    
    CustomAreaLight light;
    light.vertices = vertices;
    light.normals = normals;
    light.vertex_count = 3;
    light.position = Vector3(0, 5, 0);
    light.color = Vector3(1, 1, 1);
    light.intensity = 1.0f;
    
    AreaSamplingParams sampling;
    sampling.num_samples = state.range(0);
    
    for (auto _ : state) {
        auto result = CalculateCustomAreaLight(surface_point, surface_normal,
            view_direction, material_roughness, light, sampling);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_CustomAreaLighting)->Range(8, 8<<10);

static void BM_AreaLightVisibility(benchmark::State& state) {
    Vector3 surface_point(0, 0, 0);
    Vector3 sample_point(0, 5, 0);
    Vector3 light_normal(0, -1, 0);
    
    for (auto _ : state) {
        auto result = CalculateAreaLightVisibility(surface_point, sample_point, light_normal);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AreaLightVisibility);

static void BM_RectAreaLighting_VaryingRoughness(benchmark::State& state) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    Vector3 view_direction(0, 1, 0);
    float material_roughness = state.range(0) / 1000.0f;  // 0-1 range
    
    RectAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.up = Vector3(0, 0, 1);
    light.width = 2.0f;
    light.height = 2.0f;
    light.color = Vector3(1, 1, 1);
    light.intensity = 1.0f;
    
    AreaSamplingParams sampling;
    sampling.num_samples = 64;  // Fixed sample count
    
    for (auto _ : state) {
        auto result = CalculateRectAreaLight(surface_point, surface_normal,
            view_direction, material_roughness, light, sampling);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RectAreaLighting_VaryingRoughness)->DenseRange(0, 1000, 100);