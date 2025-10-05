#ifndef PYNOVAGE_MATH_MATRIX4_HPP
#define PYNOVAGE_MATH_MATRIX4_HPP

#include "simd_utils.hpp"
#include "vector4.hpp"
#include "vector3.hpp"
#include "math_constants.hpp"
#include <cmath>

namespace pynovage {
namespace math {

/**
 * @brief A SIMD-optimized 4x4 matrix class
 * 
 * Provides efficient operations for 3D transformations including:
 * - Rotations
 * - Scaling
 * - Translation
 * - Perspective and orthographic projections
 * - View transformations
 */
class Matrix4x4 {
public:
    // Data storage (row-major order for easier SIMD operations)
    alignas(16) float m[4][4];

    /**
     * @brief Default constructor, creates identity matrix
     */
    Matrix4x4() {
        setIdentity();
    }

    /**
     * @brief Constructs matrix from 16 values in row-major order
     */
    Matrix4x4(float m00, float m01, float m02, float m03,
              float m10, float m11, float m12, float m13,
              float m20, float m21, float m22, float m23,
              float m30, float m31, float m32, float m33) {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }

    /**
     * @brief Copy constructor
     */
    Matrix4x4(const Matrix4x4& other) = default;

    /**
     * @brief Assignment operator
     */
    Matrix4x4& operator=(const Matrix4x4& other) = default;

    /**
     * @brief Sets this matrix to identity
     */
    void setIdentity() {
        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
        m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
    }

    /**
     * @brief Returns the identity matrix
     */
    static Matrix4x4 Identity() {
        return Matrix4x4();
    }

    /**
     * @brief Creates a translation matrix
     */
    static Matrix4x4 Translation(float x, float y, float z) {
        return Matrix4x4(
            1.0f, 0.0f, 0.0f, x,
            0.0f, 1.0f, 0.0f, y,
            0.0f, 0.0f, 1.0f, z,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    /**
     * @brief Creates a scaling matrix
     */
    static Matrix4x4 Scale(float sx, float sy, float sz) {
        return Matrix4x4(
            sx,   0.0f, 0.0f, 0.0f,
            0.0f, sy,   0.0f, 0.0f,
            0.0f, 0.0f, sz,   0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    /**
     * @brief Creates a rotation matrix around X axis
     * @param angle Rotation angle in radians
     */
    static Matrix4x4 RotationX(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Matrix4x4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, c,    -s,   0.0f,
            0.0f, s,    c,    0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    /**
     * @brief Creates a rotation matrix around Y axis
     * @param angle Rotation angle in radians
     */
    static Matrix4x4 RotationY(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Matrix4x4(
            c,    0.0f, s,    0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -s,   0.0f, c,    0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    /**
     * @brief Creates a rotation matrix around Z axis
     * @param angle Rotation angle in radians
     */
    static Matrix4x4 RotationZ(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Matrix4x4(
            c,    -s,   0.0f, 0.0f,
            s,    c,    0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    /**
     * @brief Matrix multiplication operator
     */
    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 result;
        SimdUtils::MultiplyMatrix4x4(
            reinterpret_cast<const float*>(m),
            reinterpret_cast<const float*>(other.m),
            reinterpret_cast<float*>(result.m)
        );
        return result;
    }

    /**
     * @brief Matrix-vector multiplication operator
     */
    Vector4 operator*(const Vector4& v) const {
        Vector4 result;
        SimdUtils::MultiplyMatrix4x4Vec4(
            reinterpret_cast<const float*>(m),
            reinterpret_cast<const float*>(&v),
            reinterpret_cast<float*>(&result)
        );
        return result;
    }

    /**
     * @brief Transforms a 3D point (adds homogeneous coordinate)
     */
    Vector3 transformPoint(const Vector3& point) const {
        Vector4 v(point.x, point.y, point.z, 1.0f);
        Vector4 result = (*this) * v;
        if (result.w != 0.0f && result.w != 1.0f) {
            // Normalize homogeneous coordinate
            float invW = 1.0f / result.w;
            result *= invW;
        }
        return Vector3(result.x, result.y, result.z);
    }

    /**
     * @brief Transforms a 3D vector (ignores translation)
     */
    Vector3 transformVector(const Vector3& vec) const {
        Vector4 v(vec.x, vec.y, vec.z, 0.0f);
        Vector4 result = (*this) * v;
        return Vector3(result.x, result.y, result.z);
    }

    /**
     * @brief Transposes the matrix in-place
     */
    void transpose() {
        SimdUtils::TransposeMatrix4x4(reinterpret_cast<float*>(m));
    }

    /**
     * @brief Returns the transposed matrix
     */
    Matrix4x4 transposed() const {
        Matrix4x4 result(*this);
        result.transpose();
        return result;
    }

    /**
     * @brief Calculates the determinant of the matrix
     */
    float determinant() const {
        return SimdUtils::DeterminantMatrix4x4(reinterpret_cast<const float*>(m));
    }

    /**
     * @brief Inverts the matrix if possible
     * @return true if matrix was invertible, false otherwise
     */
    bool invert() {
        return SimdUtils::InvertMatrix4x4(
            reinterpret_cast<const float*>(m),
            reinterpret_cast<float*>(m)
        );
    }

    /**
     * @brief Returns the inverse of the matrix if possible
     * @param[out] result The inverse matrix
     * @return true if matrix was invertible, false otherwise
     */
    bool getInverse(Matrix4x4& result) const {
        return SimdUtils::InvertMatrix4x4(
            reinterpret_cast<const float*>(m),
            reinterpret_cast<float*>(result.m)
        );
    }
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_MATRIX4_HPP