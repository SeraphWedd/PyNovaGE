#ifndef PYNOVAGE_FOUNDATION_MATH_SIMD_TYPES_HPP
#define PYNOVAGE_FOUNDATION_MATH_SIMD_TYPES_HPP

#include "simd/config.hpp"
#include <cstring> // for memcpy

namespace pynovage {
namespace foundation {
namespace simd {

/**
 * @brief SIMD-optimized 4-float vector type.
 * 
 * This type provides a transparent wrapper over either SSE __m128 or a
 * standard float array, depending on SIMD availability. All operations
 * are implemented using SIMD instructions when available.
 */
struct alignas(16) float4 {
#if PYNOVAGE_SIMD_HAS_SSE
    __m128 data;
#else
    float data[4];
#endif

    // Constructors
    float4() = default;

#if PYNOVAGE_SIMD_HAS_SSE
    explicit float4(__m128 v) : data(v) {}
#endif

    explicit float4(float x, float y, float z, float w) {
#if PYNOVAGE_SIMD_HAS_SSE
        data = _mm_set_ps(w, z, y, x);
#else
        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;
#endif
    }

    // Static factory methods
    static float4 zero() {
#if PYNOVAGE_SIMD_HAS_SSE
        return float4(_mm_setzero_ps());
#else
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
#endif
    }

    static float4 load(const float* ptr) {
        float4 result;
#if PYNOVAGE_SIMD_HAS_SSE
        result.data = _mm_loadu_ps(ptr);
#else
        std::memcpy(result.data, ptr, sizeof(float) * 4);
#endif
        return result;
    }

    static float4 load_aligned(const float* ptr) {
        float4 result;
#if PYNOVAGE_SIMD_HAS_SSE
        result.data = _mm_load_ps(ptr);
#else
        std::memcpy(result.data, ptr, sizeof(float) * 4);
#endif
        return result;
    }

    static float4 broadcast(float value) {
#if PYNOVAGE_SIMD_HAS_SSE
        return float4(_mm_set1_ps(value));
#else
        return float4(value, value, value, value);
#endif
    }

    // Store operations
    void store(float* ptr) const {
#if PYNOVAGE_SIMD_HAS_SSE
        _mm_storeu_ps(ptr, data);
#else
        std::memcpy(ptr, data, sizeof(float) * 4);
#endif
    }

    void store_aligned(float* ptr) const {
#if PYNOVAGE_SIMD_HAS_SSE
        _mm_store_ps(ptr, data);
#else
        std::memcpy(ptr, data, sizeof(float) * 4);
#endif
    }

    // Element access
    float& operator[](int index) {
#if PYNOVAGE_SIMD_HAS_SSE
        return reinterpret_cast<float*>(&data)[index];
#else
        return data[index];
#endif
    }

    const float& operator[](int index) const {
#if PYNOVAGE_SIMD_HAS_SSE
        return reinterpret_cast<const float*>(&data)[index];
#else
        return data[index];
#endif
    }
};

/**
 * @brief SIMD-optimized 8-float vector type.
 * 
 * This type provides a transparent wrapper over either AVX __m256 or a
 * standard float array, depending on SIMD availability. All operations
 * are implemented using AVX instructions when available.
 */
struct alignas(32) float8 {
#if PYNOVAGE_SIMD_HAS_AVX
    __m256 data;
#else
    float data[8];
#endif

    // Constructors
    float8() = default;

#if PYNOVAGE_SIMD_HAS_AVX
    explicit float8(__m256 v) : data(v) {}
#endif

    explicit float8(float v0, float v1, float v2, float v3,
                   float v4, float v5, float v6, float v7) {
#if PYNOVAGE_SIMD_HAS_AVX
        data = _mm256_set_ps(v7, v6, v5, v4, v3, v2, v1, v0);
#else
        data[0] = v0; data[1] = v1; data[2] = v2; data[3] = v3;
        data[4] = v4; data[5] = v5; data[6] = v6; data[7] = v7;
#endif
    }

    // Static factory methods
    static float8 zero() {
#if PYNOVAGE_SIMD_HAS_AVX
        return float8(_mm256_setzero_ps());
#else
        return float8(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
#endif
    }

    static float8 load(const float* ptr) {
        float8 result;
#if PYNOVAGE_SIMD_HAS_AVX
        result.data = _mm256_loadu_ps(ptr);
#else
        std::memcpy(result.data, ptr, sizeof(float) * 8);
#endif
        return result;
    }

    static float8 load_aligned(const float* ptr) {
        float8 result;
#if PYNOVAGE_SIMD_HAS_AVX
        result.data = _mm256_load_ps(ptr);
#else
        std::memcpy(result.data, ptr, sizeof(float) * 8);
#endif
        return result;
    }

    static float8 broadcast(float value) {
#if PYNOVAGE_SIMD_HAS_AVX
        return float8(_mm256_set1_ps(value));
#else
        return float8(value, value, value, value,
                     value, value, value, value);
#endif
    }

    // Store operations
    void store(float* ptr) const {
#if PYNOVAGE_SIMD_HAS_AVX
        _mm256_storeu_ps(ptr, data);
#else
        std::memcpy(ptr, data, sizeof(float) * 8);
#endif
    }

    void store_aligned(float* ptr) const {
#if PYNOVAGE_SIMD_HAS_AVX
        _mm256_store_ps(ptr, data);
#else
        std::memcpy(ptr, data, sizeof(float) * 8);
#endif
    }

    // Element access
    float& operator[](int index) {
#if PYNOVAGE_SIMD_HAS_AVX
        return reinterpret_cast<float*>(&data)[index];
#else
        return data[index];
#endif
    }

    const float& operator[](int index) const {
#if PYNOVAGE_SIMD_HAS_AVX
        return reinterpret_cast<const float*>(&data)[index];
#else
        return data[index];
#endif
    }
};

} // namespace simd
} // namespace foundation
/**
 * @brief SIMD-optimized 16-float vector type.
 * 
 * This type provides a transparent wrapper over either AVX-512 __m512 or a
 * standard float array, depending on SIMD availability. All operations
 * are implemented using AVX-512 instructions when available.
 */
struct alignas(64) float16 {
#if PYNOVAGE_SIMD_HAS_AVX512F
    __m512 data;
#else
    float data[16];
#endif

    // Constructors
    float16() = default;

#if PYNOVAGE_SIMD_HAS_AVX512F
    explicit float16(__m512 v) : data(v) {}
#endif

    explicit float16(float v0, float v1, float v2, float v3,
                    float v4, float v5, float v6, float v7,
                    float v8, float v9, float v10, float v11,
                    float v12, float v13, float v14, float v15) {
#if PYNOVAGE_SIMD_HAS_AVX512F
        data = _mm512_set_ps(v15, v14, v13, v12, v11, v10, v9, v8,
                            v7, v6, v5, v4, v3, v2, v1, v0);
#else
        data[0] = v0;   data[1] = v1;   data[2] = v2;   data[3] = v3;
        data[4] = v4;   data[5] = v5;   data[6] = v6;   data[7] = v7;
        data[8] = v8;   data[9] = v9;   data[10] = v10; data[11] = v11;
        data[12] = v12; data[13] = v13; data[14] = v14; data[15] = v15;
#endif
    }

    // Static factory methods
    static float16 zero() {
#if PYNOVAGE_SIMD_HAS_AVX512F
        return float16(_mm512_setzero_ps());
#else
        return float16(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
#endif
    }

    static float16 load(const float* ptr) {
        float16 result;
#if PYNOVAGE_SIMD_HAS_AVX512F
        result.data = _mm512_loadu_ps(ptr);
#else
        std::memcpy(result.data, ptr, sizeof(float) * 16);
#endif
        return result;
    }

    static float16 load_aligned(const float* ptr) {
        float16 result;
#if PYNOVAGE_SIMD_HAS_AVX512F
        result.data = _mm512_load_ps(ptr);
#else
        std::memcpy(result.data, ptr, sizeof(float) * 16);
#endif
        return result;
    }

    static float16 broadcast(float value) {
#if PYNOVAGE_SIMD_HAS_AVX512F
        return float16(_mm512_set1_ps(value));
#else
        return float16(value, value, value, value, value, value, value, value,
                      value, value, value, value, value, value, value, value);
#endif
    }

    // Store operations
    void store(float* ptr) const {
#if PYNOVAGE_SIMD_HAS_AVX512F
        _mm512_storeu_ps(ptr, data);
#else
        std::memcpy(ptr, data, sizeof(float) * 16);
#endif
    }

    void store_aligned(float* ptr) const {
#if PYNOVAGE_SIMD_HAS_AVX512F
        _mm512_store_ps(ptr, data);
#else
        std::memcpy(ptr, data, sizeof(float) * 16);
#endif
    }

    // Element access
    float& operator[](int index) {
#if PYNOVAGE_SIMD_HAS_AVX512F
        return reinterpret_cast<float*>(&data)[index];
#else
        return data[index];
#endif
    }

    const float& operator[](int index) const {
#if PYNOVAGE_SIMD_HAS_AVX512F
        return reinterpret_cast<const float*>(&data)[index];
#else
        return data[index];
#endif
    }
};

} // namespace pynovage

#endif // PYNOVAGE_FOUNDATION_MATH_SIMD_TYPES_HPP