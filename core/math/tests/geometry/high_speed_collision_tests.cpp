#include "geometry/high_speed_collision.hpp"
#include "geometry/intersection.hpp"
#include "math_constants.hpp"
#include <gtest/gtest.h>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

TEST(HighSpeedCollisionTests, SpherePenetrationBasic) {
    Sphere sphere{Vector3(0.0f, 0.0f, 0.0f), 1.0f};
    
    // Test direct penetration through sphere center
    PenetrationTestParams params{
        Vector3(-2.0f, 0.0f, 0.0f),  // origin: 2 units to the left
        Vector3(1.0f, 0.0f, 0.0f),   // direction: right
        10.0f,                       // velocity
        0.1f                         // projectile radius
    };
    
    auto result = TestSpherePenetration(sphere, params);
    
    EXPECT_TRUE(result.penetrated);
    EXPECT_FLOAT_EQ(result.thickness, 2.0f);  // Should be diameter of sphere
    EXPECT_EQ(result.surface_normal, Vector3(-1.0f, 0.0f, 0.0f));
}

TEST(HighSpeedCollisionTests, SpherePenetrationOffset) {
    Sphere sphere{Vector3(0.0f, 0.0f, 0.0f), 1.0f};
    
    // Test penetration through sphere at an offset
    PenetrationTestParams params{
        Vector3(-2.0f, 0.5f, 0.0f),  // origin: offset up by 0.5
        Vector3(1.0f, 0.0f, 0.0f),   // direction: right
        10.0f,                       // velocity
        0.1f                         // projectile radius
    };
    
    auto result = TestSpherePenetration(sphere, params);
    
    EXPECT_TRUE(result.penetrated);
    EXPECT_LT(result.thickness, 2.0f);  // Should be less than diameter due to offset
}

TEST(HighSpeedCollisionTests, SpherePenetrationMiss) {
    Sphere sphere{Vector3(0.0f, 0.0f, 0.0f), 1.0f};
    
    // Test ray that misses sphere
    PenetrationTestParams params{
        Vector3(-2.0f, 2.0f, 0.0f),  // origin: too high to hit
        Vector3(1.0f, 0.0f, 0.0f),   // direction: right
        10.0f,                       // velocity
        0.1f                         // projectile radius
    };
    
    auto result = TestSpherePenetration(sphere, params);
    
    EXPECT_FALSE(result.penetrated);
}

TEST(HighSpeedCollisionTests, AABBPenetrationBasic) {
    AABB box{Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f)};
    
    // Test direct penetration through box center
    PenetrationTestParams params{
        Vector3(-2.0f, 0.0f, 0.0f),  // origin: 2 units to the left
        Vector3(1.0f, 0.0f, 0.0f),   // direction: right
        10.0f,                       // velocity
        0.1f                         // projectile radius
    };
    
    auto result = TestAABBPenetration(box, params);
    
    EXPECT_TRUE(result.penetrated);
    EXPECT_FLOAT_EQ(result.thickness, 2.0f);  // Should be width of box
    EXPECT_EQ(result.surface_normal, Vector3(-1.0f, 0.0f, 0.0f));
}

TEST(HighSpeedCollisionTests, AABBPenetrationAngled) {
    AABB box{Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f)};
    
    // Test angled penetration through box
    PenetrationTestParams params{
        Vector3(-2.0f, -2.0f, 0.0f),            // origin: lower left
        Vector3(1.0f, 1.0f, 0.0f).normalized(), // direction: 45 degrees up
        10.0f,                                  // velocity
        0.1f                                    // projectile radius
    };
    
    auto result = TestAABBPenetration(box, params);
    
    EXPECT_TRUE(result.penetrated);
    EXPECT_GT(result.thickness, 2.0f);  // Should be greater than width due to angle
}

TEST(HighSpeedCollisionTests, AABBPenetrationMiss) {
    AABB box{Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f)};
    
    // Test ray that misses box
    PenetrationTestParams params{
        Vector3(-2.0f, 2.0f, 0.0f),  // origin: too high to hit
        Vector3(1.0f, 0.0f, 0.0f),   // direction: right
        10.0f,                       // velocity
        0.1f                         // projectile radius
    };
    
    auto result = TestAABBPenetration(box, params);
    
    EXPECT_FALSE(result.penetrated);
}

TEST(HighSpeedCollisionTests, ZeroVelocityNoCollision) {
    Sphere sphere{Vector3(0.0f, 0.0f, 0.0f), 1.0f};
    
    // Test with zero velocity
    PenetrationTestParams params{
        Vector3(-2.0f, 0.0f, 0.0f),  // origin
        Vector3(1.0f, 0.0f, 0.0f),   // direction
        0.0f,                        // velocity: zero
        0.1f                         // projectile radius
    };
    
    auto result = TestSpherePenetration(sphere, params);
    
    EXPECT_FALSE(result.penetrated);  // Should not penetrate with zero velocity
}

TEST(HighSpeedCollisionTests, GrazingSphereCollision) {
    Sphere sphere{Vector3(0.0f, 0.0f, 0.0f), 1.0f};
    
    // Test grazing collision with sphere
    PenetrationTestParams params{
        Vector3(-2.0f, 0.999f, 0.0f),  // origin: just barely hitting
        Vector3(1.0f, 0.0f, 0.0f),     // direction: right
        10.0f,                         // velocity
        0.1f                           // projectile radius
    };
    
    auto result = TestSpherePenetration(sphere, params);
    
    EXPECT_TRUE(result.penetrated);
    EXPECT_LT(result.thickness, 0.1f);  // Should have very small penetration depth
}

TEST(HighSpeedCollisionTests, GrazingAABBCollision) {
    AABB box{Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f)};
    
    // Test grazing collision with box edge
    PenetrationTestParams params{
        Vector3(-2.0f, 0.999f, 0.0f),  // origin: just barely hitting
        Vector3(1.0f, 0.0f, 0.0f),     // direction: right
        10.0f,                         // velocity
        0.1f                           // projectile radius
    };
    
    auto result = TestAABBPenetration(box, params);
    
    EXPECT_TRUE(result.penetrated);
    EXPECT_LT(result.thickness, 2.0f);  // Should be less than full width
}