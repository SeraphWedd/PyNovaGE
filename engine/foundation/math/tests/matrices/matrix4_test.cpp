#include <gtest/gtest.h>
#include <cmath>

#include "../../include/matrices/matrix4.hpp"

using namespace PyNovaGE;

TEST(Matrix4Test, Construction) {
    // Default constructor (Identity)
    Matrix4<float> identity;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j)
                EXPECT_FLOAT_EQ(identity[i][j], 1.0f);
            else
                EXPECT_FLOAT_EQ(identity[i][j], 0.0f);
        }
    }

    // Constructor with individual elements
    Matrix4<float> mat(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    );
    float value = 1.0f;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(mat[i][j], value);
            value += 1.0f;
        }
    }

    // Constructor with Vector4s
    Vector4<float> row0(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4<float> row1(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4<float> row2(9.0f, 10.0f, 11.0f, 12.0f);
    Vector4<float> row3(13.0f, 14.0f, 15.0f, 16.0f);
    Matrix4<float> mat2(row0, row1, row2, row3);
    value = 1.0f;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(mat2[i][j], value);
            value += 1.0f;
        }
    }
}

TEST(Matrix4Test, BasicOperations) {
    Matrix4<float> m1(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    );
    Matrix4<float> m2(
        16.0f, 15.0f, 14.0f, 13.0f,
        12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f,
        4.0f, 3.0f, 2.0f, 1.0f
    );

    // Addition
    Matrix4<float> sum = m1 + m2;
    float expected_sum = 17.0f;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(sum[i][j], expected_sum);
        }
    }

    // Subtraction
    Matrix4<float> diff = m2 - m1;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float expected = 16.0f - i * 4.0f - j - (1.0f + i * 4.0f + j);
            EXPECT_FLOAT_EQ(diff[i][j], expected);
        }
    }

    // Scalar multiplication
    Matrix4<float> scaled = m1 * 2.0f;
    float value = 2.0f;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(scaled[i][j], value);
            value += 2.0f;
        }
    }
}

TEST(Matrix4Test, MatrixMultiplication) {
    Matrix4<float> m1(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    );
    Matrix4<float> m2(
        16.0f, 15.0f, 14.0f, 13.0f,
        12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f,
        4.0f, 3.0f, 2.0f, 1.0f
    );

    Matrix4<float> product = m1 * m2;
    // Verify a few key elements (full verification would be too verbose)
    EXPECT_FLOAT_EQ(product[0][0], 80.0f);   // 1*16 + 2*12 + 3*8 + 4*4
    EXPECT_FLOAT_EQ(product[0][3], 50.0f);   // 1*13 + 2*9 + 3*5 + 4*1
    EXPECT_FLOAT_EQ(product[3][0], 560.0f);  // 13*16 + 14*12 + 15*8 + 16*4
    EXPECT_FLOAT_EQ(product[3][3], 386.0f);  // 13*13 + 14*9 + 15*5 + 16*1
}

TEST(Matrix4Test, VectorMultiplication) {
    Matrix4<float> m(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    );
    Vector4<float> v(2.0f, 1.0f, 3.0f, 4.0f);

    Vector4<float> result = m * v;
    EXPECT_FLOAT_EQ(result[0], 29.0f);   // 1*2 + 2*1 + 3*3 + 4*4
    EXPECT_FLOAT_EQ(result[1], 69.0f);   // 5*2 + 6*1 + 7*3 + 8*4
    EXPECT_FLOAT_EQ(result[2], 109.0f);  // 9*2 + 10*1 + 11*3 + 12*4
    EXPECT_FLOAT_EQ(result[3], 149.0f);  // 13*2 + 14*1 + 15*3 + 16*4
}

TEST(Matrix4Test, Transformations) {
    // Test translation
    Vector4<float> point(1.0f, 2.0f, 3.0f, 1.0f);
    Matrix4<float> translation = Matrix4<float>::Translation(2.0f, 3.0f, 4.0f);
    Vector4<float> translated = translation * point;
    EXPECT_FLOAT_EQ(translated[0], 3.0f); // x + 2
    EXPECT_FLOAT_EQ(translated[1], 5.0f); // y + 3
    EXPECT_FLOAT_EQ(translated[2], 7.0f); // z + 4
    EXPECT_FLOAT_EQ(translated[3], 1.0f); // w unchanged

    // Test scaling
    Matrix4<float> scale = Matrix4<float>::Scale(2.0f, 3.0f, 4.0f);
    Vector4<float> scaled = scale * point;
    EXPECT_FLOAT_EQ(scaled[0], 2.0f);  // x * 2
    EXPECT_FLOAT_EQ(scaled[1], 6.0f);  // y * 3
    EXPECT_FLOAT_EQ(scaled[2], 12.0f); // z * 4
    EXPECT_FLOAT_EQ(scaled[3], 1.0f);  // w unchanged

    // Test rotation (45 degrees around Y-axis)
    float angle = M_PI / 4.0f;
    Matrix4<float> rotation = Matrix4<float>::RotationY(angle);
    Vector4<float> rotated = rotation * Vector4<float>(1.0f, 0.0f, 0.0f, 1.0f);
    float s = std::sin(angle);
    float c = std::cos(angle);
    EXPECT_NEAR(rotated[0], c, 1e-6f);
    EXPECT_NEAR(rotated[1], 0.0f, 1e-6f);
    EXPECT_NEAR(rotated[2], -s, 1e-6f);
    EXPECT_FLOAT_EQ(rotated[3], 1.0f);
}

TEST(Matrix4Test, ProjectionMatrices) {
    // Test perspective projection
    float fov = M_PI / 4.0f;
    float aspect = 16.0f / 9.0f;
    float near = 0.1f;
    float far = 100.0f;
    Matrix4<float> perspective = Matrix4<float>::Perspective(fov, aspect, near, far);
    
    // Test orthographic projection
    float left = -10.0f;
    float right = 10.0f;
    float bottom = -5.0f;
    float top = 5.0f;
    Matrix4<float> ortho = Matrix4<float>::Orthographic(left, right, bottom, top, near, far);

    // Test a point transformation with each
    Vector4<float> point(1.0f, 1.0f, -1.0f, 1.0f);
    Vector4<float> persp_transformed = perspective * point;
    Vector4<float> ortho_transformed = ortho * point;

    // Basic sanity checks
    EXPECT_LT(persp_transformed[2], persp_transformed[3]); // With our projection, Z should be less than W for this point
    EXPECT_LT(std::abs(ortho_transformed[0]), right);      // X should be within bounds for ortho
    EXPECT_LT(std::abs(ortho_transformed[1]), top);        // Y should be within bounds for ortho
}

TEST(Matrix4Test, LookAt) {
    Vector3<float> eye(0.0f, 0.0f, 5.0f);
    Vector3<float> target(0.0f, 0.0f, 0.0f);
    Vector3<float> up(0.0f, 1.0f, 0.0f);
    
    Matrix4<float> view = Matrix4<float>::LookAt(eye, target, up);
    
    // A point at the target should be transformed to have negative Z
    Vector4<float> point(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4<float> transformed = view * point;
    EXPECT_NEAR(transformed[2], -5.0f, 1e-6f);
}