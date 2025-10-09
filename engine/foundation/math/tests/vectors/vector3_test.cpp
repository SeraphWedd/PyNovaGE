#include <gtest/gtest.h>
#include "../../include/vectors/vector3.hpp"
#include <cmath>
#include <sstream>

using namespace nova;

class Vector3Test : public ::testing::Test {
protected:
    const float EPSILON = 1e-6f;
    Vector3f v1{1.0f, 2.0f, 3.0f};
    Vector3f v2{4.0f, 5.0f, 6.0f};
    Vector3f zero{0.0f, 0.0f, 0.0f};
    Vector3f unit{1.0f, 1.0f, 1.0f};
    // Unit vectors for cross product tests
    Vector3f unitX{1.0f, 0.0f, 0.0f};
    Vector3f unitY{0.0f, 1.0f, 0.0f};
    Vector3f unitZ{0.0f, 0.0f, 1.0f};
};

// Constructor Tests
TEST_F(Vector3Test, DefaultConstructor) {
    Vector3f v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(Vector3Test, ComponentConstructor) {
    EXPECT_FLOAT_EQ(v1.x, 1.0f);
    EXPECT_FLOAT_EQ(v1.y, 2.0f);
    EXPECT_FLOAT_EQ(v1.z, 3.0f);
}

TEST_F(Vector3Test, ScalarConstructor) {
    Vector3f v(3.0f);
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 3.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

// Access Tests
TEST_F(Vector3Test, ComponentAccess) {
    // Test different union member access
    EXPECT_FLOAT_EQ(v1.x, v1.r);
    EXPECT_FLOAT_EQ(v1.y, v1.g);
    EXPECT_FLOAT_EQ(v1.z, v1.b);
    EXPECT_FLOAT_EQ(v1.x, v1.u);
    EXPECT_FLOAT_EQ(v1.y, v1.v);
    EXPECT_FLOAT_EQ(v1.z, v1.w);
}

TEST_F(Vector3Test, ArrayAccess) {
    EXPECT_FLOAT_EQ(v1[0], v1.x);
    EXPECT_FLOAT_EQ(v1[1], v1.y);
    EXPECT_FLOAT_EQ(v1[2], v1.z);
    EXPECT_FLOAT_EQ(v1.data[0], v1.x);
    EXPECT_FLOAT_EQ(v1.data[1], v1.y);
    EXPECT_FLOAT_EQ(v1.data[2], v1.z);
}

// Arithmetic Tests
TEST_F(Vector3Test, Addition) {
    Vector3f result = v1 + v2;
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
    
    result = v1 + 2.0f;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 5.0f);
}

TEST_F(Vector3Test, Subtraction) {
    Vector3f result = v2 - v1;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 3.0f);
    
    result = v2 - 1.0f;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 5.0f);
}

TEST_F(Vector3Test, Multiplication) {
    Vector3f result = v1 * v2;
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 10.0f);
    EXPECT_FLOAT_EQ(result.z, 18.0f);
    
    result = v1 * 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

TEST_F(Vector3Test, Division) {
    Vector3f result = v2 / v1;
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 2.5f);
    EXPECT_FLOAT_EQ(result.z, 2.0f);
    
    result = v2 / 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 2.5f);
    EXPECT_FLOAT_EQ(result.z, 3.0f);
}

// Assignment Tests
TEST_F(Vector3Test, AdditionAssignment) {
    Vector3f v = v1;
    v += v2;
    EXPECT_FLOAT_EQ(v.x, 5.0f);
    EXPECT_FLOAT_EQ(v.y, 7.0f);
    EXPECT_FLOAT_EQ(v.z, 9.0f);
    
    v = v1;
    v += 2.0f;
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
    EXPECT_FLOAT_EQ(v.z, 5.0f);
}

TEST_F(Vector3Test, SubtractionAssignment) {
    Vector3f v = v2;
    v -= v1;
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 3.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
    
    v = v2;
    v -= 1.0f;
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
    EXPECT_FLOAT_EQ(v.z, 5.0f);
}

TEST_F(Vector3Test, MultiplicationAssignment) {
    Vector3f v = v1;
    v *= v2;
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 10.0f);
    EXPECT_FLOAT_EQ(v.z, 18.0f);
    
    v = v1;
    v *= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 2.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
    EXPECT_FLOAT_EQ(v.z, 6.0f);
}

TEST_F(Vector3Test, DivisionAssignment) {
    Vector3f v = v2;
    v /= v1;
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 2.5f);
    EXPECT_FLOAT_EQ(v.z, 2.0f);
    
    v = v2;
    v /= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 2.0f);
    EXPECT_FLOAT_EQ(v.y, 2.5f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

// Comparison Tests
TEST_F(Vector3Test, Equality) {
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(1.0f, 2.0f, 3.0f);
    Vector3f c(3.0f, 2.0f, 1.0f);
    
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST_F(Vector3Test, LexicographicalComparison) {
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(1.0f, 2.0f, 4.0f);
    Vector3f c(2.0f, 1.0f, 1.0f);
    
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a < c);
    EXPECT_FALSE(b < a);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= a);
    EXPECT_TRUE(a >= a);
}

TEST_F(Vector3Test, MagnitudeComparison) {
    Vector3f a(3.0f, 0.0f, 0.0f);  // length = 3
    Vector3f b(2.0f, 2.0f, 0.0f);  // length ≈ 2.83
    Vector3f c(0.0f, 2.0f, 0.0f);  // length = 2
    
    EXPECT_TRUE(a.isLongerThan(b));
    EXPECT_TRUE(b.isLongerThan(c));
    EXPECT_TRUE(c.isShorterThan(a));
    EXPECT_TRUE(b.isShorterThan(a));
    EXPECT_TRUE(a.isLongerThanOrEqual(a));
    EXPECT_TRUE(c.isShorterThanOrEqual(c));
}

// Vector Operations Tests
TEST_F(Vector3Test, DotProduct) {
    float dot = v1.dot(v2);
    EXPECT_FLOAT_EQ(dot, 32.0f);
}

TEST_F(Vector3Test, CrossProduct) {
    // Test unit vector cross products
    Vector3f result = unitX.cross(unitY);
    EXPECT_NEAR(result.x, unitZ.x, EPSILON);
    EXPECT_NEAR(result.y, unitZ.y, EPSILON);
    EXPECT_NEAR(result.z, unitZ.z, EPSILON);
    
    result = unitY.cross(unitZ);
    EXPECT_NEAR(result.x, unitX.x, EPSILON);
    EXPECT_NEAR(result.y, unitX.y, EPSILON);
    EXPECT_NEAR(result.z, unitX.z, EPSILON);
    
    result = unitZ.cross(unitX);
    EXPECT_NEAR(result.x, unitY.x, EPSILON);
    EXPECT_NEAR(result.y, unitY.y, EPSILON);
    EXPECT_NEAR(result.z, unitY.z, EPSILON);
    
    // Test arbitrary vectors
    result = v1.cross(v2);
    EXPECT_FLOAT_EQ(result.x, -3.0f);  // 2*6 - 3*5
    EXPECT_FLOAT_EQ(result.y, 6.0f);   // 3*4 - 1*6
    EXPECT_FLOAT_EQ(result.z, -3.0f);  // 1*5 - 2*4
}

TEST_F(Vector3Test, Length) {
    EXPECT_FLOAT_EQ(unit.length(), std::sqrt(3.0f));
    EXPECT_FLOAT_EQ(zero.length(), 0.0f);
    EXPECT_FLOAT_EQ(v1.length(), std::sqrt(14.0f));
}

TEST_F(Vector3Test, LengthSquared) {
    EXPECT_FLOAT_EQ(unit.lengthSquared(), 3.0f);
    EXPECT_FLOAT_EQ(zero.lengthSquared(), 0.0f);
    EXPECT_FLOAT_EQ(v1.lengthSquared(), 14.0f);
}

TEST_F(Vector3Test, Normalize) {
    Vector3f v = v1;
    float len = v.length();
    v.normalize();
    EXPECT_NEAR(v.length(), 1.0f, EPSILON);
    EXPECT_NEAR(v.x, v1.x/len, EPSILON);
    EXPECT_NEAR(v.y, v1.y/len, EPSILON);
    EXPECT_NEAR(v.z, v1.z/len, EPSILON);
    
    // Test normalized()
    Vector3f n = v1.normalized();
    EXPECT_NEAR(n.length(), 1.0f, EPSILON);
    EXPECT_NEAR(n.x, v1.x/len, EPSILON);
    EXPECT_NEAR(n.y, v1.y/len, EPSILON);
    EXPECT_NEAR(n.z, v1.z/len, EPSILON);
    
    // Test zero vector
    Vector3f z = zero;
    z.normalize();
    EXPECT_FLOAT_EQ(z.x, 0.0f);
    EXPECT_FLOAT_EQ(z.y, 0.0f);
    EXPECT_FLOAT_EQ(z.z, 0.0f);
}

// Data Access Tests
TEST_F(Vector3Test, GetData) {
    const float* data = v1.getData();
    EXPECT_FLOAT_EQ(data[0], v1.x);
    EXPECT_FLOAT_EQ(data[1], v1.y);
    EXPECT_FLOAT_EQ(data[2], v1.z);
}

TEST_F(Vector3Test, Size) {
    EXPECT_EQ(Vector3f::size(), 3);
}