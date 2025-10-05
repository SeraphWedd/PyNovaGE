#include <benchmark/benchmark.h>
#include <matrix2.hpp>
#include <matrix4.hpp>
#include <vector2.hpp>
#include <vector4.hpp>
#include <vector>
#include <random>
#include <array>

using namespace pynovage::math;

// Utility function to generate random matrices
static std::vector<float> GenerateRandomFloats(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-10.0f, 10.0f);
    
    std::vector<float> result(count);
    for (auto& val : result) {
        val = dis(gen);
    }
    return result;
}

// Matrix2 Operation Benchmarks
static void BM_Matrix2_Construction(benchmark::State& state) {
    auto random_floats = GenerateRandomFloats(4);
    for (auto _ : state) {
        Matrix2 m(random_floats[0], random_floats[1],
                  random_floats[2], random_floats[3]);
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(BM_Matrix2_Construction);

static void BM_Matrix2_Multiplication(benchmark::State& state) {
    const size_t MatrixCount = 1000000;
    auto random_floats = GenerateRandomFloats(MatrixCount * 4);
    std::vector<Matrix2> matrices;
    matrices.reserve(MatrixCount);
    
    for (size_t i = 0; i < MatrixCount * 4; i += 4) {
        matrices.emplace_back(random_floats[i], random_floats[i + 1],
                            random_floats[i + 2], random_floats[i + 3]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix2 result = matrices[index % (MatrixCount-1)] * matrices[(index + 1) % (MatrixCount-1)];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix2_Multiplication);

static void BM_Matrix2_VectorMultiplication(benchmark::State& state) {
    const size_t Count = 1000000;
    auto random_floats = GenerateRandomFloats(Count * 6);  // 4 for matrix, 2 for vector
    std::vector<Matrix2> matrices;
    std::vector<Vector2> vectors;
    matrices.reserve(Count);
    vectors.reserve(Count);
    
    for (size_t i = 0; i < (Count * 6); i += 6) {
        matrices.emplace_back(random_floats[i], random_floats[i + 1],
                            random_floats[i + 2], random_floats[i + 3]);
        vectors.emplace_back(random_floats[i + 4], random_floats[i + 5]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector2 result = matrices[index % Count] * vectors[index % Count];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix2_VectorMultiplication);

static void BM_Matrix2_Inverse(benchmark::State& state) {
    const size_t Count = 1000000;
    auto random_floats = GenerateRandomFloats(Count * 4);
    std::vector<Matrix2> matrices;
    matrices.reserve(Count);
    
    // Generate matrices with determinant != 0
    for (size_t i = 0; i < Count * 4; i += 4) {
        float det;
        Matrix2 m;
        do {
            m = Matrix2(random_floats[i], random_floats[i + 1],
                       random_floats[i + 2], random_floats[i + 3]);
            det = m.determinant();
        } while (std::abs(det) < 1e-12f); // Match SimdUtils::InvertMatrix2x2 epsilon
        matrices.push_back(m);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix2 inv;
        matrices[index % Count].inverse(inv);
        benchmark::DoNotOptimize(inv);
        index++;
    }
}
BENCHMARK(BM_Matrix2_Inverse);

static void BM_Matrix2_Rotation(benchmark::State& state) {
    std::vector<float> angles = GenerateRandomFloats(1000);
    size_t index = 0;
    for (auto _ : state) {
        Matrix2 rot = Matrix2::rotation(angles[index % 1000]);
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix2_Rotation);

// Cache performance tests
static void BM_Matrix2_Sequential_Multiplication(benchmark::State& state) {
    const size_t MatrixCount = 1024;  // Cache-friendly size
    std::vector<Matrix2> matrices(MatrixCount);
    std::vector<Matrix2> results(MatrixCount);
    
    auto random_floats = GenerateRandomFloats(MatrixCount * 4);
    for (size_t i = 0; i < MatrixCount; ++i) {
        matrices[i] = Matrix2(random_floats[i*4], random_floats[i*4 + 1],
                            random_floats[i*4 + 2], random_floats[i*4 + 3]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < MatrixCount-1; ++i) {
            results[i] = matrices[i] * matrices[i+1];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_Matrix2_Sequential_Multiplication);

static void BM_Matrix2_Random_Multiplication(benchmark::State& state) {
    const size_t MatrixCount = 1024;  // Same size as sequential for comparison
    std::vector<Matrix2> matrices(MatrixCount);
    std::vector<Matrix2> results(MatrixCount);
    std::vector<size_t> indices(MatrixCount);
    
    // Initialize matrices
    auto random_floats = GenerateRandomFloats(MatrixCount * 4);
    for (size_t i = 0; i < MatrixCount; ++i) {
        matrices[i] = Matrix2(random_floats[i*4], random_floats[i*4 + 1],
                            random_floats[i*4 + 2], random_floats[i*4 + 3]);
        indices[i] = i;
    }
    
    // Shuffle indices
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);
    
    for (auto _ : state) {
        for (size_t i = 0; i < MatrixCount-1; ++i) {
            results[i] = matrices[indices[i]] * matrices[indices[i+1]];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_Matrix2_Random_Multiplication);

// Memory alignment impact test
static void BM_Matrix2_Aligned_Operations(benchmark::State& state) {
    const size_t Count = 1024;
    alignas(16) std::array<Matrix2, Count> matrices;
    alignas(16) std::array<Matrix2, Count> results;
    
    auto random_floats = GenerateRandomFloats(Count * 4);
    for (size_t i = 0; i < Count; ++i) {
        matrices[i] = Matrix2(random_floats[i*4], random_floats[i*4 + 1],
                            random_floats[i*4 + 2], random_floats[i*4 + 3]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count-1; ++i) {
            results[i] = matrices[i] * matrices[i+1];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_Matrix2_Aligned_Operations);

// Compare Matrix2 vs hand-coded operations
static void BM_Matrix2_Raw_Multiply(benchmark::State& state) {
    const size_t Count = 1024;
    std::vector<float> m1(Count * 4);
    std::vector<float> m2(Count * 4);
    std::vector<float> result(Count * 4);
    
    auto random_floats = GenerateRandomFloats(Count * 8);
    std::copy(random_floats.begin(), random_floats.begin() + Count * 4, m1.begin());
    std::copy(random_floats.begin() + Count * 4, random_floats.end(), m2.begin());
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count * 4; i += 4) {
            result[i]   = m1[i] * m2[i] + m1[i+1] * m2[i+2];
            result[i+1] = m1[i] * m2[i+1] + m1[i+1] * m2[i+3];
            result[i+2] = m1[i+2] * m2[i] + m1[i+3] * m2[i+2];
            result[i+3] = m1[i+2] * m2[i+1] + m1[i+3] * m2[i+3];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix2_Raw_Multiply);

// Matrix4 Operation Benchmarks
static void BM_Matrix4_Multiplication(benchmark::State& state) {
    const size_t MatrixCount = 1000000;
    auto random_floats = GenerateRandomFloats(MatrixCount * 16);
    std::vector<Matrix4> matrices;
    matrices.reserve(MatrixCount);
    
    for (size_t i = 0; i < (MatrixCount * 16); i += 16) {
        matrices.emplace_back(
            random_floats[i],    random_floats[i+1],  random_floats[i+2],  random_floats[i+3],
            random_floats[i+4],  random_floats[i+5],  random_floats[i+6],  random_floats[i+7],
            random_floats[i+8],  random_floats[i+9],  random_floats[i+10], random_floats[i+11],
            random_floats[i+12], random_floats[i+13], random_floats[i+14], random_floats[i+15]
        );
    }
    
    size_t index = 0;
    const size_t maxIndex = matrices.size() - 1;
    for (auto _ : state) {
        Matrix4 result = matrices[index % maxIndex] * matrices[(index + 1) % maxIndex];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4_Multiplication);

static void BM_Matrix4_VectorMultiplication(benchmark::State& state) {
    const size_t Count = 1000000;
    auto random_floats = GenerateRandomFloats(Count * 20);  // 16 for matrix, 4 for vector
    std::vector<Matrix4> matrices;
    std::vector<Vector4> vectors;
    matrices.reserve(Count);
    vectors.reserve(Count);
    
    for (size_t i = 0; i < (Count * 20); i += 20) {
        matrices.emplace_back(
            random_floats[i],    random_floats[i+1],  random_floats[i+2],  random_floats[i+3],
            random_floats[i+4],  random_floats[i+5],  random_floats[i+6],  random_floats[i+7],
            random_floats[i+8],  random_floats[i+9],  random_floats[i+10], random_floats[i+11],
            random_floats[i+12], random_floats[i+13], random_floats[i+14], random_floats[i+15]
        );
        vectors.emplace_back(random_floats[i+16], random_floats[i+17], random_floats[i+18], random_floats[i+19]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector4 result = matrices[index % Count] * vectors[index % Count];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4_VectorMultiplication);

// Cache performance tests for Matrix4
static void BM_Matrix4_Sequential_Multiplication(benchmark::State& state) {
    const size_t MatrixCount = 1024;  // Cache-friendly size
    std::vector<Matrix4> matrices(MatrixCount);
    std::vector<Matrix4> results(MatrixCount);
    
    auto random_floats = GenerateRandomFloats(MatrixCount * 16);
    for (size_t i = 0; i < MatrixCount; ++i) {
        const size_t offset = i * 16;
        matrices[i] = Matrix4(
            random_floats[offset],    random_floats[offset+1],  random_floats[offset+2],  random_floats[offset+3],
            random_floats[offset+4],  random_floats[offset+5],  random_floats[offset+6],  random_floats[offset+7],
            random_floats[offset+8],  random_floats[offset+9],  random_floats[offset+10], random_floats[offset+11],
            random_floats[offset+12], random_floats[offset+13], random_floats[offset+14], random_floats[offset+15]
        );
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < MatrixCount-1; ++i) {
            results[i] = matrices[i] * matrices[i+1];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_Matrix4_Sequential_Multiplication);

static void BM_Matrix4_Random_Multiplication(benchmark::State& state) {
    const size_t MatrixCount = 1024;  // Same size as sequential for comparison
    std::vector<Matrix4> matrices(MatrixCount);
    std::vector<Matrix4> results(MatrixCount);
    std::vector<size_t> indices(MatrixCount);
    
    auto random_floats = GenerateRandomFloats(MatrixCount * 16);
    for (size_t i = 0; i < MatrixCount; ++i) {
        const size_t offset = i * 16;
        matrices[i] = Matrix4(
            random_floats[offset],    random_floats[offset+1],  random_floats[offset+2],  random_floats[offset+3],
            random_floats[offset+4],  random_floats[offset+5],  random_floats[offset+6],  random_floats[offset+7],
            random_floats[offset+8],  random_floats[offset+9],  random_floats[offset+10], random_floats[offset+11],
            random_floats[offset+12], random_floats[offset+13], random_floats[offset+14], random_floats[offset+15]
        );
        indices[i] = i;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);
    
    for (auto _ : state) {
        for (size_t i = 0; i < MatrixCount-1; ++i) {
            results[i] = matrices[indices[i]] * matrices[indices[i+1]];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_Matrix4_Random_Multiplication);
