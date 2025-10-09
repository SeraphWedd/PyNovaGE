#pragma once

#include <cstdint>

namespace PyNovaGE {
namespace SIMD {

// SIMD instruction set detection
#if defined(_MSC_VER) // Microsoft Visual C++
    #if defined(_M_AMD64) || defined(_M_X64)
        #define NOVA_X64
    #elif defined(_M_ARM64)
        #define NOVA_ARM64
    #endif
    
    #if defined(__AVX2__)
        #define NOVA_AVX2_AVAILABLE
    #endif
    #if defined(__AVX__)
        #define NOVA_AVX_AVAILABLE
    #endif
    #if defined(_M_IX86_FP) && _M_IX86_FP >= 2
        #define NOVA_SSE2_AVAILABLE
    #endif
    #if defined(__SSE4_1__) || defined(__AVX__)
        #define NOVA_SSE4_1_AVAILABLE
    #endif
    #if defined(__ARM_NEON)
        #define NOVA_NEON_AVAILABLE
    #endif

#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang
    #if defined(__x86_64__)
        #define NOVA_X64
    #elif defined(__aarch64__)
        #define NOVA_ARM64
    #endif
    
    #if defined(__AVX2__)
        #define NOVA_AVX2_AVAILABLE
    #endif
    #if defined(__AVX__)
        #define NOVA_AVX_AVAILABLE
    #endif
    #if defined(__SSE2__)
        #define NOVA_SSE2_AVAILABLE
    #endif
    #if defined(__SSE4_1__)
        #define NOVA_SSE4_1_AVAILABLE
    #endif
    #if defined(__ARM_NEON)
        #define NOVA_NEON_AVAILABLE
    #endif
#endif

// SIMD width detection (in bytes)
#if defined(NOVA_AVX2_AVAILABLE)
    constexpr size_t SIMD_WIDTH = 32;
#elif defined(NOVA_AVX_AVAILABLE)
    constexpr size_t SIMD_WIDTH = 32;
#elif defined(NOVA_SSE4_1_AVAILABLE)
    constexpr size_t SIMD_WIDTH = 16;
#elif defined(NOVA_SSE2_AVAILABLE) || defined(NOVA_NEON_AVAILABLE)
    constexpr size_t SIMD_WIDTH = 16;
#else
    constexpr size_t SIMD_WIDTH = 4;
#endif

// Feature detection functions
constexpr bool HasAVX2() {
    #if defined(NOVA_AVX2_AVAILABLE)
        return true;
    #else
        return false;
    #endif
}

constexpr bool HasAVX() {
    #if defined(NOVA_AVX_AVAILABLE)
        return true;
    #else
        return false;
    #endif
}

constexpr bool HasSSE2() {
    #if defined(NOVA_SSE2_AVAILABLE)
        return true;
    #else
        return false;
    #endif
}

constexpr bool HasSSE4_1() {
    #if defined(NOVA_SSE4_1_AVAILABLE)
        return true;
    #else
        return false;
    #endif
}

constexpr bool HasNEON() {
    #if defined(NOVA_NEON_AVAILABLE)
        return true;
    #else
        return false;
    #endif
}

// CPU architecture detection
enum class CPUArchitecture {
    Unknown,
    x86_64,
    ARM64
};

constexpr CPUArchitecture GetCPUArchitecture() {
    #if defined(NOVA_X64)
        return CPUArchitecture::x86_64;
    #elif defined(NOVA_ARM64)
        return CPUArchitecture::ARM64;
    #else
        return CPUArchitecture::Unknown;
    #endif
}

} // namespace SIMD
} // namespace PyNovaGE