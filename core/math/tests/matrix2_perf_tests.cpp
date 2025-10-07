#include <benchmark/benchmark.h>
#include "matrix2.hpp"
#include "vector2.hpp"
#include "simd_utils.hpp"
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

// Basic Matrix2 Operation Benchmarks
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

// Advanced Operation Benchmarks
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
        } while (std::abs(det) < 1e-12f);
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

// Memory Layout and Cache Performance Benchmarks
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
    
    auto random_floats = GenerateRandomFloats(MatrixCount * 4);
    for (size_t i = 0; i < MatrixCount; ++i) {
        matrices[i] = Matrix2(random_floats[i*4], random_floats[i*4 + 1],
                            random_floats[i*4 + 2], random_floats[i*4 + 3]);
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
BENCHMARK(BM_Matrix2_Random_Multiplication);

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

// Compare with raw operations
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

// New benchmarks for Matrix2

// Test different memory layouts
static void BM_Matrix2_RowMajor_Vs_ColMajor(benchmark::State& state) {
    const bool use_row_major = state.range(0);
    const size_t Count = 1024;
    std::vector<float> m1(Count * 4);
    std::vector<float> m2(Count * 4);
    std::vector<float> result(Count * 4);
    
    auto random_floats = GenerateRandomFloats(Count * 8);
    std::copy(random_floats.begin(), random_floats.begin() + Count * 4, m1.begin());
    std::copy(random_floats.begin() + Count * 4, random_floats.end(), m2.begin());
    
    for (auto _ : state) {
        if (use_row_major) {
            // Row-major multiplication
            for (size_t i = 0; i < Count * 4; i += 4) {
                result[i]   = m1[i] * m2[i] + m1[i+1] * m2[i+2];
                result[i+1] = m1[i] * m2[i+1] + m1[i+1] * m2[i+3];
                result[i+2] = m1[i+2] * m2[i] + m1[i+3] * m2[i+2];
                result[i+3] = m1[i+2] * m2[i+1] + m1[i+3] * m2[i+3];
            }
        } else {
            // Column-major multiplication
            for (size_t i = 0; i < Count * 4; i += 4) {
                result[i]   = m1[i] * m2[i] + m1[i+2] * m2[i+1];
                result[i+1] = m1[i+1] * m2[i] + m1[i+3] * m2[i+1];
                result[i+2] = m1[i] * m2[i+2] + m1[i+2] * m2[i+3];
                result[i+3] = m1[i+1] * m2[i+2] + m1[i+3] * m2[i+3];
            }
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix2_RowMajor_Vs_ColMajor)->Arg(0)->Arg(1)->Name("Matrix2_Layout(Col=0/Row=1)");

// Test chain multiplication optimization
static void BM_Matrix2_ChainMultiplication(benchmark::State& state) {
    const size_t ChainLength = state.range(0);
    std::vector<Matrix2> matrices(ChainLength);
    Matrix2 result;
    
    // Initialize matrices with random data
    auto random_floats = GenerateRandomFloats(ChainLength * 4);
    for (size_t i = 0; i < ChainLength; ++i) {
        matrices[i] = Matrix2(random_floats[i*4], random_floats[i*4+1],
                            random_floats[i*4+2], random_floats[i*4+3]);
    }
    
    for (auto _ : state) {
        // Copy first matrix to result
        result = matrices[0];
        
        // Multiply through the chain
        for (size_t i = 1; i < ChainLength; ++i) {
            result = result * matrices[i];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix2_ChainMultiplication)->Range(2, 8)->Name("Matrix2_ChainMultiplication");

// Test special case optimization (Identity, Scaling, Rotation)
static void BM_Matrix2_SpecialCases(benchmark::State& state) {
    const size_t Count = 1024;
    std::vector<Matrix2> matrices;
    matrices.reserve(Count);
    Matrix2 result;
    
    // Create a mix of special case matrices
    auto random_floats = GenerateRandomFloats(Count);
    for (size_t i = 0; i < Count; ++i) {
        switch (i % 3) {
            case 0: // Identity
                matrices.push_back(Matrix2::identity());
                break;
            case 1: // Scaling
                matrices.push_back(Matrix2::scale(random_floats[i], random_floats[i]));
                break;
            case 2: // Rotation
                matrices.push_back(Matrix2::rotation(random_floats[i]));
                break;
        }
    }
    
    size_t index = 0;
    for (auto _ : state) {
        result = matrices[index % (Count-1)] * matrices[(index + 1) % (Count-1)];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix2_SpecialCases);

// Test matrix operations with different data alignments
template<size_t Alignment>
static void BM_Matrix2_DataAlignment(benchmark::State& state) {
    const size_t Count = 1024;
    alignas(Alignment) std::array<Matrix2, Count> matrices;
    alignas(Alignment) std::array<Matrix2, Count> results;
    
    auto random_floats = GenerateRandomFloats(Count * 4);
    for (size_t i = 0; i < Count; ++i) {
        matrices[i] = Matrix2(random_floats[i*4], random_floats[i*4+1],
                            random_floats[i*4+2], random_floats[i*4+3]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count-1; ++i) {
            results[i] = matrices[i] * matrices[i+1];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK_TEMPLATE(BM_Matrix2_DataAlignment, 4)->Name("Matrix2_Alignment_4bytes");
BENCHMARK_TEMPLATE(BM_Matrix2_DataAlignment, 8)->Name("Matrix2_Alignment_8bytes");
BENCHMARK_TEMPLATE(BM_Matrix2_DataAlignment, 16)->Name("Matrix2_Alignment_16bytes");
BENCHMARK_TEMPLATE(BM_Matrix2_DataAlignment, 32)->Name("Matrix2_Alignment_32bytes");

// Test SIMD vs non-SIMD operations
static void BM_Matrix2_SIMD_vs_Scalar(benchmark::State& state) {
    const bool use_simd = state.range(0);
    const size_t Count = 1024;
    auto m1_data = GenerateRandomFloats(Count * 4);
    auto m2_data = GenerateRandomFloats(Count * 4);
    auto result_data = std::vector<float>(Count * 4);
    
    for (auto _ : state) {
        if (use_simd) {
            for (size_t i = 0; i < Count * 4; i += 4) {
                SimdUtils::MultiplyMatrix2x2(&m1_data[i], &m2_data[i], &result_data[i]);
            }
        } else {
            for (size_t i = 0; i < Count * 4; i += 4) {
                // Manual scalar multiplication
                result_data[i]   = m1_data[i] * m2_data[i] + m1_data[i+1] * m2_data[i+2];
                result_data[i+1] = m1_data[i] * m2_data[i+1] + m1_data[i+1] * m2_data[i+3];
                result_data[i+2] = m1_data[i+2] * m2_data[i] + m1_data[i+3] * m2_data[i+2];
                result_data[i+3] = m1_data[i+2] * m2_data[i+1] + m1_data[i+3] * m2_data[i+3];
            }
        }
        benchmark::DoNotOptimize(result_data);
    }
}
BENCHMARK(BM_Matrix2_SIMD_vs_Scalar)->Arg(0)->Arg(1)->Name("Matrix2_Compute(Scalar=0/SIMD=1)");