#include <benchmark/benchmark.h>
#include "../../include/vectors/vectors.hpp"
#include <vector>
#include <random>
#include <algorithm>

using namespace PyNovaGE;

namespace {
// Helper to generate semi-random data that mimics real-world patterns
template<typename T>
class VectorDataGenerator {
private:
    std::mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<T> random_dis{-100.0, 100.0};
    std::normal_distribution<T> normal_dis{0.0, 1.0};
    std::uniform_real_distribution<T> small_dis{-1.0, 1.0};

public:
    // Vector2 generators
    Vector2<T> generateMovementVector2() {
        return Vector2<T>(
            small_dis(gen),
            small_dis(gen)
        );
    }

    Vector2<T> generatePositionVector2() {
        return Vector2<T>(
            random_dis(gen),
            random_dis(gen)
        );
    }

    Vector2<T> generateNormalizedVector2() {
        Vector2<T> v(
            normal_dis(gen),
            normal_dis(gen)
        );
        return v.normalized();
    }

    // Vector3 generators
    Vector3<T> generateMovementVector3() {
        return Vector3<T>(
            small_dis(gen),
            small_dis(gen),
            small_dis(gen)
        );
    }

    Vector3<T> generatePositionVector3() {
        return Vector3<T>(
            random_dis(gen),
            random_dis(gen),
            random_dis(gen)
        );
    }

    Vector3<T> generateNormalizedVector3() {
        Vector3<T> v(
            normal_dis(gen),
            normal_dis(gen),
            normal_dis(gen)
        );
        return v.normalized();
    }

    // Vector4 generators
    Vector4<T> generateMovementVector4() {
        return Vector4<T>(
            small_dis(gen),
            small_dis(gen),
            small_dis(gen),
            small_dis(gen)
        );
    }

    Vector4<T> generatePositionVector4() {
        return Vector4<T>(
            random_dis(gen),
            random_dis(gen),
            random_dis(gen),
            random_dis(gen)
        );
    }

    Vector4<T> generateNormalizedVector4() {
        Vector4<T> v(
            normal_dis(gen),
            normal_dis(gen),
            normal_dis(gen),
            normal_dis(gen)
        );
        return v.normalized();
    }
};
}

// Vector3 Operation Benchmarks

// Vector2 Operation Benchmarks

static void BM_Vector2_Addition(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector2f> positions(N);
    std::vector<Vector2f> velocities(N);
    std::vector<Vector2f> results(N);
    
    // Initialize with realistic position and velocity data
    for (size_t i = 0; i < N; ++i) {
        positions[i] = gen.generatePositionVector2();
        velocities[i] = gen.generateMovementVector2();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = positions[i] + velocities[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector2_ScalarMultiply(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector2f> vectors(N);
    std::vector<Vector2f> results(N);
    const float scalar = 0.016667f; // Typical dt value (1/60)
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generateMovementVector2();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i] * scalar;
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector2_DotProduct(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector2f> normals(N);
    std::vector<Vector2f> directions(N);
    std::vector<float> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        normals[i] = gen.generateNormalizedVector2();
        directions[i] = gen.generateNormalizedVector2();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = normals[i].dot(directions[i]);
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector2_Normalize(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector2f> vectors(N);
    std::vector<Vector2f> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generatePositionVector2();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i].normalized();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector2_Length(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector2f> vectors(N);
    std::vector<float> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generatePositionVector2();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i].length();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector2_TypicalFrameUpdate(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector2f> positions(N);
    std::vector<Vector2f> velocities(N);
    std::vector<Vector2f> forces(N);
    const float dt = 0.016667f; // 60 FPS
    const Vector2f gravity(0.0f, -9.81f);
    
    for (size_t i = 0; i < N; ++i) {
        positions[i] = gen.generatePositionVector2();
        velocities[i] = gen.generateMovementVector2();
        forces[i] = gen.generateMovementVector2();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            forces[i] += gravity;
            velocities[i] += forces[i] * dt;
            positions[i] += velocities[i] * dt;
            
            Vector2f direction = velocities[i].normalized();
            float alignment = direction.dot(Vector2f(0.0f, 1.0f));
            benchmark::DoNotOptimize(alignment);
        }
        benchmark::DoNotOptimize(positions);
        benchmark::DoNotOptimize(velocities);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// Vector3 Operation Benchmarks

static void BM_Vector3_Addition(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector3f> positions(N);
    std::vector<Vector3f> velocities(N);
    std::vector<Vector3f> results(N);
    
    // Initialize with realistic position and velocity data
    for (size_t i = 0; i < N; ++i) {
        positions[i] = gen.generatePositionVector3();
        velocities[i] = gen.generateMovementVector3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = positions[i] + velocities[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector3_ScalarMultiply(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector3f> vectors(N);
    std::vector<Vector3f> results(N);
    const float scalar = 0.016667f; // Typical dt value (1/60)
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generateMovementVector3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i] * scalar;
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector3_DotProduct(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector3f> normals(N);
    std::vector<Vector3f> directions(N);
    std::vector<float> results(N);
    
    // Initialize with normalized vectors (common in dot product usage)
    for (size_t i = 0; i < N; ++i) {
        normals[i] = gen.generateNormalizedVector3();
        directions[i] = gen.generateNormalizedVector3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = normals[i].dot(directions[i]);
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector3_CrossProduct(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector3f> normals(N);
    std::vector<Vector3f> tangents(N);
    std::vector<Vector3f> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        normals[i] = gen.generateNormalizedVector3();
        tangents[i] = gen.generateNormalizedVector3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = normals[i].cross(tangents[i]);
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector3_Normalize(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector3f> vectors(N);
    std::vector<Vector3f> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generatePositionVector3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i].normalized();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector3_Length(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector3f> vectors(N);
    std::vector<float> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generatePositionVector3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i].length();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector3_LengthSquared(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector3f> vectors(N);
    std::vector<float> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generatePositionVector3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i].lengthSquared();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// Mixed operation benchmark that represents a typical frame update
static void BM_Vector3_TypicalFrameUpdate(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector3f> positions(N);
    std::vector<Vector3f> velocities(N);
    std::vector<Vector3f> forces(N);
    const float dt = 0.016667f; // 60 FPS
    const Vector3f gravity(0.0f, -9.81f, 0.0f);
    
    for (size_t i = 0; i < N; ++i) {
        positions[i] = gen.generatePositionVector3();
        velocities[i] = gen.generateMovementVector3();
        forces[i] = gen.generateMovementVector3();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            // Typical physics update sequence
            forces[i] += gravity;
            velocities[i] += forces[i] * dt;
            positions[i] += velocities[i] * dt;
            
            // Typical vector normalization for direction
            Vector3f direction = velocities[i].normalized();
            
            // Typical dot product for alignment check
            float alignment = direction.dot(Vector3f(0.0f, 1.0f, 0.0f));
            benchmark::DoNotOptimize(alignment);
        }
        benchmark::DoNotOptimize(positions);
        benchmark::DoNotOptimize(velocities);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// Vector4 Operation Benchmarks

static void BM_Vector4_Addition(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector4f> a(N);
    std::vector<Vector4f> b(N);
    std::vector<Vector4f> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        a[i] = gen.generatePositionVector4();
        b[i] = gen.generateMovementVector4();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = a[i] + b[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector4_ScalarMultiply(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector4f> vectors(N);
    std::vector<Vector4f> results(N);
    const float scalar = 0.016667f;
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generateMovementVector4();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i] * scalar;
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector4_DotProduct(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector4f> normals(N);
    std::vector<Vector4f> directions(N);
    std::vector<float> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        normals[i] = gen.generateNormalizedVector4();
        directions[i] = gen.generateNormalizedVector4();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = normals[i].dot(directions[i]);
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector4_Normalize(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector4f> vectors(N);
    std::vector<Vector4f> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generatePositionVector4();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i].normalized();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector4_Length(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector4f> vectors(N);
    std::vector<float> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        vectors[i] = gen.generatePositionVector4();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = vectors[i].length();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Vector4_TypicalFrameUpdate(benchmark::State& state) {
    const size_t N = state.range(0);
    VectorDataGenerator<float> gen;
    std::vector<Vector4f> positions(N);
    std::vector<Vector4f> velocities(N);
    std::vector<Vector4f> forces(N);
    const float dt = 0.016667f; // 60 FPS
    const Vector4f gravity(0.0f, -9.81f, 0.0f, 0.0f);
    
    for (size_t i = 0; i < N; ++i) {
        positions[i] = gen.generatePositionVector4();
        velocities[i] = gen.generateMovementVector4();
        forces[i] = gen.generateMovementVector4();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            forces[i] += gravity;
            velocities[i] += forces[i] * dt;
            positions[i] += velocities[i] * dt;
            
            Vector4f direction = velocities[i].normalized();
            float alignment = direction.dot(Vector4f(0.0f, 1.0f, 0.0f, 0.0f));
            benchmark::DoNotOptimize(alignment);
        }
        benchmark::DoNotOptimize(positions);
        benchmark::DoNotOptimize(velocities);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// Register benchmarks with different sizes
BENCHMARK(BM_Vector2_Addition)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector2_ScalarMultiply)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector2_DotProduct)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector2_Normalize)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector2_Length)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector2_TypicalFrameUpdate)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector3_Addition)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector3_ScalarMultiply)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector3_DotProduct)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector3_CrossProduct)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector3_Normalize)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector3_Length)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector3_LengthSquared)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector3_TypicalFrameUpdate)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector4_Addition)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector4_ScalarMultiply)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector4_DotProduct)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector4_Normalize)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector4_Length)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Vector4_TypicalFrameUpdate)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);
