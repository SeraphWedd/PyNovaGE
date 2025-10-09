#pragma once

#include "../simd/vector_ops.hpp"
#include "vector3.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)  // nameless struct/union warning
#pragma warning(disable: 4458)  // declaration hides class member
#endif

namespace PyNovaGE {

template<typename T>
class Vector4 {
public:
    union {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        struct { T s, t, p, q; };
        T data[4];
    };

    // Constructors
    constexpr Vector4() : x(T(0)), y(T(0)), z(T(0)), w(T(0)) {}
    constexpr Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    constexpr explicit Vector4(T v) : x(v), y(v), z(v), w(v) {}
    
    // Construct from Vector3
    constexpr explicit Vector4(const Vector3<T>& vec, T w = T(1)) : x(vec.x), y(vec.y), z(vec.z), w(w) {}
    
    // Array subscript operator
    constexpr T& operator[](size_t i) { return data[i]; }
    constexpr const T& operator[](size_t i) const { return data[i]; }
    
    // Basic arithmetic operators
    Vector4 operator+(const Vector4& v) const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            // Use aligned load if the data is aligned
            if (reinterpret_cast<std::uintptr_t>(data) % 16 == 0 && 
                reinterpret_cast<std::uintptr_t>(v.data) % 16 == 0) {
                __m128 a = _mm_load_ps(data);
                __m128 b = _mm_load_ps(v.data);
                Vector4 result;
                _mm_store_ps(result.data, _mm_add_ps(a, b));
                return result;
            } else {
                __m128 a = _mm_loadu_ps(data);
                __m128 b = _mm_loadu_ps(v.data);
                Vector4 result;
                _mm_storeu_ps(result.data, _mm_add_ps(a, b));
                return result;
            }
            #else
            return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
            #endif
        } else {
            return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
        }
    }

    Vector4 operator-(const Vector4& v) const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_loadu_ps(v.data);
            Vector4 result;
            _mm_storeu_ps(result.data, _mm_sub_ps(a, b));
            return result;
            #else
            return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
            #endif
        } else {
            return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
        }
    }

    Vector4 operator*(const Vector4& v) const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_loadu_ps(v.data);
            Vector4 result;
            _mm_storeu_ps(result.data, _mm_mul_ps(a, b));
            return result;
            #else
            return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
            #endif
        } else {
            return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
        }
    }

    Vector4 operator/(const Vector4& v) const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_loadu_ps(v.data);
            Vector4 result;
            _mm_storeu_ps(result.data, _mm_div_ps(a, b));
            return result;
            #else
            return Vector4(x / v.x, y / v.y, z / v.z, w / v.w);
            #endif
        } else {
            return Vector4(x / v.x, y / v.y, z / v.z, w / v.w);
        }
    }
    
    Vector4 operator+(T s) const { return Vector4(x + s, y + s, z + s, w + s); }
    Vector4 operator-(T s) const { return Vector4(x - s, y - s, z - s, w - s); }
    
    Vector4 operator*(T s) const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_set1_ps(s);
            Vector4 result;
            _mm_storeu_ps(result.data, _mm_mul_ps(a, b));
            return result;
            #else
            return Vector4(x * s, y * s, z * s, w * s);
            #endif
        } else {
            return Vector4(x * s, y * s, z * s, w * s);
        }
    }
    
    Vector4 operator/(T s) const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_set1_ps(s);
            Vector4 result;
            _mm_storeu_ps(result.data, _mm_div_ps(a, b));
            return result;
            #else
            return Vector4(x / s, y / s, z / s, w / s);
            #endif
        } else {
            return Vector4(x / s, y / s, z / s, w / s);
        }
    }
    
    // Assignment operators
    Vector4& operator+=(const Vector4& v) {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_loadu_ps(v.data);
            _mm_storeu_ps(data, _mm_add_ps(a, b));
            #else
            x += v.x; y += v.y; z += v.z; w += v.w;
            #endif
        } else {
            x += v.x; y += v.y; z += v.z; w += v.w;
        }
        return *this;
    }

    Vector4& operator-=(const Vector4& v) {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_loadu_ps(v.data);
            _mm_storeu_ps(data, _mm_sub_ps(a, b));
            #else
            x -= v.x; y -= v.y; z -= v.z; w -= v.w;
            #endif
        } else {
            x -= v.x; y -= v.y; z -= v.z; w -= v.w;
        }
        return *this;
    }

    Vector4& operator*=(const Vector4& v) {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_loadu_ps(v.data);
            _mm_storeu_ps(data, _mm_mul_ps(a, b));
            #else
            x *= v.x; y *= v.y; z *= v.z; w *= v.w;
            #endif
        } else {
            x *= v.x; y *= v.y; z *= v.z; w *= v.w;
        }
        return *this;
    }

    Vector4& operator/=(const Vector4& v) {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_loadu_ps(v.data);
            _mm_storeu_ps(data, _mm_div_ps(a, b));
            #else
            x /= v.x; y /= v.y; z /= v.z; w /= v.w;
            #endif
        } else {
            x /= v.x; y /= v.y; z /= v.z; w /= v.w;
        }
        return *this;
    }
    
    Vector4& operator+=(T s) { x += s; y += s; z += s; w += s; return *this; }
    Vector4& operator-=(T s) { x -= s; y -= s; z -= s; w -= s; return *this; }
    
    Vector4& operator*=(T s) {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_set1_ps(s);
            _mm_storeu_ps(data, _mm_mul_ps(a, b));
            #else
            x *= s; y *= s; z *= s; w *= s;
            #endif
        } else {
            x *= s; y *= s; z *= s; w *= s;
        }
        return *this;
    }
    
    Vector4& operator/=(T s) {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_set1_ps(s);
            _mm_storeu_ps(data, _mm_div_ps(a, b));
            #else
            x /= s; y /= s; z /= s; w /= s;
            #endif
        } else {
            x /= s; y /= s; z /= s; w /= s;
        }
        return *this;
    }
    
    // Comparison operators
    bool operator==(const Vector4& v) const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_loadu_ps(v.data);
            __m128 cmp = _mm_cmpeq_ps(a, b);
            return _mm_movemask_ps(cmp) == 0xF;
            #else
            return x == v.x && y == v.y && z == v.z && w == v.w;
            #endif
        } else {
            return x == v.x && y == v.y && z == v.z && w == v.w;
        }
    }
    bool operator!=(const Vector4& v) const { return !(*this == v); }
    
    // Lexicographical ordering (for containers like std::map, std::set)
    constexpr bool operator<(const Vector4& v) const { 
        return (x < v.x) || 
               (x == v.x && y < v.y) || 
               (x == v.x && y == v.y && z < v.z) ||
               (x == v.x && y == v.y && z == v.z && w < v.w); 
    }
    constexpr bool operator>(const Vector4& v) const { return v < *this; }
    constexpr bool operator<=(const Vector4& v) const { return !(v < *this); }
    constexpr bool operator>=(const Vector4& v) const { return !(*this < v); }
    
    // Magnitude comparison (comparing lengths)
    bool isLongerThan(const Vector4& v) const { return lengthSquared() > v.lengthSquared(); }
    bool isShorterThan(const Vector4& v) const { return lengthSquared() < v.lengthSquared(); }
    bool isLongerThanOrEqual(const Vector4& v) const { return lengthSquared() >= v.lengthSquared(); }
    bool isShorterThanOrEqual(const Vector4& v) const { return lengthSquared() <= v.lengthSquared(); }
    
    // SIMD component-wise comparison (returns int mask)
    int compare(const Vector4& v) const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 b = _mm_loadu_ps(v.data);
            return _mm_movemask_ps(_mm_cmp_ps(a, b, _CMP_GT_OQ));
            #else
            return ((x > v.x) ? 1 : 0) |
                   ((y > v.y) ? 2 : 0) |
                   ((z > v.z) ? 4 : 0) |
                   ((w > v.w) ? 8 : 0);
            #endif
        } else {
            return ((x > v.x) ? 1 : 0) |
                   ((y > v.y) ? 2 : 0) |
                   ((z > v.z) ? 4 : 0) |
                   ((w > v.w) ? 8 : 0);
        }
    }
    
    // Unary operators
    Vector4 operator-() const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            Vector4 result;
            _mm_storeu_ps(result.data, _mm_xor_ps(a, _mm_set1_ps(-0.0f)));
            return result;
            #else
            return Vector4(-x, -y, -z, -w);
            #endif
        } else {
            return Vector4(-x, -y, -z, -w);
        }
    }
    
    // Common vector operations
    T dot(const Vector4& v) const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            if (reinterpret_cast<std::uintptr_t>(data) % 16 == 0 && 
                reinterpret_cast<std::uintptr_t>(v.data) % 16 == 0) {
                __m128 a = _mm_load_ps(data);
                __m128 b = _mm_load_ps(v.data);
                #if defined(NOVA_FMA3_AVAILABLE)
                // Use FMA for more accurate dot product
                __m128 mul = _mm_mul_ps(a, b);
                __m128 sum = _mm_hadd_ps(mul, mul);
                sum = _mm_hadd_ps(sum, sum);
                return _mm_cvtss_f32(sum);
                #else
                __m128 dp = _mm_dp_ps(a, b, 0xF1);
                return _mm_cvtss_f32(dp);
                #endif
            } else {
                __m128 a = _mm_loadu_ps(data);
                __m128 b = _mm_loadu_ps(v.data);
                #if defined(NOVA_FMA3_AVAILABLE)
                __m128 mul = _mm_mul_ps(a, b);
                __m128 sum = _mm_hadd_ps(mul, mul);
                sum = _mm_hadd_ps(sum, sum);
                return _mm_cvtss_f32(sum);
                #else
                __m128 dp = _mm_dp_ps(a, b, 0xF1);
                return _mm_cvtss_f32(dp);
                #endif
            }
            #else
            return x * v.x + y * v.y + z * v.z + w * v.w;
            #endif
        } else {
            return x * v.x + y * v.y + z * v.z + w * v.w;
        }
    }
    
    T lengthSquared() const { return dot(*this); }
    
    T length() const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 a = _mm_loadu_ps(data);
            __m128 sq = _mm_mul_ps(a, a);
            __m128 sum = _mm_hadd_ps(sq, sq);
            sum = _mm_hadd_ps(sum, sum);
            return _mm_cvtss_f32(_mm_sqrt_ps(sum));
            #else
            return std::sqrt(lengthSquared());
            #endif
        } else {
            return std::sqrt(lengthSquared());
        }
    }
    
    Vector4 normalized() const {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            if (reinterpret_cast<std::uintptr_t>(data) % 16 == 0) {
                __m128 v = _mm_load_ps(data);
                __m128 sq = _mm_mul_ps(v, v);
                #if defined(NOVA_FMA3_AVAILABLE)
                __m128 sum = _mm_dp_ps(v, v, 0xF1); // More accurate dot product
                #else
                __m128 sum = _mm_hadd_ps(sq, sq);
                sum = _mm_hadd_ps(sum, sum);
                #endif
                
                // Check for zero length
                if (_mm_cvtss_f32(sum) == 0.0f) return *this;
                
                // Fast reciprocal sqrt with refined Newton-Raphson iteration
                __m128 approx = _mm_rsqrt_ps(sum);
                __m128 half = _mm_set1_ps(0.5f);
                __m128 three = _mm_set1_ps(3.0f);
                
                // Two iterations for better accuracy with large values
                for (int i = 0; i < 2; ++i) {
                    __m128 approxSq = _mm_mul_ps(approx, approx);
                    #if defined(NOVA_FMA3_AVAILABLE)
                    __m128 correction = _mm_fmsub_ps(sum, approxSq, three);
                    #else
                    __m128 correction = _mm_sub_ps(three, _mm_mul_ps(sum, approxSq));
                    #endif
                    approx = _mm_mul_ps(_mm_mul_ps(half, approx), correction);
                }
                
                Vector4 result;
                _mm_store_ps(result.data, _mm_mul_ps(v, approx));
                return result;
            } else {
                __m128 v = _mm_loadu_ps(data);
                __m128 sq = _mm_mul_ps(v, v);
                #if defined(NOVA_FMA3_AVAILABLE)
                __m128 sum = _mm_dp_ps(v, v, 0xF1);
                #else
                __m128 sum = _mm_hadd_ps(sq, sq);
                sum = _mm_hadd_ps(sum, sum);
                #endif
                
                if (_mm_cvtss_f32(sum) == 0.0f) return *this;
                
                __m128 approx = _mm_rsqrt_ps(sum);
                __m128 half = _mm_set1_ps(0.5f);
                __m128 three = _mm_set1_ps(3.0f);
                
                for (int i = 0; i < 2; ++i) {
                    __m128 approxSq = _mm_mul_ps(approx, approx);
                    #if defined(NOVA_FMA3_AVAILABLE)
                    __m128 correction = _mm_fmsub_ps(sum, approxSq, three);
                    #else
                    __m128 correction = _mm_sub_ps(three, _mm_mul_ps(sum, approxSq));
                    #endif
                    approx = _mm_mul_ps(_mm_mul_ps(half, approx), correction);
                }
                
                Vector4 result;
                _mm_storeu_ps(result.data, _mm_mul_ps(v, approx));
                return result;
            }
            #else
            T len = length();
            return len > T(0) ? *this / len : *this;
            #endif
        } else {
            T len = length();
            return len > T(0) ? *this / len : *this;
        }
    }
    
    void normalize() {
        if constexpr (std::is_same_v<T, float>) {
            #if defined(NOVA_AVX2_AVAILABLE)
            __m128 v = _mm_loadu_ps(data);
            __m128 sq = _mm_mul_ps(v, v);
            __m128 sum = _mm_hadd_ps(sq, sq);
            sum = _mm_hadd_ps(sum, sum);
            
            // Check for zero length
            if (_mm_cvtss_f32(sum) == 0.0f) return;
            
            // Fast reciprocal sqrt with one Newton-Raphson iteration for better accuracy
            __m128 rsqrt = _mm_rsqrt_ps(sum);
            __m128 half = _mm_set1_ps(0.5f);
            __m128 one_point_five = _mm_set1_ps(1.5f);
            __m128 rsqrtSq = _mm_mul_ps(rsqrt, rsqrt);
            __m128 correction = _mm_mul_ps(sum, rsqrtSq);
            correction = _mm_sub_ps(one_point_five, _mm_mul_ps(half, correction));
            rsqrt = _mm_mul_ps(rsqrt, correction);
            
            _mm_storeu_ps(data, _mm_mul_ps(v, rsqrt));
            #else
            T len = length();
            if (len > T(0)) {
                *this /= len;
            }
            #endif
        } else {
            T len = length();
            if (len > T(0)) {
                *this /= len;
            }
        }
    }

    // Data access
    constexpr T* getData() { return data; }
    constexpr const T* getData() const { return data; }
    static constexpr size_t size() { return 4; }
};

// Common type aliases
using Vector4f = Vector4<float>;
using Vector4d = Vector4<double>;
using Vector4i = Vector4<int>;

} // namespace PyNovaGE

#ifdef _MSC_VER
#pragma warning(pop)
#endif
