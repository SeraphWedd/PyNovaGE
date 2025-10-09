#include <gtest/gtest.h>
#include "simd/vector_ops.hpp"
#include <cmath>

using namespace pynovage::foundation::simd;

class Vec4OpsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test vectors
        v1 = float4(1.0f, 2.0f, 3.0f, 4.0f);
        v2 = float4(5.0f, 6.0f, 7.0f, 8.0f);
        zeros = float4(0.0f, 0.0f, 0.0f, 0.0f);
        ones = float4(1.0f, 1.0f, 1.0f, 1.0f);

        // Initialize batch test data
        for (int i = 0; i < 16; ++i) {
            batch_data1[i] = static_cast<float>(i + 1);
            batch_data2[i] = static_cast<float>(i + 2);
        }
    }

    float4 v1, v2, zeros, ones;
    float batch_data1[16], batch_data2[16];

    void ExpectVec4Equal(const float4& result, float x, float y, float z, float w) {
        EXPECT_NEAR(result[0], x, 1e-6f);
        EXPECT_NEAR(result[1], y, 1e-6f);
        EXPECT_NEAR(result[2], z, 1e-6f);
        EXPECT_NEAR(result[3], w, 1e-6f);
    }
};

TEST_F(Vec4OpsTest, Addition) {
    float4 result;
    Vec4Ops::add(v1, v2, result);
    ExpectVec4Equal(result, 6.0f, 8.0f, 10.0f, 12.0f);
}

TEST_F(Vec4OpsTest, Subtraction) {
    float4 result;
    Vec4Ops::subtract(v1, v2, result);
    ExpectVec4Equal(result, -4.0f, -4.0f, -4.0f, -4.0f);
}

TEST_F(Vec4OpsTest, Multiplication) {
    float4 result;
    Vec4Ops::multiply(v1, v2, result);
    ExpectVec4Equal(result, 5.0f, 12.0f, 21.0f, 32.0f);
}

TEST_F(Vec4OpsTest, Division) {
    float4 result;
    Vec4Ops::divide(v2, v1, result);
    ExpectVec4Equal(result, 5.0f, 3.0f, 7.0f/3.0f, 2.0f);
}

TEST_F(Vec4OpsTest, ScalarMultiplication) {
    float4 result;
    Vec4Ops::multiply_scalar(v1, 2.0f, result);
    ExpectVec4Equal(result, 2.0f, 4.0f, 6.0f, 8.0f);
}

TEST_F(Vec4OpsTest, ScalarDivision) {
    float4 result;
    Vec4Ops::divide_scalar(v1, 2.0f, result);
    ExpectVec4Equal(result, 0.5f, 1.0f, 1.5f, 2.0f);
}

TEST_F(Vec4OpsTest, DotProduct) {
    float dot = Vec4Ops::dot(v1, v2);
    EXPECT_NEAR(dot, 70.0f, 1e-6f);  // 1*5 + 2*6 + 3*7 + 4*8 = 70
}

TEST_F(Vec4OpsTest, Length) {
    float len = Vec4Ops::length(v1);
    EXPECT_NEAR(len, std::sqrt(30.0f), 1e-6f);  // sqrt(1^2 + 2^2 + 3^2 + 4^2)
}

TEST_F(Vec4OpsTest, LengthSquared) {
    float len_sq = Vec4Ops::length_squared(v1);
    EXPECT_NEAR(len_sq, 30.0f, 1e-6f);  // 1^2 + 2^2 + 3^2 + 4^2
}

TEST_F(Vec4OpsTest, Normalize) {
    float4 result;
    Vec4Ops::normalize(v1, result);
    float inv_len = 1.0f / std::sqrt(30.0f);
    ExpectVec4Equal(result, 1.0f * inv_len, 2.0f * inv_len, 3.0f * inv_len, 4.0f * inv_len);
}

TEST_F(Vec4OpsTest, NormalizeZeroVector) {
    float4 result;
    Vec4Ops::normalize(zeros, result);
    ExpectVec4Equal(result, 0.0f, 0.0f, 0.0f, 0.0f);
}

TEST_F(Vec4OpsTest, MultiplyAdd) {
    float4 a(2.0f, 3.0f, 4.0f, 5.0f);
    float4 b(3.0f, 4.0f, 5.0f, 6.0f);
    float4 c(1.0f, 1.0f, 1.0f, 1.0f);
    float4 result;
    
    Vec4Ops::multiply_add(a, b, c, result);
    ExpectVec4Equal(result, 7.0f, 13.0f, 21.0f, 31.0f);  // a*b + c
}

TEST_F(Vec4OpsTest, MultiplySub) {
    float4 a(2.0f, 3.0f, 4.0f, 5.0f);
    float4 b(3.0f, 4.0f, 5.0f, 6.0f);
    float4 c(1.0f, 1.0f, 1.0f, 1.0f);
    float4 result;
    
    Vec4Ops::multiply_sub(a, b, c, result);
    ExpectVec4Equal(result, 5.0f, 11.0f, 19.0f, 29.0f);  // a*b - c
}

TEST_F(Vec4OpsTest, BatchOperations) {
    float16 a = float16::load(batch_data1);
    float16 b = float16::load(batch_data2);
    float16 result;
    float output[16];

    // Test batch add
    Vec4Ops::add_batch4(a, b, result);
    result.store(output);
    for (int i = 0; i < 16; ++i) {
        EXPECT_NEAR(output[i], batch_data1[i] + batch_data2[i], 1e-6f);
    }

    // Test batch multiply
    Vec4Ops::multiply_batch4(a, b, result);
    result.store(output);
    for (int i = 0; i < 16; ++i) {
        EXPECT_NEAR(output[i], batch_data1[i] * batch_data2[i], 1e-6f);
    }

    // Test batch multiply-add
    float16 c = float16::broadcast(1.0f);
    Vec4Ops::multiply_add_batch4(a, b, c, result);
    result.store(output);
    for (int i = 0; i < 16; ++i) {
        EXPECT_NEAR(output[i], batch_data1[i] * batch_data2[i] + 1.0f, 1e-6f);
    }
}