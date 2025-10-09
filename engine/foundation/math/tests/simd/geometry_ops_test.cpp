#include <gtest/gtest.h>
#include "../../include/simd/geometry_ops.hpp"
#include "../../include/simd/config.hpp"
#include <type_traits>

namespace {

using namespace PyNovaGE::SIMD;

// Test type traits and layouts
TEST(GeometryStructureTest, TypeTraits) {
    // AABB should be trivially copyable and have standard layout
    EXPECT_TRUE((std::is_trivially_copyable_v<AABB<float>>));
    EXPECT_TRUE((std::is_standard_layout_v<AABB<float>>));
    
    // Sphere should be trivially copyable and have standard layout
    EXPECT_TRUE((std::is_trivially_copyable_v<Sphere<float>>));
    EXPECT_TRUE((std::is_standard_layout_v<Sphere<float>>));
    
    // Ray should be trivially copyable and have standard layout
    EXPECT_TRUE((std::is_trivially_copyable_v<Ray<float>>));
    EXPECT_TRUE((std::is_standard_layout_v<Ray<float>>));
    
    // Plane should be trivially copyable and have standard layout
    EXPECT_TRUE((std::is_trivially_copyable_v<Plane<float>>));
    EXPECT_TRUE((std::is_standard_layout_v<Plane<float>>));
}

// Test memory alignment
TEST(GeometryStructureTest, MemoryLayout) {
    // AABB layout
    constexpr size_t vector_align = alignof(Vector3f);
    EXPECT_EQ(alignof(AABB<float>), vector_align);
    EXPECT_EQ(sizeof(AABB<float>), 2 * sizeof(Vector3f));
    
    // Sphere layout
    EXPECT_EQ(alignof(Sphere<float>), vector_align);
    EXPECT_GE(sizeof(Sphere<float>), sizeof(Vector3f) + sizeof(float));
    
    // Ray layout
    EXPECT_EQ(alignof(Ray<float>), vector_align);
    EXPECT_EQ(sizeof(Ray<float>), 2 * sizeof(Vector3f));
    
    // Plane layout
    EXPECT_EQ(alignof(Plane<float>), vector_align);
    EXPECT_GE(sizeof(Plane<float>), sizeof(Vector3f) + sizeof(float));
}

// Test data member alignment
TEST(GeometryStructureTest, DataAlignment) {
    // All vector members should maintain proper alignment
    constexpr size_t required_align = alignof(Vector3f);

    // Test AABB alignment
    AABB<float> aabb;
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&aabb.min) % required_align, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&aabb.max) % required_align, 0);

    // Test Sphere alignment
    Sphere<float> sphere;
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&sphere.center) % required_align, 0);

    // Test Ray alignment
    Ray<float> ray;
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&ray.origin) % required_align, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&ray.direction) % required_align, 0);

    // Test Plane alignment
    Plane<float> plane;
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&plane.normal) % required_align, 0);
}

// Test template constraints
TEST(GeometryStructureTest, TemplateConstraints) {
    // Test that AABB template parameter must be floating point
    EXPECT_TRUE((std::is_floating_point_v<typename AABB<float>::value_type>));
    EXPECT_TRUE((std::is_floating_point_v<typename AABB<double>::value_type>));

    // Test that Sphere template parameter must be floating point
    EXPECT_TRUE((std::is_floating_point_v<typename Sphere<float>::value_type>));
    EXPECT_TRUE((std::is_floating_point_v<typename Sphere<double>::value_type>));

    // Test that Ray template parameter must be floating point
    EXPECT_TRUE((std::is_floating_point_v<typename Ray<float>::value_type>));
    EXPECT_TRUE((std::is_floating_point_v<typename Ray<double>::value_type>));

    // Test that Plane template parameter must be floating point
    EXPECT_TRUE((std::is_floating_point_v<typename Plane<float>::value_type>));
    EXPECT_TRUE((std::is_floating_point_v<typename Plane<double>::value_type>));
}

} // namespace