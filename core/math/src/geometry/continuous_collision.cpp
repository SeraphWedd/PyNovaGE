#include "geometry/continuous_collision.hpp"
#include <algorithm>

namespace pynovage {
namespace math {
namespace geometry {

std::optional<ContinuousCollisionResult> testMovingSphereSphere(
    const Sphere& movingSphere,
    const Sphere& staticSphere,
    const Vector3& start,
    const Vector3& end,
    float timeStep)
{
    // The problem can be reduced to finding when a moving point hits a sphere
    // The moving point is the center of the moving sphere
    // The target sphere's radius is increased by the radius of the moving sphere
    Sphere expandedSphere(staticSphere.center, staticSphere.radius + movingSphere.radius);
    
    // Calculate velocity
    Vector3 velocity = (end - start) / timeStep;
    
    // Quadratic equation coefficients
    // |S + tv - C|^2 = r^2, where S=start, v=velocity, C=sphere center, r=combined radius
    Vector3 toSphere = start - expandedSphere.center;
    float a = velocity.dot(velocity);
    float b = 2.0f * velocity.dot(toSphere);
    float c = toSphere.dot(toSphere) - (expandedSphere.radius * expandedSphere.radius);
    
    // Solve quadratic equation at^2 + bt + c = 0
    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) {
        return std::nullopt;  // No collision
    }
    
    // Compute times of impact
    float sqrtD = std::sqrt(discriminant);
    float t0 = (-b - sqrtD) / (2.0f * a);
    float t1 = (-b + sqrtD) / (2.0f * a);
    
    // Get earliest valid collision time within the time step
    float t = std::numeric_limits<float>::infinity();
    if (t0 >= 0.0f && t0 <= timeStep) t = t0;
    if (t1 >= 0.0f && t1 <= timeStep && t1 < t) t = t1;
    
    if (t == std::numeric_limits<float>::infinity()) {
        return std::nullopt;  // No collision within time step
    }
    
    // Compute collision details
    ContinuousCollisionResult result;
    result.intersects = true;
    result.timeOfImpact = t / timeStep;  // Normalize to [0,1]
    
    // Point of collision
    Vector3 collisionPoint = start + velocity * t;
    result.point = collisionPoint;
    
    // Surface normal is from sphere center to collision point
    result.normal = (collisionPoint - staticSphere.center).normalized();
    
    // Distance traveled to collision
    result.distance = (collisionPoint - start).length();
    
    return result;
}

std::optional<ContinuousCollisionResult> testMovingSphereAABB(
    const Sphere& sphere,
    const AABB& aabb,
    const Vector3& start,
    const Vector3& end,
    float timeStep)
{
    Vector3 velocity = (end - start) / timeStep;

    // First check if we're already intersecting at start position
    Vector3 closest = Vector3(
        std::clamp(start.x, aabb.min.x, aabb.max.x),
        std::clamp(start.y, aabb.min.y, aabb.max.y),
        std::clamp(start.z, aabb.min.z, aabb.max.z)
    );
    
    Vector3 toSphere = start - closest;
    if (toSphere.lengthSquared() <= sphere.radius * sphere.radius) {
        ContinuousCollisionResult result;
        result.intersects = true;
        result.timeOfImpact = 0.0f;
        result.point = start;
        result.normal = toSphere.normalized();
        result.distance = 0.0f;
        return result;
    }
    
    // Expand AABB by sphere radius
    AABB expandedAABB;
    expandedAABB.min = aabb.min - Vector3(sphere.radius, sphere.radius, sphere.radius);
    expandedAABB.max = aabb.max + Vector3(sphere.radius, sphere.radius, sphere.radius);
    
    float t_near = 0.0f;
    float t_far = timeStep;
    
    // Helper to process a single axis slab
    auto process_axis = [&](float startCoord, float velCoord, float minCoord, float maxCoord) -> bool {
        if (std::abs(velCoord) < constants::epsilon) {
            // Moving parallel to this slab; must start within the slab
            if (startCoord < minCoord || startCoord > maxCoord) {
                return false;
            }
            return true;
        }
        float t1 = (minCoord - startCoord) / velCoord;
        float t2 = (maxCoord - startCoord) / velCoord;
        if (t1 > t2) std::swap(t1, t2);
        t_near = std::max(t_near, t1);
        t_far = std::min(t_far, t2);
        if (t_near > t_far || t_far < 0.0f) {
            return false;
        }
        return true;
    };

    // X axis
    if (!process_axis(start.x, velocity.x, expandedAABB.min.x, expandedAABB.max.x)) {
        return std::nullopt;
    }
    // Y axis
    if (!process_axis(start.y, velocity.y, expandedAABB.min.y, expandedAABB.max.y)) {
        return std::nullopt;
    }
    // Z axis
    if (!process_axis(start.z, velocity.z, expandedAABB.min.z, expandedAABB.max.z)) {
        return std::nullopt;
    }
    
    // If nearest intersection is beyond our time step, no collision
    if (t_near > timeStep) {
        return std::nullopt;
    }
    
    // We have a collision - compute the details
    ContinuousCollisionResult result;
    result.intersects = true;
    result.timeOfImpact = t_near / timeStep;  // Normalize to [0,1]
    
    // Collision point
    Vector3 collisionPoint = start + velocity * t_near;
    result.point = collisionPoint;
    
    // Find closest point on original AABB for normal calculation
    closest = Vector3(
        std::clamp(collisionPoint.x, aabb.min.x, aabb.max.x),
        std::clamp(collisionPoint.y, aabb.min.y, aabb.max.y),
        std::clamp(collisionPoint.z, aabb.min.z, aabb.max.z)
    );
    
    // Normal points from closest point to sphere center
    result.normal = (collisionPoint - closest).normalized();
    result.distance = velocity.length() * t_near;
    
    return result;
}

} // namespace geometry
} // namespace math
} // namespace pynovage