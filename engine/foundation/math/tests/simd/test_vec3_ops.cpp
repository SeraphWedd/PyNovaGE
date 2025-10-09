#include <gtest/gtest.h>
#include "simd/vector_ops.hpp"
#include <cmath>

using namespace pynovage::foundation::simd;

class Vec3OpsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test vectors
        v1 = float4(1.0f, 2.0f, 3.0f, 0.0f);
        v2 = float4(4.0f, 5.0f, 6.0f, 0.0f);
        zeros = float4(0.0f, 0.0f, 0.0f, 0.0f);
        ones = float4(1.0f, 1.0f, 1.0f, 0.0f);

        // Initialize batch test data
        for (int i = 0; i < 12; ++i) {
            batch_data1[i] = static_cast<float>(i + 1);
            batch_data2[i] = static_cast<float>(i + 2);
        }
        // Set w components to 0
        batch_data1[3] = batch_data1[7] = batch_data1[11] = 0.0f;
        batch_data2[3] = batch_data2[7] = batch_data2[11] = 0.0f;
    }

    float4 v1, v2, zeros, ones;
    float batch_data1[12], batch_data2[12];

    void ExpectVec3Equal(const float4& result, float x, float y, float z) {
        EXPECT_NEAR(result[0], x, 1e-6f);
        EXPECT_NEAR(result[1], y, 1e-6f);
        EXPECT_NEAR(result[2], z, 1e-6f);
        EXPECT_NEAR(result[3], 0.0f, 1e-6f);
    }
};

TEST_F(Vec3OpsTest, Addition) {
    float4 result;
    Vec3Ops::add(v1, v2, result);
    ExpectVec3Equal(result, 5.0f, 7.0f, 9.0f);
}

TEST_F(Vec3OpsTest, Subtraction) {
    float4 result;
    Vec3Ops::subtract(v1, v2, result);
    ExpectVec3Equal(result, -3.0f, -3.0f, -3.0f);
}

TEST_F(Vec3OpsTest, Multiplication) {
    float4 result;
    Vec3Ops::multiply(v1, v2, result);
    ExpectVec3Equal(result, 4.0f, 10.0f, 18.0f);
}

TEST_F(Vec3OpsTest, Division) {
    float4 result;
    Vec3Ops::divide(v2, v1, result);
    ExpectVec3Equal(result, 4.0f, 2.5f, 2.0f);
}

TEST_F(Vec3OpsTest, ScalarMultiplication) {
    float4 result;
    Vec3Ops::multiply_scalar(v1, 2.0f, result);
    ExpectVec3Equal(result, 2.0f, 4.0f, 6.0f);
}

TEST_F(Vec3OpsTest, ScalarDivision) {
    float4 result;
    Vec3Ops::divide_scalar(v1, 2.0f, result);
    ExpectVec3Equal(result, 0.5f, 1.0f, 1.5f);
}

TEST_F(Vec3OpsTest, DotProduct) {
    float dot = Vec3Ops::dot(v1, v2);
    EXPECT_NEAR(dot, 32.0f, 1e-6f);  // 1*4 + 2*5 + 3*6 = 32
}

TEST_F(Vec3OpsTest, CrossProduct) {
    float4 result;
    Vec3Ops::cross(v1, v2, result);
    // Cross product = (2*6 - 3*5, 3*4 - 1*6, 1*5 - 2*4)
    ExpectVec3Equal(result, -3.0f, 6.0f, -3.0f);
}

TEST_F(Vec3OpsTest, Length) {
    float len = Vec3Ops::length(v1);
    EXPECT_NEAR(len, std::sqrt(14.0f), 1e-6f);  // sqrt(1^2 + 2^2 + 3^2)
}

TEST_F(Vec3OpsTest, LengthSquared) {
    float len_sq = Vec3Ops::length_squared(v1);
    EXPECT_NEAR(len_sq, 14.0f, 1e-6f);  // 1^2 + 2^2 + 3^2
}

TEST_F(Vec3OpsTest, Normalize) {
    float4 result;
    Vec3Ops::normalize(v1, result);
    float inv_len = 1.0f / std::sqrt(14.0f);
    ExpectVec3Equal(result, 1.0f * inv_len, 2.0f * inv_len, 3.0f * inv_len);
}

TEST_F(Vec3OpsTest, NormalizeZeroVector) {
    float4 result;
    Vec3Ops::normalize(zeros, result);
    ExpectVec3Equal(result, 0.0f, 0.0f, 0.0f);
}

TEST_F(Vec3OpsTest, BatchOperations) {
    float16 a = float16::load(batch_data1);
    float16 b = float16::load(batch_data2);
    float16 result;
    float output[16];

    // Test batch add
    Vec3Ops::add_batch4(a, b, result);
    result.store(output);
    for (int i = 0; i < 4; ++i) {
        int base = i * 4;
        EXPECT_NEAR(output[base], batch_data1[base] + batch_data2[base], 1e-6f);
        EXPECT_NEAR(output[base + 1], batch_data1[base + 1] + batch_data2[base + 1], 1e-6f);
        EXPECT_NEAR(output[base + 2], batch_data1[base + 2] + batch_data2[base + 2], 1e-6f);
        EXPECT_NEAR(output[base + 3], 0.0f, 1e-6f);  // w component should be 0
    }

    // Test batch multiply
    Vec3Ops::multiply_batch4(a, b, result);
    result.store(output);
    for (int i = 0; i < 4; ++i) {
        int base = i * 4;
        EXPECT_NEAR(output[base], batch_data1[base] * batch_data2[base], 1e-6f);
        EXPECT_NEAR(output[base + 1], batch_data1[base + 1] * batch_data2[base + 1], 1e-6f);
        EXPECT_NEAR(output[base + 2], batch_data1[base + 2] * batch_data2[base + 2], 1e-6f);
        EXPECT_NEAR(output[base + 3], 0.0f, 1e-6f);  // w component should be 0
    }
    
    // Test batch cross product
    float cross_data1[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,  // x axis
        0.0f, 1.0f, 0.0f, 0.0f,  // y axis
        0.0f, 0.0f, 1.0f, 0.0f,  // z axis
        1.0f, 1.0f, 1.0f, 0.0f   // diagonal
    };
    float cross_data2[16] = {
        0.0f, 1.0f, 0.0f, 0.0f,  // y axis
        0.0f, 0.0f, 1.0f, 0.0f,  // z axis
        1.0f, 0.0f, 0.0f, 0.0f,  // x axis
        -1.0f, 1.0f, -1.0f, 0.0f // test vector
    };
    float16 c1 = float16::load(cross_data1);
    float16 c2 = float16::load(cross_data2);
    
    Vec3Ops::cross_batch4(c1, c2, result);
    result.store(output);
    
    // First vector should be (0,0,1)
    EXPECT_NEAR(output[0], 0.0f, 1e-6f);
    EXPECT_NEAR(output[1], 0.0f, 1e-6f);
    EXPECT_NEAR(output[2], 1.0f, 1e-6f);
    EXPECT_NEAR(output[3], 0.0f, 1e-6f);
    
    // Second vector should be (1,0,0)
    EXPECT_NEAR(output[4], 1.0f, 1e-6f);
    EXPECT_NEAR(output[5], 0.0f, 1e-6f);
    EXPECT_NEAR(output[6], 0.0f, 1e-6f);
    EXPECT_NEAR(output[7], 0.0f, 1e-6f);
    
    // Third vector should be (0,1,0)
    EXPECT_NEAR(output[8], 0.0f, 1e-6f);
    EXPECT_NEAR(output[9], 1.0f, 1e-6f);
    EXPECT_NEAR(output[10], 0.0f, 1e-6f);
    EXPECT_NEAR(output[11], 0.0f, 1e-6f);
}