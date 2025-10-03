#include "math/simd_utils.hpp"

namespace pynovage {
namespace math {

bool SimdUtils::HasSSE() {
#if PYNOVAGE_MATH_HAS_SSE
    return true;
#else
    return false;
#endif
}

bool SimdUtils::HasSSE2() {
#if PYNOVAGE_MATH_HAS_SSE2
    return true;
#else
    return false;
#endif
}

bool SimdUtils::HasAVX() {
#if PYNOVAGE_MATH_HAS_AVX
    return true;
#else
    return false;
#endif
}

void SimdUtils::Add2f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_add_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
#else
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
#endif
}

void SimdUtils::Subtract2f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_sub_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
#else
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
#endif
}

void SimdUtils::Multiply2f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_mul_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
#else
    result[0] = a[0] * b[0];
    result[1] = a[1] * b[1];
#endif
}

void SimdUtils::Divide2f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_div_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
#else
    result[0] = a[0] / b[0];
    result[1] = a[1] / b[1];
#endif
}

float SimdUtils::DotProduct2f(const float* a, const float* b) {
#if PYNOVAGE_MATH_HAS_SSE4_1
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 mul = _mm_mul_ps(va, vb);
    __m128 hadd = _mm_hadd_ps(mul, mul);
    return _mm_cvtss_f32(hadd);
#else
    return a[0] * b[0] + a[1] * b[1];
#endif
}

void SimdUtils::Add3f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);  // Load 4 floats (we only need 3)
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_add_ps(va, vb);
    _mm_storeu_ps(result, vr);     // Store first 3 floats
#else
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
#endif
}

void SimdUtils::Subtract3f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_sub_ps(va, vb);
    _mm_storeu_ps(result, vr);
#else
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
#endif
}

void SimdUtils::Multiply3f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_mul_ps(va, vb);
    _mm_storeu_ps(result, vr);
#else
    result[0] = a[0] * b[0];
    result[1] = a[1] * b[1];
    result[2] = a[2] * b[2];
#endif
}

void SimdUtils::Divide3f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_div_ps(va, vb);
    _mm_storeu_ps(result, vr);
#else
    result[0] = a[0] / b[0];
    result[1] = a[1] / b[1];
    result[2] = a[2] / b[2];
#endif
}

float SimdUtils::DotProduct3f(const float* a, const float* b) {
#if PYNOVAGE_MATH_HAS_SSE4_1
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 mul = _mm_mul_ps(va, vb);
    // First horizontal add gets us (a0*b0 + a1*b1, a2*b2 + 0, ...)
    __m128 hadd1 = _mm_hadd_ps(mul, mul);
    // Second horizontal add sums all components
    __m128 hadd2 = _mm_hadd_ps(hadd1, hadd1);
    return _mm_cvtss_f32(hadd2);
#else
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
#endif
}

void SimdUtils::CrossProduct3f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    // Load vectors with an extra 0 for padding
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);

    // Shuffle components for cross product calculation
    // Cross product = (a1*b2 - a2*b1, a2*b0 - a0*b2, a0*b1 - a1*b0)
    __m128 v1 = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 v2 = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 v3 = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 v4 = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 0, 2, 1));

    __m128 mul1 = _mm_mul_ps(v1, v2);
    __m128 mul2 = _mm_mul_ps(v3, v4);
    __m128 vr = _mm_sub_ps(mul1, mul2);

    _mm_storeu_ps(result, vr);
#else
    // Cross product calculation: (a1*b2 - a2*b1, a2*b0 - a0*b2, a0*b1 - a1*b0)
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
#endif
}

} // namespace math
} // namespace pynovage