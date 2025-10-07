#include <benchmark/benchmark.h>
#include "simd_utils.hpp"
#include <vector>
#include <random>
#include <array>
#include <memory>

using namespace pynovage::math;

// Utility function to generate random floats with alignment
static std::vector<float, std::allocator<float>> GenerateAlignedRandomFloats(size_t count, size_t alignment = 32) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1000.0f, 1000.0f);
    
    // Allocate aligned memory
    std::vector<float, std::allocator<float>> result(count);
    for (auto& val : result) {
        val = dis(gen);
    }
    return result;
}

// Benchmark SIMD vs non-SIMD Vector2 operations
static void BM_Vector2_SIMD_VS_Scalar(benchmark::State& state) {
    const bool use_simd = state.range(0);
    const size_t VectorCount = 1000000;
    auto data1 = GenerateAlignedRandomFloats(VectorCount * 2, 32);
    auto data2 = GenerateAlignedRandomFloats(VectorCount * 2, 32);
    auto result = GenerateAlignedRandomFloats(VectorCount * 2, 32);
    
    for (auto _ : state) {
        for (size_t i = 0; i < VectorCount * 2; i += 2) {
            if (use_simd) {
                SimdUtils::Add2f(&data1[i], &data2[i], &result[i]);
            } else {
                result[i] = data1[i] + data2[i];
                result[i+1] = data1[i+1] + data2[i+1];
            }
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vector2_SIMD_VS_Scalar)->Arg(0)->Arg(1)->Name("Vector2_Add(Scalar=0/SIMD=1)");

// Benchmark SIMD vs non-SIMD Vector3 operations
static void BM_Vector3_SIMD_VS_Scalar(benchmark::State& state) {
    const bool use_simd = state.range(0);
    const size_t VectorCount = 1000000;
    auto data1 = GenerateAlignedRandomFloats(VectorCount * 4, 32); // Extra float for alignment
    auto data2 = GenerateAlignedRandomFloats(VectorCount * 4, 32);
    auto result = GenerateAlignedRandomFloats(VectorCount * 4, 32);
    
    for (auto _ : state) {
        for (size_t i = 0; i < VectorCount * 4; i += 4) {
            if (use_simd) {
                SimdUtils::Add3f(&data1[i], &data2[i], &result[i]);
            } else {
                result[i] = data1[i] + data2[i];
                result[i+1] = data1[i+1] + data2[i+1];
                result[i+2] = data1[i+2] + data2[i+2];
            }
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vector3_SIMD_VS_Scalar)->Arg(0)->Arg(1)->Name("Vector3_Add(Scalar=0/SIMD=1)");

// Benchmark SIMD vs non-SIMD Vector4 operations
static void BM_Vector4_SIMD_VS_Scalar(benchmark::State& state) {
    const bool use_simd = state.range(0);
    const size_t VectorCount = 1000000;
    auto data1 = GenerateAlignedRandomFloats(VectorCount * 4, 32);
    auto data2 = GenerateAlignedRandomFloats(VectorCount * 4, 32);
    auto result = GenerateAlignedRandomFloats(VectorCount * 4, 32);
    
    for (auto _ : state) {
        for (size_t i = 0; i < VectorCount * 4; i += 4) {
            if (use_simd) {
                SimdUtils::Add4f(&data1[i], &data2[i], &result[i]);
            } else {
                result[i] = data1[i] + data2[i];
                result[i+1] = data1[i+1] + data2[i+1];
                result[i+2] = data1[i+2] + data2[i+2];
                result[i+3] = data1[i+3] + data2[i+3];
            }
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vector4_SIMD_VS_Scalar)->Arg(0)->Arg(1)->Name("Vector4_Add(Scalar=0/SIMD=1)");

// Benchmark SIMD vs non-SIMD Matrix4x4 operations
static void BM_Matrix4x4_SIMD_VS_Scalar(benchmark::State& state) {
    const bool use_simd = state.range(0);
    const size_t MatrixCount = 10000;
    auto data1 = GenerateAlignedRandomFloats(MatrixCount * 16, 32);
    auto data2 = GenerateAlignedRandomFloats(MatrixCount * 16, 32);
    auto result = GenerateAlignedRandomFloats(MatrixCount * 16, 32);
    
    for (auto _ : state) {
        for (size_t i = 0; i < MatrixCount * 16; i += 16) {
            if (use_simd) {
                SimdUtils::MultiplyMatrix4x4(&data1[i], &data2[i], &result[i]);
            } else {
                // Manual scalar 4x4 matrix multiplication
                for (int row = 0; row < 4; row++) {
                    for (int col = 0; col < 4; col++) {
                        float sum = 0;
                        for (int k = 0; k < 4; k++) {
                            sum += data1[i + row*4 + k] * data2[i + k*4 + col];
                        }
                        result[i + row*4 + col] = sum;
                    }
                }
            }
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix4x4_SIMD_VS_Scalar)->Arg(0)->Arg(1)->Name("Matrix4x4_Multiply(Scalar=0/SIMD=1)");

// Benchmark cache alignment impact
template<size_t Alignment>
static void BM_CacheAlignment(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto data1 = GenerateAlignedRandomFloats(VectorCount * 4, Alignment);
    auto data2 = GenerateAlignedRandomFloats(VectorCount * 4, Alignment);
    auto result = GenerateAlignedRandomFloats(VectorCount * 4, Alignment);
    
    for (auto _ : state) {
        for (size_t i = 0; i < VectorCount * 4; i += 4) {
            SimdUtils::Add4f(&data1[i], &data2[i], &result[i]);
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK_TEMPLATE(BM_CacheAlignment, 4)->Name("CacheAlignment_4bytes");
BENCHMARK_TEMPLATE(BM_CacheAlignment, 8)->Name("CacheAlignment_8bytes");
BENCHMARK_TEMPLATE(BM_CacheAlignment, 16)->Name("CacheAlignment_16bytes");
BENCHMARK_TEMPLATE(BM_CacheAlignment, 32)->Name("CacheAlignment_32bytes");

// Benchmark AoS vs SoA layout for SIMD operations
static void BM_DataLayout_AoS(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto data1 = GenerateAlignedRandomFloats(VectorCount * 4, 32);
    auto data2 = GenerateAlignedRandomFloats(VectorCount * 4, 32);
    auto result = GenerateAlignedRandomFloats(VectorCount * 4, 32);
    
    for (auto _ : state) {
        for (size_t i = 0; i < VectorCount * 4; i += 4) {
            SimdUtils::Add4f(&data1[i], &data2[i], &result[i]);
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_DataLayout_AoS)->Name("DataLayout_ArrayOfStructs");

static void BM_DataLayout_SoA(benchmark::State& state) {
    const size_t VectorCount = 1000000;
    auto data1_x = GenerateAlignedRandomFloats(VectorCount, 32);
    auto data1_y = GenerateAlignedRandomFloats(VectorCount, 32);
    auto data1_z = GenerateAlignedRandomFloats(VectorCount, 32);
    auto data1_w = GenerateAlignedRandomFloats(VectorCount, 32);
    auto data2_x = GenerateAlignedRandomFloats(VectorCount, 32);
    auto data2_y = GenerateAlignedRandomFloats(VectorCount, 32);
    auto data2_z = GenerateAlignedRandomFloats(VectorCount, 32);
    auto data2_w = GenerateAlignedRandomFloats(VectorCount, 32);
    auto result_x = GenerateAlignedRandomFloats(VectorCount, 32);
    auto result_y = GenerateAlignedRandomFloats(VectorCount, 32);
    auto result_z = GenerateAlignedRandomFloats(VectorCount, 32);
    auto result_w = GenerateAlignedRandomFloats(VectorCount, 32);
    
    for (auto _ : state) {
        for (size_t i = 0; i < VectorCount; i += 4) {
            // Process 4 vectors at once using SIMD on each component array
            SimdUtils::Add4f(&data1_x[i], &data2_x[i], &result_x[i]);
            SimdUtils::Add4f(&data1_y[i], &data2_y[i], &result_y[i]);
            SimdUtils::Add4f(&data1_z[i], &data2_z[i], &result_z[i]);
            SimdUtils::Add4f(&data1_w[i], &data2_w[i], &result_w[i]);
        }
        benchmark::DoNotOptimize(result_x);
        benchmark::DoNotOptimize(result_y);
        benchmark::DoNotOptimize(result_z);
        benchmark::DoNotOptimize(result_w);
    }
}
BENCHMARK(BM_DataLayout_SoA)->Name("DataLayout_StructOfArrays");

// Compare different SIMD instruction sets
static void BM_InstructionSet_SSE(benchmark::State& state) {
    if (!SimdUtils::HasSSE()) {
        state.SkipWithError("SSE not supported");
        return;
    }
    
    const size_t VectorCount = 1000000;
    auto data1 = GenerateAlignedRandomFloats(VectorCount * 4, 16);
    auto data2 = GenerateAlignedRandomFloats(VectorCount * 4, 16);
    auto result = GenerateAlignedRandomFloats(VectorCount * 4, 16);
    
    for (auto _ : state) {
        for (size_t i = 0; i < VectorCount * 4; i += 4) {
            SimdUtils::Add4f(&data1[i], &data2[i], &result[i]);
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_InstructionSet_SSE);

static void BM_InstructionSet_AVX(benchmark::State& state) {
    if (!SimdUtils::HasAVX()) {
        state.SkipWithError("AVX not supported");
        return;
    }
    
    const size_t VectorCount = 1000000;
    auto data1 = GenerateAlignedRandomFloats(VectorCount * 8, 32);
    auto data2 = GenerateAlignedRandomFloats(VectorCount * 8, 32);
    auto result = GenerateAlignedRandomFloats(VectorCount * 8, 32);
    
    for (auto _ : state) {
        for (size_t i = 0; i < VectorCount * 8; i += 8) {
            // Process 8 floats at once using AVX
            SimdUtils::MultiplyMatrix4x4(&data1[i], &data2[i], &result[i]);
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_InstructionSet_AVX);

// Benchmark matrix operation optimizations
static void BM_Matrix4x4_SpecialCase(benchmark::State& state) {
    const size_t MatrixCount = 10000;
    auto matrices = GenerateAlignedRandomFloats(MatrixCount * 16, 32);
    auto result = GenerateAlignedRandomFloats(16, 32);
    
    // Create matrices that are special cases (translation, rotation, scale)
    for (size_t i = 0; i < MatrixCount * 16; i += 16) {
        // Make every third matrix a translation matrix
        if ((i/16) % 3 == 0) {
            std::fill(&matrices[i], &matrices[i+16], 0.0f);
            matrices[i+0] = 1.0f;
            matrices[i+5] = 1.0f;
            matrices[i+10] = 1.0f;
            matrices[i+15] = 1.0f;
            matrices[i+3] = (float)rand() / RAND_MAX;
            matrices[i+7] = (float)rand() / RAND_MAX;
            matrices[i+11] = (float)rand() / RAND_MAX;
        }
        // Make every third matrix + 1 a rotation matrix
        else if ((i/16) % 3 == 1) {
            float angle = (float)rand() / RAND_MAX * 3.14159f;
            float c = cos(angle);
            float s = sin(angle);
            std::fill(&matrices[i], &matrices[i+16], 0.0f);
            matrices[i+0] = c;
            matrices[i+1] = -s;
            matrices[i+4] = s;
            matrices[i+5] = c;
            matrices[i+10] = 1.0f;
            matrices[i+15] = 1.0f;
        }
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < MatrixCount * 16; i += 16) {
            SimdUtils::InvertMatrix4x4(&matrices[i], &result[0]);
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix4x4_SpecialCase)->Name("Matrix4x4_SpecialCase_Optimizations");

// Benchmark matrix chain multiplication
static void BM_Matrix4x4_Chain(benchmark::State& state) {
    const size_t ChainLength = state.range(0);
    std::vector<std::array<float, 16>> matrices(ChainLength);
    std::array<float, 16> result;
    
    // Initialize matrices with random data
    for (auto& matrix : matrices) {
        auto temp = GenerateAlignedRandomFloats(16, 32);
        std::copy(temp.begin(), temp.end(), matrix.begin());
    }
    
    for (auto _ : state) {
        // Copy first matrix to result
        std::copy(matrices[0].begin(), matrices[0].end(), result.begin());
        
        // Multiply through the chain
        for (size_t i = 1; i < ChainLength; ++i) {
            std::array<float, 16> temp;
            SimdUtils::MultiplyMatrix4x4(result.data(), matrices[i].data(), temp.data());
            std::copy(temp.begin(), temp.end(), result.begin());
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix4x4_Chain)->Range(2, 8)->Name("Matrix4x4_ChainMultiplication");