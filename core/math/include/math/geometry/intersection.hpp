#ifndef PYNOVAGE_MATH_GEOMETRY_INTERSECTION_HPP
#define PYNOVAGE_MATH_GEOMETRY_INTERSECTION_HPP

#include "primitives.hpp"
#include <optional>

namespace pynovage {
namespace math {
namespace geometry {

/**
 * @brief Stores information about an intersection
 */
struct IntersectionResult {
    bool intersects = false;    // Whether intersection occurred
    float distance = 0.0f;     // Distance to intersection
    Vector3 point;                 // Point of intersection
    Vector3 normal;                // Surface normal at intersection
};

/**
 * @brief Tests if a ray intersects with a plane
 * @param ray The ray to test
 * @param plane The plane to test against
 * @return Optional intersection result, empty if no intersection
 */
inline std::optional<IntersectionResult> rayPlaneIntersection(const Ray3D& ray, const Plane& plane) {
    float denom = ray.direction.dot(plane.normal);
    
    // Ray is parallel to plane
    if (std::abs(denom) < constants::epsilon) {
        return std::nullopt;
    }
    
    float t = (plane.distance - ray.origin.dot(plane.normal)) / denom;
    
    // Intersection is behind ray origin
    if (t < 0.0f) {
        return std::nullopt;
    }
    
    IntersectionResult result;
    result.intersects = true;
    result.distance = t;
    result.point = ray.getPoint(t);
    result.normal = denom < 0.0f ? plane.normal : plane.normal * -1.0f;
    
    return result;
}

/**
 * @brief Tests if a ray intersects with a sphere
 * @param ray The ray to test
 * @param sphere The sphere to test against
 * @return Optional intersection result, empty if no intersection
 */
inline std::optional<IntersectionResult> raySphereIntersection(const Ray3D& ray, const Sphere& sphere) {
    // Solve |O + tD - C|^2 = r^2 -> (D·D) t^2 + 2 D·(O-C) t + |O-C|^2 - r^2 = 0
    Vector3 oc = ray.origin - sphere.center;
    float a = ray.direction.dot(ray.direction); // Should be 1 since direction is normalized
    float b = 2.0f * ray.direction.dot(oc);
    float c = oc.dot(oc) - sphere.radius * sphere.radius;
    
    // Solve quadratic equation at^2 + bt + c = 0
    float discriminant = b * b - 4.0f * a * c;
    
    if (discriminant < 0.0f) {
        return std::nullopt; // No intersection
    }
    
    // Compute both intersections
    float sqrtD = std::sqrt(discriminant);
    float t0 = (-b - sqrtD) / (2.0f * a);
    float t1 = (-b + sqrtD) / (2.0f * a);
    
    // Get closest valid intersection
    float t;
    if (t0 >= 0.0f && t1 >= 0.0f) t = std::min(t0, t1);
    else if (t0 >= 0.0f) t = t0;
    else if (t1 >= 0.0f) t = t1;
    else return std::nullopt;
    
    IntersectionResult result;
    result.intersects = true;
    result.distance = t;
    result.point = ray.getPoint(t);
    result.normal = (result.point - sphere.center).normalized();
    
    return result;
}

/**
 * @brief Tests if a ray intersects with an AABB
 * @param ray The ray to test
 * @param aabb The AABB to test against
 * @return Optional intersection result, empty if no intersection
 */
inline std::optional<IntersectionResult> rayAABBIntersection(const Ray3D& ray, const AABB& aabb) {
    Vector3 invDir(
        1.0f / ray.direction.x,
        1.0f / ray.direction.y,
        1.0f / ray.direction.z
    );
    
    Vector3 t1(
        (aabb.min.x - ray.origin.x) * invDir.x,
        (aabb.min.y - ray.origin.y) * invDir.y,
        (aabb.min.z - ray.origin.z) * invDir.z
    );
    Vector3 t2(
        (aabb.max.x - ray.origin.x) * invDir.x,
        (aabb.max.y - ray.origin.y) * invDir.y,
        (aabb.max.z - ray.origin.z) * invDir.z
    );
    
    Vector3 tmin(
        std::min(t1.x, t2.x),
        std::min(t1.y, t2.y),
        std::min(t1.z, t2.z)
    );
    
    Vector3 tmax(
        std::max(t1.x, t2.x),
        std::max(t1.y, t2.y),
        std::max(t1.z, t2.z)
    );
    
    float enterT = std::max(std::max(tmin.x, tmin.y), tmin.z);
    float exitT = std::min(std::min(tmax.x, tmax.y), tmax.z);
    
    if (enterT > exitT || exitT < 0.0f) {
        return std::nullopt;
    }
    
    IntersectionResult result;
    result.intersects = true;
    result.distance = enterT;
    result.point = ray.getPoint(enterT);
    
    // Calculate normal based on which face was hit
    if (enterT == tmin.x) result.normal = Vector3(invDir.x < 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
    else if (enterT == tmin.y) result.normal = Vector3(0.0f, invDir.y < 0.0f ? 1.0f : -1.0f, 0.0f);
    else result.normal = Vector3(0.0f, 0.0f, invDir.z < 0.0f ? 1.0f : -1.0f);
    
    return result;
}

/**
 * @brief Tests if two spheres intersect
 * @param sphere1 First sphere
 * @param sphere2 Second sphere
 * @return Optional intersection result, empty if no intersection
 */
inline std::optional<IntersectionResult> sphereSphereIntersection(const Sphere& sphere1, const Sphere& sphere2) {
    Vector3 toSphere = sphere2.center - sphere1.center;
    float distSq = toSphere.lengthSquared();
    float radiusSum = sphere1.radius + sphere2.radius;
    
    if (distSq > radiusSum * radiusSum) {
        return std::nullopt;
    }
    
    IntersectionResult result;
    result.intersects = true;
    result.distance = std::sqrt(distSq);
    result.normal = toSphere.normalized();
    result.point = sphere1.center + result.normal * sphere1.radius;
    
    return result;
}

/**
 * @brief Tests if two AABBs intersect
 * @param aabb1 First AABB
 * @param aabb2 Second AABB
 * @return Optional intersection result, empty if no intersection
 */
inline std::optional<IntersectionResult> aabbAABBIntersection(const AABB& aabb1, const AABB& aabb2) {
    // Check for no intersection
    if (aabb1.max.x < aabb2.min.x || aabb1.min.x > aabb2.max.x ||
        aabb1.max.y < aabb2.min.y || aabb1.min.y > aabb2.max.y ||
        aabb1.max.z < aabb2.min.z || aabb1.min.z > aabb2.max.z) {
        return std::nullopt;
    }
    
    IntersectionResult result;
    result.intersects = true;
    
    // Calculate intersection box
    Vector3 intersectMin(
        std::max(aabb1.min.x, aabb2.min.x),
        std::max(aabb1.min.y, aabb2.min.y),
        std::max(aabb1.min.z, aabb2.min.z)
    );
    
    Vector3 intersectMax(
        std::min(aabb1.max.x, aabb2.max.x),
        std::min(aabb1.max.y, aabb2.max.y),
        std::min(aabb1.max.z, aabb2.max.z)
    );
    
    // Use center of intersection as contact point
    result.point = (intersectMin + intersectMax) * 0.5f;
    
    // Calculate normal based on smallest penetration axis
    Vector3 penetration = intersectMax - intersectMin;
    if (penetration.x < penetration.y && penetration.x < penetration.z) {
        result.normal = Vector3(1.0f, 0.0f, 0.0f);
        result.distance = penetration.x;
    } else if (penetration.y < penetration.z) {
        result.normal = Vector3(0.0f, 1.0f, 0.0f);
        result.distance = penetration.y;
    } else {
        result.normal = Vector3(0.0f, 0.0f, 1.0f);
        result.distance = penetration.z;
    }
    
    return result;
}

} // namespace geometry
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_INTERSECTION_HPP