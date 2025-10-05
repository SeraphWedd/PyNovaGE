#include <gtest/gtest.h>
#include "vector3.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    Vector3 projected = v.projectOnto(onto);
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
    Vector3 result = v.projectOnto(zero);
    // Projection onto zero vector should return zero vector
    EXPECT_TRUE(result.isZero());
}

// Array access bounds checking
TEST_F(Vector3Test, ArrayAccess) {
    Vector3 v(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v[0], 1.0f);
    EXPECT_FLOAT_EQ(v[1], 2.0f);
    EXPECT_FLOAT_EQ(v[2], 3.0f);

#ifdef _DEBUG
    EXPECT_THROW(v[-1], std::out_of_range);
    EXPECT_THROW(v[3], std::out_of_range);
#endif
}

// String conversion and stream operators
TEST_F(Vector3Test, StringConversion) {
    Vector3 v(1.234f, 2.345f, 3.456f);
    std::string str = v.toString();
    // Check format with 3 decimal places
    EXPECT_EQ(str, "(1.234, 2.345, 3.456)");

    std::stringstream ss;
    ss << v;
    EXPECT_EQ(ss.str(), str);

    Vector3 parsed;
    ss.clear();
    ss.str("(4.567, 5.678, 6.789)");
    ss >> parsed;
    EXPECT_NEAR(parsed.x, 4.567f, 0.001f);
    EXPECT_NEAR(parsed.y, 5.678f, 0.001f);
    EXPECT_NEAR(parsed.z, 6.789f, 0.001f);
}

// Component-wise operations
TEST_F(Vector3Test, ComponentWiseOperations) {
    Vector3 v1(2.0f, 3.0f, 4.0f);
    Vector3 v2(3.0f, 2.0f, 1.0f);

    Vector3 product = v1.cwiseProduct(v2);
    EXPECT_FLOAT_EQ(product.x, 6.0f);
    EXPECT_FLOAT_EQ(product.y, 6.0f);
    EXPECT_FLOAT_EQ(product.z, 4.0f);

    Vector3 quotient = v1.cwiseQuotient(v2);
    EXPECT_FLOAT_EQ(quotient.x, 2.0f/3.0f);
    EXPECT_FLOAT_EQ(quotient.y, 1.5f);
    EXPECT_FLOAT_EQ(quotient.z, 4.0f);
}

// Min/Max operations
TEST_F(Vector3Test, MinMaxOperations) {
    Vector3 v1(1.0f, 4.0f, 2.0f);
    Vector3 v2(2.0f, 3.0f, 1.0f);

    Vector3 minVec = min(v1, v2);
    EXPECT_FLOAT_EQ(minVec.x, 1.0f);
    EXPECT_FLOAT_EQ(minVec.y, 3.0f);
    EXPECT_FLOAT_EQ(minVec.z, 1.0f);

    Vector3 maxVec = max(v1, v2);
    EXPECT_FLOAT_EQ(maxVec.x, 2.0f);
    EXPECT_FLOAT_EQ(maxVec.y, 4.0f);
    EXPECT_FLOAT_EQ(maxVec.z, 2.0f);

    // Also test static min/max methods
    Vector3 minVecS = Vector3::min(v1, v2);
    EXPECT_FLOAT_EQ(minVecS.x, 1.0f);
    EXPECT_FLOAT_EQ(minVecS.y, 3.0f);
    EXPECT_FLOAT_EQ(minVecS.z, 1.0f);

    Vector3 maxVecS = Vector3::max(v1, v2);
    EXPECT_FLOAT_EQ(maxVecS.x, 2.0f);
    EXPECT_FLOAT_EQ(maxVecS.y, 4.0f);
    EXPECT_FLOAT_EQ(maxVecS.z, 2.0f);
}

// Distance and angle calculations
TEST_F(Vector3Test, DistanceAndAngle) {
    Vector3 v1(1.0f, 0.0f, 0.0f);
    Vector3 v2(2.0f, 0.0f, 0.0f);
    Vector3 v3(0.0f, 1.0f, 0.0f);

    EXPECT_FLOAT_EQ(v1.distanceTo(v2), 1.0f);
    EXPECT_FLOAT_EQ(v1.distanceSquaredTo(v2), 1.0f);
    EXPECT_FLOAT_EQ(v1.angleTo(v3), M_PI / 2.0f);

    Vector3 v4(-1.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(v1.angleTo(v4), M_PI);

    Vector3 v5(1.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(v1.angleTo(v5), 0.0f);
}

// Lerp function tests
TEST_F(Vector3Test, ComparisonOperators) {
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(1.0f, 2.0f, 3.0f);
    Vector3 c(2.0f, 1.0f, 4.0f);
    Vector3 d(2.0f, 3.0f, 4.0f);

    // Less than (true only if ALL components are less)
    EXPECT_TRUE(a < d);     // (1,2,3) < (2,3,4) -- all components are less
    EXPECT_FALSE(d < a);   // No components less
    EXPECT_FALSE(a < b);   // Equal components
    EXPECT_FALSE(c < d);   // Not all components less

    // Less than or equal (true if ALL components are less or equal)
    EXPECT_TRUE(a <= d);   // All components less or equal
    EXPECT_FALSE(d <= a);  // Some components greater
    EXPECT_TRUE(a <= b);   // All components equal
    EXPECT_TRUE(c <= d);   // All components less or equal (2<=2, 1<=3, 4<=4)

    // Greater than
    EXPECT_FALSE(a > d);   // All components less
    EXPECT_TRUE(d > a);    // All components greater
    EXPECT_FALSE(a > b);   // Equal components
    EXPECT_FALSE(c > d);   // Mixed components

    // Greater than or equal
    EXPECT_FALSE(a >= d);  // All components less
    EXPECT_TRUE(d >= a);   // All components greater
    EXPECT_TRUE(a >= b);   // Equal components
    EXPECT_FALSE(c >= d);  // Mixed components
}

TEST_F(Vector3Test, LerpFunction) {
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(2.0f, 4.0f, 6.0f);

    // Test new lowercase lerp
    Vector3 result1 = Vector3::lerp(v1, v2, 0.5f);
    EXPECT_FLOAT_EQ(result1.x, 1.0f);
    EXPECT_FLOAT_EQ(result1.y, 2.0f);
    EXPECT_FLOAT_EQ(result1.z, 3.0f);


    // Test edge cases
    Vector3 result3 = Vector3::lerp(v1, v2, 0.0f);
    EXPECT_FLOAT_EQ(result3.x, v1.x);
    EXPECT_FLOAT_EQ(result3.y, v1.y);
    EXPECT_FLOAT_EQ(result3.z, v1.z);

    Vector3 result4 = Vector3::lerp(v1, v2, 1.0f);
    EXPECT_FLOAT_EQ(result4.x, v2.x);
    EXPECT_FLOAT_EQ(result4.y, v2.y);
    EXPECT_FLOAT_EQ(result4.z, v2.z);
}
