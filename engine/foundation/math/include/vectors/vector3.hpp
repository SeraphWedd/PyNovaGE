#pragma once

#include "../simd/vector_ops.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)  // nameless struct/union warning
#pragma warning(disable: 4458)  // declaration hides class member
#endif

namespace nova {

template<typename T>
class Vector3 {
public:
    union {
        struct { T x, y, z; };
        struct { T r, g, b; };
        struct { T u, v, w; };
        T data[3];
    };

    // Constructors
    constexpr Vector3() : x(T(0)), y(T(0)), z(T(0)) {}
    constexpr Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
    constexpr explicit Vector3(T v) : x(v), y(v), z(v) {}
    
    // Array subscript operator
    constexpr T& operator[](size_t i) { return data[i]; }
    constexpr const T& operator[](size_t i) const { return data[i]; }
    
    // Basic arithmetic operators
    constexpr Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    constexpr Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    constexpr Vector3 operator*(const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
    constexpr Vector3 operator/(const Vector3& v) const { return Vector3(x / v.x, y / v.y, z / v.z); }
    
    constexpr Vector3 operator+(T s) const { return Vector3(x + s, y + s, z + s); }
    constexpr Vector3 operator-(T s) const { return Vector3(x - s, y - s, z - s); }
    constexpr Vector3 operator*(T s) const { return Vector3(x * s, y * s, z * s); }
    constexpr Vector3 operator/(T s) const { return Vector3(x / s, y / s, z / s); }
    
    // Assignment operators
    constexpr Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    constexpr Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    constexpr Vector3& operator*=(const Vector3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    constexpr Vector3& operator/=(const Vector3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
    
    constexpr Vector3& operator+=(T s) { x += s; y += s; z += s; return *this; }
    constexpr Vector3& operator-=(T s) { x -= s; y -= s; z -= s; return *this; }
    constexpr Vector3& operator*=(T s) { x *= s; y *= s; z *= s; return *this; }
    constexpr Vector3& operator/=(T s) { x /= s; y /= s; z /= s; return *this; }
    
    // Comparison operators
    constexpr bool operator==(const Vector3& v) const { return x == v.x && y == v.y && z == v.z; }
    constexpr bool operator!=(const Vector3& v) const { return !(*this == v); }
    
    // Lexicographical ordering (for containers like std::map, std::set)
    constexpr bool operator<(const Vector3& v) const { 
        return (x < v.x) || 
               (x == v.x && y < v.y) || 
               (x == v.x && y == v.y && z < v.z); 
    }
    constexpr bool operator>(const Vector3& v) const { return v < *this; }
    constexpr bool operator<=(const Vector3& v) const { return !(v < *this); }
    constexpr bool operator>=(const Vector3& v) const { return !(*this < v); }
    
    // Magnitude comparison (comparing lengths)
    constexpr bool isLongerThan(const Vector3& v) const { return lengthSquared() > v.lengthSquared(); }
    constexpr bool isShorterThan(const Vector3& v) const { return lengthSquared() < v.lengthSquared(); }
    constexpr bool isLongerThanOrEqual(const Vector3& v) const { return lengthSquared() >= v.lengthSquared(); }
    constexpr bool isShorterThanOrEqual(const Vector3& v) const { return lengthSquared() <= v.lengthSquared(); }
    
    // Unary operators
    constexpr Vector3 operator-() const { return Vector3(-x, -y, -z); }
    
    // Common vector operations
    constexpr T dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vector3 cross(const Vector3& v) const {
        return Vector3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
    constexpr T lengthSquared() const { return dot(*this); }
    T length() const { return std::sqrt(lengthSquared()); }
    
    Vector3 normalized() const {
        T len = length();
        return len > T(0) ? *this / len : *this;
    }
    
    void normalize() {
        T len = length();
        if (len > T(0)) {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    // Data access
    constexpr T* getData() { return data; }
    constexpr const T* getData() const { return data; }
    static constexpr size_t size() { return 3; }
};

// Common type aliases
using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;
using Vector3i = Vector3<int>;

} // namespace nova

#ifdef _MSC_VER
#pragma warning(pop)
#endif
