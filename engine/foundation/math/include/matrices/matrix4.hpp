#pragma once

#include <type_traits>
#include <array>
#include <cmath>

#include "../vectors/vector4.hpp"
#include "../vectors/vector3.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PyNovaGE {

template<typename T>
class Matrix4 {
public:
    // Ensure T is a floating point type
    static_assert(std::is_floating_point_v<T>, "Matrix4 only supports floating point types");

    // Flat array storage for better cache locality and compiler optimization
    // Elements stored in row-major order: [m00, m01, m02, m03, m10, m11, ...]
    std::array<T, 16> data;

    // Constructors
    Matrix4() noexcept : data{{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }} {} // Identity matrix

    Matrix4(const Vector4<T>& row0, const Vector4<T>& row1,
           const Vector4<T>& row2, const Vector4<T>& row3) noexcept
        : data{{
            row0[0], row0[1], row0[2], row0[3],
            row1[0], row1[1], row1[2], row1[3],
            row2[0], row2[1], row2[2], row2[3],
            row3[0], row3[1], row3[2], row3[3]
        }} {}

    Matrix4(T m00, T m01, T m02, T m03,
            T m10, T m11, T m12, T m13,
            T m20, T m21, T m22, T m23,
            T m30, T m31, T m32, T m33) noexcept
        : data{{
            m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33
        }} {}

    // Access operators - return Vector4 constructed from flat array data
    Vector4<T> operator[](size_t idx) const {
        size_t base = idx * 4;
        return Vector4<T>{data[base], data[base + 1], data[base + 2], data[base + 3]};
    }
    
    // Direct element access for modifications
    T& at(size_t row, size_t col) { return data[row * 4 + col]; }
    const T& at(size_t row, size_t col) const { return data[row * 4 + col]; }

    // Basic arithmetic operators
    Matrix4 operator+(const Matrix4& other) const noexcept {
        Matrix4 result;
        for (size_t i = 0; i < 16; ++i) {
            result.data[i] = data[i] + other.data[i];
        }
        return result;
    }

    Matrix4 operator-(const Matrix4& other) const noexcept {
        Matrix4 result;
        for (size_t i = 0; i < 16; ++i) {
            result.data[i] = data[i] - other.data[i];
        }
        return result;
    }

    Matrix4 operator*(T scalar) const noexcept {
        Matrix4 result;
        for (size_t i = 0; i < 16; ++i) {
            result.data[i] = data[i] * scalar;
        }
        return result;
    }

    // Matrix multiplication - unrolled for better performance
    Matrix4 operator*(const Matrix4& other) const noexcept {
        const auto& a = data;
        const auto& b = other.data;
        
        return Matrix4{
            // Row 0
            a[0]*b[0] + a[1]*b[4] + a[2]*b[8]  + a[3]*b[12],
            a[0]*b[1] + a[1]*b[5] + a[2]*b[9]  + a[3]*b[13],
            a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14],
            a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15],
            
            // Row 1
            a[4]*b[0] + a[5]*b[4] + a[6]*b[8]  + a[7]*b[12],
            a[4]*b[1] + a[5]*b[5] + a[6]*b[9]  + a[7]*b[13],
            a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14],
            a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15],
            
            // Row 2
            a[8]*b[0]  + a[9]*b[4]  + a[10]*b[8]  + a[11]*b[12],
            a[8]*b[1]  + a[9]*b[5]  + a[10]*b[9]  + a[11]*b[13],
            a[8]*b[2]  + a[9]*b[6]  + a[10]*b[10] + a[11]*b[14],
            a[8]*b[3]  + a[9]*b[7]  + a[10]*b[11] + a[11]*b[15],
            
            // Row 3
            a[12]*b[0] + a[13]*b[4] + a[14]*b[8]  + a[15]*b[12],
            a[12]*b[1] + a[13]*b[5] + a[14]*b[9]  + a[15]*b[13],
            a[12]*b[2] + a[13]*b[6] + a[14]*b[10] + a[15]*b[14],
            a[12]*b[3] + a[13]*b[7] + a[14]*b[11] + a[15]*b[15]
        };
    }

    // Vector multiplication
    Vector4<T> operator*(const Vector4<T>& vec) const noexcept {
        return Vector4<T>{
            data[0]*vec[0] + data[1]*vec[1] + data[2]*vec[2] + data[3]*vec[3],
            data[4]*vec[0] + data[5]*vec[1] + data[6]*vec[2] + data[7]*vec[3],
            data[8]*vec[0] + data[9]*vec[1] + data[10]*vec[2] + data[11]*vec[3],
            data[12]*vec[0] + data[13]*vec[1] + data[14]*vec[2] + data[15]*vec[3]
        };
    }

    // Determinant using cofactor expansion along first row
    T determinant() const noexcept {
        // Calculate 3x3 determinants for cofactors
        T det3_0 = data[5]  * (data[10] * data[15] - data[11] * data[14]) -
                   data[6]  * (data[9]  * data[15] - data[11] * data[13]) +
                   data[7]  * (data[9]  * data[14] - data[10] * data[13]);
                   
        T det3_1 = data[4]  * (data[10] * data[15] - data[11] * data[14]) -
                   data[6]  * (data[8]  * data[15] - data[11] * data[12]) +
                   data[7]  * (data[8]  * data[14] - data[10] * data[12]);
                   
        T det3_2 = data[4]  * (data[9]  * data[15] - data[11] * data[13]) -
                   data[5]  * (data[8]  * data[15] - data[11] * data[12]) +
                   data[7]  * (data[8]  * data[13] - data[9]  * data[12]);
                   
        T det3_3 = data[4]  * (data[9]  * data[14] - data[10] * data[13]) -
                   data[5]  * (data[8]  * data[14] - data[10] * data[12]) +
                   data[6]  * (data[8]  * data[13] - data[9]  * data[12]);
        
        // Final determinant
        return data[0] * det3_0 - data[1] * det3_1 + data[2] * det3_2 - data[3] * det3_3;
    }

    // Inverse using adjugate matrix / determinant
    Matrix4 inverse() const noexcept {
        T det = determinant();
        if (det == 0) return *this; // Return original matrix if not invertible

        T invDet = 1 / det;
        
        // Calculate cofactors directly - this is complex for 4x4, using simplified approach
        // For production code, consider using a more robust implementation
        Matrix4 result;
        
        // Calculate cofactor matrix (transposed adjugate)
        result.data[0]  = invDet * (data[5]*(data[10]*data[15] - data[11]*data[14]) - data[6]*(data[9]*data[15] - data[11]*data[13]) + data[7]*(data[9]*data[14] - data[10]*data[13]));
        result.data[1]  = invDet * -(data[1]*(data[10]*data[15] - data[11]*data[14]) - data[2]*(data[9]*data[15] - data[11]*data[13]) + data[3]*(data[9]*data[14] - data[10]*data[13]));
        result.data[2]  = invDet * (data[1]*(data[6]*data[15] - data[7]*data[14]) - data[2]*(data[5]*data[15] - data[7]*data[13]) + data[3]*(data[5]*data[14] - data[6]*data[13]));
        result.data[3]  = invDet * -(data[1]*(data[6]*data[11] - data[7]*data[10]) - data[2]*(data[5]*data[11] - data[7]*data[9]) + data[3]*(data[5]*data[10] - data[6]*data[9]));
        
        result.data[4]  = invDet * -(data[4]*(data[10]*data[15] - data[11]*data[14]) - data[6]*(data[8]*data[15] - data[11]*data[12]) + data[7]*(data[8]*data[14] - data[10]*data[12]));
        result.data[5]  = invDet * (data[0]*(data[10]*data[15] - data[11]*data[14]) - data[2]*(data[8]*data[15] - data[11]*data[12]) + data[3]*(data[8]*data[14] - data[10]*data[12]));
        result.data[6]  = invDet * -(data[0]*(data[6]*data[15] - data[7]*data[14]) - data[2]*(data[4]*data[15] - data[7]*data[12]) + data[3]*(data[4]*data[14] - data[6]*data[12]));
        result.data[7]  = invDet * (data[0]*(data[6]*data[11] - data[7]*data[10]) - data[2]*(data[4]*data[11] - data[7]*data[8]) + data[3]*(data[4]*data[10] - data[6]*data[8]));
        
        result.data[8]  = invDet * (data[4]*(data[9]*data[15] - data[11]*data[13]) - data[5]*(data[8]*data[15] - data[11]*data[12]) + data[7]*(data[8]*data[13] - data[9]*data[12]));
        result.data[9]  = invDet * -(data[0]*(data[9]*data[15] - data[11]*data[13]) - data[1]*(data[8]*data[15] - data[11]*data[12]) + data[3]*(data[8]*data[13] - data[9]*data[12]));
        result.data[10] = invDet * (data[0]*(data[5]*data[15] - data[7]*data[13]) - data[1]*(data[4]*data[15] - data[7]*data[12]) + data[3]*(data[4]*data[13] - data[5]*data[12]));
        result.data[11] = invDet * -(data[0]*(data[5]*data[11] - data[7]*data[9]) - data[1]*(data[4]*data[11] - data[7]*data[8]) + data[3]*(data[4]*data[9] - data[5]*data[8]));
        
        result.data[12] = invDet * -(data[4]*(data[9]*data[14] - data[10]*data[13]) - data[5]*(data[8]*data[14] - data[10]*data[12]) + data[6]*(data[8]*data[13] - data[9]*data[12]));
        result.data[13] = invDet * (data[0]*(data[9]*data[14] - data[10]*data[13]) - data[1]*(data[8]*data[14] - data[10]*data[12]) + data[2]*(data[8]*data[13] - data[9]*data[12]));
        result.data[14] = invDet * -(data[0]*(data[5]*data[14] - data[6]*data[13]) - data[1]*(data[4]*data[14] - data[6]*data[12]) + data[2]*(data[4]*data[13] - data[5]*data[12]));
        result.data[15] = invDet * (data[0]*(data[5]*data[10] - data[6]*data[9]) - data[1]*(data[4]*data[10] - data[6]*data[8]) + data[2]*(data[4]*data[9] - data[5]*data[8]));
        
        return result;
    }

    // Transpose
    Matrix4 transpose() const noexcept {
        return Matrix4{
            data[0], data[4], data[8],  data[12],
            data[1], data[5], data[9],  data[13],
            data[2], data[6], data[10], data[14],
            data[3], data[7], data[11], data[15]
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