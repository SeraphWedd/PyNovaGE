#ifndef PYNOVAGE_MATH_MATRIX2_HPP
#define PYNOVAGE_MATH_MATRIX2_HPP

#include "simd_utils.hpp"
#include "vector2.hpp"
#include "math_constants.hpp"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <iosfwd>

namespace pynovage {
namespace math {

/**
 * @brief A SIMD-optimized 2x2 matrix class
 * 
 * Provides efficient operations for 2D transformations including:
 * - Rotations
 * - Scaling
 * - Linear transformations
 * - And more...
 */
class Matrix2 {
public:
    // Data storage (row-major order with SIMD-aligned layout)
    union {
        alignas(16) float m[4];      // Direct array access
        struct { float m00, m01,      // First row
                      m10, m11; };    // Second row
    };

    /**
     * @brief Default constructor, creates identity matrix
     */
    Matrix2() { setIdentity(); }

    /**
     * @brief Constructs matrix from 4 values in row-major order
     */
    Matrix2(float m00, float m01, float m10, float m11) {
        m[0] = m00; m[1] = m01;
        m[2] = m10; m[3] = m11;
    }

    /**
     * @brief Returns the identity matrix
     */
    static Matrix2 identity() {
        return Matrix2(1.0f, 0.0f, 0.0f, 1.0f);
    }
    /**
     * @brief Creates a rotation matrix
     * @param radians Rotation angle in radians
     */
    static Matrix2 rotation(float radians) {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return Matrix2(c, -s, s, c);
    }
    static Matrix2 scale(float sx, float sy) {
        return Matrix2(sx, 0.0f, 0.0f, sy);
    }
    
    /**
     * @brief Performs batch matrix multiplication of multiple 2x2 matrices
     * @param matrices Array of 2x2 matrices to multiply
     * @param count Number of matrices in the array
     * @return Product of all matrices in order
     */
    static Matrix2 batchMultiply(const Matrix2* matrices, size_t count) {
        if (count == 0) return Matrix2::identity();
        if (count == 1) return matrices[0];
        
        Matrix2 result = matrices[0];
        for (size_t i = 1; i < count; ++i) {
            result = result * matrices[i];
        }
        return result;
    }
    
    /**
     * @brief Extracts the scale components from the matrix
     * @return Vector2 containing scale for x and y axes
     */
    Vector2 extractScale() const {
        return Vector2(
            std::sqrt(m00 * m00 + m10 * m10),  // x scale
            std::sqrt(m01 * m01 + m11 * m11)   // y scale
        );
    }
    
    /**
     * @brief Extracts the rotation angle from the matrix
     * @return Rotation angle in radians
     */
    float extractRotation() const {
        Vector2 scale = extractScale();
        float invScaleX = 1.0f / scale.x;
        float invScaleY = 1.0f / scale.y;
        
        // Remove scale to get pure rotation
        float cos_theta = m00 * invScaleX;
        float sin_theta = m10 * invScaleX;
        
        return std::atan2(sin_theta, cos_theta);
    }
    
    /**
     * @brief Linear interpolation between two matrices
     * @param a First matrix
     * @param b Second matrix
     * @param t Interpolation factor (0-1)
     */
    static Matrix2 lerp(const Matrix2& a, const Matrix2& b, float t) {
        // Extract components
        Vector2 scaleA = a.extractScale();
        float rotA = a.extractRotation();
        
        Vector2 scaleB = b.extractScale();
        float rotB = b.extractRotation();
        
        // Interpolate scale and rotation
        Vector2 scale = Vector2::lerp(scaleA, scaleB, t);
        float rot = rotA + t * (rotB - rotA);
        
        // Reconstruct matrix
        return Matrix2::scale(scale.x, scale.y) * Matrix2::rotation(rot);
    }

    void setIdentity() {
        m00 = 1.0f; m01 = 0.0f;
        m10 = 0.0f; m11 = 1.0f;
    }

    // Component-wise addition
    Matrix2 operator+(const Matrix2& other) const {
        Matrix2 result;
        SimdUtils::Add4f(m, other.m, result.m);
        return result;
    }

    Matrix2& operator+=(const Matrix2& other) {
        SimdUtils::Add4f(m, other.m, m);
        return *this;
    }

    // Component-wise subtraction
    Matrix2 operator-(const Matrix2& other) const {
        Matrix2 result;
        SimdUtils::Subtract4f(m, other.m, result.m);
        return result;
    }

    Matrix2& operator-=(const Matrix2& other) {
        SimdUtils::Subtract4f(m, other.m, m);
        return *this;
    }

    Matrix2 operator*(const Matrix2& other) const {
        Matrix2 r;
        SimdUtils::MultiplyMatrix2x2(m, other.m, r.m);
        return r;
    }

    Vector2 operator*(const Vector2& v) const {
        Vector2 r;
        SimdUtils::MultiplyMatrix2x2Vec2(m, &v.x, &r.x);
        return r;
    }

    float determinant() const { return SimdUtils::DeterminantMatrix2x2(m); }

    bool inverse(Matrix2& out) const { return SimdUtils::InvertMatrix2x2(m, out.m); }

    void transposeInPlace() { SimdUtils::TransposeMatrix2x2(m); }

    Matrix2 transposed() const {
        Matrix2 r(*this);
        r.transposeInPlace();
        return r;
    }

    // Array subscript operators with bounds checking
    float* operator[](int row) {
        if (row < 0 || row >= 2) {
            throw std::out_of_range("Matrix2 row index out of range");
        }
        return &m[row * 2];
    }

    const float* operator[](int row) const {
        if (row < 0 || row >= 2) {
            throw std::out_of_range("Matrix2 row index out of range");
        }
        return &m[row * 2];
    }

    // Basic operators
    bool operator==(const Matrix2& other) const {
        const float epsilon = 1e-6f;
        return std::abs(m[0] - other.m[0]) < epsilon &&
               std::abs(m[1] - other.m[1]) < epsilon &&
               std::abs(m[2] - other.m[2]) < epsilon &&
               std::abs(m[3] - other.m[3]) < epsilon;
    }

    bool operator!=(const Matrix2& other) const {
        return !(*this == other);
    }

    // Matrix-scalar operations using SIMD
    Matrix2 operator*(float scalar) const {
        Matrix2 result;
        SimdUtils::Multiply4fScalar(m, scalar, result.m);
        return result;
    }

    Matrix2& operator*=(float scalar) {
        SimdUtils::Multiply4fScalar(m, scalar, m);
        return *this;
    }

    // Component-wise operations using SIMD
    Matrix2 cwiseProduct(const Matrix2& other) const {
        Matrix2 result;
        SimdUtils::Multiply4f(m, other.m, result.m);
        return result;
    }

    Matrix2 cwiseQuotient(const Matrix2& other) const {
        Matrix2 result;
        SimdUtils::Divide4f(m, other.m, result.m);
        return result;
    }

    // String conversion
    std::string toString() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);
        ss << "[" << m[0] << ", " << m[1] << "]\n";
        ss << "[" << m[2] << ", " << m[3] << "]";
        return ss.str();
    }
};

// Stream operators
inline std::ostream& operator<<(std::ostream& os, const Matrix2& m) {
    os << m.toString();
    return os;
}

inline std::istream& operator>>(std::istream& is, Matrix2& m) {
    char dummy;
    float x0, x1, y0, y1;

    // Expect '[' then two numbers separated by ',' then ']'
    is >> dummy;        // '['
    is >> x0 >> dummy >> x1; // number , number
    is >> dummy;        // ']'

    // Consume optional newline/whitespace and next '['
    is >> std::ws;
    is >> dummy;        // '['

    // Second row: two numbers then ']'
    is >> y0 >> dummy >> y1;
    is >> dummy;        // ']'

    m = Matrix2(x0, x1, y0, y1);
    return is;
}

// Scalar multiplication
inline Matrix2 operator*(float scalar, const Matrix2& mat) {
    return mat * scalar;
}

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_MATRIX2_HPP