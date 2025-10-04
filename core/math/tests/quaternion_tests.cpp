#include "quaternion.hpp"
#include "math_constants.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace pynovage::math;

class QuaternionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Identity quaternion
        identity = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
        
        // 90-degree rotation around X axis
        float halfAngle = constants::quarter_pi; // half of 90 degrees
        rotX90 = Quaternion(std::cos(halfAngle), std::sin(halfAngle), 0.0f, 0.0f);
        
        // 90-degree rotation around Y axis
        // Note: positive rotation around Y takes X to -Z and Z to X
        rotY90 = Quaternion(std::cos(halfAngle), 0.0f, -std::sin(halfAngle), 0.0f);
        
        // 90-degree rotation around Z axis
        rotZ90 = Quaternion(std::cos(halfAngle), 0.0f, 0.0f, std::sin(halfAngle));
        
        // Test vectors
        xAxis = Vector3(1.0f, 0.0f, 0.0f);
        yAxis = Vector3(0.0f, 1.0f, 0.0f);
        zAxis = Vector3(0.0f, 0.0f, 1.0f);
    }

    Quaternion identity;
    Quaternion rotX90;
    Quaternion rotY90;
    Quaternion rotZ90;
    Vector3 xAxis;
    Vector3 yAxis;
    Vector3 zAxis;
};

TEST_F(QuaternionTest, DefaultConstructor) {
    Quaternion q;
    EXPECT_FLOAT_EQ(q.w, 1.0f);
    EXPECT_FLOAT_EQ(q.x, 0.0f);
    EXPECT_FLOAT_EQ(q.y, 0.0f);
    EXPECT_FLOAT_EQ(q.z, 0.0f);
}

TEST_F(QuaternionTest, ComponentConstructor) {
    Quaternion q(2.0f, 3.0f, 4.0f, 5.0f);
    EXPECT_FLOAT_EQ(q.w, 2.0f);
    EXPECT_FLOAT_EQ(q.x, 3.0f);
    EXPECT_FLOAT_EQ(q.y, 4.0f);
    EXPECT_FLOAT_EQ(q.z, 5.0f);
}

TEST_F(QuaternionTest, CopyConstructor) {
    Quaternion q1(2.0f, 3.0f, 4.0f, 5.0f);
    Quaternion q2(q1);
    EXPECT_FLOAT_EQ(q2.w, 2.0f);
    EXPECT_FLOAT_EQ(q2.x, 3.0f);
    EXPECT_FLOAT_EQ(q2.y, 4.0f);
    EXPECT_FLOAT_EQ(q2.z, 5.0f);
}

TEST_F(QuaternionTest, Assignment) {
    Quaternion q1(2.0f, 3.0f, 4.0f, 5.0f);
    Quaternion q2;
    q2 = q1;
    EXPECT_FLOAT_EQ(q2.w, 2.0f);
    EXPECT_FLOAT_EQ(q2.x, 3.0f);
    EXPECT_FLOAT_EQ(q2.y, 4.0f);
    EXPECT_FLOAT_EQ(q2.z, 5.0f);
}

TEST_F(QuaternionTest, Equality) {
    Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion q2(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion q3(1.1f, 2.0f, 3.0f, 4.0f);
    
    EXPECT_TRUE(q1 == q2);
    EXPECT_FALSE(q1 == q3);
    EXPECT_FALSE(q1 != q2);
    EXPECT_TRUE(q1 != q3);
}

TEST_F(QuaternionTest, Magnitude) {
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    float expectedMag = std::sqrt(30.0f); // 1² + 2² + 3² + 4²
    EXPECT_FLOAT_EQ(q.Magnitude(), expectedMag);
    EXPECT_FLOAT_EQ(q.MagnitudeSquared(), 30.0f);
}

TEST_F(QuaternionTest, Normalization) {
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    float mag = q.Magnitude();
    
    Quaternion normalized = q.Normalized();
    EXPECT_FLOAT_EQ(normalized.w, 1.0f/mag);
    EXPECT_FLOAT_EQ(normalized.x, 2.0f/mag);
    EXPECT_FLOAT_EQ(normalized.y, 3.0f/mag);
    EXPECT_FLOAT_EQ(normalized.z, 4.0f/mag);
    EXPECT_FLOAT_EQ(normalized.Magnitude(), 1.0f);
    
    q.Normalize();
    EXPECT_FLOAT_EQ(q.w, 1.0f/mag);
    EXPECT_FLOAT_EQ(q.x, 2.0f/mag);
    EXPECT_FLOAT_EQ(q.y, 3.0f/mag);
    EXPECT_FLOAT_EQ(q.z, 4.0f/mag);
    EXPECT_FLOAT_EQ(q.Magnitude(), 1.0f);
}

TEST_F(QuaternionTest, Conjugate) {
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion conj = q.Conjugate();
    EXPECT_FLOAT_EQ(conj.w, 1.0f);
    EXPECT_FLOAT_EQ(conj.x, -2.0f);
    EXPECT_FLOAT_EQ(conj.y, -3.0f);
    EXPECT_FLOAT_EQ(conj.z, -4.0f);
}

TEST_F(QuaternionTest, DotProduct) {
    Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion q2(2.0f, 3.0f, 4.0f, 5.0f);
    float expected = 1.0f*2.0f + 2.0f*3.0f + 3.0f*4.0f + 4.0f*5.0f;
    EXPECT_FLOAT_EQ(q1.Dot(q2), expected);
}

TEST_F(QuaternionTest, Inverse) {
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion inv = q.Inverse();
    Quaternion result = q * inv;
    EXPECT_FLOAT_EQ(result.w, 1.0f);
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
}

TEST_F(QuaternionTest, Multiplication) {
    // Test that rotation composition works correctly
    // Rotating 90° around X then 90° around Y should give a specific result
    Quaternion result = rotY90 * rotX90;
    
    // Verify the result maintains unit length
    EXPECT_NEAR(result.Magnitude(), 1.0f, 1e-6f);
    
    // Test multiplication identity properties
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion qTimesIdentity = q * identity;
    EXPECT_EQ(q, qTimesIdentity);
    
    Quaternion identityTimesQ = identity * q;
    EXPECT_EQ(q, identityTimesQ);
}

TEST_F(QuaternionTest, VectorRotation) {
    // Test rotating vectors by quaternions
    
    // Rotate x-axis 90° around z-axis should give y-axis
    Vector3 rotatedX = rotZ90.RotateVector(xAxis);
    EXPECT_NEAR(rotatedX.x, 0.0f, 1e-6f);
    EXPECT_NEAR(rotatedX.y, 1.0f, 1e-6f);
    EXPECT_NEAR(rotatedX.z, 0.0f, 1e-6f);
    
    // Rotate y-axis 90° around x-axis should give z-axis
    Vector3 rotatedY = rotX90.RotateVector(yAxis);
    EXPECT_NEAR(rotatedY.x, 0.0f, 1e-6f);
    EXPECT_NEAR(rotatedY.y, 0.0f, 1e-6f);
    EXPECT_NEAR(rotatedY.z, 1.0f, 1e-6f);
    
    // Rotate z-axis 90° around y-axis should give -x-axis
    Vector3 rotatedZ = rotY90.RotateVector(zAxis);
    EXPECT_NEAR(rotatedZ.x, -1.0f, 1e-6f);
    EXPECT_NEAR(rotatedZ.y, 0.0f, 1e-6f);
    EXPECT_NEAR(rotatedZ.z, 0.0f, 1e-6f);
}

TEST_F(QuaternionTest, AxisAngleConversion) {
    // Test converting from and to axis-angle representation
    Vector3 axis(1.0f, 0.0f, 0.0f);
    float angle = constants::half_pi; // 90 degrees
    
    Quaternion q(axis, angle);
    Vector3 resultAxis;
    float resultAngle;
    q.ToAxisAngle(resultAxis, resultAngle);
    
    EXPECT_NEAR(resultAngle, angle, 1e-6f);
    EXPECT_NEAR(resultAxis.x, axis.x, 1e-6f);
    EXPECT_NEAR(resultAxis.y, axis.y, 1e-6f);
    EXPECT_NEAR(resultAxis.z, axis.z, 1e-6f);
}

TEST_F(QuaternionTest, EulerAngleConversion) {
    // Test converting from and to Euler angles
    float roll = constants::quarter_pi;  // 45 degrees
    float pitch = constants::pi / 3.0f; // 60 degrees
    float yaw = constants::pi / 6.0f;   // 30 degrees
    
    Quaternion q = Quaternion::FromEulerAngles(roll, pitch, yaw);
    float resultRoll, resultPitch, resultYaw;
    q.ToEulerAngles(resultRoll, resultPitch, resultYaw);
    
    EXPECT_NEAR(resultRoll, roll, 1e-6f);
    EXPECT_NEAR(resultPitch, pitch, 1e-6f);
    EXPECT_NEAR(resultYaw, yaw, 1e-6f);
}

TEST_F(QuaternionTest, Interpolation) {
    // Test linear interpolation (LERP)
    Quaternion start = identity;
    Quaternion end = rotX90;
    
    Quaternion middle = Quaternion::Lerp(start, end, 0.5f);
    EXPECT_NEAR(middle.Magnitude(), 1.0f, 1e-6f);
    
    // Test spherical linear interpolation (SLERP)
    middle = Quaternion::Slerp(start, end, 0.5f);
    EXPECT_NEAR(middle.Magnitude(), 1.0f, 1e-6f);
    
    // Test properties of SLERP
    Quaternion q0 = Quaternion::Slerp(start, end, 0.0f);
    Quaternion q1 = Quaternion::Slerp(start, end, 1.0f);
    EXPECT_EQ(q0, start);
    EXPECT_EQ(q1, end);
}