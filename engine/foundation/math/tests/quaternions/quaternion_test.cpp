#include <gtest/gtest.h>
#include "quaternions/quaternion.hpp"
#include "matrices/matrices.hpp"

using namespace PyNovaGE;

class QuaternionTest : public ::testing::Test {
protected:
    static constexpr float EPSILON = 1e-5f;
    
    bool isNear(float a, float b, float epsilon = EPSILON) {
        return std::abs(a - b) < epsilon;
    }
    
    bool isNear(const Quaternionf& a, const Quaternionf& b, float epsilon = EPSILON) {
        return isNear(a.x(), b.x(), epsilon) &&
               isNear(a.y(), b.y(), epsilon) &&
               isNear(a.z(), b.z(), epsilon) &&
               isNear(a.w(), b.w(), epsilon);
    }
    
    bool isNear(const Vector3f& a, const Vector3f& b, float epsilon = EPSILON) {
        return isNear(a[0], b[0], epsilon) &&
               isNear(a[1], b[1], epsilon) &&
               isNear(a[2], b[2], epsilon);
    }
};

TEST_F(QuaternionTest, Construction) {
    // Default constructor (identity)
    Quaternionf q1;
    EXPECT_FLOAT_EQ(q1.x(), 0.0f);
    EXPECT_FLOAT_EQ(q1.y(), 0.0f);
    EXPECT_FLOAT_EQ(q1.z(), 0.0f);
    EXPECT_FLOAT_EQ(q1.w(), 1.0f);
    
    // Component constructor
    Quaternionf q2(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(q2.x(), 1.0f);
    EXPECT_FLOAT_EQ(q2.y(), 2.0f);
    EXPECT_FLOAT_EQ(q2.z(), 3.0f);
    EXPECT_FLOAT_EQ(q2.w(), 4.0f);
    
    // Axis-angle constructor
    Vector3f axis(1.0f, 0.0f, 0.0f);
    float angle = M_PI / 2.0f;
    Quaternionf q3(axis, angle);
    EXPECT_TRUE(isNear(q3.x(), 0.7071068f));
    EXPECT_TRUE(isNear(q3.y(), 0.0f));
    EXPECT_TRUE(isNear(q3.z(), 0.0f));
    EXPECT_TRUE(isNear(q3.w(), 0.7071068f));
}

TEST_F(QuaternionTest, ComponentAccess) {
    Quaternionf q(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Named access
    EXPECT_FLOAT_EQ(q.x(), 1.0f);
    EXPECT_FLOAT_EQ(q.y(), 2.0f);
    EXPECT_FLOAT_EQ(q.z(), 3.0f);
    EXPECT_FLOAT_EQ(q.w(), 4.0f);
    
    // Array-style access
    EXPECT_FLOAT_EQ(q[0], 1.0f);
    EXPECT_FLOAT_EQ(q[1], 2.0f);
    EXPECT_FLOAT_EQ(q[2], 3.0f);
    EXPECT_FLOAT_EQ(q[3], 4.0f);
    
    // Modify through named access
    q.x() = 5.0f;
    q.y() = 6.0f;
    q.z() = 7.0f;
    q.w() = 8.0f;
    
    EXPECT_FLOAT_EQ(q[0], 5.0f);
    EXPECT_FLOAT_EQ(q[1], 6.0f);
    EXPECT_FLOAT_EQ(q[2], 7.0f);
    EXPECT_FLOAT_EQ(q[3], 8.0f);
}

TEST_F(QuaternionTest, BasicArithmetic) {
    Quaternionf q1(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternionf q2(5.0f, 6.0f, 7.0f, 8.0f);
    
    // Addition
    Quaternionf sum = q1 + q2;
    EXPECT_FLOAT_EQ(sum.x(), 6.0f);
    EXPECT_FLOAT_EQ(sum.y(), 8.0f);
    EXPECT_FLOAT_EQ(sum.z(), 10.0f);
    EXPECT_FLOAT_EQ(sum.w(), 12.0f);
    
    // Subtraction
    Quaternionf diff = q2 - q1;
    EXPECT_FLOAT_EQ(diff.x(), 4.0f);
    EXPECT_FLOAT_EQ(diff.y(), 4.0f);
    EXPECT_FLOAT_EQ(diff.z(), 4.0f);
    EXPECT_FLOAT_EQ(diff.w(), 4.0f);
    
    // Scalar multiplication
    Quaternionf scaled = q1 * 2.0f;
    EXPECT_FLOAT_EQ(scaled.x(), 2.0f);
    EXPECT_FLOAT_EQ(scaled.y(), 4.0f);
    EXPECT_FLOAT_EQ(scaled.z(), 6.0f);
    EXPECT_FLOAT_EQ(scaled.w(), 8.0f);
}

TEST_F(QuaternionTest, QuaternionMultiplication) {
    // Test quaternion multiplication (Hamilton product)
    Quaternionf q1(1.0f, 0.0f, 0.0f, 0.0f);  // i
    Quaternionf q2(0.0f, 1.0f, 0.0f, 0.0f);  // j
    
    Quaternionf result = q1 * q2;  // i * j = k
    EXPECT_TRUE(isNear(result.x(), 0.0f));
    EXPECT_TRUE(isNear(result.y(), 0.0f));
    EXPECT_TRUE(isNear(result.z(), 1.0f));
    EXPECT_TRUE(isNear(result.w(), 0.0f));
    
    // Test with identity
    Quaternionf identity = Quaternionf::Identity();
    Quaternionf test(1.0f, 2.0f, 3.0f, 4.0f);
    
    Quaternionf result1 = identity * test;
    Quaternionf result2 = test * identity;
    
    EXPECT_TRUE(isNear(result1, test));
    EXPECT_TRUE(isNear(result2, test));
}

TEST_F(QuaternionTest, VectorRotation) {
    // Rotate vector around Z-axis by 90 degrees
    Quaternionf q(Vector3f(0.0f, 0.0f, 1.0f), M_PI / 2.0f);
    Vector3f v(1.0f, 0.0f, 0.0f);
    
    Vector3f rotated = q * v;
    EXPECT_TRUE(isNear(rotated, Vector3f(0.0f, 1.0f, 0.0f)));
    
    // Rotate vector around X-axis by 90 degrees
    Quaternionf qx(Vector3f(1.0f, 0.0f, 0.0f), M_PI / 2.0f);
    Vector3f vy(0.0f, 1.0f, 0.0f);
    
    Vector3f rotatedX = qx * vy;
    EXPECT_TRUE(isNear(rotatedX, Vector3f(0.0f, 0.0f, 1.0f)));
}

TEST_F(QuaternionTest, MagnitudeAndNormalization) {
    Quaternionf q(3.0f, 4.0f, 0.0f, 0.0f);
    
    EXPECT_FLOAT_EQ(q.lengthSquared(), 25.0f);
    EXPECT_FLOAT_EQ(q.length(), 5.0f);
    
    Quaternionf normalized = q.normalized();
    EXPECT_TRUE(isNear(normalized.length(), 1.0f));
    EXPECT_TRUE(isNear(normalized, Quaternionf(0.6f, 0.8f, 0.0f, 0.0f)));
    
    // Test normalize in place
    q.normalize();
    EXPECT_TRUE(isNear(q.length(), 1.0f));
}

TEST_F(QuaternionTest, ConjugateAndInverse) {
    Quaternionf q(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Test conjugate
    Quaternionf conj = q.conjugate();
    EXPECT_FLOAT_EQ(conj.x(), -1.0f);
    EXPECT_FLOAT_EQ(conj.y(), -2.0f);
    EXPECT_FLOAT_EQ(conj.z(), -3.0f);
    EXPECT_FLOAT_EQ(conj.w(), 4.0f);
    
    // Test inverse
    Quaternionf inv = q.inverse();
    Quaternionf product = q * inv;
    
    // q * q^-1 should equal identity
    EXPECT_TRUE(isNear(product, Quaternionf::Identity(), 1e-4f));
}

TEST_F(QuaternionTest, DotProduct) {
    Quaternionf q1(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternionf q2(5.0f, 6.0f, 7.0f, 8.0f);
    
    float dot = q1.dot(q2);
    EXPECT_FLOAT_EQ(dot, 70.0f); // 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
}

// Temporarily commented out due to circular dependency issues
// Will be re-enabled when matrix conversion methods are implemented
/*
TEST_F(QuaternionTest, MatrixConversion) {
    // Test quaternion to matrix conversion
    Quaternionf q(Vector3f(0.0f, 0.0f, 1.0f), M_PI / 2.0f);
    
    Matrix3f mat3 = q.toMatrix3();
    Matrix4f mat4 = q.toMatrix4();
    
    // Apply rotation to vector using matrix and quaternion
    Vector3f v(1.0f, 0.0f, 0.0f);
    Vector3f rotated_quat = q * v;
    Vector3f rotated_mat3 = mat3 * v;
    
    EXPECT_TRUE(isNear(rotated_quat, rotated_mat3));
    
    // Test round trip: matrix -> quaternion -> matrix
    Quaternionf q_from_mat = Quaternionf::FromMatrix3(mat3);
    Matrix3f mat3_roundtrip = q_from_mat.toMatrix3();
    
    // Should be approximately equal (within numerical precision)
    Vector3f test_vec(1.0f, 1.0f, 1.0f);
    Vector3f result1 = mat3 * test_vec;
    Vector3f result2 = mat3_roundtrip * test_vec;
    EXPECT_TRUE(isNear(result1, result2, 1e-4f));
}
*/

TEST_F(QuaternionTest, EulerAngleConversion) {
    // Test Euler angles to quaternion conversion
    float roll = M_PI / 6.0f;   // 30 degrees
    float pitch = M_PI / 4.0f;  // 45 degrees
    float yaw = M_PI / 3.0f;    // 60 degrees
    
    Quaternionf q(roll, pitch, yaw);
    Vector3f euler = q.toEulerAngles();
    
    // Convert back and check if they're approximately equal
    Quaternionf q_roundtrip = Quaternionf::EulerAngles(euler[0], euler[1], euler[2]);
    
    // Due to gimbal lock and multiple representations, we test if rotations are equivalent
    Vector3f test_vec(1.0f, 2.0f, 3.0f);
    Vector3f rotated1 = q * test_vec;
    Vector3f rotated2 = q_roundtrip * test_vec;
    
    EXPECT_TRUE(isNear(rotated1, rotated2, 1e-4f));
}

TEST_F(QuaternionTest, AxisAngleExtraction) {
    Vector3f original_axis(0.0f, 1.0f, 0.0f);
    float original_angle = M_PI / 3.0f;
    
    Quaternionf q(original_axis, original_angle);
    
    Vector3f extracted_axis = q.getAxis();
    float extracted_angle = q.getAngle();
    
    EXPECT_TRUE(isNear(extracted_axis, original_axis));
    EXPECT_TRUE(isNear(extracted_angle, original_angle));
}

TEST_F(QuaternionTest, Interpolation) {
    Quaternionf q1 = Quaternionf::Identity();
    Quaternionf q2(Vector3f(0.0f, 0.0f, 1.0f), M_PI / 2.0f);
    
    // Test LERP
    Quaternionf lerp_mid = Quaternionf::Lerp(q1, q2, 0.5f);
    
    // Test SLERP
    Quaternionf slerp_mid = Quaternionf::Slerp(q1, q2, 0.5f);
    
    // At t=0, should equal q1
    Quaternionf slerp_start = Quaternionf::Slerp(q1, q2, 0.0f);
    EXPECT_TRUE(isNear(slerp_start, q1));
    
    // At t=1, should equal q2
    Quaternionf slerp_end = Quaternionf::Slerp(q1, q2, 1.0f);
    EXPECT_TRUE(isNear(slerp_end, q2, 1e-4f));
    
    // Interpolated quaternions should be normalized
    EXPECT_TRUE(isNear(lerp_mid.length(), 1.0f));
    EXPECT_TRUE(isNear(slerp_mid.length(), 1.0f));
}

TEST_F(QuaternionTest, StaticFactories) {
    // Test Identity
    Quaternionf identity = Quaternionf::Identity();
    EXPECT_TRUE(isNear(identity, Quaternionf(0.0f, 0.0f, 0.0f, 1.0f)));
    
    // Test AxisAngle
    Vector3f axis(1.0f, 0.0f, 0.0f);
    float angle = M_PI / 4.0f;
    Quaternionf q1(axis, angle);
    Quaternionf q2 = Quaternionf::AxisAngle(axis, angle);
    EXPECT_TRUE(isNear(q1, q2));
    
    // Test EulerAngles
    float roll = M_PI / 6.0f, pitch = M_PI / 4.0f, yaw = M_PI / 3.0f;
    Quaternionf q3(roll, pitch, yaw);
    Quaternionf q4 = Quaternionf::EulerAngles(roll, pitch, yaw);
    EXPECT_TRUE(isNear(q3, q4));
}

TEST_F(QuaternionTest, ComparisonOperators) {
    Quaternionf q1(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternionf q2(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternionf q3(1.1f, 2.0f, 3.0f, 4.0f);
    
    EXPECT_TRUE(q1 == q2);
    EXPECT_FALSE(q1 == q3);
    EXPECT_FALSE(q1 != q2);
    EXPECT_TRUE(q1 != q3);
}

TEST_F(QuaternionTest, AssignmentOperators) {
    Quaternionf q1(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternionf q2(5.0f, 6.0f, 7.0f, 8.0f);
    
    // Test +=
    Quaternionf q3 = q1;
    q3 += q2;
    EXPECT_TRUE(isNear(q3, q1 + q2));
    
    // Test -=
    Quaternionf q4 = q2;
    q4 -= q1;
    EXPECT_TRUE(isNear(q4, q2 - q1));
    
    // Test *=
    Quaternionf q5 = q1;
    q5 *= 2.0f;
    EXPECT_TRUE(isNear(q5, q1 * 2.0f));
    
    // Test quaternion *=
    Quaternionf q6 = q1;
    q6 *= q2;
    EXPECT_TRUE(isNear(q6, q1 * q2));
}