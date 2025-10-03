#include <gtest/gtest.h>
#include "math/vector2.hpp"
#include <cmath>

using namespace pynovage::math;

class Vector2Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test vectors
        v1 = Vector2(1.0f, 2.0f);
        v2 = Vector2(3.0f, 4.0f);
    }

    Vector2 v1;
    Vector2 v2;
    const float epsilon = 1e-6f;
};

// Construction tests
TEST_F(Vector2Test, DefaultConstruction) {
    Vector2 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST_F(Vector2Test, ValueConstruction) {
    EXPECT_FLOAT_EQ(v1.x, 1.0f);
    EXPECT_FLOAT_EQ(v1.y, 2.0f);
}

TEST_F(Vector2Test, CopyConstruction) {
    Vector2 v(v1);
    EXPECT_FLOAT_EQ(v.x, v1.x);
    EXPECT_FLOAT_EQ(v.y, v1.y);
}

// Basic arithmetic tests
TEST_F(Vector2Test, Addition) {
    Vector2 sum = v1 + v2;
    EXPECT_FLOAT_EQ(sum.x, 4.0f);  // 1 + 3
    EXPECT_FLOAT_EQ(sum.y, 6.0f);  // 2 + 4
}

TEST_F(Vector2Test, Subtraction) {
    Vector2 diff = v1 - v2;
    EXPECT_FLOAT_EQ(diff.x, -2.0f);  // 1 - 3
    EXPECT_FLOAT_EQ(diff.y, -2.0f);  // 2 - 4
}

TEST_F(Vector2Test, ScalarMultiplication) {
    Vector2 scaled = v1 * 2.0f;
    EXPECT_FLOAT_EQ(scaled.x, 2.0f);  // 1 * 2
    EXPECT_FLOAT_EQ(scaled.y, 4.0f);  // 2 * 2

    // Test commutative property
    Vector2 scaled2 = 2.0f * v1;
    EXPECT_FLOAT_EQ(scaled.x, scaled2.x);
    EXPECT_FLOAT_EQ(scaled.y, scaled2.y);
}

TEST_F(Vector2Test, ScalarDivision) {
    Vector2 divided = v1 / 2.0f;
    EXPECT_FLOAT_EQ(divided.x, 0.5f);  // 1 / 2
    EXPECT_FLOAT_EQ(divided.y, 1.0f);  // 2 / 2
}

// Compound assignment tests
TEST_F(Vector2Test, CompoundAddition) {
    Vector2 v(v1);
    v += v2;
    EXPECT_FLOAT_EQ(v.x, 4.0f);  // 1 + 3
    EXPECT_FLOAT_EQ(v.y, 6.0f);  // 2 + 4
}

TEST_F(Vector2Test, CompoundSubtraction) {
    Vector2 v(v1);
    v -= v2;
    EXPECT_FLOAT_EQ(v.x, -2.0f);  // 1 - 3
    EXPECT_FLOAT_EQ(v.y, -2.0f);  // 2 - 4
}

TEST_F(Vector2Test, CompoundMultiplication) {
    Vector2 v(v1);
    v *= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 2.0f);  // 1 * 2
    EXPECT_FLOAT_EQ(v.y, 4.0f);  // 2 * 2
}

TEST_F(Vector2Test, CompoundDivision) {
    Vector2 v(v1);
    v /= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 0.5f);  // 1 / 2
    EXPECT_FLOAT_EQ(v.y, 1.0f);  // 2 / 2
}

// Geometric operation tests
TEST_F(Vector2Test, DotProduct) {
    float dot = v1.dot(v2);
    EXPECT_FLOAT_EQ(dot, 11.0f);  // 1*3 + 2*4
}

TEST_F(Vector2Test, Length) {
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.length(), 5.0f);  // sqrt(3^2 + 4^2) = 5
}

TEST_F(Vector2Test, LengthSquared) {
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.lengthSquared(), 25.0f);  // 3^2 + 4^2 = 25
}

TEST_F(Vector2Test, Normalization) {
    Vector2 v(3.0f, 4.0f);
    v.normalize();
    EXPECT_FLOAT_EQ(v.x, 0.6f);  // 3/5
    EXPECT_FLOAT_EQ(v.y, 0.8f);  // 4/5
    EXPECT_NEAR(v.length(), 1.0f, epsilon);
}

TEST_F(Vector2Test, Normalized) {
    Vector2 v(3.0f, 4.0f);
    Vector2 n = v.normalized();
    EXPECT_FLOAT_EQ(n.x, 0.6f);  // 3/5
    EXPECT_FLOAT_EQ(n.y, 0.8f);  // 4/5
    EXPECT_NEAR(n.length(), 1.0f, epsilon);
    // Original vector should be unchanged
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
}

// Utility function tests
TEST_F(Vector2Test, IsZero) {
    EXPECT_TRUE(Vector2().isZero());
    EXPECT_TRUE(Vector2(0.0f, 0.0f).isZero());
    EXPECT_FALSE(v1.isZero());
}

TEST_F(Vector2Test, SetZero) {
    Vector2 v(v1);
    v.setZero();
    EXPECT_TRUE(v.isZero());
}

TEST_F(Vector2Test, StaticCreation) {
    Vector2 zero = Vector2::zero();
    EXPECT_TRUE(zero.isZero());

    Vector2 one = Vector2::one();
    EXPECT_FLOAT_EQ(one.x, 1.0f);
    EXPECT_FLOAT_EQ(one.y, 1.0f);

    Vector2 unitX = Vector2::unitX();
    EXPECT_FLOAT_EQ(unitX.x, 1.0f);
    EXPECT_FLOAT_EQ(unitX.y, 0.0f);

    Vector2 unitY = Vector2::unitY();
    EXPECT_FLOAT_EQ(unitY.x, 0.0f);
    EXPECT_FLOAT_EQ(unitY.y, 1.0f);
}

// Edge case tests
TEST_F(Vector2Test, ZeroNormalization) {
    Vector2 v;  // zero vector
    v.normalize();
    // Should remain zero
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST_F(Vector2Test, LargeValues) {
    Vector2 large(1e6f, 2e6f);
    Vector2 normalized = large.normalized();
    EXPECT_NEAR(normalized.length(), 1.0f, epsilon);
}
