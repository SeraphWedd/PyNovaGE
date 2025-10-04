#ifndef PYNOVAGE_MATH_VECTOR2_HPP
#define PYNOVAGE_MATH_VECTOR2_HPP

#include "simd_utils.hpp"
#include <cmath>

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

    // Component access
    float x;
    float y;
};

// Global operators
inline Vector2 operator*(float scalar, const Vector2& vec) {
    return vec * scalar;
}

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_VECTOR2_HPP