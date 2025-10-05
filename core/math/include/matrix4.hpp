#ifndef PYNOVAGE_MATH_MATRIX4_HPP
#define PYNOVAGE_MATH_MATRIX4_HPP

#include "simd_utils.hpp"
#include "vector4.hpp"
#include "vector3.hpp"
#include "math_constants.hpp"
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <ostream>

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

    // Array subscript operators
    float* operator[](int row) { return m[row]; }
    const float* operator[](int row) const { return m[row]; }

    // Comparison operators
    bool operator==(const Matrix4x4& other) const {
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                if(m[i][j] != other.m[i][j]) return false;
            }
        }
        return true;
    }

    bool operator!=(const Matrix4x4& other) const {
        return !(*this == other);
    }

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

    // Compound assignment operators
    Matrix4x4& operator*=(const Matrix4x4& other) {
        *this = *this * other;
        return *this;
    }

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

    // String formatting
    std::string toString() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);
        for (int i = 0; i < 4; ++i) {
            ss << "[";
            for (int j = 0; j < 4; ++j) {
                ss << m[i][j];
                if (j < 3) ss << ", ";
            }
            ss << "]";
            if (i < 3) ss << "\n";
        }
        return ss.str();
    }

    // Additional static factory methods
    /**
     * @brief Creates a view matrix looking from 'eye' towards 'target'
     * @param eye Camera position
     * @param target Point to look at
     * @param up Up vector
     */
    static Matrix4x4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        Vector3 zaxis = (target - eye).normalized();  // Forward
        Vector3 xaxis = up.cross(zaxis).normalized(); // Right
        Vector3 yaxis = zaxis.cross(xaxis);          // Up

        Matrix4x4 rotation(
            xaxis.x, xaxis.y, xaxis.z, 0.0f,
            yaxis.x, yaxis.y, yaxis.z, 0.0f,
            zaxis.x, zaxis.y, zaxis.z, 0.0f,
            0.0f,    0.0f,    0.0f,    1.0f
        );

        Matrix4x4 translation = Translation(-eye.x, -eye.y, -eye.z);
        return rotation * translation;
    }

    /**
     * @brief Creates a perspective projection matrix
     * @param fovY Vertical field of view in radians
     * @param aspect Aspect ratio (width/height)
     * @param near Near clipping plane
     * @param far Far clipping plane
     */
    static Matrix4x4 Perspective(float fovY, float aspect, float near, float far) {
        using namespace constants;
        float tanHalfFovY = std::tan(fovY / 2.0f);
        float f = 1.0f / tanHalfFovY;
        float nf = 1.0f / (near - far);

        return Matrix4x4(
            f/aspect,   0.0f,       0.0f,                         0.0f,
            0.0f,       f,          0.0f,                         0.0f,
            0.0f,       0.0f,       (far+near)*nf,                (2.0f*far*near)*nf,
            0.0f,       0.0f,       -1.0f,                        0.0f
        );
    }

    /**
     * @brief Creates an orthographic projection matrix
     * @param left Left clipping plane
     * @param right Right clipping plane
     * @param bottom Bottom clipping plane
     * @param top Top clipping plane
     * @param near Near clipping plane
     * @param far Far clipping plane
     */
    static Matrix4x4 Orthographic(float left, float right, float bottom, float top, float near, float far) {
        float rml = right - left;
        float tmb = top - bottom;
        float fmn = far - near;

        return Matrix4x4(
            2.0f/rml,   0.0f,       0.0f,        -(right+left)/rml,
            0.0f,       2.0f/tmb,   0.0f,        -(top+bottom)/tmb,
            0.0f,       0.0f,       -2.0f/fmn,   -(far+near)/fmn,
            0.0f,       0.0f,       0.0f,        1.0f
        );
    }

    /**
     * @brief Creates a rotation matrix from axis and angle
     * @param axis Rotation axis (should be normalized)
     * @param angle Rotation angle in radians
     */
    static Matrix4x4 RotationAxis(const Vector3& axis, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        float t = 1.0f - c;

        return Matrix4x4(
            t*axis.x*axis.x + c,      t*axis.x*axis.y - s*axis.z, t*axis.x*axis.z + s*axis.y, 0.0f,
            t*axis.x*axis.y + s*axis.z, t*axis.y*axis.y + c,      t*axis.y*axis.z - s*axis.x, 0.0f,
            t*axis.x*axis.z - s*axis.y, t*axis.y*axis.z + s*axis.x, t*axis.z*axis.z + c,      0.0f,
            0.0f,                      0.0f,                      0.0f,                      1.0f
        );
    }

    /**
     * @brief Creates a rotation matrix from Euler angles (YXZ order)
     * @param yaw Rotation around Y axis (radians)
     * @param pitch Rotation around X axis (radians)
     * @param roll Rotation around Z axis (radians)
     */
    static Matrix4x4 FromEulerAngles(float yaw, float pitch, float roll) {
        return RotationY(yaw) * RotationX(pitch) * RotationZ(roll);
    }
};

// Stream operators
inline std::ostream& operator<<(std::ostream& os, const Matrix4x4& m) {
    os << m.toString();
    return os;
}

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_MATRIX4_HPP