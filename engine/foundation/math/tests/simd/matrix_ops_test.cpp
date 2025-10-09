#include <gtest/gtest.h>
#include "../../include/simd/matrix_ops.hpp"
#include <cmath>

namespace {

using namespace PyNovaGE::SIMD;

// Helper function to compare vectors with tolerance
template<typename T, size_t N>
testing::AssertionResult ApproxEqual(const Vector<T, N>& a, const Vector<T, N>& b, T tolerance = T(1e-5)) {
    for (size_t i = 0; i < N; ++i) {
        if (std::abs(a[i] - b[i]) > tolerance) {
            return testing::AssertionFailure() 
                << "Vector elements differ at index " << i 
                << ". Expected " << a[i] 
                << ", got " << b[i];
        }
    }
    return testing::AssertionSuccess();
}

// Helper function to compare matrices with tolerance
template<typename T, size_t N>
testing::AssertionResult ApproxEqual(const Matrix<T, N>& a, const Matrix<T, N>& b, T tolerance = T(1e-5)) {
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            if (std::abs(a(i, j) - b(i, j)) > tolerance) {
                return testing::AssertionFailure() 
                    << "Matrix elements differ at (" << i << ", " << j 
                    << "). Expected " << a(i, j) 
                    << ", got " << b(i, j);
            }
        }
    }
    return testing::AssertionSuccess();
}

TEST(MatrixOpsTest, Construction) {
    // Test identity matrix construction
    Matrix4f m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j) {
                EXPECT_FLOAT_EQ(m(i, j), 1.0f);
            } else {
                EXPECT_FLOAT_EQ(m(i, j), 0.0f);
            }
        }
    }

    // Test array construction
    float data[16] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    };
    Matrix4f m2(data);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(m2(i, j), data[j * 4 + i]);
        }
    }
}

TEST(MatrixOpsTest, MatrixMultiplication) {
    // Test identity multiplication
    Matrix4f identity;
    Matrix4f m({
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    });

    Matrix4f result = m * identity;
    EXPECT_TRUE(ApproxEqual(result, m));

    // Test specific multiplication
    Matrix4f a({
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    });

    Matrix4f b({
        17.0f, 18.0f, 19.0f, 20.0f,
        21.0f, 22.0f, 23.0f, 24.0f,
        25.0f, 26.0f, 27.0f, 28.0f,
        29.0f, 30.0f, 31.0f, 32.0f
    });

    Matrix4f expected({
        250.0f, 260.0f, 270.0f, 280.0f,
        618.0f, 644.0f, 670.0f, 696.0f,
        986.0f, 1028.0f, 1070.0f, 1112.0f,
        1354.0f, 1412.0f, 1470.0f, 1528.0f
    });

    Matrix4f mult_result = a * b;
    EXPECT_TRUE(ApproxEqual(mult_result, expected));
}

TEST(MatrixOpsTest, MatrixVectorMultiplication) {
    Matrix4f m({
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    });

    Vector4f v(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f expected(30.0f, 70.0f, 110.0f, 150.0f);

    Vector4f result = m * v;
    EXPECT_TRUE(ApproxEqual(result, expected));

    // Test with identity matrix
    Matrix4f identity;
    Vector4f identity_result = identity * v;
    EXPECT_TRUE(ApproxEqual(identity_result, v));
}

TEST(MatrixOpsTest, Transpose) {
    Matrix4f m({
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    });

    Matrix4f expected({
        1.0f, 5.0f, 9.0f, 13.0f,
        2.0f, 6.0f, 10.0f, 14.0f,
        3.0f, 7.0f, 11.0f, 15.0f,
        4.0f, 8.0f, 12.0f, 16.0f
    });

    Matrix4f result = transpose(m);
    EXPECT_TRUE(ApproxEqual(result, expected));

    // Test double transpose equals original
    Matrix4f double_transpose = transpose(transpose(m));
    EXPECT_TRUE(ApproxEqual(double_transpose, m));
}

TEST(MatrixOpsTest, Translation) {
    Vector3f trans(1.0f, 2.0f, 3.0f);
    Matrix4f trans_matrix = translate(trans);
    Vector4f point(1.0f, 1.0f, 1.0f, 1.0f);

    Vector4f expected(2.0f, 3.0f, 4.0f, 1.0f);
    Vector4f result = trans_matrix * point;

    EXPECT_TRUE(ApproxEqual(result, expected));
}

TEST(MatrixOpsTest, Scale) {
    Vector3f scale_vec(2.0f, 3.0f, 4.0f);
    Matrix4f scale_matrix = scale(scale_vec);
    Vector4f point(1.0f, 1.0f, 1.0f, 1.0f);

    Vector4f expected(2.0f, 3.0f, 4.0f, 1.0f);
    Vector4f result = scale_matrix * point;

    EXPECT_TRUE(ApproxEqual(result, expected));
}

TEST(MatrixOpsTest, Rotation) {
    // Test rotation around X axis by 90 degrees
    Vector3f x_axis(1.0f, 0.0f, 0.0f);
    float angle = M_PI / 2.0f;
    Matrix4f rot_matrix = rotate(x_axis, angle);
    Vector4f point(0.0f, 1.0f, 0.0f, 1.0f);

    Vector4f expected(0.0f, 0.0f, 1.0f, 1.0f);
    Vector4f result = rot_matrix * point;

    EXPECT_TRUE(ApproxEqual(result, expected, 1e-5f));
}

TEST(MatrixOpsTest, Consistency) {
    // Test rotation matrix orthogonality
    Vector3f axis(1.0f, 1.0f, 1.0f);
    float angle = M_PI / 4.0f;
    Matrix4f rot = rotate(normalize(axis), angle);
    Matrix4f rot_transpose = transpose(rot);
    Matrix4f identity;

    // The product of a rotation matrix and its transpose should be identity
    Matrix4f result = rot * rot_transpose;
    EXPECT_TRUE(ApproxEqual(result, identity, 1e-5f));

    // Test scale matrix commutativity
    Vector3f scale_vec(2.0f, 3.0f, 4.0f);
    Matrix4f s1 = scale(scale_vec);
    Vector3f trans_vec(1.0f, 2.0f, 3.0f);
    Matrix4f t1 = translate(trans_vec);

    Vector4f point(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4f result1 = s1 * (t1 * point);
    Vector4f result2 = t1 * (s1 * point);

    // Scale and translation are not commutative
    EXPECT_FALSE(ApproxEqual(result1, result2));
}

TEST(MatrixOpsTest, SIMDAlignment) {
    // Create aligned matrices
    alignas(16) float data1[16] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    };
    
    alignas(16) float data2[16] = {
        17.0f, 18.0f, 19.0f, 20.0f,
        21.0f, 22.0f, 23.0f, 24.0f,
        25.0f, 26.0f, 27.0f, 28.0f,
        29.0f, 30.0f, 31.0f, 32.0f
    };

    Matrix4f m1(data1);
    Matrix4f m2(data2);

    // Test alignment of operation results
    Matrix4f mult_result = m1 * m2;
    Matrix4f trans_result = transpose(m1);
    Matrix4f rot_result = rotate(Vector3f(1.0f, 0.0f, 0.0f), M_PI / 2.0f);

    // Verify alignment
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(mult_result.data()) % 16, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(trans_result.data()) % 16, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(rot_result.data()) % 16, 0);
}

} // namespace