#pragma once

#include <type_traits>
#include <array>
#include <cmath>
#include <algorithm>
#include <limits>

#include "../vectors/vector3.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PyNovaGE {

// Forward declarations
template<typename T> class Matrix3;
template<typename T> class Matrix4;

template<typename T>
class Quaternion {
public:
    // Ensure T is a floating point type
    static_assert(std::is_floating_point_v<T>, "Quaternion only supports floating point types");
    
    // Flat array storage for better cache locality and compiler optimization
    // Elements stored as [x, y, z, w] where w is the scalar component
    std::array<T, 4> data;
    
    // Constructors
    Quaternion() noexcept : data{{0, 0, 0, 1}} {} // Identity quaternion
    
    Quaternion(T x, T y, T z, T w) noexcept : data{{x, y, z, w}} {}
    
    explicit Quaternion(const Vector3<T>& axis, T angle) noexcept {
        T halfAngle = angle * T(0.5);
        T sinHalf = std::sin(halfAngle);
        T cosHalf = std::cos(halfAngle);
        
        Vector3<T> normalizedAxis = axis.normalized();
        data[0] = normalizedAxis[0] * sinHalf;  // x
        data[1] = normalizedAxis[1] * sinHalf;  // y
        data[2] = normalizedAxis[2] * sinHalf;  // z
        data[3] = cosHalf;                      // w
    }
    
    // Euler angles constructor (roll, pitch, yaw)
    explicit Quaternion(T roll, T pitch, T yaw) noexcept {
        T halfRoll = roll * T(0.5);
        T halfPitch = pitch * T(0.5);
        T halfYaw = yaw * T(0.5);
        
        T cr = std::cos(halfRoll);
        T sr = std::sin(halfRoll);
        T cp = std::cos(halfPitch);
        T sp = std::sin(halfPitch);
        T cy = std::cos(halfYaw);
        T sy = std::sin(halfYaw);
        
        data[0] = sr * cp * cy - cr * sp * sy; // x
        data[1] = cr * sp * cy + sr * cp * sy; // y
        data[2] = cr * cp * sy - sr * sp * cy; // z
        data[3] = cr * cp * cy + sr * sp * sy; // w
    }
    
    // Component access
    T& x() { return data[0]; }
    const T& x() const { return data[0]; }
    T& y() { return data[1]; }
    const T& y() const { return data[1]; }
    T& z() { return data[2]; }
    const T& z() const { return data[2]; }
    T& w() { return data[3]; }
    const T& w() const { return data[3]; }
    
    // Array-style access
    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }
    
    // Basic arithmetic operators
    Quaternion operator+(const Quaternion& other) const noexcept {
        return Quaternion{
            data[0] + other.data[0],
            data[1] + other.data[1],
            data[2] + other.data[2],
            data[3] + other.data[3]
        };
    }
    
    Quaternion operator-(const Quaternion& other) const noexcept {
        return Quaternion{
            data[0] - other.data[0],
            data[1] - other.data[1],
            data[2] - other.data[2],
            data[3] - other.data[3]
        };
    }
    
    Quaternion operator*(T scalar) const noexcept {
        return Quaternion{
            data[0] * scalar,
            data[1] * scalar,
            data[2] * scalar,
            data[3] * scalar
        };
    }
    
    // Quaternion multiplication (composition of rotations)
    Quaternion operator*(const Quaternion& other) const noexcept {
        return Quaternion{
            data[3] * other.data[0] + data[0] * other.data[3] + data[1] * other.data[2] - data[2] * other.data[1], // x
            data[3] * other.data[1] - data[0] * other.data[2] + data[1] * other.data[3] + data[2] * other.data[0], // y
            data[3] * other.data[2] + data[0] * other.data[1] - data[1] * other.data[0] + data[2] * other.data[3], // z
            data[3] * other.data[3] - data[0] * other.data[0] - data[1] * other.data[1] - data[2] * other.data[2]  // w
        };
    }
    
    // Vector rotation
    Vector3<T> operator*(const Vector3<T>& vec) const noexcept {
        // Optimized vector rotation using quaternion
        T x2 = data[0] * data[0];
        T y2 = data[1] * data[1];
        T z2 = data[2] * data[2];
        T xy = data[0] * data[1];
        T xz = data[0] * data[2];
        T yz = data[1] * data[2];
        T wx = data[3] * data[0];
        T wy = data[3] * data[1];
        T wz = data[3] * data[2];
        
        return Vector3<T>{
            vec[0] * (1 - 2 * (y2 + z2)) + vec[1] * 2 * (xy - wz) + vec[2] * 2 * (xz + wy),
            vec[0] * 2 * (xy + wz) + vec[1] * (1 - 2 * (x2 + z2)) + vec[2] * 2 * (yz - wx),
            vec[0] * 2 * (xz - wy) + vec[1] * 2 * (yz + wx) + vec[2] * (1 - 2 * (x2 + y2))
        };
    }
    
    // Assignment operators
    Quaternion& operator+=(const Quaternion& other) noexcept {
        data[0] += other.data[0];
        data[1] += other.data[1];
        data[2] += other.data[2];
        data[3] += other.data[3];
        return *this;
    }
    
    Quaternion& operator-=(const Quaternion& other) noexcept {
        data[0] -= other.data[0];
        data[1] -= other.data[1];
        data[2] -= other.data[2];
        data[3] -= other.data[3];
        return *this;
    }
    
    Quaternion& operator*=(T scalar) noexcept {
        data[0] *= scalar;
        data[1] *= scalar;
        data[2] *= scalar;
        data[3] *= scalar;
        return *this;
    }
    
    Quaternion& operator*=(const Quaternion& other) noexcept {
        *this = *this * other;
        return *this;
    }
    
    // Comparison operators
    bool operator==(const Quaternion& other) const noexcept {
        const T epsilon = std::numeric_limits<T>::epsilon() * T(10);
        return std::abs(data[0] - other.data[0]) < epsilon &&
               std::abs(data[1] - other.data[1]) < epsilon &&
               std::abs(data[2] - other.data[2]) < epsilon &&
               std::abs(data[3] - other.data[3]) < epsilon;
    }
    
    bool operator!=(const Quaternion& other) const noexcept {
        return !(*this == other);
    }
    
    // Magnitude and normalization
    T lengthSquared() const noexcept {
        return data[0] * data[0] + data[1] * data[1] + data[2] * data[2] + data[3] * data[3];
    }
    
    T length() const noexcept {
        return std::sqrt(lengthSquared());
    }
    
    Quaternion normalized() const noexcept {
        T len = length();
        if (len == T(0)) return *this;
        T invLen = T(1) / len;
        return Quaternion{
            data[0] * invLen,
            data[1] * invLen,
            data[2] * invLen,
            data[3] * invLen
        };
    }
    
    void normalize() noexcept {
        *this = normalized();
    }
    
    // Quaternion operations
    Quaternion conjugate() const noexcept {
        return Quaternion{-data[0], -data[1], -data[2], data[3]};
    }
    
    Quaternion inverse() const noexcept {
        T lenSq = lengthSquared();
        if (lenSq == T(0)) return *this;
        Quaternion conj = conjugate();
        T invLenSq = T(1) / lenSq;
        return conj * invLenSq;
    }
    
    T dot(const Quaternion& other) const noexcept {
        return data[0] * other.data[0] + data[1] * other.data[1] + 
               data[2] * other.data[2] + data[3] * other.data[3];
    }
    
    // Convert to rotation matrix (forward declared - implementation in separate file)
    Matrix3<T> toMatrix3() const noexcept;
    Matrix4<T> toMatrix4() const noexcept;
    
    // Convert to Euler angles (returns roll, pitch, yaw)
    Vector3<T> toEulerAngles() const noexcept {
        T roll, pitch, yaw;
        
        // Roll (x-axis rotation)
        T sinr_cosp = 2 * (data[3] * data[0] + data[1] * data[2]);
        T cosr_cosp = 1 - 2 * (data[0] * data[0] + data[1] * data[1]);
        roll = std::atan2(sinr_cosp, cosr_cosp);
        
        // Pitch (y-axis rotation)
        T sinp = 2 * (data[3] * data[1] - data[2] * data[0]);
        if (std::abs(sinp) >= 1)
            pitch = std::copysign(T(M_PI) / T(2), sinp); // Use 90 degrees if out of range
        else
            pitch = std::asin(sinp);
        
        // Yaw (z-axis rotation)
        T siny_cosp = 2 * (data[3] * data[2] + data[0] * data[1]);
        T cosy_cosp = 1 - 2 * (data[1] * data[1] + data[2] * data[2]);
        yaw = std::atan2(siny_cosp, cosy_cosp);
        
        return Vector3<T>{roll, pitch, yaw};
    }
    
    // Static factory methods
    static Quaternion Identity() noexcept {
        return Quaternion{};
    }
    
    static Quaternion AxisAngle(const Vector3<T>& axis, T angle) noexcept {
        return Quaternion{axis, angle};
    }
    
    static Quaternion EulerAngles(T roll, T pitch, T yaw) noexcept {
        return Quaternion{roll, pitch, yaw};
    }
    
    // FromMatrix3 method (forward declared - implementation in separate file)
    static Quaternion FromMatrix3(const Matrix3<T>& mat) noexcept;
    
    // Interpolation
    static Quaternion Lerp(const Quaternion& a, const Quaternion& b, T t) noexcept {
        return (a * (T(1) - t) + b * t).normalized();
    }
    
    static Quaternion Slerp(const Quaternion& a, const Quaternion& b, T t) noexcept {
        T dot = a.dot(b);
        
        // If the dot product is negative, the quaternions have opposite orientations.
        // In this case, slerp won't take the shorter path. Note that we use conjugate()
        // to reverse the rotation, which is cheaper than negating the quaternion.
        Quaternion b_adjusted = b;
        if (dot < T(0)) {
            b_adjusted = Quaternion{-b.data[0], -b.data[1], -b.data[2], -b.data[3]};
            dot = -dot;
        }
        
        const T DOT_THRESHOLD = T(0.9995);
        if (dot > DOT_THRESHOLD) {
            // If the inputs are too close for comfort, linearly interpolate
            return Lerp(a, b_adjusted, t);
        }
        
        T theta_0 = std::acos(dot);        // theta_0 = angle between input vectors
        T theta = theta_0 * t;             // theta = angle between a and result
        T sin_theta = std::sin(theta);     // compute this value only once
        T sin_theta_0 = std::sin(theta_0); // compute this value only once
        
        T s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
        T s1 = sin_theta / sin_theta_0;
        
        return (a * s0 + b_adjusted * s1).normalized();
    }
    
    // Utility methods
    Vector3<T> getAxis() const noexcept {
        T sinHalfAngle = std::sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
        if (sinHalfAngle == T(0)) {
            return Vector3<T>{1, 0, 0}; // Default axis
        }
        T invSinHalfAngle = T(1) / sinHalfAngle;
        return Vector3<T>{
            data[0] * invSinHalfAngle,
            data[1] * invSinHalfAngle,
            data[2] * invSinHalfAngle
        };
    }
    
    T getAngle() const noexcept {
        return T(2) * std::acos(std::clamp(std::abs(data[3]), T(0), T(1)));
    }
};

// Type aliases
using Quaternionf = Quaternion<float>;
using Quaterniond = Quaternion<double>;

} // namespace PyNovaGE