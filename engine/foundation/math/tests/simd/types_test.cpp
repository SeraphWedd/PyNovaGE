#include <gtest/gtest.h>
#include "../../include/simd/types.hpp"
#include <type_traits>

namespace {

using namespace PyNovaGE::SIMD;

TEST(TypesTest, VectorConstruction) {
    // Default construction
    Vector4f v1;
    EXPECT_FLOAT_EQ(v1[0], 0.0f);
    EXPECT_FLOAT_EQ(v1[1], 0.0f);
    EXPECT_FLOAT_EQ(v1[2], 0.0f);
    EXPECT_FLOAT_EQ(v1[3], 0.0f);

    // Scalar construction
    Vector4f v2(1.0f);
    EXPECT_FLOAT_EQ(v2[0], 1.0f);
    EXPECT_FLOAT_EQ(v2[1], 1.0f);
    EXPECT_FLOAT_EQ(v2[2], 1.0f);
    EXPECT_FLOAT_EQ(v2[3], 1.0f);

    // Component-wise construction
    Vector4f v3(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v3[0], 1.0f);
    EXPECT_FLOAT_EQ(v3[1], 2.0f);
    EXPECT_FLOAT_EQ(v3[2], 3.0f);
    EXPECT_FLOAT_EQ(v3[3], 4.0f);

    // Copy construction
    Vector4f v4(v3);
    EXPECT_FLOAT_EQ(v4[0], 1.0f);
    EXPECT_FLOAT_EQ(v4[1], 2.0f);
    EXPECT_FLOAT_EQ(v4[2], 3.0f);
    EXPECT_FLOAT_EQ(v4[3], 4.0f);
}

TEST(TypesTest, VectorAssignment) {
    Vector4f v1(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f v2;
    v2 = v1;

    EXPECT_FLOAT_EQ(v2[0], 1.0f);
    EXPECT_FLOAT_EQ(v2[1], 2.0f);
    EXPECT_FLOAT_EQ(v2[2], 3.0f);
    EXPECT_FLOAT_EQ(v2[3], 4.0f);
}

TEST(TypesTest, VectorSubscript) {
    Vector4f v(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Read access
    EXPECT_FLOAT_EQ(v[0], 1.0f);
    EXPECT_FLOAT_EQ(v[1], 2.0f);
    EXPECT_FLOAT_EQ(v[2], 3.0f);
    EXPECT_FLOAT_EQ(v[3], 4.0f);

    // Write access
    v[0] = 5.0f;
    v[1] = 6.0f;
    v[2] = 7.0f;
    v[3] = 8.0f;

    EXPECT_FLOAT_EQ(v[0], 5.0f);
    EXPECT_FLOAT_EQ(v[1], 6.0f);
    EXPECT_FLOAT_EQ(v[2], 7.0f);
    EXPECT_FLOAT_EQ(v[3], 8.0f);
}

TEST(TypesTest, VectorComponentAccess) {
    Vector4f v(1.0f, 2.0f, 3.0f, 4.0f);
    
    EXPECT_FLOAT_EQ(v.x(), 1.0f);
    EXPECT_FLOAT_EQ(v.y(), 2.0f);
    EXPECT_FLOAT_EQ(v.z(), 3.0f);
    EXPECT_FLOAT_EQ(v.w(), 4.0f);

    v.x() = 5.0f;
    v.y() = 6.0f;
    v.z() = 7.0f;
    v.w() = 8.0f;

    EXPECT_FLOAT_EQ(v[0], 5.0f);
    EXPECT_FLOAT_EQ(v[1], 6.0f);
    EXPECT_FLOAT_EQ(v[2], 7.0f);
    EXPECT_FLOAT_EQ(v[3], 8.0f);
}

TEST(TypesTest, VectorAlignment) {
    // Test proper alignment for SIMD types
    EXPECT_TRUE((alignof(Vector4f) >= 16));
    EXPECT_TRUE((alignof(Vector3f) >= 16));
    EXPECT_TRUE((alignof(Vector2f) >= 16));
}

TEST(TypesTest, VectorSizeChecks) {
    // Test compile-time size checks
    EXPECT_EQ(sizeof(Vector4f), 4 * sizeof(float));
    EXPECT_EQ(sizeof(Vector3f), 3 * sizeof(float));
    EXPECT_EQ(sizeof(Vector2f), 2 * sizeof(float));

    // Ensure proper size for double vectors
    EXPECT_EQ(sizeof(Vector4d), 4 * sizeof(double));
    EXPECT_EQ(sizeof(Vector3d), 3 * sizeof(double));
    EXPECT_EQ(sizeof(Vector2d), 2 * sizeof(double));

    // Ensure proper size for integer vectors
    EXPECT_EQ(sizeof(Vector4i), 4 * sizeof(int32_t));
    EXPECT_EQ(sizeof(Vector3i), 3 * sizeof(int32_t));
    EXPECT_EQ(sizeof(Vector2i), 2 * sizeof(int32_t));
}

TEST(TypesTest, VectorTypeTraits) {
    // Test type traits
    EXPECT_TRUE((std::is_standard_layout_v<Vector4f>));
    EXPECT_TRUE((std::is_trivially_copyable_v<Vector4f>));
    EXPECT_TRUE((std::is_default_constructible_v<Vector4f>));
    EXPECT_TRUE((std::is_copy_constructible_v<Vector4f>));
    EXPECT_TRUE((std::is_copy_assignable_v<Vector4f>));
}

TEST(TypesTest, DataAccess) {
    Vector4f v(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Test const and non-const data access
    const float* const_data = v.data();
    float* data = v.data();

    EXPECT_FLOAT_EQ(const_data[0], 1.0f);
    EXPECT_FLOAT_EQ(const_data[1], 2.0f);
    EXPECT_FLOAT_EQ(const_data[2], 3.0f);
    EXPECT_FLOAT_EQ(const_data[3], 4.0f);

    data[0] = 5.0f;
    EXPECT_FLOAT_EQ(v[0], 5.0f);
}

} // namespace