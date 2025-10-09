#include <benchmark/benchmark.h>
#include "../../include/simd/matrix_ops.hpp"
#include "../../include/simd/geometry_ops.hpp"
#include <random>
#include <vector>

using namespace PyNovaGE::SIMD;

namespace {

// Helper function to generate random float vectors
std::vector<Vector4f> GenerateRandomVectors(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-100.0f, 100.0f);

    std::vector<Vector4f> vectors;
    vectors.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        vectors.emplace_back(dist(gen), dist(gen), dist(gen), dist(gen));
    }
    return vectors;
}

// Helper function to generate random float matrices
std::vector<Matrix4f> GenerateRandomMatrices(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    std::vector<Matrix4f> matrices;
    matrices.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        float data[16];
        for (int j = 0; j < 16; ++j) {
            data[j] = dist(gen);
        }
        matrices.emplace_back(data);
    }
    return matrices;
}

// Helper function to generate random AABBs
std::vector<AABB<float>> GenerateRandomAABBs(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> size_dist(1.0f, 10.0f);

    std::vector<AABB<float>> aabbs;
    aabbs.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        Vector3f center(dist(gen), dist(gen), dist(gen));
        Vector3f extent(size_dist(gen), size_dist(gen), size_dist(gen));
        aabbs.emplace_back(center - extent, center + extent);
    }
    return aabbs;
}

// Helper function to generate random spheres
std::vector<Sphere<float>> GenerateRandomSpheres(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> radius_dist(1.0f, 10.0f);

    std::vector<Sphere<float>> spheres;
    spheres.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        Vector3f center(dist(gen), dist(gen), dist(gen));
        spheres.emplace_back(center, radius_dist(gen));
    }
    return spheres;
}

// Benchmark Matrix4f Multiplication
static void BM_Matrix4f_Multiplication(benchmark::State& state) {
    const size_t count = 1000;
    auto matrices = GenerateRandomMatrices(count);
    size_t index = 0;

    for (auto _ : state) {
        Matrix4f result = matrices[index % count] * matrices[(index + 1) % count];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4f_Multiplication);

// Benchmark Matrix4f-Vector4f Multiplication
static void BM_Matrix4f_Vector_Multiplication(benchmark::State& state) {
    const size_t count = 1000;
    auto matrices = GenerateRandomMatrices(count);
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        Vector4f result = matrices[index % count] * vectors[index % count];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4f_Vector_Multiplication);

// Benchmark Matrix4f Transpose
static void BM_Matrix4f_Transpose(benchmark::State& state) {
    const size_t count = 1000;
    auto matrices = GenerateRandomMatrices(count);
    size_t index = 0;

    for (auto _ : state) {
        Matrix4f result = transpose(matrices[index % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4f_Transpose);

// Benchmark Matrix4f Transform Operations
static void BM_Matrix4f_Transform(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        Vector3f translation(1.0f, 2.0f, 3.0f);
        Vector3f scale_factor(2.0f, 2.0f, 2.0f);
        Vector3f rotation_axis = normalize(Vector3f(1.0f, 1.0f, 1.0f));
        float angle = 0.5f;

        Matrix4f t = translate(translation);
        Matrix4f s = scale(scale_factor);
        Matrix4f r = rotate(rotation_axis, angle);
        
        Matrix4f transform = t * r * s;
        Vector4f result = transform * vectors[index % count];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4f_Transform);

// Benchmark AABB-AABB Intersection
static void BM_AABB_Intersection(benchmark::State& state) {
    const size_t count = 1000;
    auto aabbs = GenerateRandomAABBs(count);
    size_t index = 0;

    for (auto _ : state) {
        bool result = aabbs[index % count].intersects(aabbs[(index + 1) % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_AABB_Intersection);

// Benchmark Sphere-Sphere Intersection
static void BM_Sphere_Intersection(benchmark::State& state) {
    const size_t count = 1000;
    auto spheres = GenerateRandomSpheres(count);
    size_t index = 0;

    for (auto _ : state) {
        bool result = spheres[index % count].intersects(spheres[(index + 1) % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Sphere_Intersection);

// Benchmark Ray-Sphere Intersection
static void BM_Ray_Sphere_Intersection(benchmark::State& state) {
    const size_t count = 1000;
    auto spheres = GenerateRandomSpheres(count);
    std::vector<Ray<float>> rays;
    rays.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (size_t i = 0; i < count; ++i) {
        Vector3f origin(dist(gen) * 100.0f, dist(gen) * 100.0f, dist(gen) * 100.0f);
        Vector3f direction(dist(gen), dist(gen), dist(gen));
        rays.emplace_back(origin, direction);
    }

    size_t index = 0;
    float t;

    for (auto _ : state) {
        bool result = rays[index % count].intersects(spheres[index % count], t);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(t);
        index++;
    }
}
BENCHMARK(BM_Ray_Sphere_Intersection);

// Benchmark Ray-AABB Intersection
static void BM_Ray_AABB_Intersection(benchmark::State& state) {
    const size_t count = 1000;
    auto aabbs = GenerateRandomAABBs(count);
    std::vector<Ray<float>> rays;
    rays.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (size_t i = 0; i < count; ++i) {
        Vector3f origin(dist(gen) * 100.0f, dist(gen) * 100.0f, dist(gen) * 100.0f);
        Vector3f direction(dist(gen), dist(gen), dist(gen));
        rays.emplace_back(origin, direction);
    }

    size_t index = 0;
    float t;

    for (auto _ : state) {
        bool result = rays[index % count].intersects(aabbs[index % count], t);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(t);
        index++;
    }
}
BENCHMARK(BM_Ray_AABB_Intersection);

// Benchmark Plane Operations
static void BM_Plane_Operations(benchmark::State& state) {
    const size_t count = 1000;
    std::vector<Plane<float>> planes;
    auto vectors = GenerateRandomVectors(count);
    planes.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (size_t i = 0; i < count; ++i) {
        Vector3f normal(dist(gen), dist(gen), dist(gen));
        planes.emplace_back(normalize(normal), dist(gen) * 10.0f);
    }

    size_t index = 0;

    for (auto _ : state) {
        float distance = planes[index % count].signedDistance(
            Vector3f(vectors[index % count][0],
                    vectors[index % count][1],
                    vectors[index % count][2]));
        int classification = planes[index % count].classifyPoint(
            Vector3f(vectors[index % count][0],
                    vectors[index % count][1],
                    vectors[index % count][2]));
        benchmark::DoNotOptimize(distance);
        benchmark::DoNotOptimize(classification);
        index++;
    }
}
BENCHMARK(BM_Plane_Operations);

// Add parameter ranges to adjust benchmark complexity
BENCHMARK(BM_Matrix4f_Multiplication)->Range(8, 8<<10);
BENCHMARK(BM_Matrix4f_Vector_Multiplication)->Range(8, 8<<10);
BENCHMARK(BM_Matrix4f_Transpose)->Range(8, 8<<10);
BENCHMARK(BM_Matrix4f_Transform)->Range(8, 8<<10);
BENCHMARK(BM_AABB_Intersection)->Range(8, 8<<10);
BENCHMARK(BM_Sphere_Intersection)->Range(8, 8<<10);
BENCHMARK(BM_Ray_Sphere_Intersection)->Range(8, 8<<10);
BENCHMARK(BM_Ray_AABB_Intersection)->Range(8, 8<<10);
BENCHMARK(BM_Plane_Operations)->Range(8, 8<<10);

} // namespace