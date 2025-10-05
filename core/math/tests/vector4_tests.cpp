#include "../include/vector4.hpp"
#include <gtest/gtest.h>

using namespace pynovage::math;

TEST(Vector4Tests, DefaultConstructor) {
    Vector4 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
    EXPECT_FLOAT_EQ(v.w, 1.0f);  // Points by default
}

TEST(Vector4Tests, ComponentConstructor) {
    Vector4 v(1.0f, 2.0f, 3.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
    EXPECT_FLOAT_EQ(v.w, 0.0f);
}

TEST(Vector4Tests, Addition) {
    Vector4 v1(1.0f, 2.0f, 3.0f, 1.0f);
    Vector4 v2(2.0f, 3.0f, 4.0f, 0.0f);
    Vector4 result = v1 + v2;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 5.0f);
    EXPECT_FLOAT_EQ(result.z, 7.0f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

TEST(Vector4Tests, Subtraction) {
    Vector4 v1(3.0f, 4.0f, 5.0f, 1.0f);
    Vector4 v2(1.0f, 2.0f, 3.0f, 0.0f);
    Vector4 result = v1 - v2;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 2.0f);
    EXPECT_FLOAT_EQ(result.z, 2.0f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

TEST(Vector4Tests, HomogeneousCoordinates) {
    Vector4 point(1.0f, 2.0f, 3.0f, 1.0f);
    Vector4 vector(1.0f, 2.0f, 3.0f, 0.0f);
    
    EXPECT_TRUE(point.isPoint());
    EXPECT_TRUE(vector.isVector());
    EXPECT_FALSE(point.isVector());
    EXPECT_FALSE(vector.isPoint());
}