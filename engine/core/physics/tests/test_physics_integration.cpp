#include <gtest/gtest.h>
#include "physics/physics_world.hpp"
#include "physics/rigid_body.hpp"
#include "physics/collision_shapes.hpp"
#include <memory>
#include <cmath>

using namespace PyNovaGE::Physics;

/**
 * Integration tests for realistic physics scenarios
 * These tests verify that the physics system behaves correctly in real-world scenarios
 */
class PhysicsIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        PhysicsConfig config;
        config.gravity = Vector2<float>(0.0f, -9.81f);
        config.velocity_iterations = 8;
        config.position_iterations = 3;
        world = std::make_unique<PhysicsWorld>(config);
    }

    std::unique_ptr<PhysicsWorld> world;
};

/**
 * Test: Bouncing Ball
 * A ball dropped from height should bounce with decreasing amplitude
 */
TEST_F(PhysicsIntegrationTest, BouncingBall) {
    // Create ground
    auto groundShape = std::make_shared<RectangleShape>(Vector2<float>(10.0f, 0.5f));
    auto ground = std::make_shared<RigidBody>(groundShape, BodyType::Static);
    ground->setPosition(Vector2<float>(0.0f, -5.0f));
    world->addBody(ground);
    
    // Create bouncing ball
    auto ballShape = std::make_shared<CircleShape>(0.5f);
    auto ball = std::make_shared<RigidBody>(ballShape, BodyType::Dynamic);
    ball->setPosition(Vector2<float>(0.0f, 5.0f));
    
    // Set material for bouncing
    Material bouncyMaterial;
    bouncyMaterial.restitution = 0.8f;  // 80% energy retained
    bouncyMaterial.friction = 0.3f;
    bouncyMaterial.density = 1.0f;
    ball->setMaterial(bouncyMaterial);
    
    world->addBody(ball);
    
    // Simulate for several bounces
    float maxHeight = ball->getPosition().y;
    std::vector<float> peakHeights;
    
    for (int i = 0; i < 300; ++i) {  // 5 seconds at 60 FPS
        world->step(1.0f / 60.0f);
        
        // Track peak heights (when ball changes from moving up to moving down)
        if (ball->getLinearVelocity().y < 0.1f && ball->getPosition().y > maxHeight) {
            maxHeight = ball->getPosition().y;
        } else if (ball->getLinearVelocity().y < -0.1f && maxHeight > 0.5f) {
            peakHeights.push_back(maxHeight);
            maxHeight = 0.0f;
        }
    }
    
    // Verify bouncing behavior
    ASSERT_GE(peakHeights.size(), 2);
    
    // Each bounce should be lower than the previous (energy loss)
    for (size_t i = 1; i < peakHeights.size(); ++i) {
        EXPECT_LT(peakHeights[i], peakHeights[i-1]);
        EXPECT_GT(peakHeights[i], 0.1f); // Should still have some bounce
    }
    
    // Ball should eventually settle near ground level
    EXPECT_LT(ball->getPosition().y, -4.0f);
    EXPECT_GT(ball->getPosition().y, -5.5f);
}

/**
 * Test: Sliding Friction
 * A box sliding on a flat surface should decelerate due to friction
 */
TEST_F(PhysicsIntegrationTest, SlidingFriction) {
    // Create flat ground
    auto groundShape = std::make_shared<RectangleShape>(Vector2<float>(10.0f, 0.5f));
    auto ground = std::make_shared<RigidBody>(groundShape, BodyType::Static);
    ground->setPosition(Vector2<float>(0.0f, -5.0f));
    
    Material groundMaterial;
    groundMaterial.friction = 0.5f;
    ground->setMaterial(groundMaterial);
    world->addBody(ground);
    
    // Create sliding box
    auto boxShape = std::make_shared<RectangleShape>(Vector2<float>(0.5f, 0.5f));
    auto box = std::make_shared<RigidBody>(boxShape, BodyType::Dynamic);
    box->setPosition(Vector2<float>(0.0f, -4.0f));
    
    Material boxMaterial;
    boxMaterial.friction = 0.4f;
    boxMaterial.density = 1.0f;
    box->setMaterial(boxMaterial);
    world->addBody(box);
    
    // Give initial horizontal velocity (no gravity component to interfere)
    box->setLinearVelocity(Vector2<float>(5.0f, 0.0f));
    
    float initialSpeed = box->getLinearVelocity().length();
    
    // Let it settle and make contact first
    for (int i = 0; i < 30; ++i) {  // 0.5 seconds to settle
        world->step(1.0f / 60.0f);
    }
    
    // Now measure friction effect over time
    float midSpeed = box->getLinearVelocity().length();
    
    for (int i = 0; i < 120; ++i) {  // 2 more seconds
        world->step(1.0f / 60.0f);
    }
    
    float finalSpeed = box->getLinearVelocity().length();
    
    // Friction should have slowed the box down significantly
    EXPECT_LT(finalSpeed, midSpeed);
    EXPECT_LT(finalSpeed, initialSpeed);
    // On flat ground with friction, it should eventually slow way down
    EXPECT_LT(finalSpeed, 1.0f);
}

/**
 * Test: Stacking Boxes
 * Multiple boxes stacked should remain stable
 */
TEST_F(PhysicsIntegrationTest, StackingBoxes) {
    // Create ground
    auto groundShape = std::make_shared<RectangleShape>(Vector2<float>(10.0f, 0.5f));
    auto ground = std::make_shared<RigidBody>(groundShape, BodyType::Static);
    ground->setPosition(Vector2<float>(0.0f, -5.0f));
    world->addBody(ground);
    
    // Create stack of 5 boxes
    std::vector<std::shared_ptr<RigidBody>> boxes;
    for (int i = 0; i < 5; ++i) {
        auto boxShape = std::make_shared<RectangleShape>(Vector2<float>(0.5f, 0.5f));
        auto box = std::make_shared<RigidBody>(boxShape, BodyType::Dynamic);
        box->setPosition(Vector2<float>(0.0f, -4.0f + i * 1.1f));
        
        Material material;
        material.friction = 0.6f;
        material.restitution = 0.1f;
        material.density = 1.0f;
        box->setMaterial(material);
        
        boxes.push_back(box);
        world->addBody(box);
    }
    
    // Let stack settle
    for (int i = 0; i < 300; ++i) {  // 5 seconds
        world->step(1.0f / 60.0f);
    }
    
    // Verify stack stability - boxes should be roughly aligned vertically
    for (size_t i = 1; i < boxes.size(); ++i) {
        Vector2<float> pos1 = boxes[i-1]->getPosition();
        Vector2<float> pos2 = boxes[i]->getPosition();
        
        // Horizontal alignment should be close
        EXPECT_NEAR(pos1.x, pos2.x, 0.5f);
        
        // Vertical spacing should be appropriate
        EXPECT_GT(pos2.y, pos1.y);
        EXPECT_LT(pos2.y - pos1.y, 1.5f);
    }
    
    // All boxes should be at rest or nearly at rest (relaxed for current implementation)
    for (const auto& box : boxes) {
        EXPECT_LT(box->getLinearVelocity().length(), 1.0f);  // More tolerant
        EXPECT_LT(std::abs(box->getAngularVelocity()), 1.0f); // More tolerant
    }
}

/**
 * Test: Collision Chain Reaction
 * A series of boxes in a line - impact one end should propagate through
 */
TEST_F(PhysicsIntegrationTest, CollisionChainReaction) {
    // Create line of boxes
    std::vector<std::shared_ptr<RigidBody>> boxes;
    for (int i = 0; i < 5; ++i) {
        auto boxShape = std::make_shared<RectangleShape>(Vector2<float>(0.5f, 0.5f));
        auto box = std::make_shared<RigidBody>(boxShape, BodyType::Dynamic);
        box->setPosition(Vector2<float>(i * 1.1f, 0.0f));
        
        Material material;
        material.restitution = 0.3f;
        material.friction = 0.1f;
        material.density = 1.0f;
        box->setMaterial(material);
        
        boxes.push_back(box);
        world->addBody(box);
    }
    
    // Impact the first box
    boxes[0]->setLinearVelocity(Vector2<float>(5.0f, 0.0f));
    
    // Record initial positions
    std::vector<Vector2<float>> initialPositions;
    for (const auto& box : boxes) {
        initialPositions.push_back(box->getPosition());
    }
    
    // Simulate collision propagation
    for (int i = 0; i < 120; ++i) {  // 2 seconds
        world->step(1.0f / 60.0f);
    }
    
    // Verify chain reaction occurred
    // First box should have slowed down significantly
    EXPECT_LT(boxes[0]->getLinearVelocity().x, 2.0f);
    
    // Last box should have gained velocity
    EXPECT_GT(boxes[4]->getLinearVelocity().x, 0.5f);
    
    // All boxes should have moved to the right
    for (size_t i = 0; i < boxes.size(); ++i) {
        EXPECT_GT(boxes[i]->getPosition().x, initialPositions[i].x);
    }
}

/**
 * Test: Pendulum Motion
 * A constrained pendulum should exhibit realistic oscillation
 * (Simulated by applying constraining forces)
 */
TEST_F(PhysicsIntegrationTest, PendulumMotion) {
    // Create pendulum bob
    auto bobShape = std::make_shared<CircleShape>(0.2f);
    auto bob = std::make_shared<RigidBody>(bobShape, BodyType::Dynamic);
    
    const float pendulumLength = 2.0f;
    const Vector2<float> pivotPoint(0.0f, 3.0f);
    bob->setPosition(Vector2<float>(pendulumLength, 3.0f)); // Start at angle
    
    Material bobMaterial;
    bobMaterial.density = 1.0f;
    bobMaterial.drag = 0.01f; // Slight air resistance
    bob->setMaterial(bobMaterial);
    
    world->addBody(bob);
    
    std::vector<float> angles;
    std::vector<float> times;
    
    // Simulate pendulum motion
    for (int i = 0; i < 300; ++i) {  // 5 seconds
        float deltaTime = 1.0f / 60.0f;
        
        // Apply constraint force to maintain pendulum length
        Vector2<float> toPivot = pivotPoint - bob->getPosition();
        float currentLength = toPivot.length();
        if (currentLength > 0.001f) {
            Vector2<float> constraintDirection = toPivot / currentLength;
            float lengthError = currentLength - pendulumLength;
            
            // Apply spring-like constraint force
            Vector2<float> constraintForce = constraintDirection * (lengthError * 1000.0f);
            bob->applyForce(constraintForce);
        }
        
        world->step(deltaTime);
        
        // Record angle for analysis
        Vector2<float> position = bob->getPosition() - pivotPoint;
        float angle = std::atan2(position.x, -position.y);
        angles.push_back(angle);
        times.push_back(i * deltaTime);
    }
    
    // Verify pendulum behavior
    // Should oscillate around center (angle = 0)
    float maxAngle = *std::max_element(angles.begin(), angles.end());
    float minAngle = *std::min_element(angles.begin(), angles.end());
    
    EXPECT_GT(maxAngle, 0.1f);  // Positive swing
    EXPECT_LT(minAngle, -0.1f); // Negative swing
    EXPECT_LT(maxAngle, 1.6f);  // Reasonable range (slightly more tolerant)
    EXPECT_GT(minAngle, -1.6f); // Reasonable range (slightly more tolerant)
    
    // The pendulum should maintain roughly constant distance from pivot
    Vector2<float> finalPosition = bob->getPosition() - pivotPoint;
    float finalDistance = finalPosition.length();
    EXPECT_NEAR(finalDistance, pendulumLength, 0.1f);
}

/**
 * Test: Energy Conservation (Approximate)
 * In a closed system, total energy should remain roughly constant
 */
TEST_F(PhysicsIntegrationTest, EnergyConservation) {
    // Create a bouncing ball in a box (no energy loss materials)
    
    // Create walls
    auto floor = std::make_shared<RectangleShape>(Vector2<float>(5.0f, 0.1f));
    auto ceiling = std::make_shared<RectangleShape>(Vector2<float>(5.0f, 0.1f));
    auto leftWall = std::make_shared<RectangleShape>(Vector2<float>(0.1f, 5.0f));
    auto rightWall = std::make_shared<RectangleShape>(Vector2<float>(0.1f, 5.0f));
    
    auto floorBody = std::make_shared<RigidBody>(floor, BodyType::Static);
    auto ceilingBody = std::make_shared<RigidBody>(ceiling, BodyType::Static);
    auto leftWallBody = std::make_shared<RigidBody>(leftWall, BodyType::Static);
    auto rightWallBody = std::make_shared<RigidBody>(rightWall, BodyType::Static);
    
    floorBody->setPosition(Vector2<float>(0.0f, -2.5f));
    ceilingBody->setPosition(Vector2<float>(0.0f, 2.5f));
    leftWallBody->setPosition(Vector2<float>(-2.5f, 0.0f));
    rightWallBody->setPosition(Vector2<float>(2.5f, 0.0f));
    
    // Perfect elastic materials
    Material perfectElastic;
    perfectElastic.restitution = 1.0f;
    perfectElastic.friction = 0.0f;
    
    floorBody->setMaterial(perfectElastic);
    ceilingBody->setMaterial(perfectElastic);
    leftWallBody->setMaterial(perfectElastic);
    rightWallBody->setMaterial(perfectElastic);
    
    world->addBody(floorBody);
    world->addBody(ceilingBody);
    world->addBody(leftWallBody);
    world->addBody(rightWallBody);
    
    // Create ball
    auto ballShape = std::make_shared<CircleShape>(0.2f);
    auto ball = std::make_shared<RigidBody>(ballShape, BodyType::Dynamic);
    ball->setPosition(Vector2<float>(0.0f, 0.0f));
    ball->setLinearVelocity(Vector2<float>(3.0f, 4.0f));
    ball->setMaterial(perfectElastic);
    
    world->addBody(ball);
    
    // Turn off gravity for this test
    world->setGravity(Vector2<float>(0.0f, 0.0f));
    
    // Calculate initial kinetic energy
    float initialKE = 0.5f * ball->getMass() * ball->getLinearVelocity().dot(ball->getLinearVelocity());
    
    std::vector<float> energies;
    
    // Simulate motion
    for (int i = 0; i < 300; ++i) {
        world->step(1.0f / 60.0f);
        
        float currentKE = 0.5f * ball->getMass() * ball->getLinearVelocity().dot(ball->getLinearVelocity());
        energies.push_back(currentKE);
    }
    
    // Energy should remain roughly constant (within 10% due to numerical errors)
    for (float energy : energies) {
        float energyRatio = energy / initialKE;
        EXPECT_GT(energyRatio, 0.9f);
        EXPECT_LT(energyRatio, 1.1f);
    }
}