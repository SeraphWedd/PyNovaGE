#include "../../include/geometry/catmull_rom.hpp"
#include <stdexcept>

namespace pynovage {
namespace math {

namespace {
    // Helper to check if current platform supports SIMD
    bool checkSimdSupport() {
        return SimdUtils::HasAVX() || SimdUtils::HasSSE2() || SimdUtils::HasSSE();
    }

    // Constants for Catmull-Rom basis matrix conversion
    constexpr float CR_TO_HERMITE[4][4] = {
        {0.0f,  1.0f,  0.0f,  0.0f},  // P1
        {-0.5f, 0.0f,  0.5f,  0.0f},  // T1 = (P2 - P0)/2
        {1.0f, -2.5f,  2.0f, -0.5f},  // P2
        {-0.5f, 1.5f, -1.5f,  0.5f}   // T2 = (P3 - P1)/2
    };
} // anonymous namespace

CatmullRom::CatmullRom(const std::vector<Vector3>& points,
                       Parameterization param,
                       float tension)
    : points_(points)
    , param_(param)
    , tension_(tension)
    , useSimd_(checkSimdSupport())
{
    validatePoints(points);
    updateSegmentParameters();
}

void CatmullRom::validatePoints(const std::vector<Vector3>& points) {
    if (points.size() < 2) {
        throw std::invalid_argument("Catmull-Rom spline requires at least 2 control points");
    }
}

void CatmullRom::setTension(float tension) {
    if (tension <= 0.0f) {
        throw std::invalid_argument("Catmull-Rom spline tension must be positive");
    }
    tension_ = tension;
}

void CatmullRom::setParameterization(Parameterization param) {
    param_ = param;
    updateSegmentParameters();
}

float CatmullRom::computeParameter(const Vector3& p0, const Vector3& p1) const {
    float dist = (p1 - p0).length();
    
    switch (param_) {
        case Parameterization::Uniform:
            return 1.0f;  // Equal spacing
            
        case Parameterization::Centripetal:
            return std::sqrt(dist);  // Square root of chord length
            
        case Parameterization::Chordal:
            return dist;  // Actual chord length
            
        default:
            return 1.0f;  // Fallback to uniform
    }
}

void CatmullRom::updateSegmentParameters() {
    parameters_.clear();
    parameters_.reserve(points_.size());
    
    // First point has parameter 0
    parameters_.push_back(0.0f);
    
    // Compute cumulative parameters
    float total = 0.0f;
    for (size_t i = 1; i < points_.size(); ++i) {
        total += computeParameter(points_[i-1], points_[i]);
        parameters_.push_back(total);
    }
    
    // Normalize parameters to [0,1]
    if (total > 0.0f) {
        for (float& param : parameters_) {
            param /= total;
        }
    }
}

Vector3 CatmullRom::computeTangent(const Vector3& prev, const Vector3& curr, const Vector3& next) const {
    // Get the parameterization values
    float dt0 = computeParameter(prev, curr);
    float dt1 = computeParameter(curr, next);
    
    // Special case for zero distances
    if (dt0 < 1e-6f) dt0 = 1.0f;
    if (dt1 < 1e-6f) dt1 = 1.0f;
    
    // Compute tangent weighted by parameterization
    return (next - prev) * (tension_ / (dt0 + dt1));
}

Hermite CatmullRom::getSegment(size_t index) const {
    if (points_.size() < 4 || index >= points_.size() - 3) {
        throw std::out_of_range("Invalid segment index");
    }
    
    // Get points for the segment
    const Vector3& p0 = points_[index];
    const Vector3& p1 = points_[index + 1];
    const Vector3& p2 = points_[index + 2];
    const Vector3& p3 = points_[index + 3];
    
    // Compute tangents using current parameterization
    Vector3 m1 = computeTangent(p0, p1, p2);
    Vector3 m2 = computeTangent(p1, p2, p3);
    
    // Create Hermite segment
    return Hermite(p1, p2, m1, m2, tension_);
}

Vector3 CatmullRom::evaluate(float t) const {
    if (points_.size() < 4) {
        // Not enough points for interpolation
        return points_.front();  // Return first point
    }
    
    if (t <= 0.0f) return points_[1];    // First interpolated point
    if (t >= 1.0f) return points_[points_.size() - 2];  // Last interpolated point
    
    // Find the segment containing t
    size_t segment = 0;
    for (size_t i = 1; i < parameters_.size() - 2; ++i) {
        if (parameters_[i] <= t && t < parameters_[i + 1]) {
            segment = i - 1;
            break;
        }
    }
    
    // Get the segment curve and local parameter
    Hermite curve = getSegment(segment);
    float local_t = (t - parameters_[segment + 1]) / 
                   (parameters_[segment + 2] - parameters_[segment + 1]);
    
    return curve.evaluate(local_t);
}

std::vector<Vector3> CatmullRom::evaluateMultiple(const std::vector<float>& parameters) const {
    if (parameters.size() > 10000) {
        throw std::invalid_argument("Too many evaluation points requested. Maximum is 10000.");
    }
    
    std::vector<Vector3> results;
    try {
        results.reserve(parameters.size());
        
        // Handle small point counts and edge cases
        if (points_.size() < 4) {
            Vector3 point = points_.front();
            results.resize(parameters.size(), point);
            return results;
        }
        
        // Process each parameter
        for (float t : parameters) {
            results.push_back(evaluate(t));
        }
        
        return results;
    } catch (const std::bad_alloc& e) {
        throw std::runtime_error("Failed to allocate memory for curve evaluation: " + 
                               std::string(e.what()));
    }
}

Vector3 CatmullRom::derivative(float t) const {
    if (points_.size() < 4) {
        return Vector3();  // Zero derivative for simple cases
    }
    
    if (t <= 0.0f) {
        // Return initial tangent
        return computeTangent(points_[0], points_[1], points_[2]);
    }
    if (t >= 1.0f) {
        // Return final tangent
        size_t n = points_.size();
        return computeTangent(points_[n-3], points_[n-2], points_[n-1]);
    }
    
    // Find the segment containing t
    size_t segment = 0;
    for (size_t i = 1; i < parameters_.size() - 2; ++i) {
        if (parameters_[i] <= t && t < parameters_[i + 1]) {
            segment = i - 1;
            break;
        }
    }
    
    // Get the segment curve and local parameter
    Hermite curve = getSegment(segment);
    float local_t = (t - parameters_[segment + 1]) / 
                   (parameters_[segment + 2] - parameters_[segment + 1]);
    
    // Scale the derivative by the parameter range
    float scale = 1.0f / (parameters_[segment + 2] - parameters_[segment + 1]);
    return curve.derivative().evaluate(local_t) * scale;
}

void CatmullRom::addPoint(const Vector3& point) {
    points_.push_back(point);
    updateSegmentParameters();
}

void CatmullRom::insertPoint(const Vector3& point, size_t index) {
    if (index > points_.size()) {
        throw std::out_of_range("Insert index out of range");
    }
    points_.insert(points_.begin() + index, point);
    updateSegmentParameters();
}

void CatmullRom::removePoint(size_t index) {
    if (index >= points_.size()) {
        throw std::out_of_range("Remove index out of range");
    }
    if (points_.size() <= 2) {
        throw std::invalid_argument("Cannot remove points from minimum size spline");
    }
    points_.erase(points_.begin() + index);
    updateSegmentParameters();
}

} // namespace math
} // namespace pynovage