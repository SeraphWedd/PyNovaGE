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
    // Data storage (row-major order for easier SIMD operations)
    alignas(16) float m[4];

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
    static Matrix2 scale(float sx, float sy) { return Matrix2(sx,0, 0,sy); }

    void setIdentity() { m[0]=1; m[1]=0; m[2]=0; m[3]=1; }

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

    // Matrix-scalar operations
    Matrix2 operator*(float scalar) const {
        Matrix2 result;
        for (int i = 0; i < 4; ++i) {
            result.m[i] = m[i] * scalar;
        }
        return result;
    }

    Matrix2& operator*=(float scalar) {
        for (int i = 0; i < 4; ++i) {
            m[i] *= scalar;
        }
        return *this;
    }

    // Component-wise operations
    Matrix2 cwiseProduct(const Matrix2& other) const {
        Matrix2 result;
        for (int i = 0; i < 4; ++i) {
            result.m[i] = m[i] * other.m[i];
        }
        return result;
    }

    Matrix2 cwiseQuotient(const Matrix2& other) const {
        Matrix2 result;
        for (int i = 0; i < 4; ++i) {
            result.m[i] = m[i] / other.m[i];
        }
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