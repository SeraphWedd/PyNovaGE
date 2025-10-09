#pragma once

#include <type_traits>
#include <array>

#include "../vectors/vector4.hpp"
#include "../vectors/vector3.hpp"
#include "../simd/matrix_ops.hpp"

namespace PyNovaGE {

template<typename T>
class Matrix4 {
public:
    // Ensure T is a floating point type
    static_assert(std::is_floating_point_v<T>, "Matrix4 only supports floating point types");

    // SIMD-friendly data storage using array of Vector4s for rows
    std::array<Vector4<T>, 4> rows;

    // Constructors
    Matrix4() noexcept : rows{
        Vector4<T>{1, 0, 0, 0},
        Vector4<T>{0, 1, 0, 0},
        Vector4<T>{0, 0, 1, 0},
        Vector4<T>{0, 0, 0, 1}
    } {} // Identity matrix

    Matrix4(const Vector4<T>& row0, const Vector4<T>& row1,
           const Vector4<T>& row2, const Vector4<T>& row3) noexcept
        : rows{row0, row1, row2, row3} {}

    Matrix4(T m00, T m01, T m02, T m03,
            T m10, T m11, T m12, T m13,
            T m20, T m21, T m22, T m23,
            T m30, T m31, T m32, T m33) noexcept
        : rows{
            Vector4<T>{m00, m01, m02, m03},
            Vector4<T>{m10, m11, m12, m13},
            Vector4<T>{m20, m21, m22, m23},
            Vector4<T>{m30, m31, m32, m33}
        } {}

    // Access operators
    Vector4<T>& operator[](size_t idx) { return rows[idx]; }
    const Vector4<T>& operator[](size_t idx) const { return rows[idx]; }

    // Basic arithmetic operators
    Matrix4 operator+(const Matrix4& other) const noexcept {
        return Matrix4{
            rows[0] + other.rows[0],
            rows[1] + other.rows[1],
            rows[2] + other.rows[2],
            rows[3] + other.rows[3]
        };
    }

    Matrix4 operator-(const Matrix4& other) const noexcept {
        return Matrix4{
            rows[0] - other.rows[0],
            rows[1] - other.rows[1],
            rows[2] - other.rows[2],
            rows[3] - other.rows[3]
        };
    }

    Matrix4 operator*(T scalar) const noexcept {
        return Matrix4{
            rows[0] * scalar,
            rows[1] * scalar,
            rows[2] * scalar,
            rows[3] * scalar
        };
    }

    // Matrix multiplication
    Matrix4 operator*(const Matrix4& other) const noexcept {
        Matrix4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                T sum = 0;
                for (int k = 0; k < 4; ++k) {
                    sum += rows[i][k] * other.rows[k][j];
                }
                result[i][j] = sum;
            }
        }
        return result;
    }

    // Vector multiplication
    Vector4<T> operator*(const Vector4<T>& vec) const noexcept {
        return Vector4<T>{
            rows[0].dot(vec),
            rows[1].dot(vec),
            rows[2].dot(vec),
            rows[3].dot(vec)
        };
    }

    // Determinant using cofactor expansion along first row
    T determinant() const noexcept {
        // Get 2x2 determinants for the cofactor calculations
        T d2_01 = rows[2][2] * rows[3][3] - rows[2][3] * rows[3][2];
        T d2_02 = rows[2][1] * rows[3][3] - rows[2][3] * rows[3][1];
        T d2_03 = rows[2][1] * rows[3][2] - rows[2][2] * rows[3][1];
        T d2_12 = rows[2][0] * rows[3][3] - rows[2][3] * rows[3][0];
        T d2_13 = rows[2][0] * rows[3][2] - rows[2][2] * rows[3][0];
        T d2_23 = rows[2][0] * rows[3][1] - rows[2][1] * rows[3][0];

        // Get 3x3 determinants
        T d3_0 = rows[1][1] * d2_01 - rows[1][2] * d2_02 + rows[1][3] * d2_03;
        T d3_1 = rows[1][0] * d2_01 - rows[1][2] * d2_12 + rows[1][3] * d2_13;
        T d3_2 = rows[1][0] * d2_02 - rows[1][1] * d2_12 + rows[1][3] * d2_23;
        T d3_3 = rows[1][0] * d2_03 - rows[1][1] * d2_13 + rows[1][2] * d2_23;

        // Calculate final determinant
        return rows[0][0] * d3_0 - rows[0][1] * d3_1 + rows[0][2] * d3_2 - rows[0][3] * d3_3;
    }

    // Inverse using adjugate matrix / determinant
    Matrix4 inverse() const noexcept {
        T det = determinant();
        if (det == 0) return *this; // Return original matrix if not invertible

        T invDet = 1 / det;

        // Calculate cofactor matrix
        Matrix4 cofactor;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                // Calculate minor by excluding row i and column j
                T minor = 0;
                for (int k = 0; k < 3; ++k) {
                    int row1 = (k < i) ? k : k + 1;
                    for (int l = 0; l < 3; ++l) {
                        int col1 = (l < j) ? l : l + 1;
                        int row2 = (k + 1 < i) ? k + 1 : k + 2;
                        int col2 = (l + 1 < j) ? l + 1 : l + 2;
                        minor += rows[row1][col1] * rows[row2][col2];
                    }
                }
                // Apply checkerboard pattern for cofactor
                cofactor[j][i] = ((i + j) % 2 == 0 ? 1 : -1) * minor * invDet;
            }
        }
        return cofactor;
    }

    // Transpose
    Matrix4 transpose() const noexcept {
        return Matrix4{
            rows[0][0], rows[1][0], rows[2][0], rows[3][0],
            rows[0][1], rows[1][1], rows[2][1], rows[3][1],
            rows[0][2], rows[1][2], rows[2][2], rows[3][2],
            rows[0][3], rows[1][3], rows[2][3], rows[3][3]
        };
    }

    static Matrix4 Identity() noexcept {
        return Matrix4{};
    }

    static Matrix4 Scale(T x, T y, T z) noexcept {
        return Matrix4{
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1
        };
    }

    static Matrix4 RotationX(T angle) noexcept {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix4{
            1, 0,  0, 0,
            0, c, -s, 0,
            0, s,  c, 0,
            0, 0,  0, 1
        };
    }

    static Matrix4 RotationY(T angle) noexcept {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix4{
             c, 0, s, 0,
             0, 1, 0, 0,
            -s, 0, c, 0,
             0, 0, 0, 1
        };
    }

    static Matrix4 RotationZ(T angle) noexcept {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix4{
            c, -s, 0, 0,
            s,  c, 0, 0,
            0,  0, 1, 0,
            0,  0, 0, 1
        };
    }

    static Matrix4 Translation(T x, T y, T z) noexcept {
        return Matrix4{
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1
        };
    }

    static Matrix4 Perspective(T fovy, T aspect, T near, T far) noexcept {
        T f = 1 / std::tan(fovy / 2);
        T range_inv = 1 / (near - far);

        return Matrix4{
            f/aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (far + near) * range_inv, 2 * far * near * range_inv,
            0, 0, -1, 0
        };
    }

    static Matrix4 Orthographic(T left, T right, T bottom, T top, T near, T far) noexcept {
        T width = right - left;
        T height = top - bottom;
        T depth = far - near;

        return Matrix4{
            2/width, 0, 0, -(right + left)/width,
            0, 2/height, 0, -(top + bottom)/height,
            0, 0, -2/depth, -(far + near)/depth,
            0, 0, 0, 1
        };
    }

    static Matrix4 LookAt(const Vector3<T>& eye, const Vector3<T>& target, const Vector3<T>& up) noexcept {
        Vector3<T> z = (eye - target).normalized();
        Vector3<T> x = up.cross(z).normalized();
        Vector3<T> y = z.cross(x);

        return Matrix4{
            x[0], x[1], x[2], -x.dot(eye),
            y[0], y[1], y[2], -y.dot(eye),
            z[0], z[1], z[2], -z.dot(eye),
            0, 0, 0, 1
        };
    }
};

} // namespace PyNovaGE