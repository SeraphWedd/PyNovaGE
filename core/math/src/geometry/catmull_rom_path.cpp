#include "../../include/geometry/catmull_rom_path.hpp"
#include "../../include/simd_utils.hpp"
#include <algorithm>
#include <cassert>

namespace pynovage {
namespace math {

CatmullRomPath::CatmullRomPath(
    const std::vector<Vector3>& points,
    MovementMode mode)
    : Path(points, mode)
    , spline_(points, CatmullRom::Parameterization::Chordal, DEFAULT_TENSION) {
    closed_ = (points.size() >= 4 && (points.front() - points.back()).lengthSquared() < 1e-6f);
    updateSpline();
}

void CatmullRomPath::updateSpline() {
    if (!isDirty_) return;

    spline_ = CatmullRom(points_, spline_.getParameterization(), tension_);
    
    buildArcLengthTable();
    isDirty_ = false;
}

void CatmullRomPath::buildArcLengthTable() {
    arcLengths_.clear();
    parameters_.clear();
    arcLengths_.reserve(LOOKUP_TABLE_SIZE);
    parameters_.reserve(LOOKUP_TABLE_SIZE);

    float accumLength = 0.0f;
    arcLengths_.push_back(accumLength);
    parameters_.push_back(0.0f);

    // Use adaptive sampling to get more accurate arc length
    const size_t SUBDIVISIONS = 10;
    const float MIN_LENGTH = 1e-6f;
    Vector3 prevPos = spline_.evaluate(0.0f);
    
    for (size_t i = 1; i < LOOKUP_TABLE_SIZE; ++i) {
        float t = float(i) / float(LOOKUP_TABLE_SIZE - 1);
        Vector3 pos = spline_.evaluate(t);
        float dt = t - parameters_.back();
        
        // Subdivide this segment for better accuracy
        float subLen = 0.0f;
        Vector3 subPrevPos = prevPos;
        
        for (size_t j = 1; j <= SUBDIVISIONS; ++j) {
            float subT = parameters_.back() + (dt * j) / SUBDIVISIONS;
            Vector3 subPos = spline_.evaluate(subT);
            Vector3 diff = subPos - subPrevPos;
            float segLen = diff.length();
            
            if (segLen > MIN_LENGTH) {
                subLen += segLen;
            }
            
            subPrevPos = subPos;
        }
        
        accumLength += subLen;
        arcLengths_.push_back(accumLength);
        parameters_.push_back(t);
        
        prevPos = pos;
    }

    totalLength_ = accumLength;
}

CatmullRomPath::State CatmullRomPath::computeStateAtParameter(float t) const {
    State state;
    
    // Position and derivatives
    state.position = spline_.evaluate(t);
    Vector3 tangent = spline_.derivative(t);
    Vector3 accel = spline_.derivative(t);  // Second derivative not implemented yet, using first for now
    
    // Normalize tangent for orientation
    float tangentLength = tangent.length();
    if (tangentLength > 1e-6f) {
        tangent /= tangentLength;
    } else {
        // Handle zero tangent case by using previous direction or default
        tangent = Vector3(1.0f, 0.0f, 0.0f);
    }

    // Compute Frenet frame with rotation minimizing frame
    static const Vector3 WORLD_UP(0.0f, 1.0f, 0.0f);
    Vector3 up;
    
    float upDot = std::abs(tangent.dot(WORLD_UP));
    if (upDot > 0.99999f) {
        // Special case: tangent aligned with up vector
        up = Vector3(0.0f, 0.0f, tangent.y > 0.0f ? -1.0f : 1.0f);
    } else {
        up = (WORLD_UP - tangent * tangent.dot(WORLD_UP)).normalized();
    }
    
    Vector3 right = up.cross(tangent).normalized();
    up = tangent.cross(right).normalized();
    
    // Ensure right-handed coordinate system
    if (right.cross(up).dot(tangent) < 0.0f) {
        right = -right;
        up = tangent.cross(right).normalized();
    }
    
    // Set orientation
    state.rotation = Quaternion::fromBasis(tangent, up, right);
    
    // Compute curvature using normalized derivatives
    if (tangentLength > 1e-6f) {
        Vector3 normalizedTangent = tangent / tangentLength;
        Vector3 normalizedAccel = accel / tangentLength;
        Vector3 crossProd = normalizedTangent.cross(normalizedAccel);
        state.curvature = crossProd.length();
    } else {
        state.curvature = 0.0f;
    }
    
    return state;
}

float CatmullRomPath::timeToArcLength(float time) const {
    time = std::clamp(time, 0.0f, 1.0f);
    float targetLength = time * totalLength_;
    
    // Binary search for closest parameter
    size_t left = 0;
    size_t right = arcLengths_.size() - 1;
    
    while (left < right) {
        size_t mid = (left + right) / 2;
        if (arcLengths_[mid] < targetLength) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    size_t index = left;
    if (index > 0) {
        // Linear interpolation between surrounding points
        float t0 = parameters_[index - 1];
        float t1 = parameters_[index];
        float d0 = arcLengths_[index - 1];
        float d1 = arcLengths_[index];
        
        float alpha = (targetLength - d0) / (d1 - d0);
        return t0 + (t1 - t0) * alpha;
    }
    return parameters_[index];
}

float CatmullRomPath::arcLengthToTime(float arcLength) const {
    return arcLength / totalLength_;
}

CatmullRomPath::State CatmullRomPath::getState(float time) const {
    time = std::clamp(time, 0.0f, 1.0f);
    float t = timeToArcLength(time);
    State state = computeStateAtParameter(t);
    
    state.time = time;
    state.distance = time * totalLength_;
    
    if (mode_ == MovementMode::ConstantSpeed) {
        state.speed = 1.0f;  // Unit speed
    } else {
        Vector3 tangent = spline_.derivative(t);
        state.speed = tangent.length();
    }
    
    return state;
}

CatmullRomPath::State CatmullRomPath::getStateAtDistance(float distance) const {
    float time = distance / totalLength_;
    return getState(time);
}

CatmullRomPath::State CatmullRomPath::updateConstantSpeed(
    const State& currentState,
    float deltaTime) const {
    
    float newDistance = currentState.distance + 
                       currentState.speed * deltaTime;
    return getStateAtDistance(newDistance);
}

CatmullRomPath::State CatmullRomPath::getClosestPoint(
    const Vector3& point) const {
    
    float t = findClosestParameter(point);
    return computeStateAtParameter(t);
}

std::unique_ptr<Path> CatmullRomPath::blend(
    const Path& other, float blendFactor) const {
    
    // Get points from both paths
    std::vector<Vector3> points1 = points_;
    std::vector<Vector3> points2;
    
    if (auto* otherPath = dynamic_cast<const CatmullRomPath*>(&other)) {
        points2 = otherPath->points_;
    } else {
        // Sample points from the other path type
        const size_t numSamples = 20;
        points2.reserve(numSamples);
        
        for (size_t i = 0; i < numSamples; ++i) {
            float t = float(i) / float(numSamples - 1);
            points2.push_back(other.getState(t).position);
        }
    }
    
    // Ensure paths have same number of points
    size_t numPoints = std::max(points1.size(), points2.size());
    if (points1.size() < numPoints) {
        resamplePath(points1, numPoints);
    }
    if (points2.size() < numPoints) {
        resamplePath(points2, numPoints);
    }
    
    // Blend points
    std::vector<Vector3> blendedPoints(numPoints);
    for (size_t i = 0; i < numPoints; ++i) {
        blendedPoints[i] = lerp(points1[i], points2[i], blendFactor);
    }
    
    // Create new path with blended points
    auto blendedPath = std::make_unique<CatmullRomPath>(blendedPoints, mode_);
    if (auto* otherCR = dynamic_cast<const CatmullRomPath*>(&other)) {
        // Blend Catmull-Rom specific parameters
float t1 = tension_;
        float t2 = otherCR->tension_;
        float blendedTension = t1 * (1.0f - blendFactor) + t2 * blendFactor;
        blendedPath->setTension(blendedTension);
    }
    
    return blendedPath;
}

void CatmullRomPath::resamplePath(
    std::vector<Vector3>& points,
    size_t targetCount) const {
    
    if (points.size() < 2 || targetCount < 2) return;
    
    std::vector<Vector3> result;
    result.reserve(targetCount);
    
    for (size_t i = 0; i < targetCount; ++i) {
        float t = float(i) / float(targetCount - 1);
        float sourceIdx = t * float(points.size() - 1);
        
        size_t idx1 = size_t(sourceIdx);
        size_t idx2 = std::min(idx1 + 1, points.size() - 1);
        float alpha = sourceIdx - float(idx1);
        
        result.push_back(lerp(points[idx1], points[idx2], alpha));
    }
    
    points = std::move(result);
}

float CatmullRomPath::getLength() const {
    return totalLength_;
}

float CatmullRomPath::getCurvature(float time) const {
    time = std::clamp(time, 0.0f, 1.0f);
    float t = timeToArcLength(time);
    return computeStateAtParameter(t).curvature;
}

bool CatmullRomPath::isClosed() const {
    if (points_.size() < 2) return false;
    return (points_.front() - points_.back()).lengthSquared() < 1e-6f;
}

float CatmullRomPath::findClosestParameter(const Vector3& point) const {
    // Initial search using uniform sampling
    float minDistSq = std::numeric_limits<float>::max();
    size_t closestIndex = 0;
    
    const size_t COARSE_SAMPLES = 50;
    const float dt = 1.0f / (COARSE_SAMPLES - 1);
    
    // Coarse search
    for (size_t i = 0; i < COARSE_SAMPLES; ++i) {
        float t = i * dt;
        Vector3 pos = spline_.evaluate(t);
        float distSq = (pos - point).lengthSquared();
        
        if (distSq < minDistSq) {
            minDistSq = distSq;
            closestIndex = i;
        }
    }
    
    // Refined search in local neighborhood
    float t0 = std::max(0.0f, (closestIndex - 1) * dt);
    float t1 = std::min(1.0f, (closestIndex + 1) * dt);
    
    const size_t FINE_SAMPLES = 20;
    float localDt = (t1 - t0) / FINE_SAMPLES;
    float bestT = closestIndex * dt;
    
    for (size_t i = 0; i <= FINE_SAMPLES; ++i) {
        float t = t0 + i * localDt;
        Vector3 pos = spline_.evaluate(t);
        float distSq = (pos - point).lengthSquared();
        
        if (distSq < minDistSq) {
            minDistSq = distSq;
            bestT = t;
        }
    }
    
    // Final refinement using Newton-Raphson
    const int MAX_ITER = 10;
    const float EPSILON = 1e-6f;
    float t = bestT;
    float lastImprovement = minDistSq;
    
    for (int i = 0; i < MAX_ITER; ++i) {
        Vector3 pos = spline_.evaluate(t);
        Vector3 deriv = spline_.derivative(t);
        Vector3 diff = pos - point;
        
        float distSq = diff.lengthSquared();
        if (std::abs(distSq - lastImprovement) < EPSILON * EPSILON) {
            break;
        }
        lastImprovement = distSq;
        
        float numerator = diff.dot(deriv);
        float denominator = deriv.lengthSquared();
        
        if (denominator < EPSILON * EPSILON) {
            break;
        }
        
        t = std::clamp(t - numerator / denominator, 0.0f, 1.0f);
    }
    
    return arcLengthToTime(t * getLength());
}
}

void CatmullRomPath::setTension(float tension) {
    if (tension_ != tension) {
        tension_ = tension;
        isDirty_ = true;
        updateSpline();
    }
}

void CatmullRomPath::setParameterization(CatmullRom::Parameterization type) {
    spline_.setParameterization(type);
    isDirty_ = true;
    updateSpline();
}

void CatmullRomPath::addPoint(const Vector3& point) {
    points_.push_back(point);
    isDirty_ = true;
    updateSpline();
}

void CatmullRomPath::removePoint(size_t index) {
    if (index < points_.size()) {
        points_.erase(points_.begin() + index);
        isDirty_ = true;
        updateSpline();
    }
}

void CatmullRomPath::setTension(float tension) {
    if (tension_ != tension) {
        tension_ = tension;
        isDirty_ = true;
        updateSpline();
    }
}

void CatmullRomPath::setParameterization(CatmullRom::Parameterization type) {
    spline_.setParameterization(type);
    isDirty_ = true;
    updateSpline();
}

void CatmullRomPath::addPoint(const Vector3& point) {
    points_.push_back(point);
    isDirty_ = true;
    updateSpline();
}

void CatmullRomPath::removePoint(size_t index) {
    if (index < points_.size()) {
        points_.erase(points_.begin() + index);
        isDirty_ = true;
        updateSpline();
    }
}

} // namespace math
} // namespace pynovage
