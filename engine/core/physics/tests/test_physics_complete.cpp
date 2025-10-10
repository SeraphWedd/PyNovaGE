#include <gtest/gtest.h>
#include "physics/physics.hpp"
#include <memory>

using namespace PyNovaGE::Physics;

class PhysicsSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.gravity = Vector2<float>(0.0f, -9.81f);
        config_.velocity_iterations = 8;
        config_.position_iterations = 3;
        config_.enable_sleeping = true;
        
        world_ = std::make_unique<PhysicsWorld>(config_);
    }

    void TearDown() override {
        world_.reset();
    }

    PhysicsConfig config_;
    std::unique_ptr<PhysicsWorld> world_;
};

// Test basic rigid body creation and properties
TEST_F(PhysicsSystemTest, RigidBodyBasics) {
    auto shape = std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f));
    auto body = std::make_shared<RigidBody>(shape, BodyType::Dynamic);
    
    EXPECT_EQ(body->getBodyType(), BodyType::Dynamic);
    EXPECT_TRUE(body->isAwake());
    EXPECT_TRUE(body->isActive());
    EXPECT_FALSE(body->isStatic());
    
    body->setPosition(Vector2<float>(1.0f, 2.0f));
    EXPECT_FLOAT_EQ(body->getPosition().x, 1.0f);
    EXPECT_FLOAT_EQ(body->getPosition().y, 2.0f);
    
    body->setMass(5.0f);
    EXPECT_FLOAT_EQ(body->getMass(), 5.0f);
    EXPECT_GT(body->getInverseMass(), 0.0f);
}

// Test static body properties
TEST_F(PhysicsSystemTest, StaticBodyProperties) {
    auto shape = std::make_shared<RectangleShape>(Vector2<float>(10.0f, 1.0f));
    auto body = std::make_shared<RigidBody>(shape, BodyType::Static);
    
    EXPECT_TRUE(body->isStatic());
    EXPECT_FLOAT_EQ(body->getMass(), 0.0f);
    EXPECT_FLOAT_EQ(body->getInverseMass(), 0.0f);
    EXPECT_FLOAT_EQ(body->getInertia(), 0.0f);
    EXPECT_FLOAT_EQ(body->getInverseInertia(), 0.0f);
}

// Test material properties
TEST_F(PhysicsSystemTest, MaterialProperties) {
    auto shape = std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f));
    auto body = std::make_shared<RigidBody>(shape, BodyType::Dynamic);
    
    Material rubber = Materials::RUBBER;
    body->setMaterial(rubber);
    
    const auto& material = body->getMaterial();
    EXPECT_FLOAT_EQ(material.density, 1.2f);
    EXPECT_FLOAT_EQ(material.restitution, 0.9f);
    EXPECT_FLOAT_EQ(material.friction, 0.8f);
    EXPECT_FLOAT_EQ(material.drag, 0.05f);
}

// Test world body management
TEST_F(PhysicsSystemTest, WorldBodyManagement) {
    EXPECT_EQ(world_->getBodyCount(), 0);
    
    auto body1 = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f)),
        BodyType::Dynamic
    );
    auto body2 = std::make_shared<RigidBody>(
        std::make_shared<CircleShape>(0.5f),
        BodyType::Dynamic
    );
    
    world_->addBody(body1);
    EXPECT_EQ(world_->getBodyCount(), 1);
    
    world_->addBody(body2);
    EXPECT_EQ(world_->getBodyCount(), 2);
    
    world_->removeBody(body1);
    EXPECT_EQ(world_->getBodyCount(), 1);
    
    world_->clear();
    EXPECT_EQ(world_->getBodyCount(), 0);
}

// Test gravity application
TEST_F(PhysicsSystemTest, GravityApplication) {
    auto body = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f)),
        BodyType::Dynamic
    );
    body->setPosition(Vector2<float>(0.0f, 10.0f));
    body->setMass(1.0f);
    
    world_->addBody(body);
    
    // Step the simulation
    float deltaTime = 1.0f / 60.0f;
    world_->step(deltaTime);
    
    // Body should have fallen due to gravity
    EXPECT_LT(body->getPosition().y, 10.0f);
    EXPECT_LT(body->getLinearVelocity().y, 0.0f);
}

// Test collision detection between rectangles
TEST_F(PhysicsSystemTest, RectangleCollision) {
    auto body1 = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f)),
        BodyType::Dynamic
    );
    body1->setPosition(Vector2<float>(0.0f, 5.0f));
    body1->setLinearVelocity(Vector2<float>(0.0f, -10.0f));
    
    auto body2 = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(10.0f, 1.0f)),
        BodyType::Static
    );
    body2->setPosition(Vector2<float>(0.0f, -1.0f));
    
    world_->addBody(body1);
    world_->addBody(body2);
    
    // Step simulation until collision
    for (int i = 0; i < 120; ++i) {  // 2 seconds at 60 FPS
        world_->step(1.0f / 60.0f);
    }
    
    // Body should have bounced or stopped
    EXPECT_GT(body1->getPosition().y, -1.0f);  // Should be above ground
}

// Test circle collision
TEST_F(PhysicsSystemTest, CircleCollision) {
    auto circle1 = std::make_shared<RigidBody>(
        std::make_shared<CircleShape>(0.5f),
        BodyType::Dynamic
    );
    circle1->setPosition(Vector2<float>(-1.0f, 0.0f));
    circle1->setLinearVelocity(Vector2<float>(5.0f, 0.0f));
    
    auto circle2 = std::make_shared<RigidBody>(
        std::make_shared<CircleShape>(0.5f),
        BodyType::Dynamic
    );
    circle2->setPosition(Vector2<float>(1.0f, 0.0f));
    circle2->setLinearVelocity(Vector2<float>(-5.0f, 0.0f));
    
    world_->addBody(circle1);
    world_->addBody(circle2);
    
    // Step simulation
    for (int i = 0; i < 30; ++i) {  // 0.5 seconds
        world_->step(1.0f / 60.0f);
    }
    
    // Circles should have collided and changed direction
    EXPECT_LT(circle1->getLinearVelocity().x, 0.0f);
    EXPECT_GT(circle2->getLinearVelocity().x, 0.0f);
}

// Test rectangle vs circle collision
TEST_F(PhysicsSystemTest, RectangleCircleCollision) {
    auto rect = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f)),
        BodyType::Static
    );
    rect->setPosition(Vector2<float>(0.0f, 0.0f));
    
    auto circle = std::make_shared<RigidBody>(
        std::make_shared<CircleShape>(0.5f),
        BodyType::Dynamic
    );
    circle->setPosition(Vector2<float>(-5.0f, 0.0f));
    circle->setLinearVelocity(Vector2<float>(10.0f, 0.0f));
    
    world_->addBody(rect);
    world_->addBody(circle);
    
    // Step simulation
    for (int i = 0; i < 60; ++i) {  // 1 second
        world_->step(1.0f / 60.0f);
    }
    
    // Circle should have bounced back
    EXPECT_LT(circle->getLinearVelocity().x, 0.0f);
}

// Test force application
TEST_F(PhysicsSystemTest, ForceApplication) {
    auto body = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f)),
        BodyType::Dynamic
    );
    body->setPosition(Vector2<float>(0.0f, 0.0f));
    body->setMass(1.0f);
    
    world_->addBody(body);
    world_->setGravity(Vector2<float>(0.0f, 0.0f));  // Disable gravity
    
    // Apply force
    body->applyForce(Vector2<float>(10.0f, 0.0f));
    
    Vector2<float> initialPos = body->getPosition();
    
    // Step simulation
    world_->step(1.0f / 60.0f);
    
    // Body should have moved right
    EXPECT_GT(body->getPosition().x, initialPos.x);
    EXPECT_GT(body->getLinearVelocity().x, 0.0f);
}

// Test impulse application
TEST_F(PhysicsSystemTest, ImpulseApplication) {
    auto body = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f)),
        BodyType::Dynamic
    );
    body->setMass(1.0f);
    
    world_->addBody(body);
    
    // Apply impulse
    body->applyImpulse(Vector2<float>(5.0f, 0.0f));
    
    // Velocity should change immediately
    EXPECT_FLOAT_EQ(body->getLinearVelocity().x, 5.0f);
}

// Test AABB query
TEST_F(PhysicsSystemTest, AABBQuery) {
    auto body1 = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f)),
        BodyType::Dynamic
    );
    body1->setPosition(Vector2<float>(0.0f, 0.0f));
    
    auto body2 = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f)),
        BodyType::Dynamic
    );
    body2->setPosition(Vector2<float>(5.0f, 5.0f));
    
    world_->addBody(body1);
    world_->addBody(body2);
    
    // Query around origin
    AABB<float> queryBounds(
        SIMD::Vector<float, 3>(-2.0f, -2.0f, 0.0f),
        SIMD::Vector<float, 3>(2.0f, 2.0f, 0.0f)
    );
    
    auto results = world_->queryAABB(queryBounds);
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], body1.get());
}

// Test point query
TEST_F(PhysicsSystemTest, PointQuery) {
    auto body = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f)),
        BodyType::Dynamic
    );
    body->setPosition(Vector2<float>(0.0f, 0.0f));
    
    world_->addBody(body);
    
    auto results = world_->queryPoint(Vector2<float>(0.5f, 0.5f));
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], body.get());
    
    auto noResults = world_->queryPoint(Vector2<float>(5.0f, 5.0f));
    EXPECT_EQ(noResults.size(), 0);
}

// Test raycast
TEST_F(PhysicsSystemTest, Raycast) {
    auto body = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f)),
        BodyType::Static
    );
    body->setPosition(Vector2<float>(0.0f, 0.0f));
    
    world_->addBody(body);
    
    // Cast ray from left to right
    auto hit = world_->raycast(Vector2<float>(-5.0f, 0.0f), Vector2<float>(5.0f, 0.0f));
    
    EXPECT_TRUE(hit.hasHit);
    EXPECT_EQ(hit.body, body.get());
    EXPECT_GT(hit.distance, 0.0f);
    EXPECT_LT(hit.distance, 10.0f);
}

// Test physics statistics
TEST_F(PhysicsSystemTest, PhysicsStatistics) {
    auto body1 = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f)),
        BodyType::Dynamic
    );
    
    auto body2 = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f)),
        BodyType::Static
    );
    
    world_->addBody(body1);
    world_->addBody(body2);
    
    world_->step(1.0f / 60.0f);
    
    const auto& stats = world_->getStats();
    EXPECT_GT(stats.active_bodies, 0);
    EXPECT_GE(stats.step_time, 0.0f);
}

// Test body sleeping
TEST_F(PhysicsSystemTest, BodySleeping) {
    auto body = std::make_shared<RigidBody>(
        std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f)),
        BodyType::Dynamic
    );
    body->setPosition(Vector2<float>(0.0f, 0.0f));
    body->setLinearVelocity(Vector2<float>(0.0f, 0.0f));
    
    world_->addBody(body);
    world_->setGravity(Vector2<float>(0.0f, 0.0f));  // No gravity
    
    EXPECT_TRUE(body->isAwake());
    
    // Step simulation for a while to let body sleep
    for (int i = 0; i < 120; ++i) {  // 2 seconds
        world_->step(1.0f / 60.0f);
    }
    
    // Body should eventually go to sleep
    EXPECT_FALSE(body->isAwake());
}

// Test collision shape bounds
TEST_F(PhysicsSystemTest, CollisionShapeBounds) {
    auto rectShape = std::make_shared<RectangleShape>(Vector2<float>(4.0f, 2.0f));
    auto rectBody = std::make_shared<RigidBody>(rectShape, BodyType::Dynamic);
    rectBody->setPosition(Vector2<float>(1.0f, 2.0f));
    
    auto bounds = rectBody->getWorldBounds();
    
    // Check bounds are correct
    EXPECT_FLOAT_EQ(bounds.min[0], -1.0f);  // 1.0 - 2.0 (half width)
    EXPECT_FLOAT_EQ(bounds.max[0], 3.0f);   // 1.0 + 2.0 (half width)
    EXPECT_FLOAT_EQ(bounds.min[1], 1.0f);   // 2.0 - 1.0 (half height)
    EXPECT_FLOAT_EQ(bounds.max[1], 3.0f);   // 2.0 + 1.0 (half height)
}

// Test physics builder pattern
TEST_F(PhysicsSystemTest, PhysicsBuilder) {
    auto world = PhysicsWorldBuilder()
        .setGravity(Vector2<float>(0.0f, -5.0f))
        .setIterations(10, 5)
        .enableSleeping(false)
        .build();
    
    EXPECT_EQ(world->getConfig().gravity.y, -5.0f);
    EXPECT_EQ(world->getConfig().velocity_iterations, 10);
    EXPECT_EQ(world->getConfig().position_iterations, 5);
    EXPECT_FALSE(world->getConfig().enable_sleeping);
}

// Test convenience shape creators
TEST_F(PhysicsSystemTest, ConvenienceShapeCreators) {
    auto box = Shapes::box(2.0f, 1.0f);
    EXPECT_EQ(box->getSize().x, 2.0f);
    EXPECT_EQ(box->getSize().y, 1.0f);
    
    auto square = Shapes::square(3.0f);
    EXPECT_EQ(square->getSize().x, 3.0f);
    EXPECT_EQ(square->getSize().y, 3.0f);
    
    auto circle = Shapes::circle(1.5f);
    EXPECT_EQ(circle->getRadius(), 1.5f);
}

// Test convenience body creators
TEST_F(PhysicsSystemTest, ConvenienceBodyCreators) {
    auto dynamicBox = Bodies::dynamicBox(2.0f, 1.0f, Materials::METAL);
    EXPECT_EQ(dynamicBox->getBodyType(), BodyType::Dynamic);
    EXPECT_EQ(dynamicBox->getMaterial().density, 7.8f);
    
    auto staticBox = Bodies::staticBox(5.0f, 1.0f);
    EXPECT_EQ(staticBox->getBodyType(), BodyType::Static);
    
    auto dynamicCircle = Bodies::dynamicCircle(1.0f, Materials::RUBBER);
    EXPECT_EQ(dynamicCircle->getBodyType(), BodyType::Dynamic);
    EXPECT_EQ(dynamicCircle->getMaterial().restitution, 0.9f);
}