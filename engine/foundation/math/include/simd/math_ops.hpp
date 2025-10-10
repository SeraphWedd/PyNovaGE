#pragma once

#include "types.hpp"
#include <cmath>
#include <algorithm>

namespace PyNovaGE {
namespace SIMD {

// Forward declarations of platform-specific implementations
namespace detail {
    template<typename T>
    inline Vector<T, 4> sqrt_impl(const Vector<T, 4>& v);
    
    template<typename T>
    inline Vector<T, 4> rsqrt_impl(const Vector<T, 4>& v);
    
    template<typename T>
    inline Vector<T, 4> abs_impl(const Vector<T, 4>& v);
    
    template<typename T>
    inline Vector<T, 4> min_impl(const Vector<T, 4>& a, const Vector<T, 4>& b);
    
    template<typename T>
    inline Vector<T, 4> max_impl(const Vector<T, 4>& a, const Vector<T, 4>& b);
}

// Square root
template<typename T, size_t N>
inline Vector<T, N> sqrt(const Vector<T, N>& v) {
    if constexpr (N == 4) {
        return detail::sqrt_impl(v);
    } else {
        Vector<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = std::sqrt(v[i]);
        }
        return result;
    }
}

// Reciprocal square root
template<typename T, size_t N>
inline Vector<T, N> rsqrt(const Vector<T, N>& v) {
    if constexpr (N == 4) {
        return detail::rsqrt_impl(v);
    } else {
        Vector<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = T(1) / std::sqrt(v[i]);
        }
        return result;
    }
}

// Absolute value
template<typename T, size_t N>
inline Vector<T, N> abs(const Vector<T, N>& v) {
    if constexpr (N == 4) {
        return detail::abs_impl(v);
    } else {
        Vector<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = std::abs(v[i]);
        }
        return result;
    }
}

// Minimum
template<typename T, size_t N>
inline Vector<T, N> min(const Vector<T, N>& a, const Vector<T, N>& b) {
    if constexpr (N == 4) {
        return detail::min_impl(a, b);
    } else {
        Vector<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = std::min(a[i], b[i]);
        }
        return result;
    }
}

// Maximum
template<typename T, size_t N>
inline Vector<T, N> max(const Vector<T, N>& a, const Vector<T, N>& b) {
    if constexpr (N == 4) {
        return detail::max_impl(a, b);
    } else {
        Vector<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = std::max(a[i], b[i]);
        }
        return result;
    }
}

// Platform-specific implementations
namespace detail {
    template<typename T>
    inline Vector<T, 4> sqrt_impl(const Vector<T, 4>& v) {
        Vector<T, 4> result;
        #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
            if constexpr (std::is_same_v<T, float>) {
                auto va = _mm256_load_ps(v.data());
                _mm256_store_ps(result.data(), _mm256_sqrt_ps(va));
                return result;
            } else if constexpr (std::is_same_v<T, double>) {
                auto va = _mm256_load_pd(v.data());
                _mm256_store_pd(result.data(), _mm256_sqrt_pd(va));
                return result;
            }
        #elif defined(NOVA_SSE2_AVAILABLE)
            if constexpr (std::is_same_v<T, float>) {
                auto va = _mm_load_ps(v.data());
                _mm_store_ps(result.data(), _mm_sqrt_ps(va));
                return result;
            } else if constexpr (std::is_same_v<T, double>) {
                auto va = _mm_load_pd(v.data());
                _mm_store_pd(result.data(), _mm_sqrt_pd(va));
                return result;
            }
        #elif defined(NOVA_NEON_AVAILABLE)
            if constexpr (std::is_same_v<T, float>) {
                auto va = vld1q_f32(v.data());
                vst1q_f32(result.data(), vsqrtq_f32(va));
                return result;
            }
        #endif
        // Fallback implementation
        for (size_t i = 0; i < 4; ++i) {
            result[i] = std::sqrt(v[i]);
        }
        return result;
    }

    template<typename T>
    inline Vector<T, 4> rsqrt_impl(const Vector<T, 4>& v) {
        Vector<T, 4> result;
        #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
            if constexpr (std::is_same_v<T, float>) {
                auto va = _mm256_load_ps(v.data());
                // Approximate rsqrt then refine with Newton-Raphson for accuracy
                auto approx = _mm256_rsqrt_ps(va);
                auto muls = _mm256_mul_ps(_mm256_mul_ps(va, approx), approx);
                auto three = _mm256_set1_ps(3.0f);
                auto half = _mm256_set1_ps(0.5f);
                auto nr = _mm256_mul_ps(half, _mm256_mul_ps(approx, _mm256_sub_ps(three, muls)));
                _mm256_store_ps(result.data(), nr);
                return result;
            }
        #elif defined(NOVA_SSE2_AVAILABLE)
            if constexpr (std::is_same_v<T, float>) {
                auto va = _mm_load_ps(v.data());
                auto approx = _mm_rsqrt_ps(va);
                auto muls = _mm_mul_ps(_mm_mul_ps(va, approx), approx);
                auto three = _mm_set1_ps(3.0f);
                auto half = _mm_set1_ps(0.5f);
                auto nr = _mm_mul_ps(half, _mm_mul_ps(approx, _mm_sub_ps(three, muls)));
                _mm_store_ps(result.data(), nr);
                return result;
            }
        #elif defined(NOVA_NEON_AVAILABLE)
            if constexpr (std::is_same_v<T, float>) {
                auto va = vld1q_f32(v.data());
                auto approx = vrsqrteq_f32(va);
                auto muls = vmulq_f32(vmulq_f32(va, approx), approx);
                auto three = vdupq_n_f32(3.0f);
                auto half = vdupq_n_f32(0.5f);
                auto nr = vmulq_f32(half, vmulq_f32(approx, vsubq_f32(three, muls)));
                vst1q_f32(result.data(), nr);
                return result;
            }
        #endif
        for (size_t i = 0; i < 4; ++i) {
            result[i] = T(1) / std::sqrt(v[i]);
        }
        return result;
    }

    template<typename T>
    inline Vector<T, 4> abs_impl(const Vector<T, 4>& v) {
        Vector<T, 4> result;
        for (size_t i = 0; i < 4; ++i) {
            result[i] = std::abs(v[i]);
        }
        return result;
    }

    template<typename T>
    inline Vector<T, 4> min_impl(const Vector<T, 4>& a, const Vector<T, 4>& b) {
        Vector<T, 4> result;
        for (size_t i = 0; i < 4; ++i) {
            result[i] = std::min(a[i], b[i]);
        }
        return result;
    }

    template<typename T>
    inline Vector<T, 4> max_impl(const Vector<T, 4>& a, const Vector<T, 4>& b) {
        Vector<T, 4> result;
        for (size_t i = 0; i < 4; ++i) {
            result[i] = std::max(a[i], b[i]);
        }
        return result;
    }
}

} // namespace SIMD
} // namespace PyNovaGE