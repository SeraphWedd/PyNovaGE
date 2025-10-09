#pragma once

#include <type_traits>
#include <array>

#include "../vectors/vector3.hpp"
#include "../simd/matrix_ops.hpp"

namespace PyNovaGE {

template<typename T>
class Matrix3 {
public:
    // Ensure T is a floating point type
    static_assert(std::is_floating_point_v<T>, "Matrix3 only supports floating point types");

    // SIMD-friendly data storage using array of Vector3s for rows
    std::array<Vector3<T>, 3> rows;

    // Constructors
    Matrix3() noexcept : rows{
        Vector3<T>{1, 0, 0},
        Vector3<T>{0, 1, 0},
        Vector3<T>{0, 0, 1}
    } {} // Identity matrix

    Matrix3(const Vector3<T>& row0, const Vector3<T>& row1, const Vector3<T>& row2) noexcept
        : rows{row0, row1, row2} {}

    Matrix3(T m00, T m01, T m02,
            T m10, T m11, T m12,
            T m20, T m21, T m22) noexcept
        : rows{
            Vector3<T>{m00, m01, m02},
            Vector3<T>{m10, m11, m12},
            Vector3<T>{m20, m21, m22}
        } {}

    // Access operators
    Vector3<T>& operator[](size_t idx) { return rows[idx]; }
    const Vector3<T>& operator[](size_t idx) const { return rows[idx]; }

    // Basic arithmetic operators
    Matrix3 operator+(const Matrix3& other) const noexcept {
        return Matrix3{
            rows[0] + other.rows[0],
            rows[1] + other.rows[1],
            rows[2] + other.rows[2]
        };
    }

    Matrix3 operator-(const Matrix3& other) const noexcept {
        return Matrix3{
            rows[0] - other.rows[0],
            rows[1] - other.rows[1],
            rows[2] - other.rows[2]
        };
    }

    Matrix3 operator*(T scalar) const noexcept {
        return Matrix3{
            rows[0] * scalar,
            rows[1] * scalar,
            rows[2] * scalar
        };
    }

    // Matrix multiplication
    Matrix3 operator*(const Matrix3& other) const noexcept {
        Matrix3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                T sum = 0;
                for (int k = 0; k < 3; ++k) {
                    sum += rows[i][k] * other.rows[k][j];
                }
                result[i][j] = sum;
            }
        }
        return result;
    }

    // Vector multiplication
    Vector3<T> operator*(const Vector3<T>& vec) const noexcept {
        return Vector3<T>{
            rows[0].dot(vec),
            rows[1].dot(vec),
            rows[2].dot(vec)
        };
    }

    // Determinant
    T determinant() const noexcept {
        return rows[0][0] * (rows[1][1] * rows[2][2] - rows[1][2] * rows[2][1])
             - rows[0][1] * (rows[1][0] * rows[2][2] - rows[1][2] * rows[2][0])
             + rows[0][2] * (rows[1][0] * rows[2][1] - rows[1][1] * rows[2][0]);
    }

    // Inverse
    Matrix3 inverse() const noexcept {
        T det = determinant();
        if (det == 0) return *this; // Return original matrix if not invertible

        T invDet = 1 / det;

        return Matrix3{
            (rows[1][1] * rows[2][2] - rows[1][2] * rows[2][1]) * invDet,
            (rows[0][2] * rows[2][1] - rows[0][1] * rows[2][2]) * invDet,
            (rows[0][1] * rows[1][2] - rows[0][2] * rows[1][1]) * invDet,

            (rows[1][2] * rows[2][0] - rows[1][0] * rows[2][2]) * invDet,
            (rows[0][0] * rows[2][2] - rows[0][2] * rows[2][0]) * invDet,
            (rows[0][2] * rows[1][0] - rows[0][0] * rows[1][2]) * invDet,

            (rows[1][0] * rows[2][1] - rows[1][1] * rows[2][0]) * invDet,
            (rows[0][1] * rows[2][0] - rows[0][0] * rows[2][1]) * invDet,
            (rows[0][0] * rows[1][1] - rows[0][1] * rows[1][0]) * invDet
        };
    }

    // Transpose
    Matrix3 transpose() const noexcept {
        return Matrix3{
            rows[0][0], rows[1][0], rows[2][0],
            rows[0][1], rows[1][1], rows[2][1],
            rows[0][2], rows[1][2], rows[2][2]
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