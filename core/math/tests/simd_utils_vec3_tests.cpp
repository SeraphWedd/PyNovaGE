#include <gtest/gtest.h>
#include "simd_utils.hpp"
#include <cmath>

using namespace pynovage::math;

class SimdUtilsVec3Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test vectors
        a[0] = 1.0f; a[1] = 2.0f; a[2] = 3.0f;
        b[0] = 4.0f; b[1] = 5.0f; b[2] = 6.0f;
        result[0] = 0.0f; result[1] = 0.0f; result[2] = 0.0f;
    }

    float a[4];      // Extra element for SIMD alignment
    float b[4];      // Extra element for SIMD alignment
    float result[4]; // Extra element for SIMD alignment
    const float epsilon = 1e-6f;
};

TEST_F(SimdUtilsVec3Test, Add3f) {
    SimdUtils::Add3f(a, b, result);
    EXPECT_NEAR(result[0], 5.0f, epsilon);  // 1 + 4
    EXPECT_NEAR(result[1], 7.0f, epsilon);  // 2 + 5
    EXPECT_NEAR(result[2], 9.0f, epsilon);  // 3 + 6
}

TEST_F(SimdUtilsVec3Test, Subtract3f) {
    SimdUtils::Subtract3f(a, b, result);
    EXPECT_NEAR(result[0], -3.0f, epsilon);  // 1 - 4
    EXPECT_NEAR(result[1], -3.0f, epsilon);  // 2 - 5
    EXPECT_NEAR(result[2], -3.0f, epsilon);  // 3 - 6
}

TEST_F(SimdUtilsVec3Test, Multiply3f) {
    SimdUtils::Multiply3f(a, b, result);
    EXPECT_NEAR(result[0], 4.0f, epsilon);   // 1 * 4
    EXPECT_NEAR(result[1], 10.0f, epsilon);  // 2 * 5
    EXPECT_NEAR(result[2], 18.0f, epsilon);  // 3 * 6
}

TEST_F(SimdUtilsVec3Test, Divide3f) {
    SimdUtils::Divide3f(a, b, result);
    EXPECT_NEAR(result[0], 0.25f, epsilon);    // 1 / 4
    EXPECT_NEAR(result[1], 0.4f, epsilon);     // 2 / 5
    EXPECT_NEAR(result[2], 0.5f, epsilon);     // 3 / 6
}

TEST_F(SimdUtilsVec3Test, DotProduct3f) {
    float dot = SimdUtils::DotProduct3f(a, b);
    EXPECT_NEAR(dot, 32.0f, epsilon);  // 1*4 + 2*5 + 3*6
}

TEST_F(SimdUtilsVec3Test, CrossProduct3f) {
    SimdUtils::CrossProduct3f(a, b, result);
    // Cross product: (a1*b2 - a2*b1, a2*b0 - a0*b2, a0*b1 - a1*b0)
    EXPECT_NEAR(result[0], -3.0f, epsilon);   // 2*6 - 3*5 = 12 - 15 = -3
    EXPECT_NEAR(result[1], 6.0f, epsilon);    // 3*4 - 1*6 = 12 - 6 = 6
    EXPECT_NEAR(result[2], -3.0f, epsilon);   // 1*5 - 2*4 = 5 - 8 = -3
}

TEST_F(SimdUtilsVec3Test, Add3f_Zero) {
    float zero[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    SimdUtils::Add3f(a, zero, result);
    EXPECT_NEAR(result[0], a[0], epsilon);
    EXPECT_NEAR(result[1], a[1], epsilon);
    EXPECT_NEAR(result[2], a[2], epsilon);
}

TEST_F(SimdUtilsVec3Test, Multiply3f_Identity) {
    float one[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    SimdUtils::Multiply3f(a, one, result);
    EXPECT_NEAR(result[0], a[0], epsilon);
    EXPECT_NEAR(result[1], a[1], epsilon);
    EXPECT_NEAR(result[2], a[2], epsilon);
}

TEST_F(SimdUtilsVec3Test, CrossProduct3f_Orthogonal) {
    // Test with two standard basis vectors (should give the third basis vector)
    float x_axis[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    float y_axis[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    SimdUtils::CrossProduct3f(x_axis, y_axis, result);
    EXPECT_NEAR(result[0], 0.0f, epsilon);
    EXPECT_NEAR(result[1], 0.0f, epsilon);
    EXPECT_NEAR(result[2], 1.0f, epsilon);
}

TEST_F(SimdUtilsVec3Test, CrossProduct3f_Parallel) {
    // Cross product of parallel vectors should be zero
    float v[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    SimdUtils::CrossProduct3f(v, v, result);
    EXPECT_NEAR(result[0], 0.0f, epsilon);
    EXPECT_NEAR(result[1], 0.0f, epsilon);
    EXPECT_NEAR(result[2], 0.0f, epsilon);
}

TEST_F(SimdUtilsVec3Test, DotProduct3f_Orthogonal) {
    float v1[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    float v2[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    float dot = SimdUtils::DotProduct3f(v1, v2);
    EXPECT_NEAR(dot, 0.0f, epsilon);
}

TEST_F(SimdUtilsVec3Test, DotProduct3f_Parallel) {
    float v[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    float dot = SimdUtils::DotProduct3f(v, v);
    EXPECT_NEAR(dot, 1.0f, epsilon);
}