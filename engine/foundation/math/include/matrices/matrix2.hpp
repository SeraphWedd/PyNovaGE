#pragma once

#include <type_traits>
#include <array>

#include "../vectors/vector2.hpp"
#include "../simd/matrix_ops.hpp"

namespace PyNovaGE {

template<typename T>
class alignas(16) Matrix2 {
public:
    // Ensure T is a floating point type
    static_assert(std::is_floating_point_v<T>, "Matrix2 only supports floating point types");

    // SIMD-friendly data storage using array of Vector2s for rows
    std::array<Vector2<T>, 2> rows;

    // Constructors
    Matrix2() noexcept : rows{Vector2<T>{1, 0}, Vector2<T>{0, 1}} {} // Identity matrix
    Matrix2(const Vector2<T>& row0, const Vector2<T>& row1) noexcept : rows{row0, row1} {}
    Matrix2(T m00, T m01, T m10, T m11) noexcept : rows{Vector2<T>{m00, m01}, Vector2<T>{m10, m11}} {}

    // Access operators
    Vector2<T>& operator[](size_t idx) { return rows[idx]; }
    const Vector2<T>& operator[](size_t idx) const { return rows[idx]; }

    // Basic arithmetic operators
    Matrix2 operator+(const Matrix2& other) const noexcept {
        return Matrix2{
            rows[0] + other.rows[0],
            rows[1] + other.rows[1]
        };
    }

    Matrix2 operator-(const Matrix2& other) const noexcept {
        return Matrix2{
            rows[0] - other.rows[0],
            rows[1] - other.rows[1]
        };
    }

    Matrix2 operator*(T scalar) const noexcept {
        return Matrix2{
            rows[0] * scalar,
            rows[1] * scalar
        };
    }

    // Matrix multiplication
    Matrix2 operator*(const Matrix2& other) const noexcept {
        Matrix2 result;
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                T sum = 0;
                for (int k = 0; k < 2; ++k) {
                    sum += rows[i][k] * other.rows[k][j];
                }
                result[i][j] = sum;
            }
        }
        return result;
    }

    // Vector multiplication
    Vector2<T> operator*(const Vector2<T>& vec) const noexcept {
        return Vector2<T>{
            rows[0].dot(vec),
            rows[1].dot(vec)
        };
    }

    // Determinant
    T determinant() const noexcept {
        return rows[0][0] * rows[1][1] - rows[0][1] * rows[1][0];
    }

    // Inverse
    Matrix2 inverse() const noexcept {
        T det = determinant();
        if (det == 0) return *this; // Return original matrix if not invertible

        T invDet = 1 / det;
        return Matrix2{
            rows[1][1] * invDet, -rows[0][1] * invDet,
            -rows[1][0] * invDet, rows[0][0] * invDet
        };
    }

    // Transpose
    Matrix2 transpose() const noexcept {
        return Matrix2{
            rows[0][0], rows[1][0],
            rows[0][1], rows[1][1]
        };
    }

    static Matrix2 Identity() noexcept {
        return Matrix2{};
    }

    static Matrix2 Rotation(T angle) noexcept {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix2{
            c, -s,
            s, c
        };
    }

    static Matrix2 Scale(T x, T y) noexcept {
        return Matrix2{
            x, 0,
            0, y
        };
    }
};

} // namespace PyNovaGE