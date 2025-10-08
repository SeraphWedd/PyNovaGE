#include "matrix3.hpp"
#include "math_constants.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace pynovage::math;
using namespace pynovage::math::constants;

class Matrix3Test : public ::testing::Test {
protected:
    void SetUp() override {
        identity = Matrix3();
        
        rotX90 = Matrix3::rotationX(half_pi);
        rotY90 = Matrix3::rotationY(half_pi);
        rotZ90 = Matrix3::rotationZ(half_pi);
        
        scale2 = Matrix3::scale(2.0f, 2.0f, 2.0f);
    }

    Matrix3 identity;
    Matrix3 rotX90;
    Matrix3 rotY90;
    Matrix3 rotZ90;
    Matrix3 scale2;
};

TEST_F(Matrix3Test, DefaultConstruction) {
    Matrix3 m;
    EXPECT_FLOAT_EQ(m.m00, 1.0f);
    EXPECT_FLOAT_EQ(m.m01, 0.0f);
    EXPECT_FLOAT_EQ(m.m02, 0.0f);
    EXPECT_FLOAT_EQ(m.m10, 0.0f);
    EXPECT_FLOAT_EQ(m.m11, 1.0f);
    EXPECT_FLOAT_EQ(m.m12, 0.0f);
    EXPECT_FLOAT_EQ(m.m20, 0.0f);
    EXPECT_FLOAT_EQ(m.m21, 0.0f);
    EXPECT_FLOAT_EQ(m.m22, 1.0f);
}

TEST_F(Matrix3Test, ValueConstruction) {
    Matrix3 m1(1.0f, 2.0f, 3.0f,
                4.0f, 5.0f, 6.0f,
                7.0f, 8.0f, 9.0f);
    
    EXPECT_FLOAT_EQ(m1.m00, 1.0f);
    EXPECT_FLOAT_EQ(m1.m01, 2.0f);
    EXPECT_FLOAT_EQ(m1.m02, 3.0f);
    EXPECT_FLOAT_EQ(m1.m10, 4.0f);
    EXPECT_FLOAT_EQ(m1.m11, 5.0f);
    EXPECT_FLOAT_EQ(m1.m12, 6.0f);
    EXPECT_FLOAT_EQ(m1.m20, 7.0f);
    EXPECT_FLOAT_EQ(m1.m21, 8.0f);
    EXPECT_FLOAT_EQ(m1.m22, 9.0f);
}

TEST_F(Matrix3Test, Identity) {
    Matrix3 m = Matrix3::identity();
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
    Matrix3 result = rotZ90 * rotX90;
    EXPECT_EQ(result * identity, result);
    EXPECT_EQ(identity * result, result);

    // Test matrix * scalar
    Matrix3 scaled = identity * 2.0f;
    EXPECT_FLOAT_EQ(scaled.m00, 2.0f);
    EXPECT_FLOAT_EQ(scaled.m11, 2.0f);
    EXPECT_FLOAT_EQ(scaled.m22, 2.0f);
}

TEST_F(Matrix3Test, Addition) {
    Matrix3 m1(1.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f,
               0.0f, 0.0f, 1.0f);
    
    Matrix3 m2(2.0f, 0.0f, 0.0f,
               0.0f, 2.0f, 0.0f,
               0.0f, 0.0f, 2.0f);
    
    Matrix3 sum = m1 + m2;
    EXPECT_FLOAT_EQ(sum.m00, 3.0f);
    EXPECT_FLOAT_EQ(sum.m11, 3.0f);
    EXPECT_FLOAT_EQ(sum.m22, 3.0f);
}

TEST_F(Matrix3Test, Subtraction) {
    Matrix3 m1(2.0f, 0.0f, 0.0f,
               0.0f, 2.0f, 0.0f,
               0.0f, 0.0f, 2.0f);
    
    Matrix3 m2(1.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f,
               0.0f, 0.0f, 1.0f);
    
    Matrix3 diff = m1 - m2;
    EXPECT_FLOAT_EQ(diff.m00, 1.0f);
    EXPECT_FLOAT_EQ(diff.m11, 1.0f);
    EXPECT_FLOAT_EQ(diff.m22, 1.0f);
}

TEST_F(Matrix3Test, Transpose) {
    Matrix3 m(1.0f, 2.0f, 3.0f,
              4.0f, 5.0f, 6.0f,
              7.0f, 8.0f, 9.0f);
    
    Matrix3 mt = m.transposed();
    EXPECT_FLOAT_EQ(mt.m01, m.m10);
    EXPECT_FLOAT_EQ(mt.m02, m.m20);
    EXPECT_FLOAT_EQ(mt.m12, m.m21);
    
    m.transpose();
    EXPECT_EQ(m, mt);
}

TEST_F(Matrix3Test, Determinant) {
    Matrix3 m(1.0f, 2.0f, 3.0f,
                4.0f, 5.0f, 6.0f,
                7.0f, 8.0f, 9.0f);
    
    // For this matrix, det = 0
    EXPECT_NEAR(m.determinant(), 0.0f, 1e-6f);
    
    Matrix3 invertible(2.0f, -1.0f, 0.0f,
                        -1.0f, 2.0f, -1.0f,
                        0.0f, -1.0f, 2.0f);
    
    // This matrix has det = 4
    EXPECT_NEAR(invertible.determinant(), 4.0f, 1e-6f);
}

TEST_F(Matrix3Test, Inverse) {
    Matrix3 m1(2.0f, -1.0f, 0.0f,
                 -1.0f, 2.0f, -1.0f,
                 0.0f, -1.0f, 2.0f);
    
    Matrix3 m2;
    Matrix3 inv;
    EXPECT_TRUE(m1.getInverse(inv));
    
    Matrix3 result = m1 * inv;
    EXPECT_EQ(result, identity);
    
    // Test non-invertible matrix
    Matrix3 singular(1.0f, 2.0f, 3.0f,
                      4.0f, 5.0f, 6.0f,
                      7.0f, 8.0f, 9.0f);
    EXPECT_FALSE(singular.getInverse(inv));
}

TEST_F(Matrix3Test, AxisAngle) {
    Vector3 axis(1.0f, 0.0f, 0.0f);
    float angle = half_pi;
    
    Matrix3 rot = Matrix3::fromAxisAngle(axis, angle);
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