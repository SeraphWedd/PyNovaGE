#include <benchmark/benchmark.h>
#include "../include/matrix4.hpp"
#include "../include/vector4.hpp"
#include "../include/vector3.hpp"
#include "../include/simd_utils.hpp"
#include "../include/quaternion.hpp"
#include <vector>
#include <random>
#include <array>

using namespace pynovage::math;
using namespace pynovage::math::constants;

// Constants for test data sizes
const size_t SMALL_SIZE = 1000;
const size_t MEDIUM_SIZE = 10000;
const size_t LARGE_SIZE = 100000;

// Utility function to generate random floating point numbers
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

// Utility function to generate random normalized vectors
static std::vector<Vector3> GenerateRandomNormalizedVectors(size_t count) {
    auto random_floats = GenerateRandomFloats(count * 3);
    std::vector<Vector3> result;
    result.reserve(count);
    
    for (size_t i = 0; i < count * 3; i += 3) {
        Vector3 vec(random_floats[i], random_floats[i+1], random_floats[i+2]);
        result.push_back(vec.normalized());
    }
    return result;
}

// Utility function to generate random matrices
static std::vector<Matrix4> GenerateRandomMatrices(size_t count) {
    auto random_floats = GenerateRandomFloats(count * 16);
    std::vector<Matrix4> matrices;
    matrices.reserve(count);
    
    for (size_t i = 0; i < count * 16; i += 16) {
        matrices.emplace_back(
            random_floats[i],    random_floats[i+1],  random_floats[i+2],  random_floats[i+3],
            random_floats[i+4],  random_floats[i+5],  random_floats[i+6],  random_floats[i+7],
            random_floats[i+8],  random_floats[i+9],  random_floats[i+10], random_floats[i+11],
            random_floats[i+12], random_floats[i+13], random_floats[i+14], random_floats[i+15]
        );
    }
    return matrices;
}

// Basic Matrix4 Construction Benchmarks
static void BM_Matrix4DefaultConstruction(benchmark::State& state) {
    for (auto _ : state) {
        Matrix4 m;
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(BM_Matrix4DefaultConstruction);

static void BM_Matrix4CopyConstruction(benchmark::State& state) {
    Matrix4 source = Matrix4::translation(1.0f, 2.0f, 3.0f);
    for (auto _ : state) {
        Matrix4 copy(source);
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(BM_Matrix4CopyConstruction);

static void BM_Matrix4ValueConstruction(benchmark::State& state) {
    auto random_floats = GenerateRandomFloats(16);
    for (auto _ : state) {
        Matrix4 m(
            random_floats[0],  random_floats[1],  random_floats[2],  random_floats[3],
            random_floats[4],  random_floats[5],  random_floats[6],  random_floats[7],
            random_floats[8],  random_floats[9],  random_floats[10], random_floats[11],
            random_floats[12], random_floats[13], random_floats[14], random_floats[15]
        );
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(BM_Matrix4ValueConstruction);

// Matrix-Matrix Multiplication Benchmarks
static void BM_Matrix4Multiplication(benchmark::State& state) {
    const size_t MatrixCount = state.range(0);
    auto matrices = GenerateRandomMatrices(MatrixCount);
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 result = matrices[index % (MatrixCount-1)] * matrices[(index + 1) % (MatrixCount-1)];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4Multiplication)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE);

static void BM_Matrix4MultiplicationChain(benchmark::State& state) {
    const size_t ChainLength = state.range(0);
    auto matrices = GenerateRandomMatrices(ChainLength);
    Matrix4 result;
    
    for (auto _ : state) {
        result = matrices[0];
        for (size_t i = 1; i < ChainLength; ++i) {
            result = result * matrices[i];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix4MultiplicationChain)->Range(2, 8);

static void BM_Matrix4MultiplicationWithIdentity(benchmark::State& state) {
    const size_t MatrixCount = state.range(0);
    auto matrices = GenerateRandomMatrices(MatrixCount/2);
    std::vector<Matrix4> test_matrices;
    test_matrices.reserve(MatrixCount);
    
    // Alternate between random matrices and identity
    Matrix4 identity;
    for (const auto& m : matrices) {
        test_matrices.push_back(m);
        test_matrices.push_back(identity);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 result = test_matrices[index % (MatrixCount-1)] * test_matrices[(index + 1) % (MatrixCount-1)];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4MultiplicationWithIdentity)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);

// Matrix-Vector Operations
static void BM_Matrix4Vector4Multiplication(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto random_floats = GenerateRandomFloats(Count * 4);  // For vectors
    auto matrices = GenerateRandomMatrices(Count);
    std::vector<Vector4> vectors;
    vectors.reserve(Count);
    
    for (size_t i = 0; i < Count * 4; i += 4) {
        vectors.emplace_back(random_floats[i], random_floats[i+1], random_floats[i+2], random_floats[i+3]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector4 result = matrices[index % Count] * vectors[index % Count];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4Vector4Multiplication)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE);

static void BM_Matrix4PointTransformation(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto matrices = GenerateRandomMatrices(Count);
    auto random_floats = GenerateRandomFloats(Count * 3);
    std::vector<Vector3> points;
    points.reserve(Count);
    
    for (size_t i = 0; i < Count * 3; i += 3) {
        points.emplace_back(random_floats[i], random_floats[i+1], random_floats[i+2]);
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Vector3 result = matrices[index % Count].transformPoint(points[index % Count]);
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4PointTransformation)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);

static void BM_Matrix4VectorTransformBatch(benchmark::State& state) {
    const size_t BatchSize = 1024;  // Fixed batch size for cache-friendly operations
    const size_t Count = state.range(0);
    auto matrices = GenerateRandomMatrices(Count);
    auto random_floats = GenerateRandomFloats(BatchSize * 4);
    std::vector<Vector4> vectors;
    vectors.reserve(BatchSize);
    std::vector<Vector4> results(BatchSize);
    
    for (size_t i = 0; i < BatchSize * 4; i += 4) {
        vectors.emplace_back(random_floats[i], random_floats[i+1], random_floats[i+2], random_floats[i+3]);
    }
    
    size_t matrix_index = 0;
    for (auto _ : state) {
        const Matrix4& current_matrix = matrices[matrix_index % Count];
        for (size_t i = 0; i < BatchSize; ++i) {
            results[i] = current_matrix * vectors[i];
        }
        benchmark::DoNotOptimize(results);
        matrix_index++;
    }
}
BENCHMARK(BM_Matrix4VectorTransformBatch)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);

// Basic Transformation Matrix Creation Benchmarks
static void BM_Matrix4Translation(benchmark::State& state) {
    auto random_floats = GenerateRandomFloats(3);
    for (auto _ : state) {
        Matrix4 m = Matrix4::translation(random_floats[0], random_floats[1], random_floats[2]);
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(BM_Matrix4Translation);

static void BM_Matrix4Scale(benchmark::State& state) {
    auto random_floats = GenerateRandomFloats(3);
    for (auto _ : state) {
        Matrix4 m = Matrix4::scale(random_floats[0], random_floats[1], random_floats[2]);
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(BM_Matrix4Scale);

static void BM_Matrix4RotationX(benchmark::State& state) {
    std::vector<float> angles = GenerateRandomFloats(1000);
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 rot = Matrix4::rotationX(angles[index % 1000]);
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix4RotationX);

static void BM_Matrix4RotationY(benchmark::State& state) {
    std::vector<float> angles = GenerateRandomFloats(1000);
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 rot = Matrix4::rotationY(angles[index % 1000]);
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix4RotationY);

static void BM_Matrix4RotationZ(benchmark::State& state) {
    std::vector<float> angles = GenerateRandomFloats(1000);
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 rot = Matrix4::rotationZ(angles[index % 1000]);
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix4RotationZ);

// Complex Transformation Matrix Creation Benchmarks
static void BM_Matrix4AxisAngleRotation(benchmark::State& state) {
    const size_t Count = 1000;
    std::vector<float> angles = GenerateRandomFloats(Count);
    auto axes = GenerateRandomNormalizedVectors(Count);
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 rot = Matrix4::rotationAxis(axes[index % Count], angles[index % Count]);
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix4AxisAngleRotation);

static void BM_Matrix4EulerAngles(benchmark::State& state) {
    std::vector<float> angles = GenerateRandomFloats(3000);  // For yaw, pitch, roll sets
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 rot = Matrix4::fromEulerAngles(
            angles[index % 3000],
            angles[(index + 1) % 3000],
            angles[(index + 2) % 3000]
        );
        benchmark::DoNotOptimize(rot);
        index++;
    }
}
BENCHMARK(BM_Matrix4EulerAngles);

static void BM_Matrix4LookAt(benchmark::State& state) {
    const size_t Count = 1000;
    auto random_points = GenerateRandomFloats(Count * 9);  // For eye, target, up vectors
    std::vector<Vector3> eyes, targets, ups;
    eyes.reserve(Count);
    targets.reserve(Count);
    ups.reserve(Count);
    
    for (size_t i = 0; i < Count * 9; i += 9) {
        eyes.emplace_back(random_points[i], random_points[i+1], random_points[i+2]);
        targets.emplace_back(random_points[i+3], random_points[i+4], random_points[i+5]);
        Vector3 up(random_points[i+6], random_points[i+7], random_points[i+8]);
        ups.push_back(up.normalized());  // Ensure normalized up vector
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix4 view = Matrix4::lookAt(eyes[index % Count], targets[index % Count], ups[index % Count]);
        benchmark::DoNotOptimize(view);
        index++;
    }
}
BENCHMARK(BM_Matrix4LookAt);

static void BM_Matrix4Perspective(benchmark::State& state) {
    std::vector<float> params = GenerateRandomFloats(1000);
    size_t index = 0;
    for (auto _ : state) {
        float fov = std::abs(params[index % 1000]) * 0.5f;  // Keep FOV reasonable
        float aspect = 16.0f/9.0f;  // Common aspect ratio
        float near = 0.1f + std::abs(params[(index + 1) % 1000]) * 0.1f;  // Small positive near plane
        float far = near + 10.0f + std::abs(params[(index + 2) % 1000]) * 90.0f;  // Ensure far > near
        
        Matrix4 proj = Matrix4::perspective(fov, aspect, near, far);
        benchmark::DoNotOptimize(proj);
        index++;
    }
}
BENCHMARK(BM_Matrix4Perspective);

static void BM_Matrix4Orthographic(benchmark::State& state) {
    std::vector<float> params = GenerateRandomFloats(1000);
    size_t index = 0;
    for (auto _ : state) {
        float size = std::abs(params[index % 1000]) + 1.0f;
        float near = 0.1f + std::abs(params[(index + 1) % 1000]) * 0.1f;
        float far = near + 10.0f + std::abs(params[(index + 2) % 1000]) * 90.0f;
        
        Matrix4 ortho = Matrix4::orthographic(
            -size, size,    // left, right
            -size, size,    // bottom, top
            near, far       // near, far
        );
        benchmark::DoNotOptimize(ortho);
        index++;
    }
}
BENCHMARK(BM_Matrix4Orthographic);

// Performance Characteristic Benchmarks

// Access Pattern Benchmarks
static void BM_Matrix4SequentialMultiplication(benchmark::State& state) {
    const size_t MatrixCount = 1024;  // Cache-friendly size
    std::vector<Matrix4> matrices(MatrixCount);
    std::vector<Matrix4> results(MatrixCount);
    
    auto random_floats = GenerateRandomFloats(MatrixCount * 16);
    for (size_t i = 0; i < MatrixCount; ++i) {
        matrices[i] = Matrix4(
            random_floats[i*16],   random_floats[i*16+1], random_floats[i*16+2], random_floats[i*16+3],
            random_floats[i*16+4], random_floats[i*16+5], random_floats[i*16+6], random_floats[i*16+7],
            random_floats[i*16+8], random_floats[i*16+9], random_floats[i*16+10], random_floats[i*16+11],
            random_floats[i*16+12], random_floats[i*16+13], random_floats[i*16+14], random_floats[i*16+15]
        );
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < MatrixCount-1; ++i) {
            results[i] = matrices[i] * matrices[i+1];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_Matrix4SequentialMultiplication);

static void BM_Matrix4RandomAccessMultiplication(benchmark::State& state) {
    const size_t MatrixCount = 1024;
    std::vector<Matrix4> matrices(MatrixCount);
    std::vector<Matrix4> results(MatrixCount);
    std::vector<size_t> indices(MatrixCount);
    
    auto random_floats = GenerateRandomFloats(MatrixCount * 16);
    for (size_t i = 0; i < MatrixCount; ++i) {
        matrices[i] = Matrix4(
            random_floats[i*16],   random_floats[i*16+1], random_floats[i*16+2], random_floats[i*16+3],
            random_floats[i*16+4], random_floats[i*16+5], random_floats[i*16+6], random_floats[i*16+7],
            random_floats[i*16+8], random_floats[i*16+9], random_floats[i*16+10], random_floats[i*16+11],
            random_floats[i*16+12], random_floats[i*16+13], random_floats[i*16+14], random_floats[i*16+15]
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
BENCHMARK(BM_Matrix4RandomAccessMultiplication);

// Cache Performance Benchmarks
static void BM_Matrix4SmallChainMultiplication(benchmark::State& state) {
    const size_t ChainLength = 4;  // Small chain that should fit in L1 cache
    auto matrices = GenerateRandomMatrices(ChainLength);
    Matrix4 result;
    
    for (auto _ : state) {
        result = matrices[0];
        for (size_t i = 1; i < ChainLength; ++i) {
            result = result * matrices[i];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix4SmallChainMultiplication);

static void BM_Matrix4LargeChainMultiplication(benchmark::State& state) {
    const size_t ChainLength = 64;  // Large chain that should exceed cache size
    auto matrices = GenerateRandomMatrices(ChainLength);
    Matrix4 result;
    
    for (auto _ : state) {
        result = matrices[0];
        for (size_t i = 1; i < ChainLength; ++i) {
            result = result * matrices[i];
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix4LargeChainMultiplication);

template<size_t Alignment>
static void BM_Matrix4CacheAlignedOperations(benchmark::State& state) {
    const size_t Count = 1024;
    alignas(Alignment) std::array<Matrix4, Count> matrices;
    alignas(Alignment) std::array<Matrix4, Count> results;
    
    auto random_floats = GenerateRandomFloats(Count * 16);
    for (size_t i = 0; i < Count; ++i) {
        matrices[i] = Matrix4(
            random_floats[i*16],   random_floats[i*16+1], random_floats[i*16+2], random_floats[i*16+3],
            random_floats[i*16+4], random_floats[i*16+5], random_floats[i*16+6], random_floats[i*16+7],
            random_floats[i*16+8], random_floats[i*16+9], random_floats[i*16+10], random_floats[i*16+11],
            random_floats[i*16+12], random_floats[i*16+13], random_floats[i*16+14], random_floats[i*16+15]
        );
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count-1; ++i) {
            results[i] = matrices[i] * matrices[i+1];
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK_TEMPLATE(BM_Matrix4CacheAlignedOperations, 4)->Name("Matrix4_Alignment_4bytes");
BENCHMARK_TEMPLATE(BM_Matrix4CacheAlignedOperations, 8)->Name("Matrix4_Alignment_8bytes");
BENCHMARK_TEMPLATE(BM_Matrix4CacheAlignedOperations, 16)->Name("Matrix4_Alignment_16bytes");
BENCHMARK_TEMPLATE(BM_Matrix4CacheAlignedOperations, 32)->Name("Matrix4_Alignment_32bytes");

// Special Case Optimizations
static void BM_Matrix4IdentityOptimization(benchmark::State& state) {
    const size_t Count = 1000;
    std::vector<Matrix4> matrices;
    matrices.reserve(Count);
    Matrix4 result;
    
    // Create a mix of identity and non-identity matrices
    auto random_floats = GenerateRandomFloats(Count * 16);
    for (size_t i = 0; i < Count; ++i) {
        if (i % 2 == 0) {
            matrices.push_back(Matrix4());  // Identity matrix
        } else {
            matrices.push_back(Matrix4(
                random_floats[i*16],   random_floats[i*16+1], random_floats[i*16+2], random_floats[i*16+3],
                random_floats[i*16+4], random_floats[i*16+5], random_floats[i*16+6], random_floats[i*16+7],
                random_floats[i*16+8], random_floats[i*16+9], random_floats[i*16+10], random_floats[i*16+11],
                random_floats[i*16+12], random_floats[i*16+13], random_floats[i*16+14], random_floats[i*16+15]
            ));
        }
    }
    
    size_t index = 0;
    for (auto _ : state) {
        result = matrices[index % (Count-1)] * matrices[(index + 1) % (Count-1)];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4IdentityOptimization);

static void BM_Matrix4ScaleOptimization(benchmark::State& state) {
    const size_t Count = 1000;
    std::vector<Matrix4> matrices;
    matrices.reserve(Count);
    Matrix4 result;
    
    auto random_floats = GenerateRandomFloats(Count * 3);  // For scale factors
    
    // Create a mix of scale and random matrices
    for (size_t i = 0; i < Count; ++i) {
        if (i % 2 == 0) {
            matrices.push_back(Matrix4::scale(
                random_floats[i*3],
                random_floats[i*3+1],
                random_floats[i*3+2]
            ));
        } else {
            matrices.push_back(GenerateRandomMatrices(1)[0]);
        }
    }
    
    size_t index = 0;
    for (auto _ : state) {
        result = matrices[index % (Count-1)] * matrices[(index + 1) % (Count-1)];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4ScaleOptimization);

static void BM_Matrix4RotationOptimization(benchmark::State& state) {
    const size_t Count = 1000;
    std::vector<Matrix4> matrices;
    matrices.reserve(Count);
    Matrix4 result;
    
    auto angles = GenerateRandomFloats(Count);
    
    // Create a mix of rotation and random matrices
    for (size_t i = 0; i < Count; ++i) {
        switch (i % 4) {
            case 0: matrices.push_back(Matrix4::rotationX(angles[i])); break;
            case 1: matrices.push_back(Matrix4::rotationY(angles[i])); break;
            case 2: matrices.push_back(Matrix4::rotationZ(angles[i])); break;
            case 3: matrices.push_back(GenerateRandomMatrices(1)[0]); break;
        }
    }
    
    size_t index = 0;
    for (auto _ : state) {
        result = matrices[index % (Count-1)] * matrices[(index + 1) % (Count-1)];
        benchmark::DoNotOptimize(result);
        index++;
    }
}
BENCHMARK(BM_Matrix4RotationOptimization);

// Numerical Operation Benchmarks

// Core Operations
static void BM_Matrix4Determinant(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto matrices = GenerateRandomMatrices(Count);
    float det = 0.0f;
    
    size_t index = 0;
    for (auto _ : state) {
        det = matrices[index % Count].determinant();
        benchmark::DoNotOptimize(det);
        index++;
    }
}
BENCHMARK(BM_Matrix4Determinant)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE);

static void BM_Matrix4Inverse(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto random_floats = GenerateRandomFloats(Count * 16);
    std::vector<Matrix4> matrices;
    matrices.reserve(Count);
    
    // Generate invertible matrices
    for (size_t i = 0; i < Count * 16; i += 16) {
        Matrix4 m(
            random_floats[i],    random_floats[i+1],  random_floats[i+2],  random_floats[i+3],
            random_floats[i+4],  random_floats[i+5],  random_floats[i+6],  random_floats[i+7],
            random_floats[i+8],  random_floats[i+9],  random_floats[i+10], random_floats[i+11],
            random_floats[i+12], random_floats[i+13], random_floats[i+14], random_floats[i+15]
        );
        if (std::abs(m.determinant()) > 1e-6f) {
            matrices.push_back(m);
        }
    }
    
    Matrix4 inverse;
    size_t index = 0;
    for (auto _ : state) {
        bool success = matrices[index % matrices.size()].getInverse(inverse);
        benchmark::DoNotOptimize(inverse);
        benchmark::DoNotOptimize(success);
        index++;
    }
}
BENCHMARK(BM_Matrix4Inverse)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);

// Decomposition Operations
static void BM_Matrix4DecomposeTranslation(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto matrices = GenerateRandomMatrices(Count);
    Vector3 translation;
    
    size_t index = 0;
    for (auto _ : state) {
        translation = matrices[index % Count].extractTranslation();
        benchmark::DoNotOptimize(translation);
        index++;
    }
}
BENCHMARK(BM_Matrix4DecomposeTranslation)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);

static void BM_Matrix4DecomposeRotation(benchmark::State& state) {
    const size_t Count = state.range(0);
    std::vector<Matrix4> matrices;
    matrices.reserve(Count);
    
    // Generate matrices with significant rotation components
    auto angles = GenerateRandomFloats(Count * 3);
    for (size_t i = 0; i < Count; ++i) {
        Matrix4 rot = Matrix4::fromEulerAngles(
            angles[i*3], angles[i*3+1], angles[i*3+2]
        );
        matrices.push_back(rot);
    }
    
    Quaternion rotation;
    size_t index = 0;
    for (auto _ : state) {
        rotation = matrices[index % Count].extractRotation();
        benchmark::DoNotOptimize(rotation);
        index++;
    }
}
BENCHMARK(BM_Matrix4DecomposeRotation)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);

static void BM_Matrix4DecomposeScale(benchmark::State& state) {
    const size_t Count = state.range(0);
    std::vector<Matrix4> matrices;
    matrices.reserve(Count);
    
    // Generate matrices with various scale components
    auto scales = GenerateRandomFloats(Count * 3);
    for (size_t i = 0; i < Count; ++i) {
        Matrix4 scale = Matrix4::scale(
            std::abs(scales[i*3] + 1.0f),
            std::abs(scales[i*3+1] + 1.0f),
            std::abs(scales[i*3+2] + 1.0f)
        );
        matrices.push_back(scale);
    }
    
    Vector3 scale_result;
    size_t index = 0;
    for (auto _ : state) {
        scale_result = matrices[index % Count].extractScale();
        benchmark::DoNotOptimize(scale_result);
        index++;
    }
}
BENCHMARK(BM_Matrix4DecomposeScale)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);

static void BM_Matrix4DecomposeFull(benchmark::State& state) {
    const size_t Count = state.range(0);
    std::vector<Matrix4> matrices;
    matrices.reserve(Count);
    
    // Generate matrices with translation, rotation, and scale
    auto random_data = GenerateRandomFloats(Count * 7);  // 3 for translation, 3 for euler angles, 1 for uniform scale
    for (size_t i = 0; i < Count; ++i) {
        float scale_factor = std::abs(random_data[i*7+6]) + 1.0f;
        Matrix4 transform = 
            Matrix4::scale(scale_factor, scale_factor, scale_factor) *
            Matrix4::fromEulerAngles(random_data[i*7+3], random_data[i*7+4], random_data[i*7+5]) *
            Matrix4::translation(random_data[i*7], random_data[i*7+1], random_data[i*7+2]);
        matrices.push_back(transform);
    }
    
    Vector3 translation, scale;
    Quaternion rotation;
    size_t index = 0;
    for (auto _ : state) {
        translation = matrices[index % Count].extractTranslation();
        rotation = matrices[index % Count].extractRotation();
        scale = matrices[index % Count].extractScale();
        benchmark::DoNotOptimize(translation);
        benchmark::DoNotOptimize(rotation);
        benchmark::DoNotOptimize(scale);
        index++;
    }
}
BENCHMARK(BM_Matrix4DecomposeFull)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);

// SIMD vs Scalar Comparison Benchmarks

#ifdef PYNOVAGE_USE_SIMD
// SIMD Matrix Multiplication
static void BM_Matrix4MultiplicationSIMD(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto m1_data = GenerateRandomFloats(Count * 16);
    auto m2_data = GenerateRandomFloats(Count * 16);
    auto result_data = std::vector<float>(Count * 16);
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count * 16; i += 16) {
            SimdUtils::MultiplyMatrix4(&m1_data[i], &m2_data[i], &result_data[i]);
        }
        benchmark::DoNotOptimize(result_data);
    }
}
BENCHMARK(BM_Matrix4MultiplicationSIMD)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE);
#endif

// Scalar Matrix Multiplication
static void BM_Matrix4MultiplicationScalar(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto m1_data = GenerateRandomFloats(Count * 16);
    auto m2_data = GenerateRandomFloats(Count * 16);
    auto result_data = std::vector<float>(Count * 16);
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count * 16; i += 16) {
            // Manual scalar multiplication
            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 4; ++col) {
                    float sum = 0.0f;
                    for (int k = 0; k < 4; ++k) {
                        sum += m1_data[i + row*4 + k] * m2_data[i + k*4 + col];
                    }
                    result_data[i + row*4 + col] = sum;
                }
            }
        }
        benchmark::DoNotOptimize(result_data);
    }
}
BENCHMARK(BM_Matrix4MultiplicationScalar)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE);

#ifdef PYNOVAGE_USE_SIMD
// SIMD Vector Transformation
static void BM_Matrix4TransformationSIMD(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto matrix_data = GenerateRandomFloats(16);  // Single matrix
    auto vector_data = GenerateRandomFloats(Count * 4);
    auto result_data = std::vector<float>(Count * 4);
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count * 4; i += 4) {
            SimdUtils::TransformVector4(&matrix_data[0], &vector_data[i], &result_data[i]);
        }
        benchmark::DoNotOptimize(result_data);
    }
}
BENCHMARK(BM_Matrix4TransformationSIMD)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE);
#endif

// Scalar Vector Transformation
static void BM_Matrix4TransformationScalar(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto matrix_data = GenerateRandomFloats(16);  // Single matrix
    auto vector_data = GenerateRandomFloats(Count * 4);
    auto result_data = std::vector<float>(Count * 4);
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count * 4; i += 4) {
            // Manual scalar vector transformation
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int col = 0; col < 4; ++col) {
                    sum += matrix_data[row*4 + col] * vector_data[i + col];
                }
                result_data[i + row] = sum;
            }
        }
        benchmark::DoNotOptimize(result_data);
    }
}
BENCHMARK(BM_Matrix4TransformationScalar)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE);

#ifdef PYNOVAGE_USE_SIMD
// SIMD Matrix Inverse
static void BM_Matrix4InverseSIMD(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto matrix_data = GenerateRandomFloats(Count * 16);
    auto result_data = std::vector<float>(Count * 16);
    std::vector<bool> success(Count);
    
    for (auto _ : state) {
        for (size_t i = 0; i < Count * 16; i += 16) {
            success[i/16] = SimdUtils::InvertMatrix4(&matrix_data[i], &result_data[i]);
        }
        benchmark::DoNotOptimize(result_data);
        benchmark::DoNotOptimize(success);
    }
}
BENCHMARK(BM_Matrix4InverseSIMD)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);
#endif

// Scalar Matrix Inverse
static void BM_Matrix4InverseScalar(benchmark::State& state) {
    const size_t Count = state.range(0);
    auto matrices = GenerateRandomMatrices(Count);
    Matrix4 inverse;
    
    size_t index = 0;
    for (auto _ : state) {
        bool success = matrices[index % Count].getInverse(inverse);
        benchmark::DoNotOptimize(inverse);
        benchmark::DoNotOptimize(success);
        index++;
    }
}
BENCHMARK(BM_Matrix4InverseScalar)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE);

// Hardware and Compiler Information Comments
/*
These benchmarks are designed to compare SIMD vs scalar performance.

Hardware Requirements:
- CPU with SIMD support (SSE2, AVX, etc.)
- Sufficient L1/L2 cache for matrix operations

Compiler Flags:
- Enable SIMD instructions: -DPYNOVAGE_USE_SIMD
- Optimization level: -O2 or higher recommended
- Architecture-specific flags: -msse2, -mavx, etc.

Note: SIMD performance may vary significantly based on:
- Hardware architecture
- Memory alignment
- Compiler optimization settings
- Data size and cache utilization
*/

// Hardware and Runtime Notes
/*
These benchmarks should be run with:
- Release build for accurate timings
- Appropriate SIMD instruction set enabled (-msse2, -mavx, etc.)
- Sufficient warmup and iteration count (handled by Google Benchmark)
- Minimal background CPU load for consistent results
*/
