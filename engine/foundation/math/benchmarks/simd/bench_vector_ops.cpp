#include <benchmark/benchmark.h>
#include "simd/vector_ops.hpp"
#include <random>
#include <vector>

using namespace pynovage::foundation::simd;

// Helper to create random float data
std::vector<float> generate_random_data(size_t count) {
    std::vector<float> data(count);
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::uniform_real_distribution<float> dist(-1000.0f, 1000.0f);
    for (auto& f : data) {
        f = dist(gen);
    }
    return data;
}

// Vec4 Operations Benchmarks
static void BM_Vec4_Add(benchmark::State& state) {
    auto data1 = generate_random_data(4);
    auto data2 = generate_random_data(4);
    float4 v1 = float4::load(data1.data());
    float4 v2 = float4::load(data2.data());
    float4 result;
    
    for (auto _ : state) {
        Vec4Ops::add(v1, v2, result);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vec4_Add);

// Batch Operations Benchmarks
static void BM_Vec4_BatchAdd(benchmark::State& state) {
    auto data1 = generate_random_data(16);
    auto data2 = generate_random_data(16);
    float16 v1 = float16::load(data1.data());
    float16 v2 = float16::load(data2.data());
    float16 result;
    
    for (auto _ : state) {
        Vec4Ops::add_batch4(v1, v2, result);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vec4_BatchAdd);

// Compare scalar vs SIMD operations
static void BM_Vec4_Add_Scalar(benchmark::State& state) {
    auto data1 = generate_random_data(4);
    auto data2 = generate_random_data(4);
    float result[4];
    
    for (auto _ : state) {
        for (int i = 0; i < 4; ++i) {
            result[i] = data1[i] + data2[i];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vec4_Add_Scalar);

// Vec3 Cross Product Benchmark
static void BM_Vec3_Cross(benchmark::State& state) {
    auto data1 = generate_random_data(4);
    auto data2 = generate_random_data(4);
    float4 v1 = float4::load(data1.data());
    float4 v2 = float4::load(data2.data());
    float4 result;
    
    for (auto _ : state) {
        Vec3Ops::cross(v1, v2, result);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vec3_Cross);

// Batch Cross Product Benchmark
static void BM_Vec3_BatchCross(benchmark::State& state) {
    auto data1 = generate_random_data(16);
    auto data2 = generate_random_data(16);
    float16 v1 = float16::load(data1.data());
    float16 v2 = float16::load(data2.data());
    float16 result;
    
    for (auto _ : state) {
        Vec3Ops::cross_batch4(v1, v2, result);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vec3_BatchCross);

// FMA Operations Benchmark
static void BM_Vec4_MultiplyAdd(benchmark::State& state) {
    auto data1 = generate_random_data(4);
    auto data2 = generate_random_data(4);
    auto data3 = generate_random_data(4);
    float4 v1 = float4::load(data1.data());
    float4 v2 = float4::load(data2.data());
    float4 v3 = float4::load(data3.data());
    float4 result;
    
    for (auto _ : state) {
        Vec4Ops::multiply_add(v1, v2, v3, result);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vec4_MultiplyAdd);

// Compare different SIMD instruction sets
#if PYNOVAGE_SIMD_HAS_AVX512F
static void BM_Vec4_BatchAdd_AVX512(benchmark::State& state) {
    auto data1 = generate_random_data(16);
    auto data2 = generate_random_data(16);
    float16 v1 = float16::load(data1.data());
    float16 v2 = float16::load(data2.data());
    float16 result;
    
    for (auto _ : state) {
        Vec4Ops::add_batch4(v1, v2, result);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vec4_BatchAdd_AVX512);
#endif

BENCHMARK_MAIN();