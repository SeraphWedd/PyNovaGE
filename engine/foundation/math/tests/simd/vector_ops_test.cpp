#include <gtest/gtest.h>
#include "../../include/simd/vector_ops.hpp"
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

TEST(VectorOpsTest, Addition) {
    // Test Vector4f addition
    Vector4f a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f b(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4f expected(6.0f, 8.0f, 10.0f, 12.0f);
    Vector4f result = a + b;
    
    EXPECT_TRUE(ApproxEqual(result, expected));

    // Test Vector3f addition
    Vector3f a3(1.0f, 2.0f, 3.0f);
    Vector3f b3(4.0f, 5.0f, 6.0f);
    Vector3f expected3(5.0f, 7.0f, 9.0f);
    Vector3f result3 = a3 + b3;
    
    EXPECT_TRUE(ApproxEqual(result3, expected3));
}

TEST(VectorOpsTest, Subtraction) {
    // Test Vector4f subtraction
    Vector4f a(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4f b(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f expected(4.0f, 4.0f, 4.0f, 4.0f);
    Vector4f result = a - b;
    
    EXPECT_TRUE(ApproxEqual(result, expected));

    // Test Vector3f subtraction
    Vector3f a3(6.0f, 5.0f, 4.0f);
    Vector3f b3(1.0f, 2.0f, 3.0f);
    Vector3f expected3(5.0f, 3.0f, 1.0f);
    Vector3f result3 = a3 - b3;
    
    EXPECT_TRUE(ApproxEqual(result3, expected3));
}

TEST(VectorOpsTest, DotProduct) {
    // Test Vector4f dot product
    Vector4f a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f b(5.0f, 6.0f, 7.0f, 8.0f);
    float expected = 70.0f; // 1*5 + 2*6 + 3*7 + 4*8
    float result = dot(a, b);
    
    EXPECT_FLOAT_EQ(result, expected);

    // Test Vector3f dot product
    Vector3f a3(1.0f, 2.0f, 3.0f);
    Vector3f b3(4.0f, 5.0f, 6.0f);
    float expected3 = 32.0f; // 1*4 + 2*5 + 3*6
    float result3 = dot(a3, b3);
    
    EXPECT_FLOAT_EQ(result3, expected3);
}

TEST(VectorOpsTest, CrossProduct) {
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(4.0f, 5.0f, 6.0f);
    Vector3f expected(-3.0f, 6.0f, -3.0f);
    Vector3f result = cross(a, b);
    
    EXPECT_TRUE(ApproxEqual(result, expected));

    // Test anticommutativity
    Vector3f negative_result = cross(b, a);
    EXPECT_TRUE(ApproxEqual(negative_result, expected * -1.0f));
}

TEST(VectorOpsTest, Normalization) {
    // Test Vector4f normalization
    Vector4f v4(1.0f, 2.0f, 2.0f, 0.0f);
    Vector4f result4 = normalize(v4);
    float length4 = std::sqrt(dot(result4, result4));
    
    EXPECT_FLOAT_EQ(length4, 1.0f);
    EXPECT_TRUE(ApproxEqual(result4 * std::sqrt(9.0f), v4));

    // Test Vector3f normalization
    Vector3f v3(3.0f, 0.0f, 4.0f);
    Vector3f result3 = normalize(v3);
    float length3 = std::sqrt(dot(result3, result3));
    
    EXPECT_FLOAT_EQ(length3, 1.0f);
    EXPECT_TRUE(ApproxEqual(result3 * 5.0f, v3));
}

TEST(VectorOpsTest, Length) {
    // Test Vector4f length
    Vector4f v4(1.0f, 2.0f, 2.0f, 0.0f);
    float expected_length4 = 3.0f;
    float result_length4 = length(v4);
    
    EXPECT_FLOAT_EQ(result_length4, expected_length4);

    // Test Vector3f length
    Vector3f v3(3.0f, 0.0f, 4.0f);
    float expected_length3 = 5.0f;
    float result_length3 = length(v3);
    
    EXPECT_FLOAT_EQ(result_length3, expected_length3);
}

TEST(VectorOpsTest, LengthSquared) {
    // Test Vector4f length squared
    Vector4f v4(1.0f, 2.0f, 2.0f, 0.0f);
    float expected_length_sq4 = 9.0f;
    float result_length_sq4 = length_squared(v4);
    
    EXPECT_FLOAT_EQ(result_length_sq4, expected_length_sq4);

    // Test Vector3f length squared
    Vector3f v3(3.0f, 0.0f, 4.0f);
    float expected_length_sq3 = 25.0f;
    float result_length_sq3 = length_squared(v3);
    
    EXPECT_FLOAT_EQ(result_length_sq3, expected_length_sq3);
}

TEST(VectorOpsTest, Lerp) {
    // Test Vector4f lerp
    Vector4f start4(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f end4(2.0f, 4.0f, 6.0f, 8.0f);
    Vector4f expected4(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f result4 = lerp(start4, end4, 0.5f);
    
    EXPECT_TRUE(ApproxEqual(result4, expected4));

    // Test Vector3f lerp
    Vector3f start3(0.0f, 0.0f, 0.0f);
    Vector3f end3(3.0f, 6.0f, 9.0f);
    Vector3f expected3(2.0f, 4.0f, 6.0f);
    Vector3f result3 = lerp(start3, end3, 2.0f/3.0f);
    
    EXPECT_TRUE(ApproxEqual(result3, expected3));
}

TEST(VectorOpsTest, EdgeCases) {
    // Test normalization of zero vector
    Vector4f zero4(0.0f);
    Vector4f norm_zero4 = normalize(zero4);
    EXPECT_TRUE(ApproxEqual(norm_zero4, zero4));

    // Test cross product with parallel vectors
    Vector3f v3(1.0f, 2.0f, 3.0f);
    Vector3f parallel = v3 * 2.0f;
    Vector3f cross_result = cross(v3, parallel);
    Vector3f zero3(0.0f);
    EXPECT_TRUE(ApproxEqual(cross_result, zero3));

    // Test lerp with t = 0 and t = 1
    Vector4f a4(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f b4(5.0f, 6.0f, 7.0f, 8.0f);
    EXPECT_TRUE(ApproxEqual(lerp(a4, b4, 0.0f), a4));
    EXPECT_TRUE(ApproxEqual(lerp(a4, b4, 1.0f), b4));
}

TEST(VectorOpsTest, SIMDAlignment) {
    // Create aligned vectors
    alignas(16) float data1[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    alignas(16) float data2[4] = {5.0f, 6.0f, 7.0f, 8.0f};

    Vector4f v1(data1[0], data1[1], data1[2], data1[3]);
    Vector4f v2(data2[0], data2[1], data2[2], data2[3]);

    // Perform operations and check results
    Vector4f add_result = v1 + v2;
    Vector4f sub_result = v1 - v2;
    Vector4f norm_result = normalize(v1);

    // Verify results maintain proper alignment
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(add_result.data()) % 16, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(sub_result.data()) % 16, 0);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(norm_result.data()) % 16, 0);
}

TEST(VectorOpsTest, Consistency) {
    Vector4f v(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Test length consistency
    float len = length(v);
    float len_sq = length_squared(v);
    EXPECT_FLOAT_EQ(len * len, len_sq);

    // Test normalization consistency
    Vector4f norm = normalize(v);
    EXPECT_FLOAT_EQ(length(norm), 1.0f);
    EXPECT_TRUE(ApproxEqual(norm * len, v));

    // Test dot product consistency
    float dot_self = dot(v, v);
    EXPECT_FLOAT_EQ(dot_self, len_sq);
}

} // namespace