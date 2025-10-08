/**
 * @file matrix_real_world_benchmarks.cpp
 * @brief Real-world performance benchmarks for Matrix operations
 * 
 * These benchmarks simulate real-world use cases for matrices in game engine scenarios:
 * - Transform hierarchies (scene graphs, character rigs)
 * - Camera operations (view, projection matrices)
 * - Model-View-Projection pipelines
 * - Physics system transformations
 */

#include <benchmark/benchmark.h>
#include "../include/matrix2.hpp"
#include "../include/matrix3.hpp"
#include "../include/matrix4.hpp"
#include "../include/vector2.hpp"
#include "../include/vector3.hpp"
#include "../include/vector4.hpp"
#include "../include/quaternion.hpp"
#include <random>
#include <vector>

using namespace pynovage::math;

namespace {

//------------------------------------------------------------------------------
// Utility Functions
//------------------------------------------------------------------------------

// Random number generator setup
static std::mt19937 rng(std::random_device{}());
static std::uniform_real_distribution<float> angleDist(0.0f, constants::two_pi);
static std::uniform_real_distribution<float> scaleDist(0.5f, 2.0f);
static std::uniform_real_distribution<float> posDist(-100.0f, 100.0f);
static std::uniform_real_distribution<float> nearPosDist(-10.0f, 10.0f);

// Generate a random transform matrix (scale + rotation + translation)
static Matrix4 generateRandomTransform() {
    // Random rotation
    float yaw = angleDist(rng);
    float pitch = angleDist(rng) * 0.49f; // Limit pitch to avoid gimbal lock
    float roll = angleDist(rng);
    Matrix4 rotation = Matrix4::fromEulerAngles(yaw, pitch, roll);
    
    // Random scale
    Vector3 scale(scaleDist(rng), scaleDist(rng), scaleDist(rng));
    Matrix4 scaleMatrix = Matrix4::scale(scale.x, scale.y, scale.z);
    
    // Random translation
    Vector3 translation(posDist(rng), posDist(rng), posDist(rng));
    Matrix4 translateMatrix = Matrix4::translation(translation.x, translation.y, translation.z);
    
    return translateMatrix * rotation * scaleMatrix;
}

// Generate random points for transformation
static std::vector<Vector4> generateRandomPoints(size_t count) {
    std::vector<Vector4> points;
    points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        points.emplace_back(
            nearPosDist(rng),
            nearPosDist(rng),
            nearPosDist(rng),
            1.0f
        );
    }
    return points;
}

// Generate a random rotation matrix
static Matrix4 generateRandomRotation() {
    float yaw = angleDist(rng);
    float pitch = angleDist(rng) * 0.49f; // Limit pitch to avoid gimbal lock
    float roll = angleDist(rng);
    return Matrix4::fromEulerAngles(yaw, pitch, roll);
}

//------------------------------------------------------------------------------
// Transform Chain Optimization Benchmarks
//------------------------------------------------------------------------------

// Benchmark single node transform operations
static void BM_SingleNodeTransform(benchmark::State& state) {
    Matrix4 transform = generateRandomTransform();
    Vector4 point(1.0f, 2.0f, 3.0f, 1.0f);
    
    for (auto _ : state) {
        Vector4 result = transform * point;
        benchmark::DoNotOptimize(result);
    }
}

// Benchmark transform chain operations (simulates character rig or scene graph)
static void BM_TransformChain(benchmark::State& state) {
    const size_t chainLength = state.range(0);
    std::vector<Matrix4> transforms;
    transforms.reserve(chainLength);
    
    // Generate chain of transforms
    for (size_t i = 0; i < chainLength; ++i) {
        transforms.push_back(generateRandomTransform());
    }
    
    for (auto _ : state) {
        Matrix4 finalTransform = transforms[0];
        for (size_t i = 1; i < chainLength; ++i) {
            finalTransform = finalTransform * transforms[i];
        }
        benchmark::DoNotOptimize(finalTransform);
    }
    
    state.SetComplexityN(state.range(0));
}

// Benchmark mixed transform operations (translate + rotate + scale)
static void BM_MixedTransformChain(benchmark::State& state) {
    const size_t chainLength = state.range(0);
    std::vector<Vector3> translations;
    std::vector<Vector3> rotations; // Euler angles
    std::vector<Vector3> scales;
    
    translations.reserve(chainLength);
    rotations.reserve(chainLength);
    scales.reserve(chainLength);
    
    // Generate random transformations
    for (size_t i = 0; i < chainLength; ++i) {
        translations.emplace_back(posDist(rng), posDist(rng), posDist(rng));
        rotations.emplace_back(angleDist(rng), angleDist(rng) * 0.49f, angleDist(rng));
        scales.emplace_back(scaleDist(rng), scaleDist(rng), scaleDist(rng));
    }
    
    for (auto _ : state) {
        Matrix4 final = Matrix4::identity();
        for (size_t i = 0; i < chainLength; ++i) {
            // Build transformation in TRS order
            Matrix4 transform = Matrix4::translation(translations[i].x, translations[i].y, translations[i].z) *
                              Matrix4::fromEulerAngles(rotations[i].x, rotations[i].y, rotations[i].z) *
                              Matrix4::scale(scales[i].x, scales[i].y, scales[i].z);
            final = final * transform;
        }
        benchmark::DoNotOptimize(final);
    }
    
    state.SetComplexityN(state.range(0));
}

//------------------------------------------------------------------------------
// Camera Operations Benchmarks
//------------------------------------------------------------------------------

static void BM_LookAtMatrix(benchmark::State& state) {
    // Generate random camera positions and targets
    std::vector<Vector3> positions;
    std::vector<Vector3> targets;
    std::vector<Vector3> ups;
    
    const size_t numViews = 100;
    positions.reserve(numViews);
    targets.reserve(numViews);
    ups.reserve(numViews);
    
    for (size_t i = 0; i < numViews; ++i) {
        positions.emplace_back(posDist(rng), posDist(rng), posDist(rng));
        targets.emplace_back(posDist(rng), posDist(rng), posDist(rng));
        ups.emplace_back(0.0f, 1.0f, 0.0f); // Typically up vector is fixed
    }
    
    size_t currentView = 0;
    for (auto _ : state) {
        Matrix4 view = Matrix4::lookAt(
            positions[currentView],
            targets[currentView],
            ups[currentView]
        );
        benchmark::DoNotOptimize(view);
        currentView = (currentView + 1) % numViews;
    }
}

static void BM_ProjectionMatrix(benchmark::State& state) {
    // Common projection scenarios
    const float fov = constants::pi / 3.0f; // 60 degrees
    const float aspect = 16.0f / 9.0f;      // Widescreen
    const float nearPlane = 0.1f;
    const float farPlane = 1000.0f;
    
    for (auto _ : state) {
        Matrix4 proj = Matrix4::perspective(fov, aspect, nearPlane, farPlane);
        benchmark::DoNotOptimize(proj);
    }
}

static void BM_ViewProjectionPipeline(benchmark::State& state) {
    // Camera setup
    Vector3 eye(0.0f, 5.0f, 10.0f);
    Vector3 target(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);
    
    // Projection setup
    const float fov = constants::pi / 3.0f;
    const float aspect = 16.0f / 9.0f;
    const float nearPlane = 0.1f;
    const float farPlane = 1000.0f;
    
    // Generate some random view positions
    const size_t numViews = 100;
    std::vector<Vector3> cameraPositions;
    cameraPositions.reserve(numViews);
    
    for (size_t i = 0; i < numViews; ++i) {
        cameraPositions.emplace_back(
            nearPosDist(rng),
            nearPosDist(rng) + 5.0f, // Keep camera above ground
            nearPosDist(rng) + 10.0f  // Keep some distance from target
        );
    }
    
    // Pre-compute projection matrix since it rarely changes
    Matrix4 proj = Matrix4::perspective(fov, aspect, nearPlane, farPlane);
    size_t currentPos = 0;
    
    for (auto _ : state) {
        eye = cameraPositions[currentPos];
        Matrix4 view = Matrix4::lookAt(eye, target, up);
        Matrix4 viewProj = proj * view;
        benchmark::DoNotOptimize(viewProj);
        currentPos = (currentPos + 1) % numViews;
    }
}

//------------------------------------------------------------------------------
// Model-View-Projection Chain Benchmarks
//------------------------------------------------------------------------------

static void BM_MVPConstruction(benchmark::State& state) {
    // Camera and projection setup
    const Vector3 eye(0.0f, 5.0f, 10.0f);
    const Vector3 target(0.0f, 0.0f, 0.0f);
    const Vector3 up(0.0f, 1.0f, 0.0f);
    
    const float fov = constants::pi / 3.0f;
    const float aspect = 16.0f / 9.0f;
    const float nearPlane = 0.1f;
    const float farPlane = 1000.0f;
    
    // Pre-compute view-projection since camera is static
    Matrix4 view = Matrix4::lookAt(eye, target, up);
    Matrix4 proj = Matrix4::perspective(fov, aspect, nearPlane, farPlane);
    Matrix4 viewProj = proj * view;
    
    // Generate some random model transforms
    const size_t numModels = 100;
    std::vector<Matrix4> modelTransforms;
    modelTransforms.reserve(numModels);
    
    for (size_t i = 0; i < numModels; ++i) {
        modelTransforms.push_back(generateRandomTransform());
    }
    
    size_t currentModel = 0;
    
    for (auto _ : state) {
        Matrix4 mvp = viewProj * modelTransforms[currentModel];
        benchmark::DoNotOptimize(mvp);
        currentModel = (currentModel + 1) % numModels;
    }
}

static void BM_BatchVertexTransform(benchmark::State& state) {
    const size_t vertexCount = state.range(0);
    
    // Generate random vertices
    auto vertices = generateRandomPoints(vertexCount);
    
    // Setup MVP matrix
    Matrix4 model = generateRandomTransform();
    Matrix4 view = Matrix4::lookAt(
        Vector3(0.0f, 5.0f, 10.0f),
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f)
    );
    Matrix4 proj = Matrix4::perspective(
        constants::pi / 3.0f,
        16.0f / 9.0f,
        0.1f,
        1000.0f
    );
    
    Matrix4 mvp = proj * view * model;
    
    std::vector<Vector4> transformed;
    transformed.resize(vertexCount);
    
    for (auto _ : state) {
        for (size_t i = 0; i < vertexCount; ++i) {
            transformed[i] = mvp * vertices[i];
        }
        benchmark::DoNotOptimize(transformed);
    }
    
    state.SetItemsProcessed(state.iterations() * vertexCount);
}

static void BM_DynamicMVPUpdate(benchmark::State& state) {
    const size_t vertexCount = state.range(0);
    
    // Generate random vertices and transforms
    auto vertices = generateRandomPoints(vertexCount);
    std::vector<Matrix4> models;
    const size_t numModels = 100;
    models.reserve(numModels);
    
    for (size_t i = 0; i < numModels; ++i) {
        models.push_back(generateRandomTransform());
    }
    
    // Setup view-projection
    Matrix4 view = Matrix4::lookAt(
        Vector3(0.0f, 5.0f, 10.0f),
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f)
    );
    Matrix4 proj = Matrix4::perspective(
        constants::pi / 3.0f,
        16.0f / 9.0f,
        0.1f,
        1000.0f
    );
    
    Matrix4 viewProj = proj * view;
    size_t currentModel = 0;
    std::vector<Vector4> transformed;
    transformed.resize(vertexCount);
    
    for (auto _ : state) {
        Matrix4 mvp = viewProj * models[currentModel];
        for (size_t i = 0; i < vertexCount; ++i) {
            transformed[i] = mvp * vertices[i];
        }
        benchmark::DoNotOptimize(transformed);
        currentModel = (currentModel + 1) % numModels;
    }
    
    state.SetItemsProcessed(state.iterations() * vertexCount);
}

//------------------------------------------------------------------------------
// Physics Transform Benchmarks
//------------------------------------------------------------------------------

static void BM_MatrixDecomposition(benchmark::State& state) {
    // Generate random transforms to decompose
    const size_t numTransforms = 100;
    std::vector<Matrix4> transforms;
    transforms.reserve(numTransforms);
    
    for (size_t i = 0; i < numTransforms; ++i) {
        transforms.push_back(generateRandomTransform());
    }
    
    size_t currentTransform = 0;
    
    for (auto _ : state) {
        Vector3 translation = transforms[currentTransform].extractTranslation();
        Vector3 scale = transforms[currentTransform].extractScale();
        Quaternion rotation = transforms[currentTransform].extractRotation();
        benchmark::DoNotOptimize(translation);
        benchmark::DoNotOptimize(scale);
        benchmark::DoNotOptimize(rotation);
        currentTransform = (currentTransform + 1) % numTransforms;
    }
}

static void BM_TransformModification(benchmark::State& state) {
    // Initial transform
    Matrix4 transform = generateRandomTransform();
    Vector3 currentTranslation = transform.extractTranslation();
    Vector3 currentScale = transform.extractScale();
    Quaternion currentRotation = transform.extractRotation();
    
    // Motion parameters
    const float deltaTime = 1.0f / 60.0f;
    const Vector3 velocity(1.0f, 0.5f, -0.7f);
    const Vector3 angularVelocity(0.5f, 1.0f, 0.3f);
    const Vector3 scaleVelocity(0.1f, -0.05f, 0.08f);
    
    for (auto _ : state) {
        // Update transform components
        currentTranslation += velocity * deltaTime;
        currentScale += scaleVelocity * deltaTime;
        
        // Update rotation using angular velocity
        float angle = angularVelocity.length() * deltaTime;
        if (angle > 0.0f) {
            Quaternion deltaRotation(angularVelocity.normalized(), angle);
            currentRotation = deltaRotation * currentRotation;
        }
        
        // Rebuild matrix
        Matrix4 newTransform = Matrix4::fromQuaternion(currentRotation);
        newTransform[0][0] *= currentScale.x; newTransform[1][0] *= currentScale.x; newTransform[2][0] *= currentScale.x;
        newTransform[0][1] *= currentScale.y; newTransform[1][1] *= currentScale.y; newTransform[2][1] *= currentScale.y;
        newTransform[0][2] *= currentScale.z; newTransform[1][2] *= currentScale.z; newTransform[2][2] *= currentScale.z;
        newTransform[0][3] = currentTranslation.x;
        newTransform[1][3] = currentTranslation.y;
        newTransform[2][3] = currentTranslation.z;
        
        benchmark::DoNotOptimize(newTransform);
    }
}

static void BM_TransformInterpolation(benchmark::State& state) {
    // Generate start and end transforms
    Matrix4 start = generateRandomTransform();
    Matrix4 end = generateRandomTransform();
    
    // Fixed timestep simulation at 60 Hz
    const float deltaTime = 1.0f / 60.0f;
    float accumTime = 0.0f;
    const float duration = 1.0f;
    
    for (auto _ : state) {
        // Compute interpolation factor
        float t = accumTime / duration;
        t = std::min(t, 1.0f);
        
        // Interpolate transforms
        Matrix4 interpolated = Matrix4::lerp(start, end, t);
        
        benchmark::DoNotOptimize(interpolated);
        
        // Update time
        accumTime += deltaTime;
        if (accumTime >= duration) {
            accumTime = 0.0f;
            start = end;
            end = generateRandomTransform();
        }
    }
}

} // namespace

//------------------------------------------------------------------------------
// Register Benchmarks
//------------------------------------------------------------------------------

// Transform chain benchmarks
BENCHMARK(BM_SingleNodeTransform);
BENCHMARK(BM_TransformChain)->RangeMultiplier(2)->Range(4, 16);
BENCHMARK(BM_MixedTransformChain)->RangeMultiplier(2)->Range(4, 16);

// Camera operation benchmarks
BENCHMARK(BM_LookAtMatrix);
BENCHMARK(BM_ProjectionMatrix);
BENCHMARK(BM_ViewProjectionPipeline);

// MVP chain benchmarks
BENCHMARK(BM_MVPConstruction);
BENCHMARK(BM_BatchVertexTransform)->RangeMultiplier(10)->Range(100, 10000);
BENCHMARK(BM_DynamicMVPUpdate)->RangeMultiplier(10)->Range(100, 10000);

// Physics transform benchmarks
BENCHMARK(BM_MatrixDecomposition);
BENCHMARK(BM_TransformModification);
BENCHMARK(BM_TransformInterpolation);
