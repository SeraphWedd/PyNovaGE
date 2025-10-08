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
    
    // Check reasonable limits for control points
    if (points.size() > 128) {
        throw std::invalid_argument("Bezier curve with more than 128 control points may cause performance issues");
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
    const int n = getDegree();
    const float u = 1.0f - t;

    // For small degrees, use scalar path
    if (n < 4) {
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
        return;
    }

    // Compute powers in groups of 4 using SIMD
    alignas(32) float tPowers[32];
    alignas(32) float uPowers[32];
    tPowers[0] = 1.0f;
    uPowers[0] = 1.0f;

    // Create vectors of t and u
    alignas(32) float ts[4] = {t, t, t, t};
    alignas(32) float us[4] = {u, u, u, u};

    // Compute first 4 powers efficiently
    for (int i = 1; i <= 3; ++i) {
        tPowers[i] = tPowers[i-1] * t;
        uPowers[i] = uPowers[i-1] * u;
    }

    // Process remaining powers in groups of 4
    for (int i = 4; i <= n; i += 4) {
        const int count = std::min(4, n - i + 1);
        alignas(32) float prevT[4], prevU[4];

        // Load previous powers
        for (int j = 0; j < count; ++j) {
            prevT[j] = tPowers[i-4+j];
            prevU[j] = uPowers[i-4+j];
        }

        // Compute next 4 powers using SIMD
        alignas(32) float nextT[4], nextU[4];
        SimdUtils::Multiply4f(prevT, ts, nextT);
        SimdUtils::Multiply4f(prevU, us, nextU);

        // Store results
        for (int j = 0; j < count; ++j) {
            tPowers[i+j] = nextT[j];
            uPowers[i+j] = nextU[j];
        }
    }

    // Compute basis values in groups of 4
    for (int i = 0; i <= n; i += 4) {
        const int count = std::min(4, n - i + 1);
        alignas(32) float coeffs[4], t_vals[4], u_vals[4], result[4];

        // Load values
        for (int j = 0; j < count; ++j) {
            coeffs[j] = static_cast<float>(binomialCoeffs_[i+j]);
            t_vals[j] = tPowers[i+j];
            u_vals[j] = uPowers[n-(i+j)];
        }

        // Compute basis values using SIMD
        alignas(32) float temp[4];
        SimdUtils::Multiply4f(coeffs, t_vals, temp);
        SimdUtils::Multiply4f(temp, u_vals, result);

        // Store results
        for (int j = 0; j < count; ++j) {
            basis[i+j] = result[j];
        }
    }
}

Vector3 Bezier::evaluateDeCasteljau(float t) const {
    const size_t n = controlPoints_.size();
    
    // For small curves, use stack allocation
    if (n <= 16) {
        Vector3 points[16];  // Small buffer for common case
        std::copy(controlPoints_.begin(), controlPoints_.end(), points);
        
        for (size_t r = 1; r < n; ++r) {
            for (size_t i = 0; i < n - r; ++i) {
                points[i] = points[i] * (1.0f - t) + points[i + 1] * t;
            }
        }
        
        return points[0];
    }
    
    // For large curves, use heap allocation
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

    // Safety: our fixed-size basis buffer only supports up to degree 31.
    // For higher degrees, fall back to the stable De Casteljau algorithm.
    if (n >= 32) {
        return evaluateDeCasteljau(t);
    }

    alignas(32) float basis[32];  // Space for basis functions up to degree 31
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
    // Check parameter count to avoid excessive memory usage
    if (parameters.size() > 10000) {
        throw std::invalid_argument("Too many evaluation points requested. Maximum is 10000.");
    }
    
    std::vector<Vector3> results;
    try {
        results.reserve(parameters.size());
        
        // For small curves or few evaluations, use scalar path
        if (controlPoints_.size() <= 4 || parameters.size() < 8) {
        for (float t : parameters) {
            results.push_back(evaluate(t));
        }
    }
    
    return results;
    } catch (const std::bad_alloc& e) {
        throw std::runtime_error("Failed to allocate memory for curve evaluation: " + std::string(e.what()));
    }
    // Use SIMD for batch evaluation if curve degree is within limits
    if (useSimd_ && getDegree() < 32) {
        const size_t batchSize = 8;  // Process 8 points at a time
        alignas(32) float basis[32];  // Safe: we checked degree < 32
        
        for (size_t i = 0; i < parameters.size(); i += batchSize) {
            const size_t count = std::min(batchSize, parameters.size() - i);
            
            for (size_t j = 0; j < count; ++j) {
                results.push_back(evaluateSIMD(parameters[i + j]));
            }
        }
    } else {
        // Fall back to scalar path for high-degree curves or when SIMD is unavailable
        for (float t : parameters) {
            results.push_back(evaluateDeCasteljau(t));
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
    
    // Check if elevation would exceed reasonable limits
    if (n >= 127) {
        return false;  // Cannot elevate beyond degree 127
    }
    
    try {
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
    } catch (const std::bad_alloc& e) {
        // Memory allocation failed
        throw std::runtime_error("Failed to allocate memory for degree elevation: " + std::string(e.what()));
    } catch (const std::exception& e) {
        // Other errors
        throw std::runtime_error("Error during degree elevation: " + std::string(e.what()));
    }
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
    
    // For small curves (degree <= 15), use stack allocation
    if (n <= 16) {
        Vector3 left[16], right[16], a[16];
        
        // Copy control points to working buffer
        std::copy(controlPoints_.begin(), controlPoints_.end(), a);
        
        for (size_t i = 0; i < n; ++i) {
            // Set endpoints from current polygon
            left[i] = a[0];
            right[n - 1 - i] = a[n - 1 - i];
            
            // Compute next level of De Casteljau
            for (size_t j = 0; j < n - 1 - i; ++j) {
                a[j] = a[j] * (1.0f - t) + a[j + 1] * t;
            }
        }
        
        return std::make_pair(
            Bezier(std::vector<Vector3>(left, left + n)),
            Bezier(std::vector<Vector3>(right, right + n)));
    }
    
    // For large curves, use heap allocation
    std::vector<Vector3> left(n);
    std::vector<Vector3> right(n);
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