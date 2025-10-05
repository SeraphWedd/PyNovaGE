#include "simd_utils.hpp"
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
void SimdUtils::MultiplyMatrix3x3(const float* a, const float* b, float* result) {
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

void SimdUtils::MultiplyMatrix3x3Vec3(const float* m, const float* v, float* result) {
    result[0] = m[0]*v[0] + m[1]*v[1] + m[2]*v[2];
    result[1] = m[3]*v[0] + m[4]*v[1] + m[5]*v[2];
    result[2] = m[6]*v[0] + m[7]*v[1] + m[8]*v[2];
}

void SimdUtils::TransposeMatrix3x3(float* m) {
    // swap m[1] <-> m[3], m[2] <-> m[6], m[5] <-> m[7]
    float tmp;
    tmp = m[1]; m[1] = m[3]; m[3] = tmp;
    tmp = m[2]; m[2] = m[6]; m[6] = tmp;
    tmp = m[5]; m[5] = m[7]; m[7] = tmp;
}

float SimdUtils::DeterminantMatrix3x3(const float* m) {
    // Using rule of Sarrus
    float det =
        m[0]*(m[4]*m[8] - m[5]*m[7]) -
        m[1]*(m[3]*m[8] - m[5]*m[6]) +
        m[2]*(m[3]*m[7] - m[4]*m[6]);
    return det;
}

bool SimdUtils::InvertMatrix3x3(const float* m, float* result) {
    float det = DeterminantMatrix3x3(m);
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
#if PYNOVAGE_MATH_HAS_SSE
    __m128 row1 = _mm_loadu_ps(&a[0]);
    __m128 row2 = _mm_loadu_ps(&a[4]);
    __m128 row3 = _mm_loadu_ps(&a[8]);
    __m128 row4 = _mm_loadu_ps(&a[12]);
    
    for(int i = 0; i < 4; i++) {
        // Broadcast each element of the column of b
        __m128 bCol = _mm_set_ps(b[i+12], b[i+8], b[i+4], b[i]);
        
        // Multiply and add
        __m128 r1 = _mm_mul_ps(row1, bCol);
        __m128 r2 = _mm_mul_ps(row2, bCol);
        __m128 r3 = _mm_mul_ps(row3, bCol);
        __m128 r4 = _mm_mul_ps(row4, bCol);
        
        // Horizontal sum
        __m128 sum12 = _mm_hadd_ps(r1, r2);
        __m128 sum34 = _mm_hadd_ps(r3, r4);
        __m128 sum = _mm_hadd_ps(sum12, sum34);
        
        // Store the results
        _mm_storeu_ps(&result[i*4], sum);
    }
#else
    // Naive implementation
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            float sum = 0;
            for(int k = 0; k < 4; k++) {
                sum += a[i*4 + k] * b[k*4 + j];
            }
            result[i*4 + j] = sum;
        }
    }
#endif
}

void SimdUtils::MultiplyMatrix4x4Vec4(const float* m, const float* v, float* result) {
#if PYNOVAGE_MATH_HAS_SSE
    __m128 vec = _mm_loadu_ps(v);
    __m128 row1 = _mm_loadu_ps(&m[0]);
    __m128 row2 = _mm_loadu_ps(&m[4]);
    __m128 row3 = _mm_loadu_ps(&m[8]);
    __m128 row4 = _mm_loadu_ps(&m[12]);
    
    __m128 mul1 = _mm_mul_ps(row1, vec);
    __m128 mul2 = _mm_mul_ps(row2, vec);
    __m128 mul3 = _mm_mul_ps(row3, vec);
    __m128 mul4 = _mm_mul_ps(row4, vec);
    
    __m128 sum12 = _mm_hadd_ps(mul1, mul2);
    __m128 sum34 = _mm_hadd_ps(mul3, mul4);
    __m128 sum = _mm_hadd_ps(sum12, sum34);
    
    _mm_storeu_ps(result, sum);
#else
    for(int i = 0; i < 4; i++) {
        result[i] = m[i*4]*v[0] + m[i*4+1]*v[1] + m[i*4+2]*v[2] + m[i*4+3]*v[3];
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

bool SimdUtils::InvertMatrix4x4(const float* m, float* result) {
    float det = DeterminantMatrix4x4(m);
    if (std::fabs(det) < 1e-12f) return false;
    float invDet = 1.0f / det;
    
    // Calculate cofactor matrix
    // First row
    result[0] = (
        m[5] * (m[10]*m[15] - m[11]*m[14]) -
        m[6] * (m[9]*m[15] - m[11]*m[13]) +
        m[7] * (m[9]*m[14] - m[10]*m[13])
    ) * invDet;
    result[1] = -(
        m[4] * (m[10]*m[15] - m[11]*m[14]) -
        m[6] * (m[8]*m[15] - m[11]*m[12]) +
        m[7] * (m[8]*m[14] - m[10]*m[12])
    ) * invDet;
    result[2] = (
        m[4] * (m[9]*m[15] - m[11]*m[13]) -
        m[5] * (m[8]*m[15] - m[11]*m[12]) +
        m[7] * (m[8]*m[13] - m[9]*m[12])
    ) * invDet;
    result[3] = -(
        m[4] * (m[9]*m[14] - m[10]*m[13]) -
        m[5] * (m[8]*m[14] - m[10]*m[12]) +
        m[6] * (m[8]*m[13] - m[9]*m[12])
    ) * invDet;

    // Second row
    result[4] = -(
        m[1] * (m[10]*m[15] - m[11]*m[14]) -
        m[2] * (m[9]*m[15] - m[11]*m[13]) +
        m[3] * (m[9]*m[14] - m[10]*m[13])
    ) * invDet;
    result[5] = (
        m[0] * (m[10]*m[15] - m[11]*m[14]) -
        m[2] * (m[8]*m[15] - m[11]*m[12]) +
        m[3] * (m[8]*m[14] - m[10]*m[12])
    ) * invDet;
    result[6] = -(
        m[0] * (m[9]*m[15] - m[11]*m[13]) -
        m[1] * (m[8]*m[15] - m[11]*m[12]) +
        m[3] * (m[8]*m[13] - m[9]*m[12])
    ) * invDet;
    result[7] = (
        m[0] * (m[9]*m[14] - m[10]*m[13]) -
        m[1] * (m[8]*m[14] - m[10]*m[12]) +
        m[2] * (m[8]*m[13] - m[9]*m[12])
    ) * invDet;

    // Third row
    result[8] = (
        m[1] * (m[6]*m[15] - m[7]*m[14]) -
        m[2] * (m[5]*m[15] - m[7]*m[13]) +
        m[3] * (m[5]*m[14] - m[6]*m[13])
    ) * invDet;
    result[9] = -(
        m[0] * (m[6]*m[15] - m[7]*m[14]) -
        m[2] * (m[4]*m[15] - m[7]*m[12]) +
        m[3] * (m[4]*m[14] - m[6]*m[12])
    ) * invDet;
    result[10] = (
        m[0] * (m[5]*m[15] - m[7]*m[13]) -
        m[1] * (m[4]*m[15] - m[7]*m[12]) +
        m[3] * (m[4]*m[13] - m[5]*m[12])
    ) * invDet;
    result[11] = -(
        m[0] * (m[5]*m[14] - m[6]*m[13]) -
        m[1] * (m[4]*m[14] - m[6]*m[12]) +
        m[2] * (m[4]*m[13] - m[5]*m[12])
    ) * invDet;

    // Fourth row
    result[12] = -(
        m[1] * (m[6]*m[11] - m[7]*m[10]) -
        m[2] * (m[5]*m[11] - m[7]*m[9]) +
        m[3] * (m[5]*m[10] - m[6]*m[9])
    ) * invDet;
    result[13] = (
        m[0] * (m[6]*m[11] - m[7]*m[10]) -
        m[2] * (m[4]*m[11] - m[7]*m[8]) +
        m[3] * (m[4]*m[10] - m[6]*m[8])
    ) * invDet;
    result[14] = -(
        m[0] * (m[5]*m[11] - m[7]*m[9]) -
        m[1] * (m[4]*m[11] - m[7]*m[8]) +
        m[3] * (m[4]*m[9] - m[5]*m[8])
    ) * invDet;
    result[15] = (
        m[0] * (m[5]*m[10] - m[6]*m[9]) -
        m[1] * (m[4]*m[10] - m[6]*m[8]) +
        m[2] * (m[4]*m[9] - m[5]*m[8])
    ) * invDet;

    return true;
}

} // namespace math
} // namespace pynovage
