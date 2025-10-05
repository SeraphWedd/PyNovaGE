#ifndef PYNOVAGE_MATH_VECTOR4_HPP
#define PYNOVAGE_MATH_VECTOR4_HPP

#include "simd_utils.hpp"
#include <cmath>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iosfwd>

namespace pynovage {
namespace math {

/**
 * @brief 4D vector class with SIMD-optimized operations
 * 
 * Represents a 4D vector with x, y, z, and w components. All operations
 * are optimized using SIMD instructions where available. Primarily used
 * for homogeneous coordinates in 3D transformations.
 */
class Vector4 {
public:
    // Constructors
    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    Vector4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vector4(const Vector4& other) = default;
    Vector4& operator=(const Vector4& other) = default;

    // Component access by index with bounds checking
    float& operator[](int index) {
#ifndef NDEBUG
        if (index < 0 || index >= 4) {
            throw std::out_of_range("Vector4 index out of range");
        }
#endif
        return (&x)[index];
    }

    const float& operator[](int index) const {
#ifndef NDEBUG
        if (index < 0 || index >= 4) {
            throw std::out_of_range("Vector4 index out of range");
        }
#endif
        return (&x)[index];
    }

    // Basic vector operations
    Vector4 operator+(const Vector4& other) const {
        Vector4 result;
        SimdUtils::Add4f(&x, &other.x, &result.x);
        return result;
    }

    Vector4 operator-(const Vector4& other) const {
        Vector4 result;
        SimdUtils::Subtract4f(&x, &other.x, &result.x);
        return result;
    }

    Vector4 operator*(float scalar) const {
        Vector4 result;
        float scalars[4] = {scalar, scalar, scalar, scalar};
        SimdUtils::Multiply4f(&x, scalars, &result.x);
        return result;
    }

    Vector4 operator/(float scalar) const {
        Vector4 result;
        float scalars[4] = {scalar, scalar, scalar, scalar};
        SimdUtils::Divide4f(&x, scalars, &result.x);
        return result;
    }

    // Compound assignment operators
    Vector4& operator+=(const Vector4& other) {
        SimdUtils::Add4f(&x, &other.x, &x);
        return *this;
    }

    Vector4& operator-=(const Vector4& other) {
        SimdUtils::Subtract4f(&x, &other.x, &x);
        return *this;
    }

    Vector4& operator*=(float scalar) {
        float scalars[4] = {scalar, scalar, scalar, scalar};
        SimdUtils::Multiply4f(&x, scalars, &x);
        return *this;
    }

    Vector4& operator/=(float scalar) {
        float scalars[4] = {scalar, scalar, scalar, scalar};
        SimdUtils::Divide4f(&x, scalars, &x);
        return *this;
    }

    // Geometric operations
    float dot(const Vector4& other) const {
        return SimdUtils::DotProduct4f(&x, &other.x);
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

    Vector4 normalized() const {
        Vector4 result(*this);
        result.normalize();
        return result;
    }

    // Utility functions
    bool isZero() const {
        return x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f;
    }

    void setZero() {
        x = y = z = w = 0.0f;
    }

    // Homogeneous coordinate operations
    void makePoint() {
        w = 1.0f;
    }

    void makeVector() {
        w = 0.0f;
    }

    bool isPoint() const {
        return w == 1.0f;
    }

    bool isVector() const {
        return w == 0.0f;
    }

    // Static utility functions
    static Vector4 zero() {
        return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    static Vector4 one() {
        return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Directional constants
    static Vector4 up() { return Vector4(0.0f, 1.0f, 0.0f, 0.0f); }
    static Vector4 down() { return Vector4(0.0f, -1.0f, 0.0f, 0.0f); }
    static Vector4 right() { return Vector4(1.0f, 0.0f, 0.0f, 0.0f); }
    static Vector4 left() { return Vector4(-1.0f, 0.0f, 0.0f, 0.0f); }
    static Vector4 forward() { return Vector4(0.0f, 0.0f, 1.0f, 0.0f); }
    static Vector4 backward() { return Vector4(0.0f, 0.0f, -1.0f, 0.0f); }

    // Standard basis vectors
    static Vector4 unitX() { return Vector4(1.0f, 0.0f, 0.0f, 0.0f); }
    static Vector4 unitY() { return Vector4(0.0f, 1.0f, 0.0f, 0.0f); }
    static Vector4 unitZ() { return Vector4(0.0f, 0.0f, 1.0f, 0.0f); }
    static Vector4 unitW() { return Vector4(0.0f, 0.0f, 0.0f, 1.0f); }

    // Component-wise operations
    Vector4 cwiseProduct(const Vector4& other) const {
        Vector4 result;
        SimdUtils::Multiply4f(&x, &other.x, &result.x);
        return result;
    }

    Vector4 cwiseQuotient(const Vector4& other) const {
        Vector4 result;
        SimdUtils::Divide4f(&x, &other.x, &result.x);
        return result;
    }

    // Comparison operators
    bool operator==(const Vector4& other) const {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    bool operator!=(const Vector4& other) const {
        return !(*this == other);
    }

    // Linear interpolation
    static Vector4 lerp(const Vector4& a, const Vector4& b, float t) {
        return a + (b - a) * t;
    }

    // Distance functions
    float distanceTo(const Vector4& other) const {
        return (*this - other).length();
    }

    float distanceSquaredTo(const Vector4& other) const {
        return (*this - other).lengthSquared();
    }

    // Angle between vectors
    float angleTo(const Vector4& other) const {
        float cosTheta = dot(other) / (length() * other.length());
        // Clamp to avoid numerical inaccuracies
        cosTheta = std::min(1.0f, std::max(-1.0f, cosTheta));
        return std::acos(cosTheta);
    }

    // 3D cross product (ignores w component)
    Vector4 cross(const Vector4& other) const {
        return Vector4(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x,
            0.0f  // Result is a vector, not a point
        );
    }

    // Project this vector onto another vector
    Vector4 projectOnto(const Vector4& other) const {
        float otherLengthSq = other.lengthSquared();
        if (otherLengthSq < 1e-6f) return Vector4();
        return other * (dot(other) / otherLengthSq);
    }

    // Reject this vector from another vector (perpendicular component)
    Vector4 rejectFrom(const Vector4& other) const {
        return *this - projectOnto(other);
    }

    // Min/Max operations
    static Vector4 min(const Vector4& a, const Vector4& b) {
        return Vector4(
            std::min(a.x, b.x),
            std::min(a.y, b.y),
            std::min(a.z, b.z),
            std::min(a.w, b.w)
        );
    }

    static Vector4 max(const Vector4& a, const Vector4& b) {
        return Vector4(
            std::max(a.x, b.x),
            std::max(a.y, b.y),
            std::max(a.z, b.z),
            std::max(a.w, b.w)
        );
    }

    // String conversion
    std::string toString() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);
        ss << "(" << x << ", " << y << ", " << z << ", " << w << ")";
        return ss.str();
    }

    // Component access
    float x;
    float y;
    float z;
    float w;
};

// Global operators
inline Vector4 operator*(float scalar, const Vector4& vec) {
    return vec * scalar;
}

// Stream operators
inline std::ostream& operator<<(std::ostream& os, const Vector4& v) {
    os << v.toString();
    return os;
}

inline std::istream& operator>>(std::istream& is, Vector4& v) {
    char dummy;
    is >> dummy >> v.x >> dummy >> v.y >> dummy >> v.z >> dummy >> v.w >> dummy;
    return is;
}

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_VECTOR4_HPP