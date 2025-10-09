#include <gtest/gtest.h>
#include "../../include/simd/math_ops.hpp"
#include <cmath>

namespace {

using namespace PyNovaGE::SIMD;

// Helper function to compare vectors with tolerance
template<typename T, size_t N>
bool ApproxEqual(const Vector<T, N>& a, const Vector<T, N>& b, T tolerance = T(1e-5)) {
    for (size_t i = 0; i < N; ++i) {
        if (std::abs(a[i] - b[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

TEST(MathOpsTest, Sqrt) {
    Vector4f v(4.0f, 9.0f, 16.0f, 25.0f);
    Vector4f expected(2.0f, 3.0f, 4.0f, 5.0f);
    Vector4f result = sqrt(v);

    EXPECT_TRUE(ApproxEqual(result, expected));

    // Test with Vector3f
    Vector3f v3(4.0f, 9.0f, 16.0f);
    Vector3f expected3(2.0f, 3.0f, 4.0f);
    Vector3f result3 = sqrt(v3);

    EXPECT_TRUE(ApproxEqual(result3, expected3));
}

TEST(MathOpsTest, Rsqrt) {
    Vector4f v(4.0f, 9.0f, 16.0f, 25.0f);
    Vector4f expected(0.5f, 1.0f/3.0f, 0.25f, 0.2f);
    Vector4f result = rsqrt(v);

    EXPECT_TRUE(ApproxEqual(result, expected, 1e-4f));
}

TEST(MathOpsTest, Abs) {
    Vector4f v(-1.0f, 2.0f, -3.0f, 4.0f);
    Vector4f expected(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f result = abs(v);

    EXPECT_TRUE(ApproxEqual(result, expected));

    // Test with integer vectors
    Vector4i vi(-1, 2, -3, 4);
    Vector4i expectedi(1, 2, 3, 4);
    Vector4i resulti = abs(vi);

    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(resulti[i], expectedi[i]);
    }
}

TEST(MathOpsTest, Min) {
    Vector4f a(1.0f, 4.0f, 2.0f, 8.0f);
    Vector4f b(2.0f, 3.0f, 1.0f, 9.0f);
    Vector4f expected(1.0f, 3.0f, 1.0f, 8.0f);
    Vector4f result = min(a, b);

    EXPECT_TRUE(ApproxEqual(result, expected));

    // Test with integer vectors
    Vector4i ai(1, 4, 2, 8);
    Vector4i bi(2, 3, 1, 9);
    Vector4i expectedi(1, 3, 1, 8);
    Vector4i resulti = min(ai, bi);

    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(resulti[i], expectedi[i]);
    }
}

TEST(MathOpsTest, Max) {
    Vector4f a(1.0f, 4.0f, 2.0f, 8.0f);
    Vector4f b(2.0f, 3.0f, 1.0f, 9.0f);
    Vector4f expected(2.0f, 4.0f, 2.0f, 9.0f);
    Vector4f result = max(a, b);

    EXPECT_TRUE(ApproxEqual(result, expected));

    // Test with integer vectors
    Vector4i ai(1, 4, 2, 8);
    Vector4i bi(2, 3, 1, 9);
    Vector4i expectedi(2, 4, 2, 9);
    Vector4i resulti = max(ai, bi);

    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(resulti[i], expectedi[i]);
    }
}

TEST(MathOpsTest, EdgeCases) {
    // Test sqrt with zero
    Vector4f zero(0.0f);
    Vector4f sqrt_zero = sqrt(zero);
    EXPECT_TRUE(ApproxEqual(sqrt_zero, zero));

    // Test rsqrt with very small numbers
    Vector4f small(1e-20f);
    Vector4f rsqrt_small = rsqrt(small);
    for (int i = 0; i < 4; ++i) {
        EXPECT_GT(rsqrt_small[i], 0.0f);
    }

    // Test abs with zero
    Vector4f abs_zero = abs(zero);
    EXPECT_TRUE(ApproxEqual(abs_zero, zero));

    // Test min/max with identical values
    Vector4f v(1.0f);
    EXPECT_TRUE(ApproxEqual(min(v, v), v));
    EXPECT_TRUE(ApproxEqual(max(v, v), v));
}

TEST(MathOpsTest, SIMDAlignment) {
    // Create aligned vectors
    alignas(16) float data1[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    alignas(16) float data2[4] = {5.0f, 6.0f, 7.0f, 8.0f};

    Vector4f v1(data1[0], data1[1], data1[2], data1[3]);
    Vector4f v2(data2[0], data2[1], data2[2], data2[3]);

    // Perform operations and check results
    Vector4f sqrt_result = sqrt(v1);
    Vector4f rsqrt_result = rsqrt(v1);
    Vector4f min_result = min(v1, v2);
    Vector4f max_result = max(v1, v2);

    // Verify results maintain proper alignment
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(sqrt_result.data()) % 16, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(rsqrt_result.data()) % 16, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(min_result.data()) % 16, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(max_result.data()) % 16, 0);
}

} // namespace