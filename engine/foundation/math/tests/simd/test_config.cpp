#include <gtest/gtest.h>
#include "simd/config.hpp"

using namespace pynovage::foundation::simd;

class SimdConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        Features::Initialize();
    }
};

TEST_F(SimdConfigTest, FeatureDetectionMacros) {
    // Test compile-time feature detection
#if PYNOVAGE_SIMD_HAS_SSE
    EXPECT_TRUE(Features::HasSSE());
#else
    EXPECT_FALSE(Features::HasSSE());
#endif

#if PYNOVAGE_SIMD_HAS_SSE2
    EXPECT_TRUE(Features::HasSSE2());
#else
    EXPECT_FALSE(Features::HasSSE2());
#endif

#if PYNOVAGE_SIMD_HAS_SSE3
    EXPECT_TRUE(Features::HasSSE3());
#else
    EXPECT_FALSE(Features::HasSSE3());
#endif

#if PYNOVAGE_SIMD_HAS_SSSE3
    EXPECT_TRUE(Features::HasSSSE3());
#else
    EXPECT_FALSE(Features::HasSSSE3());
#endif

#if PYNOVAGE_SIMD_HAS_SSE4_1
    EXPECT_TRUE(Features::HasSSE4_1());
#else
    EXPECT_FALSE(Features::HasSSE4_1());
#endif

#if PYNOVAGE_SIMD_HAS_SSE4_2
    EXPECT_TRUE(Features::HasSSE4_2());
#else
    EXPECT_FALSE(Features::HasSSE4_2());
#endif

#if PYNOVAGE_SIMD_HAS_AVX
    EXPECT_TRUE(Features::HasAVX());
#else
    EXPECT_FALSE(Features::HasAVX());
#endif

#if PYNOVAGE_SIMD_HAS_AVX2
    EXPECT_TRUE(Features::HasAVX2());
#else
    EXPECT_FALSE(Features::HasAVX2());
#endif

#if PYNOVAGE_SIMD_HAS_FMA
    EXPECT_TRUE(Features::HasFMA());
#else
    EXPECT_FALSE(Features::HasFMA());
#endif

#if PYNOVAGE_SIMD_HAS_AVX512F
    EXPECT_TRUE(Features::HasAVX512F());
#else
    EXPECT_FALSE(Features::HasAVX512F());
#endif
}

TEST_F(SimdConfigTest, FeatureConsistency) {
    // Test feature dependencies
    if (Features::HasAVX512F()) {
        EXPECT_TRUE(Features::HasAVX2());
        EXPECT_TRUE(Features::HasAVX());
    }

    if (Features::HasAVX2()) {
        EXPECT_TRUE(Features::HasAVX());
    }

    if (Features::HasSSE4_2()) {
        EXPECT_TRUE(Features::HasSSE4_1());
    }

    if (Features::HasSSE4_1()) {
        EXPECT_TRUE(Features::HasSSSE3());
    }

    if (Features::HasSSSE3()) {
        EXPECT_TRUE(Features::HasSSE3());
    }

    if (Features::HasSSE3()) {
        EXPECT_TRUE(Features::HasSSE2());
    }

    if (Features::HasSSE2()) {
        EXPECT_TRUE(Features::HasSSE());
    }
}