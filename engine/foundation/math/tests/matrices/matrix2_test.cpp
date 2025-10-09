#include <gtest/gtest.h>
#include <cmath>

#include "../../include/matrices/matrix2.hpp"

using namespace PyNovaGE;

TEST(Matrix2Test, Construction) {
    // Default constructor (Identity)
    Matrix2<float> identity;
    EXPECT_FLOAT_EQ(identity[0][0], 1.0f);
    EXPECT_FLOAT_EQ(identity[0][1], 0.0f);
    EXPECT_FLOAT_EQ(identity[1][0], 0.0f);
    EXPECT_FLOAT_EQ(identity[1][1], 1.0f);

    // Constructor with individual elements
    Matrix2<float> mat(2.0f, 3.0f, 4.0f, 5.0f);
    EXPECT_FLOAT_EQ(mat[0][0], 2.0f);
    EXPECT_FLOAT_EQ(mat[0][1], 3.0f);
    EXPECT_FLOAT_EQ(mat[1][0], 4.0f);
    EXPECT_FLOAT_EQ(mat[1][1], 5.0f);

    // Constructor with Vector2s
    Vector2<float> row0(1.0f, 2.0f);
    Vector2<float> row1(3.0f, 4.0f);
    Matrix2<float> mat2(row0, row1);
    EXPECT_FLOAT_EQ(mat2[0][0], 1.0f);
    EXPECT_FLOAT_EQ(mat2[0][1], 2.0f);
    EXPECT_FLOAT_EQ(mat2[1][0], 3.0f);
    EXPECT_FLOAT_EQ(mat2[1][1], 4.0f);
}

TEST(Matrix2Test, BasicOperations) {
    Matrix2<float> m1(1.0f, 2.0f, 3.0f, 4.0f);
    Matrix2<float> m2(5.0f, 6.0f, 7.0f, 8.0f);

    // Addition
    Matrix2<float> sum = m1 + m2;
    EXPECT_FLOAT_EQ(sum[0][0], 6.0f);
    EXPECT_FLOAT_EQ(sum[0][1], 8.0f);
    EXPECT_FLOAT_EQ(sum[1][0], 10.0f);
    EXPECT_FLOAT_EQ(sum[1][1], 12.0f);

    // Subtraction
    Matrix2<float> diff = m2 - m1;
    EXPECT_FLOAT_EQ(diff[0][0], 4.0f);
    EXPECT_FLOAT_EQ(diff[0][1], 4.0f);
    EXPECT_FLOAT_EQ(diff[1][0], 4.0f);
    EXPECT_FLOAT_EQ(diff[1][1], 4.0f);

    // Scalar multiplication
    Matrix2<float> scaled = m1 * 2.0f;
    EXPECT_FLOAT_EQ(scaled[0][0], 2.0f);
    EXPECT_FLOAT_EQ(scaled[0][1], 4.0f);
    EXPECT_FLOAT_EQ(scaled[1][0], 6.0f);
    EXPECT_FLOAT_EQ(scaled[1][1], 8.0f);
}

TEST(Matrix2Test, MatrixMultiplication) {
    Matrix2<float> m1(1.0f, 2.0f, 3.0f, 4.0f);
    Matrix2<float> m2(5.0f, 6.0f, 7.0f, 8.0f);

    Matrix2<float> product = m1 * m2;
    EXPECT_FLOAT_EQ(product[0][0], 19.0f);  // 1*5 + 2*7
    EXPECT_FLOAT_EQ(product[0][1], 22.0f);  // 1*6 + 2*8
    EXPECT_FLOAT_EQ(product[1][0], 43.0f);  // 3*5 + 4*7
    EXPECT_FLOAT_EQ(product[1][1], 50.0f);  // 3*6 + 4*8
}

TEST(Matrix2Test, VectorMultiplication) {
    Matrix2<float> m(1.0f, 2.0f, 3.0f, 4.0f);
    Vector2<float> v(2.0f, 3.0f);

    Vector2<float> result = m * v;
    EXPECT_FLOAT_EQ(result[0], 8.0f);   // 1*2 + 2*3
    EXPECT_FLOAT_EQ(result[1], 18.0f);  // 3*2 + 4*3
}

TEST(Matrix2Test, Determinant) {
    Matrix2<float> m(1.0f, 2.0f, 3.0f, 4.0f);
    float det = m.determinant();
    EXPECT_FLOAT_EQ(det, -2.0f);  // 1*4 - 2*3
}

TEST(Matrix2Test, Inverse) {
    Matrix2<float> m(4.0f, 7.0f, 2.0f, 6.0f);
    Matrix2<float> inv = m.inverse();
    
    float det = 4.0f * 6.0f - 7.0f * 2.0f;
    float invDet = 1.0f / det;
    
    EXPECT_FLOAT_EQ(inv[0][0], 6.0f * invDet);
    EXPECT_FLOAT_EQ(inv[0][1], -7.0f * invDet);
    EXPECT_FLOAT_EQ(inv[1][0], -2.0f * invDet);
    EXPECT_FLOAT_EQ(inv[1][1], 4.0f * invDet);

    // Test multiplication with inverse gives identity
    Matrix2<float> identity = m * inv;
    EXPECT_NEAR(identity[0][0], 1.0f, 1e-6f);
    EXPECT_NEAR(identity[0][1], 0.0f, 1e-6f);
    EXPECT_NEAR(identity[1][0], 0.0f, 1e-6f);
    EXPECT_NEAR(identity[1][1], 1.0f, 1e-6f);
}

TEST(Matrix2Test, Transpose) {
    Matrix2<float> m(1.0f, 2.0f, 3.0f, 4.0f);
    Matrix2<float> t = m.transpose();
    
    EXPECT_FLOAT_EQ(t[0][0], 1.0f);
    EXPECT_FLOAT_EQ(t[0][1], 3.0f);
    EXPECT_FLOAT_EQ(t[1][0], 2.0f);
    EXPECT_FLOAT_EQ(t[1][1], 4.0f);
}

TEST(Matrix2Test, StaticCreators) {
    // Identity
    Matrix2<float> identity = Matrix2<float>::Identity();
    EXPECT_FLOAT_EQ(identity[0][0], 1.0f);
    EXPECT_FLOAT_EQ(identity[0][1], 0.0f);
    EXPECT_FLOAT_EQ(identity[1][0], 0.0f);
    EXPECT_FLOAT_EQ(identity[1][1], 1.0f);

    // Rotation (45 degrees)
    float angle = M_PI / 4.0f; // 45 degrees
    Matrix2<float> rotation = Matrix2<float>::Rotation(angle);
    float s = std::sin(angle);
    float c = std::cos(angle);
    EXPECT_FLOAT_EQ(rotation[0][0], c);
    EXPECT_FLOAT_EQ(rotation[0][1], -s);
    EXPECT_FLOAT_EQ(rotation[1][0], s);
    EXPECT_FLOAT_EQ(rotation[1][1], c);

    // Scale
    Matrix2<float> scale = Matrix2<float>::Scale(2.0f, 3.0f);
    EXPECT_FLOAT_EQ(scale[0][0], 2.0f);
    EXPECT_FLOAT_EQ(scale[0][1], 0.0f);
    EXPECT_FLOAT_EQ(scale[1][0], 0.0f);
    EXPECT_FLOAT_EQ(scale[1][1], 3.0f);
}