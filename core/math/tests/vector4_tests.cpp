#include "../include/vector4.hpp"
#include "../include/math_constants.hpp"
#include <gtest/gtest.h>
#include <sstream>

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

TEST(Vector4Tests, ComponentAccess) {
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v[0], 1.0f);
    EXPECT_FLOAT_EQ(v[1], 2.0f);
    EXPECT_FLOAT_EQ(v[2], 3.0f);
    EXPECT_FLOAT_EQ(v[3], 4.0f);

    v[0] = 5.0f;
    EXPECT_FLOAT_EQ(v.x, 5.0f);
}

TEST(Vector4Tests, StaticVectors) {
    auto x = Vector4::unitX();
    auto y = Vector4::unitY();
    auto z = Vector4::unitZ();
    auto w = Vector4::unitW();

    EXPECT_FLOAT_EQ(x.x, 1.0f);
    EXPECT_FLOAT_EQ(y.y, 1.0f);
    EXPECT_FLOAT_EQ(z.z, 1.0f);
    EXPECT_FLOAT_EQ(w.w, 1.0f);
}

TEST(Vector4Tests, ComponentWiseOperations) {
    Vector4 a(2.0f, 3.0f, 4.0f, 5.0f);
    Vector4 b(1.0f, 2.0f, 2.0f, 2.5f);

    auto prod = a.cwiseProduct(b);
    EXPECT_FLOAT_EQ(prod.x, 2.0f);
    EXPECT_FLOAT_EQ(prod.y, 6.0f);
    EXPECT_FLOAT_EQ(prod.z, 8.0f);
    EXPECT_FLOAT_EQ(prod.w, 12.5f);

    auto quot = a.cwiseQuotient(b);
    EXPECT_FLOAT_EQ(quot.x, 2.0f);
    EXPECT_FLOAT_EQ(quot.y, 1.5f);
    EXPECT_FLOAT_EQ(quot.z, 2.0f);
    EXPECT_FLOAT_EQ(quot.w, 2.0f);
}

TEST(Vector4Tests, Comparisons) {
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 c(1.0f, 2.0f, 3.0f, 5.0f);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST(Vector4Tests, Lerp) {
    Vector4 a(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4 b(2.0f, 4.0f, 6.0f, 8.0f);

    auto mid = Vector4::lerp(a, b, 0.5f);
    EXPECT_FLOAT_EQ(mid.x, 1.0f);
    EXPECT_FLOAT_EQ(mid.y, 2.0f);
    EXPECT_FLOAT_EQ(mid.z, 3.0f);
    EXPECT_FLOAT_EQ(mid.w, 4.0f);
}

TEST(Vector4Tests, Distance) {
    Vector4 a(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4 b(1.0f, 0.0f, 0.0f, 0.0f);

    EXPECT_FLOAT_EQ(a.distanceTo(b), 1.0f);
    EXPECT_FLOAT_EQ(a.distanceSquaredTo(b), 1.0f);

    Vector4 c(1.0f, 1.0f, 1.0f, 1.0f);
    EXPECT_FLOAT_EQ(a.distanceTo(c), 2.0f);
    EXPECT_FLOAT_EQ(a.distanceSquaredTo(c), 4.0f);
}

TEST(Vector4Tests, Angle) {
    Vector4 x = Vector4::unitX();
    Vector4 y = Vector4::unitY();

    using namespace pynovage::math::constants;
    EXPECT_FLOAT_EQ(x.angleTo(y), half_pi);
    EXPECT_FLOAT_EQ(x.angleTo(x), 0.0f);
}

TEST(Vector4Tests, CrossProduct) {
    Vector4 x = Vector4::unitX();
    Vector4 y = Vector4::unitY();

    auto z = x.cross(y);
    EXPECT_FLOAT_EQ(z.x, 0.0f);
    EXPECT_FLOAT_EQ(z.y, 0.0f);
    EXPECT_FLOAT_EQ(z.z, 1.0f);
    EXPECT_FLOAT_EQ(z.w, 0.0f);
}

TEST(Vector4Tests, ProjectReject) {
    Vector4 v(3.0f, 3.0f, 0.0f, 0.0f);
    Vector4 x = Vector4::unitX();

    auto proj = v.projectOnto(x);
    EXPECT_FLOAT_EQ(proj.x, 3.0f);
    EXPECT_FLOAT_EQ(proj.y, 0.0f);
    EXPECT_FLOAT_EQ(proj.z, 0.0f);
    EXPECT_FLOAT_EQ(proj.w, 0.0f);

    auto rej = v.rejectFrom(x);
    EXPECT_FLOAT_EQ(rej.x, 0.0f);
    EXPECT_FLOAT_EQ(rej.y, 3.0f);
    EXPECT_FLOAT_EQ(rej.z, 0.0f);
    EXPECT_FLOAT_EQ(rej.w, 0.0f);
}

TEST(Vector4Tests, MinMax) {
    Vector4 a(1.0f, 4.0f, 2.0f, 5.0f);
    Vector4 b(2.0f, 3.0f, 1.0f, 6.0f);

    auto min = Vector4::min(a, b);
    EXPECT_FLOAT_EQ(min.x, 1.0f);
    EXPECT_FLOAT_EQ(min.y, 3.0f);
    EXPECT_FLOAT_EQ(min.z, 1.0f);
    EXPECT_FLOAT_EQ(min.w, 5.0f);

    auto max = Vector4::max(a, b);
    EXPECT_FLOAT_EQ(max.x, 2.0f);
    EXPECT_FLOAT_EQ(max.y, 4.0f);
    EXPECT_FLOAT_EQ(max.z, 2.0f);
    EXPECT_FLOAT_EQ(max.w, 6.0f);
}

TEST(Vector4Tests, StringConversion) {
    Vector4 v(1.234f, 2.345f, 3.456f, 4.567f);
    std::string str = v.toString();
    EXPECT_EQ(str, "(1.234, 2.345, 3.456, 4.567)");
}

TEST(Vector4Tests, StreamOperators) {
    Vector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
    std::stringstream ss;
    ss << v1;
    EXPECT_EQ(ss.str(), "(1.000, 2.000, 3.000, 4.000)");

    Vector4 v2;
    ss.str("(5.0, 6.0, 7.0, 8.0)");
    ss >> v2;
    EXPECT_FLOAT_EQ(v2.x, 5.0f);
    EXPECT_FLOAT_EQ(v2.y, 6.0f);
    EXPECT_FLOAT_EQ(v2.z, 7.0f);
    EXPECT_FLOAT_EQ(v2.w, 8.0f);
}
