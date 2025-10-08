#include "../../include/geometry/bezier.hpp"
#include <stdexcept>

namespace pynovage {
namespace math {

namespace {
    // Helper to check if current platform supports SIMD
    bool checkSimdSupport() {
        // Prefer AVX if available, otherwise SSE2/SSE
        return SimdUtils::HasAVX() || SimdUtils::HasSSE2() || SimdUtils::HasSSE();
    }
} // anonymous namespace

Bezier::Bezier(const std::vector<Vector3>& controlPoints) 
    : controlPoints_(controlPoints)
    , useSimd_(checkSimdSupport())
{
    validateControlPoints(controlPoints);
    binomialCoeffs_ = computeBinomialCoefficients();
}

void Bezier::validateControlPoints(const std::vector<Vector3>& points) {
    if (points.size() < 2) {
        throw std::invalid_argument("Bezier curve requires at least 2 control points");
    }
}

std::vector<int> Bezier::computeBinomialCoefficients() const {
    const int n = getDegree();
    std::vector<int> coeffs(n + 1);
    
    coeffs[0] = 1;
    for (int i = 1; i <= n; ++i) {
        coeffs[i] = 1;
        for (int j = i - 1; j > 0; --j) {
            coeffs[j] += coeffs[j - 1];
        }
    }
    
    return coeffs;
}

std::vector<float> Bezier::computeBasis(float t) const {
    const int n = getDegree();
    std::vector<float> basis(n + 1);
    
    const float u = 1.0f - t;
    std::vector<float> tPowers(n + 1);
    std::vector<float> uPowers(n + 1);
    
    // Precompute powers
    tPowers[0] = 1.0f;
    uPowers[0] = 1.0f;
    for (int i = 1; i <= n; ++i) {
        tPowers[i] = tPowers[i-1] * t;
        uPowers[i] = uPowers[i-1] * u;
    }
    
    // Compute basis functions
    for (int i = 0; i <= n; ++i) {
        basis[i] = static_cast<float>(binomialCoeffs_[i]) * uPowers[n-i] * tPowers[i];
    }
    
    return basis;
}

void Bezier::computeBasisSIMD(float t, float* basis) const {
    // Fallback implementation using scalar math; keeping function to preserve API
    const int n = getDegree();
    const float u = 1.0f - t;

    // Compute powers
    std::vector<float> tPowers(n + 1);
    std::vector<float> uPowers(n + 1);
    tPowers[0] = 1.0f;
    uPowers[0] = 1.0f;
    for (int i = 1; i <= n; ++i) {
        tPowers[i] = tPowers[i-1] * t;
        uPowers[i] = uPowers[i-1] * u;
    }

    for (int i = 0; i <= n; ++i) {
        basis[i] = static_cast<float>(binomialCoeffs_[i]) * uPowers[n-i] * tPowers[i];
    }
}

Vector3 Bezier::evaluateDeCasteljau(float t) const {
    const size_t n = controlPoints_.size();
    std::vector<Vector3> points = controlPoints_;
    
    for (size_t r = 1; r < n; ++r) {
        for (size_t i = 0; i < n - r; ++i) {
            points[i] = points[i] * (1.0f - t) + points[i + 1] * t;
        }
    }
    
    return points[0];
}

Vector3 Bezier::evaluateSIMD(float t) const {
    const int n = getDegree();
    alignas(32) float basis[32];  // Enough space for basis functions up to degree 31

    computeBasisSIMD(t, basis);

    // Accumulate weighted sum (scalar fallback)
    Vector3 result(0.0f, 0.0f, 0.0f);
    const size_t count = controlPoints_.size();
    for (size_t i = 0; i < count; ++i) {
        result += controlPoints_[i] * basis[i];
    }

    return result;
}

Vector3 Bezier::evaluate(float t) const {
    if (t <= 0.0f) return controlPoints_.front();
    if (t >= 1.0f) return controlPoints_.back();
    
    // For small curves, De Casteljau is faster
    if (controlPoints_.size() <= 4) {
        return evaluateDeCasteljau(t);
    }
    
    return useSimd_ ? evaluateSIMD(t) : evaluateDeCasteljau(t);
}

std::vector<Vector3> Bezier::evaluateMultiple(const std::vector<float>& parameters) const {
    std::vector<Vector3> results;
    results.reserve(parameters.size());
    
    // For small curves or few evaluations, use scalar path
    if (controlPoints_.size() <= 4 || parameters.size() < 8) {
        for (float t : parameters) {
            results.push_back(evaluate(t));
        }
        return results;
    }
    
    // Use SIMD for batch evaluation
    if (useSimd_) {
        const size_t batchSize = 8;  // Process 8 points at a time
        alignas(32) float basis[32];
        
        for (size_t i = 0; i < parameters.size(); i += batchSize) {
            const size_t count = std::min(batchSize, parameters.size() - i);
            
            for (size_t j = 0; j < count; ++j) {
                results.push_back(evaluateSIMD(parameters[i + j]));
            }
        }
    } else {
        for (float t : parameters) {
            results.push_back(evaluate(t));
        }
    }
    
    return results;
}

Bezier Bezier::derivative() const {
    if (getDegree() < 1) {
        // Return a zero vector linear curve to satisfy constructor constraint
        return Bezier({Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)});
    }
    
    std::vector<Vector3> derivPoints;
    derivPoints.reserve(controlPoints_.size() - 1);
    
    const float n = static_cast<float>(getDegree());
    for (size_t i = 0; i < controlPoints_.size() - 1; ++i) {
        derivPoints.push_back(n * (controlPoints_[i + 1] - controlPoints_[i]));
    }
    
    if (derivPoints.size() == 1) {
        return Bezier({derivPoints[0], derivPoints[0]});
    }
    
    return Bezier(derivPoints);
}

bool Bezier::elevateDegree() {
    const int n = getDegree();
    std::vector<Vector3> newPoints(n + 2);
    
    for (int i = 0; i <= n + 1; ++i) {
        float t = static_cast<float>(i) / (n + 1);
        newPoints[i] = Vector3(0.0f, 0.0f, 0.0f);
        
        for (int j = std::max(0, i - 1); j <= std::min(n, i); ++j) {
            float coeff = binomialCoeffs_[j] * std::pow(1.0f - t, n - j) * std::pow(t, j);
            newPoints[i] += controlPoints_[j] * coeff;
        }
    }
    
    controlPoints_ = std::move(newPoints);
    binomialCoeffs_ = computeBinomialCoefficients();
    return true;
}

bool Bezier::reduceDegree(float maxError) {
    if (getDegree() <= 1) return false;
    
    // Compute reduced control points
    const int n = getDegree();
    std::vector<Vector3> reducedPoints(n);
    
    for (int i = 0; i < n; ++i) {
        float t = static_cast<float>(i) / (n - 1);
        reducedPoints[i] = evaluate(t);
    }
    
    // Check error
    float maxDiff = 0.0f;
    for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
        Vector3 origPoint = evaluate(t);
        
        // Create temporary curve with reduced points
        Bezier reducedCurve(reducedPoints);
        Vector3 reducedPoint = reducedCurve.evaluate(t);
        
        maxDiff = std::max(maxDiff, (origPoint - reducedPoint).length());
    }
    
    if (maxDiff <= maxError) {
        controlPoints_ = std::move(reducedPoints);
        binomialCoeffs_ = computeBinomialCoefficients();
        return true;
    }
    
    return false;
}

std::pair<Bezier, Bezier> Bezier::split(float t) const {
    const size_t n = controlPoints_.size();
    std::vector<Vector3> left(n);
    std::vector<Vector3> right(n);

    // Working copy of control points
    std::vector<Vector3> a = controlPoints_;

    for (size_t i = 0; i < n; ++i) {
        // Set endpoints from current polygon
        left[i] = a[0];
        right[n - 1 - i] = a[n - 1 - i];

        // Compute next level of De Casteljau
        for (size_t j = 0; j < n - 1 - i; ++j) {
            a[j] = a[j] * (1.0f - t) + a[j + 1] * t;
        }
    }

    return std::make_pair(Bezier(left), Bezier(right));
}

} // namespace math
} // namespace pynovage