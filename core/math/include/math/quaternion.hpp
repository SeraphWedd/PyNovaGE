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
        return std::sqrt(w*w + x*x + y*y + z*z);
    }

    /**
     * @brief Returns the squared magnitude of the quaternion
     * @return Quaternion magnitude squared
     */
    float MagnitudeSquared() const {
        return w*w + x*x + y*y + z*z;
    }

    /**
     * @brief Normalizes the quaternion to unit length
     * @return Reference to normalized quaternion
     */
    Quaternion& Normalize() {
        const float mag = Magnitude();
        if (mag > 0.0f) {
            const float invMag = 1.0f / mag;
            w *= invMag;
            x *= invMag;
            y *= invMag;
            z *= invMag;
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
        return Quaternion(
            w*other.w - x*other.x - y*other.y - z*other.z,  // w
            w*other.x + x*other.w + y*other.z - z*other.y,  // x
            w*other.y - x*other.z + y*other.w + z*other.x,  // y
            w*other.z + x*other.y - y*other.x + z*other.w   // z
        );
    }

    /**
     * @brief Rotates a vector by this quaternion
     * @param v Vector to rotate
     * @return Rotated vector
     */
    Vector3 RotateVector(const Vector3& v) const {
        // First normalize the quaternion
        Quaternion normalized = this->Normalized();
        
        // Extract the vector part and scalar part
        Vector3 u(normalized.x, normalized.y, normalized.z);
        float s = normalized.w;
        
        // Apply the rotation formula: v' = 2.0f * dot(u,v) * u + (s*s - dot(u,u)) * v + 2.0f * s * cross(u,v)
        float dot_uv = u.x * v.x + u.y * v.y + u.z * v.z;
        float dot_uu = u.x * u.x + u.y * u.y + u.z * u.z;
        
        // Calculate cross product
        Vector3 cross(
            u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x
        );
        
        // Combine all terms
        return Vector3(
            2.0f * dot_uv * u.x + (s*s - dot_uu) * v.x + 2.0f * s * cross.x,
            2.0f * dot_uv * u.y + (s*s - dot_uu) * v.y + 2.0f * s * cross.y,
            2.0f * dot_uv * u.z + (s*s - dot_uu) * v.z + 2.0f * s * cross.z
        );
    }

    /**
     * @brief Extracts axis and angle from quaternion
     * @param axis Output parameter for rotation axis
     * @param angle Output parameter for rotation angle in radians
     */
    void ToAxisAngle(Vector3& axis, float& angle) const {
        // Normalize the quaternion first
        Quaternion normalized = this->Normalized();
        
        angle = 2.0f * std::acos(normalized.w);
        float s = std::sqrt(1.0f - normalized.w * normalized.w);
        
        if (s > 1e-6f) {
            axis.x = normalized.x / s;
            axis.y = normalized.y / s;
            axis.z = normalized.z / s;
        } else {
            // If s is close to zero, the angle is close to 0 or 180 degrees
            // In this case, any axis will do
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
    static Quaternion FromEulerAngles(float roll, float pitch, float yaw) {
        // Convert euler angles to quaternion using half angles
        float cr = std::cos(roll * 0.5f);
        float sr = std::sin(roll * 0.5f);
        float cp = std::cos(pitch * 0.5f);
        float sp = std::sin(pitch * 0.5f);
        float cy = std::cos(yaw * 0.5f);
        float sy = std::sin(yaw * 0.5f);

        return Quaternion(
            cr * cp * cy + sr * sp * sy,
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy
        );
    }

    /**
     * @brief Converts quaternion to euler angles
     * @param roll Output parameter for rotation around X axis (in radians)
     * @param pitch Output parameter for rotation around Y axis (in radians)
     * @param yaw Output parameter for rotation around Z axis (in radians)
     */
    void ToEulerAngles(float& roll, float& pitch, float& yaw) const {
        // Roll (x-axis rotation)
        float sinr_cosp = 2.0f * (w * x + y * z);
        float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
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
        float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
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

        // Calculate cosine of angle between quaternions
        float cosOmega = start.Dot(end);

        // If negative dot product, negate one of the quaternions to take shorter path
        Quaternion end2 = end;
        if (cosOmega < 0.0f) {
            end2 = Quaternion(-end.w, -end.x, -end.y, -end.z);
            cosOmega = -cosOmega;
        }

        // If very close, just lerp
        if (cosOmega > 0.9999f) {
            return Lerp(start, end2, t);
        }

        // Calculate interpolation parameters
        float omega = std::acos(cosOmega);
        float sinOmega = std::sin(omega);
        float scale0 = std::sin((1.0f - t) * omega) / sinOmega;
        float scale1 = std::sin(t * omega) / sinOmega;

        // Perform spherical linear interpolation
        return Quaternion(
            scale0 * start.w + scale1 * end2.w,
            scale0 * start.x + scale1 * end2.x,
            scale0 * start.y + scale1 * end2.y,
            scale0 * start.z + scale1 * end2.z
        );
    }
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_QUATERNION_HPP