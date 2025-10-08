#ifndef PYNOVAGE_MATH_QUATERNION_HPP
#define PYNOVAGE_MATH_QUATERNION_HPP

#include "simd_utils.hpp"
#include "vector3.hpp"
#include "math_constants.hpp"

namespace pynovage {
namespace math {

/**
 * @brief A SIMD-optimized quaternion class for representing 3D rotations
 * 
 * Quaternions are represented as q = w + xi + yj + zk where:
 * - w is the scalar (real) component
 * - (x,y,z) form the vector (imaginary) component
 * 
 * This implementation focuses on efficient rotation operations and
 * uses SIMD optimization where available.
 */
class Quaternion {
public:
    // For SIMD alignment
    union {
        struct {
            float w;  // Scalar component
            float x;  // First vector component
            float y;  // Second vector component
            float z;  // Third vector component
        };
        alignas(16) float data[4];  // Raw array access for SIMD operations
    };

    /**
     * @brief Default constructor, creates identity quaternion (no rotation)
     */
    Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}

    /**
     * @brief Constructs quaternion from components
     * @param w Scalar component
     * @param x First vector component
     * @param y Second vector component
     * @param z Third vector component
     */
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    /**
     * @brief Copy constructor
     * @param other Quaternion to copy from
     */
    Quaternion(const Quaternion& other) = default;

    /**
     * @brief Constructs quaternion from axis-angle representation
     * @param axis Unit vector representing rotation axis
     * @param angle Rotation angle in radians
     */
    Quaternion(const Vector3& axis, float angle) {
        const float halfAngle = angle * 0.5f;
        const float sinHalfAngle = std::sin(halfAngle);
        w = std::cos(halfAngle);
        x = axis.x * sinHalfAngle;
        y = axis.y * sinHalfAngle;
        z = axis.z * sinHalfAngle;
    }

    /**
     * @brief Creates an identity quaternion (no rotation)
     * @return Identity quaternion
     */
    static Quaternion Identity() {
        return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
    }

    /**
     * @brief Assignment operator
     * @param other Quaternion to copy from
     * @return Reference to this quaternion
     */
    Quaternion& operator=(const Quaternion& other) = default;

    /**
     * @brief Checks if two quaternions are equal within epsilon
     * @param other Quaternion to compare with
     * @return true if quaternions are equal, false otherwise
     */
    bool operator==(const Quaternion& other) const {
        const float epsilon = 1e-6f;
        return std::abs(w - other.w) < epsilon &&
               std::abs(x - other.x) < epsilon &&
               std::abs(y - other.y) < epsilon &&
               std::abs(z - other.z) < epsilon;
    }

    /**
     * @brief Checks if two quaternions are not equal within epsilon
     * @param other Quaternion to compare with
     * @return true if quaternions are not equal, false otherwise
     */
    bool operator!=(const Quaternion& other) const {
        return !(*this == other);
    }

    /**
     * @brief Returns the conjugate of this quaternion (w, -x, -y, -z)
     * @return Conjugate quaternion
     */
    Quaternion Conjugate() const {
        return Quaternion(w, -x, -y, -z);
    }

    /**
     * @brief Returns the magnitude (length) of the quaternion
     * @return Quaternion magnitude
     */
    float Magnitude() const {
        return std::sqrt(MagnitudeSquared());
    }

    /**
     * @brief Returns the squared magnitude of the quaternion
     * @return Quaternion magnitude squared
     */
    float MagnitudeSquared() const {
        return SimdUtils::DotProduct4f(data, data);
    }

    /**
     * @brief Normalizes the quaternion to unit length using fast SIMD operations
     * @return Reference to normalized quaternion
     */
    Quaternion& Normalize() {
        float magSq = MagnitudeSquared();
        if (std::abs(magSq - 1.0f) > 1e-12f && magSq > 1e-12f) {
            float invMag = 1.0f / std::sqrt(magSq);
            SimdUtils::Multiply4fScalar(data, invMag, data);
        }
        return *this;
    }

    /**
     * @brief Returns a normalized copy of this quaternion
     * @return Normalized quaternion
     */
    Quaternion Normalized() const {
        Quaternion result(*this);
        return result.Normalize();
    }

    /**
     * @brief Calculate dot product with another quaternion
     * @param other Quaternion to calculate dot product with
     * @return Dot product result
     */
    float Dot(const Quaternion& other) const {
        return w*other.w + x*other.x + y*other.y + z*other.z;
    }

    /**
     * @brief Returns the inverse of this quaternion
     * @return Inverse quaternion
     */
    Quaternion Inverse() const {
        const float magnitudeSq = MagnitudeSquared();
        if (magnitudeSq > 0.0f) {
            const float invMagnitudeSq = 1.0f / magnitudeSq;
            return Quaternion(w * invMagnitudeSq,
                            -x * invMagnitudeSq,
                            -y * invMagnitudeSq,
                            -z * invMagnitudeSq);
        }
        return *this;
    }

    /**
     * @brief Quaternion multiplication operator (rotation composition)
     * @param other Quaternion to multiply with
     * @return Result of multiplication
     */
    Quaternion operator*(const Quaternion& other) const {
        // Special case: if either quaternion is identity, return the other one
        if (other.w == 1.0f && other.x == 0.0f && other.y == 0.0f && other.z == 0.0f) {
            return *this;
        }
        if (w == 1.0f && x == 0.0f && y == 0.0f && z == 0.0f) {
            return other;
        }

        // Regular quaternion multiplication
        float result_w = w * other.w - x * other.x - y * other.y - z * other.z;
        float result_x = w * other.x + x * other.w + y * other.z - z * other.y;
        float result_y = w * other.y - x * other.z + y * other.w + z * other.x;
        float result_z = w * other.z + x * other.y - y * other.x + z * other.w;

        // Normalize only if the quaternions aren't already normalized
        Quaternion result(result_w, result_x, result_y, result_z);
        float mag = result.MagnitudeSquared();
        if (std::abs(mag - 1.0f) > 1e-6f) {
            result.Normalize();
        }
        return result;
    }

    /**
     * @brief Rotates a vector by this quaternion
     * @param v Vector to rotate
     * @return Rotated vector
     */
    Vector3 RotateVector(const Vector3& v) const {
        // We'll store the quaternion vector part and input vector as 4D vectors for SIMD
        float qvec[4] = {x, y, z, 0.0f};
        float vec[4] = {v.x, v.y, v.z, 0.0f};
        float result[4], temp1[4], temp2[4], temp3[4];
        
        // Calculate dot(qvec, vec) using SIMD
        float dot_qv = SimdUtils::DotProduct3f(qvec, vec);
        
        // Calculate dot(qvec, qvec)
        float dot_qq = SimdUtils::DotProduct3f(qvec, qvec);
        
        // Calculate cross product using SIMD-optimized operation
        float cross[4];
        SimdUtils::CrossProduct3f(qvec, vec, cross);
        
        // Prepare scalar multipliers
        SimdUtils::Fill4f(temp1, 2.0f * dot_qv);  // 2 * dot(qvec,vec)
        SimdUtils::Fill4f(temp2, w*w - dot_qq);   // w^2 - dot(qvec,qvec)
        SimdUtils::Fill4f(temp3, 2.0f * w);      // 2w for cross product term
        
        // Calculate 2 * dot(qvec,vec) * qvec
        SimdUtils::Multiply4f(temp1, qvec, result);
        
        // Add (w^2 - dot(qvec,qvec)) * vec
        float scaled_vec[4];
        SimdUtils::Multiply4f(temp2, vec, scaled_vec);
        SimdUtils::Add4f(result, scaled_vec, result);
        
        // Add 2w * cross(qvec,vec)
        float scaled_cross[4];
        SimdUtils::Multiply4f(temp3, cross, scaled_cross);
        SimdUtils::Add4f(result, scaled_cross, result);
        
        return Vector3(result[0], result[1], result[2]);
    }

    /**
     * @brief Extracts axis and angle from quaternion
     * @param axis Output parameter for rotation axis
     * @param angle Output parameter for rotation angle in radians
     */
    void ToAxisAngle(Vector3& axis, float& angle) const {
        // Use SIMD to calculate squared norm of vector part
        float vec_part[4] = {x, y, z, 0.0f};
        float s_squared = SimdUtils::DotProduct3f(vec_part, vec_part);
        
        // Calculate angle
        angle = 2.0f * std::acos(w);
        float s = std::sqrt(s_squared);
        
        if (s > 1e-6f) {
            // Use SIMD to divide vector part by s
            float result[4];
            SimdUtils::Divide3fScalar(vec_part, s, result);
            axis.x = result[0];
            axis.y = result[1];
            axis.z = result[2];
        } else {
            // If s is close to zero, return default axis
            axis.x = 1.0f;
            axis.y = 0.0f;
            axis.z = 0.0f;
        }
    }

    /**
     * @brief Creates a quaternion from euler angles
     * @param roll Rotation around X axis (in radians)
     * @param pitch Rotation around Y axis (in radians)
     * @param yaw Rotation around Z axis (in radians)
     * @return Quaternion representing the euler angles
     */
    /**
     * @brief Creates a quaternion from an orthonormal basis
     * @param forward Forward direction (Z axis)
     * @param up Up direction (Y axis)
     * @param right Right direction (X axis)
     * @return Quaternion representing the orientation defined by the basis
     */
    static Quaternion fromBasis(const Vector3& forward, const Vector3& up, const Vector3& right) {
        // Ensure vectors are normalized
        Vector3 f = forward.normalized();
        Vector3 u = up.normalized();
        Vector3 r = right.normalized();
        
        // Construct rotation matrix from basis vectors
        float m[3][3] = {
            {r.x, r.y, r.z},
            {u.x, u.y, u.z},
            {f.x, f.y, f.z}
        };
        
        float trace = m[0][0] + m[1][1] + m[2][2];
        Quaternion q;
        
        if (trace > 0.0f) {
            float s = 0.5f / std::sqrt(trace + 1.0f);
            q.w = 0.25f / s;
            q.x = (m[2][1] - m[1][2]) * s;
            q.y = (m[0][2] - m[2][0]) * s;
            q.z = (m[1][0] - m[0][1]) * s;
        } else if (m[0][0] > m[1][1] && m[0][0] > m[2][2]) {
            float s = 2.0f * std::sqrt(1.0f + m[0][0] - m[1][1] - m[2][2]);
            q.w = (m[2][1] - m[1][2]) / s;
            q.x = 0.25f * s;
            q.y = (m[0][1] + m[1][0]) / s;
            q.z = (m[0][2] + m[2][0]) / s;
        } else if (m[1][1] > m[2][2]) {
            float s = 2.0f * std::sqrt(1.0f + m[1][1] - m[0][0] - m[2][2]);
            q.w = (m[0][2] - m[2][0]) / s;
            q.x = (m[0][1] + m[1][0]) / s;
            q.y = 0.25f * s;
            q.z = (m[1][2] + m[2][1]) / s;
        } else {
            float s = 2.0f * std::sqrt(1.0f + m[2][2] - m[0][0] - m[1][1]);
            q.w = (m[1][0] - m[0][1]) / s;
            q.x = (m[0][2] + m[2][0]) / s;
            q.y = (m[1][2] + m[2][1]) / s;
            q.z = 0.25f * s;
        }
        
        return q.Normalized();
    }
    
    static Quaternion FromEulerAngles(float roll, float pitch, float yaw) {
        // Convert to radians and compute half angles
        float cr = std::cos(roll * 0.5f);
        float sr = std::sin(roll * 0.5f);
        float cp = std::cos(pitch * 0.5f);
        float sp = std::sin(pitch * 0.5f);
        float cy = std::cos(yaw * 0.5f);
        float sy = std::sin(yaw * 0.5f);

        // Compute quaternion components
        float w = cr * cp * cy + sr * sp * sy;
        float x = sr * cp * cy - cr * sp * sy;
        float y = cr * sp * cy + sr * cp * sy;
        float z = cr * cp * sy - sr * sp * cy;

        return Quaternion(w, x, y, z).Normalize();
    }

    /**
     * @brief Converts quaternion to euler angles
     * @param roll Output parameter for rotation around X axis (in radians)
     * @param pitch Output parameter for rotation around Y axis (in radians)
     * @param yaw Output parameter for rotation around Z axis (in radians)
     */
    void ToEulerAngles(float& roll, float& pitch, float& yaw) const {
        // Use SIMD to compute all terms at once
        float squares[4];
        SimdUtils::Multiply4f(data, data, squares);

        // Roll (x-axis rotation)
        float sinr_cosp = 2.0f * (w * x + y * z);
        float cosr_cosp = 1.0f - 2.0f * (squares[1] + squares[2]); // 1 - 2(x² + y²)
        roll = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        float sinp = 2.0f * (w * y - z * x);
        if (std::abs(sinp) >= 1.0f) {
            pitch = std::copysign(constants::half_pi, sinp);
        } else {
            pitch = std::asin(sinp);
        }

        // Yaw (z-axis rotation)
        float siny_cosp = 2.0f * (w * z + x * y);
        float cosy_cosp = 1.0f - 2.0f * (squares[2] + squares[3]); // 1 - 2(y² + z²)
        yaw = std::atan2(siny_cosp, cosy_cosp);
    }

    /**
     * @brief Performs linear interpolation between two quaternions
     * @param start Start quaternion
     * @param end End quaternion
     * @param t Interpolation parameter [0,1]
     * @return Interpolated quaternion
     */
    static Quaternion Lerp(const Quaternion& start, const Quaternion& end, float t) {
        // Clamp t to [0,1]
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);

        // Linear interpolation
        Quaternion result(
            start.w * (1.0f - t) + end.w * t,
            start.x * (1.0f - t) + end.x * t,
            start.y * (1.0f - t) + end.y * t,
            start.z * (1.0f - t) + end.z * t
        );

        // Normalize the result
        return result.Normalized();
    }

    /**
     * @brief Performs spherical linear interpolation between two quaternions
     * @param start Start quaternion
     * @param end End quaternion
     * @param t Interpolation parameter [0,1]
     * @return Interpolated quaternion
     */
    static Quaternion Slerp(const Quaternion& start, const Quaternion& end, float t) {
        // Clamp t to [0,1]
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);

        // Ensure input quaternions are normalized
        Quaternion q1 = start.Normalized();
        Quaternion q2 = end.Normalized();

        // Calculate cosine of angle between quaternions
        float cosOmega = q1.Dot(q2);

        // If negative dot product, negate one of the quaternions to take shorter path
        if (cosOmega < 0.0f) {
            q2 = Quaternion(-q2.w, -q2.x, -q2.y, -q2.z);
            cosOmega = -cosOmega;
        }

        // If very close, just lerp
        if (cosOmega > 0.9999f) {
            return Lerp(q1, q2, t).Normalize();
        }

        // Calculate interpolation parameters
        float omega = std::acos(cosOmega);
        float sinOmega = std::sin(omega);
        float scale0 = std::sin((1.0f - t) * omega) / sinOmega;
        float scale1 = std::sin(t * omega) / sinOmega;

        // Perform spherical linear interpolation
        return Quaternion(
            scale0 * q1.w + scale1 * q2.w,
            scale0 * q1.x + scale1 * q2.x,
            scale0 * q1.y + scale1 * q2.y,
            scale0 * q1.z + scale1 * q2.z
        ).Normalize();
    }
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_QUATERNION_HPP