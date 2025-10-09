#include <gtest/gtest.h>
#include "simd/vector_ops.hpp"
#include <cmath>

using namespace pynovage::foundation::simd;

class Vec2OpsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize some test vectors
        v1 = float4(1.0f, 2.0f, 0.0f, 0.0f);
        v2 = float4(3.0f, 4.0f, 0.0f, 0.0f);
        zeros = float4(0.0f, 0.0f, 0.0f, 0.0f);
        ones = float4(1.0f, 1.0f, 0.0f, 0.0f);
    }

    float4 v1, v2, zeros, ones;
    
    void ExpectVec2Equal(const float4& result, float x, float y) {
        EXPECT_NEAR(result[0], x, 1e-6f);
        EXPECT_NEAR(result[1], y, 1e-6f);
        EXPECT_NEAR(result[2], 0.0f, 1e-6f);
        EXPECT_NEAR(result[3], 0.0f, 1e-6f);
    }
};

TEST_F(Vec2OpsTest, Addition) {
    float4 result;
    Vec2Ops::add(v1, v2, result);
    ExpectVec2Equal(result, 4.0f, 6.0f);
}

TEST_F(Vec2OpsTest, Subtraction) {
    float4 result;
    Vec2Ops::subtract(v1, v2, result);
    ExpectVec2Equal(result, -2.0f, -2.0f);
}

TEST_F(Vec2OpsTest, Multiplication) {
    float4 result;
    Vec2Ops::multiply(v1, v2, result);
    ExpectVec2Equal(result, 3.0f, 8.0f);
}

TEST_F(Vec2OpsTest, Division) {
    float4 result;
    Vec2Ops::divide(v2, v1, result);
    ExpectVec2Equal(result, 3.0f, 2.0f);
}

TEST_F(Vec2OpsTest, ScalarMultiplication) {
    float4 result;
    Vec2Ops::multiply_scalar(v1, 2.0f, result);
    ExpectVec2Equal(result, 2.0f, 4.0f);
}

TEST_F(Vec2OpsTest, ScalarDivision) {
    float4 result;
    Vec2Ops::divide_scalar(v1, 2.0f, result);
    ExpectVec2Equal(result, 0.5f, 1.0f);
}

TEST_F(Vec2OpsTest, DotProduct) {
    float dot = Vec2Ops::dot(v1, v2);
    EXPECT_NEAR(dot, 11.0f, 1e-6f);  // 1*3 + 2*4 = 11
}

TEST_F(Vec2OpsTest, Length) {
    float len = Vec2Ops::length(v1);
    EXPECT_NEAR(len, std::sqrt(5.0f), 1e-6f);  // sqrt(1^2 + 2^2)
}

TEST_F(Vec2OpsTest, LengthSquared) {
    float len_sq = Vec2Ops::length_squared(v1);
    EXPECT_NEAR(len_sq, 5.0f, 1e-6f);  // 1^2 + 2^2
}

TEST_F(Vec2OpsTest, Normalize) {
    float4 result;
    Vec2Ops::normalize(v1, result);
    float inv_len = 1.0f / std::sqrt(5.0f);
    ExpectVec2Equal(result, 1.0f * inv_len, 2.0f * inv_len);
}

TEST_F(Vec2OpsTest, NormalizeZeroVector) {
    float4 result;
    Vec2Ops::normalize(zeros, result);
    ExpectVec2Equal(result, 0.0f, 0.0f);
}

TEST_F(Vec2OpsTest, BatchOperations) {
    // Test processing two 2D vectors at once
    float data1[4] = {1.0f, 2.0f, 3.0f, 4.0f};  // Two 2D vectors
    float data2[4] = {5.0f, 6.0f, 7.0f, 8.0f};  // Two 2D vectors
    float result[4];
    
    float4 batch1 = float4::load(data1);
    float4 batch2 = float4::load(data2);
    float4 batch_result;
    
    // Test batch add
    Vec2Ops::add_batch2(batch1, batch2, batch_result);
    batch_result.store(result);
    EXPECT_NEAR(result[0], 6.0f, 1e-6f);   // 1 + 5
    EXPECT_NEAR(result[1], 8.0f, 1e-6f);   // 2 + 6
    EXPECT_NEAR(result[2], 10.0f, 1e-6f);  // 3 + 7
    EXPECT_NEAR(result[3], 12.0f, 1e-6f);  // 4 + 8
    
    // Test batch multiply
    Vec2Ops::multiply_batch2(batch1, batch2, batch_result);
    batch_result.store(result);
    EXPECT_NEAR(result[0], 5.0f, 1e-6f);   // 1 * 5
    EXPECT_NEAR(result[1], 12.0f, 1e-6f);  // 2 * 6
    EXPECT_NEAR(result[2], 21.0f, 1e-6f);  // 3 * 7
    EXPECT_NEAR(result[3], 32.0f, 1e-6f);  // 4 * 8
}