#include "simd/config.hpp"

namespace pynovage {
namespace foundation {
namespace simd {

// Initialize static member
uint32_t Features::s_features = 0;

void Features::Initialize() {
    s_features = 0;

#if defined(_MSC_VER)
    // Using MSVC intrinsics
    int cpuInfo[4];
    int maxFunc;

    // Get maximum supported function
    __cpuid(cpuInfo, 0);
    maxFunc = cpuInfo[0];

    // Get feature bits
    if (maxFunc >= 1) {
        __cpuid(cpuInfo, 1);

        // Check SSE features
        if (cpuInfo[3] & (1 << 25)) s_features |= Feature_SSE;
        if (cpuInfo[3] & (1 << 26)) s_features |= Feature_SSE2;
        if (cpuInfo[2] & (1 << 0))  s_features |= Feature_SSE3;
        if (cpuInfo[2] & (1 << 9))  s_features |= Feature_SSSE3;
        if (cpuInfo[2] & (1 << 19)) s_features |= Feature_SSE4_1;
        if (cpuInfo[2] & (1 << 20)) s_features |= Feature_SSE4_2;
        if (cpuInfo[2] & (1 << 28)) s_features |= Feature_AVX;
        if (cpuInfo[2] & (1 << 12)) s_features |= Feature_FMA;
    }

    // Check for AVX2 support
    if (maxFunc >= 7) {
        __cpuidex(cpuInfo, 7, 0);
        if (cpuInfo[1] & (1 << 5)) s_features |= Feature_AVX2;
        
        // Check for AVX-512F support
        if (cpuInfo[1] & (1 << 16)) s_features |= Feature_AVX512F;
    }

#else
    // Using GCC/Clang intrinsics
    #if defined(__SSE__)
        s_features |= Feature_SSE;
    #endif
    #if defined(__SSE2__)
        s_features |= Feature_SSE2;
    #endif
    #if defined(__SSE3__)
        s_features |= Feature_SSE3;
    #endif
    #if defined(__SSSE3__)
        s_features |= Feature_SSSE3;
    #endif
    #if defined(__SSE4_1__)
        s_features |= Feature_SSE4_1;
    #endif
    #if defined(__SSE4_2__)
        s_features |= Feature_SSE4_2;
    #endif
    #if defined(__AVX__)
        s_features |= Feature_AVX;
    #endif
    #if defined(__AVX2__)
        s_features |= Feature_AVX2;
    #endif
    #if defined(__FMA__)
        s_features |= Feature_FMA;
    #endif
#endif
}

} // namespace simd
} // namespace foundation
} // namespace pynovage