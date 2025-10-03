#ifndef PYNOVAGE_MATH_VECTOR3_HPP
#define PYNOVAGE_MATH_VECTOR3_HPP

#include "math/simd_utils.hpp"
#include <cmath>

namespace pynovage {
namespace math {

/**
 * @brief 3D vector class with SIMD-optimized operations
 * 
 * Represents a 3D vector with x, y, and z components. All operations
 * are optimized using SIMD instructions where available.
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

    // Utility functions
    bool isZero() const {
        return x == 0.0f && y == 0.0f && z == 0.0f;
    }

    void setZero() {
        x = y = z = w = 0.0f;
    }

    // Static utility functions
    static Vector3 zero() {
        return Vector3(0.0f, 0.0f, 0.0f);
    }

    static Vector3 one() {
        return Vector3(1.0f, 1.0f, 1.0f);
    }

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

    // Component access
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

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_VECTOR3_HPP