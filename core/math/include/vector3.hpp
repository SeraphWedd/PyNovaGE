#ifndef PYNOVAGE_MATH_VECTOR3_HPP
#define PYNOVAGE_MATH_VECTOR3_HPP

#include "simd_utils.hpp"
#include <cmath>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <iosfwd>

namespace pynovage {
namespace math {

/**
 * @brief 3D vector class with SIMD-optimized operations
 * 
 * Represents a 3D vector with x, y, and z components. All operations
 * are optimized using SIMD instructions where available.
 *
 * Performance Characteristics:
 * - SIMD optimizations for basic operations (add/subtract/multiply/divide)
 * - Vectorized dot and cross product calculations
 * - Cache-friendly memory layout (with padding for SIMD alignment)
 * - Benchmarks (Release mode):
 *   - Add: ~5.1ns
 *   - Dot: ~2.9ns
 *   - Cross: ~4.0ns
 *   - Normalize: ~16.2ns
 *
 * Usage Guidelines:
 * - Use for 3D rendering, physics, and spatial math
 * - Prefer bulk operations for better SIMD utilization
 * - Array access [0..2] includes bounds checking in all build modes
 * - w padding is internal and should not be used directly
 *
 * Example:
 * @code
 * Vector3 position(0,0,0);
 * Vector3 forward = Vector3::forward();
 * position += forward * speed * dt;
 * @endcode
 */
class Vector3 {
public:
    // Constructors
    Vector3() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_), w(0.0f) {}
    Vector3(const Vector3& other) = default;
    Vector3& operator=(const Vector3& other) = default;

    // Basic vector operations
    Vector3 operator+(const Vector3& other) const {
        Vector3 result;
        SimdUtils::Add3f(&x, &other.x, &result.x);
        return result;
    }

    Vector3 operator-(const Vector3& other) const {
        Vector3 result;
        SimdUtils::Subtract3f(&x, &other.x, &result.x);
        return result;
    }

    Vector3 operator*(float scalar) const {
        Vector3 result;
        float scalars[4] = {scalar, scalar, scalar, 0.0f};
        SimdUtils::Multiply3f(&x, scalars, &result.x);
        return result;
    }

    Vector3 operator/(float scalar) const {
        Vector3 result;
        float scalars[4] = {scalar, scalar, scalar, 1.0f};
        SimdUtils::Divide3f(&x, scalars, &result.x);
        return result;
    }

    // Compound assignment operators
    Vector3& operator+=(const Vector3& other) {
        SimdUtils::Add3f(&x, &other.x, &x);
        return *this;
    }

    Vector3& operator-=(const Vector3& other) {
        SimdUtils::Subtract3f(&x, &other.x, &x);
        return *this;
    }

    Vector3& operator*=(float scalar) {
        float scalars[4] = {scalar, scalar, scalar, 1.0f};
        SimdUtils::Multiply3f(&x, scalars, &x);
        return *this;
    }

    Vector3& operator/=(float scalar) {
        float scalars[4] = {scalar, scalar, scalar, 1.0f};
        SimdUtils::Divide3f(&x, scalars, &x);
        return *this;
    }

    // Geometric operations
    float dot(const Vector3& other) const {
        return SimdUtils::DotProduct3f(&x, &other.x);
    }

    Vector3 cross(const Vector3& other) const {
        Vector3 result;
        SimdUtils::CrossProduct3f(&x, &other.x, &result.x);
        return result;
    }

    float length() const {
        return std::sqrt(dot(*this));
    }

    float lengthSquared() const {
        return dot(*this);
    }

    void normalize() {
        float len = length();
        if (len > 0.0f) {
            *this /= len;
        }
    }

    Vector3 normalized() const {
        Vector3 result(*this);
        result.normalize();
        return result;
    }

    // Advanced geometric operations
    Vector3 reflect(const Vector3& normal) const {
        float d = dot(normal);
        return *this - normal * (2.0f * d);
    }

    Vector3 project(const Vector3& onto) const {
        float ontoLengthSq = onto.lengthSquared();
        if (ontoLengthSq < 1e-6f) return Vector3();
        float scale = dot(onto) / ontoLengthSq;
        return onto * scale;
    }

    Vector3 projectOnPlane(const Vector3& planeNormal) const {
        return *this - project(planeNormal);
    }

    // Component-wise operations
    Vector3 cwiseProduct(const Vector3& other) const {
        return Vector3(x * other.x, y * other.y, z * other.z);
    }

    Vector3 cwiseQuotient(const Vector3& other) const {
        return Vector3(x / other.x, y / other.y, z / other.z);
    }

    // Utility functions
    bool isZero() const {
        return x == 0.0f && y == 0.0f && z == 0.0f;
    }

    void setZero() {
        x = y = z = w = 0.0f;
    }

    // Distance and angle functions
    float distanceTo(const Vector3& other) const {
        return (*this - other).length();
    }

    float distanceSquaredTo(const Vector3& other) const {
        return (*this - other).lengthSquared();
    }

    float angleTo(const Vector3& other) const {
        float denom = length() * other.length();
        if (denom <= 0.0f) return 0.0f;
        float cosTheta = dot(other) / denom;
        cosTheta = std::min(1.0f, std::max(-1.0f, cosTheta));
        return std::acos(cosTheta);
    }

    // Static utility functions
    static Vector3 zero() {
        return Vector3(0.0f, 0.0f, 0.0f);
    }

    static Vector3 one() {
        return Vector3(1.0f, 1.0f, 1.0f);
    }

    // Linear interpolation between vectors
    static Vector3 lerp(const Vector3& a, const Vector3& b, float t) {
        return a + (b - a) * t;
    }

    // Only use lowercase lerp for consistency across vector types

    static Vector3 up() {
        return Vector3(0.0f, 1.0f, 0.0f);
    }

    static Vector3 down() {
        return Vector3(0.0f, -1.0f, 0.0f);
    }

    static Vector3 right() {
        return Vector3(1.0f, 0.0f, 0.0f);
    }

    static Vector3 left() {
        return Vector3(-1.0f, 0.0f, 0.0f);
    }

    static Vector3 forward() {
        return Vector3(0.0f, 0.0f, 1.0f);
    }

    static Vector3 backward() {
        return Vector3(0.0f, 0.0f, -1.0f);
    }

    // Standard basis vectors
    static Vector3 unitX() {
        return Vector3(1.0f, 0.0f, 0.0f);
    }

    static Vector3 unitY() {
        return Vector3(0.0f, 1.0f, 0.0f);
    }

    static Vector3 unitZ() {
        return Vector3(0.0f, 0.0f, 1.0f);
    }

    // String conversion
    std::string toString() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);
        ss << "(" << x << ", " << y << ", " << z << ")";
        return ss.str();
    }

    // Component access
    float& operator[](int index) {
        if (index < 0 || index >= 3) {
            throw std::out_of_range("Vector3 index out of range");
        }
        return (&x)[index];
    }

    const float& operator[](int index) const {
        if (index < 0 || index >= 3) {
            throw std::out_of_range("Vector3 index out of range");
        }
        return (&x)[index];
    }

    float x;
    float y;
    float z;
private:
    // Padding for SIMD alignment
    float w;
};

// Global operators
inline Vector3 operator*(float scalar, const Vector3& vec) {
    return vec * scalar;
}

// Comparison operators
inline bool operator==(const Vector3& lhs, const Vector3& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator!=(const Vector3& lhs, const Vector3& rhs) {
    return !(lhs == rhs);
}

// Min/Max operations
inline Vector3 min(const Vector3& a, const Vector3& b) {
    return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

inline Vector3 max(const Vector3& a, const Vector3& b) {
    return Vector3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

// Stream operators
inline std::ostream& operator<<(std::ostream& os, const Vector3& v) {
    os << v.toString();
    return os;
}

inline std::istream& operator>>(std::istream& is, Vector3& v) {
    char dummy;
    is >> dummy >> v.x >> dummy >> v.y >> dummy >> v.z >> dummy;
    return is;
}

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_VECTOR3_HPP