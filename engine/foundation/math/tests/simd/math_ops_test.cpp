#include <gtest/gtest.h>
#include "../../include/simd/math_ops.hpp"
#include "../../include/simd/config.hpp"
#include <type_traits>

namespace {

using namespace PyNovaGE::SIMD;

// Test SIMD math operation availability
TEST(MathOpsTest, OperationAvailability) {
    // Basic math operations should be available
    EXPECT_TRUE((std::is_function_v<decltype(sqrt<float, 4>)>));
    EXPECT_TRUE((std::is_function_v<decltype(rsqrt<float, 4>)>));
    EXPECT_TRUE((std::is_function_v<decltype(abs<float, 4>)>));
    EXPECT_TRUE((std::is_function_v<decltype(min<float, 4>)>));
    EXPECT_TRUE((std::is_function_v<decltype(max<float, 4>)>));
}

// Test SIMD register types for math operations
TEST(MathOpsTest, RegisterTypes) {
#if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
    // AVX/AVX2 operations should use 256-bit registers
    using float_reg = __m256;
    using int_reg = __m256i;
#elif defined(NOVA_SSE2_AVAILABLE)
    // SSE2 operations should use 128-bit registers
    using float_reg = __m128;
    using int_reg = __m128i;
#elif defined(NOVA_NEON_AVAILABLE)
    // NEON operations should use NEON registers
    using float_reg = float32x4_t;
    using int_reg = int32x4_t;
#else
    // Fallback should use scalar arrays
    using float_reg = float[4];
    using int_reg = int32_t[4];
#endif

    EXPECT_TRUE((std::is_same_v<detail::float_reg, float_reg>));
    EXPECT_TRUE((std::is_same_v<detail::int_reg, int_reg>));
}

// Test operation alignment requirements
TEST(MathOpsTest, AlignmentRequirements) {
    Vector4f v;
    // All math operations should maintain alignment
    const size_t alignment = alignof(Vector4f);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(v.data()) % alignment, 0);

// Storage alignment for our Vector types is fixed by the implementation (alignas(16))
EXPECT_GE(alignment, 16);
EXPECT_EQ(alignment % 16, 0);
}


// Test function type traits
TEST(MathOpsTest, FunctionTraits) {
    // Math operations should be available for different vector sizes
    EXPECT_TRUE((std::is_function_v<decltype(sqrt<float, 2>)>));
    EXPECT_TRUE((std::is_function_v<decltype(sqrt<float, 3>)>));
    EXPECT_TRUE((std::is_function_v<decltype(sqrt<float, 4>)>));
    
    // Math operations should work with different types
    EXPECT_TRUE((std::is_function_v<decltype(abs<float, 4>)>));
    EXPECT_TRUE((std::is_function_v<decltype(abs<double, 4>)>));
    EXPECT_TRUE((std::is_function_v<decltype(abs<int32_t, 4>)>));
}

} // namespace
