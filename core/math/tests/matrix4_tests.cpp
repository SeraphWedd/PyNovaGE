#include "../include/matrix4.hpp"
#include <gtest/gtest.h>

using namespace pynovage::math;

TEST(Matrix4Tests, DefaultConstructor) {
    Matrix4x4 m;
    // Check identity matrix
    EXPECT_FLOAT_EQ(m.m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(m.m[1][1], 1.0f);
    EXPECT_FLOAT_EQ(m.m[2][2], 1.0f);
    EXPECT_FLOAT_EQ(m.m[3][3], 1.0f);
    // Check zeros
    EXPECT_FLOAT_EQ(m.m[0][1], 0.0f);
    EXPECT_FLOAT_EQ(m.m[0][2], 0.0f);
    EXPECT_FLOAT_EQ(m.m[0][3], 0.0f);
    EXPECT_FLOAT_EQ(m.m[1][0], 0.0f);
}

TEST(Matrix4Tests, TranslationMatrix) {
    Matrix4x4 trans = Matrix4x4::Translation(2.0f, 3.0f, 4.0f);
    Vector3 point(1.0f, 1.0f, 1.0f);
    Vector3 result = trans.transformPoint(point);
    EXPECT_FLOAT_EQ(result.x, 3.0f);  // 1 + 2
    EXPECT_FLOAT_EQ(result.y, 4.0f);  // 1 + 3
    EXPECT_FLOAT_EQ(result.z, 5.0f);  // 1 + 4
}

TEST(Matrix4Tests, ScaleMatrix) {
    Matrix4x4 scale = Matrix4x4::Scale(2.0f, 3.0f, 4.0f);
    Vector3 point(1.0f, 1.0f, 1.0f);
    Vector3 result = scale.transformPoint(point);
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
}

TEST(Matrix4Tests, RotationMatrix) {
    // Test 90-degree rotation around Y axis
    Matrix4x4 rot = Matrix4x4::RotationY(constants::half_pi);
    Vector3 point(1.0f, 0.0f, 0.0f);
    Vector3 result = rot.transformPoint(point);
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.z, -1.0f, 1e-6f);
}

TEST(Matrix4Tests, MatrixVectorMultiplication) {
    Matrix4x4 trans = Matrix4x4::Translation(1.0f, 2.0f, 3.0f);
    Vector4 point(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 result = trans * point;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}