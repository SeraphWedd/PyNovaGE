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
    // Compute velocity
    Vector3 velocity = (end - start) / timeStep;
    
    // Result buffer: [time, px, py, pz]
    alignas(16) float result[4];
    
    bool hasCollision = SimdUtils::TestMovingSphereAABB(
        &start.x,
        &velocity.x,
        sphere.radius,
        &aabb.min.x,
        &aabb.max.x,
        timeStep,
        result
    );
    
    if (!hasCollision) {
        return std::nullopt;
    }
    
    // Compute collision details
    Vector3 collisionPoint(result[1], result[2], result[3]);
    
    ContinuousCollisionResult collisionResult;
    collisionResult.intersects = true;
    collisionResult.timeOfImpact = result[0] / timeStep; // Normalize to [0,1]
    collisionResult.point = collisionPoint;
    collisionResult.distance = velocity.length() * result[0];
    
    // Compute the normal based on the collision sphere center (SIMD returns center at collision)
    Vector3 sphereCenter = collisionPoint;
    
    // Find the closest point on the AABB to the sphere center at collision time
    Vector3 closest(
        std::clamp(sphereCenter.x, aabb.min.x, aabb.max.x),
        std::clamp(sphereCenter.y, aabb.min.y, aabb.max.y),
        std::clamp(sphereCenter.z, aabb.min.z, aabb.max.z)
    );
    
    // The normal is from closest point to sphere center (points away from AABB)
    collisionResult.normal = (sphereCenter - closest).normalized();
    
    return collisionResult;
}

} // namespace geometry
} // namespace math
} // namespace pynovage