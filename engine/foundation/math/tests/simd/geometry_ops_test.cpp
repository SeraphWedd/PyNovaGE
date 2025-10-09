#include <gtest/gtest.h>
#include "../../include/simd/geometry_ops.hpp"
#include <cmath>

namespace {

using namespace PyNovaGE::SIMD;

// Helper function to compare vectors with tolerance
template<typename T, size_t N>
bool ApproxEqual(const Vector<T, N>& a, const Vector<T, N>& b, T tolerance = T(1e-5)) {
    for (size_t i = 0; i < N; ++i) {
        if (std::abs(a[i] - b[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

TEST(GeometryOpsTest, AABBConstruction) {
    // Test default construction
    AABB<float> aabb;
    EXPECT_TRUE(ApproxEqual(aabb.min, Vector3f(0.0f)));
    EXPECT_TRUE(ApproxEqual(aabb.max, Vector3f(0.0f)));

    // Test construction with min/max
    Vector3f min(-1.0f, -2.0f, -3.0f);
    Vector3f max(1.0f, 2.0f, 3.0f);
    AABB<float> aabb2(min, max);
    EXPECT_TRUE(ApproxEqual(aabb2.min, min));
    EXPECT_TRUE(ApproxEqual(aabb2.max, max));
}

TEST(GeometryOpsTest, AABBContains) {
    AABB<float> aabb(Vector3f(-1.0f), Vector3f(1.0f));

    // Test points inside
    EXPECT_TRUE(aabb.contains(Vector3f(0.0f)));
    EXPECT_TRUE(aabb.contains(Vector3f(0.5f, 0.5f, 0.5f)));
    EXPECT_TRUE(aabb.contains(Vector3f(-0.5f, -0.5f, -0.5f)));

    // Test points on boundary
    EXPECT_TRUE(aabb.contains(Vector3f(1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(aabb.contains(Vector3f(-1.0f, -1.0f, -1.0f)));

    // Test points outside
    EXPECT_FALSE(aabb.contains(Vector3f(2.0f, 0.0f, 0.0f)));
    EXPECT_FALSE(aabb.contains(Vector3f(0.0f, -2.0f, 0.0f)));
    EXPECT_FALSE(aabb.contains(Vector3f(0.0f, 0.0f, 2.0f)));
}

TEST(GeometryOpsTest, AABBIntersection) {
    AABB<float> aabb1(Vector3f(-1.0f), Vector3f(1.0f));

    // Test overlapping AABBs
    AABB<float> aabb2(Vector3f(-0.5f), Vector3f(1.5f));
    EXPECT_TRUE(aabb1.intersects(aabb2));
    EXPECT_TRUE(aabb2.intersects(aabb1));

    // Test touching AABBs
    AABB<float> aabb3(Vector3f(1.0f), Vector3f(2.0f));
    EXPECT_TRUE(aabb1.intersects(aabb3));
    EXPECT_TRUE(aabb3.intersects(aabb1));

    // Test non-intersecting AABBs
    AABB<float> aabb4(Vector3f(2.0f), Vector3f(3.0f));
    EXPECT_FALSE(aabb1.intersects(aabb4));
    EXPECT_FALSE(aabb4.intersects(aabb1));
}

TEST(GeometryOpsTest, AABBProperties) {
    Vector3f min(-1.0f, -2.0f, -3.0f);
    Vector3f max(1.0f, 2.0f, 3.0f);
    AABB<float> aabb(min, max);

    // Test center
    Vector3f expected_center(0.0f);
    EXPECT_TRUE(ApproxEqual(aabb.center(), expected_center));

    // Test extent
    Vector3f expected_extent(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(ApproxEqual(aabb.extent(), expected_extent));
}

TEST(GeometryOpsTest, SphereConstruction) {
    // Test default construction
    Sphere<float> sphere;
    EXPECT_TRUE(ApproxEqual(sphere.center, Vector3f(0.0f)));
    EXPECT_FLOAT_EQ(sphere.radius, 0.0f);

    // Test construction with center and radius
    Vector3f center(1.0f, 2.0f, 3.0f);
    float radius = 2.0f;
    Sphere<float> sphere2(center, radius);
    EXPECT_TRUE(ApproxEqual(sphere2.center, center));
    EXPECT_FLOAT_EQ(sphere2.radius, radius);
}

TEST(GeometryOpsTest, SphereContains) {
    Sphere<float> sphere(Vector3f(0.0f), 1.0f);

    // Test points inside
    EXPECT_TRUE(sphere.contains(Vector3f(0.0f)));
    EXPECT_TRUE(sphere.contains(Vector3f(0.5f, 0.0f, 0.0f)));

    // Test points on boundary
    EXPECT_TRUE(sphere.contains(Vector3f(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(sphere.contains(Vector3f(0.0f, -1.0f, 0.0f)));

    // Test points outside
    EXPECT_FALSE(sphere.contains(Vector3f(2.0f, 0.0f, 0.0f)));
    EXPECT_FALSE(sphere.contains(Vector3f(0.0f, 0.0f, -2.0f)));
}

TEST(GeometryOpsTest, SphereIntersection) {
    Sphere<float> sphere1(Vector3f(0.0f), 1.0f);

    // Test overlapping spheres
    Sphere<float> sphere2(Vector3f(1.0f, 0.0f, 0.0f), 1.0f);
    EXPECT_TRUE(sphere1.intersects(sphere2));
    EXPECT_TRUE(sphere2.intersects(sphere1));

    // Test touching spheres
    Sphere<float> sphere3(Vector3f(2.0f, 0.0f, 0.0f), 1.0f);
    EXPECT_TRUE(sphere1.intersects(sphere3));
    EXPECT_TRUE(sphere3.intersects(sphere1));

    // Test non-intersecting spheres
    Sphere<float> sphere4(Vector3f(3.0f, 0.0f, 0.0f), 1.0f);
    EXPECT_FALSE(sphere1.intersects(sphere4));
    EXPECT_FALSE(sphere4.intersects(sphere1));

    // Test sphere-AABB intersection
    AABB<float> aabb(Vector3f(-1.0f), Vector3f(1.0f));
    EXPECT_TRUE(sphere1.intersects(aabb));

    // Test non-intersecting sphere-AABB
    AABB<float> aabb2(Vector3f(2.0f), Vector3f(3.0f));
    EXPECT_FALSE(sphere1.intersects(aabb2));
}

TEST(GeometryOpsTest, RayConstruction) {
    // Test default construction
    Ray<float> ray;
    EXPECT_TRUE(ApproxEqual(ray.origin, Vector3f(0.0f)));
    EXPECT_TRUE(ApproxEqual(ray.direction, Vector3f(0.0f, 0.0f, 1.0f)));

    // Test construction with origin and direction
    Vector3f origin(1.0f, 2.0f, 3.0f);
    Vector3f direction(0.0f, 1.0f, 0.0f);
    Ray<float> ray2(origin, direction);
    EXPECT_TRUE(ApproxEqual(ray2.origin, origin));
    EXPECT_TRUE(ApproxEqual(ray2.direction, normalize(direction)));
}

TEST(GeometryOpsTest, RayIntersection) {
    Ray<float> ray(Vector3f(0.0f), Vector3f(0.0f, 0.0f, 1.0f));
    float t;

    // Test sphere intersection
    Sphere<float> sphere(Vector3f(0.0f, 0.0f, 5.0f), 1.0f);
    EXPECT_TRUE(ray.intersects(sphere, t));
    EXPECT_FLOAT_EQ(t, 4.0f);

    // Test AABB intersection
    AABB<float> aabb(Vector3f(-1.0f, -1.0f, 4.0f), Vector3f(1.0f, 1.0f, 6.0f));
    EXPECT_TRUE(ray.intersects(aabb, t));
    EXPECT_FLOAT_EQ(t, 4.0f);

    // Test non-intersecting cases
    Sphere<float> sphere2(Vector3f(2.0f, 2.0f, 5.0f), 1.0f);
    EXPECT_FALSE(ray.intersects(sphere2, t));

    AABB<float> aabb2(Vector3f(2.0f, 2.0f, 4.0f), Vector3f(3.0f, 3.0f, 6.0f));
    EXPECT_FALSE(ray.intersects(aabb2, t));
}

TEST(GeometryOpsTest, PlaneConstruction) {
    // Test default construction
    Plane<float> plane;
    EXPECT_TRUE(ApproxEqual(plane.normal, Vector3f(0.0f, 1.0f, 0.0f)));
    EXPECT_FLOAT_EQ(plane.distance, 0.0f);

    // Test construction with normal and distance
    Vector3f normal(1.0f, 0.0f, 0.0f);
    float distance = 2.0f;
    Plane<float> plane2(normal, distance);
    EXPECT_TRUE(ApproxEqual(plane2.normal, normalize(normal)));
    EXPECT_FLOAT_EQ(plane2.distance, distance);

    // Test construction with normal and point
    Vector3f point(2.0f, 0.0f, 0.0f);
    Plane<float> plane3(normal, point);
    EXPECT_TRUE(ApproxEqual(plane3.normal, normalize(normal)));
    EXPECT_FLOAT_EQ(plane3.distance, -2.0f);
}

TEST(GeometryOpsTest, PlaneOperations) {
    Plane<float> plane(Vector3f(1.0f, 0.0f, 0.0f), 2.0f);

    // Test signed distance
    EXPECT_FLOAT_EQ(plane.signedDistance(Vector3f(4.0f, 0.0f, 0.0f)), 2.0f);
    EXPECT_FLOAT_EQ(plane.signedDistance(Vector3f(0.0f, 0.0f, 0.0f)), -2.0f);

    // Test point classification
    EXPECT_EQ(plane.classifyPoint(Vector3f(4.0f, 0.0f, 0.0f)), 1);  // Front
    EXPECT_EQ(plane.classifyPoint(Vector3f(0.0f, 0.0f, 0.0f)), -1); // Back
    EXPECT_EQ(plane.classifyPoint(Vector3f(2.0f, 0.0f, 0.0f)), 0);  // On plane

    // Test ray intersection
    Ray<float> ray(Vector3f(0.0f), Vector3f(1.0f, 0.0f, 0.0f));
    float t;
    EXPECT_TRUE(plane.intersects(ray, t));
    EXPECT_FLOAT_EQ(t, 2.0f);

    // Test parallel ray (no intersection)
    Ray<float> parallel_ray(Vector3f(0.0f), Vector3f(0.0f, 1.0f, 0.0f));
    EXPECT_FALSE(plane.intersects(parallel_ray, t));
}

TEST(GeometryOpsTest, SIMDAlignment) {
    // Create aligned vectors for geometric primitives
    alignas(16) float data[4] = {1.0f, 2.0f, 3.0f, 0.0f};
    Vector3f aligned_vec(data[0], data[1], data[2]);

    // Test AABB alignment
    AABB<float> aabb(aligned_vec, aligned_vec * 2.0f);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&aabb.min) % 16, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&aabb.max) % 16, 0);

    // Test Sphere alignment
    Sphere<float> sphere(aligned_vec, 1.0f);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&sphere.center) % 16, 0);

    // Test Ray alignment
    Ray<float> ray(aligned_vec, aligned_vec);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&ray.origin) % 16, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&ray.direction) % 16, 0);

    // Test Plane alignment
    Plane<float> plane(aligned_vec, 1.0f);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&plane.normal) % 16, 0);
}

} // namespace