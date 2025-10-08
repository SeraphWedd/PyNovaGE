#include "simd_utils.hpp"
#include <algorithm>

namespace pynovage {
namespace math {

void SimdUtils::Multiply2fScalar(const float* a, float scalar, float* result) {
    float scalars[2] = {scalar, scalar};
    Multiply2f(a, scalars, result);
}

void SimdUtils::Divide2fScalar(const float* a, float scalar, float* result) {
    float scalars[2] = {scalar, scalar};
    Divide2f(a, scalars, result);
}

void SimdUtils::Multiply3fScalar(const float* a, float scalar, float* result) {
    float scalars[4] = {scalar, scalar, scalar, 0.0f};
    Multiply3f(a, scalars, result);
}

void SimdUtils::Divide3fScalar(const float* a, float scalar, float* result) {
    float scalars[4] = {scalar, scalar, scalar, 1.0f};
    Divide3f(a, scalars, result);
}

void SimdUtils::Multiply4fScalar(const float* a, float scalar, float* result) {
    float scalars[4] = {scalar, scalar, scalar, scalar};
    Multiply4f(a, scalars, result);
}

void SimdUtils::Divide4fScalar(const float* a, float scalar, float* result) {
    float scalars[4] = {scalar, scalar, scalar, scalar};
    Divide4f(a, scalars, result);
}

void SimdUtils::Min4f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_min_ps(va, vb);
    _mm_storeu_ps(result, vr);
#else
    for (int i = 0; i < 4; ++i) {
        result[i] = std::min(a[i], b[i]);
    }
#endif
}

void SimdUtils::Max4f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_max_ps(va, vb);
    _mm_storeu_ps(result, vr);
#else
    for (int i = 0; i < 4; ++i) {
        result[i] = std::max(a[i], b[i]);
    }
#endif
}

} // namespace math
} // namespace pynovage
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

// ------- Matrix 3x3 operations -------
void SimdUtils::MultiplyMatrix3(const float* a, const float* b, float* result) {
    // a and b are 9-length arrays in row-major order
    result[0] = a[0]*b[0] + a[1]*b[3] + a[2]*b[6];
    result[1] = a[0]*b[1] + a[1]*b[4] + a[2]*b[7];
    result[2] = a[0]*b[2] + a[1]*b[5] + a[2]*b[8];

    result[3] = a[3]*b[0] + a[4]*b[3] + a[5]*b[6];
    result[4] = a[3]*b[1] + a[4]*b[4] + a[5]*b[7];
    result[5] = a[3]*b[2] + a[4]*b[5] + a[5]*b[8];

    result[6] = a[6]*b[0] + a[7]*b[3] + a[8]*b[6];
    result[7] = a[6]*b[1] + a[7]*b[4] + a[8]*b[7];
    result[8] = a[6]*b[2] + a[7]*b[5] + a[8]*b[8];
}

void SimdUtils::MultiplyMatrix3Vec3(const float* m, const float* v, float* result) {
    result[0] = m[0]*v[0] + m[1]*v[1] + m[2]*v[2];
    result[1] = m[3]*v[0] + m[4]*v[1] + m[5]*v[2];
    result[2] = m[6]*v[0] + m[7]*v[1] + m[8]*v[2];
}

void SimdUtils::TransposeMatrix3(float* m) {
    // swap m[1] <-> m[3], m[2] <-> m[6], m[5] <-> m[7]
    float tmp;
    tmp = m[1]; m[1] = m[3]; m[3] = tmp;
    tmp = m[2]; m[2] = m[6]; m[6] = tmp;
    tmp = m[5]; m[5] = m[7]; m[7] = tmp;
}

float SimdUtils::DeterminantMatrix3(const float* m) {
    // Using rule of Sarrus
    float det =
        m[0]*(m[4]*m[8] - m[5]*m[7]) -
        m[1]*(m[3]*m[8] - m[5]*m[6]) +
        m[2]*(m[3]*m[7] - m[4]*m[6]);
    return det;
}

bool SimdUtils::InvertMatrix3(const float* m, float* result) {
float det = DeterminantMatrix3(m);
    if (std::fabs(det) < 1e-12f) return false;
    float invDet = 1.0f / det;

    // Compute adjugate (transpose of cofactor matrix)
    result[0] =  (m[4]*m[8] - m[5]*m[7]) * invDet;
    result[1] = -(m[1]*m[8] - m[2]*m[7]) * invDet;
    result[2] =  (m[1]*m[5] - m[2]*m[4]) * invDet;

    result[3] = -(m[3]*m[8] - m[5]*m[6]) * invDet;
    result[4] =  (m[0]*m[8] - m[2]*m[6]) * invDet;
    result[5] = -(m[0]*m[5] - m[2]*m[3]) * invDet;

    result[6] =  (m[3]*m[7] - m[4]*m[6]) * invDet;
    result[7] = -(m[0]*m[7] - m[1]*m[6]) * invDet;
    result[8] =  (m[0]*m[4] - m[1]*m[3]) * invDet;

    return true;
}

void SimdUtils::TestAxisOverlap4f(float min_a, float max_a, const float* mins, const float* maxs, int* result) {
#if PYNOVAGE_MATH_HAS_SSE2
    __m128 vMinA = _mm_set1_ps(min_a);
    __m128 vMaxA = _mm_set1_ps(max_a);
    __m128 vMins = _mm_loadu_ps(mins);
    __m128 vMaxs = _mm_loadu_ps(maxs);

    __m128 cmp1 = _mm_cmple_ps(vMins, vMaxA); // mins <= max_a
    __m128 cmp2 = _mm_cmple_ps(vMinA, vMaxs); // min_a <= maxs
    __m128 overlap = _mm_and_ps(cmp1, cmp2);

    _mm_storeu_ps((float*)result, overlap);
    // Convert mask floats to int 0/1
    for (int i = 0; i < 4; ++i) result[i] = result[i] ? 1 : 0;
#else
    for (int i = 0; i < 4; ++i) {
        result[i] = (mins[i] <= max_a) && (min_a <= maxs[i]);
    }
#endif
}

void SimdUtils::TestAABBOverlap4f(const float* min_a, const float* max_a, const float* mins, const float* maxs, int* result) {
#if PYNOVAGE_MATH_HAS_SSE2
    __m128 minAx = _mm_set1_ps(min_a[0]);
    __m128 minAy = _mm_set1_ps(min_a[1]);
    __m128 minAz = _mm_set1_ps(min_a[2]);
    __m128 maxAx = _mm_set1_ps(max_a[0]);
    __m128 maxAy = _mm_set1_ps(max_a[1]);
    __m128 maxAz = _mm_set1_ps(max_a[2]);

    __m128 minBx = _mm_loadu_ps(&mins[0]);
    __m128 minBy = _mm_loadu_ps(&mins[4]);
    __m128 minBz = _mm_loadu_ps(&mins[8]);
    __m128 maxBx = _mm_loadu_ps(&maxs[0]);
    __m128 maxBy = _mm_loadu_ps(&maxs[4]);
    __m128 maxBz = _mm_loadu_ps(&maxs[8]);

    __m128 cmpX = _mm_and_ps(_mm_cmple_ps(minBx, maxAx), _mm_cmple_ps(minAx, maxBx));
    __m128 cmpY = _mm_and_ps(_mm_cmple_ps(minBy, maxAy), _mm_cmple_ps(minAy, maxBy));
    __m128 cmpZ = _mm_and_ps(_mm_cmple_ps(minBz, maxAz), _mm_cmple_ps(minAz, maxBz));

    __m128 overlap = _mm_and_ps(_mm_and_ps(cmpX, cmpY), cmpZ);
    _mm_storeu_ps((float*)result, overlap);
    for (int i = 0; i < 4; ++i) result[i] = result[i] ? 1 : 0;
#else
    for (int i = 0; i < 4; ++i) {
        float minBx = mins[i];
        float minBy = mins[4 + i];
        float minBz = mins[8 + i];
        float maxBx = maxs[i];
        float maxBy = maxs[4 + i];
        float maxBz = maxs[8 + i];
        result[i] = (minBx <= max_a[0] && min_a[0] <= maxBx) &&
                    (minBy <= max_a[1] && min_a[1] <= maxBy) &&
                    (minBz <= max_a[2] && min_a[2] <= maxBz);
    }
#endif
}

void SimdUtils::Add4f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_add_ps(va, vb);
    _mm_storeu_ps(result, vr);
#else
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
#endif
}

void SimdUtils::Subtract4f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_sub_ps(va, vb);
    _mm_storeu_ps(result, vr);
#else
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = a[3] - b[3];
#endif
}

void SimdUtils::Multiply4f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_mul_ps(va, vb);
    _mm_storeu_ps(result, vr);
#else
    result[0] = a[0] * b[0];
    result[1] = a[1] * b[1];
    result[2] = a[2] * b[2];
    result[3] = a[3] * b[3];
#endif
}

void SimdUtils::Divide4f(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_div_ps(va, vb);
    _mm_storeu_ps(result, vr);
#else
    result[0] = a[0] / b[0];
    result[1] = a[1] / b[1];
    result[2] = a[2] / b[2];
    result[3] = a[3] / b[3];
#endif
}

void SimdUtils::Fill4f(float* result, float value) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 vval = _mm_set1_ps(value);
    _mm_storeu_ps(result, vval);
#else
    result[0] = value;
    result[1] = value;
    result[2] = value;
    result[3] = value;
#endif
}

void SimdUtils::Sqrt4f(const float* a, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 va = _mm_loadu_ps(a);
    __m128 vr = _mm_sqrt_ps(va);
    _mm_storeu_ps(result, vr);
#else
    result[0] = std::sqrt(a[0]);
    result[1] = std::sqrt(a[1]);
    result[2] = std::sqrt(a[2]);
    result[3] = std::sqrt(a[3]);
#endif
}

float SimdUtils::DotProduct4f(const float* a, const float* b) {
#if PYNOVAGE_MATH_HAS_SSE4_1
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 mul = _mm_mul_ps(va, vb);
    // First horizontal add gets us (a0*b0 + a1*b1, a2*b2 + a3*b3, ...)
    __m128 hadd1 = _mm_hadd_ps(mul, mul);
    // Second horizontal add sums all components
    __m128 hadd2 = _mm_hadd_ps(hadd1, hadd1);
    return _mm_cvtss_f32(hadd2);
#else
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
#endif
}

void SimdUtils::MultiplyMatrix4x4(const float* a, const float* b, float* result) {
#if PYNOVAGE_MATH_HAS_AVX
    // Use AVX for faster matrix multiplication
    // Load the rows of matrix A
    __m256 a_row0 = _mm256_broadcast_ps((__m128*)&a[0]);
    __m256 a_row1 = _mm256_broadcast_ps((__m128*)&a[4]);
    __m256 a_row2 = _mm256_broadcast_ps((__m128*)&a[8]);
    __m256 a_row3 = _mm256_broadcast_ps((__m128*)&a[12]);

    // Load matrix B into two AVX registers (each containing two columns)
    __m256 b_col01 = _mm256_loadu_ps(&b[0]);
    __m256 b_col23 = _mm256_loadu_ps(&b[8]);

    // Process first two columns
    __m256 row0_result = _mm256_mul_ps(a_row0, b_col01);
    __m256 row1_result = _mm256_mul_ps(a_row1, b_col01);
    __m256 row2_result = _mm256_mul_ps(a_row2, b_col01);
    __m256 row3_result = _mm256_mul_ps(a_row3, b_col01);

    // Process last two columns and accumulate
    row0_result = _mm256_add_ps(row0_result, _mm256_mul_ps(a_row0, b_col23));
    row1_result = _mm256_add_ps(row1_result, _mm256_mul_ps(a_row1, b_col23));
    row2_result = _mm256_add_ps(row2_result, _mm256_mul_ps(a_row2, b_col23));
    row3_result = _mm256_add_ps(row3_result, _mm256_mul_ps(a_row3, b_col23));

    // Store results
    _mm256_storeu_ps(&result[0], row0_result);
    _mm256_storeu_ps(&result[8], row1_result);

#elif PYNOVAGE_MATH_HAS_SSE
    // SSE fallback - still much faster than scalar
    for (int i = 0; i < 4; ++i) {
        __m128 row = _mm_loadu_ps(&a[i*4]);
        
        for (int j = 0; j < 4; ++j) {
            // Broadcast the j-th column element to all elements
            __m128 bCol = _mm_set_ps(b[j+12], b[j+8], b[j+4], b[j]);
            __m128 mul = _mm_mul_ps(row, bCol);
            
            // Horizontal add using SSE3 hadd
            mul = _mm_hadd_ps(mul, mul);
            mul = _mm_hadd_ps(mul, mul);
            
            // Store the result
            result[i*4 + j] = _mm_cvtss_f32(mul);
        }
    }
#else
    // Scalar fallback with loop unrolling for better pipelining
    for (int i = 0; i < 4; ++i) {
        const float* row = &a[i*4];
        float* resultRow = &result[i*4];
        
        // Unrolled inner loops
        for (int j = 0; j < 4; ++j) {
            float sum = row[0] * b[j];
            sum += row[1] * b[j+4];
            sum += row[2] * b[j+8];
            sum += row[3] * b[j+12];
            resultRow[j] = sum;
        }
    }
#endif
}

void SimdUtils::MultiplyMatrix4x4Vec4(const float* m, const float* v, float* result) {
#if PYNOVAGE_MATH_HAS_AVX
    // Load vector into AVX register, broadcast to both 128-bit lanes
    __m256 vVec = _mm256_broadcast_ps((__m128*)v);

    // Process two rows at a time with AVX
    __m256 row01 = _mm256_loadu_ps(&m[0]);   // Load first two rows
    __m256 row23 = _mm256_loadu_ps(&m[8]);   // Load second two rows
    
    // Multiply and horizontal add
    __m256 prod01 = _mm256_mul_ps(row01, vVec);
    __m256 prod23 = _mm256_mul_ps(row23, vVec);
    
    // Horizontal add within 128-bit lanes
    __m256 sum01 = _mm256_hadd_ps(prod01, prod01);
    __m256 sum23 = _mm256_hadd_ps(prod23, prod23);
    sum01 = _mm256_hadd_ps(sum01, sum01);
    sum23 = _mm256_hadd_ps(sum23, sum23);
    
    // Extract results
    __m128 low = _mm256_castps256_ps128(sum01);
    __m128 high = _mm256_castps256_ps128(sum23);
    _mm_storel_pi((__m64*)&result[0], low);   // Store first two results
    _mm_storel_pi((__m64*)&result[2], high);  // Store second two results
    
#elif PYNOVAGE_MATH_HAS_SSE
    // Load vector once
    __m128 vVec = _mm_loadu_ps(v);
    
    for(int i = 0; i < 4; i++) {
        // Load matrix row and multiply
        __m128 row = _mm_loadu_ps(&m[i*4]);
        __m128 prod = _mm_mul_ps(row, vVec);
        
        // Horizontal add with SSE3
        prod = _mm_hadd_ps(prod, prod);
        prod = _mm_hadd_ps(prod, prod);
        
        // Store result
        result[i] = _mm_cvtss_f32(prod);
    }
#else
    // Scalar version with loop unrolling
    for(int i = 0; i < 4; i++) {
        const float* row = &m[i*4];
        float sum = row[0] * v[0];
        sum += row[1] * v[1];
        sum += row[2] * v[2];
        sum += row[3] * v[3];
        result[i] = sum;
    }
#endif
}

void SimdUtils::TransposeMatrix4x4(float* m) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 row1 = _mm_loadu_ps(&m[0]);
    __m128 row2 = _mm_loadu_ps(&m[4]);
    __m128 row3 = _mm_loadu_ps(&m[8]);
    __m128 row4 = _mm_loadu_ps(&m[12]);
    
    _MM_TRANSPOSE4_PS(row1, row2, row3, row4);
    
    _mm_storeu_ps(&m[0], row1);
    _mm_storeu_ps(&m[4], row2);
    _mm_storeu_ps(&m[8], row3);
    _mm_storeu_ps(&m[12], row4);
#else
    float tmp;
    // Swap elements across diagonal
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            tmp = m[i*4 + j];
            m[i*4 + j] = m[j*4 + i];
            m[j*4 + i] = tmp;
        }
    }
#endif
}

float SimdUtils::DeterminantMatrix4x4(const float* m) {
    // Using Laplace expansion along first row
    float cofactor0 = m[0] * (
        m[5] * (m[10]*m[15] - m[11]*m[14]) -
        m[6] * (m[9]*m[15] - m[11]*m[13]) +
        m[7] * (m[9]*m[14] - m[10]*m[13])
    );
    float cofactor1 = -m[1] * (
        m[4] * (m[10]*m[15] - m[11]*m[14]) -
        m[6] * (m[8]*m[15] - m[11]*m[12]) +
        m[7] * (m[8]*m[14] - m[10]*m[12])
    );
    float cofactor2 = m[2] * (
        m[4] * (m[9]*m[15] - m[11]*m[13]) -
        m[5] * (m[8]*m[15] - m[11]*m[12]) +
        m[7] * (m[8]*m[13] - m[9]*m[12])
    );
    float cofactor3 = -m[3] * (
        m[4] * (m[9]*m[14] - m[10]*m[13]) -
        m[5] * (m[8]*m[14] - m[10]*m[12]) +
        m[6] * (m[8]*m[13] - m[9]*m[12])
    );
    return cofactor0 + cofactor1 + cofactor2 + cofactor3;
}

namespace {
// Helper function to check if a matrix is purely a translation matrix
bool IsTranslationMatrix(const float* m) {
    return m[0] == 1.0f && m[1] == 0.0f && m[2] == 0.0f &&
           m[4] == 0.0f && m[5] == 1.0f && m[6] == 0.0f &&
           m[8] == 0.0f && m[9] == 0.0f && m[10] == 1.0f &&
           m[15] == 1.0f;
}

// Helper function to check if a matrix is purely a scale matrix
bool IsScaleMatrix(const float* m) {
    return m[1] == 0.0f && m[2] == 0.0f && m[3] == 0.0f &&
           m[4] == 0.0f && m[6] == 0.0f && m[7] == 0.0f &&
           m[8] == 0.0f && m[9] == 0.0f && m[11] == 0.0f &&
           m[12] == 0.0f && m[13] == 0.0f && m[14] == 0.0f && m[15] == 1.0f;
}

// Helper function to check if a matrix is orthonormal rotation
bool IsRotationMatrix(const float* m) {
    // Check orthogonality of first three rows/columns
    float dot01 = m[0]*m[4] + m[1]*m[5] + m[2]*m[6];
    float dot02 = m[0]*m[8] + m[1]*m[9] + m[2]*m[10];
    float dot12 = m[4]*m[8] + m[5]*m[9] + m[6]*m[10];
    
    if (std::fabs(dot01) > 1e-6f || std::fabs(dot02) > 1e-6f || std::fabs(dot12) > 1e-6f)
        return false;

    // Check unit length
    float len0 = m[0]*m[0] + m[1]*m[1] + m[2]*m[2];
    float len1 = m[4]*m[4] + m[5]*m[5] + m[6]*m[6];
    float len2 = m[8]*m[8] + m[9]*m[9] + m[10]*m[10];
    
    if (std::fabs(len0 - 1.0f) > 1e-6f || std::fabs(len1 - 1.0f) > 1e-6f || std::fabs(len2 - 1.0f) > 1e-6f)
        return false;

    // Check last row/column is [0,0,0,1]
    return m[3] == 0.0f && m[7] == 0.0f && m[11] == 0.0f &&
           m[12] == 0.0f && m[13] == 0.0f && m[14] == 0.0f && m[15] == 1.0f;
}

// Helper function to invert a translation matrix
bool InvertTranslationMatrix(const float* m, float* result) {
    // Copy upper 3x3
    for (int i = 0; i < 11; ++i) {
        result[i] = m[i];
    }
    // Negate translation components
    result[3] = -m[3];
    result[7] = -m[7];
    result[11] = -m[11];
    // Copy last row
    result[12] = m[12];
    result[13] = m[13];
    result[14] = m[14];
    result[15] = m[15];
    return true;
}

// Helper function to invert a scale matrix
bool InvertScaleMatrix(const float* m, float* result) {
    float sx = m[0];
    float sy = m[5];
    float sz = m[10];
    
    if (std::fabs(sx) < 1e-12f || std::fabs(sy) < 1e-12f || std::fabs(sz) < 1e-12f)
        return false;

    // Initialize to zero
    for (int i = 0; i < 16; ++i)
        result[i] = 0.0f;

    // Reciprocal of scale factors
    result[0] = 1.0f / sx;
    result[5] = 1.0f / sy;
    result[10] = 1.0f / sz;
    result[15] = 1.0f;
    
    return true;
}

// Helper function to invert a rotation matrix (transpose of upper 3x3)
bool InvertRotationMatrix(const float* m, float* result) {
    // Transpose the 3x3 rotation part
    result[0] = m[0]; result[1] = m[4]; result[2] = m[8];
    result[4] = m[1]; result[5] = m[5]; result[6] = m[9];
    result[8] = m[2]; result[9] = m[6]; result[10] = m[10];
    
    // Set translation and bottom row
    result[3] = 0.0f;
    result[7] = 0.0f;
    result[11] = 0.0f;
    result[12] = 0.0f;
    result[13] = 0.0f;
    result[14] = 0.0f;
    result[15] = 1.0f;
    
    return true;
}
} // anonymous namespace

bool SimdUtils::InvertMatrix4x4(const float* m, float* result) {
    // Check for special case matrices first
    if (IsTranslationMatrix(m))
        return InvertTranslationMatrix(m, result);
    if (IsRotationMatrix(m))
        return InvertRotationMatrix(m, result);
    if (IsScaleMatrix(m))
        return InvertScaleMatrix(m, result);

    // General case using adjugate matrix and determinant
    float det = DeterminantMatrix4x4(m);
    if (std::fabs(det) < 1e-12f) return false;
    float invDet = 1.0f / det;

    // Calculate cofactor matrix
    // First row
    result[0] = invDet * (m[5]*(m[10]*m[15] - m[11]*m[14]) - m[6]*(m[9]*m[15] - m[11]*m[13]) + m[7]*(m[9]*m[14] - m[10]*m[13]));
    result[1] = -invDet * (m[1]*(m[10]*m[15] - m[11]*m[14]) - m[2]*(m[9]*m[15] - m[11]*m[13]) + m[3]*(m[9]*m[14] - m[10]*m[13]));
    result[2] = invDet * (m[1]*(m[6]*m[15] - m[7]*m[14]) - m[2]*(m[5]*m[15] - m[7]*m[13]) + m[3]*(m[5]*m[14] - m[6]*m[13]));
    result[3] = -invDet * (m[1]*(m[6]*m[11] - m[7]*m[10]) - m[2]*(m[5]*m[11] - m[7]*m[9]) + m[3]*(m[5]*m[10] - m[6]*m[9]));

    // Second row
    result[4] = -invDet * (m[4]*(m[10]*m[15] - m[11]*m[14]) - m[6]*(m[8]*m[15] - m[11]*m[12]) + m[7]*(m[8]*m[14] - m[10]*m[12]));
    result[5] = invDet * (m[0]*(m[10]*m[15] - m[11]*m[14]) - m[2]*(m[8]*m[15] - m[11]*m[12]) + m[3]*(m[8]*m[14] - m[10]*m[12]));
    result[6] = -invDet * (m[0]*(m[6]*m[15] - m[7]*m[14]) - m[2]*(m[4]*m[15] - m[7]*m[12]) + m[3]*(m[4]*m[14] - m[6]*m[12]));
    result[7] = invDet * (m[0]*(m[6]*m[11] - m[7]*m[10]) - m[2]*(m[4]*m[11] - m[7]*m[8]) + m[3]*(m[4]*m[10] - m[6]*m[8]));

    // Third row
    result[8] = invDet * (m[4]*(m[9]*m[15] - m[11]*m[13]) - m[5]*(m[8]*m[15] - m[11]*m[12]) + m[7]*(m[8]*m[13] - m[9]*m[12]));
    result[9] = -invDet * (m[0]*(m[9]*m[15] - m[11]*m[13]) - m[1]*(m[8]*m[15] - m[11]*m[12]) + m[3]*(m[8]*m[13] - m[9]*m[12]));
    result[10] = invDet * (m[0]*(m[5]*m[15] - m[7]*m[13]) - m[1]*(m[4]*m[15] - m[7]*m[12]) + m[3]*(m[4]*m[13] - m[5]*m[12]));
    result[11] = -invDet * (m[0]*(m[5]*m[11] - m[7]*m[9]) - m[1]*(m[4]*m[11] - m[7]*m[8]) + m[3]*(m[4]*m[9] - m[5]*m[8]));

    // Fourth row
    result[12] = -invDet * (m[4]*(m[9]*m[14] - m[10]*m[13]) - m[5]*(m[8]*m[14] - m[10]*m[12]) + m[6]*(m[8]*m[13] - m[9]*m[12]));
    result[13] = invDet * (m[0]*(m[9]*m[14] - m[10]*m[13]) - m[1]*(m[8]*m[14] - m[10]*m[12]) + m[2]*(m[8]*m[13] - m[9]*m[12]));
    result[14] = -invDet * (m[0]*(m[5]*m[14] - m[6]*m[13]) - m[1]*(m[4]*m[14] - m[6]*m[12]) + m[2]*(m[4]*m[13] - m[5]*m[12]));
    result[15] = invDet * (m[0]*(m[5]*m[10] - m[6]*m[9]) - m[1]*(m[4]*m[10] - m[6]*m[8]) + m[2]*(m[4]*m[9] - m[5]*m[8]));

    return true;
}

} // namespace math
} // namespace pynovage
