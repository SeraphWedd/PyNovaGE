#pragma once

#include "types.hpp"
#include "math_ops.hpp"
#include <cmath>

namespace PyNovaGE {
namespace SIMD {

// Vector addition
template<typename T, size_t N>
inline Vector<T, N> operator+(const Vector<T, N>& a, const Vector<T, N>& b) {
    Vector<T, N> result;
    #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm256_load_ps(a.data());
        auto vb = _mm256_load_ps(b.data());
        _mm256_store_ps(result.data(), _mm256_add_ps(va, vb));
        return result;
    }
    #elif defined(NOVA_SSE2_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm_load_ps(a.data());
        auto vb = _mm_load_ps(b.data());
        _mm_store_ps(result.data(), _mm_add_ps(va, vb));
        return result;
    }
    #elif defined(NOVA_NEON_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = vld1q_f32(a.data());
        auto vb = vld1q_f32(b.data());
        vst1q_f32(result.data(), vaddq_f32(va, vb));
        return result;
    }
    #endif
    
    // Fallback for other sizes or types
    for (size_t i = 0; i < N; ++i) {
        result[i] = a[i] + b[i];
    }
    return result;
}

// Vector subtraction
template<typename T, size_t N>
inline Vector<T, N> operator-(const Vector<T, N>& a, const Vector<T, N>& b) {
    Vector<T, N> result;
    #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm256_load_ps(a.data());
        auto vb = _mm256_load_ps(b.data());
        _mm256_store_ps(result.data(), _mm256_sub_ps(va, vb));
        return result;
    }
    #elif defined(NOVA_SSE2_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm_load_ps(a.data());
        auto vb = _mm_load_ps(b.data());
        _mm_store_ps(result.data(), _mm_sub_ps(va, vb));
        return result;
    }
    #elif defined(NOVA_NEON_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = vld1q_f32(a.data());
        auto vb = vld1q_f32(b.data());
        vst1q_f32(result.data(), vsubq_f32(va, vb));
        return result;
    }
    #endif
    
    // Fallback for other sizes or types
    for (size_t i = 0; i < N; ++i) {
        result[i] = a[i] - b[i];
    }
    return result;
}

// Dot product
template<typename T, size_t N>
inline T dot(const Vector<T, N>& a, const Vector<T, N>& b) {
    #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm256_load_ps(a.data());
        auto vb = _mm256_load_ps(b.data());
        auto mul = _mm256_mul_ps(va, vb);
        auto sum = _mm256_hadd_ps(mul, mul);
        sum = _mm256_hadd_ps(sum, sum);
        return _mm256_cvtss_f32(sum);
    }
    #elif defined(NOVA_SSE2_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm_load_ps(a.data());
        auto vb = _mm_load_ps(b.data());
        auto mul = _mm_mul_ps(va, vb);
        auto sum = _mm_hadd_ps(mul, mul);
        sum = _mm_hadd_ps(sum, sum);
        return _mm_cvtss_f32(sum);
    }
    #elif defined(NOVA_NEON_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = vld1q_f32(a.data());
        auto vb = vld1q_f32(b.data());
        auto mul = vmulq_f32(va, vb);
        float32x2_t sum = vadd_f32(vget_low_f32(mul), vget_high_f32(mul));
        return vget_lane_f32(vpadd_f32(sum, sum), 0);
    }
    #endif

    // Fallback for other sizes or types
    T result = T(0);
    for (size_t i = 0; i < N; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

// Cross product (only for 3D vectors)
template<typename T>
inline Vector<T, 3> cross(const Vector<T, 3>& a, const Vector<T, 3>& b) {
    return Vector<T, 3>(
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    );
}

// Vector normalization
template<typename T, size_t N>
inline Vector<T, N> normalize(const Vector<T, N>& v) {
    T length_sq = dot(v, v);
    if (length_sq == T(0)) {
        return v;
    }
    return v * rsqrt(Vector<T, 1>(length_sq))[0];
}

// Vector length
template<typename T, size_t N>
inline T length(const Vector<T, N>& v) {
    return std::sqrt(dot(v, v));
}

// Vector length squared
template<typename T, size_t N>
inline T length_squared(const Vector<T, N>& v) {
    return dot(v, v);
}

// Lerp
template<typename T, size_t N>
inline Vector<T, N> lerp(const Vector<T, N>& a, const Vector<T, N>& b, T t) {
    return a + (b - a) * t;
}

} // namespace SIMD
} // namespace PyNovaGE