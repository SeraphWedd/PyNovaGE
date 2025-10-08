#include "../../include/geometry/bspline.hpp"
#include "../../include/simd_utils.hpp"
#include <cassert>
#include <cmath>

namespace pynovage {
namespace math {

BSpline::BSpline(const std::vector<Vector3>& controlPoints, int degree,
                 const std::vector<float>& knots)
    : controlPoints_(controlPoints), degree_(degree) {
    
    // Validate inputs
    if (controlPoints.empty()) {
        throw std::invalid_argument("Control points vector cannot be empty");
    }
    if (degree < 1) {
        throw std::invalid_argument("Degree must be at least 1");
    }
    if (controlPoints.size() <= static_cast<size_t>(degree)) {
        throw std::invalid_argument("Number of control points must be greater than degree");
    }

    // Use provided knots or create uniform knot vector
    if (knots.empty()) {
        createUniformKnots();
    } else {
        knots_ = knots;
        if (!validateKnots()) {
            throw std::invalid_argument("Invalid knot vector");
        }
    }
}

void BSpline::createUniformKnots() {
    // For a B-spline of degree p with n+1 control points,
    // we need m+1 knots where m = n+p+1 (so n+p+2 total knots)
    size_t n = controlPoints_.size() - 1;  // number of spans between control points
    size_t numKnots = n + degree_ + 2;     // total number of knots needed
    knots_.resize(numKnots);

    // Create a clamped knot vector (repeated knots at ends)
    // This ensures the curve passes through endpoints
    for (size_t i = 0; i <= degree_; ++i) {
        knots_[i] = 0.0f;                     // p+1 zeros at start
        knots_[numKnots-1-i] = 1.0f;          // p+1 ones at end
    }
    
    // Fill interior knots with uniform spacing
    size_t numInterior = numKnots - 2*(degree_ + 1);
    if (numInterior > 0) {
        float step = 1.0f / (numInterior + 1);
        for (size_t i = 0; i < numInterior; ++i) {
            knots_[degree_ + 1 + i] = (i + 1) * step;
        }
    }
}

bool BSpline::validateKnots() const {
    size_t n = controlPoints_.size() - 1;
    size_t requiredKnots = n + degree_ + 2;

    // Check number of knots
    if (knots_.size() != requiredKnots) {
        return false;
    }

    // Check knot vector is non-decreasing
    for (size_t i = 1; i < knots_.size(); ++i) {
        if (knots_[i] < knots_[i-1]) {
            return false;
        }
    }

    return true;
}

size_t BSpline::getMultiplicity(float u, float tolerance) const {
    size_t mult = 0;
    for (float knot : knots_) {
        if (std::abs(knot - u) <= tolerance) {
            ++mult;
        }
    }
    return mult;
}

std::vector<float> BSpline::getUniqueKnots(float tolerance) const {
    std::vector<float> uniqueKnots;
    for (float knot : knots_) {
        bool found = false;
        for (float unique : uniqueKnots) {
            if (std::abs(knot - unique) <= tolerance) {
                found = true;
                break;
            }
        }
        if (!found) {
            uniqueKnots.push_back(knot);
        }
    }
    std::sort(uniqueKnots.begin(), uniqueKnots.end());
    return uniqueKnots;
}

bool BSpline::insertKnotExact(float u, float tolerance) {
    // Find span containing u
    int k = findSpan(u);
    if (k < degree_ || k > static_cast<int>(controlPoints_.size())) {
        return false;
    }

    // Create new control points array with exact arithmetic
    std::vector<Vector3> newPoints(controlPoints_.size() + 1);
    
    // Copy points before affected region
    for (int i = 0; i <= k - degree_; ++i) {
        newPoints[i] = controlPoints_[i];
    }
    
    // Compute new points using exact ratios
    for (int i = k - degree_ + 1; i <= k; ++i) {
        float alpha = (u - knots_[i]) / (knots_[i + degree_] - knots_[i]);
        newPoints[i] = controlPoints_[i - 1] * (1.0f - alpha) +
                      controlPoints_[i] * alpha;
    }
    
    // Copy remaining points
    for (size_t i = k + 1; i < newPoints.size(); ++i) {
        newPoints[i] = controlPoints_[i - 1];
    }

    // Insert new knot
    auto it = std::lower_bound(knots_.begin(), knots_.end(), u);
    knots_.insert(it, u);
    controlPoints_ = std::move(newPoints);

    return true;
}

bool BSpline::toBezierForm(size_t startKnot, size_t endKnot) {
    if (startKnot >= endKnot || endKnot >= knots_.size()) {
        return false;
    }

    // For each unique internal knot in [startKnot, endKnot)
    auto uniqueKnots = getUniqueKnots();
    for (float knot : uniqueKnots) {
        if (knot > knots_[startKnot] && knot < knots_[endKnot]) {
            // Insert until multiplicity equals degree
            size_t current = getMultiplicity(knot);
            while (current < static_cast<size_t>(degree_)) {
                if (!insertKnotExact(knot)) {
                    return false;
                }
                ++current;
            }
        }
    }
    return true;
}

std::vector<Vector3> BSpline::elevateBezierSegment(
    const std::vector<Vector3>& points) const {
    if (points.size() != static_cast<size_t>(degree_) + 1) {
        throw std::invalid_argument("Invalid number of control points");
    }

    std::vector<Vector3> elevated(points.size() + 1);
    elevated[0] = points[0];  // First point unchanged
    elevated[elevated.size()-1] = points[points.size()-1];  // Last point unchanged

    // Compute internal points using degree elevation formula
    // Correct weights: Pi' = (i/(p+1))Pi-1 + (1 - i/(p+1))Pi
    for (size_t i = 1; i <= static_cast<size_t>(degree_); ++i) {
        float alpha = static_cast<float>(i) / static_cast<float>(degree_ + 1);
        elevated[i] = points[i-1] * alpha +
                     points[i] * (1.0f - alpha);
    }

    return elevated;
}

int BSpline::findSpan(float t) const {
    size_t n = controlPoints_.size() - 1;
    
    // Handle boundary cases
    if (t >= knots_[n + 1]) return static_cast<int>(n);
    if (t <= knots_[degree_]) return degree_;

    // Binary search for the correct span
    int low = degree_;
    int high = static_cast<int>(n + 1);
    int mid = (low + high) / 2;

    while (t < knots_[mid] || t >= knots_[mid + 1]) {
        if (t < knots_[mid]) {
            high = mid;
        } else {
            low = mid;
        }
        mid = (low + high) / 2;
    }

    return mid;
}

void BSpline::computeBasisFunctions(int span, float t, std::vector<float>& basisFuncs) const {
    basisFuncs.resize(degree_ + 1);
    std::vector<float> left(degree_ + 1);
    std::vector<float> right(degree_ + 1);

    // Initialize the zeroth degree basis function
    basisFuncs[0] = 1.0f;

    // Compute basis functions of increasing degree
    for (int j = 1; j <= degree_; ++j) {
        left[j] = t - knots_[span + 1 - j];
        right[j] = knots_[span + j] - t;
        float saved = 0.0f;

        // Compute basis functions of degree j
        for (int r = 0; r < j; ++r) {
            float denom = (right[r + 1] + left[j - r]);
            float temp = 0.0f;
            if (std::fabs(denom) > 1e-12f) {
                temp = basisFuncs[r] / denom;
            } else {
                temp = 0.0f; // occurs at knots with multiplicity; contributes nothing
            }
            basisFuncs[r] = saved + right[r + 1] * temp;
            saved = left[j - r] * temp;
        }
        basisFuncs[j] = saved;
    }
}

Vector3 BSpline::evaluate(float t) const {
    // Clamp t to valid range
    t = std::max(knots_[degree_], std::min(t, knots_[knots_.size() - degree_ - 1]));
    
    // Find the knot span and compute basis functions
    int span = findSpan(t);
    std::vector<float> basisFuncs;
    computeBasisFunctions(span, t, basisFuncs);

    // Compute point using SIMD-optimized operations
    Vector3 result;
    float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    
    for (int i = 0; i <= degree_; ++i) {
        weights[0] = basisFuncs[i];
        weights[1] = basisFuncs[i];
        weights[2] = basisFuncs[i];
        weights[3] = 0.0f;  // w component padding for SIMD
        
        Vector3 temp;
        SimdUtils::Multiply3f(&controlPoints_[span - degree_ + i].x, weights, &temp.x);
        result += temp;
    }

    return result;
}

std::vector<Vector3> BSpline::evaluateMultiple(const std::vector<float>& parameters) const {
    std::vector<Vector3> points;
    points.reserve(parameters.size());
    
    for (float t : parameters) {
        points.push_back(evaluate(t));
    }
    
    return points;
}

bool BSpline::insertKnot(float t) {
    // Find span containing t
    int k = findSpan(t);
    if (k < degree_ || k > static_cast<int>(controlPoints_.size())) {
        return false;
    }

    // Create new control points array
    std::vector<Vector3> newPoints(controlPoints_.size() + 1);
    
    // Compute alpha values
    std::vector<float> alpha(degree_ + 1);
    for (int j = 0; j <= degree_; ++j) {
        alpha[j] = (t - knots_[k - degree_ + j]) / 
                  (knots_[k + 1 + j] - knots_[k - degree_ + j]);
    }

    // Update control points
    for (int i = 0; i <= k - degree_; ++i) {
        newPoints[i] = controlPoints_[i];
    }
    
    for (int i = k - degree_ + 1; i <= k; ++i) {
        newPoints[i] = controlPoints_[i - 1] * (1.0f - alpha[i - (k - degree_)]) +
                      controlPoints_[i] * alpha[i - (k - degree_)];
    }
    
    for (size_t i = k + 1; i < controlPoints_.size() + 1; ++i) {
        newPoints[i] = controlPoints_[i - 1];
    }

    // Insert new knot
    knots_.insert(knots_.begin() + k + 1, t);
    controlPoints_ = std::move(newPoints);

    return true;
}

bool BSpline::elevateDegree() {
    if (degree_ < 1) {
        return false;
    }

    // Get unique internal knots
    auto uniqueKnots = getUniqueKnots();
    if (uniqueKnots.size() < 2) {
        return false;
    }

    // Step 1: Convert to piecewise Bézier by inserting knots
    // until each internal knot has multiplicity p
    for (size_t i = 1; i < uniqueKnots.size() - 1; ++i) {
        size_t mult = getMultiplicity(uniqueKnots[i]);
        while (mult < static_cast<size_t>(degree_)) {
            if (!insertKnotExact(uniqueKnots[i])) {
                return false;
            }
            ++mult;
        }
    }

    // Get fresh list of unique knots after insertion
    uniqueKnots = getUniqueKnots();
    size_t numSegments = uniqueKnots.size() - 1;

    // Step 2: Extract Bézier segments and elevate each one
    std::vector<Vector3> elevatedPoints;
    std::vector<float> elevatedKnots;
    
    // Reserve space for result (approximately)
    elevatedPoints.reserve(controlPoints_.size() + numSegments);
    elevatedKnots.reserve(knots_.size() + 2*numSegments);

    // Start knots with multiplicity p+2
    for (int i = 0; i <= degree_ + 1; ++i) {
        elevatedKnots.push_back(0.0f);
    }

    // Process each segment
    size_t segStart = 0;
    for (size_t i = 0; i < numSegments; ++i) {
        // Extract segment control points
        std::vector<Vector3> segPoints;
        segPoints.reserve(degree_ + 1);
        
        for (size_t j = 0; j <= static_cast<size_t>(degree_); ++j) {
            segPoints.push_back(controlPoints_[segStart + j]);
        }

        // Elevate segment
        auto elevatedSeg = elevateBezierSegment(segPoints);

        // Add elevated points, avoiding duplicates at joins
        if (i == 0) {
            elevatedPoints.insert(elevatedPoints.end(),
                                elevatedSeg.begin(),
                                elevatedSeg.end());
        } else {
            elevatedPoints.insert(elevatedPoints.end(),
                                elevatedSeg.begin() + 1,
                                elevatedSeg.end());
        }

        // Add internal knot with multiplicity p+1
        if (i < numSegments - 1) {
            for (int j = 0; j <= degree_; ++j) {
                elevatedKnots.push_back(uniqueKnots[i + 1]);
            }
        }

        segStart += degree_;
    }

    // End knots with multiplicity p+2
    for (int i = 0; i <= degree_ + 1; ++i) {
        elevatedKnots.push_back(1.0f);
    }

    // Update curve data
    ++degree_;
    controlPoints_ = std::move(elevatedPoints);
    knots_ = std::move(elevatedKnots);

    return true;
}

BSpline BSpline::derivative() const {
    if (degree_ < 1) {
        throw std::runtime_error("Cannot compute derivative of degree 0 B-spline");
    }

    // Compute new control points for derivative
    std::vector<Vector3> derivPoints(controlPoints_.size() - 1);
    for (size_t i = 0; i < derivPoints.size(); ++i) {
        float factor = static_cast<float>(degree_) / (knots_[i + degree_ + 1] - knots_[i + 1]);
        derivPoints[i] = (controlPoints_[i + 1] - controlPoints_[i]) * factor;
    }

    // Create new knot vector for derivative
    std::vector<float> derivKnots(knots_.size() - 2);
    std::copy(knots_.begin() + 1, knots_.end() - 1, derivKnots.begin());

    // Create new B-spline of degree - 1
    return BSpline(derivPoints, degree_ - 1, derivKnots);
}

} // namespace math
} // namespace pynovage