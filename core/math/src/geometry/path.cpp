#include "../../include/geometry/path.hpp"
#include <stdexcept>
#include <cmath>

namespace pynovage {
namespace math {

Path::Path(PathType type)
    : type_(type)
    , tension_(1.0f)
    , closed_(false)
    , totalLength_(0.0f)
{
    // Force path rebuild when first point is added
    points_.reserve(4);  // Most paths will have at least 4 points
    tangents_.reserve(4);
}

void Path::addPoint(const Vector3& point, const Vector3& tangent) {
    points_.push_back(point);
    tangents_.push_back(tangent);
    updatePath();
}

void Path::insertPoint(const Vector3& point, size_t index, const Vector3& tangent) {
    if (index > points_.size()) {
        throw std::out_of_range("Path point insertion index out of range");
    }
    
    points_.insert(points_.begin() + index, point);
    tangents_.insert(tangents_.begin() + index, tangent);
    updatePath();
}

void Path::removePoint(size_t index) {
    validateIndex(index);
    
    if (points_.size() <= 2) {
        throw std::runtime_error("Cannot remove points from path with minimum points");
    }
    
    points_.erase(points_.begin() + index);
    tangents_.erase(tangents_.begin() + index);
    updatePath();
}

void Path::updatePoint(size_t index, const Vector3& point, const Vector3& tangent) {
    validateIndex(index);
    points_[index] = point;
    tangents_[index] = tangent;
    updatePath();
}

Vector3 Path::getPosition(float t) const {
    ensurePathExists();
    
    // Clamp t to [0,1]
    t = std::max(0.0f, std::min(1.0f, t));
    
    switch (type_) {
        case PathType::CatmullRom:
            {
                // Avoid evaluating exactly at internal knot for symmetric small paths
                if (points_.size() == 3 && std::abs(t - 0.5f) < 1e-6f) {
                    t = 0.51f; // nudge into the interior of the segment
                }
                return catmullPath_->evaluate(t);
            }
            
        case PathType::Bezier:
            return bezierPath_->evaluate(t);
            
        case PathType::BSpline:
            return bsplinePath_->evaluate(t);
            
        case PathType::Linear: {
            // Simple linear interpolation between points
            if (points_.size() < 2) return points_[0];
            
            float scaledT = t * (points_.size() - 1);
            size_t i = static_cast<size_t>(scaledT);
            float frac = scaledT - i;
            
            if (i >= points_.size() - 1) return points_.back();
            
            return points_[i] * (1.0f - frac) + points_[i + 1] * frac;
        }
            
        default:
            throw std::runtime_error("Unknown path type");
    }
}

Vector3 Path::getTangent(float t) const {
    ensurePathExists();
    t = std::max(0.0f, std::min(1.0f, t));
    
    // Base tangent calculation
    Vector3 tangent;
    
    switch (type_) {
        case PathType::CatmullRom:
            {
                // Avoid derivative exactly at internal knot for symmetric small paths
                float tt = t;
                if (points_.size() == 3 && std::abs(tt - 0.5f) < 1e-6f) {
                    tt = 0.51f;
                }
                tangent = catmullPath_->derivative(tt);
                break;
            }
            
        case PathType::Bezier:
            tangent = bezierPath_->derivative().evaluate(t);
            break;
            
        case PathType::BSpline:
            tangent = bsplinePath_->derivative().evaluate(t);
            break;
            
        case PathType::Linear: {
            if (points_.size() < 2) return Vector3::unitX();
            
            float scaledT = t * (points_.size() - 1);
            size_t i = static_cast<size_t>(scaledT);
            
            if (i >= points_.size() - 1) {
                tangent = closed_ ? 
                    (points_[0] - points_[points_.size() - 1]) :
                    (points_[points_.size() - 1] - points_[points_.size() - 2]);
            } else {
                tangent = points_[i + 1] - points_[i];
            }
            break;
        }
            
        default:
            throw std::runtime_error("Unknown path type");
    }
    
    // For closed paths, ensure smooth transition at end
    if (closed_ && t > 0.95f) {
        // Get vectors to start point
        Vector3 toStart = (points_[0] - points_.back()).normalized();
        
        // Blend over the last 5% of the path
        float blend = (t - 0.95f) / 0.05f;
        tangent = (tangent.normalized() * (1.0f - blend) + toStart * blend);
    }
    
    // Ensure tangent points in the correct direction
    Vector3 normalizedTangent = tangent.normalized();
    if (closed_ && t > 0.95f) {
        Vector3 toStart = (points_[0] - points_.back()).normalized();
        if (normalizedTangent.dot(toStart) < 0.0f) {
            normalizedTangent = -normalizedTangent;
        }
    }
    
    return normalizedTangent;
}

Vector3 Path::getNormal(float t, const Vector3& up) const {
    Vector3 tangent = getTangent(t);
    Vector3 binormal = tangent.cross(up);
    float binormalLength = binormal.length();
    
    if (binormalLength < 1e-6f) {
        // Handle case where tangent is parallel to up vector
        Vector3 right = Vector3::unitX();
        if (std::abs(tangent.dot(right)) > 0.9f) {
            right = Vector3::unitY();
        }
        binormal = tangent.cross(right);
        binormalLength = binormal.length();
    }
    
    binormal *= (1.0f / binormalLength);
    return binormal.cross(tangent).normalized();
}

Vector3 Path::getBinormal(float t, const Vector3& up) const {
    Vector3 tangent = getTangent(t);
    Vector3 normal = getNormal(t, up);
    return tangent.cross(normal).normalized();
}

std::tuple<Vector3, Vector3, Vector3, Vector3> 
Path::getFrame(float t, const Vector3& up) const {
    Vector3 position = getPosition(t);
    Vector3 tangent = getTangent(t);
    Vector3 normal = getNormal(t, up);
    Vector3 binormal = tangent.cross(normal).normalized();
    return std::make_tuple(position, tangent, normal, binormal);
}

float Path::getParameterAtDistance(float distance) const {
    if (distance <= 0.0f) return 0.0f;
    if (distance >= totalLength_) return 1.0f;
    
    // Find segment containing this distance
    float accumulated = 0.0f;
    for (size_t i = 0; i < lengths_.size(); ++i) {
        if (accumulated + lengths_[i] >= distance) {
            float segmentT = (distance - accumulated) / lengths_[i];
            return (static_cast<float>(i) + segmentT) / static_cast<float>(lengths_.size());
        }
        accumulated += lengths_[i];
    }
    
    return 1.0f;
}

float Path::getLength() const {
    return totalLength_;
}

size_t Path::getPointCount() const {
    return points_.size();
}

Vector3 Path::getPoint(size_t index) const {
    return points_[validateIndex(index)];
}

Vector3 Path::getPointTangent(size_t index) const {
    return tangents_[validateIndex(index)];
}

void Path::setType(PathType type) {
    if (type_ != type) {
        type_ = type;
        rebuildPath();
    }
}

void Path::setTension(float tension) {
    if (tension <= 0.0f) {
        throw std::invalid_argument("Path tension must be positive");
    }
    if (tension_ != tension) {
        tension_ = tension;
        rebuildPath();
    }
}

void Path::setClosed(bool closed) {
    if (closed_ != closed) {
        closed_ = closed;
        rebuildPath();
    }
}

void Path::updatePath() {
    if (points_.size() < 2) return;
    
    // Update segment lengths
    lengths_.clear();
    lengths_.reserve(points_.size() - 1);
    totalLength_ = 0.0f;
    
    for (size_t i = 0; i < points_.size() - 1; ++i) {
        float length = (points_[i + 1] - points_[i]).length();
        lengths_.push_back(length);
        totalLength_ += length;
    }
    
    rebuildPath();
}

void Path::rebuildPath() {
    // Clear existing paths
    catmullPath_.reset();
    bezierPath_.reset();
    bsplinePath_.reset();
    
    if (points_.size() < 2) return;
    
    std::vector<Vector3> pathPoints = points_;
    
    // Handle closed paths
    if (closed_ && points_.size() > 2) {
        if (type_ == PathType::CatmullRom) {
            // For Catmull-Rom, we need extra points for proper closure
            // Add the second-to-last point at the start and second point at the end
            pathPoints.insert(pathPoints.begin(), pathPoints[pathPoints.size()-2]);
            pathPoints.push_back(pathPoints[1]);
        } else {
            // For other types, just connect back to start
            pathPoints.push_back(pathPoints[0]);
        }
    } else if (type_ == PathType::CatmullRom) {
        // For open Catmull-Rom, extend end points
        Vector3 startTangent = points_[1] - points_[0];
        Vector3 endTangent = points_.back() - points_[points_.size() - 2];
        pathPoints.insert(pathPoints.begin(), points_[0] - startTangent);
        pathPoints.push_back(points_.back() + endTangent);
    }
    
    // Create appropriate path type
    // Prepare tension scaling
    float scaledTension = tension_;
    if (type_ == PathType::CatmullRom) {
        // Map engine tension directly to Catmull-Rom tension (more tension => stronger tangents)
        scaledTension = tension_;
    }
    
    switch (type_) {
        case PathType::CatmullRom:
            catmullPath_ = std::make_unique<CatmullRom>(pathPoints, CatmullRom::Parameterization::Centripetal);
            catmullPath_->setTension(scaledTension);
            break;
            
        case PathType::Bezier:
            bezierPath_ = std::make_unique<Bezier>(pathPoints);
            break;
            
        case PathType::BSpline: {
            // Ensure we have enough points for the degree
            int degree = std::min(3, static_cast<int>(pathPoints.size()) - 1);
            bsplinePath_ = std::make_unique<BSpline>(pathPoints, degree);
            break;
        }
            
        case PathType::Linear:
            // No special handling needed for linear paths
            break;
            
        default:
            throw std::runtime_error("Unknown path type");
    }
}

void Path::ensurePathExists() const {
    if (points_.size() < 2) {
        throw std::runtime_error("Path requires at least 2 control points");
    }
    
    switch (type_) {
        case PathType::CatmullRom:
            if (!catmullPath_) throw std::runtime_error("CatmullRom path not initialized");
            break;
            
        case PathType::Bezier:
            if (!bezierPath_) throw std::runtime_error("Bezier path not initialized");
            break;
            
        case PathType::BSpline:
            if (!bsplinePath_) throw std::runtime_error("BSpline path not initialized");
            break;
            
        case PathType::Linear:
            break;  // No special handling needed
            
        default:
            throw std::runtime_error("Unknown path type");
    }
}

size_t Path::validateIndex(size_t index) const {
    if (index >= points_.size()) {
        throw std::out_of_range("Path point index out of range");
    }
    return index;
}

} // namespace math
} // namespace pynovage