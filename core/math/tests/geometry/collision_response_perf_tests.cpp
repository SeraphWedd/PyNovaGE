#include "geometry/collision_response.hpp"
#include "geometry/intersection.hpp"
#include <benchmark/benchmark.h>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

static void BM_SphereCollisionResponse(benchmark::State& state) {
    // Setup test objects
    Sphere sphere1(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    Sphere sphere2(Vector3(2.0f, 0.0f, 0.0f), 1.0f);
    
    MaterialProperties material;
    material.restitution = 0.5f;
    material.friction = 0.3f;
    material.density = 1.0f;
    
    RigidBodyProperties props1 = RigidBodyProperties::forSphere(1.0f, material);
    RigidBodyProperties props2 = RigidBodyProperties::forSphere(1.0f, material);
    
    props1.linear_velocity = Vector3(1.0f, 0.0f, 0.0f);
    props2.linear_velocity = Vector3(-1.0f, 0.0f, 0.0f);
    
    IntersectionResult contact;
    contact.intersects = true;
    contact.point = Vector3(1.0f, 0.0f, 0.0f);
    contact.normal = Vector3(1.0f, 0.0f, 0.0f);
    
    // Run benchmark
    for (auto _ : state) {
        CollisionResponse response = calculateSphereResponse(sphere1, sphere2, props1, props2, contact);
        benchmark::DoNotOptimize(response);
    }
}
BENCHMARK(BM_SphereCollisionResponse);

static void BM_BoxCollisionResponse(benchmark::State& state) {
    // Setup test objects
    AABB box1(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB box2(Vector3(1.0f, -1.0f, -1.0f), Vector3(3.0f, 1.0f, 1.0f));
    
    MaterialProperties material;
    material.restitution = 0.5f;
    material.friction = 0.3f;
    material.density = 1.0f;
    
    RigidBodyProperties props1 = RigidBodyProperties::forBox(Vector3(2.0f, 2.0f, 2.0f), material);
    RigidBodyProperties props2 = RigidBodyProperties::forBox(Vector3(2.0f, 2.0f, 2.0f), material);
    
    props1.linear_velocity = Vector3(1.0f, 0.0f, 0.0f);
    props1.angular_velocity = Vector3(0.0f, 0.0f, 1.0f);
    
    props2.linear_velocity = Vector3(-1.0f, 0.0f, 0.0f);
    props2.angular_velocity = Vector3(0.0f, 0.0f, -1.0f);
    
    IntersectionResult contact;
    contact.intersects = true;
    contact.point = Vector3(0.0f, 0.0f, 0.0f);
    contact.normal = Vector3(1.0f, 0.0f, 0.0f);
    
    // Run benchmark
    for (auto _ : state) {
        CollisionResponse response = calculateBoxResponse(box1, box2, props1, props2, contact);
        benchmark::DoNotOptimize(response);
    }
}
BENCHMARK(BM_BoxCollisionResponse);

static void BM_SphereBoxCollisionResponse(benchmark::State& state) {
    // Setup test objects
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    AABB box(Vector3(1.0f, -1.0f, -1.0f), Vector3(3.0f, 1.0f, 1.0f));
    
    MaterialProperties material;
    material.restitution = 0.5f;
    material.friction = 0.3f;
    material.density = 1.0f;
    
    RigidBodyProperties sphere_props = RigidBodyProperties::forSphere(1.0f, material);
    RigidBodyProperties box_props = RigidBodyProperties::forBox(Vector3(2.0f, 2.0f, 2.0f), material);
    
    sphere_props.linear_velocity = Vector3(1.0f, 0.0f, 0.0f);
    sphere_props.angular_velocity = Vector3(0.0f, 0.0f, 1.0f);
    
    box_props.linear_velocity = Vector3(-1.0f, 0.0f, 0.0f);
    box_props.angular_velocity = Vector3(0.0f, 0.0f, -1.0f);
    
    IntersectionResult contact;
    contact.intersects = true;
    contact.point = Vector3(1.0f, 0.0f, 0.0f);
    contact.normal = Vector3(1.0f, 0.0f, 0.0f);
    
    // Run benchmark
    for (auto _ : state) {
        CollisionResponse response = calculateSphereBoxResponse(
            sphere, box, sphere_props, box_props, contact);
        benchmark::DoNotOptimize(response);
    }
}
BENCHMARK(BM_SphereBoxCollisionResponse);

static void BM_ApplyCollisionResponse(benchmark::State& state) {
    // Setup test objects
    MaterialProperties material;
    material.restitution = 0.5f;
    material.friction = 0.3f;
    material.density = 1.0f;
    
    RigidBodyProperties props = RigidBodyProperties::forSphere(1.0f, material);
    props.linear_velocity = Vector3(1.0f, 0.0f, 0.0f);
    props.angular_velocity = Vector3(0.0f, 0.0f, 1.0f);
    
    CollisionResponse response;
    response.linear_impulse = Vector3(-0.5f, 0.0f, 0.0f);
    response.angular_impulse = Vector3(0.0f, 0.0f, -0.5f);
    response.friction_impulse = Vector3(0.0f, -0.3f, 0.0f);
    
    // Run benchmark
    for (auto _ : state) {
        RigidBodyProperties test_props = props;  // Copy to avoid accumulation
        applyCollisionResponse(response, test_props, 1.0f/60.0f);
        benchmark::DoNotOptimize(test_props);
    }
}
BENCHMARK(BM_ApplyCollisionResponse);

BENCHMARK_MAIN();