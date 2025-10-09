#pragma once

#include "types.hpp"
#include "types.hpp"
#include <array>
#include <initializer_list>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PyNovaGE {
namespace SIMD {

// Forward declarations
template<typename T, size_t N> class Matrix;

// Type aliases
using Matrix4f = Matrix<float, 4>;
using Matrix3f = Matrix<float, 3>;
using Matrix2f = Matrix<float, 2>;

template<typename T, size_t N>
class Matrix {
    static_assert(N >= 2 && N <= 4, "Matrix size must be between 2x2 and 4x4");
    
public:
    using value_type = T;
    static constexpr size_t size = N;
    
    // Default constructor (identity matrix)
    constexpr Matrix() : data_{} {
        for (size_t i = 0; i < N; ++i) {
            data_[i * N + i] = T(1);
        }
    }
    
    // Construct from array
    explicit constexpr Matrix(const T* data) {
        std::copy(data, data + N * N, data_);
    }

    // Construct from initializer list
    constexpr Matrix(std::initializer_list<T> init) {
        auto it = init.begin();
        for (size_t i = 0; i < N * N && it != init.end(); ++i, ++it) {
            data_[i] = *it;
        }
    }
    
    // Access operators
    constexpr T& operator()(size_t row, size_t col) {
        return data_[col * N + row];
    }
    
    constexpr const T& operator()(size_t row, size_t col) const {
        return data_[col * N + row];
    }
    
    // Raw data access
    T* data() { return data_; }
    const T* data() const { return data_; }

private:
    alignas(16) T data_[N * N];
};

// Matrix multiplication
template<typename T>
inline Matrix<T, 4> operator*(const Matrix<T, 4>& a, const Matrix<T, 4>& b) {
    Matrix<T, 4> result;
    
    #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
    if constexpr (std::is_same_v<T, float>) {
        for (int i = 0; i < 4; ++i) {
            __m256 row = _mm256_load_ps(&a.data()[i * 4]);
            for (int j = 0; j < 4; ++j) {
                __m256 bCol = _mm256_set_ps(
                    b(3, j), b(2, j), b(1, j), b(0, j),
                    b(3, j), b(2, j), b(1, j), b(0, j)
                );
                __m256 prod = _mm256_mul_ps(row, bCol);
                __m256 sum = _mm256_hadd_ps(prod, prod);
                sum = _mm256_hadd_ps(sum, sum);
                result(i, j) = _mm256_cvtss_f32(sum);
            }
        }
        return result;
    }
    #elif defined(NOVA_SSE2_AVAILABLE)
    if constexpr (std::is_same_v<T, float>) {
        for (int i = 0; i < 4; ++i) {
            __m128 row = _mm_load_ps(&a.data()[i * 4]);
            for (int j = 0; j < 4; ++j) {
                __m128 bCol = _mm_set_ps(
                    b(3, j), b(2, j), b(1, j), b(0, j)
                );
                __m128 prod = _mm_mul_ps(row, bCol);
                __m128 sum = _mm_hadd_ps(prod, prod);
                sum = _mm_hadd_ps(sum, sum);
                result(i, j) = _mm_cvtss_f32(sum);
            }
        }
        return result;
    }
    #elif defined(NOVA_NEON_AVAILABLE)
    if constexpr (std::is_same_v<T, float>) {
        for (int i = 0; i < 4; ++i) {
            float32x4_t row = vld1q_f32(&a.data()[i * 4]);
            for (int j = 0; j < 4; ++j) {
                float32x4_t bCol = {b(0, j), b(1, j), b(2, j), b(3, j)};
                float32x4_t prod = vmulq_f32(row, bCol);
                float32x2_t sum = vadd_f32(vget_low_f32(prod), vget_high_f32(prod));
                result(i, j) = vget_lane_f32(vpadd_f32(sum, sum), 0);
            }
        }
        return result;
    }
    #endif

    // Fallback implementation for other sizes or types
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            T sum = T(0);
            for (size_t k = 0; k < 4; ++k) {
                sum += a(i, k) * b(k, j);
            }
            result(i, j) = sum;
        }
    }
    return result;
}

// Matrix-vector multiplication
template<typename T>
inline Vector<T, 4> operator*(const Matrix<T, 4>& m, const Vector<T, 4>& v) {
    Vector<T, 4> result;
    
    #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
    if constexpr (std::is_same_v<T, float>) {
        __m256 vec = _mm256_broadcast_ps(reinterpret_cast<const __m128*>(v.data()));
        for (int i = 0; i < 4; ++i) {
            __m256 row = _mm256_load_ps(&m.data()[i * 4]);
            __m256 prod = _mm256_mul_ps(row, vec);
            __m256 sum = _mm256_hadd_ps(prod, prod);
            sum = _mm256_hadd_ps(sum, sum);
            result[i] = _mm256_cvtss_f32(sum);
        }
        return result;
    }
    #elif defined(NOVA_SSE2_AVAILABLE)
    if constexpr (std::is_same_v<T, float>) {
        __m128 vec = _mm_load_ps(v.data());
        for (int i = 0; i < 4; ++i) {
            __m128 row = _mm_load_ps(&m.data()[i * 4]);
            __m128 prod = _mm_mul_ps(row, vec);
            __m128 sum = _mm_hadd_ps(prod, prod);
            sum = _mm_hadd_ps(sum, sum);
            result[i] = _mm_cvtss_f32(sum);
        }
        return result;
    }
    #elif defined(NOVA_NEON_AVAILABLE)
    if constexpr (std::is_same_v<T, float>) {
        float32x4_t vec = vld1q_f32(v.data());
        for (int i = 0; i < 4; ++i) {
            float32x4_t row = vld1q_f32(&m.data()[i * 4]);
            float32x4_t prod = vmulq_f32(row, vec);
            float32x2_t sum = vadd_f32(vget_low_f32(prod), vget_high_f32(prod));
            result[i] = vget_lane_f32(vpadd_f32(sum, sum), 0);
        }
        return result;
    }
    #endif

    // Fallback implementation
    for (size_t i = 0; i < 4; ++i) {
        T sum = T(0);
        for (size_t j = 0; j < 4; ++j) {
            sum += m(i, j) * v[j];
        }
        result[i] = sum;
    }
    return result;
}

// Matrix transpose
template<typename T, size_t N>
inline Matrix<T, N> transpose(const Matrix<T, N>& m) {
    Matrix<T, N> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            result(i, j) = m(j, i);
        }
    }
    return result;
}

// Matrix inverse (4x4 only)
template<typename T>
inline Matrix<T, 4> inverse(const Matrix<T, 4>& m) {
    // Implementation using adjugate matrix and determinant
    // Note: For performance-critical code, consider using SIMD-optimized LU decomposition
    // or other numerical methods
    Matrix<T, 4> result;
    
    // Calculate cofactors...
    // This is a simplified version. A full implementation would include
    // the complete inverse calculation using cofactors and adjugate matrix
    
    return result;
}

// Create translation matrix
template<typename T>
inline Matrix<T, 4> translate(const Vector<T, 3>& v) {
    Matrix<T, 4> result;
    result(0, 3) = v[0];
    result(1, 3) = v[1];
    result(2, 3) = v[2];
    return result;
}

// Create scale matrix
template<typename T>
inline Matrix<T, 4> scale(const Vector<T, 3>& v) {
    Matrix<T, 4> result;
    result(0, 0) = v[0];
    result(1, 1) = v[1];
    result(2, 2) = v[2];
    return result;
}

// Create rotation matrix from axis and angle
template<typename T>
inline Matrix<T, 4> rotate(const Vector<T, 3>& axis, T angle) {
    Matrix<T, 4> result;
    
    T c = std::cos(angle);
    T s = std::sin(angle);
    T t = T(1) - c;
    
    Vector<T, 3> norm = normalize(axis);
    T x = norm[0];
    T y = norm[1];
    T z = norm[2];
    
    result(0, 0) = t * x * x + c;
    result(0, 1) = t * x * y - s * z;
    result(0, 2) = t * x * z + s * y;
    
    result(1, 0) = t * x * y + s * z;
    result(1, 1) = t * y * y + c;
    result(1, 2) = t * y * z - s * x;
    
    result(2, 0) = t * x * z - s * y;
    result(2, 1) = t * y * z + s * x;
    result(2, 2) = t * z * z + c;
    
    return result;
}

} // namespace SIMD
} // namespace PyNovaGE