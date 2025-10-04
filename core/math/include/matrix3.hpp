#ifndef PYNOVAGE_MATH_MATRIX3_HPP
#define PYNOVAGE_MATH_MATRIX3_HPP

#include "simd_utils.hpp"
#include "vector3.hpp"
#include "math_constants.hpp"

namespace pynovage {
namespace math {

/**
 * @brief A SIMD-optimized 3x3 matrix class
 * 
 * Provides efficient operations for 3D transformations including:
 * - Rotations
 * - Scaling
 * - Linear transformations
 * - And more...
 */
class Matrix3x3 {
public:
    // Data storage (row-major order for easier SIMD operations)
    alignas(16) float m[3][3];

    /**
     * @brief Default constructor, creates identity matrix
     */
    Matrix3x3() {
        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
    }

    /**
     * @brief Constructs matrix from 9 values in row-major order
     */
    Matrix3x3(float m00, float m01, float m02,
              float m10, float m11, float m12,
              float m20, float m21, float m22) {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
    }

    /**
     * @brief Copy constructor
     */
    Matrix3x3(const Matrix3x3& other) = default;

    /**
     * @brief Assignment operator
     */
    Matrix3x3& operator=(const Matrix3x3& other) = default;

    /**
     * @brief Returns the identity matrix
     */
    static Matrix3x3 Identity() {
        return Matrix3x3();
    }

    /**
     * @brief Creates a scaling matrix
     */
    static Matrix3x3 Scale(float sx, float sy, float sz) {
        return Matrix3x3(
            sx,  0.0f, 0.0f,
            0.0f, sy,   0.0f,
            0.0f, 0.0f, sz
        );
    }

    /**
     * @brief Creates a rotation matrix around X axis
     * @param angle Rotation angle in radians
     */
    static Matrix3x3 RotationX(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Matrix3x3(
            1.0f, 0.0f, 0.0f,
            0.0f, c,    -s,
            0.0f, s,    c
        );
    }

    /**
     * @brief Creates a rotation matrix around Y axis
     * @param angle Rotation angle in radians
     */
    static Matrix3x3 RotationY(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Matrix3x3(
            c,    0.0f, -s,
            0.0f, 1.0f, 0.0f,
            s,    0.0f, c
        );
    }

    /**
     * @brief Creates a rotation matrix around Z axis
     * @param angle Rotation angle in radians
     */
    static Matrix3x3 RotationZ(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Matrix3x3(
            c,    -s,   0.0f,
            s,    c,    0.0f,
            0.0f, 0.0f, 1.0f
        );
    }

    /**
     * @brief Matrix multiplication operator
     */
    Matrix3x3 operator*(const Matrix3x3& other) const {
        Matrix3x3 result;
        SimdUtils::MultiplyMatrix3x3(
            reinterpret_cast<const float*>(m),
            reinterpret_cast<const float*>(other.m),
            reinterpret_cast<float*>(result.m)
        );
        return result;
    }

    /**
     * @brief Matrix-vector multiplication operator
     */
    Vector3 operator*(const Vector3& v) const {
        Vector3 result;
        SimdUtils::MultiplyMatrix3x3Vec3(
            reinterpret_cast<const float*>(m),
            reinterpret_cast<const float*>(&v),
            reinterpret_cast<float*>(&result)
        );
        return result;
    }

    /**
     * @brief Matrix addition operator
     */
    Matrix3x3 operator+(const Matrix3x3& other) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.m[i][j] = m[i][j] + other.m[i][j];
            }
        }
        return result;
    }

    /**
     * @brief Matrix subtraction operator
     */
    Matrix3x3 operator-(const Matrix3x3& other) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.m[i][j] = m[i][j] - other.m[i][j];
            }
        }
        return result;
    }

    /**
     * @brief Matrix-scalar multiplication operator
     */
    Matrix3x3 operator*(float scalar) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.m[i][j] = m[i][j] * scalar;
            }
        }
        return result;
    }

    /**
     * @brief Equality operator
     */
    bool operator==(const Matrix3x3& other) const {
        const float epsilon = 1e-6f;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (std::abs(m[i][j] - other.m[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * @brief Inequality operator
     */
    bool operator!=(const Matrix3x3& other) const {
        return !(*this == other);
    }

    /**
     * @brief Transposes the matrix in-place
     */
    void Transpose() {
        SimdUtils::TransposeMatrix3x3(reinterpret_cast<float*>(m));
    }

    /**
     * @brief Returns the transposed matrix
     */
    Matrix3x3 Transposed() const {
        Matrix3x3 result(*this);
        result.Transpose();
        return result;
    }

    /**
     * @brief Calculates the determinant of the matrix
     */
    float Determinant() const {
        return SimdUtils::DeterminantMatrix3x3(reinterpret_cast<const float*>(m));
    }

    /**
     * @brief Inverts the matrix if possible
     * @return true if matrix was invertible, false otherwise
     */
    bool Invert() {
        return SimdUtils::InvertMatrix3x3(
            reinterpret_cast<const float*>(m),
            reinterpret_cast<float*>(m)
        );
    }

    /**
     * @brief Returns the inverse of the matrix if possible
     * @param[out] result The inverse matrix
     * @return true if matrix was invertible, false otherwise
     */
    bool GetInverse(Matrix3x3& result) const {
        return SimdUtils::InvertMatrix3x3(
            reinterpret_cast<const float*>(m),
            reinterpret_cast<float*>(result.m)
        );
    }

    /**
     * @brief Returns a rotation matrix from axis and angle
     * @param axis Rotation axis (must be normalized)
     * @param angle Rotation angle in radians
     */
    static Matrix3x3 FromAxisAngle(const Vector3& axis, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        float t = 1.0f - c;

        float x = axis.x;
        float y = axis.y;
        float z = axis.z;

        return Matrix3x3(
            t*x*x + c,   t*x*y - s*z, t*x*z + s*y,
            t*x*y + s*z, t*y*y + c,   t*y*z - s*x,
            t*x*z - s*y, t*y*z + s*x, t*z*z + c
        );
    }
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_MATRIX3_HPP