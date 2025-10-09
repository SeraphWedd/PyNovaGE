#include <gtest/gtest.h>
#include "../../include/simd/config.hpp"

namespace {

using namespace PyNovaGE::SIMD;

TEST(ConfigTest, ArchitectureDetection) {
    #if defined(_M_AMD64) || defined(_M_X64) || defined(__x86_64__)
        EXPECT_EQ(GetCPUArchitecture(), CPUArchitecture::x86_64);
    #elif defined(_M_ARM64) || defined(__aarch64__)
        EXPECT_EQ(GetCPUArchitecture(), CPUArchitecture::ARM64);
    #else
        EXPECT_EQ(GetCPUArchitecture(), CPUArchitecture::Unknown);
    #endif
}

TEST(ConfigTest, SIMDWidth) {
    #if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
        EXPECT_EQ(SIMD_WIDTH, 32);
    #elif defined(NOVA_SSE2_AVAILABLE) || defined(NOVA_NEON_AVAILABLE)
        EXPECT_EQ(SIMD_WIDTH, 16);
    #else
        EXPECT_EQ(SIMD_WIDTH, 4);
    #endif
}

TEST(ConfigTest, FeatureDetection) {
    // Test AVX2 detection
    #ifdef __AVX2__
        EXPECT_TRUE(HasAVX2());
    #else
        EXPECT_FALSE(HasAVX2());
    #endif

    // Test AVX detection
    #ifdef __AVX__
        EXPECT_TRUE(HasAVX());
    #else
        EXPECT_FALSE(HasAVX());
    #endif

    // Test SSE2 detection
    #if defined(_M_IX86_FP) && _M_IX86_FP >= 2
        EXPECT_TRUE(HasSSE2());
    #elif defined(__SSE2__)
        EXPECT_TRUE(HasSSE2());
    #else
        EXPECT_FALSE(HasSSE2());
    #endif

    // Test NEON detection
    #ifdef __ARM_NEON
        EXPECT_TRUE(HasNEON());
    #else
        EXPECT_FALSE(HasNEON());
    #endif
}

TEST(ConfigTest, FeatureConsistency) {
    // Test that AVX2 implies AVX
    if (HasAVX2()) {
        EXPECT_TRUE(HasAVX());
    }

    // Test SIMD width consistency
    if (HasAVX2() || HasAVX()) {
        EXPECT_EQ(SIMD_WIDTH, 32);
    } else if (HasSSE2() || HasNEON()) {
        EXPECT_EQ(SIMD_WIDTH, 16);
    }
}

} // namespace