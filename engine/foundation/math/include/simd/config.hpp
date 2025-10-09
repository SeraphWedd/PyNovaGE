#ifndef PYNOVAGE_FOUNDATION_MATH_SIMD_CONFIG_HPP
#define PYNOVAGE_FOUNDATION_MATH_SIMD_CONFIG_HPP

// SIMD Feature Detection Headers
#if defined(_MSC_VER)
    #include <intrin.h>
#else
    #include <x86intrin.h>
#endif

// Include cstdint for fixed-width integer types
#include <cstdint>

namespace pynovage {
namespace foundation {
namespace simd {

// SIMD Feature Detection Macros

// Check for SSE support
#if defined(__SSE__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1) || defined(_M_AMD64) || defined(_M_X64)
    #define PYNOVAGE_SIMD_HAS_SSE 1
#else
    #define PYNOVAGE_SIMD_HAS_SSE 0
#endif

// Check for SSE2 support (always available on x64)
#if defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(_M_AMD64) || defined(_M_X64)
    #define PYNOVAGE_SIMD_HAS_SSE2 1
#else
    #define PYNOVAGE_SIMD_HAS_SSE2 0
#endif

// Check for SSE3 support
#if defined(__SSE3__) || (defined(_MSC_FULL_VER) && defined(__AVX__))
    #define PYNOVAGE_SIMD_HAS_SSE3 1
#else
    #define PYNOVAGE_SIMD_HAS_SSE3 0
#endif

// Check for SSSE3 support
#if defined(__SSSE3__) || (defined(_MSC_FULL_VER) && defined(__AVX__))
    #define PYNOVAGE_SIMD_HAS_SSSE3 1
#else
    #define PYNOVAGE_SIMD_HAS_SSSE3 0
#endif

// Check for SSE4.1 support
#if defined(__SSE4_1__) || (defined(_MSC_FULL_VER) && defined(__AVX__))
    #define PYNOVAGE_SIMD_HAS_SSE4_1 1
#else
    #define PYNOVAGE_SIMD_HAS_SSE4_1 0
#endif

// Check for SSE4.2 support
#if defined(__SSE4_2__) || (defined(_MSC_FULL_VER) && defined(__AVX__))
    #define PYNOVAGE_SIMD_HAS_SSE4_2 1
#else
    #define PYNOVAGE_SIMD_HAS_SSE4_2 0
#endif

// Check for AVX support
#if defined(__AVX__) || (defined(_MSC_FULL_VER) && defined(__AVX__))
    #define PYNOVAGE_SIMD_HAS_AVX 1
#else
    #define PYNOVAGE_SIMD_HAS_AVX 0
#endif

// Check for AVX2 support
#if defined(__AVX2__) || (defined(_MSC_FULL_VER) && defined(__AVX2__))
    #define PYNOVAGE_SIMD_HAS_AVX2 1
#else
    #define PYNOVAGE_SIMD_HAS_AVX2 0
#endif

// Check for FMA support
#if defined(__FMA__) || (defined(_MSC_FULL_VER) && defined(__AVX2__))
    #define PYNOVAGE_SIMD_HAS_FMA 1
#else
    #define PYNOVAGE_SIMD_HAS_FMA 0
#endif

// Check for AVX-512F support (Foundation)
#if defined(__AVX512F__) || (defined(_MSC_FULL_VER) && defined(__AVX512F__))
    #define PYNOVAGE_SIMD_HAS_AVX512F 1
#else
    #define PYNOVAGE_SIMD_HAS_AVX512F 0
#endif

/**
 * @brief Runtime SIMD feature detection and configuration.
 * 
 * This class provides runtime detection of SIMD features and configuration
 * options. While the macros above provide compile-time detection, this class
 * allows for runtime feature checking which can be useful for:
 * 1. Selecting optimal code paths
 * 2. Graceful fallbacks when features aren't available
 * 3. Debugging and diagnostics
 */
class Features {
public:
    /**
     * @brief Initialize SIMD feature detection.
     * Should be called once at program startup.
     */
    static void Initialize();

    /**
     * @brief Check if SSE instructions are available at runtime.
     * @return true if SSE is supported, false otherwise.
     */
    static bool HasSSE() { return s_features & Feature_SSE; }

    /**
     * @brief Check if SSE2 instructions are available at runtime.
     * @return true if SSE2 is supported, false otherwise.
     */
    static bool HasSSE2() { return s_features & Feature_SSE2; }

    /**
     * @brief Check if SSE3 instructions are available at runtime.
     * @return true if SSE3 is supported, false otherwise.
     */
    static bool HasSSE3() { return s_features & Feature_SSE3; }

    /**
     * @brief Check if SSSE3 instructions are available at runtime.
     * @return true if SSSE3 is supported, false otherwise.
     */
    static bool HasSSSE3() { return s_features & Feature_SSSE3; }

    /**
     * @brief Check if SSE4.1 instructions are available at runtime.
     * @return true if SSE4.1 is supported, false otherwise.
     */
    static bool HasSSE4_1() { return s_features & Feature_SSE4_1; }

    /**
     * @brief Check if SSE4.2 instructions are available at runtime.
     * @return true if SSE4.2 is supported, false otherwise.
     */
    static bool HasSSE4_2() { return s_features & Feature_SSE4_2; }

    /**
     * @brief Check if AVX instructions are available at runtime.
     * @return true if AVX is supported, false otherwise.
     */
    static bool HasAVX() { return s_features & Feature_AVX; }

    /**
     * @brief Check if AVX2 instructions are available at runtime.
     * @return true if AVX2 is supported, false otherwise.
     */
    static bool HasAVX2() { return s_features & Feature_AVX2; }

    /**
     * @brief Check if FMA instructions are available at runtime.
     * @return true if FMA is supported, false otherwise.
     */
    static bool HasFMA() { return s_features & Feature_FMA; }

    /**
     * @brief Check if AVX-512F instructions are available at runtime.
     * @return true if AVX-512F is supported, false otherwise.
     */
    static bool HasAVX512F() { return s_features & Feature_AVX512F; }

private:
    // Feature flags for runtime detection
    enum FeatureFlags : uint32_t {
        Feature_SSE     = 1 << 0,
        Feature_SSE2    = 1 << 1,
        Feature_SSE3    = 1 << 2,
        Feature_SSSE3   = 1 << 3,
        Feature_SSE4_1  = 1 << 4,
        Feature_SSE4_2  = 1 << 5,
        Feature_AVX     = 1 << 6,
        Feature_AVX2    = 1 << 7,
        Feature_FMA     = 1 << 8,
        Feature_AVX512F = 1 << 9
    };

    static uint32_t s_features;  // Stores detected features
};

} // namespace simd
} // namespace foundation
} // namespace pynovage

#endif // PYNOVAGE_FOUNDATION_MATH_SIMD_CONFIG_HPP