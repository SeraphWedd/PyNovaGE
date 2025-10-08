#include <benchmark/benchmark.h>
#include "../include/quaternion.hpp"
#include "../include/vector3.hpp"
#include "../include/matrix4.hpp"
#include <vector>
#include <random>

using namespace pynovage::math;

namespace {

//------------------------------------------------------------------------------
// Utility Functions
//------------------------------------------------------------------------------

static std::mt19937 rng(std::random_device{}());
static std::uniform_real_distribution<float> angleDist(0.0f, constants::two_pi);
static std::uniform_real_distribution<float> normalizedDist(-1.0f, 1.0f);
static std::uniform_real_distribution<float> posDist(-100.0f, 100.0f);

// Generate random axis
static Vector3 generateRandomAxis() {
    Vector3 axis(
        normalizedDist(rng),
        normalizedDist(rng),
        normalizedDist(rng)
    );
    return axis.normalized();
}

// Generate random quaternion
static Quaternion generateRandomRotation() {
    return Quaternion(generateRandomAxis(), angleDist(rng));
}

// Generate array of random quaternions
static std::vector<Quaternion> generateRandomRotations(size_t count) {
    std::vector<Quaternion> rotations;
    rotations.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        rotations.push_back(generateRandomRotation());
    }
    return rotations;
}

// Generate array of random vectors
static std::vector<Vector3> generateRandomPoints(size_t count) {
    std::vector<Vector3> points;
    points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        points.emplace_back(posDist(rng), posDist(rng), posDist(rng));
    }
    return points;
}

//------------------------------------------------------------------------------
// Character Animation Benchmarks
//------------------------------------------------------------------------------

// Benchmark bone chain rotations (e.g., arm or leg chain)
static void BM_BoneChainRotation(benchmark::State& state) {
    const size_t chainLength = state.range(0);
    auto boneRotations = generateRandomRotations(chainLength);
    Vector3 endPoint(1.0f, 0.0f, 0.0f);  // Initial point at unit X
    
    for (auto _ : state) {
        Vector3 result = endPoint;
        Quaternion accumulated = Quaternion();  // Identity
        
        // Accumulate rotations down the chain
        for (const auto& rotation : boneRotations) {
            accumulated = rotation * accumulated;
            result = accumulated.RotateVector(endPoint);
        }
        
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations() * chainLength);
}

// Benchmark skeletal animation blending
static void BM_AnimationBlending(benchmark::State& state) {
    const size_t numJoints = state.range(0);
    auto pose1 = generateRandomRotations(numJoints);  // First animation pose
    auto pose2 = generateRandomRotations(numJoints);  // Second animation pose
    
    const float blendSteps = 60.0f;  // 60 fps animation
    float blendFactor = 0.0f;
    const float blendIncrement = 1.0f / blendSteps;
    
    std::vector<Quaternion> blendedPose(numJoints);
    
    for (auto _ : state) {
        // Blend between poses using SLERP
        for (size_t i = 0; i < numJoints; ++i) {
            blendedPose[i] = Quaternion::Slerp(pose1[i], pose2[i], blendFactor);
        }
        
        benchmark::DoNotOptimize(blendedPose);
        
        // Update blend factor
        blendFactor += blendIncrement;
        if (blendFactor >= 1.0f) {
            blendFactor = 0.0f;
            // Swap poses to simulate continuous animation
            std::swap(pose1, pose2);
            pose2 = generateRandomRotations(numJoints);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * numJoints);
}

//------------------------------------------------------------------------------
// Camera Control Benchmarks
//------------------------------------------------------------------------------

// Benchmark first-person camera rotations
static void BM_FirstPersonCamera(benchmark::State& state) {
    const size_t numFrames = state.range(0);
    
    // Simulate mouse movements
    std::vector<float> yawDeltas(numFrames);
    std::vector<float> pitchDeltas(numFrames);
    
    std::uniform_real_distribution<float> mouseDist(-0.1f, 0.1f);
    for (size_t i = 0; i < numFrames; ++i) {
        yawDeltas[i] = mouseDist(rng);
        pitchDeltas[i] = mouseDist(rng) * 0.5f;  // Less pitch than yaw
    }
    
    Quaternion cameraRotation;  // Start at identity
    size_t frame = 0;
    
    for (auto _ : state) {
        // Update camera rotation based on mouse movement
        Quaternion yawRotation(Vector3(0.0f, 1.0f, 0.0f), yawDeltas[frame]);
        Quaternion pitchRotation(Vector3(1.0f, 0.0f, 0.0f), pitchDeltas[frame]);
        
        cameraRotation = yawRotation * pitchRotation * cameraRotation;
        cameraRotation.Normalize();  // Prevent drift
        
        // Rotate view direction
        Vector3 viewDir = cameraRotation.RotateVector(Vector3(0.0f, 0.0f, -1.0f));
        
        benchmark::DoNotOptimize(viewDir);
        frame = (frame + 1) % numFrames;
    }
    
    state.SetItemsProcessed(state.iterations());
}

// Benchmark orbit camera
static void BM_OrbitCamera(benchmark::State& state) {
    const size_t numFrames = state.range(0);
    
    // Setup orbit parameters
    Vector3 target(0.0f, 0.0f, 0.0f);
    float distance = 10.0f;
    float orbitSpeed = 0.01f;
    
    // Generate random orbit adjustments
    std::vector<Vector3> orbitAdjustments(numFrames);
    std::uniform_real_distribution<float> adjustDist(-0.05f, 0.05f);
    for (auto& adj : orbitAdjustments) {
        adj = Vector3(adjustDist(rng), adjustDist(rng), 0.0f);
    }
    
    Quaternion orbitRotation;
    size_t frame = 0;
    
    for (auto _ : state) {
        // Update orbit rotation
        Quaternion deltaRotation(Vector3(0.0f, 1.0f, 0.0f), orbitSpeed);
        orbitRotation = deltaRotation * orbitRotation;
        
        // Apply orbit adjustments
        Vector3 adjustment = orbitAdjustments[frame];
        Quaternion adjustX(Vector3(1.0f, 0.0f, 0.0f), adjustment.x);
        Quaternion adjustY(Vector3(0.0f, 1.0f, 0.0f), adjustment.y);
        orbitRotation = adjustY * adjustX * orbitRotation;
        
        // Calculate camera position
        Vector3 offset(0.0f, 0.0f, distance);
        Vector3 position = target + orbitRotation.RotateVector(offset);
        
        benchmark::DoNotOptimize(position);
        frame = (frame + 1) % numFrames;
    }
    
    state.SetItemsProcessed(state.iterations());
}

//------------------------------------------------------------------------------
// Physics Simulation Benchmarks
//------------------------------------------------------------------------------

// Benchmark rigid body orientation updates
static void BM_RigidBodyRotation(benchmark::State& state) {
    const size_t numBodies = state.range(0);
    auto orientations = generateRandomRotations(numBodies);
    
    // Generate random angular velocities
    std::vector<Vector3> angularVelocities(numBodies);
    std::uniform_real_distribution<float> velDist(-1.0f, 1.0f);
    for (auto& vel : angularVelocities) {
        vel = Vector3(velDist(rng), velDist(rng), velDist(rng));
    }
    
    const float dt = 1.0f / 60.0f;  // 60 fps physics
    
    for (auto _ : state) {
        for (size_t i = 0; i < numBodies; ++i) {
            // Convert angular velocity to rotation
            float angle = angularVelocities[i].length() * dt;
            if (angle > 0.0f) {
                Vector3 axis = angularVelocities[i].normalized();
                Quaternion deltaRot(axis, angle);
                
                // Update orientation
                orientations[i] = deltaRot * orientations[i];
                orientations[i].Normalize();
            }
        }
        benchmark::DoNotOptimize(orientations);
    }
    
    state.SetItemsProcessed(state.iterations() * numBodies);
}

// Benchmark collision response rotations
static void BM_CollisionResponse(benchmark::State& state) {
    const size_t numCollisions = state.range(0);
    auto orientations = generateRandomRotations(numCollisions);
    auto impactPoints = generateRandomPoints(numCollisions);
    auto impactNormals = generateRandomPoints(numCollisions);
    
    // Normalize impact normals
    for (auto& normal : impactNormals) {
        normal.normalize();
    }
    
    const float dt = 1.0f / 60.0f;
    const float restitution = 0.5f;
    
    for (auto _ : state) {
        for (size_t i = 0; i < numCollisions; ++i) {
            // Calculate rotation response from impact
            Vector3 rotationAxis = impactPoints[i].cross(impactNormals[i]);
            float rotationAngle = rotationAxis.length() * restitution * dt;
            
            if (rotationAngle > 0.0f) {
                rotationAxis.normalize();
                Quaternion response(rotationAxis, rotationAngle);
                orientations[i] = response * orientations[i];
                orientations[i].Normalize();
            }
        }
        benchmark::DoNotOptimize(orientations);
    }
    
    state.SetItemsProcessed(state.iterations() * numCollisions);
}

//------------------------------------------------------------------------------
// Visual Effects Benchmarks
//------------------------------------------------------------------------------

// Benchmark particle system orientation updates
static void BM_ParticleSystemRotation(benchmark::State& state) {
    const size_t numParticles = state.range(0);
    auto orientations = generateRandomRotations(numParticles);
    
    // Generate random rotation speeds
    std::vector<float> rotationSpeeds(numParticles);
    std::uniform_real_distribution<float> speedDist(0.1f, 2.0f);
    for (auto& speed : rotationSpeeds) {
        speed = speedDist(rng);
    }
    
    const float dt = 1.0f / 60.0f;
    Vector3 rotationAxis(0.0f, 1.0f, 0.0f);  // Common rotation axis
    
    for (auto _ : state) {
        for (size_t i = 0; i < numParticles; ++i) {
            float angle = rotationSpeeds[i] * dt;
            Quaternion deltaRot(rotationAxis, angle);
            orientations[i] = deltaRot * orientations[i];
        }
        benchmark::DoNotOptimize(orientations);
    }
    
    state.SetItemsProcessed(state.iterations() * numParticles);
}

// Benchmark smooth orientation transitions
static void BM_SmoothRotation(benchmark::State& state) {
    const size_t numObjects = state.range(0);
    auto currentOrientations = generateRandomRotations(numObjects);
    auto targetOrientations = generateRandomRotations(numObjects);
    
    std::vector<float> interpolationFactors(numObjects, 0.0f);
    const float transitionSpeed = 2.0f;  // Full transition in 0.5 seconds
    const float dt = 1.0f / 60.0f;
    
    for (auto _ : state) {
        for (size_t i = 0; i < numObjects; ++i) {
            // Update interpolation
            interpolationFactors[i] = std::min(1.0f, interpolationFactors[i] + dt * transitionSpeed);
            
            // Smoothly interpolate orientation
            currentOrientations[i] = Quaternion::Slerp(
                currentOrientations[i],
                targetOrientations[i],
                interpolationFactors[i]
            );
            
            // If transition complete, set new target
            if (interpolationFactors[i] >= 1.0f) {
                interpolationFactors[i] = 0.0f;
                currentOrientations[i] = targetOrientations[i];
                targetOrientations[i] = generateRandomRotation();
            }
        }
        benchmark::DoNotOptimize(currentOrientations);
    }
    
    state.SetItemsProcessed(state.iterations() * numObjects);
}

} // namespace

//------------------------------------------------------------------------------
// Register Benchmarks
//------------------------------------------------------------------------------

// Character Animation
BENCHMARK(BM_BoneChainRotation)
    ->RangeMultiplier(2)->Range(8, 32)  // Typical bone chain lengths
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_AnimationBlending)
    ->RangeMultiplier(2)->Range(32, 128)  // Typical skeleton sizes
    ->Unit(benchmark::kMicrosecond);

// Camera Control
BENCHMARK(BM_FirstPersonCamera)
    ->RangeMultiplier(2)->Range(60, 240)  // Frame counts at different refresh rates
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_OrbitCamera)
    ->RangeMultiplier(2)->Range(60, 240)
    ->Unit(benchmark::kMicrosecond);

// Physics Simulation
BENCHMARK(BM_RigidBodyRotation)
    ->RangeMultiplier(8)->Range(64, 1024)  // Number of physics bodies
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_CollisionResponse)
    ->RangeMultiplier(8)->Range(64, 1024)  // Number of collision responses
    ->Unit(benchmark::kMicrosecond);

// Visual Effects
BENCHMARK(BM_ParticleSystemRotation)
    ->RangeMultiplier(8)->Range(128, 8<<10)  // Particle counts
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SmoothRotation)
    ->RangeMultiplier(8)->Range(64, 1024)  // Number of interpolating objects
    ->Unit(benchmark::kMicrosecond);