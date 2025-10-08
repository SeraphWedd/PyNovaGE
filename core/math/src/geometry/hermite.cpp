#include "../../include/geometry/hermite.hpp"
#include <stdexcept>

namespace pynovage {
namespace math {

namespace {
    // Helper to check if current platform supports SIMD
    bool checkSimdSupport() {
        return SimdUtils::HasAVX() || SimdUtils::HasSSE2() || SimdUtils::HasSSE();
    }

    // Constants for basis computation
    constexpr float h00(float t) { return 2.0f*t*t*t - 3.0f*t*t + 1.0f; }  // Position basis 1
    constexpr float h10(float t) { return t*t*t - 2.0f*t*t + t; }          // Tangent basis 1
    constexpr float h01(float t) { return -2.0f*t*t*t + 3.0f*t*t; }        // Position basis 2
    constexpr float h11(float t) { return t*t*t - t*t; }                   // Tangent basis 2
} // anonymous namespace

Hermite::Hermite(const Vector3& p0, const Vector3& p1,
                 const Vector3& t0, const Vector3& t1,
                 float tension)
    : p0_(p0)
    , p1_(p1)
    , t0_(t0)
    , t1_(t1)
    , useSimd_(checkSimdSupport())
{
    validateTension(tension);
    tension_ = tension;
}

void Hermite::validateTension(float tension) {
    if (tension <= 0.0f) {
        throw std::invalid_argument("Hermite curve tension must be positive");
    }
}

void Hermite::setTension(float tension) {
    validateTension(tension);
    tension_ = tension;
}

void Hermite::computeBasis(float t, float* basis) const {
    // Compute standard Hermite basis functions
    basis[0] = h00(t);            // Position 1 coefficient
    basis[1] = h10(t) * tension_; // Tangent 1 coefficient
    basis[2] = h01(t);            // Position 2 coefficient
    basis[3] = h11(t) * tension_; // Tangent 2 coefficient
}

void Hermite::computeBasisSIMD(float t, float* basis) const {
    // Precompute powers of t for vectorized operations
    alignas(32) float t_powers[4] = {t*t*t, t*t, t, 1.0f};
    alignas(32) float coeffs[4][4] = {
        {2.0f, -3.0f, 0.0f, 1.0f},    // h00 coefficients
        {1.0f, -2.0f, 1.0f, 0.0f},    // h10 coefficients
        {-2.0f, 3.0f, 0.0f, 0.0f},    // h01 coefficients
        {1.0f, -1.0f, 0.0f, 0.0f}     // h11 coefficients
    };

    // Use SIMD operations for basis computation
    for (int i = 0; i < 4; ++i) {
        float result;
        SimdUtils::DotProduct4f(t_powers, coeffs[i], &result);
        basis[i] = result * (i % 2 == 1 ? tension_ : 1.0f);
    }
}

Vector3 Hermite::evaluate(float t) const {
    if (t <= 0.0f) return p0_;
    if (t >= 1.0f) return p1_;

    alignas(32) float basis[4];
    if (useSimd_) {
        computeBasisSIMD(t, basis);
    } else {
        computeBasis(t, basis);
    }

    return p0_ * basis[0] + t0_ * basis[1] + p1_ * basis[2] + t1_ * basis[3];
}

std::vector<Vector3> Hermite::evaluateMultiple(const std::vector<float>& parameters) const {
    if (parameters.size() > 10000) {
        throw std::invalid_argument("Too many evaluation points requested. Maximum is 10000.");
    }

    std::vector<Vector3> results;
    try {
        results.reserve(parameters.size());

        // SIMD batch processing
        if (useSimd_ && parameters.size() >= 4) {
            const size_t batchSize = 4;  // Process 4 points at a time
            alignas(32) float basis[4];

            for (size_t i = 0; i < parameters.size(); i += batchSize) {
                const size_t count = std::min(batchSize, parameters.size() - i);
                
                for (size_t j = 0; j < count; ++j) {
                    const float t = parameters[i + j];
                    if (t <= 0.0f) {
                        results.push_back(p0_);
                    } else if (t >= 1.0f) {
                        results.push_back(p1_);
                    } else {
                        computeBasisSIMD(t, basis);
                        results.push_back(p0_ * basis[0] + t0_ * basis[1] + 
                                        p1_ * basis[2] + t1_ * basis[3]);
                    }
                }
            }
        } else {
            // Scalar path for small batches
            alignas(32) float basis[4];
            for (float t : parameters) {
                if (t <= 0.0f) {
                    results.push_back(p0_);
                } else if (t >= 1.0f) {
                    results.push_back(p1_);
                } else {
                    computeBasis(t, basis);
                    results.push_back(p0_ * basis[0] + t0_ * basis[1] + 
                                    p1_ * basis[2] + t1_ * basis[3]);
                }
            }
        }

        return results;
    } catch (const std::bad_alloc& e) {
        throw std::runtime_error("Failed to allocate memory for curve evaluation: " + 
                               std::string(e.what()));
    }
}

Hermite Hermite::derivative() const {
    // For a cubic Hermite curve H(t), the derivative H'(t) is another Hermite curve
    // with different endpoints and tangents:
    // - New endpoints are the original tangents scaled by 3
    // - New tangents are computed from the original points and tangents
    Vector3 dp0 = t0_ * 3.0f;
    Vector3 dp1 = t1_ * 3.0f;
    Vector3 dt0 = (p1_ - p0_) * 6.0f - t0_ * 4.0f - t1_ * 2.0f;
    Vector3 dt1 = (p1_ - p0_) * -6.0f + t0_ * 2.0f + t1_ * 4.0f;

    return Hermite(dp0, dp1, dt0, dt1, tension_);
}

} // namespace math
} // namespace pynovage