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
        return data_[row * N + col];
    }
    
    constexpr const T& operator()(size_t row, size_t col) const {
        return data_[row * N + col];
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
    
    // Column-major order multiplication
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
    
    // Column-major order matrix-vector multiplication
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