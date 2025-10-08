#ifndef PYNOVAGE_MATH_SIMD_UTILS_HPP
#define PYNOVAGE_MATH_SIMD_UTILS_HPP

// SIMD Feature Detection
#if defined(_MSC_VER)
    #include <intrin.h>
#else
    #include <x86intrin.h>
#endif

// Check for SSE support
#if defined(__SSE__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1) || defined(_M_AMD64) || defined(_M_X64)
    #define PYNOVAGE_MATH_HAS_SSE 1
#else
    #define PYNOVAGE_MATH_HAS_SSE 0
#endif

// Check for SSE2 support (always available on x64)
#if defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(_M_AMD64) || defined(_M_X64)
    #define PYNOVAGE_MATH_HAS_SSE2 1
#else
    #define PYNOVAGE_MATH_HAS_SSE2 0
#endif

// Check for AVX support
#if defined(__AVX__) || (defined(_MSC_FULL_VER) && defined(__AVX__))
    #define PYNOVAGE_MATH_HAS_AVX 1
#else
    #define PYNOVAGE_MATH_HAS_AVX 0
#endif

namespace pynovage {
namespace math {

/**
 * @brief Class providing SIMD-optimized math operations with fallbacks for non-SIMD platforms
 */
class SimdUtils {
public:
    /**
     * @brief Checks if SSE instructions are available at runtime
     * @return true if SSE is supported, false otherwise
     */
    static bool HasSSE();

    /**
     * @brief Checks if SSE2 instructions are available at runtime
     * @return true if SSE2 is supported, false otherwise
     */
    static bool HasSSE2();

    /**
     * @brief Checks if AVX instructions are available at runtime
     * @return true if AVX is supported, false otherwise
     */
    static bool HasAVX();

    /**
     * @brief Performs two float additions using SIMD if available
     * @param a First pair of floats
     * @param b Second pair of floats
     * @return Result of component-wise addition
     */
    static void Add2f(const float* a, const float* b, float* result);

    /**
     * @brief Performs two float subtractions using SIMD if available
     * @param a First pair of floats
     * @param b Second pair of floats
     * @return Result of component-wise subtraction
     */
    static void Subtract2f(const float* a, const float* b, float* result);

    /**
     * @brief Performs two float multiplications using SIMD if available
     * @param a First pair of floats
     * @param b Second pair of floats
     * @return Result of component-wise multiplication
     */
    static void Multiply2f(const float* a, const float* b, float* result);

    /**
     * @brief Performs two float divisions using SIMD if available
     * @param a First pair of floats
     * @param b Second pair of floats
     * @return Result of component-wise division
     */
    static void Divide2f(const float* a, const float* b, float* result);

    /**
     * @brief Calculates dot product of two 2D vectors using SIMD if available
     * @param a First vector
     * @param b Second vector
     * @return Dot product result
     */
    static float DotProduct2f(const float* a, const float* b);

    /**
     * @brief Performs three float additions using SIMD if available
     * @param a First triplet of floats
     * @param b Second triplet of floats
     * @param result Result of component-wise addition
     */
    static void Add3f(const float* a, const float* b, float* result);

    /**
     * @brief Performs three float subtractions using SIMD if available
     * @param a First triplet of floats
     * @param b Second triplet of floats
     * @param result Result of component-wise subtraction
     */
    static void Subtract3f(const float* a, const float* b, float* result);

    /**
     * @brief Performs three float multiplications using SIMD if available
     * @param a First triplet of floats
     * @param b Second triplet of floats
     * @param result Result of component-wise multiplication
     */
    static void Multiply3f(const float* a, const float* b, float* result);

    /**
     * @brief Performs three float divisions using SIMD if available
     * @param a First triplet of floats
     * @param b Second triplet of floats
     * @param result Result of component-wise division
     */
    static void Divide3f(const float* a, const float* b, float* result);

    /**
     * @brief Calculates dot product of two 3D vectors using SIMD if available
     * @param a First vector
     * @param b Second vector
     * @return Dot product result
     */
    static float DotProduct3f(const float* a, const float* b);

    /**
     * @brief Calculates cross product of two 3D vectors using SIMD if available
     * @param a First vector
     * @param b Second vector
     * @param result Cross product result
     */
    static void CrossProduct3f(const float* a, const float* b, float* result);

    // Matrix operations

    /**
     * @brief Multiplies two 2x2 matrices using SIMD if available
     * @param a First matrix (row-major)
     * @param b Second matrix (row-major)
     * @param result Result matrix
     */
    static void MultiplyMatrix2x2(const float* a, const float* b, float* result);

    /**
     * @brief Multiplies two 3x3 matrices using SIMD if available
     * @param a First matrix (row-major)
     * @param b Second matrix (row-major)
     * @param result Result matrix
     */
    static void MultiplyMatrix3(const float* a, const float* b, float* result);

    /**
     * @brief Multiplies two 4x4 matrices using SIMD if available
     * @param a First matrix (row-major)
     * @param b Second matrix (row-major)
     * @param result Result matrix
     */
    static void MultiplyMatrix4x4(const float* a, const float* b, float* result);

    /**
     * @brief Multiplies a 2x2 matrix by a 2D vector using SIMD if available
     * @param m Matrix (row-major)
     * @param v Vector
     * @param result Result vector
     */
    static void MultiplyMatrix2x2Vec2(const float* m, const float* v, float* result);

    /**
     * @brief Multiplies a 3x3 matrix by a 3D vector using SIMD if available
     * @param m Matrix (row-major)
     * @param v Vector
     * @param result Result vector
     */
    static void MultiplyMatrix3Vec3(const float* m, const float* v, float* result);

    /**
     * @brief Multiplies a 4x4 matrix by a 4D vector using SIMD if available
     * @param m Matrix (row-major)
     * @param v Vector
     * @param result Result vector
     */
    static void MultiplyMatrix4x4Vec4(const float* m, const float* v, float* result);

    /**
     * @brief Transposes a 2x2 matrix in-place using SIMD if available
     * @param m Matrix to transpose (row-major)
     */
    static void TransposeMatrix2x2(float* m);

    /**
     * @brief Transposes a 3x3 matrix in-place using SIMD if available
     * @param m Matrix to transpose (row-major)
     */
    static void TransposeMatrix3(float* m);

    /**
     * @brief Transposes a 4x4 matrix in-place using SIMD if available
     * @param m Matrix to transpose (row-major)
     */
    static void TransposeMatrix4x4(float* m);

    /**
     * @brief Calculates determinant of a 2x2 matrix using SIMD if available
     * @param m Matrix (row-major)
     * @return Determinant value
     */
    static float DeterminantMatrix2x2(const float* m);

    /**
     * @brief Reciprocal square root of 4 floats using SIMD if available
     * More efficient than calculating 1.0f/sqrt() separately
     * @param a Input values
     * @param result Output values
     */
    static void ReciprocalSqrt4f(const float* a, float* result);

    /**
     * @brief Calculates determinant of a 3x3 matrix using SIMD if available
     * @param m Matrix (row-major)
     * @return Determinant value
     */
    static float DeterminantMatrix3(const float* m);

    /**
     * @brief Calculates determinant of a 4x4 matrix using SIMD if available
     * @param m Matrix (row-major)
     * @return Determinant value
     */
    static float DeterminantMatrix4x4(const float* m);

    /**
     * @brief Inverts a 2x2 matrix using SIMD if available
     * @param m Matrix to invert (row-major)
     * @param result Inverted matrix
     * @return true if matrix was invertible, false otherwise
     */
    static bool InvertMatrix2x2(const float* m, float* result);

    /**
     * @brief Inverts a 3x3 matrix using SIMD if available
     * @param m Matrix to invert (row-major)
     * @param result Inverted matrix
     * @return true if matrix was invertible, false otherwise
     */
    static bool InvertMatrix3(const float* m, float* result);

    /**
     * @brief Inverts a 4x4 matrix using SIMD if available
     * @param m Matrix to invert (row-major)
     * @param result Inverted matrix
     * @return true if matrix was invertible, false otherwise
     */
    static bool InvertMatrix4x4(const float* m, float* result);

    /**
     * @brief Fills 4 floats with a value using SIMD if available
     * @param result Array to fill
     * @param value Value to fill with
     */
    static void Fill4f(float* result, float value);

    /**
     * @brief Calculates square root of 4 floats using SIMD if available
     * @param a Input array of 4 floats
     * @param result Array to store results
     */
    static void Sqrt4f(const float* a, float* result);

    /**
     * @brief Tests if 4 AABBs overlap with a target AABB along a single axis using SIMD
     * 
     * @param min_a Min value of target AABB for the axis
     * @param max_a Max value of target AABB for the axis
     * @param mins Array of 4 min values from test AABBs
     * @param maxs Array of 4 max values from test AABBs
     * @param result Array to store 4 boolean results (as integers)
     */
    static void TestAxisOverlap4f(float min_a, float max_a, const float* mins, const float* maxs, int* result);

    /**
     * @brief Tests if 4 AABBs overlap with a target AABB using SIMD (all 3 axes at once)
     * 
     * @param min_a Min values of target AABB (x,y,z)
     * @param max_a Max values of target AABB (x,y,z)
     * @param mins Array of 4 sets of min values (x0,x1,x2,x3, y0,y1,y2,y3, z0,z1,z2,z3)
     * @param maxs Array of 4 sets of max values (x0,x1,x2,x3, y0,y1,y2,y3, z0,z1,z2,z3)
     * @param result Array to store 4 boolean results (as integers)
     */
    static void TestAABBOverlap4f(const float* min_a, const float* max_a, const float* mins, const float* maxs, int* result);

    /**
     * @brief Performs four float additions using SIMD if available
     * @param a First set of four floats
     * @param b Second set of four floats
     * @param result Result of component-wise addition
     */
    static void Add4f(const float* a, const float* b, float* result);

    /**
     * @brief Performs four float subtractions using SIMD if available
     * @param a First set of four floats
     * @param b Second set of four floats
     * @param result Result of component-wise subtraction
     */
    static void Subtract4f(const float* a, const float* b, float* result);

    /**
     * @brief Performs four float multiplications using SIMD if available
     * @param a First set of four floats
     * @param b Second set of four floats
     * @param result Result of component-wise multiplication
     */
    static void Multiply4f(const float* a, const float* b, float* result);

    /**
     * @brief Performs four float divisions using SIMD if available
     * @param a First set of four floats
     * @param b Second set of four floats
     * @param result Result of component-wise division
     */
    static void Divide4f(const float* a, const float* b, float* result);

    /**
     * @brief Multiplies 2D vector by scalar using SIMD if available
     * @param a Input vector
     * @param scalar Scalar value
     * @param result Result vector
     */
    static void Multiply2fScalar(const float* a, float scalar, float* result);

    /**
     * @brief Divides 2D vector by scalar using SIMD if available
     * @param a Input vector
     * @param scalar Scalar value
     * @param result Result vector
     */
    static void Divide2fScalar(const float* a, float scalar, float* result);

    /**
     * @brief Multiplies 3D vector by scalar using SIMD if available
     * @param a Input vector
     * @param scalar Scalar value
     * @param result Result vector
     */
    static void Multiply3fScalar(const float* a, float scalar, float* result);

    /**
     * @brief Divides 3D vector by scalar using SIMD if available
     * @param a Input vector
     * @param scalar Scalar value
     * @param result Result vector
     */
    static void Divide3fScalar(const float* a, float scalar, float* result);

    /**
     * @brief Multiplies 4D vector by scalar using SIMD if available
     * @param a Input vector
     * @param scalar Scalar value
     * @param result Result vector
     */
    static void Multiply4fScalar(const float* a, float scalar, float* result);

    /**
     * @brief Divides 4D vector by scalar using SIMD if available
     * @param a Input vector
     * @param scalar Scalar value
     * @param result Result vector
     */
    static void Divide4fScalar(const float* a, float scalar, float* result);

    /**
     * @brief Component-wise minimum of 4 floats using SIMD if available
     * @param a First vector
     * @param b Second vector
     * @param result Result vector
     */
    static void Min4f(const float* a, const float* b, float* result);

    /**
     * @brief Component-wise maximum of 4 floats using SIMD if available
     * @param a First vector
     * @param b Second vector
     * @param result Result vector
     */
    static void Max4f(const float* a, const float* b, float* result);

    /**
     * @brief Calculates dot product of two 4D vectors using SIMD if available
     * @param a First vector
     * @param b Second vector
     * @return Dot product result
     */
    static float DotProduct4f(const float* a, const float* b);
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_SIMD_UTILS_HPP