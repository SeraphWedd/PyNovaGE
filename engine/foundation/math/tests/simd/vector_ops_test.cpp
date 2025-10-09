#include <gtest/gtest.h>
#include "../../include/simd/vector_ops.hpp"
#include "../../include/simd/config.hpp"
#include <type_traits>

namespace {

using namespace PyNovaGE::SIMD;

// Test SIMD feature detection
TEST(VectorStructureTest, SIMDFeatureDetection) {
#if defined(NOVA_AVX2_AVAILABLE)
    EXPECT_TRUE(true) << "AVX2 is available";
#endif

#if defined(NOVA_AVX_AVAILABLE)
    EXPECT_TRUE(true) << "AVX is available";
#endif

#if defined(NOVA_SSE2_AVAILABLE)
    EXPECT_TRUE(true) << "SSE2 is available";
#endif

#if defined(NOVA_NEON_AVAILABLE)
    EXPECT_TRUE(true) << "NEON is available";
#endif
}

// Test vector type traits
TEST(VectorStructureTest, TypeTraits) {
    // Test vector types are properly defined
    EXPECT_TRUE((std::is_same_v<Vector4f::value_type, float>));
    EXPECT_TRUE((std::is_same_v<Vector3f::value_type, float>));
    EXPECT_TRUE((std::is_same_v<Vector2f::value_type, float>));
    
    EXPECT_TRUE((std::is_same_v<Vector4d::value_type, double>));
    EXPECT_TRUE((std::is_same_v<Vector3d::value_type, double>));
    EXPECT_TRUE((std::is_same_v<Vector2d::value_type, double>));
    
    EXPECT_TRUE((std::is_same_v<Vector4i::value_type, int32_t>));
    EXPECT_TRUE((std::is_same_v<Vector3i::value_type, int32_t>));
    EXPECT_TRUE((std::is_same_v<Vector2i::value_type, int32_t>));
}

// Test memory alignment
TEST(VectorStructureTest, MemoryAlignment) {
    // Test Vector4f alignment
    Vector4f v4;
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(v4.data()) % 16, 0);
    
    // Test Vector3f alignment
    Vector3f v3;
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(v3.data()) % 16, 0);
    
    // Test proper padding
    EXPECT_GE(sizeof(Vector3f), 16);
    EXPECT_EQ(sizeof(Vector4f), 16);
}

// Test SIMD register types
TEST(VectorStructureTest, SIMDRegisterTypes) {
#if defined(NOVA_AVX2_AVAILABLE) || defined(NOVA_AVX_AVAILABLE)
    EXPECT_TRUE((std::is_same_v<detail::float_reg, __m256>));
    EXPECT_TRUE((std::is_same_v<detail::double_reg, __m256d>));
    EXPECT_TRUE((std::is_same_v<detail::int_reg, __m256i>));
#elif defined(NOVA_SSE2_AVAILABLE)
    EXPECT_TRUE((std::is_same_v<detail::float_reg, __m128>));
    EXPECT_TRUE((std::is_same_v<detail::double_reg, __m128d>));
    EXPECT_TRUE((std::is_same_v<detail::int_reg, __m128i>));
#elif defined(NOVA_NEON_AVAILABLE)
    EXPECT_TRUE((std::is_same_v<detail::float_reg, float32x4_t>));
    EXPECT_TRUE((std::is_same_v<detail::double_reg, float64x2_t>));
    EXPECT_TRUE((std::is_same_v<detail::int_reg, int32x4_t>));
#else
    // Test scalar fallback
    EXPECT_TRUE((std::is_array_v<detail::float_reg>));
    EXPECT_TRUE((std::is_array_v<detail::double_reg>));
    EXPECT_TRUE((std::is_array_v<detail::int_reg>));
#endif
}

// Test vector construction
TEST(VectorStructureTest, Construction) {
    // Test scalar constructor
    Vector4f v1(1.0f);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(v1[i], 1.0f);
    }
    
    // Test component-wise constructor
    Vector3f v2(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v2[0], 1.0f);
    EXPECT_EQ(v2[1], 2.0f);
    EXPECT_EQ(v2[2], 3.0f);
    
    // Test size constraints
    static_assert(Vector4f::size == 4, "Vector4f size must be 4");
    static_assert(Vector3f::size == 3, "Vector3f size must be 3");
    static_assert(Vector2f::size == 2, "Vector2f size must be 2");
}

// Test component access
TEST(VectorStructureTest, ComponentAccess) {
    Vector4f v(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Test array subscript operator
    EXPECT_EQ(v[0], 1.0f);
    EXPECT_EQ(v[1], 2.0f);
    EXPECT_EQ(v[2], 3.0f);
    EXPECT_EQ(v[3], 4.0f);
    
    // Test component accessors
    EXPECT_EQ(v.x(), 1.0f);
    EXPECT_EQ(v.y(), 2.0f);
    EXPECT_EQ(v.z(), 3.0f);
    EXPECT_EQ(v.w(), 4.0f);
}

} // namespace
