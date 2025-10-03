#include "math/geometry/continuous_collision.hpp"
#include <gtest/gtest.h>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

TEST(MovingSphereSphereTests, NoCollision) {
    // Moving sphere passing by static sphere at a distance
    Sphere movingSphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    Sphere staticSphere(Vector3(0.0f, 2.0f, 0.0f), 0.5f);
    Vector3 start(2.0f, 0.0f, -2.0f);
    Vector3 end(2.0f, 0.0f, 2.0f);
    float timeStep = 1.0f;
    
    auto result = testMovingSphereSphere(movingSphere, staticSphere, start, end, timeStep);
    EXPECT_FALSE(result.has_value());
}

TEST(MovingSphereSphereTests, DirectHit) {
    // Moving sphere directly into static sphere
    Sphere movingSphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    Sphere staticSphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    Vector3 start(0.0f, 0.0f, -2.0f);
    Vector3 end(0.0f, 0.0f, 2.0f);
    float timeStep = 1.0f;
    
    auto result = testMovingSphereSphere(movingSphere, staticSphere, start, end, timeStep);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    EXPECT_FLOAT_EQ(result->timeOfImpact, 0.25f);  // Should collide at z=-1 (combined radius = 1)
    EXPECT_EQ(result->point, Vector3(0.0f, 0.0f, -1.0f));
    EXPECT_EQ(result->normal, Vector3(0.0f, 0.0f, -1.0f));
}

TEST(MovingSphereSphereTests, GlancingHit) {
    // Moving sphere grazing static sphere
    Sphere movingSphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    Sphere staticSphere(Vector3(1.0f, 0.0f, 0.0f), 0.5f);
    Vector3 start(0.0f, 0.0f, -2.0f);
    Vector3 end(0.0f, 0.0f, 2.0f);
    float timeStep = 1.0f;
    
    auto result = testMovingSphereSphere(movingSphere, staticSphere, start, end, timeStep);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    EXPECT_FLOAT_EQ(result->timeOfImpact, 0.5f);  // Should collide at z=0
}

TEST(MovingSphereAABBTests, NoCollision) {
    // Moving sphere passing by AABB at a distance
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    Vector3 start(2.0f, 0.0f, -2.0f);
    Vector3 end(2.0f, 0.0f, 2.0f);
    float timeStep = 1.0f;
    
    auto result = testMovingSphereAABB(sphere, aabb, start, end, timeStep);
    EXPECT_FALSE(result.has_value());
}

TEST(MovingSphereAABBTests, DirectHit) {
    // Moving sphere directly into AABB face
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    Vector3 start(0.0f, 0.0f, -2.0f);
    Vector3 end(0.0f, 0.0f, 2.0f);
    float timeStep = 1.0f;
    
    auto result = testMovingSphereAABB(sphere, aabb, start, end, timeStep);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    EXPECT_NEAR(result->timeOfImpact, 0.125f, 0.01f);  // Should collide at z=-1.5 (expanded face)
    EXPECT_EQ(result->normal.z, -1.0f);  // Normal should point in -z direction
}

TEST(MovingSphereAABBTests, EdgeHit) {
    // Moving sphere hitting AABB edge
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    Vector3 start(1.5f, 1.5f, -2.0f);
    Vector3 end(1.5f, 1.5f, 2.0f);
    float timeStep = 1.0f;
    
    auto result = testMovingSphereAABB(sphere, aabb, start, end, timeStep);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    
    // With current start/end, first contact is corner; normal is (1,1,-1)/sqrt(3)
    EXPECT_NEAR(result->normal.x, 1.0f / std::sqrt(3.0f), 0.01f);
    EXPECT_NEAR(result->normal.y, 1.0f / std::sqrt(3.0f), 0.01f);
    EXPECT_NEAR(result->normal.z, -1.0f / std::sqrt(3.0f), 0.01f);
}