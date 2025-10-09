#include <gtest/gtest.h>
#include "../../include/vectors/vector4.hpp"
#include <cmath>
#include <sstream>

using namespace nova;

class Vector4Test : public ::testing::Test {
protected:
    const float EPSILON = 1e-6f;
    Vector4f v1{1.0f, 2.0f, 3.0f, 4.0f};
    Vector4f v2{5.0f, 6.0f, 7.0f, 8.0f};
    Vector4f zero{0.0f, 0.0f, 0.0f, 0.0f};
    Vector4f unit{1.0f, 1.0f, 1.0f, 1.0f};
};

// Constructor Tests
TEST_F(Vector4Test, DefaultConstructor) {
    Vector4f v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
    EXPECT_FLOAT_EQ(v.w, 0.0f);
}

TEST_F(Vector4Test, ComponentConstructor) {
    EXPECT_FLOAT_EQ(v1.x, 1.0f);
    EXPECT_FLOAT_EQ(v1.y, 2.0f);
    EXPECT_FLOAT_EQ(v1.z, 3.0f);
    EXPECT_FLOAT_EQ(v1.w, 4.0f);
}

TEST_F(Vector4Test, ScalarConstructor) {
    Vector4f v(3.0f);
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 3.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
    EXPECT_FLOAT_EQ(v.w, 3.0f);
}

TEST_F(Vector4Test, Vector3Constructor) {
    Vector3f v3(1.0f, 2.0f, 3.0f);
    Vector4f v4(v3);  // Should have w = 1.0f by default
    EXPECT_FLOAT_EQ(v4.x, 1.0f);
    EXPECT_FLOAT_EQ(v4.y, 2.0f);
    EXPECT_FLOAT_EQ(v4.z, 3.0f);
    EXPECT_FLOAT_EQ(v4.w, 1.0f);
    
    Vector4f v4w(v3, 0.0f);  // Explicit w value
    EXPECT_FLOAT_EQ(v4w.x, 1.0f);
    EXPECT_FLOAT_EQ(v4w.y, 2.0f);
    EXPECT_FLOAT_EQ(v4w.z, 3.0f);
    EXPECT_FLOAT_EQ(v4w.w, 0.0f);
}

// Access Tests
TEST_F(Vector4Test, ComponentAccess) {
    // Test different union member access
    EXPECT_FLOAT_EQ(v1.x, v1.r);
    EXPECT_FLOAT_EQ(v1.y, v1.g);
    EXPECT_FLOAT_EQ(v1.z, v1.b);
    EXPECT_FLOAT_EQ(v1.w, v1.a);
    EXPECT_FLOAT_EQ(v1.x, v1.s);
    EXPECT_FLOAT_EQ(v1.y, v1.t);
    EXPECT_FLOAT_EQ(v1.z, v1.p);
    EXPECT_FLOAT_EQ(v1.w, v1.q);
}

TEST_F(Vector4Test, ArrayAccess) {
    EXPECT_FLOAT_EQ(v1[0], v1.x);
    EXPECT_FLOAT_EQ(v1[1], v1.y);
    EXPECT_FLOAT_EQ(v1[2], v1.z);
    EXPECT_FLOAT_EQ(v1[3], v1.w);
    EXPECT_FLOAT_EQ(v1.data[0], v1.x);
    EXPECT_FLOAT_EQ(v1.data[1], v1.y);
    EXPECT_FLOAT_EQ(v1.data[2], v1.z);
    EXPECT_FLOAT_EQ(v1.data[3], v1.w);
}

// Arithmetic Tests
TEST_F(Vector4Test, Addition) {
    Vector4f result = v1 + v2;
    EXPECT_FLOAT_EQ(result.x, 6.0f);
    EXPECT_FLOAT_EQ(result.y, 8.0f);
    EXPECT_FLOAT_EQ(result.z, 10.0f);
    EXPECT_FLOAT_EQ(result.w, 12.0f);
    
    result = v1 + 2.0f;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 5.0f);
    EXPECT_FLOAT_EQ(result.w, 6.0f);
}

TEST_F(Vector4Test, Subtraction) {
    Vector4f result = v2 - v1;
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
    EXPECT_FLOAT_EQ(result.w, 4.0f);
    
    result = v2 - 1.0f;
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 5.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
    EXPECT_FLOAT_EQ(result.w, 7.0f);
}

TEST_F(Vector4Test, Multiplication) {
    Vector4f result = v1 * v2;
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 12.0f);
    EXPECT_FLOAT_EQ(result.z, 21.0f);
    EXPECT_FLOAT_EQ(result.w, 32.0f);
    
    result = v1 * 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
    EXPECT_FLOAT_EQ(result.w, 8.0f);
}

TEST_F(Vector4Test, Division) {
    Vector4f result = v2 / v1;
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 7.0f/3.0f);
    EXPECT_FLOAT_EQ(result.w, 2.0f);
    
    result = v2 / 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.5f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 3.5f);
    EXPECT_FLOAT_EQ(result.w, 4.0f);
}

// Assignment Tests
TEST_F(Vector4Test, AdditionAssignment) {
    Vector4f v = v1;
    v += v2;
    EXPECT_FLOAT_EQ(v.x, 6.0f);
    EXPECT_FLOAT_EQ(v.y, 8.0f);
    EXPECT_FLOAT_EQ(v.z, 10.0f);
    EXPECT_FLOAT_EQ(v.w, 12.0f);
    
    v = v1;
    v += 2.0f;
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
    EXPECT_FLOAT_EQ(v.z, 5.0f);
    EXPECT_FLOAT_EQ(v.w, 6.0f);
}

TEST_F(Vector4Test, SubtractionAssignment) {
    Vector4f v = v2;
    v -= v1;
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
    EXPECT_FLOAT_EQ(v.z, 4.0f);
    EXPECT_FLOAT_EQ(v.w, 4.0f);
    
    v = v2;
    v -= 1.0f;
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 5.0f);
    EXPECT_FLOAT_EQ(v.z, 6.0f);
    EXPECT_FLOAT_EQ(v.w, 7.0f);
}

TEST_F(Vector4Test, MultiplicationAssignment) {
    Vector4f v = v1;
    v *= v2;
    EXPECT_FLOAT_EQ(v.x, 5.0f);
    EXPECT_FLOAT_EQ(v.y, 12.0f);
    EXPECT_FLOAT_EQ(v.z, 21.0f);
    EXPECT_FLOAT_EQ(v.w, 32.0f);
    
    v = v1;
    v *= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 2.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
    EXPECT_FLOAT_EQ(v.z, 6.0f);
    EXPECT_FLOAT_EQ(v.w, 8.0f);
}

TEST_F(Vector4Test, DivisionAssignment) {
    Vector4f v = v2;
    v /= v1;
    EXPECT_FLOAT_EQ(v.x, 5.0f);
    EXPECT_FLOAT_EQ(v.y, 3.0f);
    EXPECT_FLOAT_EQ(v.z, 7.0f/3.0f);
    EXPECT_FLOAT_EQ(v.w, 2.0f);
    
    v = v2;
    v /= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 2.5f);
    EXPECT_FLOAT_EQ(v.y, 3.0f);
    EXPECT_FLOAT_EQ(v.z, 3.5f);
    EXPECT_FLOAT_EQ(v.w, 4.0f);
}

// Comparison Tests
TEST_F(Vector4Test, Equality) {
    Vector4f a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f b(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f c(4.0f, 3.0f, 2.0f, 1.0f);
    
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST_F(Vector4Test, LexicographicalComparison) {
    Vector4f a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f b(1.0f, 2.0f, 3.0f, 5.0f);
    Vector4f c(2.0f, 1.0f, 1.0f, 1.0f);
    
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a < c);
    EXPECT_FALSE(b < a);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= a);
    EXPECT_TRUE(a >= a);
}

TEST_F(Vector4Test, MagnitudeComparison) {
    Vector4f a(4.0f, 0.0f, 0.0f, 0.0f);  // length = 4
    Vector4f b(2.0f, 2.0f, 2.0f, 0.0f);  // length ≈ 3.46
    Vector4f c(0.0f, 2.0f, 0.0f, 0.0f);  // length = 2
    
    EXPECT_TRUE(a.isLongerThan(b));
    EXPECT_TRUE(b.isLongerThan(c));
    EXPECT_TRUE(c.isShorterThan(a));
    EXPECT_TRUE(b.isShorterThan(a));
    EXPECT_TRUE(a.isLongerThanOrEqual(a));
    EXPECT_TRUE(c.isShorterThanOrEqual(c));
}

// SIMD-specific comparison test
TEST_F(Vector4Test, ComponentWiseComparison) {
    Vector4f a(1.0f, 5.0f, 3.0f, 7.0f);
    Vector4f b(4.0f, 2.0f, 6.0f, 4.0f);
    
    // Compare a > b component-wise
    // Should set bits where a component is greater than b's component
    // a = (1,5,3,7), b = (4,2,6,4)
    // Result: (0,1,0,1) in binary: 0b1010 = 10
    int mask = a.compare(b);
    EXPECT_EQ(mask & 1, 0);     // x: 1 < 4
    EXPECT_EQ(mask & 2, 2);     // y: 5 > 2
    EXPECT_EQ(mask & 4, 0);     // z: 3 < 6
    EXPECT_EQ(mask & 8, 8);     // w: 7 > 4
}

// Vector Operations Tests
TEST_F(Vector4Test, DotProduct) {
    float dot = v1.dot(v2);
    EXPECT_FLOAT_EQ(dot, 70.0f);  // 1*5 + 2*6 + 3*7 + 4*8
}

TEST_F(Vector4Test, Length) {
    EXPECT_FLOAT_EQ(unit.length(), 2.0f);
    EXPECT_FLOAT_EQ(zero.length(), 0.0f);
    EXPECT_FLOAT_EQ(v1.length(), std::sqrt(30.0f));
}

TEST_F(Vector4Test, LengthSquared) {
    EXPECT_FLOAT_EQ(unit.lengthSquared(), 4.0f);
    EXPECT_FLOAT_EQ(zero.lengthSquared(), 0.0f);
    EXPECT_FLOAT_EQ(v1.lengthSquared(), 30.0f);
}

TEST_F(Vector4Test, Normalize) {
    Vector4f v = v1;
    float len = v.length();
    v.normalize();
    EXPECT_NEAR(v.length(), 1.0f, EPSILON);
    EXPECT_NEAR(v.x, v1.x/len, EPSILON);
    EXPECT_NEAR(v.y, v1.y/len, EPSILON);
    EXPECT_NEAR(v.z, v1.z/len, EPSILON);
    EXPECT_NEAR(v.w, v1.w/len, EPSILON);
    
    // Test normalized()
    Vector4f n = v1.normalized();
    EXPECT_NEAR(n.length(), 1.0f, EPSILON);
    EXPECT_NEAR(n.x, v1.x/len, EPSILON);
    EXPECT_NEAR(n.y, v1.y/len, EPSILON);
    EXPECT_NEAR(n.z, v1.z/len, EPSILON);
    EXPECT_NEAR(n.w, v1.w/len, EPSILON);
    
    // Test zero vector
    Vector4f z = zero;
    z.normalize();
    EXPECT_FLOAT_EQ(z.x, 0.0f);
    EXPECT_FLOAT_EQ(z.y, 0.0f);
    EXPECT_FLOAT_EQ(z.z, 0.0f);
    EXPECT_FLOAT_EQ(z.w, 0.0f);
}

// Data Access Tests
TEST_F(Vector4Test, GetData) {
    const float* data = v1.getData();
    EXPECT_FLOAT_EQ(data[0], v1.x);
    EXPECT_FLOAT_EQ(data[1], v1.y);
    EXPECT_FLOAT_EQ(data[2], v1.z);
    EXPECT_FLOAT_EQ(data[3], v1.w);
}

TEST_F(Vector4Test, Size) {
    EXPECT_EQ(Vector4f::size(), 4);
}