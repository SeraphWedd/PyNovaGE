#include "geometry/collision_response.hpp"
#include "geometry/intersection.hpp"
#include <gtest/gtest.h>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

class CollisionResponseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up common material properties
        elastic_material.restitution = 1.0f;    // Perfectly elastic
        elastic_material.friction = 0.0f;       // No friction
        elastic_material.density = 1.0f;        // Unit density
        
        inelastic_material.restitution = 0.0f;  // Perfectly inelastic
        inelastic_material.friction = 0.5f;     // Moderate friction
        inelastic_material.density = 1.0f;      // Unit density
    }
    
    MaterialProperties elastic_material;
    MaterialProperties inelastic_material;
    
    // Helper to check energy conservation
    void verifyEnergyConservation(
        const RigidBodyProperties& props1_before,
        const RigidBodyProperties& props2_before,
        const RigidBodyProperties& props1_after,
        const RigidBodyProperties& props2_after,
        const CollisionResponse& response,
        float tolerance = 1e-4f)
    {
        // Calculate kinetic energy before collision
        float ke1_before = 0.5f * props1_before.mass * props1_before.linear_velocity.lengthSquared();
        float ke2_before = 0.5f * props2_before.mass * props2_before.linear_velocity.lengthSquared();
        
        // Add rotational energy
        float re1_before = 0.5f * props1_before.angular_velocity.dot(
            props1_before.inertia_tensor * props1_before.angular_velocity);
        float re2_before = 0.5f * props2_before.angular_velocity.dot(
            props2_before.inertia_tensor * props2_before.angular_velocity);
        
        float total_energy_before = ke1_before + ke2_before + re1_before + re2_before;
        
        // Calculate kinetic energy after collision
        float ke1_after = 0.5f * props1_after.mass * props1_after.linear_velocity.lengthSquared();
        float ke2_after = 0.5f * props2_after.mass * props2_after.linear_velocity.lengthSquared();
        
        // Add rotational energy
        float re1_after = 0.5f * props1_after.angular_velocity.dot(
            props1_after.inertia_tensor * props1_after.angular_velocity);
        float re2_after = 0.5f * props2_after.angular_velocity.dot(
            props2_after.inertia_tensor * props2_after.angular_velocity);
        
        float total_energy_after = ke1_after + ke2_after + re1_after + re2_after;
        
        // Energy after + energy loss should equal energy before
        EXPECT_NEAR(total_energy_before, total_energy_after + response.energy_loss, tolerance);
    }
};

TEST_F(CollisionResponseTest, ElasticSphereCollision) {
    // Set up two spheres
    Sphere sphere1(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    Sphere sphere2(Vector3(2.0f, 0.0f, 0.0f), 1.0f);
    
    // Create properties with elastic material
    RigidBodyProperties props1 = RigidBodyProperties::forSphere(1.0f, elastic_material);
    RigidBodyProperties props2 = RigidBodyProperties::forSphere(1.0f, elastic_material);
    
    // Set initial velocities (head-on collision)
    props1.linear_velocity = Vector3(1.0f, 0.0f, 0.0f);
    props2.linear_velocity = Vector3(-1.0f, 0.0f, 0.0f);
    
    // Simulate intersection at contact point
    IntersectionResult contact;
    contact.intersects = true;
    contact.point = Vector3(1.0f, 0.0f, 0.0f);
    contact.normal = Vector3(1.0f, 0.0f, 0.0f);
    
    // Store initial properties
    RigidBodyProperties props1_before = props1;
    RigidBodyProperties props2_before = props2;
    
    // Calculate and apply response
    CollisionResponse response = calculateSphereResponse(sphere1, sphere2, props1, props2, contact);
    applyCollisionResponse(response, props1, 1.0f);
    
    // For elastic collision, velocities should be perfectly reversed
    EXPECT_NEAR(props1.linear_velocity.x, -1.0f, 1e-4f);
    EXPECT_NEAR(props2.linear_velocity.x, 1.0f, 1e-4f);
    
    // No energy should be lost in elastic collision
    EXPECT_NEAR(response.energy_loss, 0.0f, 1e-4f);
    
    // Verify energy conservation
    verifyEnergyConservation(props1_before, props2_before, props1, props2, response);
}

TEST_F(CollisionResponseTest, InelasticSphereCollision) {
    // Set up two spheres with inelastic material
    Sphere sphere1(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    Sphere sphere2(Vector3(2.0f, 0.0f, 0.0f), 1.0f);
    
    // Create properties with inelastic material
    RigidBodyProperties props1 = RigidBodyProperties::forSphere(1.0f, inelastic_material);
    RigidBodyProperties props2 = RigidBodyProperties::forSphere(1.0f, inelastic_material);
    
    // Set initial velocities
    props1.linear_velocity = Vector3(1.0f, 0.0f, 0.0f);
    props2.linear_velocity = Vector3(-1.0f, 0.0f, 0.0f);
    
    IntersectionResult contact;
    contact.intersects = true;
    contact.point = Vector3(1.0f, 0.0f, 0.0f);
    contact.normal = Vector3(1.0f, 0.0f, 0.0f);
    
    RigidBodyProperties props1_before = props1;
    RigidBodyProperties props2_before = props2;
    
    // Calculate and apply response
    CollisionResponse response = calculateSphereResponse(sphere1, sphere2, props1, props2, contact);
    applyCollisionResponse(response, props1, 1.0f);
    applyCollisionResponse(response, props2, 1.0f);
    
    // For perfectly inelastic collision, objects should stick together
    EXPECT_NEAR(props1.linear_velocity.x, 0.0f, 1e-4f);
    EXPECT_NEAR(props2.linear_velocity.x, 0.0f, 1e-4f);
    
    // There should be significant energy loss
    EXPECT_GT(response.energy_loss, 0.0f);
    
    // Verify energy conservation including loss
    verifyEnergyConservation(props1_before, props2_before, props1, props2, response);
}

TEST_F(CollisionResponseTest, BoxCollisionRotation) {
    // Set up two boxes
    AABB box1(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB box2(Vector3(1.0f, -1.0f, -1.0f), Vector3(3.0f, 1.0f, 1.0f));
    
    // Create properties with mixed elasticity
    MaterialProperties mixed_material;
    mixed_material.restitution = 0.5f;
    mixed_material.friction = 0.3f;
    mixed_material.density = 1.0f;
    
    RigidBodyProperties props1 = RigidBodyProperties::forBox(Vector3(2.0f, 2.0f, 2.0f), mixed_material);
    RigidBodyProperties props2 = RigidBodyProperties::forBox(Vector3(2.0f, 2.0f, 2.0f), mixed_material);
    
    // Set initial velocities with rotation
    props1.linear_velocity = Vector3(1.0f, 0.0f, 0.0f);
    props1.angular_velocity = Vector3(0.0f, 0.0f, 1.0f);
    
    props2.linear_velocity = Vector3(-0.5f, 0.0f, 0.0f);
    props2.angular_velocity = Vector3(0.0f, 0.0f, -0.5f);
    
    IntersectionResult contact;
    contact.intersects = true;
    contact.point = Vector3(0.0f, 0.0f, 0.0f);
    contact.normal = Vector3(1.0f, 0.0f, 0.0f);
    
    RigidBodyProperties props1_before = props1;
    RigidBodyProperties props2_before = props2;
    
    // Calculate and apply response
    CollisionResponse response = calculateBoxResponse(box1, box2, props1, props2, contact);
    applyCollisionResponse(response, props1, 1.0f);
    applyCollisionResponse(response, props2, 1.0f);
    
    // Should have both linear and angular response
    EXPECT_NE(props1.linear_velocity, props1_before.linear_velocity);
    EXPECT_NE(props1.angular_velocity, props1_before.angular_velocity);
    EXPECT_NE(props2.linear_velocity, props2_before.linear_velocity);
    EXPECT_NE(props2.angular_velocity, props2_before.angular_velocity);
    
    // Verify energy conservation
    verifyEnergyConservation(props1_before, props2_before, props1, props2, response);
}

TEST_F(CollisionResponseTest, SphereBoxCollision) {
    // Set up sphere and box
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    AABB box(Vector3(1.0f, -1.0f, -1.0f), Vector3(3.0f, 1.0f, 1.0f));
    
    // Create properties with mixed elasticity
    MaterialProperties mixed_material;
    mixed_material.restitution = 0.5f;
    mixed_material.friction = 0.3f;
    mixed_material.density = 1.0f;
    
    RigidBodyProperties sphere_props = RigidBodyProperties::forSphere(1.0f, mixed_material);
    RigidBodyProperties box_props = RigidBodyProperties::forBox(Vector3(2.0f, 2.0f, 2.0f), mixed_material);
    
    // Set initial velocities
    sphere_props.linear_velocity = Vector3(1.0f, 0.0f, 0.0f);
    sphere_props.angular_velocity = Vector3(0.0f, 0.0f, 1.0f);
    
    box_props.linear_velocity = Vector3(-0.5f, 0.0f, 0.0f);
    box_props.angular_velocity = Vector3(0.0f, 0.0f, -0.5f);
    
    IntersectionResult contact;
    contact.intersects = true;
    contact.point = Vector3(1.0f, 0.0f, 0.0f);
    contact.normal = Vector3(1.0f, 0.0f, 0.0f);
    
    RigidBodyProperties sphere_props_before = sphere_props;
    RigidBodyProperties box_props_before = box_props;
    
    // Calculate and apply response
    CollisionResponse response = calculateSphereBoxResponse(sphere, box, sphere_props, box_props, contact);
    applyCollisionResponse(response, sphere_props, 1.0f);
    applyCollisionResponse(response, box_props, 1.0f);
    
    // Should have both linear and angular response
    EXPECT_NE(sphere_props.linear_velocity, sphere_props_before.linear_velocity);
    EXPECT_NE(sphere_props.angular_velocity, sphere_props_before.angular_velocity);
    EXPECT_NE(box_props.linear_velocity, box_props_before.linear_velocity);
    EXPECT_NE(box_props.angular_velocity, box_props_before.angular_velocity);
    
    // Verify energy conservation
    verifyEnergyConservation(sphere_props_before, box_props_before, sphere_props, box_props, response);
}

TEST_F(CollisionResponseTest, FrictionEffect) {
    // Set up two boxes with high friction
    AABB box1(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB box2(Vector3(1.0f, -1.0f, -1.0f), Vector3(3.0f, 1.0f, 1.0f));
    
    MaterialProperties high_friction;
    high_friction.restitution = 0.5f;
    high_friction.friction = 0.8f;
    high_friction.density = 1.0f;
    
    RigidBodyProperties props1 = RigidBodyProperties::forBox(Vector3(2.0f, 2.0f, 2.0f), high_friction);
    RigidBodyProperties props2 = RigidBodyProperties::forBox(Vector3(2.0f, 2.0f, 2.0f), high_friction);
    
    // Set initial velocities with tangential component
    props1.linear_velocity = Vector3(1.0f, 1.0f, 0.0f);  // Moving diagonally
    props2.linear_velocity = Vector3(-1.0f, -1.0f, 0.0f);
    
    IntersectionResult contact;
    contact.intersects = true;
    contact.point = Vector3(0.0f, 0.0f, 0.0f);
    contact.normal = Vector3(1.0f, 0.0f, 0.0f);  // Normal along x-axis
    
    // Calculate response
    CollisionResponse response = calculateBoxResponse(box1, box2, props1, props2, contact);
    
    // Should have significant friction impulse
    EXPECT_GT(response.friction_impulse.length(), 0.1f);
    
    // Friction should act primarily in y direction (tangent to collision)
    EXPECT_GT(std::abs(response.friction_impulse.y), std::abs(response.friction_impulse.x));
}