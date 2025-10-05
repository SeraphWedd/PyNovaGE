#ifndef PYNOVAGE_MATH_VECTOR4_HPP
#define PYNOVAGE_MATH_VECTOR4_HPP

#include "simd_utils.hpp"
#include <cmath>

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

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_VECTOR4_HPP