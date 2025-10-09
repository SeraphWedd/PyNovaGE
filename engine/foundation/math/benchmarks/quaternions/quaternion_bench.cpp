#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include "quaternions/quaternion.hpp"
#include "matrices/matrices.hpp"

using namespace PyNovaGE;

namespace {
    // Random number generator
    thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    
    // Data generator for consistent benchmarking
    class QuaternionDataGenerator {
    public:
        static Quaternionf generateRandomQuaternion() {
            return Quaternionf{dist(rng), dist(rng), dist(rng), dist(rng)}.normalized();
        }
        
        static Vector3f generateRandomVector() {
            return Vector3f{dist(rng), dist(rng), dist(rng)};
        }
        
        static std::vector<Quaternionf> generateQuaternionBatch(size_t count) {
            std::vector<Quaternionf> batch;
            batch.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                batch.emplace_back(generateRandomQuaternion());
            }
            return batch;
        }
        
        static std::vector<Vector3f> generateVectorBatch(size_t count) {
            std::vector<Vector3f> batch;
            batch.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                batch.emplace_back(generateRandomVector());
            }
            return batch;
        }
    };
}

// Benchmark quaternion multiplication
static void BM_Quaternion_Multiplication(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto quaternions_a = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    auto quaternions_b = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    
    size_t index = 0;
    for (auto _ : state) {
        Quaternionf result = quaternions_a[index % batch_size] * quaternions_b[index % batch_size];
        benchmark::DoNotOptimize(result);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * sizeof(Quaternionf) * 2);
}

// Benchmark vector rotation by quaternion
static void BM_Quaternion_VectorRotation(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto quaternions = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    auto vectors = QuaternionDataGenerator::generateVectorBatch(batch_size);
    
    size_t index = 0;
    for (auto _ : state) {
        Vector3f result = quaternions[index % batch_size] * vectors[index % batch_size];
        benchmark::DoNotOptimize(result);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * (sizeof(Quaternionf) + sizeof(Vector3f)));
}

// Benchmark quaternion normalization
static void BM_Quaternion_Normalization(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto quaternions = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    
    size_t index = 0;
    for (auto _ : state) {
        Quaternionf result = quaternions[index % batch_size].normalized();
        benchmark::DoNotOptimize(result);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * sizeof(Quaternionf));
}

// Benchmark quaternion inverse
static void BM_Quaternion_Inverse(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto quaternions = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    
    size_t index = 0;
    for (auto _ : state) {
        Quaternionf result = quaternions[index % batch_size].inverse();
        benchmark::DoNotOptimize(result);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * sizeof(Quaternionf));
}

// Temporarily commented out due to circular dependency issues
/*
// Benchmark quaternion to matrix conversion
static void BM_Quaternion_ToMatrix3(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto quaternions = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix3f result = quaternions[index % batch_size].toMatrix3();
        benchmark::DoNotOptimize(result);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * sizeof(Quaternionf));
}

// Benchmark quaternion to 4x4 matrix conversion
static void BM_Quaternion_ToMatrix4(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto quaternions = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    
    size_t index = 0;
    for (auto _ : state) {
        Matrix4f result = quaternions[index % batch_size].toMatrix4();
        benchmark::DoNotOptimize(result);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * sizeof(Quaternionf));
}
*/

// Benchmark SLERP interpolation
static void BM_Quaternion_Slerp(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto quaternions_a = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    auto quaternions_b = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    
    size_t index = 0;
    for (auto _ : state) {
        float t = 0.5f; // Mid-point interpolation
        Quaternionf result = Quaternionf::Slerp(
            quaternions_a[index % batch_size], 
            quaternions_b[index % batch_size], 
            t
        );
        benchmark::DoNotOptimize(result);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * sizeof(Quaternionf) * 2);
}

// Benchmark Euler angle conversion
static void BM_Quaternion_ToEulerAngles(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto quaternions = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    
    size_t index = 0;
    for (auto _ : state) {
        Vector3f result = quaternions[index % batch_size].toEulerAngles();
        benchmark::DoNotOptimize(result);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * sizeof(Quaternionf));
}

// Benchmark axis-angle construction
static void BM_Quaternion_AxisAngleConstruction(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto axes = QuaternionDataGenerator::generateVectorBatch(batch_size);
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * M_PI);
    
    std::vector<float> angles;
    angles.reserve(batch_size);
    for (size_t i = 0; i < batch_size; ++i) {
        angles.push_back(angle_dist(rng));
    }
    
    size_t index = 0;
    for (auto _ : state) {
        Quaternionf result = Quaternionf::AxisAngle(
            axes[index % batch_size].normalized(), 
            angles[index % batch_size]
        );
        benchmark::DoNotOptimize(result);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * (sizeof(Vector3f) + sizeof(float)));
}

// Benchmark typical game object rotation update
static void BM_Quaternion_TypicalGameObjectUpdate(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto current_orientations = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    auto rotation_deltas = QuaternionDataGenerator::generateQuaternionBatch(batch_size);
    auto test_vectors = QuaternionDataGenerator::generateVectorBatch(batch_size);
    
    // Simulate a typical game object transformation update
    size_t index = 0;
    for (auto _ : state) {
        size_t i = index % batch_size;
        
        // Apply rotation delta
        Quaternionf new_orientation = current_orientations[i] * rotation_deltas[i];
        new_orientation.normalize();
        
        // Transform a test vector (e.g., forward direction)
        Vector3f transformed = new_orientation * test_vectors[i];
        
        // Update for next iteration
        current_orientations[i] = new_orientation;
        
        benchmark::DoNotOptimize(new_orientation);
        benchmark::DoNotOptimize(transformed);
        ++index;
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * (sizeof(Quaternionf) * 2 + sizeof(Vector3f)));
}

// Register benchmarks with different batch sizes
BENCHMARK(BM_Quaternion_Multiplication)->RangeMultiplier(2)->Range(1024, 1048576);
BENCHMARK(BM_Quaternion_VectorRotation)->RangeMultiplier(2)->Range(1024, 1048576);
BENCHMARK(BM_Quaternion_Normalization)->RangeMultiplier(2)->Range(1024, 1048576);
BENCHMARK(BM_Quaternion_Inverse)->RangeMultiplier(2)->Range(1024, 1048576);
// Temporarily commented out due to circular dependency issues
//BENCHMARK(BM_Quaternion_ToMatrix3)->RangeMultiplier(2)->Range(1024, 1048576);
//BENCHMARK(BM_Quaternion_ToMatrix4)->RangeMultiplier(2)->Range(1024, 1048576);
BENCHMARK(BM_Quaternion_Slerp)->RangeMultiplier(2)->Range(1024, 1048576);
BENCHMARK(BM_Quaternion_ToEulerAngles)->RangeMultiplier(2)->Range(1024, 1048576);
BENCHMARK(BM_Quaternion_AxisAngleConstruction)->RangeMultiplier(2)->Range(1024, 1048576);
BENCHMARK(BM_Quaternion_TypicalGameObjectUpdate)->RangeMultiplier(2)->Range(1024, 1048576);
