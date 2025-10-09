#include <benchmark/benchmark.h>
#include "../../include/simd/math_ops.hpp"
#include "../../include/simd/vector_ops.hpp"
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

// Benchmark Vector4f Addition
static void BM_Vector4f_Addition(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        Vector4f result = vectors[index % count] + vectors[(index + 1) % count];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_Addition);

// Benchmark Vector4f Dot Product
static void BM_Vector4f_DotProduct(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        float result = dot(vectors[index % count], vectors[(index + 1) % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_DotProduct);

// Benchmark Vector4f Normalization
static void BM_Vector4f_Normalize(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        Vector4f result = normalize(vectors[index % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_Normalize);

// Benchmark Vector4f Square Root
static void BM_Vector4f_Sqrt(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    // Ensure all components are positive
    for (auto& v : vectors) {
        for (int i = 0; i < 4; ++i) {
            v[i] = std::abs(v[i]);
        }
    }
    size_t index = 0;

    for (auto _ : state) {
        Vector4f result = sqrt(vectors[index % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_Sqrt);

// Benchmark Vector4f Reciprocal Square Root
static void BM_Vector4f_RSqrt(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    // Ensure all components are positive
    for (auto& v : vectors) {
        for (int i = 0; i < 4; ++i) {
            v[i] = std::abs(v[i]) + 0.1f; // Avoid zero
        }
    }
    size_t index = 0;

    for (auto _ : state) {
        Vector4f result = rsqrt(vectors[index % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_RSqrt);

// Benchmark Vector4f Minimum
static void BM_Vector4f_Min(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        Vector4f result = min(vectors[index % count], vectors[(index + 1) % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_Min);

// Benchmark Vector4f Maximum
static void BM_Vector4f_Max(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        Vector4f result = max(vectors[index % count], vectors[(index + 1) % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_Max);

// Benchmark Vector4f Absolute Value
static void BM_Vector4f_Abs(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        Vector4f result = abs(vectors[index % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_Abs);

// Benchmark Vector4f Length
static void BM_Vector4f_Length(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        float result = length(vectors[index % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_Length);

// Benchmark Vector4f Length Squared
static void BM_Vector4f_LengthSquared(benchmark::State& state) {
    const size_t count = 1000;
    auto vectors = GenerateRandomVectors(count);
    size_t index = 0;

    for (auto _ : state) {
        float result = length_squared(vectors[index % count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4f_LengthSquared);

// Add parameter ranges to adjust benchmark complexity
BENCHMARK(BM_Vector4f_Addition)->Range(8, 8<<10);
BENCHMARK(BM_Vector4f_DotProduct)->Range(8, 8<<10);
BENCHMARK(BM_Vector4f_Normalize)->Range(8, 8<<10);
BENCHMARK(BM_Vector4f_Sqrt)->Range(8, 8<<10);
BENCHMARK(BM_Vector4f_RSqrt)->Range(8, 8<<10);
BENCHMARK(BM_Vector4f_Min)->Range(8, 8<<10);
BENCHMARK(BM_Vector4f_Max)->Range(8, 8<<10);
BENCHMARK(BM_Vector4f_Abs)->Range(8, 8<<10);
BENCHMARK(BM_Vector4f_Length)->Range(8, 8<<10);
BENCHMARK(BM_Vector4f_LengthSquared)->Range(8, 8<<10);

} // namespace