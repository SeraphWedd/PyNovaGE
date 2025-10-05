#include <gtest/gtest.h>
#include "vector2.hpp"
#include "math_constants.hpp"
#include <cmath>
#include <sstream>
#include <stdexcept>

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

// New functionality tests
TEST_F(Vector2Test, ArrayAccess) {
    Vector2 v(1.0f, 2.0f);
    EXPECT_FLOAT_EQ(v[0], 1.0f);
    EXPECT_FLOAT_EQ(v[1], 2.0f);

    // Test mutability
    v[0] = 3.0f;
    v[1] = 4.0f;
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);

#ifdef _DEBUG
    // Test bounds checking
    EXPECT_THROW(v[-1], std::out_of_range);
    EXPECT_THROW(v[2], std::out_of_range);
#endif
}

TEST_F(Vector2Test, ComparisonOperators) {
    Vector2 a(1.0f, 2.0f);
    Vector2 b(1.0f, 2.0f);
    Vector2 c(2.0f, 1.0f);
    Vector2 d(2.0f, 3.0f);

    // Equality
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);

    // Less than (true only if ALL components are less)
    EXPECT_TRUE(a < d);     // (1,2) < (2,3) -- all components are less
    EXPECT_FALSE(d < a);   // No components less
    EXPECT_FALSE(a < b);   // Equal components
    EXPECT_FALSE(c < d);   // Not all components less

    // Less than or equal (true if ALL components are less or equal)
    EXPECT_TRUE(a <= d);   // All components less or equal
    EXPECT_FALSE(d <= a);  // Some components greater
    EXPECT_TRUE(a <= b);   // All components equal
    EXPECT_TRUE(c <= d);   // All components less or equal (2<=2, 1<=3)

    // Greater than
    EXPECT_FALSE(a > d);   // Both components less
    EXPECT_TRUE(d > a);    // Both components greater
    EXPECT_FALSE(a > b);   // Equal components
    EXPECT_FALSE(c > d);   // Mixed components

    // Greater than or equal
    EXPECT_FALSE(a >= d);  // Both components less
    EXPECT_TRUE(d >= a);   // Both components greater
    EXPECT_TRUE(a >= b);   // Equal components
    EXPECT_FALSE(c >= d);  // Mixed components
}

TEST_F(Vector2Test, StringConversion) {
    Vector2 v(1.234f, -5.678f);
    std::string str = v.toString();
    EXPECT_EQ(str, "(1.234, -5.678)");

    // Test stream operators
    std::stringstream ss;
    ss << v;
    EXPECT_EQ(ss.str(), "(1.234, -5.678)");

    Vector2 parsed;
    ss.str("(3.456, -7.890)");
    ss >> parsed;
    EXPECT_FLOAT_EQ(parsed.x, 3.456f);
    EXPECT_FLOAT_EQ(parsed.y, -7.890f);
}

TEST_F(Vector2Test, LerpFunction) {
    Vector2 a(0.0f, 0.0f);
    Vector2 b(2.0f, 4.0f);

    Vector2 mid = Vector2::lerp(a, b, 0.5f);
    EXPECT_FLOAT_EQ(mid.x, 1.0f);
    EXPECT_FLOAT_EQ(mid.y, 2.0f);

    Vector2 start = Vector2::lerp(a, b, 0.0f);
    EXPECT_FLOAT_EQ(start.x, a.x);
    EXPECT_FLOAT_EQ(start.y, a.y);

    Vector2 end = Vector2::lerp(a, b, 1.0f);
    EXPECT_FLOAT_EQ(end.x, b.x);
    EXPECT_FLOAT_EQ(end.y, b.y);
}

TEST_F(Vector2Test, DistanceAndAngle) {
    Vector2 a(1.0f, 0.0f);
    Vector2 b(2.0f, 2.0f);

    EXPECT_FLOAT_EQ(a.distanceTo(b), std::sqrt(5.0f));
    EXPECT_FLOAT_EQ(a.distanceSquaredTo(b), 5.0f);

    // Test angle between vectors
    Vector2 right(1.0f, 0.0f);
    Vector2 up(0.0f, 1.0f);
    EXPECT_NEAR(right.angleTo(up), constants::half_pi, epsilon);
    
    // Test angle with self (should be 0)
    EXPECT_NEAR(right.angleTo(right), 0.0f, epsilon);
}

TEST_F(Vector2Test, ComponentWiseOperations) {
    Vector2 a(2.0f, 3.0f);
    Vector2 b(4.0f, 2.0f);

    Vector2 prod = a.cwiseProduct(b);
    EXPECT_FLOAT_EQ(prod.x, 8.0f);
    EXPECT_FLOAT_EQ(prod.y, 6.0f);

    Vector2 quot = a.cwiseQuotient(b);
    EXPECT_FLOAT_EQ(quot.x, 0.5f);
    EXPECT_FLOAT_EQ(quot.y, 1.5f);

    // Test min/max
    Vector2 minVec = min(a, b);
    EXPECT_FLOAT_EQ(minVec.x, 2.0f);
    EXPECT_FLOAT_EQ(minVec.y, 2.0f);

    Vector2 maxVec = max(a, b);
    EXPECT_FLOAT_EQ(maxVec.x, 4.0f);
    EXPECT_FLOAT_EQ(maxVec.y, 3.0f);
}

TEST_F(Vector2Test, DirectionalConstants) {
    Vector2 left = Vector2::left();
    EXPECT_FLOAT_EQ(left.x, -1.0f);
    EXPECT_FLOAT_EQ(left.y, 0.0f);

    Vector2 right = Vector2::right();
    EXPECT_FLOAT_EQ(right.x, 1.0f);
    EXPECT_FLOAT_EQ(right.y, 0.0f);

    Vector2 up = Vector2::up();
    EXPECT_FLOAT_EQ(up.x, 0.0f);
    EXPECT_FLOAT_EQ(up.y, 1.0f);

    Vector2 down = Vector2::down();
    EXPECT_FLOAT_EQ(down.x, 0.0f);
    EXPECT_FLOAT_EQ(down.y, -1.0f);

    // Test relationships using unary negation
    EXPECT_FLOAT_EQ((-right).x, left.x);
    EXPECT_FLOAT_EQ((-right).y, left.y);
    EXPECT_FLOAT_EQ((-up).x, down.x);
    EXPECT_FLOAT_EQ((-up).y, down.y);
}
