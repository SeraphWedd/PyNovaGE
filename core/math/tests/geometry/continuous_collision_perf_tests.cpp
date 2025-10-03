#include "math/geometry/continuous_collision.hpp"
#include <benchmark/benchmark.h>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

static void BM_MovingSphereSphere_DirectHit(benchmark::State& state) {
    Sphere movingSphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    Sphere staticSphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    Vector3 start(0.0f, 0.0f, -2.0f);
    Vector3 end(0.0f, 0.0f, 2.0f);
    float timeStep = 1.0f;
    
    for (auto _ : state) {
        auto result = testMovingSphereSphere(movingSphere, staticSphere, start, end, timeStep);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_MovingSphereSphere_DirectHit);

static void BM_MovingSphereSphere_NoCollision(benchmark::State& state) {
    Sphere movingSphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    Sphere staticSphere(Vector3(0.0f, 2.0f, 0.0f), 0.5f);
    Vector3 start(2.0f, 0.0f, -2.0f);
    Vector3 end(2.0f, 0.0f, 2.0f);
    float timeStep = 1.0f;
    
    for (auto _ : state) {
        auto result = testMovingSphereSphere(movingSphere, staticSphere, start, end, timeStep);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_MovingSphereSphere_NoCollision);

static void BM_MovingSphereAABB_DirectHit(benchmark::State& state) {
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    Vector3 start(0.0f, 0.0f, -2.0f);
    Vector3 end(0.0f, 0.0f, 2.0f);
    float timeStep = 1.0f;
    
    for (auto _ : state) {
        auto result = testMovingSphereAABB(sphere, aabb, start, end, timeStep);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_MovingSphereAABB_DirectHit);

static void BM_MovingSphereAABB_NoCollision(benchmark::State& state) {
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    Vector3 start(2.0f, 0.0f, -2.0f);
    Vector3 end(2.0f, 0.0f, 2.0f);
    float timeStep = 1.0f;
    
    for (auto _ : state) {
        auto result = testMovingSphereAABB(sphere, aabb, start, end, timeStep);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_MovingSphereAABB_NoCollision);

// Benchmarks with varying movement speeds
static void BM_MovingSphereSphere_VaryingSpeed(benchmark::State& state) {
    Sphere movingSphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    Sphere staticSphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    Vector3 start(0.0f, 0.0f, -2.0f);
    float speed = static_cast<float>(state.range(0));
    Vector3 end(0.0f, 0.0f, -2.0f + speed);
    float timeStep = 1.0f;
    
    for (auto _ : state) {
        auto result = testMovingSphereSphere(movingSphere, staticSphere, start, end, timeStep);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_MovingSphereSphere_VaryingSpeed)
    ->Args({1})     // Slow speed
    ->Args({10})    // Medium speed
    ->Args({100});  // High speed