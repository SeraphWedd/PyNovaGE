#include "math/geometry/intersection.hpp"
#include <benchmark/benchmark.h>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

static void BM_RayPlaneIntersection(benchmark::State& state) {
    Ray3D ray(Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f).normalized());
    Plane plane(Vector3(0.0f, 1.0f, 0.0f), 0.0f);
    
    for (auto _ : state) {
        auto result = rayPlaneIntersection(ray, plane);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RayPlaneIntersection);

static void BM_RaySphereIntersection(benchmark::State& state) {
    Ray3D ray(Vector3(0.0f, 0.0f, -2.0f), Vector3(0.0f, 0.0f, 1.0f));
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    
    for (auto _ : state) {
        auto result = raySphereIntersection(ray, sphere);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RaySphereIntersection);

static void BM_RayAABBIntersection(benchmark::State& state) {
    Ray3D ray(Vector3(0.0f, 0.0f, -2.0f), Vector3(0.0f, 0.0f, 1.0f));
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    
    for (auto _ : state) {
        auto result = rayAABBIntersection(ray, aabb);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RayAABBIntersection);

static void BM_SphereSphereIntersection(benchmark::State& state) {
    Sphere sphere1(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    Sphere sphere2(Vector3(1.5f, 0.0f, 0.0f), 1.0f);
    
    for (auto _ : state) {
        auto result = sphereSphereIntersection(sphere1, sphere2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SphereSphereIntersection);

static void BM_AABBAABBIntersection(benchmark::State& state) {
    AABB aabb1(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB aabb2(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f));
    
    for (auto _ : state) {
        auto result = aabbAABBIntersection(aabb1, aabb2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AABBAABBIntersection);

// Benchmark collision tests with varying distances
static void BM_RaySphereIntersection_VaryingDistance(benchmark::State& state) {
    const int distanceRange = state.range(0);
    Ray3D ray(Vector3(0.0f, 0.0f, -static_cast<float>(distanceRange)), Vector3(0.0f, 0.0f, 1.0f));
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    
    for (auto _ : state) {
        auto result = raySphereIntersection(ray, sphere);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RaySphereIntersection_VaryingDistance)
    ->Args({2})    // Near intersection
    ->Args({10})   // Medium distance
    ->Args({100}); // Far distance

// Benchmark collision tests with near misses
static void BM_RaySphereIntersection_NearMiss(benchmark::State& state) {
    const float offset = static_cast<float>(state.range(0)) * 0.1f;
    Ray3D ray(Vector3(1.0f + offset, 0.0f, -2.0f), Vector3(0.0f, 0.0f, 1.0f));
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    
    for (auto _ : state) {
        auto result = raySphereIntersection(ray, sphere);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RaySphereIntersection_NearMiss)
    ->Args({0})    // Exact hit
    ->Args({1})    // Just miss
    ->Args({10});  // Clear miss