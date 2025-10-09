#ifndef PYNOVAGE_FOUNDATION_MATH_SIMD_VECTOR_OPS_HPP
#define PYNOVAGE_FOUNDATION_MATH_SIMD_VECTOR_OPS_HPP

#include "simd/config.hpp"
#include "simd/types.hpp"

namespace pynovage {
namespace foundation {
namespace simd {

/**
 * @brief SIMD-optimized 2D vector operations
 */
class Vec2Ops {
public:
    static void add(const float4& a, const float4& b, float4& result);
    static void subtract(const float4& a, const float4& b, float4& result);
    static void multiply(const float4& a, const float4& b, float4& result);
    static void multiply_scalar(const float4& a, float scalar, float4& result);
    static void divide(const float4& a, const float4& b, float4& result);
    static void divide_scalar(const float4& a, float scalar, float4& result);
    static float dot(const float4& a, const float4& b);
    static float length_squared(const float4& v);
    static float length(const float4& v);
    static void normalize(const float4& v, float4& result);

    // Batch operations (process 2 vec2s at once using float4)
    static void add_batch2(const float4& a, const float4& b, float4& result);
    static void subtract_batch2(const float4& a, const float4& b, float4& result);
    static void multiply_batch2(const float4& a, const float4& b, float4& result);
    static void multiply_scalar_batch2(const float4& a, float scalar, float4& result);
    static void divide_batch2(const float4& a, const float4& b, float4& result);
    static void divide_scalar_batch2(const float4& a, float scalar, float4& result);
};

/**
 * @brief SIMD-optimized 3D vector operations
 */
class Vec3Ops {
public:
    static void add(const float4& a, const float4& b, float4& result);
    static void subtract(const float4& a, const float4& b, float4& result);
    static void multiply(const float4& a, const float4& b, float4& result);
    static void multiply_scalar(const float4& a, float scalar, float4& result);
    static void divide(const float4& a, const float4& b, float4& result);
    static void divide_scalar(const float4& a, float scalar, float4& result);
    static float dot(const float4& a, const float4& b);
    static void cross(const float4& a, const float4& b, float4& result);
    static float length_squared(const float4& v);
    static float length(const float4& v);
    static void normalize(const float4& v, float4& result);

    // Batch operations using AVX
    static void add_batch4(const float16& a, const float16& b, float16& result);
    static void subtract_batch4(const float16& a, const float16& b, float16& result);
    static void multiply_batch4(const float16& a, const float16& b, float16& result);
    static void multiply_scalar_batch4(const float16& a, float scalar, float16& result);
    static void divide_batch4(const float16& a, const float16& b, float16& result);
    static void divide_scalar_batch4(const float16& a, float scalar, float16& result);
    static void cross_batch4(const float16& a, const float16& b, float16& result);
};

/**
 * @brief SIMD-optimized 4D vector operations
 */
class Vec4Ops {
public:
    static void add(const float4& a, const float4& b, float4& result);
    static void subtract(const float4& a, const float4& b, float4& result);
    static void multiply(const float4& a, const float4& b, float4& result);
    static void multiply_scalar(const float4& a, float scalar, float4& result);
    static void divide(const float4& a, const float4& b, float4& result);
    static void divide_scalar(const float4& a, float scalar, float4& result);
    static float dot(const float4& a, const float4& b);
    static float length_squared(const float4& v);
    static float length(const float4& v);
    static void normalize(const float4& v, float4& result);

    // Fused multiply-add operations
    static void multiply_add(const float4& a, const float4& b, const float4& c, float4& result);
    static void multiply_sub(const float4& a, const float4& b, const float4& c, float4& result);

    // Batch operations
    static void add_batch4(const float16& a, const float16& b, float16& result);
    static void subtract_batch4(const float16& a, const float16& b, float16& result);
    static void multiply_batch4(const float16& a, const float16& b, float16& result);
    static void multiply_scalar_batch4(const float16& a, float scalar, float16& result);
    static void divide_batch4(const float16& a, const float16& b, float16& result);
    static void divide_scalar_batch4(const float16& a, float scalar, float16& result);
    static void multiply_add_batch4(const float16& a, const float16& b, const float16& c, float16& result);
};

/**
 * @brief SIMD vector comparison operations
 */
class VecCompareOps {
public:
    // Single vector operations (float4)
    static void less_than(const float4& a, const float4& b, int* result);
    static void less_equal(const float4& a, const float4& b, int* result);
    static void greater_than(const float4& a, const float4& b, int* result);
    static void greater_equal(const float4& a, const float4& b, int* result);
    static void equal(const float4& a, const float4& b, int* result);
    static void not_equal(const float4& a, const float4& b, int* result);

    // Batch operations (float16)
    static void less_than_batch4(const float16& a, const float16& b, int* result);
    static void less_equal_batch4(const float16& a, const float16& b, int* result);
    static void greater_than_batch4(const float16& a, const float16& b, int* result);
    static void greater_equal_batch4(const float16& a, const float16& b, int* result);
    static void equal_batch4(const float16& a, const float16& b, int* result);
    static void not_equal_batch4(const float16& a, const float16& b, int* result);
};

} // namespace simd
} // namespace foundation
} // namespace pynovage

#endif // PYNOVAGE_FOUNDATION_MATH_SIMD_VECTOR_OPS_HPP