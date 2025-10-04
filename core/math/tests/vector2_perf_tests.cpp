#include <benchmark/benchmark.h>
#include "vector2.hpp"
#include <vector>
#include <chrono>
#include <random>
#include <array>
#include <numeric>

using namespace pynovage::math;

// Utility function to generate random floats
static std::vector<float> GenerateRandomFloats(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1000.0f, 1000.0f);
    
    std::vector<float> result(count);
    for (auto& val : result) {
        val = dis(gen);
    }
    return result;
}

// Benchmark basic vector operations
static void BM_Vector2Addition(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 2);
    std::vector<Vector2> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 2; i += 2) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector2 result = vectors[index % VectorCount] + vectors[(index + 1) % VectorCount];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector2Addition);

// Benchmark vector normalization (more complex operation)
static void BM_Vector2Normalization(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 2);
    std::vector<Vector2> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 2; i += 2) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector2 result = vectors[index % VectorCount].normalized();
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector2Normalization);

// Benchmark memory operations (creation and destruction)
static void BM_Vector2Creation(benchmark::State& state) {
    const size_t VectorCount = state.range(0);
    auto random_floats = GenerateRandomFloats(VectorCount * 2);
    
    for (auto _ : state) {
        std::vector<Vector2> vectors;
        vectors.reserve(VectorCount);
        
        for (size_t i = 0; i < VectorCount * 2; i += 2) {
            vectors.emplace_back(random_floats[i], random_floats[i + 1]);
        }
        benchmark::DoNotOptimize(vectors);
    }
}
BENCHMARK(BM_Vector2Creation)->Range(8, 8<<10);

// Benchmark cache utilization with sequential vs random access
static void BM_Vector2SequentialAccess(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 2);
    std::vector<Vector2> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 2; i += 2) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1]);
    }
    
    size_t index = 0;
    float sum = 0.0f;
    for (auto _ : state) {
        sum += vectors[index % VectorCount].length();
        index++;
    }
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_Vector2SequentialAccess);

static void BM_Vector2RandomAccess(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 2);
    std::vector<Vector2> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 2; i += 2) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1]);
    }
    
    // Generate random access pattern
    std::vector<size_t> indices(VectorCount);
    std::iota(indices.begin(), indices.end(), 0);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);
    
    size_t index = 0;
    float sum = 0.0f;
    for (auto _ : state) {
        sum += vectors[indices[index % VectorCount]].length();
        index++;
    }
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_Vector2RandomAccess);

// Benchmark SIMD vs non-SIMD operations
static void BM_Vector2DotProduct(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 2);
    std::vector<Vector2> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 2; i += 2) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1]);
    }
    
    size_t index = 0;
    float sum = 0.0f;
    for (auto _ : state) {
        sum += vectors[index % VectorCount].dot(vectors[(index + 1) % VectorCount]);
        index++;
    }
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_Vector2DotProduct);

// Measure allocation time
static void BM_Vector2AllocationTime(benchmark::State& state) {
    for (auto _ : state) {
        Vector2* v = new Vector2(1.0f, 2.0f);
        benchmark::DoNotOptimize(v);
        delete v;
    }
}
BENCHMARK(BM_Vector2AllocationTime);

// Array operations to test cache-friendly patterns
static void BM_Vector2ArrayOperations(benchmark::State& state) {
    const size_t ArraySize = 1024; // Cache-friendly size
    std::array<Vector2, ArraySize> arr1, arr2, result;
    
    auto random_floats = GenerateRandomFloats(ArraySize * 4);  // Need 4x for both arrays
    for (size_t i = 0; i < ArraySize; ++i) {
        arr1[i] = Vector2(random_floats[i*2], random_floats[i*2+1]);
        arr2[i] = Vector2(random_floats[(ArraySize*2) + i*2], random_floats[(ArraySize*2) + i*2+1]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < ArraySize; ++i) {
            result[i] = arr1[i] + arr2[i];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vector2ArrayOperations);
