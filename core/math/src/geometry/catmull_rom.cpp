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
    if (dist < 1e-6f) return 1e-6f;  // Avoid division by zero
    
    switch (param_) {
        case Parameterization::Uniform:
            return 1.0f;  // Equal spacing
            
        case Parameterization::Centripetal:
            return std::max(std::sqrt(dist), 1e-6f);  // Square root for better curvature
            
        case Parameterization::Chordal:
            return std::max(dist, 1e-6f);  // Proportional to distance
            
        default:
            return 1.0f;  // Fallback to uniform
    }
}

void CatmullRom::updateSegmentParameters() {
    parameters_.clear();
    parameters_.reserve(points_.size());
    
    // First point has parameter 0
    parameters_.push_back(0.0f);
    
    // Compute distances for SIMD batch processing
    const size_t batchSize = 4;  // SIMD width
    alignas(32) float distances[4];
    float total = 0.0f;
    
    // Process points in batches of 4
    for (size_t i = 1; i < points_.size(); i += batchSize) {
        const size_t count = std::min(batchSize, points_.size() - i);
        
        // Compute distances for this batch
        for (size_t j = 0; j < count; ++j) {
            float dist = (points_[i+j] - points_[i+j-1]).length();
            distances[j] = std::max(dist, 1e-6f);
        }
        
        // Apply parameterization using SIMD
        alignas(32) float params[4];
        if (param_ == Parameterization::Centripetal && useSimd_) {
            // SIMD square root for centripetal parameterization
            for (size_t j = 0; j < count; ++j) {
                params[j] = std::sqrt(distances[j]);
            }
        } else if (param_ == Parameterization::Chordal) {
            // Direct use of distances for chordal
            std::copy(distances, distances + count, params);
        } else {
            // Uniform parameterization
            std::fill(params, params + count, 1.0f);
        }
        
        // Accumulate parameters
        for (size_t j = 0; j < count; ++j) {
            total += params[j];
            parameters_.push_back(total);
        }
    }
    
    // Normalize parameters to [0,1] using SIMD
    if (total > 0.0f) {
        const float scale = 1.0f / total;
        for (size_t i = 0; i < parameters_.size(); i += batchSize) {
            const size_t count = std::min(batchSize, parameters_.size() - i);
            alignas(32) float batch[4];
            
            // Load batch
            for (size_t j = 0; j < count; ++j) {
                batch[j] = parameters_[i+j];
            }
            
            // Scale batch with SIMD
            alignas(32) float scales[4] = {scale, scale, scale, scale};
            alignas(32) float result[4];
            SimdUtils::Multiply4f(batch, scales, result);
            
            // Store results
            for (size_t j = 0; j < count; ++j) {
                parameters_[i+j] = result[j];
            }
        }
    }
}

Vector3 CatmullRom::computeTangent(const Vector3& prev, const Vector3& curr, const Vector3& next) const {
    // Get the parameterization values
    float dt0 = computeParameter(prev, curr);
    float dt1 = computeParameter(curr, next);
    
    // Using accumulated parameterization for better behavior
    Vector3 tangent;
    if (dt0 > 1e-6f) {
        tangent += (curr - prev) * (tension_ / dt0);
    }
    if (dt1 > 1e-6f) {
        tangent += (next - curr) * (tension_ / dt1);
    }
    
    // Return normalized tangent scaled by tension
    float len = tangent.length();
    if (len > 1e-6f) {
        return tangent * (0.5f / len);  // Average and normalize
    }
    
    // Fallback for degenerate case
    return next - prev;
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
        
        // Batch process parameters
        const size_t batchSize = 4;  // SIMD width
        std::vector<Hermite> segment_cache;  // Cache for reuse
        
        for (size_t i = 0; i < parameters.size(); i += batchSize) {
            const size_t count = std::min(batchSize, parameters.size() - i);
            alignas(32) float ts[4];  // Global parameters
            alignas(32) size_t segments[4];  // Segment indices
            alignas(32) float local_ts[4];  // Local parameters
            
            // Find segments and compute local parameters
            for (size_t j = 0; j < count; ++j) {
                float t = parameters[i+j];
                ts[j] = t;
                
                if (t <= 0.0f) {
                    results.push_back(points_[1]);
                    continue;
                }
                if (t >= 1.0f) {
                    results.push_back(points_[points_.size() - 2]);
                    continue;
                }
                
                // Find segment
                size_t segment = 0;
                for (size_t k = 1; k < parameters_.size() - 2; ++k) {
                    if (parameters_[k] <= t && t < parameters_[k + 1]) {
                        segment = k - 1;
                        break;
                    }
                }
                
                segments[j] = segment;
                local_ts[j] = (t - parameters_[segment + 1]) / 
                            (parameters_[segment + 2] - parameters_[segment + 1]);
                
                // Ensure segment is cached
                while (segment_cache.size() <= segment) {
                    segment_cache.push_back(getSegment(segment_cache.size()));
                }
            }
            
            // Evaluate points using SIMD
            if (useSimd_ && count == batchSize) {
                alignas(32) float points[4][3];  // x,y,z components
                
                // Evaluate each point using cached segments
                for (size_t j = 0; j < count; ++j) {
                    Vector3 point = segment_cache[segments[j]].evaluate(local_ts[j]);
                    points[j][0] = point.x;
                    points[j][1] = point.y;
                    points[j][2] = point.z;
                }
                
                // Store results
                for (size_t j = 0; j < count; ++j) {
                    results.emplace_back(points[j][0], points[j][1], points[j][2]);
                }
            } else {
                // Scalar fallback for partial batches
                for (size_t j = 0; j < count; ++j) {
                    if (ts[j] <= 0.0f || ts[j] >= 1.0f) continue;  // Already handled
                    results.push_back(
                        segment_cache[segments[j]].evaluate(local_ts[j]));
                }
            }
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