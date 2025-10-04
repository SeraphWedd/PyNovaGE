#include "geometry/primitives.hpp"
#include <gtest/gtest.h>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

TEST(Line3DTests, DefaultConstructor) {
    Line3D line;
    EXPECT_EQ(line.origin, Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(line.direction, Vector3(1.0f, 0.0f, 0.0f));
}

TEST(Line3DTests, CustomConstructor) {
    Vector3 origin(1.0f, 2.0f, 3.0f);
    Vector3 direction(0.0f, 1.0f, 0.0f);
    Line3D line(origin, direction);
    EXPECT_EQ(line.origin, origin);
    EXPECT_EQ(line.direction, direction.normalized());
}

TEST(Line3DTests, ClosestPoint) {
    Line3D line(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
    Vector3 point(0.0f, 1.0f, 0.0f);
    Vector3 closest = line.closestPoint(point);
    EXPECT_EQ(closest, Vector3(0.0f, 0.0f, 0.0f));
}

TEST(Ray3DTests, DefaultConstructor) {
    Ray3D ray;
    EXPECT_EQ(ray.origin, Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(ray.direction, Vector3(1.0f, 0.0f, 0.0f));
}

TEST(Ray3DTests, CustomConstructor) {
    Vector3 origin(1.0f, 2.0f, 3.0f);
    Vector3 direction(0.0f, 1.0f, 0.0f);
    Ray3D ray(origin, direction);
    EXPECT_EQ(ray.origin, origin);
    EXPECT_EQ(ray.direction, direction.normalized());
}

TEST(Ray3DTests, GetPoint) {
    Ray3D ray(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
    EXPECT_EQ(ray.getPoint(2.0f), Vector3(2.0f, 0.0f, 0.0f));
}

TEST(Ray3DTests, ClosestPoint) {
    Ray3D ray(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
    
    // Point in front of ray
    Vector3 point1(2.0f, 1.0f, 0.0f);
    EXPECT_EQ(ray.closestPoint(point1), Vector3(2.0f, 0.0f, 0.0f));
    
    // Point behind ray
    Vector3 point2(-2.0f, 1.0f, 0.0f);
    EXPECT_EQ(ray.closestPoint(point2), Vector3(0.0f, 0.0f, 0.0f));
}

TEST(PlaneTests, DefaultConstructor) {
    Plane plane;
    EXPECT_EQ(plane.normal, Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_FLOAT_EQ(plane.distance, 0.0f);
}

TEST(PlaneTests, CustomConstructor) {
    Vector3 normal(0.0f, 1.0f, 0.0f);
    float distance = 5.0f;
    Plane plane(normal, distance);
    EXPECT_EQ(plane.normal, normal);
    EXPECT_FLOAT_EQ(plane.distance, distance);
}

TEST(PlaneTests, FromPointAndNormal) {
    Vector3 point(0.0f, 5.0f, 0.0f);
    Vector3 normal(0.0f, 1.0f, 0.0f);
    Plane plane = Plane::fromPointAndNormal(point, normal);
    EXPECT_EQ(plane.normal, normal);
    EXPECT_FLOAT_EQ(plane.distance, 5.0f);
}

TEST(PlaneTests, SignedDistance) {
    Plane plane(Vector3(0.0f, 1.0f, 0.0f), 5.0f);
    
    // Point above plane
    EXPECT_FLOAT_EQ(plane.signedDistance(Vector3(0.0f, 10.0f, 0.0f)), 5.0f);
    
    // Point below plane
    EXPECT_FLOAT_EQ(plane.signedDistance(Vector3(0.0f, 0.0f, 0.0f)), -5.0f);
    
    // Point on plane
    EXPECT_FLOAT_EQ(plane.signedDistance(Vector3(0.0f, 5.0f, 0.0f)), 0.0f);
}

TEST(AABBTests, DefaultConstructor) {
    AABB aabb;
    EXPECT_EQ(aabb.min, Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(aabb.max, Vector3(0.0f, 0.0f, 0.0f));
}

TEST(AABBTests, CustomConstructor) {
    Vector3 min(-1.0f, -1.0f, -1.0f);
    Vector3 max(1.0f, 1.0f, 1.0f);
    AABB aabb(min, max);
    EXPECT_EQ(aabb.min, min);
    EXPECT_EQ(aabb.max, max);
}

TEST(AABBTests, Center) {
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    EXPECT_EQ(aabb.center(), Vector3(0.0f, 0.0f, 0.0f));
}

TEST(AABBTests, Dimensions) {
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    EXPECT_EQ(aabb.dimensions(), Vector3(2.0f, 2.0f, 2.0f));
}

TEST(AABBTests, Contains) {
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    
    // Point inside
    EXPECT_TRUE(aabb.contains(Vector3(0.0f, 0.0f, 0.0f)));
    
    // Point outside
    EXPECT_FALSE(aabb.contains(Vector3(2.0f, 0.0f, 0.0f)));
    
    // Point on boundary
    EXPECT_TRUE(aabb.contains(Vector3(1.0f, 0.0f, 0.0f)));
}

TEST(SphereTests, DefaultConstructor) {
    Sphere sphere;
    EXPECT_EQ(sphere.center, Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_FLOAT_EQ(sphere.radius, 1.0f);
}

TEST(SphereTests, CustomConstructor) {
    Vector3 center(1.0f, 2.0f, 3.0f);
    float radius = 5.0f;
    Sphere sphere(center, radius);
    EXPECT_EQ(sphere.center, center);
    EXPECT_FLOAT_EQ(sphere.radius, radius);
}

TEST(SphereTests, Contains) {
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    
    // Point inside
    EXPECT_TRUE(sphere.contains(Vector3(0.5f, 0.0f, 0.0f)));
    
    // Point outside
    EXPECT_FALSE(sphere.contains(Vector3(2.0f, 0.0f, 0.0f)));
    
    // Point on surface
    EXPECT_TRUE(sphere.contains(Vector3(1.0f, 0.0f, 0.0f)));
}

TEST(SphereTests, ClosestPoint) {
    Sphere sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    
    // Point inside sphere
    Vector3 inside(0.5f, 0.0f, 0.0f);
    EXPECT_EQ(sphere.closestPoint(inside), inside);
    
    // Point outside sphere
    Vector3 outside(2.0f, 0.0f, 0.0f);
    EXPECT_EQ(sphere.closestPoint(outside), Vector3(1.0f, 0.0f, 0.0f));
}