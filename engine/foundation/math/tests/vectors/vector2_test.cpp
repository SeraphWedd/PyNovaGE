#include <gtest/gtest.h>
#include "../../include/vectors/vector2.hpp"
#include <cmath>
#include <sstream>

using namespace nova;

class Vector2Test : public ::testing::Test {
protected:
    const float EPSILON = 1e-6f;
    Vector2f v1{1.0f, 2.0f};
    Vector2f v2{3.0f, 4.0f};
    Vector2f zero{0.0f, 0.0f};
    Vector2f unit{1.0f, 1.0f};
};

// Constructor Tests
TEST_F(Vector2Test, DefaultConstructor) {
    Vector2f v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST_F(Vector2Test, ComponentConstructor) {
    EXPECT_FLOAT_EQ(v1.x, 1.0f);
    EXPECT_FLOAT_EQ(v1.y, 2.0f);
}

TEST_F(Vector2Test, ScalarConstructor) {
    Vector2f v(3.0f);
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 3.0f);
}

// Access Tests
TEST_F(Vector2Test, ComponentAccess) {
    // Test different union member access
    EXPECT_FLOAT_EQ(v1.x, v1.r);
    EXPECT_FLOAT_EQ(v1.y, v1.g);
    EXPECT_FLOAT_EQ(v1.x, v1.u);
    EXPECT_FLOAT_EQ(v1.y, v1.v);
}

TEST_F(Vector2Test, ArrayAccess) {
    EXPECT_FLOAT_EQ(v1[0], v1.x);
    EXPECT_FLOAT_EQ(v1[1], v1.y);
    EXPECT_FLOAT_EQ(v1.data[0], v1.x);
    EXPECT_FLOAT_EQ(v1.data[1], v1.y);
}

// Arithmetic Tests
TEST_F(Vector2Test, Addition) {
    Vector2f result = v1 + v2;
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
    
    result = v1 + 2.0f;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
}

TEST_F(Vector2Test, Subtraction) {
    Vector2f result = v2 - v1;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 2.0f);
    
    result = v2 - 1.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
}

TEST_F(Vector2Test, Multiplication) {
    Vector2f result = v1 * v2;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 8.0f);
    
    result = v1 * 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
}

TEST_F(Vector2Test, Division) {
    Vector2f result = v2 / v1;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 2.0f);
    
    result = v2 / 2.0f;
    EXPECT_FLOAT_EQ(result.x, 1.5f);
    EXPECT_FLOAT_EQ(result.y, 2.0f);
}

// Assignment Tests
TEST_F(Vector2Test, AdditionAssignment) {
    Vector2f v = v1;
    v += v2;
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 6.0f);
    
    v = v1;
    v += 2.0f;
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
}

TEST_F(Vector2Test, SubtractionAssignment) {
    Vector2f v = v2;
    v -= v1;
    EXPECT_FLOAT_EQ(v.x, 2.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    
    v = v2;
    v -= 1.0f;
    EXPECT_FLOAT_EQ(v.x, 2.0f);
    EXPECT_FLOAT_EQ(v.y, 3.0f);
}

TEST_F(Vector2Test, MultiplicationAssignment) {
    Vector2f v = v1;
    v *= v2;
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 8.0f);
    
    v = v1;
    v *= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 2.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
}

TEST_F(Vector2Test, DivisionAssignment) {
    Vector2f v = v2;
    v /= v1;
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    
    v = v2;
    v /= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 1.5f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
}

// Comparison Tests
TEST_F(Vector2Test, Equality) {
    Vector2f a(1.0f, 2.0f);
    Vector2f b(1.0f, 2.0f);
    Vector2f c(2.0f, 1.0f);
    
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST_F(Vector2Test, LexicographicalComparison) {
    Vector2f a(1.0f, 2.0f);
    Vector2f b(1.0f, 3.0f);
    Vector2f c(2.0f, 1.0f);
    
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a < c);
    EXPECT_FALSE(b < a);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= a);
    EXPECT_TRUE(a >= a);
}

TEST_F(Vector2Test, MagnitudeComparison) {
    Vector2f a(3.0f, 0.0f);  // length = 3
    Vector2f b(2.0f, 2.0f);  // length ≈ 2.83
    Vector2f c(0.0f, 2.0f);  // length = 2
    
    EXPECT_TRUE(a.isLongerThan(b));
    EXPECT_TRUE(b.isLongerThan(c));
    EXPECT_TRUE(c.isShorterThan(a));
    EXPECT_TRUE(b.isShorterThan(a));
    EXPECT_TRUE(a.isLongerThanOrEqual(a));
    EXPECT_TRUE(c.isShorterThanOrEqual(c));
}

// Vector Operations Tests
TEST_F(Vector2Test, DotProduct) {
    float dot = v1.dot(v2);
    EXPECT_FLOAT_EQ(dot, 11.0f);
}

TEST_F(Vector2Test, Length) {
    EXPECT_FLOAT_EQ(unit.length(), std::sqrt(2.0f));
    EXPECT_FLOAT_EQ(zero.length(), 0.0f);
    EXPECT_FLOAT_EQ(v1.length(), std::sqrt(5.0f));
}

TEST_F(Vector2Test, LengthSquared) {
    EXPECT_FLOAT_EQ(unit.lengthSquared(), 2.0f);
    EXPECT_FLOAT_EQ(zero.lengthSquared(), 0.0f);
    EXPECT_FLOAT_EQ(v1.lengthSquared(), 5.0f);
}

TEST_F(Vector2Test, Normalize) {
    Vector2f v = v1;
    float len = v.length();
    v.normalize();
    EXPECT_NEAR(v.length(), 1.0f, EPSILON);
    EXPECT_NEAR(v.x, v1.x/len, EPSILON);
    EXPECT_NEAR(v.y, v1.y/len, EPSILON);
    
    // Test normalized()
    Vector2f n = v1.normalized();
    EXPECT_NEAR(n.length(), 1.0f, EPSILON);
    EXPECT_NEAR(n.x, v1.x/len, EPSILON);
    EXPECT_NEAR(n.y, v1.y/len, EPSILON);
    
    // Test zero vector
    Vector2f z = zero;
    z.normalize();
    EXPECT_FLOAT_EQ(z.x, 0.0f);
    EXPECT_FLOAT_EQ(z.y, 0.0f);
}

// Data Access Tests
TEST_F(Vector2Test, GetData) {
    const float* data = v1.getData();
    EXPECT_FLOAT_EQ(data[0], v1.x);
    EXPECT_FLOAT_EQ(data[1], v1.y);
}

TEST_F(Vector2Test, Size) {
    EXPECT_EQ(Vector2f::size(), 2);
}