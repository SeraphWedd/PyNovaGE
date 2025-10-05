#ifndef PYNOVAGE_MATH_TEMPLATE_OPERATIONS_HPP
#define PYNOVAGE_MATH_TEMPLATE_OPERATIONS_HPP

#include "math_constants.hpp"
#include "simd_utils.hpp"
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>

namespace pynovage {
namespace math {

/**
 * @brief Template for vector operations with SIMD optimization
 * 
 * This template demonstrates standard vector operations that should
 * be implemented consistently across Vector2, Vector3, and Vector4.
 *
 * Performance Characteristics:
 * - SIMD optimized where available
 * - Cache-aligned data structures
 * - Vectorized operations for bulk processing
 *
 * @tparam T Component type (usually float)
 * @tparam N Vector dimension (2, 3, or 4)
 */
template<typename T, size_t N>
class VectorTemplate {
public:
    // Static assertions
    static_assert(N >= 2 && N <= 4, "Vector dimension must be 2, 3, or 4");
    static_assert(std::is_floating_point<T>::value, "Vector type must be floating point");

    // SIMD-aligned storage
    alignas(16) T data[N];

    /**
     * @brief Default constructor, initializes to zero
     */
    VectorTemplate() {
        for (size_t i = 0; i < N; ++i) {
            data[i] = T(0);
        }
    }

    /**
     * @brief Constructs vector from components
     * @param values Array of component values
     * @throw std::invalid_argument if values is null
     */
    explicit VectorTemplate(const T* values) {
        if (!values) {
            throw std::invalid_argument("Null pointer provided to Vector constructor");
        }
        for (size_t i = 0; i < N; ++i) {
            data[i] = values[i];
        }
    }

    /**
     * @brief Array subscript operator with bounds checking
     * @param index Component index
     * @return Reference to component
     * @throw std::out_of_range if index is invalid
     */
    T& operator[](size_t index) {
        if (index >= N) {
            throw std::out_of_range("Vector index out of range");
        }
        return data[index];
    }

    /**
     * @brief Const array subscript operator with bounds checking
     * @param index Component index
     * @return Const reference to component
     * @throw std::out_of_range if index is invalid
     */
    const T& operator[](size_t index) const {
        if (index >= N) {
            throw std::out_of_range("Vector index out of range");
        }
        return data[index];
    }

    /**
     * @brief Calculates dot product with another vector
     * @param other Vector to calculate dot product with
     * @return Dot product result
     */
    T dot(const VectorTemplate& other) const {
        T result = T(0);
        for (size_t i = 0; i < N; ++i) {
            result += data[i] * other.data[i];
        }
        return result;
    }

    /**
     * @brief Calculates vector length (magnitude)
     * @return Vector length
     */
    T length() const {
        return std::sqrt(lengthSquared());
    }

    /**
     * @brief Calculates squared vector length
     * @return Squared vector length
     */
    T lengthSquared() const {
        return dot(*this);
    }

    /**
     * @brief Normalizes vector to unit length
     * @return Reference to this vector
     */
    VectorTemplate& normalize() {
        T len = length();
        if (len > T(0)) {
            T invLen = T(1) / len;
            for (size_t i = 0; i < N; ++i) {
                data[i] *= invLen;
            }
        }
        return *this;
    }

    /**
     * @brief Returns normalized copy of vector
     * @return Normalized vector
     */
    VectorTemplate normalized() const {
        VectorTemplate result(*this);
        result.normalize();
        return result;
    }

    /**
     * @brief Checks if vector is zero
     * @return true if all components are zero
     */
    bool isZero() const {
        for (size_t i = 0; i < N; ++i) {
            if (data[i] != T(0)) return false;
        }
        return true;
    }

    /**
     * @brief Sets vector to zero
     */
    void setZero() {
        for (size_t i = 0; i < N; ++i) {
            data[i] = T(0);
        }
    }

    /**
     * @brief Calculates distance to another vector
     * @param other Vector to calculate distance to
     * @return Distance between vectors
     */
    T distanceTo(const VectorTemplate& other) const {
        return (*this - other).length();
    }

    /**
     * @brief Calculates squared distance to another vector
     * @param other Vector to calculate squared distance to
     * @return Squared distance between vectors
     */
    T distanceSquaredTo(const VectorTemplate& other) const {
        return (*this - other).lengthSquared();
    }

    /**
     * @brief Linear interpolation between vectors
     * @param a Start vector
     * @param b End vector
     * @param t Interpolation parameter [0,1]
     * @return Interpolated vector
     */
    static VectorTemplate lerp(const VectorTemplate& a, const VectorTemplate& b, T t) {
        return a + (b - a) * t;
    }

    /**
     * @brief Converts vector to string representation
     * @return String representation of vector
     */
    std::string toString() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3) << "(";
        for (size_t i = 0; i < N; ++i) {
            if (i > 0) ss << ", ";
            ss << data[i];
        }
        ss << ")";
        return ss.str();
    }

    // Arithmetic operators
    VectorTemplate operator+(const VectorTemplate& other) const {
        VectorTemplate result;
        for (size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] + other.data[i];
        }
        return result;
    }

    VectorTemplate operator-(const VectorTemplate& other) const {
        VectorTemplate result;
        for (size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] - other.data[i];
        }
        return result;
    }

    VectorTemplate operator*(T scalar) const {
        VectorTemplate result;
        for (size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] * scalar;
        }
        return result;
    }

    VectorTemplate operator/(T scalar) const {
        VectorTemplate result;
        for (size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] / scalar;
        }
        return result;
    }

    // Compound assignment operators
    VectorTemplate& operator+=(const VectorTemplate& other) {
        for (size_t i = 0; i < N; ++i) {
            data[i] += other.data[i];
        }
        return *this;
    }

    VectorTemplate& operator-=(const VectorTemplate& other) {
        for (size_t i = 0; i < N; ++i) {
            data[i] -= other.data[i];
        }
        return *this;
    }

    VectorTemplate& operator*=(T scalar) {
        for (size_t i = 0; i < N; ++i) {
            data[i] *= scalar;
        }
        return *this;
    }

    VectorTemplate& operator/=(T scalar) {
        for (size_t i = 0; i < N; ++i) {
            data[i] /= scalar;
        }
        return *this;
    }

    // Comparison operators
    bool operator==(const VectorTemplate& other) const {
        for (size_t i = 0; i < N; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

    bool operator!=(const VectorTemplate& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Template for matrix operations with SIMD optimization
 * 
 * This template demonstrates standard matrix operations that should
 * be implemented consistently across Matrix2, Matrix3, and Matrix4.
 *
 * Performance Characteristics:
 * - SIMD optimized where available
 * - Cache-aligned data structures
 * - Optimized for common transforms
 *
 * @tparam T Component type (usually float)
 * @tparam N Matrix dimension (2, 3, or 4)
 */
template<typename T, size_t N>
class MatrixTemplate {
public:
    // Static assertions
    static_assert(N >= 2 && N <= 4, "Matrix dimension must be 2, 3, or 4");
    static_assert(std::is_floating_point<T>::value, "Matrix type must be floating point");

    // SIMD-aligned storage (row-major order)
    alignas(16) T data[N][N];

    /**
     * @brief Default constructor, initializes to identity
     */
    MatrixTemplate() {
        setIdentity();
    }

    /**
     * @brief Sets matrix to identity
     * @return Reference to this matrix
     */
    MatrixTemplate& setIdentity() {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                data[i][j] = (i == j) ? T(1) : T(0);
            }
        }
        return *this;
    }

    /**
     * @brief Creates identity matrix
     * @return Identity matrix
     */
    static MatrixTemplate identity() {
        return MatrixTemplate();
    }

    /**
     * @brief Array subscript operator with bounds checking
     * @param row Row index
     * @return Pointer to row data
     * @throw std::out_of_range if row is invalid
     */
    T* operator[](size_t row) {
        if (row >= N) {
            throw std::out_of_range("Matrix row index out of range");
        }
        return data[row];
    }

    /**
     * @brief Const array subscript operator with bounds checking
     * @param row Row index
     * @return Const pointer to row data
     * @throw std::out_of_range if row is invalid
     */
    const T* operator[](size_t row) const {
        if (row >= N) {
            throw std::out_of_range("Matrix row index out of range");
        }
        return data[row];
    }

    /**
     * @brief Transposes matrix in-place
     * @return Reference to this matrix
     */
    MatrixTemplate& transpose() {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = i + 1; j < N; ++j) {
                std::swap(data[i][j], data[j][i]);
            }
        }
        return *this;
    }

    /**
     * @brief Returns transposed copy of matrix
     * @return Transposed matrix
     */
    MatrixTemplate transposed() const {
        MatrixTemplate result(*this);
        result.transpose();
        return result;
    }

    /**
     * @brief Calculates matrix determinant
     * @return Matrix determinant
     */
    T determinant() const {
        // Implementation depends on dimension
        // This is a placeholder - actual implementation would be specialized
        return T(0);
    }

    /**
     * @brief Attempts to invert matrix
     * @param[out] result Inverted matrix if successful
     * @return true if matrix was invertible
     */
    bool getInverse(MatrixTemplate& result) const {
        // Implementation depends on dimension
        // This is a placeholder - actual implementation would be specialized
        return false;
    }

    /**
     * @brief Matrix-matrix multiplication
     * @param other Matrix to multiply with
     * @return Result of multiplication
     */
    MatrixTemplate operator*(const MatrixTemplate& other) const {
        MatrixTemplate result;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                result.data[i][j] = T(0);
                for (size_t k = 0; k < N; ++k) {
                    result.data[i][j] += data[i][k] * other.data[k][j];
                }
            }
        }
        return result;
    }

    /**
     * @brief Matrix-vector multiplication
     * @param vec Vector to multiply with
     * @return Transformed vector
     */
    VectorTemplate<T,N> operator*(const VectorTemplate<T,N>& vec) const {
        VectorTemplate<T,N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = T(0);
            for (size_t j = 0; j < N; ++j) {
                result[i] += data[i][j] * vec[j];
            }
        }
        return result;
    }

    /**
     * @brief Converts matrix to string representation
     * @return String representation of matrix
     */
    std::string toString() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);
        for (size_t i = 0; i < N; ++i) {
            ss << "[";
            for (size_t j = 0; j < N; ++j) {
                if (j > 0) ss << ", ";
                ss << data[i][j];
            }
            ss << "]";
            if (i < N-1) ss << "\n";
        }
        return ss.str();
    }

    // Comparison operators
    bool operator==(const MatrixTemplate& other) const {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                if (data[i][j] != other.data[i][j]) return false;
            }
        }
        return true;
    }

    bool operator!=(const MatrixTemplate& other) const {
        return !(*this == other);
    }
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_TEMPLATE_OPERATIONS_HPP