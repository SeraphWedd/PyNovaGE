#include <gtest/gtest.h>
#include <cmath>

#include "../../include/matrices/matrix3.hpp"

using namespace PyNovaGE;

TEST(Matrix3Test, Construction) {
    // Default constructor (Identity)
    Matrix3<float> identity;
    EXPECT_FLOAT_EQ(identity[0][0], 1.0f);
    EXPECT_FLOAT_EQ(identity[0][1], 0.0f);
    EXPECT_FLOAT_EQ(identity[0][2], 0.0f);
    EXPECT_FLOAT_EQ(identity[1][0], 0.0f);
    EXPECT_FLOAT_EQ(identity[1][1], 1.0f);
    EXPECT_FLOAT_EQ(identity[1][2], 0.0f);
    EXPECT_FLOAT_EQ(identity[2][0], 0.0f);
    EXPECT_FLOAT_EQ(identity[2][1], 0.0f);
    EXPECT_FLOAT_EQ(identity[2][2], 1.0f);

    // Constructor with individual elements
    Matrix3<float> mat(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    EXPECT_FLOAT_EQ(mat[0][0], 1.0f);
    EXPECT_FLOAT_EQ(mat[0][1], 2.0f);
    EXPECT_FLOAT_EQ(mat[0][2], 3.0f);
    EXPECT_FLOAT_EQ(mat[1][0], 4.0f);
    EXPECT_FLOAT_EQ(mat[1][1], 5.0f);
    EXPECT_FLOAT_EQ(mat[1][2], 6.0f);
    EXPECT_FLOAT_EQ(mat[2][0], 7.0f);
    EXPECT_FLOAT_EQ(mat[2][1], 8.0f);
    EXPECT_FLOAT_EQ(mat[2][2], 9.0f);

    // Constructor with Vector3s
    Vector3<float> row0(1.0f, 2.0f, 3.0f);
    Vector3<float> row1(4.0f, 5.0f, 6.0f);
    Vector3<float> row2(7.0f, 8.0f, 9.0f);
    Matrix3<float> mat2(row0, row1, row2);
    EXPECT_FLOAT_EQ(mat2[0][0], 1.0f);
    EXPECT_FLOAT_EQ(mat2[0][1], 2.0f);
    EXPECT_FLOAT_EQ(mat2[0][2], 3.0f);
    EXPECT_FLOAT_EQ(mat2[1][0], 4.0f);
    EXPECT_FLOAT_EQ(mat2[1][1], 5.0f);
    EXPECT_FLOAT_EQ(mat2[1][2], 6.0f);
    EXPECT_FLOAT_EQ(mat2[2][0], 7.0f);
    EXPECT_FLOAT_EQ(mat2[2][1], 8.0f);
    EXPECT_FLOAT_EQ(mat2[2][2], 9.0f);
}

TEST(Matrix3Test, BasicOperations) {
    Matrix3<float> m1(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    Matrix3<float> m2(
        9.0f, 8.0f, 7.0f,
        6.0f, 5.0f, 4.0f,
        3.0f, 2.0f, 1.0f
    );

    // Addition
    Matrix3<float> sum = m1 + m2;
    EXPECT_FLOAT_EQ(sum[0][0], 10.0f);
    EXPECT_FLOAT_EQ(sum[0][1], 10.0f);
    EXPECT_FLOAT_EQ(sum[0][2], 10.0f);
    EXPECT_FLOAT_EQ(sum[1][0], 10.0f);
    EXPECT_FLOAT_EQ(sum[1][1], 10.0f);
    EXPECT_FLOAT_EQ(sum[1][2], 10.0f);
    EXPECT_FLOAT_EQ(sum[2][0], 10.0f);
    EXPECT_FLOAT_EQ(sum[2][1], 10.0f);
    EXPECT_FLOAT_EQ(sum[2][2], 10.0f);

    // Subtraction
    Matrix3<float> diff = m1 - m2;
    EXPECT_FLOAT_EQ(diff[0][0], -8.0f);
    EXPECT_FLOAT_EQ(diff[0][1], -6.0f);
    EXPECT_FLOAT_EQ(diff[0][2], -4.0f);
    EXPECT_FLOAT_EQ(diff[1][0], -2.0f);
    EXPECT_FLOAT_EQ(diff[1][1], 0.0f);
    EXPECT_FLOAT_EQ(diff[1][2], 2.0f);
    EXPECT_FLOAT_EQ(diff[2][0], 4.0f);
    EXPECT_FLOAT_EQ(diff[2][1], 6.0f);
    EXPECT_FLOAT_EQ(diff[2][2], 8.0f);

    // Scalar multiplication
    Matrix3<float> scaled = m1 * 2.0f;
    EXPECT_FLOAT_EQ(scaled[0][0], 2.0f);
    EXPECT_FLOAT_EQ(scaled[0][1], 4.0f);
    EXPECT_FLOAT_EQ(scaled[0][2], 6.0f);
    EXPECT_FLOAT_EQ(scaled[1][0], 8.0f);
    EXPECT_FLOAT_EQ(scaled[1][1], 10.0f);
    EXPECT_FLOAT_EQ(scaled[1][2], 12.0f);
    EXPECT_FLOAT_EQ(scaled[2][0], 14.0f);
    EXPECT_FLOAT_EQ(scaled[2][1], 16.0f);
    EXPECT_FLOAT_EQ(scaled[2][2], 18.0f);
}

TEST(Matrix3Test, MatrixMultiplication) {
    Matrix3<float> m1(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    Matrix3<float> m2(
        9.0f, 8.0f, 7.0f,
        6.0f, 5.0f, 4.0f,
        3.0f, 2.0f, 1.0f
    );

    Matrix3<float> product = m1 * m2;
    EXPECT_FLOAT_EQ(product[0][0], 30.0f);
    EXPECT_FLOAT_EQ(product[0][1], 24.0f);
    EXPECT_FLOAT_EQ(product[0][2], 18.0f);
    EXPECT_FLOAT_EQ(product[1][0], 84.0f);
    EXPECT_FLOAT_EQ(product[1][1], 69.0f);
    EXPECT_FLOAT_EQ(product[1][2], 54.0f);
    EXPECT_FLOAT_EQ(product[2][0], 138.0f);
    EXPECT_FLOAT_EQ(product[2][1], 114.0f);
    EXPECT_FLOAT_EQ(product[2][2], 90.0f);
}

TEST(Matrix3Test, VectorMultiplication) {
    Matrix3<float> m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    Vector3<float> v(2.0f, 1.0f, 3.0f);

    Vector3<float> result = m * v;
    EXPECT_FLOAT_EQ(result[0], 13.0f);  // 1*2 + 2*1 + 3*3
    EXPECT_FLOAT_EQ(result[1], 31.0f);  // 4*2 + 5*1 + 6*3
    EXPECT_FLOAT_EQ(result[2], 49.0f);  // 7*2 + 8*1 + 9*3
}

TEST(Matrix3Test, Determinant) {
    Matrix3<float> m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    float det = m.determinant();
    EXPECT_FLOAT_EQ(det, 0.0f);  // This matrix is singular

    Matrix3<float> m2(
        2.0f, -1.0f, 1.0f,
        3.0f, 2.0f, -2.0f,
        1.0f, 1.0f, 1.0f
    );
    det = m2.determinant();
    EXPECT_FLOAT_EQ(det, 14.0f);
}

TEST(Matrix3Test, Inverse) {
    Matrix3<float> m(
        2.0f, -1.0f, 1.0f,
        3.0f, 2.0f, -2.0f,
        1.0f, 1.0f, 1.0f
    );
    Matrix3<float> inv = m.inverse();
    
    // Test multiplication with inverse gives identity
    Matrix3<float> identity = m * inv;
    EXPECT_NEAR(identity[0][0], 1.0f, 1e-6f);
    EXPECT_NEAR(identity[0][1], 0.0f, 1e-6f);
    EXPECT_NEAR(identity[0][2], 0.0f, 1e-6f);
    EXPECT_NEAR(identity[1][0], 0.0f, 1e-6f);
    EXPECT_NEAR(identity[1][1], 1.0f, 1e-6f);
    EXPECT_NEAR(identity[1][2], 0.0f, 1e-6f);
    EXPECT_NEAR(identity[2][0], 0.0f, 1e-6f);
    EXPECT_NEAR(identity[2][1], 0.0f, 1e-6f);
    EXPECT_NEAR(identity[2][2], 1.0f, 1e-6f);
}

TEST(Matrix3Test, Transpose) {
    Matrix3<float> m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    Matrix3<float> t = m.transpose();
    
    EXPECT_FLOAT_EQ(t[0][0], 1.0f);
    EXPECT_FLOAT_EQ(t[0][1], 4.0f);
    EXPECT_FLOAT_EQ(t[0][2], 7.0f);
    EXPECT_FLOAT_EQ(t[1][0], 2.0f);
    EXPECT_FLOAT_EQ(t[1][1], 5.0f);
    EXPECT_FLOAT_EQ(t[1][2], 8.0f);
    EXPECT_FLOAT_EQ(t[2][0], 3.0f);
    EXPECT_FLOAT_EQ(t[2][1], 6.0f);
    EXPECT_FLOAT_EQ(t[2][2], 9.0f);
}

TEST(Matrix3Test, Transformations) {
    // Test 2D rotation
    float angle = M_PI / 4.0f; // 45 degrees
    Matrix3<float> rotation = Matrix3<float>::Rotation(angle);
    float s = std::sin(angle);
    float c = std::cos(angle);
    EXPECT_FLOAT_EQ(rotation[0][0], c);
    EXPECT_FLOAT_EQ(rotation[0][1], -s);
    EXPECT_FLOAT_EQ(rotation[1][0], s);
    EXPECT_FLOAT_EQ(rotation[1][1], c);
    EXPECT_FLOAT_EQ(rotation[2][2], 1.0f);

    // Test scaling
    Matrix3<float> scale = Matrix3<float>::Scale(2.0f, 3.0f);
    EXPECT_FLOAT_EQ(scale[0][0], 2.0f);
    EXPECT_FLOAT_EQ(scale[1][1], 3.0f);
    EXPECT_FLOAT_EQ(scale[2][2], 1.0f);

    // Test translation
    Matrix3<float> translation = Matrix3<float>::Translation(2.0f, 3.0f);
    EXPECT_FLOAT_EQ(translation[0][2], 2.0f);
    EXPECT_FLOAT_EQ(translation[1][2], 3.0f);
    EXPECT_FLOAT_EQ(translation[2][2], 1.0f);

    // Test combined transformation
    Vector3<float> point(1.0f, 1.0f, 1.0f);
    Vector3<float> transformed = translation * scale * rotation * point;
    // Point should be rotated, then scaled, then translated
    // After rotation: (c - s, s + c, 1)
    // After scaling: (2(c - s), 3(s + c), 1)
    // After translation: (2(c - s) + 2, 3(s + c) + 3, 1)
    EXPECT_NEAR(transformed[0], 2.0f * (c - s) + 2.0f, 1e-5f);
    EXPECT_NEAR(transformed[1], 3.0f * (s + c) + 3.0f, 1e-5f);
    EXPECT_FLOAT_EQ(transformed[2], 1.0f);
}