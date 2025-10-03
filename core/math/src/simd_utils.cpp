#include "math/simd_utils.hpp"
#include <cmath>

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

// ------- Matrix 2x2 operations -------
void SimdUtils::MultiplyMatrix2x2(const float* a, const float* b, float* result) {
    // Row-major: a = [a00 a01; a10 a11], b = [b00 b01; b10 b11]
    result[0] = a[0]*b[0] + a[1]*b[2];
    result[1] = a[0]*b[1] + a[1]*b[3];
    result[2] = a[2]*b[0] + a[3]*b[2];
    result[3] = a[2]*b[1] + a[3]*b[3];
}

void SimdUtils::MultiplyMatrix2x2Vec2(const float* m, const float* v, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 row0 = _mm_set_ps(0.0f, 0.0f, m[1], m[0]);
    __m128 row1 = _mm_set_ps(0.0f, 0.0f, m[3], m[2]);
    __m128 vv   = _mm_set_ps(0.0f, 0.0f, v[1], v[0]);
    __m128 mul0 = _mm_mul_ps(row0, vv);
    __m128 mul1 = _mm_mul_ps(row1, vv);
    __m128 sum0 = _mm_hadd_ps(mul0, mul0);
    __m128 sum1 = _mm_hadd_ps(mul1, mul1);
    result[0] = _mm_cvtss_f32(sum0);
    result[1] = _mm_cvtss_f32(sum1);
#else
    result[0] = m[0]*v[0] + m[1]*v[1];
    result[1] = m[2]*v[0] + m[3]*v[1];
#endif
}

void SimdUtils::TransposeMatrix2x2(float* m) {
    float tmp = m[1];
    m[1] = m[2];
    m[2] = tmp;
}

float SimdUtils::DeterminantMatrix2x2(const float* m) {
    return m[0]*m[3] - m[1]*m[2];
}

bool SimdUtils::InvertMatrix2x2(const float* m, float* result) {
    float det = DeterminantMatrix2x2(m);
    if (std::fabs(det) < 1e-12f) return false;
    float invDet = 1.0f / det;
    result[0] =  m[3] * invDet;
    result[1] = -m[1] * invDet;
    result[2] = -m[2] * invDet;
    result[3] =  m[0] * invDet;
    return true;
}

} // namespace math
} // namespace pynovage