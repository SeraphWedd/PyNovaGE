#include <benchmark/benchmark.h>
#include "vector3.hpp"
#include "math/vector2.hpp"
#include <vector>
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

// Basic vector operations benchmark
static void BM_Vector3Addition(benchmark::State& state) {
    const size_t VectorCount = 100000;  // Reduced to prevent memory pressure
    auto random_floats = GenerateRandomFloats(VectorCount * 3);
    std::vector<Vector3> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 3; i += 3) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector3 result = vectors[index % VectorCount] + vectors[(index + 1) % VectorCount];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector3Addition);

// Vector normalization benchmark
static void BM_Vector3Normalization(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 3);
    std::vector<Vector3> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 3; i += 3) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector3 result = vectors[index % VectorCount].normalized();
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector3Normalization);

// Memory operations benchmark
static void BM_Vector3Creation(benchmark::State& state) {
    const size_t VectorCount = state.range(0);
    auto random_floats = GenerateRandomFloats(VectorCount * 3);
    
    for (auto _ : state) {
        std::vector<Vector3> vectors;
        vectors.reserve(VectorCount);
        
        for (size_t i = 0; i < VectorCount * 3; i += 3) {
            vectors.emplace_back(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
        }
        benchmark::DoNotOptimize(vectors);
    }
}
BENCHMARK(BM_Vector3Creation)->Range(8, 8<<10);

// Cache performance benchmarks
static void BM_Vector3SequentialAccess(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 3);
    std::vector<Vector3> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 3; i += 3) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
    }
    
    size_t index = 0;
    float sum = 0.0f;
    for (auto _ : state) {
        sum += vectors[index % (VectorCount-1)].length();
        index++;
    }
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_Vector3SequentialAccess);

static void BM_Vector3RandomAccess(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 3);
    std::vector<Vector3> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 3; i += 3) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
    }
    
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
BENCHMARK(BM_Vector3RandomAccess);

// Dot and Cross product benchmarks
static void BM_Vector3DotProduct(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 3);
    std::vector<Vector3> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 3; i += 3) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
    }
    
    size_t index = 0;
    float sum = 0.0f;
    for (auto _ : state) {
        sum += vectors[index % VectorCount].dot(vectors[(index + 1) % VectorCount]);
        index++;
    }
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_Vector3DotProduct);

static void BM_Vector3CrossProduct(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 3);
    std::vector<Vector3> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 3; i += 3) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector3 result = vectors[index % VectorCount].cross(vectors[(index + 1) % VectorCount]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector3CrossProduct);

// Advanced geometric operations
static void BM_Vector3Reflection(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 3);
    std::vector<Vector3> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 3; i += 3) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
    }
    
    // Generate normalized normals for reflection
    std::vector<Vector3> normals;
    normals.reserve(VectorCount);
    for (size_t i = 0; i < VectorCount * 3; i += 3) {
        Vector3 normal(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
        normals.push_back(normal.normalized());
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector3 result = vectors[index % VectorCount].reflect(normals[index % VectorCount]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector3Reflection);

// Compare 2D vs 3D performance for common operations
static void BM_Vector2vs3_Addition(benchmark::State& state) {
    const bool use3D = state.range(0);
    const size_t VectorCount = 1000000;
    
    if (use3D) {
        auto random_floats = GenerateRandomFloats(VectorCount * 3);
        std::vector<Vector3> vectors;
        vectors.reserve(VectorCount);
        
        for (size_t i = 0; i < VectorCount * 3; i += 3) {
            vectors.emplace_back(random_floats[i], random_floats[i + 1], random_floats[i + 2]);
        }
        
        size_t index = 0;
        for (auto _ : state) {
            Vector3 result = vectors[index % VectorCount] + vectors[(index + 1) % VectorCount];
            benchmark::DoNotOptimize(result);
            index++;
        }
    } else {
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
}
BENCHMARK(BM_Vector2vs3_Addition)->Arg(0)->Arg(1)->Name("BM_Vector2vs3_Addition(2D=0/3D=1)");

// Array operations with different sizes
static void BM_Vector3ArrayOperations(benchmark::State& state) {
    const size_t ArraySize = 1024; // Cache-friendly size
    std::array<Vector3, ArraySize> arr1, arr2, result;
    
    auto random_floats = GenerateRandomFloats(ArraySize * 6);  // Need double for arr1 and arr2
    for (size_t i = 0; i < ArraySize; ++i) {
        arr1[i] = Vector3(random_floats[i*3], random_floats[i*3+1], random_floats[i*3+2]);
        // Use second half of random_floats for arr2
        const size_t offset = ArraySize * 3;
        arr2[i] = Vector3(random_floats[offset + i*3], 
                         random_floats[offset + i*3+1],
                         random_floats[offset + i*3+2]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < ArraySize; ++i) {
            result[i] = arr1[i] + arr2[i];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vector3ArrayOperations);
