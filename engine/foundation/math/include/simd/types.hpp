#pragma once

#include "config.hpp"
#include <cstddef>
#include <cstring>

// Include SIMD headers based on available features
#if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
    #include <immintrin.h>
#elif defined(NOVA_SSE2_AVAILABLE)
    #include <emmintrin.h>
#elif defined(NOVA_NEON_AVAILABLE)
    #include <arm_neon.h>
#endif

namespace PyNovaGE {
namespace SIMD {

// Forward declarations
template<typename T, size_t N> class Vector;

// Specialized Vector types for different sizes
using Vector4f = Vector<float, 4>;
using Vector3f = Vector<float, 3>;
using Vector2f = Vector<float, 2>;
using Vector4d = Vector<double, 4>;
using Vector3d = Vector<double, 3>;
using Vector2d = Vector<double, 2>;
using Vector4i = Vector<int32_t, 4>;
using Vector3i = Vector<int32_t, 3>;
using Vector2i = Vector<int32_t, 2>;

// SIMD register types based on architecture
namespace detail {
    #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
        using float_reg = __m256;
        using double_reg = __m256d;
        using int_reg = __m256i;
    #elif defined(NOVA_SSE2_AVAILABLE)
        using float_reg = __m128;
        using double_reg = __m128d;
        using int_reg = __m128i;
    #elif defined(NOVA_NEON_AVAILABLE)
        using float_reg = float32x4_t;
        using double_reg = float64x2_t;
        using int_reg = int32x4_t;
    #else
        template<typename T, size_t N>
        struct scalar_reg {
            T data[N];
        };
        using float_reg = scalar_reg<float, 4>;
        using double_reg = scalar_reg<double, 4>;
        using int_reg = scalar_reg<int32_t, 4>;
    #endif
}

// Main Vector class template
template<typename T, size_t N>
class Vector {
    static_assert(N > 0 && N <= 4, "Vector size must be between 1 and 4");
    static_assert(std::is_arithmetic_v<T>, "Vector type must be arithmetic");

public:
    using value_type = T;
    static constexpr size_t size = N;

    // Default constructor - zero initialization
    constexpr Vector() : data_{} {}

    // Scalar constructor
    explicit constexpr Vector(T scalar) : data_{} {
        for (size_t i = 0; i < N; ++i) {
            data_[i] = scalar;
        }
    }

    // Component-wise constructor
    template<typename... Args>
    constexpr Vector(Args... args) : data_{static_cast<T>(args)...} {
        static_assert(sizeof...(Args) == N, "Number of arguments must match vector size");
    }

    // Array subscript operator
    constexpr T& operator[](size_t index) {
        return data_[index];
    }

    constexpr const T& operator[](size_t index) const {
        return data_[index];
    }

    // Access to raw data
    T* data() { return data_; }
    const T* data() const { return data_; }

    // Scalar multiplication
    friend constexpr Vector operator*(const Vector& v, T scalar) {
        Vector result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] * scalar;
        }
        return result;
    }

    friend constexpr Vector operator*(T scalar, const Vector& v) {
        return v * scalar;
    }

    // Scalar division
    friend constexpr Vector operator/(const Vector& v, T scalar) {
        Vector result;
        T inv_scalar = T(1) / scalar;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] * inv_scalar;
        }
        return result;
    }

    // Component access for common vector sizes
    template<size_t M = N>
    constexpr std::enable_if_t<M >= 1, T&> x() { return data_[0]; }
    
    template<size_t M = N>
    constexpr std::enable_if_t<M >= 2, T&> y() { return data_[1]; }
    
    template<size_t M = N>
    constexpr std::enable_if_t<M >= 3, T&> z() { return data_[2]; }
    
    template<size_t M = N>
    constexpr std::enable_if_t<M >= 4, T&> w() { return data_[3]; }

private:
    alignas(16) T data_[N];
};

} // namespace SIMD
} // namespace PyNovaGE