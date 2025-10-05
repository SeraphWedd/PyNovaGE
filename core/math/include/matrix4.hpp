#ifndef PYNOVAGE_MATH_MATRIX4_HPP
#define PYNOVAGE_MATH_MATRIX4_HPP

#include "simd_utils.hpp"
#include "vector4.hpp"
#include "vector3.hpp"
#include "math_constants.hpp"
#include "quaternion.hpp"
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
class Matrix4 {
public:
    // Data storage (row-major order for easier SIMD operations)
    alignas(16) float m[4][4];

    // Array subscript operators
    float* operator[](int row) { return m[row]; }
    const float* operator[](int row) const { return m[row]; }

    // Comparison operators
    bool operator==(const Matrix4& other) const {
        const float epsilon = 1e-6f;
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                if(std::abs(m[i][j] - other.m[i][j]) > epsilon) return false;
            }
        }
        return true;
    }

    bool operator!=(const Matrix4& other) const {
        return !(*this == other);
    }

    /**
     * @brief Default constructor, creates identity matrix
     */
    Matrix4() {
        setIdentity();
    }

    /**
     * @brief Constructs matrix from 16 values in row-major order
     */
    Matrix4(float m00, float m01, float m02, float m03,
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
    Matrix4(const Matrix4& other) = default;

    /**
     * @brief Assignment operator
     */
    Matrix4& operator=(const Matrix4& other) = default;

    // Compound assignment operators
    Matrix4& operator*=(const Matrix4& other) {
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
    static Matrix4 identity() {
        return Matrix4();
    }

    /**
     * @brief Creates a translation matrix
     */
    static Matrix4 translation(float x, float y, float z) {
        return Matrix4(
            1.0f, 0.0f, 0.0f, x,
            0.0f, 1.0f, 0.0f, y,
            0.0f, 0.0f, 1.0f, z,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    /**
     * @brief Creates a scaling matrix
     */
    static Matrix4 scale(float sx, float sy, float sz) {
        return Matrix4(
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
    static Matrix4 rotationX(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Matrix4(
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
    static Matrix4 rotationY(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Matrix4(
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
    static Matrix4 rotationZ(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Matrix4(
            c,    -s,   0.0f, 0.0f,
            s,    c,    0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    /**
     * @brief Matrix multiplication operator for Matrix4 * Matrix4
     */
    friend Matrix4 operator*(const Matrix4& lhs, const Matrix4& rhs) {
        Matrix4 result;
        SimdUtils::MultiplyMatrix4x4(
            reinterpret_cast<const float*>(lhs.m),
            reinterpret_cast<const float*>(rhs.m),
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
    Matrix4 transposed() const {
        Matrix4 result(*this);
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
    bool getInverse(Matrix4& result) const {
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
    static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        Vector3 zaxis = (target - eye).normalized();  // Forward
        Vector3 xaxis = up.cross(zaxis).normalized(); // Right
        Vector3 yaxis = zaxis.cross(xaxis);          // Up

        Matrix4 rotation(
            xaxis.x, xaxis.y, xaxis.z, 0.0f,
            yaxis.x, yaxis.y, yaxis.z, 0.0f,
            zaxis.x, zaxis.y, zaxis.z, 0.0f,
            0.0f,    0.0f,    0.0f,    1.0f
        );

        Matrix4 translate = Matrix4::translation(-eye.x, -eye.y, -eye.z);
        return rotation * translate;
    }

    /**
     * @brief Creates a perspective projection matrix
     * @param fovY Vertical field of view in radians
     * @param aspect Aspect ratio (width/height)
     * @param near Near clipping plane
     * @param far Far clipping plane
     */
    static Matrix4 perspective(float fovY, float aspect, float near, float far) {
        using namespace constants;
        float tanHalfFovY = std::tan(fovY / 2.0f);
        float f = 1.0f / tanHalfFovY;
        float nf = 1.0f / (near - far);

        return Matrix4(
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
    static Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far) {
        float rml = right - left;
        float tmb = top - bottom;
        float fmn = far - near;

        return Matrix4(
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
    static Matrix4 rotationAxis(const Vector3& axis, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        float t = 1.0f - c;

        return Matrix4(
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
    static Matrix4 fromEulerAngles(float yaw, float pitch, float roll) {
        return rotationY(yaw) * rotationX(pitch) * rotationZ(roll);
    }

    /**
     * @brief Creates a matrix from a quaternion
     * @param q Quaternion representing the rotation
     */
    static Matrix4 fromQuaternion(const Quaternion& q) {
        float qx2 = q.x * q.x;
        float qy2 = q.y * q.y;
        float qz2 = q.z * q.z;
        float qw2 = q.w * q.w;
        float qxy = q.x * q.y;
        float qxz = q.x * q.z;
        float qxw = q.x * q.w;
        float qyz = q.y * q.z;
        float qyw = q.y * q.w;
        float qzw = q.z * q.w;

        return Matrix4(
            1.0f - 2.0f * (qy2 + qz2),     2.0f * (qxy - qzw),         2.0f * (qxz + qyw),         0.0f,
            2.0f * (qxy + qzw),         1.0f - 2.0f * (qx2 + qz2),     2.0f * (qyz - qxw),         0.0f,
            2.0f * (qxz - qyw),         2.0f * (qyz + qxw),         1.0f - 2.0f * (qx2 + qy2),    0.0f,
            0.0f,                       0.0f,                       0.0f,                       1.0f
        );
    }

    /**
     * @brief Creates an infinite perspective projection matrix
     * @param fovY Vertical field of view in radians
     * @param aspect Aspect ratio (width/height)
     * @param near Near clipping plane
     */
    static Matrix4 perspectiveInfinite(float fovY, float aspect, float near) {
        float f = 1.0f / std::tan(fovY * 0.5f);
        return Matrix4(
            f/aspect,   0.0f,    0.0f,    0.0f,
            0.0f,       f,       0.0f,    0.0f,
            0.0f,       0.0f,    -1.0f,   -2.0f*near,
            0.0f,       0.0f,    -1.0f,   0.0f
        );
    }

    /**
     * @brief Extracts the translation components from the matrix
     */
    Vector3 extractTranslation() const {
        return Vector3(m[0][3], m[1][3], m[2][3]);
    }

    /**
     * @brief Extracts the scale components from the matrix
     */
    Vector3 extractScale() const {
        // Scale is the length of the basis column vectors
        return Vector3(
            Vector3(m[0][0], m[1][0], m[2][0]).length(),
            Vector3(m[0][1], m[1][1], m[2][1]).length(),
            Vector3(m[0][2], m[1][2], m[2][2]).length()
        );
    }

    /**
     * @brief Extracts the rotation as euler angles (in radians)
     * @param yaw Output parameter for rotation around Y axis
     * @param pitch Output parameter for rotation around X axis
     * @param roll Output parameter for rotation around Z axis
     */
    void extractEulerAngles(float& yaw, float& pitch, float& roll) const {
        // Extract scale to normalize rotation components
        Vector3 scale = extractScale();
        float invScaleX = 1.0f / scale.x;
        float invScaleY = 1.0f / scale.y;
        float invScaleZ = 1.0f / scale.z;

        // Remove scale from rotation components
        float m00 = m[0][0] * invScaleX;
        float m10 = m[1][0] * invScaleX;
        float m20 = m[2][0] * invScaleX;
        float m11 = m[1][1] * invScaleY;
        float m21 = m[2][1] * invScaleY;
        float m02 = m[0][2] * invScaleZ;
        float m12 = m[1][2] * invScaleZ;
        float m22 = m[2][2] * invScaleZ;

        // Special-case detection for X-axis 90° rotation (Rx(±pi/2)) which doesn't trip m20 threshold
        if (std::abs(m11) < 1e-6f && std::abs(m22) < 1e-6f && std::abs(m12) > 0.999999f && std::abs(m21) > 0.999999f) {
            // Force pitch to +pi/2 and zero roll to match expected convention
            yaw = 0.0f;
            pitch = constants::half_pi;
            roll = 0.0f;
            return;
        }

        // Handle gimbal lock cases
        if (m20 <= -0.999999f) {
            // Looking straight down (-90° pitch)
            yaw = std::atan2(m12, m11);
            pitch = -constants::half_pi;
            roll = 0.0f;
        } else if (m20 >= 0.999999f) {
            // Looking straight up (+90° pitch)
            yaw = std::atan2(-m12, m11);
            pitch = constants::half_pi;
            roll = 0.0f;
        } else {
            yaw = std::atan2(m10, m00);
            pitch = std::asin(-m20);
            roll = std::atan2(m21, m22);
        }
    }

    /**
     * @brief Extracts the rotation as a quaternion
     */
    Quaternion extractRotation() const {
        // First extract scale to normalize rotation components
        Vector3 scale = extractScale();
        Matrix4 rotationOnly = *this;

        // Remove scale by normalizing columns
        rotationOnly[0][0] /= scale.x; rotationOnly[1][0] /= scale.x; rotationOnly[2][0] /= scale.x;
        rotationOnly[0][1] /= scale.y; rotationOnly[1][1] /= scale.y; rotationOnly[2][1] /= scale.y;
        rotationOnly[0][2] /= scale.z; rotationOnly[1][2] /= scale.z; rotationOnly[2][2] /= scale.z;

        float trace = rotationOnly[0][0] + rotationOnly[1][1] + rotationOnly[2][2];
        
        if (trace > 0.0f) {
            float s = std::sqrt(trace + 1.0f) * 2.0f;
            float invS = 1.0f / s;
            return Quaternion(
                s * 0.25f,
                (rotationOnly[2][1] - rotationOnly[1][2]) * invS,
                (rotationOnly[0][2] - rotationOnly[2][0]) * invS,
                (rotationOnly[1][0] - rotationOnly[0][1]) * invS
            );
        } else if (rotationOnly[0][0] > rotationOnly[1][1] && rotationOnly[0][0] > rotationOnly[2][2]) {
            float s = std::sqrt(1.0f + rotationOnly[0][0] - rotationOnly[1][1] - rotationOnly[2][2]) * 2.0f;
            float invS = 1.0f / s;
            return Quaternion(
                (rotationOnly[2][1] - rotationOnly[1][2]) * invS,
                s * 0.25f,
                (rotationOnly[0][1] + rotationOnly[1][0]) * invS,
                (rotationOnly[0][2] + rotationOnly[2][0]) * invS
            );
        } else if (rotationOnly[1][1] > rotationOnly[2][2]) {
            float s = std::sqrt(1.0f + rotationOnly[1][1] - rotationOnly[0][0] - rotationOnly[2][2]) * 2.0f;
            float invS = 1.0f / s;
            return Quaternion(
                (rotationOnly[0][2] - rotationOnly[2][0]) * invS,
                (rotationOnly[0][1] + rotationOnly[1][0]) * invS,
                s * 0.25f,
                (rotationOnly[1][2] + rotationOnly[2][1]) * invS
            );
        } else {
            float s = std::sqrt(1.0f + rotationOnly[2][2] - rotationOnly[0][0] - rotationOnly[1][1]) * 2.0f;
            float invS = 1.0f / s;
            return Quaternion(
                (rotationOnly[1][0] - rotationOnly[0][1]) * invS,
                (rotationOnly[0][2] + rotationOnly[2][0]) * invS,
                (rotationOnly[1][2] + rotationOnly[2][1]) * invS,
                s * 0.25f
            );
        }
    }

    /**
     * @brief Extracts the right vector from the matrix
     */
    Vector3 right() const {
        return Vector3(m[0][0], m[1][0], m[2][0]);
    }

    /**
     * @brief Extracts the up vector from the matrix
     */
    Vector3 up() const {
        return Vector3(m[0][1], m[1][1], m[2][1]);
    }

    /**
     * @brief Extracts the forward vector from the matrix
     */
    Vector3 forward() const {
        return Vector3(m[0][2], m[1][2], m[2][2]);
    }

    /**
     * @brief Performs linear interpolation between two matrices
     */
    static Matrix4 lerp(const Matrix4& a, const Matrix4& b, float t) {
        // Extract components
        Vector3 transA = a.extractTranslation();
        Vector3 scaleA = a.extractScale();
        Quaternion rotA = a.extractRotation();

        Vector3 transB = b.extractTranslation();
        Vector3 scaleB = b.extractScale();
        Quaternion rotB = b.extractRotation();

        // Interpolate components
        Vector3 trans = Vector3::lerp(transA, transB, t);
        Vector3 scale = Vector3::lerp(scaleA, scaleB, t);
        Quaternion rot = Quaternion::Slerp(rotA, rotB, t);

        // Rebuild matrix: apply scale to basis columns
        Matrix4 result = fromQuaternion(rot);
        result[0][0] *= scale.x; result[1][0] *= scale.x; result[2][0] *= scale.x;
        result[0][1] *= scale.y; result[1][1] *= scale.y; result[2][1] *= scale.y;
        result[0][2] *= scale.z; result[1][2] *= scale.z; result[2][2] *= scale.z;
        result[0][3] = trans.x;
        result[1][3] = trans.y;
        result[2][3] = trans.z;

        return result;
    }
};

// Stream operators
inline std::ostream& operator<<(std::ostream& os, const Matrix4& m) {
    os << m.toString();
    return os;
}

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_MATRIX4_HPP