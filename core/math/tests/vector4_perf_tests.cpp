#include <benchmark/benchmark.h>
#include "vector4.hpp"
#include "vector3.hpp"
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
static void BM_Vector4Addition(benchmark::State& state) {
    const size_t VectorCount = 100000;  // Reduced to prevent memory pressure
    auto random_floats = GenerateRandomFloats(VectorCount * 4);
    std::vector<Vector4> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 4; i += 4) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], 
                           random_floats[i + 2], random_floats[i + 3]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector4 result = vectors[index % VectorCount] + vectors[(index + 1) % VectorCount];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4Addition);

// Vector normalization benchmark
static void BM_Vector4Normalization(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 4);
    std::vector<Vector4> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 4; i += 4) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], 
                           random_floats[i + 2], random_floats[i + 3]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector4 result = vectors[index % VectorCount].normalized();
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4Normalization);

// Memory operations benchmark
static void BM_Vector4Creation(benchmark::State& state) {
    const size_t VectorCount = state.range(0);
    auto random_floats = GenerateRandomFloats(VectorCount * 4);
    
    for (auto _ : state) {
        std::vector<Vector4> vectors;
        vectors.reserve(VectorCount);
        
        for (size_t i = 0; i < VectorCount * 4; i += 4) {
            vectors.emplace_back(random_floats[i], random_floats[i + 1], 
                               random_floats[i + 2], random_floats[i + 3]);
        }
        benchmark::DoNotOptimize(vectors);
    }
}
BENCHMARK(BM_Vector4Creation)->Range(8, 8<<10);

// Cache performance benchmarks
static void BM_Vector4SequentialAccess(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 4);
    std::vector<Vector4> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 4; i += 4) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], 
                           random_floats[i + 2], random_floats[i + 3]);
    }
    
    size_t index = 0;
    float sum = 0.0f;
    for (auto _ : state) {
        sum += vectors[index % (VectorCount-1)].length();
        index++;
    }
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_Vector4SequentialAccess);

static void BM_Vector4RandomAccess(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 4);
    std::vector<Vector4> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 4; i += 4) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], 
                           random_floats[i + 2], random_floats[i + 3]);
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
BENCHMARK(BM_Vector4RandomAccess);

// Dot product benchmark
static void BM_Vector4DotProduct(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 4);
    std::vector<Vector4> vectors;
    vectors.reserve(VectorCount);
    
    for (size_t i = 0; i < VectorCount * 4; i += 4) {
        vectors.emplace_back(random_floats[i], random_floats[i + 1], 
                           random_floats[i + 2], random_floats[i + 3]);
    }
    
    size_t index = 0;
    float sum = 0.0f;
    for (auto _ : state) {
        sum += vectors[index % VectorCount].dot(vectors[(index + 1) % VectorCount]);
        index++;
    }
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(BM_Vector4DotProduct);

// Homogeneous coordinate operations
static void BM_Vector4HomogeneousNormalization(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto random_floats = GenerateRandomFloats(VectorCount * 4);
    std::vector<Vector4> vectors;
    vectors.reserve(VectorCount);
    
    // Ensure w component is non-zero for homogeneous coordinates
    for (size_t i = 0; i < VectorCount * 4; i += 4) {
        float w = random_floats[i + 3];
        if (std::abs(w) < 0.0001f) w = 1.0f; // Prevent near-zero w values
        vectors.emplace_back(random_floats[i], random_floats[i + 1], 
                           random_floats[i + 2], w);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector4 v = vectors[index % VectorCount];
        Vector4 result(v.x / v.w, v.y / v.w, v.z / v.w, 1.0f);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Vector4HomogeneousNormalization);

// Compare Vector3 to Vector4 performance
static void BM_Vector3vs4_Addition(benchmark::State& state) {
    const bool use4D = state.range(0);
    const size_t VectorCount = 1000000;
    
    if (use4D) {
        auto random_floats = GenerateRandomFloats(VectorCount * 4);
        std::vector<Vector4> vectors;
        vectors.reserve(VectorCount);
        
        for (size_t i = 0; i < VectorCount * 4; i += 4) {
            vectors.emplace_back(random_floats[i], random_floats[i + 1], 
                               random_floats[i + 2], random_floats[i + 3]);
        }
        
        size_t index = 0;
        for (auto _ : state) {
            Vector4 result = vectors[index % VectorCount] + vectors[(index + 1) % VectorCount];
            benchmark::DoNotOptimize(result);
            index++;
        }
    } else {
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
}
BENCHMARK(BM_Vector3vs4_Addition)->Arg(0)->Arg(1)->Name("BM_Vector3vs4_Addition(3D=0/4D=1)");

// Benchmark measuring performance of 1024 Vector4 addition operations
static void BM_Vector4Array1024Additions(benchmark::State& state) {
    const size_t ArraySize = 1024; // Cache-friendly size
    std::array<Vector4, ArraySize> arr1, arr2, result;
    
    auto random_floats = GenerateRandomFloats(ArraySize * 8);  // Need double for arr1 and arr2
    for (size_t i = 0; i < ArraySize; ++i) {
        arr1[i] = Vector4(random_floats[i*4], random_floats[i*4+1], 
                         random_floats[i*4+2], random_floats[i*4+3]);
        // Use second half of random_floats for arr2
        const size_t offset = ArraySize * 4;
        arr2[i] = Vector4(random_floats[offset + i*4], 
                         random_floats[offset + i*4+1],
                         random_floats[offset + i*4+2],
                         random_floats[offset + i*4+3]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < ArraySize; ++i) {
            result[i] = arr1[i] + arr2[i];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vector4Array1024Additions)->Name("Vector4_1024_Array_Additions");