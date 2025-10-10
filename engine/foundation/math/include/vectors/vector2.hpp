#pragma once

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Undefine Windows min/max macros if already defined
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "../simd/vector_ops.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)  // nameless struct/union warning
#pragma warning(disable: 4458)  // declaration hides class member
#endif

namespace PyNovaGE {

template<typename T>
class Vector2 {
public:
    union {
        struct { T x, y; };
        struct { T u, v; };
        struct { T r, g; };
        T data[2];
    };

    // Constructors
    constexpr Vector2() : x(T(0)), y(T(0)) {}
    constexpr Vector2(T x, T y) : x(x), y(y) {}
    constexpr explicit Vector2(T v) : x(v), y(v) {}
    
    // Array subscript operator
    constexpr T& operator[](size_t i) { return (&x)[i]; }
    constexpr const T& operator[](size_t i) const { return (&x)[i]; }
    
    // Basic arithmetic operators
    constexpr Vector2 operator+(const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
    constexpr Vector2 operator-(const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
    constexpr Vector2 operator*(const Vector2& v) const { return Vector2(x * v.x, y * v.y); }
    constexpr Vector2 operator/(const Vector2& v) const { return Vector2(x / v.x, y / v.y); }
    
    constexpr Vector2 operator+(T s) const { return Vector2(x + s, y + s); }
    constexpr Vector2 operator-(T s) const { return Vector2(x - s, y - s); }
    constexpr Vector2 operator*(T s) const { return Vector2(x * s, y * s); }
    constexpr Vector2 operator/(T s) const { 
        return (s != T(0)) ? Vector2(x / s, y / s) : Vector2(x, y);
    }
    
    // Assignment operators
    constexpr Vector2& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
    constexpr Vector2& operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
    constexpr Vector2& operator*=(const Vector2& v) { x *= v.x; y *= v.y; return *this; }
    constexpr Vector2& operator/=(const Vector2& v) { x /= v.x; y /= v.y; return *this; }
    
    constexpr Vector2& operator+=(T s) { x += s; y += s; return *this; }
    constexpr Vector2& operator-=(T s) { x -= s; y -= s; return *this; }
    constexpr Vector2& operator*=(T s) { x *= s; y *= s; return *this; }
    constexpr Vector2& operator/=(T s) { 
        if (s != T(0)) {
            x /= s; y /= s; 
        }
        return *this; 
    }
    
    // Comparison operators
    constexpr bool operator==(const Vector2& v) const { return x == v.x && y == v.y; }
    constexpr bool operator!=(const Vector2& v) const { return !(*this == v); }
    
    // Lexicographical ordering (for containers like std::map, std::set)
    constexpr bool operator<(const Vector2& v) const { 
        return (x < v.x) || (x == v.x && y < v.y); 
    }
    constexpr bool operator>(const Vector2& v) const { return v < *this; }
    constexpr bool operator<=(const Vector2& v) const { return !(v < *this); }
    constexpr bool operator>=(const Vector2& v) const { return !(*this < v); }
    
    // Magnitude comparison (comparing lengths)
    constexpr bool isLongerThan(const Vector2& v) const { return lengthSquared() > v.lengthSquared(); }
    constexpr bool isShorterThan(const Vector2& v) const { return lengthSquared() < v.lengthSquared(); }
    constexpr bool isLongerThanOrEqual(const Vector2& v) const { return lengthSquared() >= v.lengthSquared(); }
    constexpr bool isShorterThanOrEqual(const Vector2& v) const { return lengthSquared() <= v.lengthSquared(); }
    
    // Unary operators
    constexpr Vector2 operator-() const { return Vector2(-x, -y); }
    
    // Common vector operations
    constexpr T dot(const Vector2& v) const { return x * v.x + y * v.y; }
    constexpr T lengthSquared() const { return dot(*this); }
    T length() const { return std::sqrt(lengthSquared()); }
    
    Vector2 normalized() const {
        T len = length();
        if (len > T(1e-6)) {
            return Vector2(x / len, y / len);
        }
        return *this;
    }
    
    void normalize() {
        T len = length();
        if (len > T(1e-6)) {
            x /= len;
            y /= len;
        }
    }

    // Data access
    constexpr T* getData() { return data; }
    constexpr const T* getData() const { return data; }
    static constexpr size_t size() { return 2; }
};

// Common type aliases
using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2i = Vector2<int>;

} // namespace PyNovaGE

#ifdef _MSC_VER
#pragma warning(pop)
#endif
