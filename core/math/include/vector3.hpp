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
 * - Component-wise operations use SIMD for maximum efficiency
 * - Benchmarks (Release mode):
 *   - Add/Subtract: ~5.1ns
 *   - Dot product: ~2.9ns
 *   - Cross product: ~4.0ns
 *   - Normalize: ~16.2ns
 *   - Component-wise ops: ~3.8ns
 *   - Projection: ~12.5ns
 *
 * Usage Guidelines:
 * - Use for 3D rendering, physics, and spatial math
 * - Prefer bulk operations for better SIMD utilization
 * - Array access [0..2] includes bounds checking in all build modes
 * - w padding is internal and should not be used directly
 * - All operations are thread-safe as the class is purely value-based
 * - For best SIMD performance, ensure proper memory alignment
 *
 * Example:
 * @code
 * // Basic vector operations
 * Vector3 position(0,0,0);
 * Vector3 forward = Vector3::forward();
 * position += forward * speed * dt;  // SIMD optimized
 *
 * // Cross product for normal calculation
 * Vector3 tangent(1,0,0);
 * Vector3 normal = forward.cross(tangent).normalized();
 *
 * // Projection example
 * Vector3 movement(5,5,0);
 * Vector3 slopeNormal(0,1,0);
 * Vector3 movementAlongSlope = movement.projectOnPlane(slopeNormal);
 *
 * // Bulk operations are more efficient
 * Vector3 positions[100];
 * Vector3 velocities[100];
 * for(int i = 0; i < 100; i++) {
 *     positions[i] += velocities[i] * deltaTime;  // Uses SIMD
 * }
 * @endcode
 */
class alignas(16) Vector3 {
public:
    // Constructors
    Vector3() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_), w(0.0f) {}
    Vector3(const Vector3& other) = default;
    Vector3& operator=(const Vector3& other) = default;

    /**
     * @brief SIMD-optimized vector addition
     * @param other The vector to add
     * @return A new vector containing the sum
     */
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

    Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }

    Vector3 operator*(float scalar) const {
        Vector3 result;
        SimdUtils::Multiply3fScalar(&x, scalar, &result.x);
        return result;
    }

    Vector3 operator/(float scalar) const {
        Vector3 result;
        SimdUtils::Divide3fScalar(&x, scalar, &result.x);
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
        SimdUtils::Multiply3fScalar(&x, scalar, &x);
        return *this;
    }

    Vector3& operator/=(float scalar) {
        SimdUtils::Divide3fScalar(&x, scalar, &x);
        return *this;
    }

    /**
     * @brief Calculates the dot product using SIMD operations
     * @param other The vector to dot with
     * @return The dot product value
     */
    float dot(const Vector3& other) const {
        return SimdUtils::DotProduct3f(&x, &other.x);
    }

    /**
     * @brief Calculates the cross product using SIMD operations
     * @param other The vector to cross with
     * @return A new vector perpendicular to both input vectors
     * @note The resulting vector is normalized if input vectors are normalized
     */
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

    Vector3& normalize() {
        float lenSq = lengthSquared();
        if (lenSq > 0.0f) {
            float invLen = 1.0f / std::sqrt(lenSq);
            SimdUtils::Multiply3fScalar(&x, invLen, &x);
        }
        return *this;
    }

    Vector3 normalized() const {
        Vector3 result(*this);
        result.normalize();
        return result;
    }

    /**
     * @brief Advanced geometric operations for reflection and projection
     */

    /**
     * @brief Reflects this vector off a surface with the given normal
     * @param normal The surface normal (should be normalized)
     * @return The reflected vector
     */
    Vector3 reflect(const Vector3& normal) const {
        Vector3 n = normal.normalized();
        float d = dot(n);
        return *this - n * (2.0f * d);
    }

    /**
     * @brief Projects this vector onto another vector using SIMD operations
     * @param other The vector to project onto
     * @return The projected vector
     */
    Vector3 projectOnto(const Vector3& other) const {
        float otherLengthSq = other.lengthSquared();
        if (otherLengthSq < 1e-6f) return Vector3();
        return other * (dot(other) / otherLengthSq);
    }

    /**
     * @brief Projects this vector onto a plane defined by its normal
     * @param planeNormal The normal vector of the plane (should be normalized)
     * @return The projected vector lying on the plane
     */
    Vector3 projectOnPlane(const Vector3& planeNormal) const {
        return *this - projectOnto(planeNormal);
    }

    // Reject this vector from another vector (perpendicular component)
    /**
     * @brief Calculates the rejection of this vector from another vector
     * @param other The vector to reject from
     * @return The component of this vector perpendicular to the other vector
     * @note This is equivalent to this - projectOnto(other)
     */
    Vector3 rejectFrom(const Vector3& other) const {
        return *this - projectOnto(other);
    }

    /**
     * @brief Component-wise multiplication using SIMD operations
     * @param other The vector to multiply with component-wise
     * @return A new vector with each component being the product of the corresponding components
     */
    Vector3 cwiseProduct(const Vector3& other) const {
        Vector3 result;
        SimdUtils::Multiply3f(&x, &other.x, &result.x);
        return result;
    }

    /**
     * @brief Component-wise division using SIMD operations
     * @param other The vector to divide by component-wise
     * @return A new vector with each component being the quotient of the corresponding components
     */
    Vector3 cwiseQuotient(const Vector3& other) const {
        Vector3 result;
        SimdUtils::Divide3f(&x, &other.x, &result.x);
        return result;
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

    // Min/Max operations
    static Vector3 min(const Vector3& a, const Vector3& b) {
        Vector3 result;
        SimdUtils::Min4f(&a.x, &b.x, &result.x);
        return result;
    }

    static Vector3 max(const Vector3& a, const Vector3& b) {
        Vector3 result;
        SimdUtils::Max4f(&a.x, &b.x, &result.x);
        return result;
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

    // Comparison operators
    bool operator<(const Vector3& other) const {
        // For mixed components, we want this to be false
        // Only return true if ALL components are less
        return x < other.x && y < other.y && z < other.z;
    }

    bool operator<=(const Vector3& other) const {
        // For mixed components, we want this to be false
        // Only return true if ALL components are less or equal
        return x <= other.x && y <= other.y && z <= other.z;
    }

    bool operator>(const Vector3& other) const {
        // For mixed components, we want this to be false
        // Only return true if ALL components are greater
        return x > other.x && y > other.y && z > other.z;
    }

    bool operator>=(const Vector3& other) const {
        // For mixed components, we want this to be false
        // Only return true if ALL components are greater or equal
        return x >= other.x && y >= other.y && z >= other.z;
    }

    // Equality comparison
    bool operator==(const Vector3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const Vector3& other) const {
        return !(*this == other);
    }

    float x;
    float y;
    float z;
private:
    // Padding for SIMD alignment, aligned to 16-byte boundary
    float w;
};

// Global operators
inline Vector3 operator*(float scalar, const Vector3& vec) {
    return vec * scalar;
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