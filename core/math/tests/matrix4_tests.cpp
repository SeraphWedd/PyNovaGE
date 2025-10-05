#include "../include/matrix4.hpp"
#include <gtest/gtest.h>
#include <sstream>

using namespace pynovage::math;
using namespace pynovage::math::constants;

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
    Matrix4x4 mat = Matrix4x4::Translation(1.0f, 2.0f, 3.0f);
    Vector4 point(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 result = mat * point;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

TEST(Matrix4Tests, CompoundMultiplication) {
    Matrix4x4 mat1 = Matrix4x4::Translation(1.0f, 0.0f, 0.0f);
    Matrix4x4 mat2 = Matrix4x4::Translation(0.0f, 1.0f, 0.0f);
    mat1 *= mat2;

    Vector4 point(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = mat1 * point;
    EXPECT_FLOAT_EQ(result.x, 1.0f);
    EXPECT_FLOAT_EQ(result.y, 1.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

TEST(Matrix4Tests, Comparison) {
    Matrix4x4 mat1;
    Matrix4x4 mat2;
    Matrix4x4 mat3 = Matrix4x4::Translation(1.0f, 0.0f, 0.0f);

    EXPECT_TRUE(mat1 == mat2);
    EXPECT_FALSE(mat1 != mat2);
    EXPECT_FALSE(mat1 == mat3);
    EXPECT_TRUE(mat1 != mat3);
}

TEST(Matrix4Tests, ArraySubscript) {
    Matrix4x4 mat;
    mat[0][0] = 2.0f;
    mat[1][1] = 3.0f;

    EXPECT_FLOAT_EQ(mat[0][0], 2.0f);
    EXPECT_FLOAT_EQ(mat[1][1], 3.0f);
    EXPECT_FLOAT_EQ(mat[2][2], 1.0f); // Identity
}

TEST(Matrix4Tests, LookAt) {
    Vector3 eye(0.0f, 0.0f, 5.0f);
    Vector3 target(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Matrix4x4 view = Matrix4x4::LookAt(eye, target, up);
    
    // Eye should transform to origin in view space
    Vector4 eye_h(eye.x, eye.y, eye.z, 1.0f);
    Vector4 at_origin = view * eye_h;
    EXPECT_NEAR(at_origin.x, 0.0f, 1e-6f);
    EXPECT_NEAR(at_origin.y, 0.0f, 1e-6f);
    EXPECT_NEAR(at_origin.z, 0.0f, 1e-6f);
}

TEST(Matrix4Tests, Perspective) {
    float fov = half_pi;
    float aspect = 16.0f/9.0f;
    float near = 0.1f;
    float far = 100.0f;

    Matrix4x4 proj = Matrix4x4::Perspective(fov, aspect, near, far);

    // Test that w' = -z for points (OpenGL-style)
    Vector4 any(0.0f, 0.0f, 2.0f, 1.0f);
    Vector4 r = proj * any;
    EXPECT_NEAR(r.w, -any.z, 1e-5f);
}

TEST(Matrix4Tests, Orthographic) {
    Matrix4x4 ortho = Matrix4x4::Orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);

    // Test center point
    Vector4 center(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = ortho * center;
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
}

TEST(Matrix4Tests, RotationAxis) {
    Vector3 axis(1.0f, 0.0f, 0.0f);
    float angle = half_pi;

    Matrix4x4 rot = Matrix4x4::RotationAxis(axis, angle);
    Vector4 point(0.0f, 1.0f, 0.0f, 1.0f);

    Vector4 result = rot * point;
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
EXPECT_NEAR(result.z, 1.0f, 1e-6f);
}

TEST(Matrix4Tests, EulerAngles) {
    // 90 degrees around Y axis should transform (0,0,1) to (1,0,0)
    Matrix4x4 rot = Matrix4x4::FromEulerAngles(half_pi, 0.0f, 0.0f);
    Vector4 forward(0.0f, 0.0f, 1.0f, 0.0f);

    Vector4 result = rot * forward;
    EXPECT_NEAR(result.x, 1.0f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
}

TEST(Matrix4Tests, StringFormatting) {
    Matrix4x4 mat = Matrix4x4::Translation(1.0f, 2.0f, 3.0f);
    std::string str = mat.toString();
    
    // Just verify basic formatting structure
    EXPECT_NE(str.find("["), std::string::npos);
    EXPECT_NE(str.find("]"), std::string::npos);
    EXPECT_NE(str.find(","), std::string::npos);
}

TEST(Matrix4Tests, StreamOperator) {
    Matrix4x4 mat = Matrix4x4::Identity();
    std::stringstream ss;
    ss << mat;
    
    // Verify that something was written
    EXPECT_FALSE(ss.str().empty());
}
