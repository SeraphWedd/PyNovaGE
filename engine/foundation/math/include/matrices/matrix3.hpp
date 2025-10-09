#pragma once

#include <type_traits>
#include <array>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../vectors/vector3.hpp"

namespace PyNovaGE {

template<typename T>
class Matrix3 {
public:
    // Ensure T is a floating point type
    static_assert(std::is_floating_point_v<T>, "Matrix3 only supports floating point types");

    // Simple flat array storage for better cache locality and compiler optimization
    std::array<T, 9> data;

    // Constructors
    Matrix3() noexcept : data{
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    } {} // Identity matrix

    Matrix3(const Vector3<T>& row0, const Vector3<T>& row1, const Vector3<T>& row2) noexcept
        : data{
            row0.x, row0.y, row0.z,
            row1.x, row1.y, row1.z,
            row2.x, row2.y, row2.z
        } {}

    Matrix3(T m00, T m01, T m02,
            T m10, T m11, T m12,
            T m20, T m21, T m22) noexcept
        : data{m00, m01, m02, m10, m11, m12, m20, m21, m22} {}

    // Direct element access - more efficient
    T& operator()(size_t row, size_t col) { return data[row * 3 + col]; }
    const T& operator()(size_t row, size_t col) const { return data[row * 3 + col]; }
    
    // Legacy row access for compatibility - returns Vector3 for API compatibility
    Vector3<T> operator[](size_t row) const {
        const T* ptr = &data[row * 3];
        return Vector3<T>{ptr[0], ptr[1], ptr[2]};
    }
    
    // Element access using array indices
    T& at(size_t row, size_t col) { return data[row * 3 + col]; }
    const T& at(size_t row, size_t col) const { return data[row * 3 + col]; }

    // Basic arithmetic operators - simplified with direct array operations
    Matrix3 operator+(const Matrix3& other) const noexcept {
        Matrix3 result;
        for (int i = 0; i < 9; ++i) {
            result.data[i] = data[i] + other.data[i];
        }
        return result;
    }

    Matrix3 operator-(const Matrix3& other) const noexcept {
        Matrix3 result;
        for (int i = 0; i < 9; ++i) {
            result.data[i] = data[i] - other.data[i];
        }
        return result;
    }

    Matrix3 operator*(T scalar) const noexcept {
        Matrix3 result;
        for (int i = 0; i < 9; ++i) {
            result.data[i] = data[i] * scalar;
        }
        return result;
    }

    // Matrix multiplication - unrolled for better optimization
    Matrix3 operator*(const Matrix3& other) const noexcept {
        const T* a = data.data();
        const T* b = other.data.data();
        
        return Matrix3{
            a[0]*b[0] + a[1]*b[3] + a[2]*b[6], a[0]*b[1] + a[1]*b[4] + a[2]*b[7], a[0]*b[2] + a[1]*b[5] + a[2]*b[8],
            a[3]*b[0] + a[4]*b[3] + a[5]*b[6], a[3]*b[1] + a[4]*b[4] + a[5]*b[7], a[3]*b[2] + a[4]*b[5] + a[5]*b[8],
            a[6]*b[0] + a[7]*b[3] + a[8]*b[6], a[6]*b[1] + a[7]*b[4] + a[8]*b[7], a[6]*b[2] + a[7]*b[5] + a[8]*b[8]
        };
    }

    // Vector multiplication - direct computation
    Vector3<T> operator*(const Vector3<T>& vec) const noexcept {
        const T* m = data.data();
        return Vector3<T>{
            m[0]*vec.x + m[1]*vec.y + m[2]*vec.z,
            m[3]*vec.x + m[4]*vec.y + m[5]*vec.z,
            m[6]*vec.x + m[7]*vec.y + m[8]*vec.z
        };
    }

    // Determinant - direct array access
    T determinant() const noexcept {
        const T* m = data.data();
        return m[0] * (m[4] * m[8] - m[5] * m[7])
             - m[1] * (m[3] * m[8] - m[5] * m[6])
             + m[2] * (m[3] * m[7] - m[4] * m[6]);
    }

    // Inverse - direct array access with precomputed values
    Matrix3 inverse() const noexcept {
        const T* m = data.data();
        
        // Precompute cofactors
        T c00 = m[4] * m[8] - m[5] * m[7];
        T c01 = m[5] * m[6] - m[3] * m[8];
        T c02 = m[3] * m[7] - m[4] * m[6];
        
        T det = m[0] * c00 + m[1] * c01 + m[2] * c02;
        if (det == T(0)) return *this;
        
        T invDet = T(1) / det;
        
        return Matrix3{
            c00 * invDet,
            (m[2] * m[7] - m[1] * m[8]) * invDet,
            (m[1] * m[5] - m[2] * m[4]) * invDet,
            
            c01 * invDet,
            (m[0] * m[8] - m[2] * m[6]) * invDet,
            (m[2] * m[3] - m[0] * m[5]) * invDet,
            
            c02 * invDet,
            (m[1] * m[6] - m[0] * m[7]) * invDet,
            (m[0] * m[4] - m[1] * m[3]) * invDet
        };
    }

    // Transpose - direct array access
    Matrix3 transpose() const noexcept {
        const T* m = data.data();
        return Matrix3{
            m[0], m[3], m[6],
            m[1], m[4], m[7],
            m[2], m[5], m[8]
        };
    }

    static Matrix3 Identity() noexcept {
        return Matrix3{};
    }

    static Matrix3 Scale(T x, T y) noexcept {
        return Matrix3{
            x, 0, 0,
            0, y, 0,
            0, 0, 1
        };
    }

    static Matrix3 Scale(T x, T y, T z) noexcept {
        return Matrix3{
            x, 0, 0,
            0, y, 0,
            0, 0, z
        };
    }

    static Matrix3 Rotation(T angle) noexcept {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix3{
            c, -s, 0,
            s,  c, 0,
            0,  0, 1
        };
    }

    static Matrix3 RotationX(T angle) noexcept {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix3{
            1, 0,  0,
            0, c, -s,
            0, s,  c
        };
    }

    static Matrix3 RotationY(T angle) noexcept {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix3{
             c, 0, s,
             0, 1, 0,
            -s, 0, c
        };
    }

    static Matrix3 RotationZ(T angle) noexcept {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix3{
            c, -s, 0,
            s,  c, 0,
            0,  0, 1
        };
    }

    static Matrix3 Translation(T x, T y) noexcept {
        return Matrix3{
            1, 0, x,
            0, 1, y,
            0, 0, 1
        };
    }
};

} // namespace PyNovaGE