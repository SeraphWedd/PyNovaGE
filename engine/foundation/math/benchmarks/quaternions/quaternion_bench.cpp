#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include <numeric>
#include "quaternions/quaternion.hpp"
#include "vectors/vectors.hpp"

using namespace PyNovaGE;

namespace {
    class QuaternionDataGenerator {
    private:
        mutable std::mt19937 rng;
        mutable std::uniform_real_distribution<float> dist;
        
    public:
        QuaternionDataGenerator() : rng(42), dist(-1.0f, 1.0f) {} // Fixed seed for reproducibility
        
        Quaternionf generateRandomQuaternion() const {
            return Quaternionf(dist(rng), dist(rng), dist(rng), dist(rng)).normalized();
        }
        
        Vector3f generateRandomVector() const {
            return Vector3f(dist(rng) * 10.0f, dist(rng) * 10.0f, dist(rng) * 10.0f);
        }
        
        Quaternionf generateRotationQuaternion() const {
            // Generate rotation around random axis with random angle
            Vector3f axis = Vector3f(dist(rng), dist(rng), dist(rng)).normalized();
            float angle = dist(rng) * 3.14159f; // 0 to PI radians
            return Quaternionf::AxisAngle(axis, angle);
        }
    };
}

static void BM_Quaternion_Multiplication(benchmark::State& state) {
    const size_t N = state.range(0);
    QuaternionDataGenerator gen;
    std::vector<Quaternionf> quats_a(N);
    std::vector<Quaternionf> quats_b(N);
    std::vector<Quaternionf> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        quats_a[i] = gen.generateRandomQuaternion();
        quats_b[i] = gen.generateRandomQuaternion();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = quats_a[i] * quats_b[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Quaternion_VectorRotation(benchmark::State& state) {
    const size_t N = state.range(0);
    QuaternionDataGenerator gen;
    std::vector<Quaternionf> rotations(N);
    std::vector<Vector3f> vectors(N);
    std::vector<Vector3f> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        rotations[i] = gen.generateRotationQuaternion();
        vectors[i] = gen.generateRandomVector();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = rotations[i] * vectors[i];
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Quaternion_Normalization(benchmark::State& state) {
    const size_t N = state.range(0);
    QuaternionDataGenerator gen;
    std::vector<Quaternionf> quaternions(N);
    std::vector<Quaternionf> results(N);
    
    // Create non-normalized quaternions
    for (size_t i = 0; i < N; ++i) {
        quaternions[i] = gen.generateRandomQuaternion() * 2.5f; // Make non-unit
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = quaternions[i].normalized();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Quaternion_Inverse(benchmark::State& state) {
    const size_t N = state.range(0);
    QuaternionDataGenerator gen;
    std::vector<Quaternionf> quaternions(N);
    std::vector<Quaternionf> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        quaternions[i] = gen.generateRandomQuaternion();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = quaternions[i].inverse();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Quaternion_Slerp(benchmark::State& state) {
    const size_t N = state.range(0);
    QuaternionDataGenerator gen;
    std::vector<Quaternionf> quats_a(N);
    std::vector<Quaternionf> quats_b(N);
    std::vector<Quaternionf> results(N);
    std::vector<float> t_values(N);
    
    for (size_t i = 0; i < N; ++i) {
        quats_a[i] = gen.generateRotationQuaternion();
        quats_b[i] = gen.generateRotationQuaternion();
        t_values[i] = static_cast<float>(i) / N; // Varying t from 0 to 1
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = Quaternionf::Slerp(quats_a[i], quats_b[i], t_values[i]);
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Quaternion_ToEulerAngles(benchmark::State& state) {
    const size_t N = state.range(0);
    QuaternionDataGenerator gen;
    std::vector<Quaternionf> quaternions(N);
    std::vector<Vector3f> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        quaternions[i] = gen.generateRotationQuaternion();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = quaternions[i].toEulerAngles();
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Quaternion_AxisAngleConstruction(benchmark::State& state) {
    const size_t N = state.range(0);
    QuaternionDataGenerator gen;
    std::vector<Vector3f> axes(N);
    std::vector<float> angles(N);
    std::vector<Quaternionf> results(N);
    
    for (size_t i = 0; i < N; ++i) {
        axes[i] = gen.generateRandomVector().normalized();
        angles[i] = gen.generateRandomQuaternion()[0] * 3.14159f; // Random angle
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            results[i] = Quaternionf::AxisAngle(axes[i], angles[i]);
        }
        benchmark::DoNotOptimize(results);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// Typical game object rotation update scenario
static void BM_Quaternion_TypicalGameObjectUpdate(benchmark::State& state) {
    const size_t N = state.range(0);
    QuaternionDataGenerator gen;
    std::vector<Quaternionf> orientations(N);
    std::vector<Quaternionf> rotation_deltas(N);
    std::vector<Vector3f> test_vectors(N);
    const float dt = 0.016667f; // 60 FPS
    
    for (size_t i = 0; i < N; ++i) {
        orientations[i] = gen.generateRotationQuaternion();
        // Small rotation delta for realistic game object movement
        Vector3f axis = gen.generateRandomVector().normalized();
        float angle = dt * 0.5f; // 30 degrees per second
        rotation_deltas[i] = Quaternionf::AxisAngle(axis, angle);
        test_vectors[i] = gen.generateRandomVector();
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            // Apply rotation delta
            orientations[i] = orientations[i] * rotation_deltas[i];
            orientations[i].normalize();
            
            // Transform a test vector (e.g., forward direction)
            Vector3f transformed = orientations[i] * test_vectors[i];
            
            benchmark::DoNotOptimize(orientations[i]);
            benchmark::DoNotOptimize(transformed);
        }
    }
    state.SetItemsProcessed(state.iterations() * N);
}

// Register benchmarks with different sizes
BENCHMARK(BM_Quaternion_Multiplication)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Quaternion_VectorRotation)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Quaternion_Normalization)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Quaternion_Inverse)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Quaternion_Slerp)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Quaternion_ToEulerAngles)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Quaternion_AxisAngleConstruction)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK(BM_Quaternion_TypicalGameObjectUpdate)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Unit(benchmark::kNanosecond);
