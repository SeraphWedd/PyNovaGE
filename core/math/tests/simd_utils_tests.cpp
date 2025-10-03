#include <gtest/gtest.h>
#include "math/simd_utils.hpp"
#include <cmath>

using namespace pynovage::math;

class SimdUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test vectors
        a[0] = 1.0f; a[1] = 2.0f;
        b[0] = 3.0f; b[1] = 4.0f;
        result[0] = 0.0f; result[1] = 0.0f;
    }

    float a[2];
    float b[2];
    float result[2];

    const float epsilon = 1e-6f;
};

TEST_F(SimdUtilsTest, Add2f) {
    SimdUtils::Add2f(a, b, result);
    EXPECT_NEAR(result[0], 4.0f, epsilon);  // 1 + 3
    EXPECT_NEAR(result[1], 6.0f, epsilon);  // 2 + 4
}

TEST_F(SimdUtilsTest, Subtract2f) {
    SimdUtils::Subtract2f(a, b, result);
    EXPECT_NEAR(result[0], -2.0f, epsilon);  // 1 - 3
    EXPECT_NEAR(result[1], -2.0f, epsilon);  // 2 - 4
}

TEST_F(SimdUtilsTest, Multiply2f) {
    SimdUtils::Multiply2f(a, b, result);
    EXPECT_NEAR(result[0], 3.0f, epsilon);  // 1 * 3
    EXPECT_NEAR(result[1], 8.0f, epsilon);  // 2 * 4
}

TEST_F(SimdUtilsTest, Divide2f) {
    SimdUtils::Divide2f(a, b, result);
    EXPECT_NEAR(result[0], 1.0f/3.0f, epsilon);  // 1 / 3
    EXPECT_NEAR(result[1], 2.0f/4.0f, epsilon);  // 2 / 4
}

TEST_F(SimdUtilsTest, DotProduct2f) {
    float dot = SimdUtils::DotProduct2f(a, b);
    EXPECT_NEAR(dot, 11.0f, epsilon);  // 1*3 + 2*4
}

TEST_F(SimdUtilsTest, Add2f_Zero) {
    float zero[2] = {0.0f, 0.0f};
    SimdUtils::Add2f(a, zero, result);
    EXPECT_NEAR(result[0], a[0], epsilon);
    EXPECT_NEAR(result[1], a[1], epsilon);
}

TEST_F(SimdUtilsTest, Multiply2f_Identity) {
    float one[2] = {1.0f, 1.0f};
    SimdUtils::Multiply2f(a, one, result);
    EXPECT_NEAR(result[0], a[0], epsilon);
    EXPECT_NEAR(result[1], a[1], epsilon);
}

TEST_F(SimdUtilsTest, DotProduct2f_Orthogonal) {
    float orthogonal[2] = {-2.0f, 1.0f};  // Orthogonal to (1,2)
    float dot = SimdUtils::DotProduct2f(a, orthogonal);
    EXPECT_NEAR(dot, 0.0f, epsilon);
}

TEST_F(SimdUtilsTest, SIMD_Support) {
    // These tests just verify that the detection functions run without crashing
    // The actual return values will depend on the platform
    EXPECT_NO_THROW(SimdUtils::HasSSE());
    EXPECT_NO_THROW(SimdUtils::HasSSE2());
    EXPECT_NO_THROW(SimdUtils::HasAVX());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}