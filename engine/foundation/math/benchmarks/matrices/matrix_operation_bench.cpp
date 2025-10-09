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

static void BM_Matrix3_TypicalFrameUpdate(benchmark::State& state) {
    const size_t N = state.range(0);
    MatrixDataGenerator<float> gen;
    std::vector<Matrix3<float>> transforms(N);
    std::vector<Vector3<float>> positions(N);
    std::vector<Vector3<float>> velocities(N);
    const float dt = 0.016667f;  // 60 FPS
    
    for (size_t i = 0; i < N; ++i) {
        transforms[i] = gen.generateTransformMatrix3();
        positions[i] = Vector3f(gen.generateRandomMatrix3()[0]);
        velocities[i] = Vector3f(gen.generateRandomMatrix3()[0]);
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            // Update position
            positions[i] += velocities[i] * dt;
            
            // Apply transform
Vector3<float> transformed = transforms[i] * positions[i];
            
            // Update transform with slight rotation
            float angle = dt * 0.5f;  // 30 degrees per second
Matrix3<float> rotation = Matrix3<float>::Rotation(angle);
            transforms[i] = rotation * transforms[i];
            
            benchmark::DoNotOptimize(transformed);
            benchmark::DoNotOptimize(transforms[i]);
        }
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