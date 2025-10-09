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
const size_t VERTEX_BATCH_SIZE = 100000;     // For vertex processing (larger mesh size)
const size_t COLLISION_OBJECT_COUNT = 10000;   // For broad-phase collision (larger scene size)
const size_t PARTICLE_COUNT = 100000;         // For particle systems (larger particle count)

// SIMD alignment
const size_t SIMD_ALIGNMENT = 32;  // AVX alignment requirement

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

// Aligned allocator for SIMD operations
template<typename T>
class AlignedAllocator {
public:
    using value_type = T;
    
    AlignedAllocator() noexcept {}
    
    template<typename U>
    AlignedAllocator(const AlignedAllocator<U>&) noexcept {}
    
    T* allocate(std::size_t n) {
        if (n == 0) return nullptr;
        void* ptr = _aligned_malloc(n * sizeof(T), SIMD_ALIGNMENT);
        if (!ptr) throw std::bad_alloc();
        return static_cast<T*>(ptr);
    }
    
    void deallocate(T* p, std::size_t) noexcept {
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

// Structure of Arrays for particle data
struct ParticleSOA {
    std::vector<float, AlignedAllocator<float>> x, y, z, w;
    
    ParticleSOA(size_t count) : 
        x(count), y(count), z(count), w(count) {}
};

// Particle System Position Update (Vector Addition)
static void BM_ParticleSystem_SIMD(benchmark::State& state) {
    // Setup particle data using SoA layout
    ParticleSOA positions(PARTICLE_COUNT);
    ParticleSOA velocities(PARTICLE_COUNT);
    ParticleSOA results(PARTICLE_COUNT);
    
    // Initialize particles
    for (size_t i = 0; i < PARTICLE_COUNT; ++i) {
        auto pos = generateRandomVector4();
        positions.x[i] = pos[0];
        positions.y[i] = pos[1];
        positions.z[i] = pos[2];
        positions.w[i] = pos[3];
        
        velocities.x[i] = smallPosDist(rng);
        velocities.y[i] = smallPosDist(rng);
        velocities.z[i] = smallPosDist(rng);
        velocities.w[i] = 0.0f;
    }
    
    const float dt = 1.0f / 60.0f;  // 60 FPS simulation
    const size_t CHUNK_SIZE = 256;  // Process more particles per chunk
    
    for (auto _ : state) {
        // Update particles in larger chunks
        for (size_t i = 0; i < PARTICLE_COUNT; i += CHUNK_SIZE) {
            size_t end = std::min(i + CHUNK_SIZE, PARTICLE_COUNT);
            for (size_t j = i; j < end; j += 8) {  // Process 8 particles at a time with AVX
                // Load position and velocity data
                __m256 px = _mm256_load_ps(&positions.x[j]);
                __m256 py = _mm256_load_ps(&positions.y[j]);
                __m256 pz = _mm256_load_ps(&positions.z[j]);
                
                __m256 vx = _mm256_load_ps(&velocities.x[j]);
                __m256 vy = _mm256_load_ps(&velocities.y[j]);
                __m256 vz = _mm256_load_ps(&velocities.z[j]);
                
                // Scale velocities by dt
                __m256 dt_vec = _mm256_set1_ps(dt);
                vx = _mm256_mul_ps(vx, dt_vec);
                vy = _mm256_mul_ps(vy, dt_vec);
                vz = _mm256_mul_ps(vz, dt_vec);
                
                // Add scaled velocities to positions
                __m256 rx = _mm256_add_ps(px, vx);
                __m256 ry = _mm256_add_ps(py, vy);
                __m256 rz = _mm256_add_ps(pz, vz);
                
                // Store results
                _mm256_store_ps(&results.x[j], rx);
                _mm256_store_ps(&results.y[j], ry);
                _mm256_store_ps(&results.z[j], rz);
                _mm256_store_ps(&results.w[j], _mm256_set1_ps(1.0f));
            }
        }
        std::swap(positions.x, results.x);
        std::swap(positions.y, results.y);
        std::swap(positions.z, results.z);
        std::swap(positions.w, results.w);
        
        benchmark::DoNotOptimize(positions.x[0]);
        benchmark::DoNotOptimize(positions.x[PARTICLE_COUNT-1]);
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

// Structure of Arrays for ray data
struct RaySOA {
    std::vector<float, AlignedAllocator<float>> x, y, z, w;
    
    RaySOA(size_t count) : 
        x(count), y(count), z(count), w(count) {}
};

// Ray Direction Normalization (Dot Product)
static void BM_RayNormalization_SIMD(benchmark::State& state) {
    // Setup ray data using SoA layout
    RaySOA directions(VERTEX_BATCH_SIZE);
    RaySOA normalized(VERTEX_BATCH_SIZE);
    
    // Initialize random directions
    for (size_t i = 0; i < VERTEX_BATCH_SIZE; ++i) {
        auto dir = generateRandomVector4();
        directions.x[i] = dir[0];
        directions.y[i] = dir[1];
        directions.z[i] = dir[2];
        directions.w[i] = dir[3];
    }
    
    const size_t CHUNK_SIZE = 256;  // Process rays in larger chunks
    
    for (auto _ : state) {
        // Process rays in chunks for better cache utilization
        for (size_t i = 0; i < VERTEX_BATCH_SIZE; i += CHUNK_SIZE) {
            size_t end = std::min(i + CHUNK_SIZE, VERTEX_BATCH_SIZE);
            for (size_t j = i; j < end; j += 8) {  // Process 8 rays at a time with AVX
                // Load ray components
                __m256 rx = _mm256_load_ps(&directions.x[j]);
                __m256 ry = _mm256_load_ps(&directions.y[j]);
                __m256 rz = _mm256_load_ps(&directions.z[j]);
                
                // Calculate length squared (rx*rx + ry*ry + rz*rz)
                __m256 lenSq = _mm256_mul_ps(rx, rx);
                lenSq = _mm256_fmadd_ps(ry, ry, lenSq);  // FMA for better performance
                lenSq = _mm256_fmadd_ps(rz, rz, lenSq);
                
                // Calculate inverse length using fast reciprocal sqrt
                __m256 invLen = _mm256_rsqrt_ps(lenSq);
                
                // One Newton-Raphson iteration to improve accuracy
                // invLen = invLen * (1.5f - 0.5f * lenSq * invLen * invLen)
                __m256 half = _mm256_set1_ps(0.5f);
                __m256 three_halves = _mm256_set1_ps(1.5f);
                __m256 invLenSq = _mm256_mul_ps(invLen, invLen);
                __m256 correction = _mm256_mul_ps(lenSq, invLenSq);
                correction = _mm256_mul_ps(half, correction);
                correction = _mm256_sub_ps(three_halves, correction);
                invLen = _mm256_mul_ps(invLen, correction);
                
                // Normalize the components
                rx = _mm256_mul_ps(rx, invLen);
                ry = _mm256_mul_ps(ry, invLen);
                rz = _mm256_mul_ps(rz, invLen);
                
                // Store normalized results
                _mm256_store_ps(&normalized.x[j], rx);
                _mm256_store_ps(&normalized.y[j], ry);
                _mm256_store_ps(&normalized.z[j], rz);
                _mm256_store_ps(&normalized.w[j], _mm256_set1_ps(1.0f));
            }
        }
        std::swap(directions.x, normalized.x);
        std::swap(directions.y, normalized.y);
        std::swap(directions.z, normalized.z);
        std::swap(directions.w, normalized.w);
        
        benchmark::DoNotOptimize(directions.x[0]);
        benchmark::DoNotOptimize(directions.x[VERTEX_BATCH_SIZE-1]);
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