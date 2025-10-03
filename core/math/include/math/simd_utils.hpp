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
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_SIMD_UTILS_HPP