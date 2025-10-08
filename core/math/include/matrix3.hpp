#ifndef PYNOVAGE_MATH_MATRIX3_HPP
#define PYNOVAGE_MATH_MATRIX3_HPP

#include "simd_utils.hpp"
#include "vector3.hpp"
#include "math_constants.hpp"
#include <algorithm>
#include <cmath>

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
class Matrix3 {
public:
    // Data storage (row-major order with SIMD-aligned blocks)
    union {
        // First layout: direct matrix access via named components (flat, padded rows)
        struct {
            float m00, m01, m02, _pad0; // First row (16-byte aligned)
            float m10, m11, m12, _pad1; // Second row (16-byte aligned)
            float m20, m21, m22, _pad2; // Third row (16-byte aligned)
        };
        
        // Second layout: SIMD-optimized array access
        alignas(16) float m[12];  // 3 rows × 4 floats (including padding)
        
        // Third layout: Row-based access (each row is 16-byte aligned)
        struct {
            alignas(16) float row0[4]; // [m00 m01 m02 _pad0]
            alignas(16) float row1[4]; // [m10 m11 m12 _pad1]
            alignas(16) float row2[4]; // [m20 m21 m22 _pad2]
        };
    };

    /**
     * @brief Default constructor, creates identity matrix
     * 
     * Note: All padding elements are initialized to 0 for consistent SIMD operations
     */
    Matrix3() {
        // First row
        m00 = 1.0f; m01 = 0.0f; m02 = 0.0f; _pad0 = 0.0f;
        // Second row
        m10 = 0.0f; m11 = 1.0f; m12 = 0.0f; _pad1 = 0.0f;
        // Third row
        m20 = 0.0f; m21 = 0.0f; m22 = 1.0f; _pad2 = 0.0f;
    }

    /**
     * @brief Constructs matrix from 9 values in row-major order
     * 
     * Note: All padding elements are initialized to 0 for consistent SIMD operations
     */
    Matrix3(float _m00, float _m01, float _m02,
            float _m10, float _m11, float _m12,
            float _m20, float _m21, float _m22) {
        // First row
        m00 = _m00; m01 = _m01; m02 = _m02; _pad0 = 0.0f;
        // Second row
        m10 = _m10; m11 = _m11; m12 = _m12; _pad1 = 0.0f;
        // Third row
        m20 = _m20; m21 = _m21; m22 = _m22; _pad2 = 0.0f;
    }
    
    /**
     * @brief Array subscript operator - provides access to matrix rows
     * 
     * @param i Row index (0-2)
     * @return Pointer to the start of the row (includes padding)
     */
float* operator[](int i) {
        if (i < 0 || i >= 3) {
            throw std::out_of_range("Matrix3 row index out of range");
        }
        switch(i) {
            case 0: return row0;
            case 1: return row1;
            case 2: return row2;
            default: return nullptr; // Should never happen due to range check
        }
    }

    const float* operator[](int i) const {
        if (i < 0 || i >= 3) {
            throw std::out_of_range("Matrix3 row index out of range");
        }
        switch(i) {
            case 0: return row0;
            case 1: return row1;
            case 2: return row2;
            default: return nullptr; // Should never happen due to range check
        }
    }
    
    /**
     * @brief Get the raw pointer to matrix data
     * @return Pointer to the first element of the matrix
     */
    float* data() { return m; }
    const float* data() const { return m; }
    
    /**
     * @brief Get pointer to a specific row's data
     * @param row Row index (0-2)
     * @return Pointer to the row's data (includes padding)
     */
    float* row_data(int row) {
        if (row < 0 || row >= 3) {
            throw std::out_of_range("Matrix3 row index out of range");
        }
        return row_data_unchecked(row);
    }
    
    const float* row_data(int row) const {
        if (row < 0 || row >= 3) {
            throw std::out_of_range("Matrix3 row index out of range");
        }
        return row_data_unchecked(row);
    }
    
    /**
     * @brief Get pointer to a specific row's data without bounds checking
     * @param row Row index (0-2)
     * @return Pointer to the row's data (includes padding)
     */
    float* row_data_unchecked(int row) { return &m[row * 4]; }
    const float* row_data_unchecked(int row) const { return &m[row * 4]; }

    /**
     * @brief Copy constructor
     */
    Matrix3(const Matrix3& other) = default;

    /**
     * @brief Assignment operator
     */
    Matrix3& operator=(const Matrix3& other) = default;

    /**
     * @brief Safely converts row/column indices to flat array index
     * @param row Row index (0-2)
     * @param col Column index (0-2)
     * @return Index into the flat array
     */
    static constexpr int idx(int row, int col) {
        return row * 4 + col;  // Each row is padded to 4 floats
    }

    /**
     * @brief Access matrix element by row and column
     * @param row Row index (0-2)
     * @param col Column index (0-2)
     * @return Reference to matrix element
     */
    float& at(int row, int col) {
        if (row < 0 || row >= 3 || col < 0 || col >= 3) {
            throw std::out_of_range("Matrix3 index out of range");
        }
        return m[idx(row, col)];
    }

    const float& at(int row, int col) const {
        if (row < 0 || row >= 3 || col < 0 || col >= 3) {
            throw std::out_of_range("Matrix3 index out of range");
        }
        return m[idx(row, col)];
    }

    /**
     * @brief Returns the identity matrix
     */
    static Matrix3 identity() {
        return Matrix3();
    }

    /**
     * @brief Creates a scaling matrix
     */
static Matrix3 scale(float sx, float sy, float sz) {
        Matrix3 result;
        result.m00 = sx;   result.m01 = 0.0f; result.m02 = 0.0f; result._pad0 = 0.0f;
        result.m10 = 0.0f; result.m11 = sy;   result.m12 = 0.0f; result._pad1 = 0.0f;
        result.m20 = 0.0f; result.m21 = 0.0f; result.m22 = sz;   result._pad2 = 0.0f;
        return result;
    }

/**
     * @brief Creates a rotation matrix around X axis
     * @param angle Rotation angle in radians
     */
static Matrix3 rotationX(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix3 result;
        result.m00 = 1.0f; result.m01 = 0.0f; result.m02 = 0.0f; result._pad0 = 0.0f;
        result.m10 = 0.0f; result.m11 = c;    result.m12 = -s;   result._pad1 = 0.0f;
        result.m20 = 0.0f; result.m21 = s;    result.m22 = c;    result._pad2 = 0.0f;
        return result;
    }

/**
     * @brief Creates a rotation matrix around Y axis
     * @param angle Rotation angle in radians
     */
static Matrix3 rotationY(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix3 result;
        result.m00 = c;    result.m01 = 0.0f; result.m02 = -s;   result._pad0 = 0.0f;
        result.m10 = 0.0f; result.m11 = 1.0f; result.m12 = 0.0f; result._pad1 = 0.0f;
        result.m20 = s;    result.m21 = 0.0f; result.m22 = c;    result._pad2 = 0.0f;
        return result;
    }

/**
     * @brief Creates a rotation matrix around Z axis
     * @param angle Rotation angle in radians
     */
static Matrix3 rotationZ(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix3 result;
        result.m00 = c;    result.m01 = -s;   result.m02 = 0.0f; result._pad0 = 0.0f;
        result.m10 = s;    result.m11 = c;    result.m12 = 0.0f; result._pad1 = 0.0f;
        result.m20 = 0.0f; result.m21 = 0.0f; result.m22 = 1.0f; result._pad2 = 0.0f;
        return result;
    }

/**
     * @brief Matrix multiplication operator
     */
Matrix3 operator*(const Matrix3& other) const {
        Matrix3 result;
        // Each row of result is computed by multiplying current matrix rows with other matrix columns
        result.m00 = m00 * other.m00 + m01 * other.m10 + m02 * other.m20;
        result.m01 = m00 * other.m01 + m01 * other.m11 + m02 * other.m21;
        result.m02 = m00 * other.m02 + m01 * other.m12 + m02 * other.m22;
        
        result.m10 = m10 * other.m00 + m11 * other.m10 + m12 * other.m20;
        result.m11 = m10 * other.m01 + m11 * other.m11 + m12 * other.m21;
        result.m12 = m10 * other.m02 + m11 * other.m12 + m12 * other.m22;
        
        result.m20 = m20 * other.m00 + m21 * other.m10 + m22 * other.m20;
        result.m21 = m20 * other.m01 + m21 * other.m11 + m22 * other.m21;
        result.m22 = m20 * other.m02 + m21 * other.m12 + m22 * other.m22;
        
        // Zero-out padding
        result._pad0 = 0.0f;
        result._pad1 = 0.0f;
        result._pad2 = 0.0f;
        return result;
    }

/**
     * @brief Matrix-vector multiplication operator
     */
    Vector3 operator*(const Vector3& v) const {
        return Vector3(
            m00 * v.x + m01 * v.y + m02 * v.z,
            m10 * v.x + m11 * v.y + m12 * v.z,
            m20 * v.x + m21 * v.y + m22 * v.z
        );
    }

    /**
     * @brief Matrix addition operator
     */
    Matrix3 operator+(const Matrix3& other) const {
        Matrix3 result;
        SimdUtils::Add4f(row0, other.row0, result.row0);
        SimdUtils::Add4f(row1, other.row1, result.row1);
        SimdUtils::Add4f(row2, other.row2, result.row2);
        return result;
    }

    /**
     * @brief Matrix subtraction operator
     */
    Matrix3 operator-(const Matrix3& other) const {
        Matrix3 result;
        SimdUtils::Subtract4f(row0, other.row0, result.row0);
        SimdUtils::Subtract4f(row1, other.row1, result.row1);
        SimdUtils::Subtract4f(row2, other.row2, result.row2);
        return result;
    }

    /**
     * @brief Matrix-scalar multiplication operator
     */
    Matrix3 operator*(float scalar) const {
        Matrix3 result;
        SimdUtils::Multiply4fScalar(row0, scalar, result.row0);
        SimdUtils::Multiply4fScalar(row1, scalar, result.row1);
        SimdUtils::Multiply4fScalar(row2, scalar, result.row2);
        return result;
    }

    /**
     * @brief Equality operator
     */
bool operator==(const Matrix3& other) const {
        const float epsilon = 1e-6f;
        
        // Compare each row's first 3 components (ignoring padding)
        if (std::abs(m00 - other.m00) > epsilon || 
            std::abs(m01 - other.m01) > epsilon || 
            std::abs(m02 - other.m02) > epsilon) {
            return false;
        }
        
        if (std::abs(m10 - other.m10) > epsilon || 
            std::abs(m11 - other.m11) > epsilon || 
            std::abs(m12 - other.m12) > epsilon) {
            return false;
        }
        
        if (std::abs(m20 - other.m20) > epsilon || 
            std::abs(m21 - other.m21) > epsilon || 
            std::abs(m22 - other.m22) > epsilon) {
            return false;
        }
        
        return true;
    }

    /**
     * @brief Inequality operator
     */
bool operator!=(const Matrix3& other) const {
        return !(*this == other);
    }

/**
     * @brief Transposes the matrix in-place
     */
void transpose() {
        std::swap(m01, m10);
        std::swap(m02, m20);
        std::swap(m12, m21);
    }

    /**
     * @brief Returns the transposed matrix
     */
Matrix3 transposed() const {
        return Matrix3(
            m00, m10, m20,
            m01, m11, m21,
            m02, m12, m22
        );
    }

/**
     * @brief Calculates the determinant of the matrix
     */
float determinant() const {
        return m00 * (m11 * m22 - m12 * m21)
             - m01 * (m10 * m22 - m12 * m20)
             + m02 * (m10 * m21 - m11 * m20);
    }

/**
     * @brief Inverts the matrix if possible
     * @return true if matrix was invertible, false otherwise
     */
bool invert() {
        float c00 = m11 * m22 - m12 * m21;
        float c01 = m12 * m20 - m10 * m22;
        float c02 = m10 * m21 - m11 * m20;
        float c10 = m02 * m21 - m01 * m22;
        float c11 = m00 * m22 - m02 * m20;
        float c12 = m01 * m20 - m00 * m21;
        float c20 = m01 * m12 - m02 * m11;
        float c21 = m02 * m10 - m00 * m12;
        float c22 = m00 * m11 - m01 * m10;

        float det = m00 * c00 + m01 * c01 + m02 * c02;
        if (std::abs(det) < 1e-6f) {
            return false;
        }
        float invDet = 1.0f / det;

        // Adjugate transpose of cofactors times 1/det
        float nm00 = c00 * invDet;
        float nm01 = c10 * invDet;
        float nm02 = c20 * invDet;
        float nm10 = c01 * invDet;
        float nm11 = c11 * invDet;
        float nm12 = c21 * invDet;
        float nm20 = c02 * invDet;
        float nm21 = c12 * invDet;
        float nm22 = c22 * invDet;

        m00 = nm00; m01 = nm01; m02 = nm02;
        m10 = nm10; m11 = nm11; m12 = nm12;
        m20 = nm20; m21 = nm21; m22 = nm22;
        _pad0 = _pad1 = _pad2 = 0.0f;
        return true;
    }

/**
     * @brief Returns the inverse of the matrix if possible
     * @param[out] result The inverse matrix
     * @return true if matrix was invertible, false otherwise
     */
bool getInverse(Matrix3& result) const {
        float c00 = m11 * m22 - m12 * m21;
        float c01 = m12 * m20 - m10 * m22;
        float c02 = m10 * m21 - m11 * m20;
        float c10 = m02 * m21 - m01 * m22;
        float c11 = m00 * m22 - m02 * m20;
        float c12 = m01 * m20 - m00 * m21;
        float c20 = m01 * m12 - m02 * m11;
        float c21 = m02 * m10 - m00 * m12;
        float c22 = m00 * m11 - m01 * m10;

        float det = m00 * c00 + m01 * c01 + m02 * c02;
        if (std::abs(det) < 1e-6f) {
            return false;
        }
        float invDet = 1.0f / det;

        result.m00 = c00 * invDet; result.m01 = c10 * invDet; result.m02 = c20 * invDet;
        result.m10 = c01 * invDet; result.m11 = c11 * invDet; result.m12 = c21 * invDet;
        result.m20 = c02 * invDet; result.m21 = c12 * invDet; result.m22 = c22 * invDet;
        result._pad0 = result._pad1 = result._pad2 = 0.0f;
        return true;
    }

    /**
     * @brief Returns a rotation matrix from axis and angle
     * @param axis Rotation axis (must be normalized)
     * @param angle Rotation angle in radians
     */
static Matrix3 fromAxisAngle(const Vector3& axis, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        float t = 1.0f - c;

        float x = axis.x;
        float y = axis.y;
        float z = axis.z;

        Matrix3 result;
        result.m00 = t*x*x + c;   result.m01 = t*x*y - s*z; result.m02 = t*x*z + s*y; result._pad0 = 0.0f;
        result.m10 = t*x*y + s*z; result.m11 = t*y*y + c;   result.m12 = t*y*z - s*x; result._pad1 = 0.0f;
        result.m20 = t*x*z - s*y; result.m21 = t*y*z + s*x; result.m22 = t*z*z + c;   result._pad2 = 0.0f;
        return result;
    }
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_MATRIX3_HPP