#include "matrix3.hpp"
#include "math/math_constants.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace pynovage::math;
using namespace pynovage::math::constants;

class Matrix3Test : public ::testing::Test {
protected:
    void SetUp() override {
        identity = Matrix3x3();
        
        rotX90 = Matrix3x3::RotationX(half_pi);
        rotY90 = Matrix3x3::RotationY(half_pi);
        rotZ90 = Matrix3x3::RotationZ(half_pi);
        
        scale2 = Matrix3x3::Scale(2.0f, 2.0f, 2.0f);
    }

    Matrix3x3 identity;
    Matrix3x3 rotX90;
    Matrix3x3 rotY90;
    Matrix3x3 rotZ90;
    Matrix3x3 scale2;
};

TEST_F(Matrix3Test, DefaultConstructor) {
    Matrix3x3 m;
    EXPECT_FLOAT_EQ(m.m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(m.m[0][1], 0.0f);
    EXPECT_FLOAT_EQ(m.m[0][2], 0.0f);
    EXPECT_FLOAT_EQ(m.m[1][0], 0.0f);
    EXPECT_FLOAT_EQ(m.m[1][1], 1.0f);
    EXPECT_FLOAT_EQ(m.m[1][2], 0.0f);
    EXPECT_FLOAT_EQ(m.m[2][0], 0.0f);
    EXPECT_FLOAT_EQ(m.m[2][1], 0.0f);
    EXPECT_FLOAT_EQ(m.m[2][2], 1.0f);
}

TEST_F(Matrix3Test, ComponentConstructor) {
    Matrix3x3 m(1.0f, 2.0f, 3.0f,
                4.0f, 5.0f, 6.0f,
                7.0f, 8.0f, 9.0f);
    
    EXPECT_FLOAT_EQ(m.m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(m.m[0][1], 2.0f);
    EXPECT_FLOAT_EQ(m.m[0][2], 3.0f);
    EXPECT_FLOAT_EQ(m.m[1][0], 4.0f);
    EXPECT_FLOAT_EQ(m.m[1][1], 5.0f);
    EXPECT_FLOAT_EQ(m.m[1][2], 6.0f);
    EXPECT_FLOAT_EQ(m.m[2][0], 7.0f);
    EXPECT_FLOAT_EQ(m.m[2][1], 8.0f);
    EXPECT_FLOAT_EQ(m.m[2][2], 9.0f);
}

TEST_F(Matrix3Test, Identity) {
    Matrix3x3 m = Matrix3x3::Identity();
    EXPECT_EQ(m, identity);
}

TEST_F(Matrix3Test, Scale) {
    Vector3 v(1.0f, 1.0f, 1.0f);
    Vector3 scaled = scale2 * v;
    EXPECT_FLOAT_EQ(scaled.x, 2.0f);
    EXPECT_FLOAT_EQ(scaled.y, 2.0f);
    EXPECT_FLOAT_EQ(scaled.z, 2.0f);
}

TEST_F(Matrix3Test, Rotation) {
    // Test X rotation
    Vector3 v(0.0f, 1.0f, 0.0f);
    Vector3 rotated = rotX90 * v;
    EXPECT_NEAR(rotated.x, 0.0f, 1e-6f);
    EXPECT_NEAR(rotated.y, 0.0f, 1e-6f);
    EXPECT_NEAR(rotated.z, 1.0f, 1e-6f);

    // Test Y rotation
    v = Vector3(0.0f, 0.0f, 1.0f);
    rotated = rotY90 * v;
    EXPECT_NEAR(rotated.x, -1.0f, 1e-6f);
    EXPECT_NEAR(rotated.y, 0.0f, 1e-6f);
    EXPECT_NEAR(rotated.z, 0.0f, 1e-6f);

    // Test Z rotation
    v = Vector3(1.0f, 0.0f, 0.0f);
    rotated = rotZ90 * v;
    EXPECT_NEAR(rotated.x, 0.0f, 1e-6f);
    EXPECT_NEAR(rotated.y, 1.0f, 1e-6f);
    EXPECT_NEAR(rotated.z, 0.0f, 1e-6f);
}

TEST_F(Matrix3Test, Multiplication) {
    // Test matrix * matrix
    Matrix3x3 result = rotZ90 * rotX90;
    EXPECT_EQ(result * identity, result);
    EXPECT_EQ(identity * result, result);

    // Test matrix * scalar
    Matrix3x3 scaled = identity * 2.0f;
    EXPECT_FLOAT_EQ(scaled.m[0][0], 2.0f);
    EXPECT_FLOAT_EQ(scaled.m[1][1], 2.0f);
    EXPECT_FLOAT_EQ(scaled.m[2][2], 2.0f);
}

TEST_F(Matrix3Test, Addition) {
    Matrix3x3 m1(1.0f, 0.0f, 0.0f,
                 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 1.0f);
    
    Matrix3x3 m2(2.0f, 0.0f, 0.0f,
                 0.0f, 2.0f, 0.0f,
                 0.0f, 0.0f, 2.0f);
    
    Matrix3x3 sum = m1 + m2;
    EXPECT_FLOAT_EQ(sum.m[0][0], 3.0f);
    EXPECT_FLOAT_EQ(sum.m[1][1], 3.0f);
    EXPECT_FLOAT_EQ(sum.m[2][2], 3.0f);
}

TEST_F(Matrix3Test, Subtraction) {
    Matrix3x3 m1(2.0f, 0.0f, 0.0f,
                 0.0f, 2.0f, 0.0f,
                 0.0f, 0.0f, 2.0f);
    
    Matrix3x3 m2(1.0f, 0.0f, 0.0f,
                 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 1.0f);
    
    Matrix3x3 diff = m1 - m2;
    EXPECT_FLOAT_EQ(diff.m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(diff.m[1][1], 1.0f);
    EXPECT_FLOAT_EQ(diff.m[2][2], 1.0f);
}

TEST_F(Matrix3Test, Transpose) {
    Matrix3x3 m(1.0f, 2.0f, 3.0f,
                4.0f, 5.0f, 6.0f,
                7.0f, 8.0f, 9.0f);
    
    Matrix3x3 mt = m.Transposed();
    EXPECT_FLOAT_EQ(mt.m[0][1], m.m[1][0]);
    EXPECT_FLOAT_EQ(mt.m[0][2], m.m[2][0]);
    EXPECT_FLOAT_EQ(mt.m[1][2], m.m[2][1]);
    
    m.Transpose();
    EXPECT_EQ(m, mt);
}

TEST_F(Matrix3Test, Determinant) {
    Matrix3x3 m(1.0f, 2.0f, 3.0f,
                4.0f, 5.0f, 6.0f,
                7.0f, 8.0f, 9.0f);
    
    // For this matrix, det = 0
    EXPECT_NEAR(m.Determinant(), 0.0f, 1e-6f);
    
    Matrix3x3 invertible(2.0f, -1.0f, 0.0f,
                        -1.0f, 2.0f, -1.0f,
                        0.0f, -1.0f, 2.0f);
    
    // This matrix has det = 4
    EXPECT_NEAR(invertible.Determinant(), 4.0f, 1e-6f);
}

TEST_F(Matrix3Test, Inverse) {
    Matrix3x3 m(2.0f, -1.0f, 0.0f,
                -1.0f, 2.0f, -1.0f,
                0.0f, -1.0f, 2.0f);
    
    Matrix3x3 inv;
    EXPECT_TRUE(m.GetInverse(inv));
    
    Matrix3x3 result = m * inv;
    EXPECT_EQ(result, identity);
    
    // Test non-invertible matrix
    Matrix3x3 singular(1.0f, 2.0f, 3.0f,
                      4.0f, 5.0f, 6.0f,
                      7.0f, 8.0f, 9.0f);
    EXPECT_FALSE(singular.GetInverse(inv));
}

TEST_F(Matrix3Test, AxisAngle) {
    Vector3 axis(1.0f, 0.0f, 0.0f);
    float angle = half_pi;
    
    Matrix3x3 rot = Matrix3x3::FromAxisAngle(axis, angle);
    Vector3 v(0.0f, 1.0f, 0.0f);
    Vector3 rotated = rot * v;
    
    EXPECT_NEAR(rotated.x, 0.0f, 1e-6f);
    EXPECT_NEAR(rotated.y, 0.0f, 1e-6f);
    EXPECT_NEAR(rotated.z, 1.0f, 1e-6f);
}

TEST_F(Matrix3Test, OrthogonalityPreservation) {
    // Test that rotation matrices preserve orthogonality
    Vector3 x(1.0f, 0.0f, 0.0f);
    Vector3 y(0.0f, 1.0f, 0.0f);
    
    Vector3 rotX = rotZ90 * x;
    Vector3 rotY = rotZ90 * y;
    
    // Rotated vectors should still be perpendicular
    float dot = rotX.x * rotY.x + rotX.y * rotY.y + rotX.z * rotY.z;
    EXPECT_NEAR(dot, 0.0f, 1e-6f);
    
    // And maintain unit length
    EXPECT_NEAR(rotX.x * rotX.x + rotX.y * rotX.y + rotX.z * rotX.z, 1.0f, 1e-6f);
    EXPECT_NEAR(rotY.x * rotY.x + rotY.y * rotY.y + rotY.z * rotY.z, 1.0f, 1e-6f);
}