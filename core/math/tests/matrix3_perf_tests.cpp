#include <benchmark/benchmark.h>
#include "matrix3.hpp"
#include "vector3.hpp"
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

// Basic Matrix3 Operation Benchmarks
static void BM_Matrix3_Construction(benchmark::State& state) {
    auto random_floats = GenerateRandomFloats(9);
    for (auto _ : state) {
        Matrix3 m(
            random_floats[0], random_floats[1], random_floats[2],
            random_floats[3], random_floats[4], random_floats[5],
            random_floats[6], random_floats[7], random_floats[8]
        );
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(BM_Matrix3_Construction);

static void BM_Matrix3_Multiplication(benchmark::State& state) {
    const size_t MatrixCount = 1000000;
    auto random_floats = GenerateRandomFloats(MatrixCount * 9);
    std::vector<Matrix3> matrices;
    matrices.reserve(MatrixCount);
    
    for (size_t i = 0; i < MatrixCount * 9; i += 9) {
        matrices.emplace_back(
            random_floats[i], random_floats[i+1], random_floats[i+2],
            random_floats[i+3], random_floats[i+4], random_floats[i+5],
            random_floats[i+6], random_floats[i+7], random_floats[i+8]
        );
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix3 result = matrices[index % (MatrixCount-1)] * matrices[(index + 1) % (MatrixCount-1)];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix3_Multiplication);

static void BM_Matrix3_VectorMultiplication(benchmark::State& state) {
    const size_t Count = 1000000;
    auto random_floats = GenerateRandomFloats(Count * 12);  // 9 for matrix, 3 for vector
    std::vector<Matrix3> matrices;
    std::vector<Vector3> vectors;
    matrices.reserve(Count);
    vectors.reserve(Count);
    
    for (size_t i = 0; i < Count * 12; i += 12) {
        matrices.emplace_back(
            random_floats[i], random_floats[i+1], random_floats[i+2],
            random_floats[i+3], random_floats[i+4], random_floats[i+5],
            random_floats[i+6], random_floats[i+7], random_floats[i+8]
        );
        vectors.emplace_back(random_floats[i+9], random_floats[i+10], random_floats[i+11]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector3 result = matrices[index % Count] * vectors[index % Count];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix3_VectorMultiplication);

// 3D-specific rotation benchmarks
static void BM_Matrix3_RotationAxis(benchmark::State& state) {
    const size_t Count = 1000;
    std::vector<float> angles = GenerateRandomFloats(Count);
    std::vector<Vector3> axes(Count);
    
    // Generate normalized random axes
    for (size_t i = 0; i < Count; ++i) {
        Vector3 axis(
            angles[(i * 3) % Count],
            angles[(i * 3 + 1) % Count],
            angles[(i * 3 + 2) % Count]
        );
        axes[i] = axis.normalized();
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix3 rot = Matrix3::fromAxisAngle(axes[index % Count], angles[index % Count]);
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix3_RotationAxis);

// Benchmark individual axis rotations
static void BM_Matrix3_RotationX(benchmark::State& state) {
    std::vector<float> angles = GenerateRandomFloats(1000);
    size_t index = 0;
    for (auto _ : state) {
        Matrix3 rot = Matrix3::rotationX(angles[index % 1000]);
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix3_RotationX);

static void BM_Matrix3_RotationY(benchmark::State& state) {
    std::vector<float> angles = GenerateRandomFloats(1000);
    size_t index = 0;
    for (auto _ : state) {
        Matrix3 rot = Matrix3::rotationY(angles[index % 1000]);
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix3_RotationY);

static void BM_Matrix3_RotationZ(benchmark::State& state) {
    std::vector<float> angles = GenerateRandomFloats(1000);
    size_t index = 0;
    for (auto _ : state) {
        Matrix3 rot = Matrix3::rotationZ(angles[index % 1000]);
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix3_RotationZ);

// Memory Layout and Cache Performance Benchmarks
static void BM_Matrix3_Sequential_Multiplication(benchmark::State& state) {
    const size_t MatrixCount = 1024;  // Cache-friendly size
    std::vector<Matrix3> matrices(MatrixCount);
    std::vector<Matrix3> results(MatrixCount);
    
    auto random_floats = GenerateRandomFloats(MatrixCount * 9);
    for (size_t i = 0; i < MatrixCount; ++i) {
        matrices[i] = Matrix3(
            random_floats[i*9], random_floats[i*9+1], random_floats[i*9+2],
            random_floats[i*9+3], random_floats[i*9+4], random_floats[i*9+5],
            random_floats[i*9+6], random_floats[i*9+7], random_floats[i*9+8]
        );
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < MatrixCount-1; ++i) {
            results[i] = matrices[i] * matrices[i+1];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_Matrix3_Sequential_Multiplication);

static void BM_Matrix3_Random_Multiplication(benchmark::State& state) {
    const size_t MatrixCount = 1024;
    std::vector<Matrix3> matrices(MatrixCount);
    std::vector<Matrix3> results(MatrixCount);
    std::vector<size_t> indices(MatrixCount);
    
    auto random_floats = GenerateRandomFloats(MatrixCount * 9);
    for (size_t i = 0; i < MatrixCount; ++i) {
        matrices[i] = Matrix3(
            random_floats[i*9], random_floats[i*9+1], random_floats[i*9+2],
            random_floats[i*9+3], random_floats[i*9+4], random_floats[i*9+5],
            random_floats[i*9+6], random_floats[i*9+7], random_floats[i*9+8]
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
BENCHMARK(BM_Matrix3_Random_Multiplication);

// Test chain multiplication optimization
static void BM_Matrix3_ChainMultiplication(benchmark::State& state) {
    const size_t ChainLength = state.range(0);
    std::vector<Matrix3> matrices(ChainLength);
    Matrix3 result;
    
    auto random_floats = GenerateRandomFloats(ChainLength * 9);
    for (size_t i = 0; i < ChainLength; ++i) {
        matrices[i] = Matrix3(
            random_floats[i*9], random_floats[i*9+1], random_floats[i*9+2],
            random_floats[i*9+3], random_floats[i*9+4], random_floats[i*9+5],
            random_floats[i*9+6], random_floats[i*9+7], random_floats[i*9+8]
        );
    }
    
    for (auto _ : state) {
        result = matrices[0];
        for (size_t i = 1; i < ChainLength; ++i) {
            result = result * matrices[i];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix3_ChainMultiplication)->Range(2, 8)->Name("Matrix3_ChainMultiplication");

// Test special case optimization (Identity, Scaling, Rotation)
static void BM_Matrix3_SpecialCases(benchmark::State& state) {
    const size_t Count = 1024;
    std::vector<Matrix3> matrices;
    matrices.reserve(Count);
    Matrix3 result;
    
    auto random_floats = GenerateRandomFloats(Count);
    for (size_t i = 0; i < Count; ++i) {
        switch (i % 4) {
            case 0: // Identity
                matrices.push_back(Matrix3::identity());
                break;
            case 1: // Scaling
                matrices.push_back(Matrix3::scale(random_floats[i], random_floats[i], random_floats[i]));
                break;
            case 2: // X Rotation
                matrices.push_back(Matrix3::rotationX(random_floats[i]));
                break;
            case 3: // Axis-angle rotation
                Vector3 axis(random_floats[i], random_floats[(i+1)%Count], random_floats[(i+2)%Count]);
                matrices.push_back(Matrix3::fromAxisAngle(axis.normalized(), random_floats[i]));
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
BENCHMARK(BM_Matrix3_SpecialCases);

// Test matrix operations with different data alignments
template<size_t Alignment>
static void BM_Matrix3_DataAlignment(benchmark::State& state) {
    const size_t Count = 1024;
    alignas(Alignment) std::array<Matrix3, Count> matrices;
    alignas(Alignment) std::array<Matrix3, Count> results;
    
    auto random_floats = GenerateRandomFloats(Count * 9);
    for (size_t i = 0; i < Count; ++i) {
        matrices[i] = Matrix3(
            random_floats[i*9], random_floats[i*9+1], random_floats[i*9+2],
            random_floats[i*9+3], random_floats[i*9+4], random_floats[i*9+5],
            random_floats[i*9+6], random_floats[i*9+7], random_floats[i*9+8]
        );
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count-1; ++i) {
            results[i] = matrices[i] * matrices[i+1];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK_TEMPLATE(BM_Matrix3_DataAlignment, 4)->Name("Matrix3_Alignment_4bytes");
BENCHMARK_TEMPLATE(BM_Matrix3_DataAlignment, 8)->Name("Matrix3_Alignment_8bytes");
BENCHMARK_TEMPLATE(BM_Matrix3_DataAlignment, 16)->Name("Matrix3_Alignment_16bytes");
BENCHMARK_TEMPLATE(BM_Matrix3_DataAlignment, 32)->Name("Matrix3_Alignment_32bytes");

// Test SIMD vs non-SIMD operations
static void BM_Matrix3_SIMD_vs_Scalar(benchmark::State& state) {
    const bool use_simd = state.range(0);
    const size_t Count = 1024;
    auto m1_data = GenerateRandomFloats(Count * 9);
    auto m2_data = GenerateRandomFloats(Count * 9);
    auto result_data = std::vector<float>(Count * 9);
    
    for (auto _ : state) {
        if (use_simd) {
            for (size_t i = 0; i < Count * 9; i += 9) {
                SimdUtils::MultiplyMatrix3(&m1_data[i], &m2_data[i], &result_data[i]);
            }
        } else {
            for (size_t i = 0; i < Count * 9; i += 9) {
                // Manual scalar multiplication
                for (int row = 0; row < 3; ++row) {
                    for (int col = 0; col < 3; ++col) {
                        float sum = 0;
                        for (int k = 0; k < 3; ++k) {
                            sum += m1_data[i + row*3 + k] * m2_data[i + k*3 + col];
                        }
                        result_data[i + row*3 + col] = sum;
                    }
                }
            }
        }
        benchmark::DoNotOptimize(result_data);
    }
}
BENCHMARK(BM_Matrix3_SIMD_vs_Scalar)->Arg(0)->Arg(1)->Name("Matrix3_Compute(Scalar=0/SIMD=1)");

// Benchmark determinant and inverse operations
static void BM_Matrix3_Determinant(benchmark::State& state) {
    const size_t Count = 1000000;
    auto random_floats = GenerateRandomFloats(Count * 9);
    std::vector<Matrix3> matrices;
    matrices.reserve(Count);
    
    for (size_t i = 0; i < Count * 9; i += 9) {
        matrices.emplace_back(
            random_floats[i], random_floats[i+1], random_floats[i+2],
            random_floats[i+3], random_floats[i+4], random_floats[i+5],
            random_floats[i+6], random_floats[i+7], random_floats[i+8]
        );
    }
    
    size_t index = 0;
    for (auto _ : state) {
        float det = matrices[index % Count].determinant();
        benchmark::DoNotOptimize(det);
        index++;
    }
}
BENCHMARK(BM_Matrix3_Determinant);

static void BM_Matrix3_Inverse(benchmark::State& state) {
    const size_t Count = 1000000;
    auto random_floats = GenerateRandomFloats(Count * 9);
    std::vector<Matrix3> matrices;
    matrices.reserve(Count);
    
    // Generate invertible matrices
    for (size_t i = 0; i < Count * 9; i += 9) {
        Matrix3 m(
            random_floats[i], random_floats[i+1], random_floats[i+2],
            random_floats[i+3], random_floats[i+4], random_floats[i+5],
            random_floats[i+6], random_floats[i+7], random_floats[i+8]
        );
        if (std::abs(m.determinant()) > 1e-6f) {
            matrices.push_back(m);
        }
    }
    
    Matrix3 result;
    size_t index = 0;
    for (auto _ : state) {
        matrices[index % matrices.size()].getInverse(result);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix3_Inverse);