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
    #if defined(NOVA_AVX2_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm256_castps128_ps256(_mm_loadu_ps(a.data()));
        auto vb = _mm256_castps128_ps256(_mm_loadu_ps(b.data()));
        _mm_storeu_ps(result.data(),
                    _mm256_castps256_ps128(_mm256_add_ps(va, vb)));
        return result;
    }
    #elif defined(NOVA_SSE2_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm_loadu_ps(a.data());
        auto vb = _mm_loadu_ps(b.data());
        _mm_storeu_ps(result.data(), _mm_add_ps(va, vb));
        return result;
    } else if constexpr (N == 2 && std::is_same_v<T, float>) {
        // Use unaligned loads/stores safely
        __m128 va = _mm_castpd_ps(_mm_loadu_pd(reinterpret_cast<const double*>(a.data())));
        __m128 vb = _mm_castpd_ps(_mm_loadu_pd(reinterpret_cast<const double*>(b.data())));
        __m128 sum = _mm_add_ps(va, vb);
        _mm_storeu_pd(reinterpret_cast<double*>(result.data()), _mm_castps_pd(sum));
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
    #if defined(NOVA_SSE2_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm_loadu_ps(a.data());
        auto vb = _mm_loadu_ps(b.data());
        _mm_storeu_ps(result.data(), _mm_sub_ps(va, vb));
        return result;
    } else if constexpr (N == 2 && std::is_same_v<T, float>) {
        __m128 va = _mm_castpd_ps(_mm_loadu_pd(reinterpret_cast<const double*>(a.data())));
        __m128 vb = _mm_castpd_ps(_mm_loadu_pd(reinterpret_cast<const double*>(b.data())));
        __m128 diff = _mm_sub_ps(va, vb);
        _mm_storeu_pd(reinterpret_cast<double*>(result.data()), _mm_castps_pd(diff));
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
    #if defined(NOVA_SSE2_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        auto va = _mm_loadu_ps(a.data());
        auto vb = _mm_loadu_ps(b.data());
        auto mul = _mm_mul_ps(va, vb);
        // More efficient horizontal sum using shuffles
        auto shuf = _mm_movehdup_ps(mul);        // Duplicate odd elements
        auto sums = _mm_add_ps(mul, shuf);       // Add pairs
        shuf = _mm_movehl_ps(shuf, sums);        // High half -> low half
        sums = _mm_add_ss(sums, shuf);           // Add last pair
        return _mm_cvtss_f32(sums);
    } else if constexpr (N == 2 && std::is_same_v<T, float>) {
        __m128 va = _mm_castpd_ps(_mm_loadu_pd(reinterpret_cast<const double*>(a.data())));
        __m128 vb = _mm_castpd_ps(_mm_loadu_pd(reinterpret_cast<const double*>(b.data())));
        __m128 mul = _mm_mul_ps(va, vb);
        __m128 sum = _mm_add_ss(mul, _mm_shuffle_ps(mul, mul, 1));
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
    Vector<T, N> result;
    
    #if defined(NOVA_AVX2_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        // Load vector and compute dot product more efficiently
        auto vv = _mm_loadu_ps(v.data());
        auto sq = _mm_mul_ps(vv, vv);
        auto sum = _mm_add_ps(_mm_movehl_ps(sq, sq), sq);
        sum = _mm_add_ss(_mm_shuffle_ps(sum, sum, 1), sum);
        
        if (_mm_cvtss_f32(sum) == 0.0f) return v;
        
        // Broadcast length squared to all elements
        auto len_sq = _mm_shuffle_ps(sum, sum, 0);
        
        // Fast reciprocal square root
        auto rsqrt = _mm_rsqrt_ps(len_sq);
        
        // One Newton-Raphson iteration for better accuracy
        auto half = _mm_set1_ps(0.5f);
        auto three = _mm_set1_ps(3.0f);
        auto rsqrtSq = _mm_mul_ps(rsqrt, rsqrt);
        auto correction = _mm_mul_ps(len_sq, rsqrtSq);
        correction = _mm_sub_ps(three, _mm_mul_ps(half, correction));
        rsqrt = _mm_mul_ps(rsqrt, correction);
        
        _mm_storeu_ps(result.data(), _mm_mul_ps(vv, rsqrt));
        return result;
    }
    #elif defined(NOVA_SSE4_1_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        // SSE4.1 path: Use dp for dot product
        auto vv = _mm_loadu_ps(v.data());
        auto len_sq = _mm_dp_ps(vv, vv, 0xFF);
        
        if (_mm_cvtss_f32(len_sq) == 0.0f) return v;
        
        auto rsqrt = _mm_rsqrt_ps(len_sq);
        _mm_storeu_ps(result.data(), _mm_mul_ps(vv, rsqrt));
        return result;
    }
    #elif defined(NOVA_SSE2_AVAILABLE)
    if constexpr (N == 4 && std::is_same_v<T, float>) {
        // SSE2 path: Manual dot product
        auto vv = _mm_loadu_ps(v.data());
        auto sq = _mm_mul_ps(vv, vv);
        auto sum = _mm_add_ps(_mm_movehl_ps(sq, sq), sq);
        sum = _mm_add_ss(_mm_shuffle_ps(sum, sum, 1), sum);
        
        if (_mm_cvtss_f32(sum) == 0.0f) return v;
        
        auto len_sq = _mm_shuffle_ps(sum, sum, 0);
        auto rsqrt = _mm_rsqrt_ps(len_sq);
        _mm_storeu_ps(result.data(), _mm_mul_ps(vv, rsqrt));
        return result;
    } else if constexpr (N == 2 && std::is_same_v<T, float>) {
        // SSE2 path for Vector2 with unaligned loads/stores
        __m128 vv = _mm_castpd_ps(_mm_loadu_pd(reinterpret_cast<const double*>(v.data())));
        __m128 sq = _mm_mul_ps(vv, vv);
        __m128 shuf = _mm_shuffle_ps(sq, sq, 1);
        __m128 sum = _mm_add_ss(sq, shuf);
        if (_mm_cvtss_f32(sum) == 0.0f) return v;
        __m128 len_sq = _mm_shuffle_ps(sum, sum, 0);
        __m128 rsqrt = _mm_rsqrt_ps(len_sq);
        _mm_storeu_pd(reinterpret_cast<double*>(result.data()), _mm_castps_pd(_mm_mul_ps(vv, rsqrt)));
        return result;
    }
    #endif
    
    // Non-SIMD fallback
    T length_sq = dot(v, v);
    if (length_sq == T(0)) return v;
    return v * (T(1) / std::sqrt(length_sq));
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