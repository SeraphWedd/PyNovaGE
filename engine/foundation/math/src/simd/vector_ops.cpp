#include "simd/vector_ops.hpp"
#include <cmath>

namespace pynovage {
namespace foundation {
namespace simd {

// Vec2Ops Implementation

void Vec2Ops::add(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_add_ps(a.data, b.data));
#else
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = 0.0f;
    result[3] = 0.0f;
#endif
}

void Vec2Ops::subtract(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_sub_ps(a.data, b.data));
#else
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = 0.0f;
    result[3] = 0.0f;
#endif
}

void Vec2Ops::multiply(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_mul_ps(a.data, b.data));
#else
    result[0] = a[0] * b[0];
    result[1] = a[1] * b[1];
    result[2] = 0.0f;
    result[3] = 0.0f;
#endif
}

void Vec2Ops::multiply_scalar(const float4& a, float scalar, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 scalar_vec = _mm_set1_ps(scalar);
    result = float4(_mm_mul_ps(a.data, scalar_vec));
#else
    result[0] = a[0] * scalar;
    result[1] = a[1] * scalar;
    result[2] = 0.0f;
    result[3] = 0.0f;
#endif
}

void Vec2Ops::divide(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_div_ps(a.data, b.data));
#else
    result[0] = a[0] / b[0];
    result[1] = a[1] / b[1];
    result[2] = 0.0f;
    result[3] = 0.0f;
#endif
}

void Vec2Ops::divide_scalar(const float4& a, float scalar, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 scalar_vec = _mm_set1_ps(scalar);
    result = float4(_mm_div_ps(a.data, scalar_vec));
#else
    float inv_scalar = 1.0f / scalar;
    result[0] = a[0] * inv_scalar;
    result[1] = a[1] * inv_scalar;
    result[2] = 0.0f;
    result[3] = 0.0f;
#endif
}

float Vec2Ops::dot(const float4& a, const float4& b) {
#if PYNOVAGE_SIMD_HAS_SSE4_1
    // Using SSE4.1's dot product instruction
    __m128 mul = _mm_mul_ps(a.data, b.data);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    return _mm_cvtss_f32(hadd);
#elif PYNOVAGE_SIMD_HAS_SSE
    // Manual dot product with SSE
    __m128 mul = _mm_mul_ps(a.data, b.data);
    __m128 shuf = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(1, 1, 1, 0));
    __m128 sum = _mm_add_ss(shuf, mul);
    return _mm_cvtss_f32(sum);
#else
    return a[0] * b[0] + a[1] * b[1];
#endif
}

float Vec2Ops::length_squared(const float4& v) {
    return dot(v, v);
}

float Vec2Ops::length(const float4& v) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 mul = _mm_mul_ps(v.data, v.data);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    __m128 sqrt = _mm_sqrt_ss(hadd);
    return _mm_cvtss_f32(sqrt);
#else
    return std::sqrt(length_squared(v));
#endif
}

void Vec2Ops::normalize(const float4& v, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 mul = _mm_mul_ps(v.data, v.data);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    __m128 len = _mm_sqrt_ps(hadd);
    __m128 mask = _mm_cmpgt_ps(len, _mm_set1_ps(1e-6f));
    __m128 rcp_len = _mm_div_ps(_mm_set1_ps(1.0f), len);
    result = float4(_mm_and_ps(_mm_mul_ps(v.data, rcp_len), mask));
#else
    float len = length(v);
    if (len > 1e-6f) {
        float inv_len = 1.0f / len;
        result[0] = v[0] * inv_len;
        result[1] = v[1] * inv_len;
    } else {
        result[0] = 0.0f;
        result[1] = 0.0f;
    }
    result[2] = 0.0f;
    result[3] = 0.0f;
#endif
}

// Batch operations for Vec2Ops

void Vec2Ops::add_batch2(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_add_ps(a.data, b.data));
#else
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
#endif
}

void Vec2Ops::subtract_batch2(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_sub_ps(a.data, b.data));
#else
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = a[3] - b[3];
#endif
}

void Vec2Ops::multiply_batch2(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_mul_ps(a.data, b.data));
#else
    result[0] = a[0] * b[0];
    result[1] = a[1] * b[1];
    result[2] = a[2] * b[2];
    result[3] = a[3] * b[3];
#endif
}

void Vec2Ops::multiply_scalar_batch2(const float4& a, float scalar, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_mul_ps(a.data, _mm_set1_ps(scalar)));
#else
    result[0] = a[0] * scalar;
    result[1] = a[1] * scalar;
    result[2] = a[2] * scalar;
    result[3] = a[3] * scalar;
#endif
}

void Vec2Ops::divide_batch2(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_div_ps(a.data, b.data));
#else
    result[0] = a[0] / b[0];
    result[1] = a[1] / b[1];
    result[2] = a[2] / b[2];
    result[3] = a[3] / b[3];
#endif
}

void Vec2Ops::divide_scalar_batch2(const float4& a, float scalar, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_div_ps(a.data, _mm_set1_ps(scalar)));
#else
    float inv_scalar = 1.0f / scalar;
    result[0] = a[0] * inv_scalar;
    result[1] = a[1] * inv_scalar;
    result[2] = a[2] * inv_scalar;
    result[3] = a[3] * inv_scalar;
#endif
}

// Vec3Ops Implementation

void Vec3Ops::add(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_add_ps(a.data, b.data));
    // Zero out w component
    __m128 mask = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
    result.data = _mm_and_ps(result.data, mask);
#else
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = 0.0f;
#endif
}

void Vec3Ops::subtract(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_sub_ps(a.data, b.data));
    // Zero out w component
    __m128 mask = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
    result.data = _mm_and_ps(result.data, mask);
#else
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = 0.0f;
#endif
}

void Vec3Ops::multiply(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_mul_ps(a.data, b.data));
    // Zero out w component
    __m128 mask = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
    result.data = _mm_and_ps(result.data, mask);
#else
    result[0] = a[0] * b[0];
    result[1] = a[1] * b[1];
    result[2] = a[2] * b[2];
    result[3] = 0.0f;
#endif
}

void Vec3Ops::multiply_scalar(const float4& a, float scalar, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 scalar_vec = _mm_set1_ps(scalar);
    result = float4(_mm_mul_ps(a.data, scalar_vec));
    // Zero out w component
    __m128 mask = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
    result.data = _mm_and_ps(result.data, mask);
#else
    result[0] = a[0] * scalar;
    result[1] = a[1] * scalar;
    result[2] = a[2] * scalar;
    result[3] = 0.0f;
#endif
}

void Vec3Ops::divide(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_div_ps(a.data, b.data));
    // Zero out w component
    __m128 mask = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
    result.data = _mm_and_ps(result.data, mask);
#else
    result[0] = a[0] / b[0];
    result[1] = a[1] / b[1];
    result[2] = a[2] / b[2];
    result[3] = 0.0f;
#endif
}

void Vec3Ops::divide_scalar(const float4& a, float scalar, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 scalar_vec = _mm_set1_ps(scalar);
    result = float4(_mm_div_ps(a.data, scalar_vec));
    // Zero out w component
    __m128 mask = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
    result.data = _mm_and_ps(result.data, mask);
#else
    float inv_scalar = 1.0f / scalar;
    result[0] = a[0] * inv_scalar;
    result[1] = a[1] * inv_scalar;
    result[2] = a[2] * inv_scalar;
    result[3] = 0.0f;
#endif
}

float Vec3Ops::dot(const float4& a, const float4& b) {
#if PYNOVAGE_SIMD_HAS_SSE4_1
    // Using SSE4.1's dot product instruction with mask for xyz
    __m128 mul = _mm_mul_ps(a.data, b.data);
    __m128 mask = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
    mul = _mm_and_ps(mul, mask);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    hadd = _mm_hadd_ps(hadd, hadd);
    return _mm_cvtss_f32(hadd);
#elif PYNOVAGE_SIMD_HAS_SSE3
    // Manual dot product with SSE3
    __m128 mul = _mm_mul_ps(a.data, b.data);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    hadd = _mm_hadd_ps(hadd, hadd);
    return _mm_cvtss_f32(hadd);
#else
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
#endif
}

void Vec3Ops::cross(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    // Shuffle components for cross product
    __m128 v1 = _mm_shuffle_ps(a.data, a.data, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 v2 = _mm_shuffle_ps(b.data, b.data, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 v3 = _mm_shuffle_ps(a.data, a.data, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 v4 = _mm_shuffle_ps(b.data, b.data, _MM_SHUFFLE(3, 0, 2, 1));

    __m128 mul1 = _mm_mul_ps(v1, v2);
    __m128 mul2 = _mm_mul_ps(v3, v4);
    __m128 sub = _mm_sub_ps(mul1, mul2);

    // Zero out w component
    __m128 mask = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
    result = float4(_mm_and_ps(sub, mask));
#else
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
    result[3] = 0.0f;
#endif
}

float Vec3Ops::length_squared(const float4& v) {
    return dot(v, v);
}

float Vec3Ops::length(const float4& v) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 mul = _mm_mul_ps(v.data, v.data);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    hadd = _mm_hadd_ps(hadd, hadd);
    __m128 sqrt = _mm_sqrt_ss(hadd);
    return _mm_cvtss_f32(sqrt);
#else
    return std::sqrt(length_squared(v));
#endif
}

void Vec3Ops::normalize(const float4& v, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 mul = _mm_mul_ps(v.data, v.data);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    hadd = _mm_hadd_ps(hadd, hadd);
    __m128 len = _mm_sqrt_ps(hadd);
    __m128 mask = _mm_cmpgt_ps(len, _mm_set1_ps(1e-6f));
    __m128 rcp_len = _mm_div_ps(_mm_set1_ps(1.0f), len);
    result = float4(_mm_and_ps(_mm_mul_ps(v.data, rcp_len), mask));
    // Ensure w component is 0
    result.data = _mm_and_ps(result.data, _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f));
#else
    float len = length(v);
    if (len > 1e-6f) {
        float inv_len = 1.0f / len;
        result[0] = v[0] * inv_len;
        result[1] = v[1] * inv_len;
        result[2] = v[2] * inv_len;
    } else {
        result[0] = 0.0f;
        result[1] = 0.0f;
        result[2] = 0.0f;
    }
    result[3] = 0.0f;
#endif
}

// Batch operations for Vec3Ops

void Vec3Ops::add_batch4(const float16& a, const float16& b, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_add_ps(a.data, b.data));
#elif PYNOVAGE_SIMD_HAS_AVX
    // Process 8 floats at a time using AVX
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_add_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0])));
    _mm256_storeu_ps(temp + 8, _mm256_add_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8])));
    result = float16::load(temp);
#else
    // Scalar fallback
    for (int i = 0; i < 12; i++) {
        result[i] = a[i] + b[i];
    }
    // Zero the w components
    result[3] = result[7] = result[11] = result[15] = 0.0f;
#endif
}

void Vec3Ops::subtract_batch4(const float16& a, const float16& b, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_sub_ps(a.data, b.data));
#elif PYNOVAGE_SIMD_HAS_AVX
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_sub_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0])));
    _mm256_storeu_ps(temp + 8, _mm256_sub_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8])));
    result = float16::load(temp);
#else
    for (int i = 0; i < 12; i++) {
        result[i] = a[i] - b[i];
    }
    result[3] = result[7] = result[11] = result[15] = 0.0f;
#endif
}

void Vec3Ops::multiply_batch4(const float16& a, const float16& b, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_mul_ps(a.data, b.data));
#elif PYNOVAGE_SIMD_HAS_AVX
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_mul_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0])));
    _mm256_storeu_ps(temp + 8, _mm256_mul_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8])));
    result = float16::load(temp);
#else
    for (int i = 0; i < 12; i++) {
        result[i] = a[i] * b[i];
    }
    result[3] = result[7] = result[11] = result[15] = 0.0f;
#endif
}

void Vec3Ops::multiply_scalar_batch4(const float16& a, float scalar, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_mul_ps(a.data, _mm512_set1_ps(scalar)));
#elif PYNOVAGE_SIMD_HAS_AVX
    __m256 scalar_vec = _mm256_set1_ps(scalar);
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_mul_ps(_mm256_loadu_ps(&a[0]), scalar_vec));
    _mm256_storeu_ps(temp + 8, _mm256_mul_ps(_mm256_loadu_ps(&a[8]), scalar_vec));
    result = float16::load(temp);
#else
    for (int i = 0; i < 12; i++) {
        result[i] = a[i] * scalar;
    }
    result[3] = result[7] = result[11] = result[15] = 0.0f;
#endif
}

void Vec3Ops::divide_batch4(const float16& a, const float16& b, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_div_ps(a.data, b.data));
#elif PYNOVAGE_SIMD_HAS_AVX
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_div_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0])));
    _mm256_storeu_ps(temp + 8, _mm256_div_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8])));
    result = float16::load(temp);
#else
    for (int i = 0; i < 12; i++) {
        result[i] = a[i] / b[i];
    }
    result[3] = result[7] = result[11] = result[15] = 0.0f;
#endif
}

void Vec3Ops::divide_scalar_batch4(const float16& a, float scalar, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_div_ps(a.data, _mm512_set1_ps(scalar)));
#elif PYNOVAGE_SIMD_HAS_AVX
    __m256 scalar_vec = _mm256_set1_ps(scalar);
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_div_ps(_mm256_loadu_ps(&a[0]), scalar_vec));
    _mm256_storeu_ps(temp + 8, _mm256_div_ps(_mm256_loadu_ps(&a[8]), scalar_vec));
    result = float16::load(temp);
#else
    float inv_scalar = 1.0f / scalar;
    for (int i = 0; i < 12; i++) {
        result[i] = a[i] * inv_scalar;
    }
    result[3] = result[7] = result[11] = result[15] = 0.0f;
#endif
}

void Vec3Ops::cross_batch4(const float16& a, const float16& b, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    // Implement cross product for 4 vectors at once using AVX-512
    __m512 a_yzx = _mm512_permute_ps(a.data, _MM_SHUFFLE(3, 0, 2, 1));
    __m512 b_zxy = _mm512_permute_ps(b.data, _MM_SHUFFLE(3, 1, 0, 2));
    __m512 a_zxy = _mm512_permute_ps(a.data, _MM_SHUFFLE(3, 1, 0, 2));
    __m512 b_yzx = _mm512_permute_ps(b.data, _MM_SHUFFLE(3, 0, 2, 1));
    
    __m512 mul1 = _mm512_mul_ps(a_yzx, b_zxy);
    __m512 mul2 = _mm512_mul_ps(a_zxy, b_yzx);
    result = float16(_mm512_sub_ps(mul1, mul2));

    // Zero out w components
    __m512 mask = _mm512_set_ps(
        0.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f
    );
    result = float16(_mm512_and_ps(result.data, mask));
#elif PYNOVAGE_SIMD_HAS_AVX
    float temp[16];
    // Process first two vectors
    __m256 a1 = _mm256_loadu_ps(&a[0]);
    __m256 b1 = _mm256_loadu_ps(&b[0]);
    __m256 a1_yzx = _mm256_permute_ps(a1, _MM_SHUFFLE(3, 0, 2, 1));
    __m256 b1_zxy = _mm256_permute_ps(b1, _MM_SHUFFLE(3, 1, 0, 2));
    __m256 a1_zxy = _mm256_permute_ps(a1, _MM_SHUFFLE(3, 1, 0, 2));
    __m256 b1_yzx = _mm256_permute_ps(b1, _MM_SHUFFLE(3, 0, 2, 1));
    _mm256_storeu_ps(temp, _mm256_sub_ps(
        _mm256_mul_ps(a1_yzx, b1_zxy),
        _mm256_mul_ps(a1_zxy, b1_yzx)
    ));

    // Process second two vectors
    __m256 a2 = _mm256_loadu_ps(&a[8]);
    __m256 b2 = _mm256_loadu_ps(&b[8]);
    __m256 a2_yzx = _mm256_permute_ps(a2, _MM_SHUFFLE(3, 0, 2, 1));
    __m256 b2_zxy = _mm256_permute_ps(b2, _MM_SHUFFLE(3, 1, 0, 2));
    __m256 a2_zxy = _mm256_permute_ps(a2, _MM_SHUFFLE(3, 1, 0, 2));
    __m256 b2_yzx = _mm256_permute_ps(b2, _MM_SHUFFLE(3, 0, 2, 1));
    _mm256_storeu_ps(temp + 8, _mm256_sub_ps(
        _mm256_mul_ps(a2_yzx, b2_zxy),
        _mm256_mul_ps(a2_zxy, b2_yzx)
    ));

    result = float16::load(temp);
    // Zero out w components
    result[3] = result[7] = result[11] = result[15] = 0.0f;
#else
    // Process each vector individually
    for (int i = 0; i < 4; i++) {
        int base = i * 4;
        result[base] = a[base + 1] * b[base + 2] - a[base + 2] * b[base + 1];
        result[base + 1] = a[base + 2] * b[base] - a[base] * b[base + 2];
        result[base + 2] = a[base] * b[base + 1] - a[base + 1] * b[base];
        result[base + 3] = 0.0f;
    }
#endif
}

// Vec4Ops Implementation

void Vec4Ops::add(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_add_ps(a.data, b.data));
#else
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
#endif
}

void Vec4Ops::subtract(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_sub_ps(a.data, b.data));
#else
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = a[3] - b[3];
#endif
}

void Vec4Ops::multiply(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_mul_ps(a.data, b.data));
#else
    result[0] = a[0] * b[0];
    result[1] = a[1] * b[1];
    result[2] = a[2] * b[2];
    result[3] = a[3] * b[3];
#endif
}

void Vec4Ops::multiply_scalar(const float4& a, float scalar, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 scalar_vec = _mm_set1_ps(scalar);
    result = float4(_mm_mul_ps(a.data, scalar_vec));
#else
    result[0] = a[0] * scalar;
    result[1] = a[1] * scalar;
    result[2] = a[2] * scalar;
    result[3] = a[3] * scalar;
#endif
}

void Vec4Ops::divide(const float4& a, const float4& b, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    result = float4(_mm_div_ps(a.data, b.data));
#else
    result[0] = a[0] / b[0];
    result[1] = a[1] / b[1];
    result[2] = a[2] / b[2];
    result[3] = a[3] / b[3];
#endif
}

void Vec4Ops::divide_scalar(const float4& a, float scalar, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 scalar_vec = _mm_set1_ps(scalar);
    result = float4(_mm_div_ps(a.data, scalar_vec));
#else
    float inv_scalar = 1.0f / scalar;
    result[0] = a[0] * inv_scalar;
    result[1] = a[1] * inv_scalar;
    result[2] = a[2] * inv_scalar;
    result[3] = a[3] * inv_scalar;
#endif
}

float Vec4Ops::dot(const float4& a, const float4& b) {
#if PYNOVAGE_SIMD_HAS_SSE4_1
    // Using SSE4.1's native dot product
    __m128 mul = _mm_dp_ps(a.data, b.data, 0xFF);
    return _mm_cvtss_f32(mul);
#elif PYNOVAGE_SIMD_HAS_SSE3
    // Manual dot product with SSE3
    __m128 mul = _mm_mul_ps(a.data, b.data);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    hadd = _mm_hadd_ps(hadd, hadd);
    return _mm_cvtss_f32(hadd);
#else
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
#endif
}

float Vec4Ops::length_squared(const float4& v) {
    return dot(v, v);
}

float Vec4Ops::length(const float4& v) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 mul = _mm_mul_ps(v.data, v.data);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    hadd = _mm_hadd_ps(hadd, hadd);
    __m128 sqrt = _mm_sqrt_ss(hadd);
    return _mm_cvtss_f32(sqrt);
#else
    return std::sqrt(length_squared(v));
#endif
}

void Vec4Ops::normalize(const float4& v, float4& result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 mul = _mm_mul_ps(v.data, v.data);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    hadd = _mm_hadd_ps(hadd, hadd);
    __m128 len = _mm_sqrt_ps(hadd);
    __m128 mask = _mm_cmpgt_ps(len, _mm_set1_ps(1e-6f));
    __m128 rcp_len = _mm_div_ps(_mm_set1_ps(1.0f), len);
    result = float4(_mm_and_ps(_mm_mul_ps(v.data, rcp_len), mask));
#else
    float len = length(v);
    if (len > 1e-6f) {
        float inv_len = 1.0f / len;
        result[0] = v[0] * inv_len;
        result[1] = v[1] * inv_len;
        result[2] = v[2] * inv_len;
        result[3] = v[3] * inv_len;
    } else {
        result[0] = result[1] = result[2] = result[3] = 0.0f;
    }
#endif
}

// Fused multiply-add operations

void Vec4Ops::multiply_add(const float4& a, const float4& b, const float4& c, float4& result) {
#if PYNOVAGE_SIMD_HAS_FMA
    // Use hardware FMA instruction
    result = float4(_mm_fmadd_ps(a.data, b.data, c.data));
#elif PYNOVAGE_SIMD_HAS_SSE
    // Fallback to separate multiply and add
    result = float4(_mm_add_ps(_mm_mul_ps(a.data, b.data), c.data));
#else
    result[0] = a[0] * b[0] + c[0];
    result[1] = a[1] * b[1] + c[1];
    result[2] = a[2] * b[2] + c[2];
    result[3] = a[3] * b[3] + c[3];
#endif
}

void Vec4Ops::multiply_sub(const float4& a, const float4& b, const float4& c, float4& result) {
#if PYNOVAGE_SIMD_HAS_FMA
    // Use hardware FMA instruction
    result = float4(_mm_fmsub_ps(a.data, b.data, c.data));
#elif PYNOVAGE_SIMD_HAS_SSE
    // Fallback to separate multiply and subtract
    result = float4(_mm_sub_ps(_mm_mul_ps(a.data, b.data), c.data));
#else
    result[0] = a[0] * b[0] - c[0];
    result[1] = a[1] * b[1] - c[1];
    result[2] = a[2] * b[2] - c[2];
    result[3] = a[3] * b[3] - c[3];
#endif
}

// Batch operations

void Vec4Ops::add_batch4(const float16& a, const float16& b, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_add_ps(a.data, b.data));
#elif PYNOVAGE_SIMD_HAS_AVX
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_add_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0])));
    _mm256_storeu_ps(temp + 8, _mm256_add_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8])));
    result = float16::load(temp);
#else
    for (int i = 0; i < 16; i++) {
        result[i] = a[i] + b[i];
    }
#endif
}

void Vec4Ops::subtract_batch4(const float16& a, const float16& b, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_sub_ps(a.data, b.data));
#elif PYNOVAGE_SIMD_HAS_AVX
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_sub_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0])));
    _mm256_storeu_ps(temp + 8, _mm256_sub_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8])));
    result = float16::load(temp);
#else
    for (int i = 0; i < 16; i++) {
        result[i] = a[i] - b[i];
    }
#endif
}

void Vec4Ops::multiply_batch4(const float16& a, const float16& b, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_mul_ps(a.data, b.data));
#elif PYNOVAGE_SIMD_HAS_AVX
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_mul_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0])));
    _mm256_storeu_ps(temp + 8, _mm256_mul_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8])));
    result = float16::load(temp);
#else
    for (int i = 0; i < 16; i++) {
        result[i] = a[i] * b[i];
    }
#endif
}

void Vec4Ops::multiply_scalar_batch4(const float16& a, float scalar, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_mul_ps(a.data, _mm512_set1_ps(scalar)));
#elif PYNOVAGE_SIMD_HAS_AVX
    __m256 scalar_vec = _mm256_set1_ps(scalar);
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_mul_ps(_mm256_loadu_ps(&a[0]), scalar_vec));
    _mm256_storeu_ps(temp + 8, _mm256_mul_ps(_mm256_loadu_ps(&a[8]), scalar_vec));
    result = float16::load(temp);
#else
    for (int i = 0; i < 16; i++) {
        result[i] = a[i] * scalar;
    }
#endif
}

void Vec4Ops::divide_batch4(const float16& a, const float16& b, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_div_ps(a.data, b.data));
#elif PYNOVAGE_SIMD_HAS_AVX
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_div_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0])));
    _mm256_storeu_ps(temp + 8, _mm256_div_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8])));
    result = float16::load(temp);
#else
    for (int i = 0; i < 16; i++) {
        result[i] = a[i] / b[i];
    }
#endif
}

void Vec4Ops::divide_scalar_batch4(const float16& a, float scalar, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    result = float16(_mm512_div_ps(a.data, _mm512_set1_ps(scalar)));
#elif PYNOVAGE_SIMD_HAS_AVX
    __m256 scalar_vec = _mm256_set1_ps(scalar);
    float temp[16];
    _mm256_storeu_ps(temp, _mm256_div_ps(_mm256_loadu_ps(&a[0]), scalar_vec));
    _mm256_storeu_ps(temp + 8, _mm256_div_ps(_mm256_loadu_ps(&a[8]), scalar_vec));
    result = float16::load(temp);
#else
    float inv_scalar = 1.0f / scalar;
    for (int i = 0; i < 16; i++) {
        result[i] = a[i] * inv_scalar;
    }
#endif
}

void Vec4Ops::multiply_add_batch4(const float16& a, const float16& b, const float16& c, float16& result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    #if PYNOVAGE_SIMD_HAS_FMA
        result = float16(_mm512_fmadd_ps(a.data, b.data, c.data));
    #else
        result = float16(_mm512_add_ps(_mm512_mul_ps(a.data, b.data), c.data));
    #endif
#elif PYNOVAGE_SIMD_HAS_AVX
    float temp[16];
    __m256 a1 = _mm256_loadu_ps(&a[0]);
    __m256 b1 = _mm256_loadu_ps(&b[0]);
    __m256 c1 = _mm256_loadu_ps(&c[0]);
    __m256 a2 = _mm256_loadu_ps(&a[8]);
    __m256 b2 = _mm256_loadu_ps(&b[8]);
    __m256 c2 = _mm256_loadu_ps(&c[8]);
    
    #if PYNOVAGE_SIMD_HAS_FMA
        _mm256_storeu_ps(temp, _mm256_fmadd_ps(a1, b1, c1));
        _mm256_storeu_ps(temp + 8, _mm256_fmadd_ps(a2, b2, c2));
    #else
        _mm256_storeu_ps(temp, _mm256_add_ps(_mm256_mul_ps(a1, b1), c1));
        _mm256_storeu_ps(temp + 8, _mm256_add_ps(_mm256_mul_ps(a2, b2), c2));
    #endif
    
    result = float16::load(temp);
#else
    for (int i = 0; i < 16; i++) {
        result[i] = a[i] * b[i] + c[i];
    }
#endif
}

// VecCompareOps Implementation

void VecCompareOps::less_than(const float4& a, const float4& b, int* result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 cmp = _mm_cmplt_ps(a.data, b.data);
    __m128i mask = _mm_castps_si128(cmp);
    // Extract results for each component
    result[0] = _mm_extract_epi32(mask, 0);
    result[1] = _mm_extract_epi32(mask, 1);
    result[2] = _mm_extract_epi32(mask, 2);
    result[3] = _mm_extract_epi32(mask, 3);
#else
    result[0] = a[0] < b[0] ? -1 : 0;
    result[1] = a[1] < b[1] ? -1 : 0;
    result[2] = a[2] < b[2] ? -1 : 0;
    result[3] = a[3] < b[3] ? -1 : 0;
#endif
}

void VecCompareOps::less_equal(const float4& a, const float4& b, int* result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 cmp = _mm_cmple_ps(a.data, b.data);
    __m128i mask = _mm_castps_si128(cmp);
    result[0] = _mm_extract_epi32(mask, 0);
    result[1] = _mm_extract_epi32(mask, 1);
    result[2] = _mm_extract_epi32(mask, 2);
    result[3] = _mm_extract_epi32(mask, 3);
#else
    result[0] = a[0] <= b[0] ? -1 : 0;
    result[1] = a[1] <= b[1] ? -1 : 0;
    result[2] = a[2] <= b[2] ? -1 : 0;
    result[3] = a[3] <= b[3] ? -1 : 0;
#endif
}

void VecCompareOps::greater_than(const float4& a, const float4& b, int* result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 cmp = _mm_cmpgt_ps(a.data, b.data);
    __m128i mask = _mm_castps_si128(cmp);
    result[0] = _mm_extract_epi32(mask, 0);
    result[1] = _mm_extract_epi32(mask, 1);
    result[2] = _mm_extract_epi32(mask, 2);
    result[3] = _mm_extract_epi32(mask, 3);
#else
    result[0] = a[0] > b[0] ? -1 : 0;
    result[1] = a[1] > b[1] ? -1 : 0;
    result[2] = a[2] > b[2] ? -1 : 0;
    result[3] = a[3] > b[3] ? -1 : 0;
#endif
}

void VecCompareOps::greater_equal(const float4& a, const float4& b, int* result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 cmp = _mm_cmpge_ps(a.data, b.data);
    __m128i mask = _mm_castps_si128(cmp);
    result[0] = _mm_extract_epi32(mask, 0);
    result[1] = _mm_extract_epi32(mask, 1);
    result[2] = _mm_extract_epi32(mask, 2);
    result[3] = _mm_extract_epi32(mask, 3);
#else
    result[0] = a[0] >= b[0] ? -1 : 0;
    result[1] = a[1] >= b[1] ? -1 : 0;
    result[2] = a[2] >= b[2] ? -1 : 0;
    result[3] = a[3] >= b[3] ? -1 : 0;
#endif
}

void VecCompareOps::equal(const float4& a, const float4& b, int* result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 cmp = _mm_cmpeq_ps(a.data, b.data);
    __m128i mask = _mm_castps_si128(cmp);
    result[0] = _mm_extract_epi32(mask, 0);
    result[1] = _mm_extract_epi32(mask, 1);
    result[2] = _mm_extract_epi32(mask, 2);
    result[3] = _mm_extract_epi32(mask, 3);
#else
    result[0] = a[0] == b[0] ? -1 : 0;
    result[1] = a[1] == b[1] ? -1 : 0;
    result[2] = a[2] == b[2] ? -1 : 0;
    result[3] = a[3] == b[3] ? -1 : 0;
#endif
}

void VecCompareOps::not_equal(const float4& a, const float4& b, int* result) {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 cmp = _mm_cmpneq_ps(a.data, b.data);
    __m128i mask = _mm_castps_si128(cmp);
    result[0] = _mm_extract_epi32(mask, 0);
    result[1] = _mm_extract_epi32(mask, 1);
    result[2] = _mm_extract_epi32(mask, 2);
    result[3] = _mm_extract_epi32(mask, 3);
#else
    result[0] = a[0] != b[0] ? -1 : 0;
    result[1] = a[1] != b[1] ? -1 : 0;
    result[2] = a[2] != b[2] ? -1 : 0;
    result[3] = a[3] != b[3] ? -1 : 0;
#endif
}

// Batch comparison operations

void VecCompareOps::less_than_batch4(const float16& a, const float16& b, int* result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    __mmask16 mask = _mm512_cmp_ps_mask(a.data, b.data, _CMP_LT_OQ);
    uint16_t bits = mask;
    for (int i = 0; i < 16; ++i) {
        result[i] = (bits & (1 << i)) ? -1 : 0;
    }
#elif PYNOVAGE_SIMD_HAS_AVX
    // Process first 8 elements
    __m256 cmp1 = _mm256_cmp_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0]), _CMP_LT_OQ);
    __m256i mask1 = _mm256_castps_si256(cmp1);
    // Process second 8 elements
    __m256 cmp2 = _mm256_cmp_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8]), _CMP_LT_OQ);
    __m256i mask2 = _mm256_castps_si256(cmp2);
    
    // Store results
    int* result_ptr = result;
    for (int i = 0; i < 8; ++i) {
        result_ptr[i] = _mm256_extract_epi32(mask1, i);
        result_ptr[i + 8] = _mm256_extract_epi32(mask2, i);
    }
#else
    for (int i = 0; i < 16; ++i) {
        result[i] = a[i] < b[i] ? -1 : 0;
    }
#endif
}

void VecCompareOps::less_equal_batch4(const float16& a, const float16& b, int* result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    __mmask16 mask = _mm512_cmp_ps_mask(a.data, b.data, _CMP_LE_OQ);
    uint16_t bits = mask;
    for (int i = 0; i < 16; ++i) {
        result[i] = (bits & (1 << i)) ? -1 : 0;
    }
#elif PYNOVAGE_SIMD_HAS_AVX
    __m256 cmp1 = _mm256_cmp_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0]), _CMP_LE_OQ);
    __m256i mask1 = _mm256_castps_si256(cmp1);
    __m256 cmp2 = _mm256_cmp_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8]), _CMP_LE_OQ);
    __m256i mask2 = _mm256_castps_si256(cmp2);
    
    int* result_ptr = result;
    for (int i = 0; i < 8; ++i) {
        result_ptr[i] = _mm256_extract_epi32(mask1, i);
        result_ptr[i + 8] = _mm256_extract_epi32(mask2, i);
    }
#else
    for (int i = 0; i < 16; ++i) {
        result[i] = a[i] <= b[i] ? -1 : 0;
    }
#endif
}

void VecCompareOps::greater_than_batch4(const float16& a, const float16& b, int* result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    __mmask16 mask = _mm512_cmp_ps_mask(a.data, b.data, _CMP_GT_OQ);
    uint16_t bits = mask;
    for (int i = 0; i < 16; ++i) {
        result[i] = (bits & (1 << i)) ? -1 : 0;
    }
#elif PYNOVAGE_SIMD_HAS_AVX
    __m256 cmp1 = _mm256_cmp_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0]), _CMP_GT_OQ);
    __m256i mask1 = _mm256_castps_si256(cmp1);
    __m256 cmp2 = _mm256_cmp_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8]), _CMP_GT_OQ);
    __m256i mask2 = _mm256_castps_si256(cmp2);
    
    int* result_ptr = result;
    for (int i = 0; i < 8; ++i) {
        result_ptr[i] = _mm256_extract_epi32(mask1, i);
        result_ptr[i + 8] = _mm256_extract_epi32(mask2, i);
    }
#else
    for (int i = 0; i < 16; ++i) {
        result[i] = a[i] > b[i] ? -1 : 0;
    }
#endif
}

void VecCompareOps::greater_equal_batch4(const float16& a, const float16& b, int* result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    __mmask16 mask = _mm512_cmp_ps_mask(a.data, b.data, _CMP_GE_OQ);
    uint16_t bits = mask;
    for (int i = 0; i < 16; ++i) {
        result[i] = (bits & (1 << i)) ? -1 : 0;
    }
#elif PYNOVAGE_SIMD_HAS_AVX
    __m256 cmp1 = _mm256_cmp_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0]), _CMP_GE_OQ);
    __m256i mask1 = _mm256_castps_si256(cmp1);
    __m256 cmp2 = _mm256_cmp_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8]), _CMP_GE_OQ);
    __m256i mask2 = _mm256_castps_si256(cmp2);
    
    int* result_ptr = result;
    for (int i = 0; i < 8; ++i) {
        result_ptr[i] = _mm256_extract_epi32(mask1, i);
        result_ptr[i + 8] = _mm256_extract_epi32(mask2, i);
    }
#else
    for (int i = 0; i < 16; ++i) {
        result[i] = a[i] >= b[i] ? -1 : 0;
    }
#endif
}

void VecCompareOps::equal_batch4(const float16& a, const float16& b, int* result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    __mmask16 mask = _mm512_cmp_ps_mask(a.data, b.data, _CMP_EQ_OQ);
    uint16_t bits = mask;
    for (int i = 0; i < 16; ++i) {
        result[i] = (bits & (1 << i)) ? -1 : 0;
    }
#elif PYNOVAGE_SIMD_HAS_AVX
    __m256 cmp1 = _mm256_cmp_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0]), _CMP_EQ_OQ);
    __m256i mask1 = _mm256_castps_si256(cmp1);
    __m256 cmp2 = _mm256_cmp_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8]), _CMP_EQ_OQ);
    __m256i mask2 = _mm256_castps_si256(cmp2);
    
    int* result_ptr = result;
    for (int i = 0; i < 8; ++i) {
        result_ptr[i] = _mm256_extract_epi32(mask1, i);
        result_ptr[i + 8] = _mm256_extract_epi32(mask2, i);
    }
#else
    for (int i = 0; i < 16; ++i) {
        result[i] = a[i] == b[i] ? -1 : 0;
    }
#endif
}

void VecCompareOps::not_equal_batch4(const float16& a, const float16& b, int* result) {
#if PYNOVAGE_SIMD_HAS_AVX512F
    __mmask16 mask = _mm512_cmp_ps_mask(a.data, b.data, _CMP_NEQ_OQ);
    uint16_t bits = mask;
    for (int i = 0; i < 16; ++i) {
        result[i] = (bits & (1 << i)) ? -1 : 0;
    }
#elif PYNOVAGE_SIMD_HAS_AVX
    __m256 cmp1 = _mm256_cmp_ps(_mm256_loadu_ps(&a[0]), _mm256_loadu_ps(&b[0]), _CMP_NEQ_OQ);
    __m256i mask1 = _mm256_castps_si256(cmp1);
    __m256 cmp2 = _mm256_cmp_ps(_mm256_loadu_ps(&a[8]), _mm256_loadu_ps(&b[8]), _CMP_NEQ_OQ);
    __m256i mask2 = _mm256_castps_si256(cmp2);
    
    int* result_ptr = result;
    for (int i = 0; i < 8; ++i) {
        result_ptr[i] = _mm256_extract_epi32(mask1, i);
        result_ptr[i + 8] = _mm256_extract_epi32(mask2, i);
    }
#else
    for (int i = 0; i < 16; ++i) {
        result[i] = a[i] != b[i] ? -1 : 0;
    }
#endif
}

} // namespace simd
} // namespace foundation
} // namespace pynovage