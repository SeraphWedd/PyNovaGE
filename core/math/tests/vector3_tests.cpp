#include <gtest/gtest.h>
#include "math/vector3.hpp"
#include <cmath>

using namespace pynovage::math;

class Vector3Test : public ::testing::Test {
protected:
    void SetUp() override {
        v1 = Vector3(1.0f, 2.0f, 3.0f);
        v2 = Vector3(4.0f, 5.0f, 6.0f);
    }

    Vector3 v1;
    Vector3 v2;
    const float epsilon = 1e-6f;
};

// Construction tests
TEST_F(Vector3Test, DefaultConstruction) {
    Vector3 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(Vector3Test, ValueConstruction) {
    EXPECT_FLOAT_EQ(v1.x, 1.0f);
    EXPECT_FLOAT_EQ(v1.y, 2.0f);
    EXPECT_FLOAT_EQ(v1.z, 3.0f);
}

TEST_F(Vector3Test, CopyConstruction) {
    Vector3 v(v1);
    EXPECT_FLOAT_EQ(v.x, v1.x);
    EXPECT_FLOAT_EQ(v.y, v1.y);
    EXPECT_FLOAT_EQ(v.z, v1.z);
}

// Basic arithmetic tests
TEST_F(Vector3Test, Addition) {
    Vector3 sum = v1 + v2;
    EXPECT_FLOAT_EQ(sum.x, 5.0f);  // 1 + 4
    EXPECT_FLOAT_EQ(sum.y, 7.0f);  // 2 + 5
    EXPECT_FLOAT_EQ(sum.z, 9.0f);  // 3 + 6
}

TEST_F(Vector3Test, Subtraction) {
    Vector3 diff = v1 - v2;
    EXPECT_FLOAT_EQ(diff.x, -3.0f);  // 1 - 4
    EXPECT_FLOAT_EQ(diff.y, -3.0f);  // 2 - 5
    EXPECT_FLOAT_EQ(diff.z, -3.0f);  // 3 - 6
}

TEST_F(Vector3Test, ScalarMultiplication) {
    Vector3 scaled = v1 * 2.0f;
    EXPECT_FLOAT_EQ(scaled.x, 2.0f);   // 1 * 2
    EXPECT_FLOAT_EQ(scaled.y, 4.0f);   // 2 * 2
    EXPECT_FLOAT_EQ(scaled.z, 6.0f);   // 3 * 2

    // Test commutative property
    Vector3 scaled2 = 2.0f * v1;
    EXPECT_FLOAT_EQ(scaled.x, scaled2.x);
    EXPECT_FLOAT_EQ(scaled.y, scaled2.y);
    EXPECT_FLOAT_EQ(scaled.z, scaled2.z);
}

TEST_F(Vector3Test, ScalarDivision) {
    Vector3 divided = v1 / 2.0f;
    EXPECT_FLOAT_EQ(divided.x, 0.5f);   // 1 / 2
    EXPECT_FLOAT_EQ(divided.y, 1.0f);   // 2 / 2
    EXPECT_FLOAT_EQ(divided.z, 1.5f);   // 3 / 2
}

// Compound assignment tests
TEST_F(Vector3Test, CompoundAddition) {
    Vector3 v(v1);
    v += v2;
    EXPECT_FLOAT_EQ(v.x, 5.0f);  // 1 + 4
    EXPECT_FLOAT_EQ(v.y, 7.0f);  // 2 + 5
    EXPECT_FLOAT_EQ(v.z, 9.0f);  // 3 + 6
}

TEST_F(Vector3Test, CompoundSubtraction) {
    Vector3 v(v1);
    v -= v2;
    EXPECT_FLOAT_EQ(v.x, -3.0f);  // 1 - 4
    EXPECT_FLOAT_EQ(v.y, -3.0f);  // 2 - 5
    EXPECT_FLOAT_EQ(v.z, -3.0f);  // 3 - 6
}

TEST_F(Vector3Test, CompoundMultiplication) {
    Vector3 v(v1);
    v *= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 2.0f);   // 1 * 2
    EXPECT_FLOAT_EQ(v.y, 4.0f);   // 2 * 2
    EXPECT_FLOAT_EQ(v.z, 6.0f);   // 3 * 2
}

TEST_F(Vector3Test, CompoundDivision) {
    Vector3 v(v1);
    v /= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 0.5f);   // 1 / 2
    EXPECT_FLOAT_EQ(v.y, 1.0f);   // 2 / 2
    EXPECT_FLOAT_EQ(v.z, 1.5f);   // 3 / 2
}

// Geometric operation tests
TEST_F(Vector3Test, DotProduct) {
    float dot = v1.dot(v2);
    EXPECT_FLOAT_EQ(dot, 32.0f);  // 1*4 + 2*5 + 3*6
}

TEST_F(Vector3Test, CrossProduct) {
    Vector3 cross = v1.cross(v2);
    // Cross product: (a1*b2 - a2*b1, a2*b0 - a0*b2, a0*b1 - a1*b0)
    EXPECT_FLOAT_EQ(cross.x, -3.0f);  // 2*6 - 3*5
    EXPECT_FLOAT_EQ(cross.y, 6.0f);   // 3*4 - 1*6
    EXPECT_FLOAT_EQ(cross.z, -3.0f);  // 1*5 - 2*4
}

TEST_F(Vector3Test, Length) {
    Vector3 v(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.length(), 5.0f);  // sqrt(3^2 + 4^2)
}

TEST_F(Vector3Test, LengthSquared) {
    Vector3 v(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.lengthSquared(), 25.0f);  // 3^2 + 4^2
}

TEST_F(Vector3Test, Normalization) {
    Vector3 v(3.0f, 0.0f, 4.0f);
    v.normalize();
    EXPECT_FLOAT_EQ(v.x, 0.6f);   // 3/5
    EXPECT_FLOAT_EQ(v.y, 0.0f);   // 0/5
    EXPECT_FLOAT_EQ(v.z, 0.8f);   // 4/5
    EXPECT_NEAR(v.length(), 1.0f, epsilon);
}

TEST_F(Vector3Test, Normalized) {
    Vector3 v(3.0f, 0.0f, 4.0f);
    Vector3 n = v.normalized();
    EXPECT_FLOAT_EQ(n.x, 0.6f);   // 3/5
    EXPECT_FLOAT_EQ(n.y, 0.0f);   // 0/5
    EXPECT_FLOAT_EQ(n.z, 0.8f);   // 4/5
    EXPECT_NEAR(n.length(), 1.0f, epsilon);
    // Original vector should be unchanged
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 4.0f);
}

// Advanced geometric operation tests
TEST_F(Vector3Test, Reflection) {
    Vector3 v(1.0f, -1.0f, 0.0f);
    Vector3 normal(0.0f, 1.0f, 0.0f);
    Vector3 reflected = v.reflect(normal);
    EXPECT_NEAR(reflected.x, 1.0f, epsilon);
    EXPECT_NEAR(reflected.y, 1.0f, epsilon);
    EXPECT_NEAR(reflected.z, 0.0f, epsilon);
}

TEST_F(Vector3Test, Projection) {
    Vector3 v(2.0f, 1.0f, 0.0f);
    Vector3 onto(1.0f, 0.0f, 0.0f);
    Vector3 projected = v.project(onto);
    EXPECT_NEAR(projected.x, 2.0f, epsilon);
    EXPECT_NEAR(projected.y, 0.0f, epsilon);
    EXPECT_NEAR(projected.z, 0.0f, epsilon);
}

TEST_F(Vector3Test, ProjectionOnPlane) {
    Vector3 v(1.0f, 1.0f, 0.0f);
    Vector3 normal(0.0f, 1.0f, 0.0f);
    Vector3 projected = v.projectOnPlane(normal);
    EXPECT_NEAR(projected.x, 1.0f, epsilon);
    EXPECT_NEAR(projected.y, 0.0f, epsilon);
    EXPECT_NEAR(projected.z, 0.0f, epsilon);
}

// Utility function tests
TEST_F(Vector3Test, IsZero) {
    EXPECT_TRUE(Vector3().isZero());
    EXPECT_TRUE(Vector3(0.0f, 0.0f, 0.0f).isZero());
    EXPECT_FALSE(v1.isZero());
}

TEST_F(Vector3Test, SetZero) {
    Vector3 v(v1);
    v.setZero();
    EXPECT_TRUE(v.isZero());
}

// Static creation tests
TEST_F(Vector3Test, StaticCreation) {
    Vector3 zero = Vector3::zero();
    EXPECT_TRUE(zero.isZero());

    Vector3 one = Vector3::one();
    EXPECT_FLOAT_EQ(one.x, 1.0f);
    EXPECT_FLOAT_EQ(one.y, 1.0f);
    EXPECT_FLOAT_EQ(one.z, 1.0f);

    Vector3 up = Vector3::up();
    EXPECT_FLOAT_EQ(up.x, 0.0f);
    EXPECT_FLOAT_EQ(up.y, 1.0f);
    EXPECT_FLOAT_EQ(up.z, 0.0f);

    Vector3 down = Vector3::down();
    EXPECT_FLOAT_EQ(down.x, 0.0f);
    EXPECT_FLOAT_EQ(down.y, -1.0f);
    EXPECT_FLOAT_EQ(down.z, 0.0f);

    Vector3 right = Vector3::right();
    EXPECT_FLOAT_EQ(right.x, 1.0f);
    EXPECT_FLOAT_EQ(right.y, 0.0f);
    EXPECT_FLOAT_EQ(right.z, 0.0f);

    Vector3 left = Vector3::left();
    EXPECT_FLOAT_EQ(left.x, -1.0f);
    EXPECT_FLOAT_EQ(left.y, 0.0f);
    EXPECT_FLOAT_EQ(left.z, 0.0f);

    Vector3 forward = Vector3::forward();
    EXPECT_FLOAT_EQ(forward.x, 0.0f);
    EXPECT_FLOAT_EQ(forward.y, 0.0f);
    EXPECT_FLOAT_EQ(forward.z, 1.0f);

    Vector3 backward = Vector3::backward();
    EXPECT_FLOAT_EQ(backward.x, 0.0f);
    EXPECT_FLOAT_EQ(backward.y, 0.0f);
    EXPECT_FLOAT_EQ(backward.z, -1.0f);
}

// Edge cases
TEST_F(Vector3Test, ZeroNormalization) {
    Vector3 v;  // zero vector
    v.normalize();
    // Should remain zero
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(Vector3Test, LargeValues) {
    Vector3 large(1e6f, 2e6f, 3e6f);
    Vector3 normalized = large.normalized();
    EXPECT_NEAR(normalized.length(), 1.0f, epsilon);
}

TEST_F(Vector3Test, CrossProductOrthogonality) {
    Vector3 v1(1.0f, 0.0f, 0.0f);
    Vector3 v2(0.0f, 1.0f, 0.0f);
    Vector3 cross = v1.cross(v2);
    
    // Cross product should be orthogonal to both input vectors
    EXPECT_NEAR(cross.dot(v1), 0.0f, epsilon);
    EXPECT_NEAR(cross.dot(v2), 0.0f, epsilon);
}

TEST_F(Vector3Test, ProjectionOnZeroVector) {
    Vector3 v(1.0f, 2.0f, 3.0f);
    Vector3 zero;
    Vector3 result = v.project(zero);
    // Projection onto zero vector should return zero vector
    EXPECT_TRUE(result.isZero());
}