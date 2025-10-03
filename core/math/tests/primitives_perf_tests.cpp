#include <benchmark/benchmark.h>
#include "primitives.hpp"

using namespace pynovage::math;

// Ray Performance Tests
static void BM_RayPointCalculation(benchmark::State& state) {
    Ray ray(Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
    float t = 2.0f;
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(ray.getPoint(t));
    }
}
BENCHMARK(BM_RayPointCalculation);

// AABB Performance Tests
static void BM_AABBPropertyCalculation(benchmark::State& state) {
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(aabb.getCenter());
        benchmark::DoNotOptimize(aabb.getExtents());
        benchmark::DoNotOptimize(aabb.getSize());
    }
}
BENCHMARK(BM_AABBPropertyCalculation);

static void BM_AABBExpansion(benchmark::State& state) {
    for (auto _ : state) {
        AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
        aabb.expand(1.0f);
        benchmark::DoNotOptimize(aabb);
    }
}
BENCHMARK(BM_AABBExpansion);

// Plane Performance Tests
static void BM_PlaneConstruction(benchmark::State& state) {
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 point(2.0f, 0.0f, 0.0f);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(Plane(normal, point));
    }
}
BENCHMARK(BM_PlaneConstruction);

static void BM_PlaneDistanceCalculation(benchmark::State& state) {
    Plane plane(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Vector3 point(2.0f, 0.0f, 0.0f);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(plane.getSignedDistance(point));
    }
}
BENCHMARK(BM_PlaneDistanceCalculation);

// Triangle Performance Tests
static void BM_TriangleProperties(benchmark::State& state) {
    Triangle tri(Vector3(0.0f, 0.0f, 0.0f), 
                Vector3(1.0f, 0.0f, 0.0f), 
                Vector3(0.0f, 1.0f, 0.0f));
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(tri.getNormal());
        benchmark::DoNotOptimize(tri.getArea());
        benchmark::DoNotOptimize(tri.getCenter());
    }
}
BENCHMARK(BM_TriangleProperties);

// OBB Performance Tests
static void BM_OBBConstruction(benchmark::State& state) {
    Vector3 center(1.0f, 1.0f, 1.0f);
    Vector3 extents(2.0f, 2.0f, 2.0f);
    
    for (auto _ : state) {
        OBB obb;
        obb.center = center;
        obb.halfExtents = extents;
        benchmark::DoNotOptimize(obb);
    }
}
BENCHMARK(BM_OBBConstruction);

// Capsule Performance Tests
static void BM_CapsuleProperties(benchmark::State& state) {
    Capsule capsule(Vector3(0.0f, 0.0f, 0.0f), 
                   Vector3(0.0f, 2.0f, 0.0f), 
                   1.0f);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(capsule.getHeight());
        benchmark::DoNotOptimize(capsule.getDirection());
    }
}
BENCHMARK(BM_CapsuleProperties);