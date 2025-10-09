#include <benchmark/benchmark.h>
#include "../../include/matrices/matrix2.hpp"
#include "../../include/matrices/matrix3.hpp"
#include "../../include/matrices/matrix4.hpp"
#include "../../include/vectors/vector2.hpp"
#include "../../include/vectors/vector3.hpp"
#include "../../include/vectors/vector4.hpp"
#include <vector>
#include <random>
#include <memory>
#include <immintrin.h>

namespace {

using namespace PyNovaGE;

template<typename T>
class AlignedAllocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template<typename U>
    struct rebind { using other = AlignedAllocator<U>; };

    AlignedAllocator() noexcept {}
    template<typename U>
    AlignedAllocator(const AlignedAllocator<U>&) noexcept {}

    pointer allocate(size_type n) {
        if (n == 0) return nullptr;
        void* p = _aligned_malloc(n * sizeof(T), 32);
        if (!p) throw std::bad_alloc();
        return static_cast<pointer>(p);
    }

    void deallocate(pointer p, size_type) noexcept {
        _aligned_free(p);
    }
};

template<typename T, typename U>
bool operator==(const AlignedAllocator<T>&, const AlignedAllocator<U>&) noexcept {
    return true;
}

template<typename T, typename U>
bool operator!=(const AlignedAllocator<T>&, const AlignedAllocator<U>&) noexcept {
    return false;
}

// Helper to generate semi-random data that mimics real-world patterns
template<typename T>
class MatrixDataGenerator {
private:
    std::mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<T> random_dis{-100.0, 100.0};
    std::normal_distribution<T> normal_dis{0.0, 1.0};
    std::uniform_real_distribution<T> small_dis{-1.0, 1.0};

public:
    // Matrix3 generators
    Matrix3<T> generateTransformMatrix3() {
        T angle = small_dis(gen) * T(M_PI);
        T scale_x = T(1.0) + small_dis(gen) * T(0.2);
        T scale_y = T(1.0) + small_dis(gen) * T(0.2);
        T tx = small_dis(gen) * T(5.0);
        T ty = small_dis(gen) * T(5.0);
        
        return Matrix3<T>::Translation(tx, ty) *
               Matrix3<T>::Scale(scale_x, scale_y) *
               Matrix3<T>::Rotation(angle);
    }

    Matrix3<T> generateRandomMatrix3() {
        return Matrix3<T>(
            random_dis(gen), random_dis(gen), random_dis(gen),
            random_dis(gen), random_dis(gen), random_dis(gen),
            random_dis(gen), random_dis(gen), random_dis(gen)
        );
    }

    Matrix3<T> generateOrthonormalMatrix3() {
        Matrix3<T> m = generateRandomMatrix3();
        Vector3<T> col1(m[0][0], m[1][0], m[2][0]);
        Vector3<T> col2(m[0][1], m[1][1], m[2][1]);
        Vector3<T> col3(m[0][2], m[1][2], m[2][2]);

        // Gram-Schmidt process
        col1 = col1.normalized();
        col2 = (col2 - col1 * col2.dot(col1)).normalized();
        col3 = (col3 - col1 * col3.dot(col1) - col2 * col3.dot(col2)).normalized();

        return Matrix3<T>(
            col1[0], col2[0], col3[0],
            col1[1], col2[1], col3[1],
            col1[2], col2[2], col3[2]
        );
    }

    // Matrix4 generators
    Matrix4<T> generateTransformMatrix4() {
        T angle = small_dis(gen) * T(M_PI);
        T scale = T(1.0) + small_dis(gen) * T(0.2);
        T tx = small_dis(gen) * T(5.0);
        T ty = small_dis(gen) * T(5.0);
        T tz = small_dis(gen) * T(5.0);
        
        return Matrix4<T>::Translation(tx, ty, tz) *
               Matrix4<T>::Scale(scale, scale, scale) *
               Matrix4<T>::RotationY(angle);
    }

    Matrix4<T> generateViewMatrix4() {
        Vector3<T> eye(random_dis(gen), random_dis(gen), random_dis(gen));
        Vector3<T> target(random_dis(gen), random_dis(gen), random_dis(gen));
        Vector3<T> up(small_dis(gen), T(1.0) + small_dis(gen) * T(0.1), small_dis(gen));
        return Matrix4<T>::LookAt(eye, target, up.normalized());
    }

    Matrix4<T> generateProjectionMatrix4() {
        T fov = T(M_PI) / T(4.0) + small_dis(gen) * T(0.1);
        T aspect = T(16.0) / T(9.0) + small_dis(gen) * T(0.1);
        T near = T(0.1) + std::abs(small_dis(gen)) * T(0.05);
        T far = T(100.0) + std::abs(small_dis(gen)) * T(10.0);
        return Matrix4<T>::Perspective(fov, aspect, near, far);
    }

    Matrix4<T> generateRandomMatrix4() {
        return Matrix4<T>(
            random_dis(gen), random_dis(gen), random_dis(gen), random_dis(gen),
            random_dis(gen), random_dis(gen), random_dis(gen), random_dis(gen),
            random_dis(gen), random_dis(gen), random_dis(gen), random_dis(gen),
            random_dis(gen), random_dis(gen), random_dis(gen), random_dis(gen)
        );
    }
};
}

// Matrix3 Operation Benchmarks

static void BM_Matrix3_Multiplication(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix3<float>, AlignedAllocator<Matrix3<float>>> transforms(N);
    std::vector<Matrix3<float>, AlignedAllocator<Matrix3<float>>> matrices(N);
    std::vector<Matrix3<float>> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        transforms[i] = gen.generateTransformMatrix3();
        matrices[i] = gen.generateRandomMatrix3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = transforms[i] * matrices[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Matrix3_VectorMultiplication(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix3<float>> transforms(N);
    std::vector<Vector3<float>> vectors(N);
    std::vector<Vector3<float>> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        transforms[i] = gen.generateTransformMatrix3();
        vectors[i] = Vector3<float>(gen.generateRandomMatrix3()[0]);  // Use first row as vector
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = transforms[i] * vectors[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Matrix3_Inverse(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix3<float>> matrices(N);
    std::vector<Matrix3<float>> results(N);
    
    // Use orthonormal matrices to ensure they're invertible
    for (size_t i = 0; i < N; ++i) {
        matrices[i] = gen.generateOrthonormalMatrix3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = matrices[i].inverse();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Matrix3_Determinant(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix3<float>> matrices(N);
    std::vector<float> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        matrices[i] = gen.generateRandomMatrix3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = matrices[i].determinant();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// SoA layout for better cache utilization
struct TransformSoA {
    std::vector<float, AlignedAllocator<float>> m00, m01, m02;
    std::vector<float, AlignedAllocator<float>> m10, m11, m12;
    std::vector<float, AlignedAllocator<float>> m20, m21, m22;

    explicit TransformSoA(size_t size) : 
        m00(size), m01(size), m02(size),
        m10(size), m11(size), m12(size),
        m20(size), m21(size), m22(size) {}

    void setFromMatrix3(size_t idx, const Matrix3<float>& mat) {
        m00[idx] = mat[0][0]; m01[idx] = mat[0][1]; m02[idx] = mat[0][2];
        m10[idx] = mat[1][0]; m11[idx] = mat[1][1]; m12[idx] = mat[1][2];
        m20[idx] = mat[2][0]; m21[idx] = mat[2][1]; m22[idx] = mat[2][2];
    }
};

struct Vector3SoA {
    std::vector<float, AlignedAllocator<float>> x, y, z;
    
    explicit Vector3SoA(size_t size) : x(size), y(size), z(size) {}
    
    void setFromVector3(size_t idx, const Vector3<float>& vec) {
        x[idx] = vec[0]; y[idx] = vec[1]; z[idx] = vec[2];
    }
};

static void BM_Matrix3_TypicalFrameUpdate(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    
    // Use SoA layout
    TransformSoA transforms(N);
    Vector3SoA positions(N);
    Vector3SoA velocities(N);
    Vector3SoA results(N);
    
    // Initialize data
    for (size_t i = 0; i < N; ++i) {
        transforms.setFromMatrix3(i, gen.generateTransformMatrix3());
        Vector3f pos = Vector3f(gen.generateRandomMatrix3()[0]);
        Vector3f vel = Vector3f(gen.generateRandomMatrix3()[0]);
        positions.setFromVector3(i, pos);
        velocities.setFromVector3(i, vel);
    }
    
    // Pre-compute rotation matrix values
    const float dt = 0.016667f;  // 60 FPS
    const float angle = dt * 0.5f;  // 30 degrees per second
    const float cos_angle = std::cos(angle);
    const float sin_angle = std::sin(angle);
    
    // Create vectors for SIMD operations
    const __m256 dt_vec = _mm256_set1_ps(dt);
    const __m256 cos_vec = _mm256_set1_ps(cos_angle);
    const __m256 sin_vec = _mm256_set1_ps(sin_angle);
    
    for (auto _ : state) {
        // Process in chunks of 8 for AVX
        for (size_t i = 0; i < N; i += 8) {
            const size_t chunk = std::min(size_t(8), N - i);
            
            // Update positions using SIMD
            for (size_t j = 0; j < chunk; j += 8) {
                __m256 px = _mm256_load_ps(&positions.x[i+j]);
                __m256 py = _mm256_load_ps(&positions.y[i+j]);
                __m256 pz = _mm256_load_ps(&positions.z[i+j]);
                
                __m256 vx = _mm256_load_ps(&velocities.x[i+j]);
                __m256 vy = _mm256_load_ps(&velocities.y[i+j]);
                __m256 vz = _mm256_load_ps(&velocities.z[i+j]);
                
                // p += v * dt
                px = _mm256_fmadd_ps(vx, dt_vec, px);
                py = _mm256_fmadd_ps(vy, dt_vec, py);
                pz = _mm256_fmadd_ps(vz, dt_vec, pz);
                
                _mm256_store_ps(&positions.x[i+j], px);
                _mm256_store_ps(&positions.y[i+j], py);
                _mm256_store_ps(&positions.z[i+j], pz);
            }
            
            // Update transforms using SIMD
            for (size_t j = 0; j < chunk; j += 8) {
                // Load transform rows
                __m256 m00 = _mm256_load_ps(&transforms.m00[i+j]);
                __m256 m01 = _mm256_load_ps(&transforms.m01[i+j]);
                __m256 m02 = _mm256_load_ps(&transforms.m02[i+j]);
                __m256 m10 = _mm256_load_ps(&transforms.m10[i+j]);
                __m256 m11 = _mm256_load_ps(&transforms.m11[i+j]);
                __m256 m12 = _mm256_load_ps(&transforms.m12[i+j]);
                __m256 m20 = _mm256_load_ps(&transforms.m20[i+j]);
                __m256 m21 = _mm256_load_ps(&transforms.m21[i+j]);
                __m256 m22 = _mm256_load_ps(&transforms.m22[i+j]);
                
                // Apply rotation: R * M
                __m256 new_m00 = _mm256_fmadd_ps(cos_vec, m00, _mm256_mul_ps(sin_vec, m10));
                __m256 new_m01 = _mm256_fmadd_ps(cos_vec, m01, _mm256_mul_ps(sin_vec, m11));
                __m256 new_m02 = _mm256_fmadd_ps(cos_vec, m02, _mm256_mul_ps(sin_vec, m12));
                
                __m256 new_m10 = _mm256_fnmadd_ps(sin_vec, m00, _mm256_mul_ps(cos_vec, m10));
                __m256 new_m11 = _mm256_fnmadd_ps(sin_vec, m01, _mm256_mul_ps(cos_vec, m11));
                __m256 new_m12 = _mm256_fnmadd_ps(sin_vec, m02, _mm256_mul_ps(cos_vec, m12));
                
                // Store results
                _mm256_store_ps(&transforms.m00[i+j], new_m00);
                _mm256_store_ps(&transforms.m01[i+j], new_m01);
                _mm256_store_ps(&transforms.m02[i+j], new_m02);
                _mm256_store_ps(&transforms.m10[i+j], new_m10);
                _mm256_store_ps(&transforms.m11[i+j], new_m11);
                _mm256_store_ps(&transforms.m12[i+j], new_m12);
                _mm256_store_ps(&transforms.m20[i+j], m20);
                _mm256_store_ps(&transforms.m21[i+j], m21);
                _mm256_store_ps(&transforms.m22[i+j], m22);
                
                // Transform positions
                __m256 px = _mm256_load_ps(&positions.x[i+j]);
                __m256 py = _mm256_load_ps(&positions.y[i+j]);
                __m256 pz = _mm256_load_ps(&positions.z[i+j]);
                
                __m256 rx = _mm256_fmadd_ps(m00, px, _mm256_fmadd_ps(m01, py, _mm256_mul_ps(m02, pz)));
                __m256 ry = _mm256_fmadd_ps(m10, px, _mm256_fmadd_ps(m11, py, _mm256_mul_ps(m12, pz)));
                __m256 rz = _mm256_fmadd_ps(m20, px, _mm256_fmadd_ps(m21, py, _mm256_mul_ps(m22, pz)));
                
                _mm256_store_ps(&results.x[i+j], rx);
                _mm256_store_ps(&results.y[i+j], ry);
                _mm256_store_ps(&results.z[i+j], rz);
            }
        }
        benchmark::DoNotOptimize(results.x[0]);
        benchmark::DoNotOptimize(transforms.m00[0]);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// Matrix4 Operation Benchmarks

static void BM_Matrix4_Multiplication(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix4<float>, AlignedAllocator<Matrix4<float>>> transforms(N);
    std::vector<Matrix4<float>, AlignedAllocator<Matrix4<float>>> matrices(N);
    std::vector<Matrix4<float>> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        transforms[i] = gen.generateTransformMatrix4();
        matrices[i] = gen.generateRandomMatrix4();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = transforms[i] * matrices[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Matrix4_VectorMultiplication(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix4<float>> transforms(N);
    std::vector<Vector4<float>> vectors(N);
    std::vector<Vector4<float>> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        transforms[i] = gen.generateTransformMatrix4();
        vectors[i] = Vector4f(gen.generateRandomMatrix4()[0]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = transforms[i] * vectors[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Matrix4_Inverse(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix4<float>> matrices(N);
    std::vector<Matrix4<float>> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        matrices[i] = gen.generateViewMatrix4();  // View matrices are always invertible
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = matrices[i].inverse();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Matrix4_ViewProjection(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix4<float>> views(N);
    std::vector<Matrix4<float>> projections(N);
    std::vector<Vector4<float>> positions(N);
    std::vector<Vector4<float>> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        views[i] = gen.generateViewMatrix4();
        projections[i] = gen.generateProjectionMatrix4();
        positions[i] = Vector4f(gen.generateRandomMatrix4()[0]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            // Typical view-projection transform of a position
            Matrix4<float> vp = projections[i] * views[i];
            results[i] = vp * positions[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Matrix4_TypicalFrameUpdate(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix4<float>> models(N);
    std::vector<Matrix4<float>> views(N);
    std::vector<Matrix4<float>> projections(N);
    std::vector<Vector4<float>> positions(N);
    const float dt = 0.016667f;  // 60 FPS
    
    for (size_t i = 0; i < N; ++i) {
        models[i] = gen.generateTransformMatrix4();
        views[i] = gen.generateViewMatrix4();
        projections[i] = gen.generateProjectionMatrix4();
        positions[i] = Vector4f(gen.generateRandomMatrix4()[0]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            // Update model transform with rotation
            float angle = dt * 0.5f;  // 30 degrees per second
Matrix4<float> rotation = Matrix4<float>::RotationY(angle);
            models[i] = rotation * models[i];
            
            // Compute MVP and transform position
Matrix4<float> mvp = projections[i] * views[i] * models[i];
            Vector4<float> transformed = mvp * positions[i];
            
            benchmark::DoNotOptimize(transformed);
            benchmark::DoNotOptimize(models[i]);
        }
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// Register benchmarks with different sizes
BENCHMARK(BM_Matrix3_Multiplication)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Matrix3_VectorMultiplication)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Matrix3_Inverse)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Matrix3_Determinant)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Matrix3_TypicalFrameUpdate)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Matrix4_Multiplication)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Matrix4_VectorMultiplication)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Matrix4_Inverse)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Matrix4_ViewProjection)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Matrix4_TypicalFrameUpdate)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);