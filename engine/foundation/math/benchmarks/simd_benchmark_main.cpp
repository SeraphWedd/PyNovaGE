#include <benchmark/benchmark.h>
#include "../include/simd/vector_ops.hpp"
#include "../include/simd/geometry_ops.hpp"
#include <random>
#include <vector>

using namespace PyNovaGE::SIMD;

//------------------------------------------------------------------------------
// Constants & Utilities
//------------------------------------------------------------------------------

// Batch sizes for different scenarios
const size_t VERTEX_BATCH_SIZE = 10000;     // For vertex processing (typical mesh size)
const size_t COLLISION_OBJECT_COUNT = 1000;   // For broad-phase collision (typical scene size)
const size_t PARTICLE_COUNT = 10000;         // For particle systems (typical particle count)

// Random number generation
static std::mt19937 rng(std::random_device{}());
static std::uniform_real_distribution<float> posDist(-100.0f, 100.0f);
static std::uniform_real_distribution<float> smallPosDist(-10.0f, 10.0f);
static std::uniform_real_distribution<float> sizeDist(0.5f, 5.0f);

// Helper function to generate random vectors
static Vector<float, 4> generateRandomVector4() {
    return Vector<float, 4>(
        posDist(rng),
        posDist(rng),
        posDist(rng),
        1.0f
    );
}

static Vector<float, 3> generateRandomVector3() {
    return Vector<float, 3>(
        posDist(rng),
        posDist(rng),
        posDist(rng)
    );
}

// Helper class for scalar operations
template<typename T, size_t N>
class ScalarVector {
public:
    T data_[N];
    
    ScalarVector() = default;
    ScalarVector(std::initializer_list<T> init) {
        std::copy(init.begin(), init.end(), data_);
    }
    
    T operator[](size_t i) const { return data_[i]; }
    T& operator[](size_t i) { return data_[i]; }
    
    const T* data() const { return data_; }
    T* data() { return data_; }
};

//------------------------------------------------------------------------------
// Vector Operation Benchmarks
//------------------------------------------------------------------------------

// Particle System Position Update (Vector Addition)
static void BM_ParticleSystem_SIMD(benchmark::State& state) {
    // Setup particle data
    std::vector<Vector<float, 4>> positions;
    std::vector<Vector<float, 4>> velocities;
    positions.reserve(PARTICLE_COUNT);
    velocities.reserve(PARTICLE_COUNT);
    
    // Pre-allocate result vector
    std::vector<Vector<float, 4>> results;
    positions.reserve(PARTICLE_COUNT);
    velocities.reserve(PARTICLE_COUNT);
    results.reserve(PARTICLE_COUNT);
    
    // Initialize particles in chunks for better cache utilization
    for (size_t i = 0; i < PARTICLE_COUNT; ++i) {
        positions.push_back(generateRandomVector4());
        velocities.push_back(Vector<float, 4>(
            smallPosDist(rng),
            smallPosDist(rng),
            smallPosDist(rng),
            0.0f
        ));
        results.push_back(Vector<float, 4>());  // Reserve space
    }
    
    const float dt = 1.0f / 60.0f;  // 60 FPS simulation
    
    for (auto _ : state) {
        // Update particles in chunks for better cache utilization
        for (size_t i = 0; i < PARTICLE_COUNT; i += 64) {
            size_t end = std::min(i + 64, PARTICLE_COUNT);
            for (size_t j = i; j < end; ++j) {
                results[j] = positions[j] + (velocities[j] * dt);
            }
        }
        std::swap(positions, results);
        benchmark::DoNotOptimize(positions[0]);
        benchmark::DoNotOptimize(positions[PARTICLE_COUNT-1]);
    }
    
    state.SetItemsProcessed(state.iterations() * PARTICLE_COUNT);
}
BENCHMARK(BM_ParticleSystem_SIMD);

static void BM_ParticleSystem_Scalar(benchmark::State& state) {
    // Setup particle data
    std::vector<ScalarVector<float, 4>> positions;
    std::vector<ScalarVector<float, 4>> velocities;
    positions.reserve(PARTICLE_COUNT);
    velocities.reserve(PARTICLE_COUNT);
    
    // Initialize particles
    for (size_t i = 0; i < PARTICLE_COUNT; ++i) {
        auto pos = generateRandomVector4();
        positions.push_back(ScalarVector<float, 4>{pos[0], pos[1], pos[2], pos[3]});
        velocities.push_back(ScalarVector<float, 4>{
            smallPosDist(rng),
            smallPosDist(rng),
            smallPosDist(rng),
            0.0f
        });
    }
    
    const float dt = 1.0f / 60.0f;
    
    for (auto _ : state) {
        // Update all particle positions
        for (size_t i = 0; i < PARTICLE_COUNT; ++i) {
            ScalarVector<float, 4> result;
            for (size_t j = 0; j < 4; ++j) {
                result[j] = positions[i][j] + (velocities[i][j] * dt);
            }
            positions[i] = result;
        }
        benchmark::DoNotOptimize(positions);
    }
    
    state.SetItemsProcessed(state.iterations() * PARTICLE_COUNT);
}
BENCHMARK(BM_ParticleSystem_Scalar);

// Ray Direction Normalization (Dot Product)
static void BM_RayNormalization_SIMD(benchmark::State& state) {
    // Setup a batch of ray directions
    std::vector<Vector<float, 4>> directions;
    directions.reserve(VERTEX_BATCH_SIZE);
    
    // Initialize random directions
    for (size_t i = 0; i < VERTEX_BATCH_SIZE; ++i) {
        directions.push_back(generateRandomVector4());
    }
    
    for (auto _ : state) {
        // Normalize all directions using dot product
        for (auto& dir : directions) {
            float lenSq = dot(dir, dir);
            float invLen = 1.0f / std::sqrt(lenSq);
            dir = dir * invLen;
        }
        benchmark::DoNotOptimize(directions);
    }
    
    state.SetItemsProcessed(state.iterations() * VERTEX_BATCH_SIZE);
}
BENCHMARK(BM_RayNormalization_SIMD);

static void BM_RayNormalization_Scalar(benchmark::State& state) {
    // Setup a batch of ray directions
    std::vector<ScalarVector<float, 4>> directions;
    directions.reserve(VERTEX_BATCH_SIZE);
    
    // Initialize random directions
    for (size_t i = 0; i < VERTEX_BATCH_SIZE; ++i) {
        auto dir = generateRandomVector4();
        directions.push_back(ScalarVector<float, 4>{dir[0], dir[1], dir[2], dir[3]});
    }
    
    for (auto _ : state) {
        // Normalize all directions using scalar operations
        for (auto& dir : directions) {
            float lenSq = 0.0f;
            for (size_t i = 0; i < 4; ++i) {
                lenSq += dir[i] * dir[i];
            }
            float invLen = 1.0f / std::sqrt(lenSq);
            for (size_t i = 0; i < 4; ++i) {
                dir[i] *= invLen;
            }
        }
        benchmark::DoNotOptimize(directions);
    }
    
    state.SetItemsProcessed(state.iterations() * VERTEX_BATCH_SIZE);
}
BENCHMARK(BM_RayNormalization_Scalar);

//------------------------------------------------------------------------------
// Geometry Operation Benchmarks
//------------------------------------------------------------------------------

// Broad Phase Collision Detection
static void BM_BroadPhase_SIMD(benchmark::State& state) {
    // Create a set of AABBs
    std::vector<AABB<float>> objects;
    objects.reserve(COLLISION_OBJECT_COUNT);
    
    // Initialize objects with random positions and sizes
    for (size_t i = 0; i < COLLISION_OBJECT_COUNT; ++i) {
        Vector<float, 3> center = generateRandomVector3();
        Vector<float, 3> extent(sizeDist(rng), sizeDist(rng), sizeDist(rng));
        objects.emplace_back(center - extent, center + extent);
    }
    
    for (auto _ : state) {
        size_t collisions = 0;
        // Test all pairs of objects
        for (size_t i = 0; i < COLLISION_OBJECT_COUNT; ++i) {
            for (size_t j = i + 1; j < COLLISION_OBJECT_COUNT; ++j) {
                if (objects[i].intersects(objects[j])) {
                    collisions++;
                }
            }
        }
        benchmark::DoNotOptimize(collisions);
    }
    
    state.SetItemsProcessed(state.iterations() * (COLLISION_OBJECT_COUNT * (COLLISION_OBJECT_COUNT - 1) / 2));
}
BENCHMARK(BM_BroadPhase_SIMD);

static void BM_BroadPhase_Scalar(benchmark::State& state) {
    // Create a set of AABBs using scalar vectors
    struct ScalarAABB {
        ScalarVector<float, 3> min;
        ScalarVector<float, 3> max;
    };
    
    std::vector<ScalarAABB> objects;
    objects.reserve(COLLISION_OBJECT_COUNT);
    
    // Initialize objects with random positions and sizes
    for (size_t i = 0; i < COLLISION_OBJECT_COUNT; ++i) {
        Vector<float, 3> center = generateRandomVector3();
        Vector<float, 3> extent(sizeDist(rng), sizeDist(rng), sizeDist(rng));
        objects.push_back(ScalarAABB{
            ScalarVector<float, 3>{center[0] - extent[0], center[1] - extent[1], center[2] - extent[2]},
            ScalarVector<float, 3>{center[0] + extent[0], center[1] + extent[1], center[2] + extent[2]}
        });
    }
    
    for (auto _ : state) {
        size_t collisions = 0;
        // Test all pairs of objects
        for (size_t i = 0; i < COLLISION_OBJECT_COUNT; ++i) {
            for (size_t j = i + 1; j < COLLISION_OBJECT_COUNT; ++j) {
                bool intersects = 
                    objects[i].min[0] <= objects[j].max[0] && objects[i].max[0] >= objects[j].min[0] &&
                    objects[i].min[1] <= objects[j].max[1] && objects[i].max[1] >= objects[j].min[1] &&
                    objects[i].min[2] <= objects[j].max[2] && objects[i].max[2] >= objects[j].min[2];
                if (intersects) {
                    collisions++;
                }
            }
        }
        benchmark::DoNotOptimize(collisions);
    }
    
    state.SetItemsProcessed(state.iterations() * (COLLISION_OBJECT_COUNT * (COLLISION_OBJECT_COUNT - 1) / 2));
}
BENCHMARK(BM_BroadPhase_Scalar);

// Point Cloud Containment Test
static void BM_PointCloudContainment_SIMD(benchmark::State& state) {
    // Create test points
    std::vector<Vector<float, 3>> points;
    points.reserve(VERTEX_BATCH_SIZE);
    
    // Initialize random points
    for (size_t i = 0; i < VERTEX_BATCH_SIZE; ++i) {
        points.push_back(generateRandomVector3());
    }
    
    // Create test AABB
    AABB<float> bounds(
        Vector<float, 3>(-10.0f, -10.0f, -10.0f),
        Vector<float, 3>(10.0f, 10.0f, 10.0f)
    );
    
    for (auto _ : state) {
        size_t contained = 0;
        for (const auto& point : points) {
            if (bounds.contains(point)) {
                contained++;
            }
        }
        benchmark::DoNotOptimize(contained);
    }
    
    state.SetItemsProcessed(state.iterations() * VERTEX_BATCH_SIZE);
}
BENCHMARK(BM_PointCloudContainment_SIMD);

static void BM_PointCloudContainment_Scalar(benchmark::State& state) {
    // Create test points
    std::vector<ScalarVector<float, 3>> points;
    points.reserve(VERTEX_BATCH_SIZE);
    
    // Initialize random points
    for (size_t i = 0; i < VERTEX_BATCH_SIZE; ++i) {
        auto point = generateRandomVector3();
        points.push_back(ScalarVector<float, 3>{point[0], point[1], point[2]});
    }
    
    // Create test bounds
    ScalarVector<float, 3> min{-10.0f, -10.0f, -10.0f};
    ScalarVector<float, 3> max{10.0f, 10.0f, 10.0f};
    
    for (auto _ : state) {
        size_t contained = 0;
        for (const auto& point : points) {
            bool isContained = 
                point[0] >= min[0] && point[0] <= max[0] &&
                point[1] >= min[1] && point[1] <= max[1] &&
                point[2] >= min[2] && point[2] <= max[2];
            if (isContained) {
                contained++;
            }
        }
        benchmark::DoNotOptimize(contained);
    }
    
    state.SetItemsProcessed(state.iterations() * VERTEX_BATCH_SIZE);
}
BENCHMARK(BM_PointCloudContainment_Scalar);

BENCHMARK_MAIN();