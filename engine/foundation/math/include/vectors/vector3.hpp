#pragma once

#include "../simd/vector_ops.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)  // nameless struct/union warning
#pragma warning(disable: 4458)  // declaration hides class member
#endif

namespace PyNovaGE {

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
        #if defined(NOVA_SSE2_AVAILABLE)
        if constexpr (std::is_same_v<T, float>) {
            Vector3 result;
            // Load vectors with zero w component
            __m128 va = _mm_setr_ps(x, y, z, 0.0f);
            __m128 vb = _mm_setr_ps(v.x, v.y, v.z, 0.0f);
            
            // Shuffle for cross product: (a.y, a.z, a.x, _) and (b.z, b.x, b.y, _)
            __m128 va_yzx = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 0, 2, 1));
            __m128 vb_zxy = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 1, 0, 2));
            
            // Shuffle for second part: (a.z, a.x, a.y, _) and (b.y, b.z, b.x, _)
            __m128 va_zxy = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 1, 0, 2));
            __m128 vb_yzx = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 0, 2, 1));
            
            // Cross product calculation
            __m128 mul1 = _mm_mul_ps(va_yzx, vb_zxy);
            __m128 mul2 = _mm_mul_ps(va_zxy, vb_yzx);
            __m128 cross = _mm_sub_ps(mul1, mul2);
            
            // Store result
            alignas(16) float tmp[4];
            _mm_storeu_ps(tmp, cross);
            result.x = tmp[0];
            result.y = tmp[1];
            result.z = tmp[2];
            return result;
        }
        #endif
        
        // Fallback scalar implementation
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

} // namespace PyNovaGE

#ifdef _MSC_VER
#pragma warning(pop)
#endif
