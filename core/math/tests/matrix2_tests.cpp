#include <gtest/gtest.h>
#include "math/matrix2.hpp"
#include "math/math_constants.hpp"
#include <cmath>

using namespace pynovage::math;

class Matrix2Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test matrices
        m1 = Matrix2(1, 2,
                     3, 4);
        m2 = Matrix2(5, 6,
                     7, 8);
    }

    Matrix2 m1, m2;
    const float epsilon = 1e-6f;
};

// Construction tests
TEST_F(Matrix2Test, DefaultConstruction) {
    Matrix2 m;
    EXPECT_FLOAT_EQ(m.m[0], 1.0f);  // Identity matrix
    EXPECT_FLOAT_EQ(m.m[1], 0.0f);
    EXPECT_FLOAT_EQ(m.m[2], 0.0f);
    EXPECT_FLOAT_EQ(m.m[3], 1.0f);
}

TEST_F(Matrix2Test, ValueConstruction) {
    EXPECT_FLOAT_EQ(m1.m[0], 1.0f);
    EXPECT_FLOAT_EQ(m1.m[1], 2.0f);
    EXPECT_FLOAT_EQ(m1.m[2], 3.0f);
    EXPECT_FLOAT_EQ(m1.m[3], 4.0f);
}

// Static factory methods
TEST_F(Matrix2Test, Identity) {
    Matrix2 m = Matrix2::identity();
    EXPECT_FLOAT_EQ(m.m[0], 1.0f);
    EXPECT_FLOAT_EQ(m.m[1], 0.0f);
    EXPECT_FLOAT_EQ(m.m[2], 0.0f);
    EXPECT_FLOAT_EQ(m.m[3], 1.0f);
}

TEST_F(Matrix2Test, Scale) {
    Matrix2 m = Matrix2::scale(2.0f, 3.0f);
    EXPECT_FLOAT_EQ(m.m[0], 2.0f);
    EXPECT_FLOAT_EQ(m.m[1], 0.0f);
    EXPECT_FLOAT_EQ(m.m[2], 0.0f);
    EXPECT_FLOAT_EQ(m.m[3], 3.0f);
}

TEST_F(Matrix2Test, Rotation) {
float angle = pynovage::math::constants::half_pi;  // 90 degrees
    Matrix2 m = Matrix2::rotation(angle);
    EXPECT_NEAR(m.m[0], 0.0f, epsilon);   // cos(90°) ≈ 0
    EXPECT_NEAR(m.m[1], -1.0f, epsilon);  // -sin(90°) ≈ -1
    EXPECT_NEAR(m.m[2], 1.0f, epsilon);   // sin(90°) ≈ 1
    EXPECT_NEAR(m.m[3], 0.0f, epsilon);   // cos(90°) ≈ 0
}

// Matrix operations
TEST_F(Matrix2Test, Multiplication) {
    Matrix2 result = m1 * m2;
    EXPECT_FLOAT_EQ(result.m[0], 19.0f);  // 1*5 + 2*7
    EXPECT_FLOAT_EQ(result.m[1], 22.0f);  // 1*6 + 2*8
    EXPECT_FLOAT_EQ(result.m[2], 43.0f);  // 3*5 + 4*7
    EXPECT_FLOAT_EQ(result.m[3], 50.0f);  // 3*6 + 4*8
}

TEST_F(Matrix2Test, VectorMultiplication) {
    Vector2 v(2.0f, 3.0f);
    Vector2 result = m1 * v;
    EXPECT_FLOAT_EQ(result.x, 8.0f);   // 1*2 + 2*3
    EXPECT_FLOAT_EQ(result.y, 18.0f);  // 3*2 + 4*3
}

TEST_F(Matrix2Test, Determinant) {
    float det = m1.determinant();
    EXPECT_FLOAT_EQ(det, -2.0f);  // 1*4 - 2*3
}

TEST_F(Matrix2Test, Inverse) {
    Matrix2 inv;
    EXPECT_TRUE(m1.inverse(inv));
    
    // Check that m1 * inv = identity
    Matrix2 result = m1 * inv;
    EXPECT_NEAR(result.m[0], 1.0f, epsilon);
    EXPECT_NEAR(result.m[1], 0.0f, epsilon);
    EXPECT_NEAR(result.m[2], 0.0f, epsilon);
    EXPECT_NEAR(result.m[3], 1.0f, epsilon);
}

TEST_F(Matrix2Test, InverseNonInvertible) {
    Matrix2 singular(1.0f, 2.0f,
                    0.5f, 1.0f);  // linearly dependent rows
    Matrix2 inv;
    EXPECT_FALSE(singular.inverse(inv));
}

TEST_F(Matrix2Test, Transpose) {
    Matrix2 transposed = m1.transposed();
    EXPECT_FLOAT_EQ(transposed.m[0], 1.0f);
    EXPECT_FLOAT_EQ(transposed.m[1], 3.0f);
    EXPECT_FLOAT_EQ(transposed.m[2], 2.0f);
    EXPECT_FLOAT_EQ(transposed.m[3], 4.0f);
}

TEST_F(Matrix2Test, TransposeInPlace) {
    m1.transposeInPlace();
    EXPECT_FLOAT_EQ(m1.m[0], 1.0f);
    EXPECT_FLOAT_EQ(m1.m[1], 3.0f);
    EXPECT_FLOAT_EQ(m1.m[2], 2.0f);
    EXPECT_FLOAT_EQ(m1.m[3], 4.0f);
}

// Special cases and edge cases
TEST_F(Matrix2Test, RotationFullCircle) {
Matrix2 rot = Matrix2::rotation(pynovage::math::constants::two_pi);
    EXPECT_NEAR(rot.m[0], 1.0f, epsilon);
    EXPECT_NEAR(rot.m[1], 0.0f, epsilon);
    EXPECT_NEAR(rot.m[2], 0.0f, epsilon);
    EXPECT_NEAR(rot.m[3], 1.0f, epsilon);
}

TEST_F(Matrix2Test, ScaleZero) {
    Matrix2 scale = Matrix2::scale(0.0f, 0.0f);
    Vector2 v(1.0f, 1.0f);
    Vector2 result = scale * v;
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
}

TEST_F(Matrix2Test, ScaleNegative) {
    Matrix2 scale = Matrix2::scale(-1.0f, -1.0f);
    Vector2 v(1.0f, 2.0f);
    Vector2 result = scale * v;
    EXPECT_FLOAT_EQ(result.x, -1.0f);
    EXPECT_FLOAT_EQ(result.y, -2.0f);
}

TEST_F(Matrix2Test, MultiplyIdentity) {
    Matrix2 id = Matrix2::identity();
    Matrix2 result = m1 * id;
    EXPECT_FLOAT_EQ(result.m[0], m1.m[0]);
    EXPECT_FLOAT_EQ(result.m[1], m1.m[1]);
    EXPECT_FLOAT_EQ(result.m[2], m1.m[2]);
    EXPECT_FLOAT_EQ(result.m[3], m1.m[3]);
}

TEST_F(Matrix2Test, RotationOrthogonality) {
float angle = pynovage::math::constants::pi / 6.0f;  // 30 degrees
    Matrix2 rot = Matrix2::rotation(angle);
    Vector2 v1(1.0f, 0.0f);
    Vector2 v2 = rot * v1;
    // Rotated vector should have same length
    EXPECT_NEAR(v2.length(), v1.length(), epsilon);
    // Angle between vectors should be 30 degrees
    float dot = v1.dot(v2);
    EXPECT_NEAR(dot, std::cos(angle), epsilon);
}