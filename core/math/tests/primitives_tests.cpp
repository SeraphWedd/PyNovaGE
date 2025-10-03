#include <gtest/gtest.h>
#include "primitives.hpp"
#include <cmath>

using namespace pynovage::math;

TEST(RayTests, Construction) {
    Ray ray;
    EXPECT_FLOAT_EQ(ray.origin.x, 0.0f);
    EXPECT_FLOAT_EQ(ray.origin.y, 0.0f);
    EXPECT_FLOAT_EQ(ray.origin.z, 0.0f);
    EXPECT_FLOAT_EQ(ray.direction.x, 0.0f);
    EXPECT_FLOAT_EQ(ray.direction.y, 0.0f);
    EXPECT_FLOAT_EQ(ray.direction.z, 1.0f);

    Vector3 origin(1.0f, 2.0f, 3.0f);
    Vector3 dir(1.0f, 0.0f, 0.0f);
    Ray ray2(origin, dir);
    EXPECT_FLOAT_EQ(ray2.origin.x, 1.0f);
    EXPECT_FLOAT_EQ(ray2.origin.y, 2.0f);
    EXPECT_FLOAT_EQ(ray2.origin.z, 3.0f);
    EXPECT_FLOAT_EQ(ray2.direction.x, 1.0f);
    EXPECT_FLOAT_EQ(ray2.direction.y, 0.0f);
    EXPECT_FLOAT_EQ(ray2.direction.z, 0.0f);
}

TEST(RayTests, GetPoint) {
    Ray ray(Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
    Vector3 point = ray.getPoint(2.0f);
    EXPECT_FLOAT_EQ(point.x, 3.0f);
    EXPECT_FLOAT_EQ(point.y, 0.0f);
    EXPECT_FLOAT_EQ(point.z, 0.0f);
}

TEST(SphereTests, Construction) {
    Sphere sphere;
    EXPECT_FLOAT_EQ(sphere.center.x, 0.0f);
    EXPECT_FLOAT_EQ(sphere.center.y, 0.0f);
    EXPECT_FLOAT_EQ(sphere.center.z, 0.0f);
    EXPECT_FLOAT_EQ(sphere.radius, 1.0f);

    Sphere sphere2(Vector3(1.0f, 2.0f, 3.0f), 2.0f);
    EXPECT_FLOAT_EQ(sphere2.center.x, 1.0f);
    EXPECT_FLOAT_EQ(sphere2.center.y, 2.0f);
    EXPECT_FLOAT_EQ(sphere2.center.z, 3.0f);
    EXPECT_FLOAT_EQ(sphere2.radius, 2.0f);
}

TEST(AABBTests, Construction) {
    AABB aabb;
    EXPECT_FLOAT_EQ(aabb.min.x, 0.0f);
    EXPECT_FLOAT_EQ(aabb.min.y, 0.0f);
    EXPECT_FLOAT_EQ(aabb.min.z, 0.0f);
    EXPECT_FLOAT_EQ(aabb.max.x, 0.0f);
    EXPECT_FLOAT_EQ(aabb.max.y, 0.0f);
    EXPECT_FLOAT_EQ(aabb.max.z, 0.0f);

    AABB aabb2(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    EXPECT_FLOAT_EQ(aabb2.min.x, -1.0f);
    EXPECT_FLOAT_EQ(aabb2.min.y, -1.0f);
    EXPECT_FLOAT_EQ(aabb2.min.z, -1.0f);
    EXPECT_FLOAT_EQ(aabb2.max.x, 1.0f);
    EXPECT_FLOAT_EQ(aabb2.max.y, 1.0f);
    EXPECT_FLOAT_EQ(aabb2.max.z, 1.0f);
}

TEST(AABBTests, Properties) {
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    
    Vector3 center = aabb.getCenter();
    EXPECT_FLOAT_EQ(center.x, 0.0f);
    EXPECT_FLOAT_EQ(center.y, 0.0f);
    EXPECT_FLOAT_EQ(center.z, 0.0f);

    Vector3 extents = aabb.getExtents();
    EXPECT_FLOAT_EQ(extents.x, 1.0f);
    EXPECT_FLOAT_EQ(extents.y, 1.0f);
    EXPECT_FLOAT_EQ(extents.z, 1.0f);

    Vector3 size = aabb.getSize();
    EXPECT_FLOAT_EQ(size.x, 2.0f);
    EXPECT_FLOAT_EQ(size.y, 2.0f);
    EXPECT_FLOAT_EQ(size.z, 2.0f);
}

TEST(AABBTests, Expansion) {
    AABB aabb(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    aabb.expand(1.0f);
    
    EXPECT_FLOAT_EQ(aabb.min.x, -2.0f);
    EXPECT_FLOAT_EQ(aabb.min.y, -2.0f);
    EXPECT_FLOAT_EQ(aabb.min.z, -2.0f);
    EXPECT_FLOAT_EQ(aabb.max.x, 2.0f);
    EXPECT_FLOAT_EQ(aabb.max.y, 2.0f);
    EXPECT_FLOAT_EQ(aabb.max.z, 2.0f);
}

TEST(PlaneTests, Construction) {
    Plane plane;
    EXPECT_FLOAT_EQ(plane.normal.x, 0.0f);
    EXPECT_FLOAT_EQ(plane.normal.y, 1.0f);
    EXPECT_FLOAT_EQ(plane.normal.z, 0.0f);
    EXPECT_FLOAT_EQ(plane.d, 0.0f);

    Plane plane2(Vector3(1.0f, 0.0f, 0.0f), 2.0f);
    EXPECT_FLOAT_EQ(plane2.normal.x, 1.0f);
    EXPECT_FLOAT_EQ(plane2.normal.y, 0.0f);
    EXPECT_FLOAT_EQ(plane2.normal.z, 0.0f);
    EXPECT_FLOAT_EQ(plane2.d, 2.0f);

    Plane plane3(Vector3(1.0f, 0.0f, 0.0f), Vector3(2.0f, 0.0f, 0.0f));
    EXPECT_FLOAT_EQ(plane3.normal.x, 1.0f);
    EXPECT_FLOAT_EQ(plane3.normal.y, 0.0f);
    EXPECT_FLOAT_EQ(plane3.normal.z, 0.0f);
    EXPECT_FLOAT_EQ(plane3.d, -2.0f);
}

TEST(PlaneTests, SignedDistance) {
    Plane plane(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    
    EXPECT_FLOAT_EQ(plane.getSignedDistance(Vector3(2.0f, 0.0f, 0.0f)), 2.0f);
    EXPECT_FLOAT_EQ(plane.getSignedDistance(Vector3(-2.0f, 0.0f, 0.0f)), -2.0f);
    EXPECT_FLOAT_EQ(plane.getSignedDistance(Vector3(0.0f, 1.0f, 0.0f)), 0.0f);
}

TEST(TriangleTests, Construction) {
    Triangle tri;
    EXPECT_FLOAT_EQ(tri.v0.x, 0.0f);
    EXPECT_FLOAT_EQ(tri.v0.y, 0.0f);
    EXPECT_FLOAT_EQ(tri.v0.z, 0.0f);

    Triangle tri2(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_FLOAT_EQ(tri2.v1.x, 1.0f);
    EXPECT_FLOAT_EQ(tri2.v1.y, 0.0f);
    EXPECT_FLOAT_EQ(tri2.v2.x, 0.0f);
    EXPECT_FLOAT_EQ(tri2.v2.y, 1.0f);
}

TEST(TriangleTests, Properties) {
    Triangle tri(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    
    Vector3 normal = tri.getNormal();
    EXPECT_FLOAT_EQ(normal.x, 0.0f);
    EXPECT_FLOAT_EQ(normal.y, 0.0f);
    EXPECT_FLOAT_EQ(normal.z, 1.0f);

    float area = tri.getArea();
    EXPECT_FLOAT_EQ(area, 0.5f);

    Vector3 center = tri.getCenter();
    EXPECT_FLOAT_EQ(center.x, 1.0f/3.0f);
    EXPECT_FLOAT_EQ(center.y, 1.0f/3.0f);
    EXPECT_FLOAT_EQ(center.z, 0.0f);
}

TEST(OBBTests, Construction) {
    OBB obb;
    EXPECT_FLOAT_EQ(obb.center.x, 0.0f);
    EXPECT_FLOAT_EQ(obb.center.y, 0.0f);
    EXPECT_FLOAT_EQ(obb.center.z, 0.0f);
    EXPECT_FLOAT_EQ(obb.halfExtents.x, 1.0f);
    EXPECT_FLOAT_EQ(obb.halfExtents.y, 1.0f);
    EXPECT_FLOAT_EQ(obb.halfExtents.z, 1.0f);
    
    // Basic sanity: halfExtents set and center default; orientation existence is compile-time checked by type.
}

TEST(CapsuleTests, Construction) {
    Capsule capsule;
    EXPECT_FLOAT_EQ(capsule.point1.x, 0.0f);
    EXPECT_FLOAT_EQ(capsule.point1.y, 0.0f);
    EXPECT_FLOAT_EQ(capsule.point1.z, 0.0f);
    EXPECT_FLOAT_EQ(capsule.point2.x, 0.0f);
    EXPECT_FLOAT_EQ(capsule.point2.y, 0.0f);
    EXPECT_FLOAT_EQ(capsule.point2.z, 0.0f);
    EXPECT_FLOAT_EQ(capsule.radius, 1.0f);
}

TEST(CapsuleTests, Properties) {
    Capsule capsule(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f), 1.0f);
    
    float height = capsule.getHeight();
    EXPECT_FLOAT_EQ(height, 2.0f);

    Vector3 dir = capsule.getDirection();
    EXPECT_FLOAT_EQ(dir.x, 0.0f);
    EXPECT_FLOAT_EQ(dir.y, 1.0f);
    EXPECT_FLOAT_EQ(dir.z, 0.0f);
}