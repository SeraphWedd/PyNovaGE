#ifndef PYNOVAGE_MATH_MATRIX2_HPP
#define PYNOVAGE_MATH_MATRIX2_HPP

#include "math/simd_utils.hpp"
#include "math/vector2.hpp"
#include "math/math_constants.hpp"
#include <cmath>

namespace pynovage {
namespace math {

class Matrix2 {
public:
    // Row-major storage: [ m00 m01 ; m10 m11 ]
    Matrix2() { setIdentity(); }
    Matrix2(float m00, float m01, float m10, float m11) {
        m[0]=m00; m[1]=m01; m[2]=m10; m[3]=m11;
    }

    static Matrix2 identity() { return Matrix2(1,0, 0,1); }
    static Matrix2 rotation(float radians) {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return Matrix2(c, -s, s, c);
    }
    static Matrix2 scale(float sx, float sy) { return Matrix2(sx,0, 0,sy); }

    void setIdentity() { m[0]=1; m[1]=0; m[2]=0; m[3]=1; }

    Matrix2 operator*(const Matrix2& other) const {
        Matrix2 r;
        SimdUtils::MultiplyMatrix2x2(m, other.m, r.m);
        return r;
    }

    Vector2 operator*(const Vector2& v) const {
        Vector2 r;
        SimdUtils::MultiplyMatrix2x2Vec2(m, &v.x, &r.x);
        return r;
    }

    float determinant() const { return SimdUtils::DeterminantMatrix2x2(m); }

    bool inverse(Matrix2& out) const { return SimdUtils::InvertMatrix2x2(m, out.m); }

    void transposeInPlace() { SimdUtils::TransposeMatrix2x2(m); }

    Matrix2 transposed() const {
        Matrix2 r(*this);
        r.transposeInPlace();
        return r;
    }

    float m[4];
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_MATRIX2_HPP