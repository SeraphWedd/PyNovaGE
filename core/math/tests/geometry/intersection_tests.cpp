#include "math/geometry/intersection.hpp"
#include <gtest/gtest.h>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

TEST(RayPlaneIntersectionTests, BasicIntersection) {
    Ray3D ray(Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
    Plane plane(Vector3(0.0f, 1.0f, 0.0f), 0.0f);
    
    auto result = rayPlaneIntersection(ray, plane);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    EXPECT_FLOAT_EQ(result->distance, 2.0f);
    EXPECT_EQ(result->point, Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(result->normal, Vector3::unitY());
}

TEST(RayPlaneIntersectionTests, NoIntersection) {
    Ray3D ray(Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    Plane plane(Vector3(0.0f, 1.0f, 0.0f), 0.0f);
    
    auto result = rayPlaneIntersection(ray, plane);
    EXPECT_FALSE(result.has_value());
}

TEST(RayPlaneIntersectionTests, ParallelRay) {
    Ray3D ray(Vector3(0.0f, 2.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
    Plane plane(Vector3(0.0f, 1.0f, 0.0f), 0.0f);
    
    auto result = rayPlaneIntersection(ray, plane);
    EXPECT_FALSE(result.has_value());
}

TEST(RaySphereIntersectionTests, BasicIntersection) {
    Ray3D ray(Vector3(0.0f, 0.0f, -2.0f), Vector3(0.0f, 0.0f, 1.0f));
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    
    auto result = raySphereIntersection(ray, sphere);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    EXPECT_FLOAT_EQ(result->distance, 1.0f);
    EXPECT_EQ(result->point, Vector3(0.0f, 0.0f, -1.0f));
    EXPECT_EQ(result->normal, Vector3(0.0f, 0.0f, -1.0f));
}

TEST(RaySphereIntersectionTests, NoIntersection) {
    Ray3D ray(Vector3(2.0f, 0.0f, -2.0f), Vector3(0.0f, 0.0f, 1.0f));
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    
    auto result = raySphereIntersection(ray, sphere);
    EXPECT_FALSE(result.has_value());
}

TEST(RaySphereIntersectionTests, Tangent) {
    Ray3D ray(Vector3(1.0f, 0.0f, -2.0f), Vector3(0.0f, 0.0f, 1.0f));
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    
    auto result = raySphereIntersection(ray, sphere);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    EXPECT_EQ(result->point, Vector3(1.0f, 0.0f, 0.0f));
}

TEST(RayAABBIntersectionTests, BasicIntersection) {
    Ray3D ray(Vector3(0.0f, 0.0f, -2.0f), Vector3(0.0f, 0.0f, 1.0f));
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    
    auto result = rayAABBIntersection(ray, aabb);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    EXPECT_FLOAT_EQ(result->distance, 1.0f);
    EXPECT_EQ(result->point, Vector3(0.0f, 0.0f, -1.0f));
    EXPECT_EQ(result->normal, Vector3(0.0f, 0.0f, -1.0f));
}

TEST(RayAABBIntersectionTests, NoIntersection) {
    Ray3D ray(Vector3(2.0f, 0.0f, -2.0f), Vector3(0.0f, 0.0f, 1.0f));
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    
    auto result = rayAABBIntersection(ray, aabb);
    EXPECT_FALSE(result.has_value());
}

TEST(SphereSphereIntersectionTests, BasicIntersection) {
    Sphere sphere1(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    Sphere sphere2(Vector3(1.5f, 0.0f, 0.0f), 1.0f);
    
    auto result = sphereSphereIntersection(sphere1, sphere2);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    EXPECT_FLOAT_EQ(result->distance, 1.5f);
}

TEST(SphereSphereIntersectionTests, NoIntersection) {
    Sphere sphere1(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    Sphere sphere2(Vector3(3.0f, 0.0f, 0.0f), 1.0f);
    
    auto result = sphereSphereIntersection(sphere1, sphere2);
    EXPECT_FALSE(result.has_value());
}

TEST(SphereSphereIntersectionTests, Touching) {
    Sphere sphere1(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    Sphere sphere2(Vector3(2.0f, 0.0f, 0.0f), 1.0f);
    
    auto result = sphereSphereIntersection(sphere1, sphere2);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
    EXPECT_FLOAT_EQ(result->distance, 2.0f);
}

TEST(AABBAABBIntersectionTests, BasicIntersection) {
    AABB aabb1(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB aabb2(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f));
    
    auto result = aabbAABBIntersection(aabb1, aabb2);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
}

TEST(AABBAABBIntersectionTests, NoIntersection) {
    AABB aabb1(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB aabb2(Vector3(2.0f, 2.0f, 2.0f), Vector3(3.0f, 3.0f, 3.0f));
    
    auto result = aabbAABBIntersection(aabb1, aabb2);
    EXPECT_FALSE(result.has_value());
}

TEST(AABBAABBIntersectionTests, Touching) {
    AABB aabb1(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB aabb2(Vector3(1.0f, 1.0f, 1.0f), Vector3(2.0f, 2.0f, 2.0f));
    
    auto result = aabbAABBIntersection(aabb1, aabb2);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->intersects);
}