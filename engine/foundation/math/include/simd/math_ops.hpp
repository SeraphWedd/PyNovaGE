#pragma once

#include "types.hpp"
#include <cmath>

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
        #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
            if constexpr (std::is_same_v<T, float>) {
                return Vector<T, 4>(_mm256_sqrt_ps(*reinterpret_cast<const __m256*>(v.data())));
            } else if constexpr (std::is_same_v<T, double>) {
                return Vector<T, 4>(_mm256_sqrt_pd(*reinterpret_cast<const __m256d*>(v.data())));
            }
        #elif defined(NOVA_SSE2_AVAILABLE)
            if constexpr (std::is_same_v<T, float>) {
                return Vector<T, 4>(_mm_sqrt_ps(*reinterpret_cast<const __m128*>(v.data())));
            } else if constexpr (std::is_same_v<T, double>) {
                return Vector<T, 4>(_mm_sqrt_pd(*reinterpret_cast<const __m128d*>(v.data())));
            }
        #elif defined(NOVA_NEON_AVAILABLE)
            if constexpr (std::is_same_v<T, float>) {
                return Vector<T, 4>(vsqrtq_f32(*reinterpret_cast<const float32x4_t*>(v.data())));
            }
        #endif
        // Fallback implementation
        return sqrt<T, 4>(v);
    }

    // Similar implementations for rsqrt_impl, abs_impl, min_impl, and max_impl...
    // Each with their own platform-specific optimizations
}

} // namespace SIMD
} // namespace PyNovaGE