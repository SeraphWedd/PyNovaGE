#ifndef PYNOVAGE_MATH_VECTOR2_HPP
#define PYNOVAGE_MATH_VECTOR2_HPP

#include "simd_utils.hpp"
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <iosfwd>

namespace pynovage {
namespace math {

/**
 * @brief 2D vector class with SIMD-optimized operations
 * 
 * Represents a 2D vector with x and y components. All operations
 * are optimized using SIMD instructions where available.
 */
class Vector2 {
public:
    // Constructors
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float x_, float y_) : x(x_), y(y_) {}
    Vector2(const Vector2& other) = default;
    Vector2& operator=(const Vector2& other) = default;

    // Basic vector operations
    Vector2 operator+(const Vector2& other) const {
        Vector2 result;
        SimdUtils::Add2f(&x, &other.x, &result.x);
        return result;
    }

    Vector2 operator-(const Vector2& other) const {
        Vector2 result;
        SimdUtils::Subtract2f(&x, &other.x, &result.x);
        return result;
    }

    Vector2 operator*(float scalar) const {
        Vector2 result;
        float scalars[2] = {scalar, scalar};
        SimdUtils::Multiply2f(&x, scalars, &result.x);
        return result;
    }

    Vector2 operator/(float scalar) const {
        Vector2 result;
        float scalars[2] = {scalar, scalar};
        SimdUtils::Divide2f(&x, scalars, &result.x);
        return result;
    }

    // Compound assignment operators
    Vector2& operator+=(const Vector2& other) {
        SimdUtils::Add2f(&x, &other.x, &x);
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        SimdUtils::Subtract2f(&x, &other.x, &x);
        return *this;
    }

    Vector2& operator*=(float scalar) {
        float scalars[2] = {scalar, scalar};
        SimdUtils::Multiply2f(&x, scalars, &x);
        return *this;
    }

    Vector2& operator/=(float scalar) {
        float scalars[2] = {scalar, scalar};
        SimdUtils::Divide2f(&x, scalars, &x);
        return *this;
    }

    // Geometric operations
    float dot(const Vector2& other) const {
        return SimdUtils::DotProduct2f(&x, &other.x);
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

    Vector2 normalized() const {
        Vector2 result(*this);
        result.normalize();
        return result;
    }

    // Utility functions
    bool isZero() const {
        return x == 0.0f && y == 0.0f;
    }

    void setZero() {
        x = y = 0.0f;
    }

    // Distance and angle functions
    float distanceTo(const Vector2& other) const {
        return (*this - other).length();
    }

    float distanceSquaredTo(const Vector2& other) const {
        return (*this - other).lengthSquared();
    }

    float angleTo(const Vector2& other) const {
        float denom = length() * other.length();
        if (denom <= 0.0f) return 0.0f;
        float cosTheta = dot(other) / denom;
        cosTheta = std::min(1.0f, std::max(-1.0f, cosTheta));
        return std::acos(cosTheta);
    }

    // Component-wise operations
    Vector2 cwiseProduct(const Vector2& other) const {
        return Vector2(x * other.x, y * other.y);
    }

    Vector2 cwiseQuotient(const Vector2& other) const {
        return Vector2(x / other.x, y / other.y);
    }

    // String conversion
    std::string toString() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);
        ss << "(" << x << ", " << y << ")";
        return ss.str();
    }

    // Static utility functions
    static Vector2 zero() {
        return Vector2(0.0f, 0.0f);
    }

    static Vector2 one() {
        return Vector2(1.0f, 1.0f);
    }

    static Vector2 unitX() {
        return Vector2(1.0f, 0.0f);
    }

    static Vector2 unitY() {
        return Vector2(0.0f, 1.0f);
    }

    // Directional constants
    static Vector2 left() { return Vector2(-1.0f, 0.0f); }
    static Vector2 right() { return Vector2(1.0f, 0.0f); }
    static Vector2 up() { return Vector2(0.0f, 1.0f); }
    static Vector2 down() { return Vector2(0.0f, -1.0f); }

    // Linear interpolation
    static Vector2 lerp(const Vector2& a, const Vector2& b, float t) {
        return a + (b - a) * t;
    }

    // Unary operators
    Vector2 operator-() const {
        return Vector2(-x, -y);
    }

    // Component access by index
    float& operator[](int index) {
        if (index < 0 || index >= 2) {
            throw std::out_of_range("Vector2 index out of range");
        }
        return (&x)[index];
    }

    const float& operator[](int index) const {
        if (index < 0 || index >= 2) {
            throw std::out_of_range("Vector2 index out of range");
        }
        return (&x)[index];
    }

    // Component access
    float x;
    float y;
};

// Global operators
inline Vector2 operator*(float scalar, const Vector2& vec) {
    return vec * scalar;
}

// Comparison operators
inline bool operator==(const Vector2& lhs, const Vector2& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(const Vector2& lhs, const Vector2& rhs) {
    return !(lhs == rhs);
}

// Min/Max operations
inline math::Vector2 min(const math::Vector2& a, const math::Vector2& b) {
    return math::Vector2(std::min(a.x, b.x), std::min(a.y, b.y));
}

inline math::Vector2 max(const math::Vector2& a, const math::Vector2& b) {
    return math::Vector2(std::max(a.x, b.x), std::max(a.y, b.y));
}

// Stream operators
inline std::ostream& operator<<(std::ostream& os, const Vector2& v) {
    os << v.toString();
    return os;
}

inline std::istream& operator>>(std::istream& is, Vector2& v) {
    char dummy;
    is >> dummy >> v.x >> dummy >> v.y >> dummy;
    return is;
}

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_VECTOR2_HPP