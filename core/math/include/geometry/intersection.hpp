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
    Vector3 oc = ray.origin - sphere.center;
    float radiusSq = sphere.radius * sphere.radius;
    
    // Early rejection: if ray origin is outside sphere and pointing away from it
    float ocLenSq = oc.lengthSquared();
    if (ocLenSq > radiusSq) {
        float dirDotOC = ray.direction.dot(oc);
        if (dirDotOC >= 0.0f) {
            return std::nullopt;
        }
    }
    
    // Using optimized quadratic equation solution since ray direction is normalized (a = 1)
    float b = ray.direction.dot(oc); // No need for 2.0f * since we'll compensate in discriminant
    float c = ocLenSq - radiusSq;
    float discriminant = b * b - c; // Since a = 1, simplified from b^2 - 4ac
    
    if (discriminant < 0.0f) {
        return std::nullopt;
    }
    
    // Only compute closest intersection point
    float sqrtD = std::sqrt(discriminant);
    float t = -b - sqrtD; // Start with closer intersection
    
    // If closer intersection is behind ray, try farther one
    if (t < 0.0f) {
        t = -b + sqrtD;
        if (t < 0.0f) {
            return std::nullopt;
        }
    }
    
    // Compute intersection details
    Vector3 point = ray.getPoint(t);
    Vector3 normal = (point - sphere.center);
    float invRadius = 1.0f / sphere.radius; // Avoid division in normalization
    normal *= invRadius; // Faster than normalized() since we know the length
    
    IntersectionResult result;
    result.intersects = true;
    result.distance = t;
    result.point = point;
    result.normal = normal;
    
    return result;
}

/**
 * @brief Tests if a ray intersects with an AABB
 * @param ray The ray to test
 * @param aabb The AABB to test against
 * @return Optional intersection result, empty if no intersection
 */
inline std::optional<IntersectionResult> rayAABBIntersection(const Ray3D& ray, const AABB& aabb) {
    // Robust slab method with epsilon handling and per-axis logic
    float t_near = 0.0f;
    float t_far = std::numeric_limits<float>::infinity();
    float eps = constants::epsilon;

    // Store entry times per axis for normal computation
    float t_min_axis[3] = { -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() };

    // X axis
    if (std::abs(ray.direction.x) < eps) {
        if (ray.origin.x < aabb.min.x - eps || ray.origin.x > aabb.max.x + eps) {
            return std::nullopt;
        }
    } else {
        float t1 = (aabb.min.x - ray.origin.x) / ray.direction.x;
        float t2 = (aabb.max.x - ray.origin.x) / ray.direction.x;
        if (t1 > t2) std::swap(t1, t2);
        t_min_axis[0] = t1;
        t_near = std::max(t_near, t1);
        t_far = std::min(t_far, t2);
        if (t_near > t_far || t_far < 0.0f) return std::nullopt;
    }

    // Y axis
    if (std::abs(ray.direction.y) < eps) {
        if (ray.origin.y < aabb.min.y - eps || ray.origin.y > aabb.max.y + eps) {
            return std::nullopt;
        }
    } else {
        float t1 = (aabb.min.y - ray.origin.y) / ray.direction.y;
        float t2 = (aabb.max.y - ray.origin.y) / ray.direction.y;
        if (t1 > t2) std::swap(t1, t2);
        t_min_axis[1] = t1;
        t_near = std::max(t_near, t1);
        t_far = std::min(t_far, t2);
        if (t_near > t_far || t_far < 0.0f) return std::nullopt;
    }

    // Z axis
    if (std::abs(ray.direction.z) < eps) {
        if (ray.origin.z < aabb.min.z - eps || ray.origin.z > aabb.max.z + eps) {
            return std::nullopt;
        }
    } else {
        float t1 = (aabb.min.z - ray.origin.z) / ray.direction.z;
        float t2 = (aabb.max.z - ray.origin.z) / ray.direction.z;
        if (t1 > t2) std::swap(t1, t2);
        t_min_axis[2] = t1;
        t_near = std::max(t_near, t1);
        t_far = std::min(t_far, t2);
        if (t_near > t_far || t_far < 0.0f) return std::nullopt;
    }

    // If we reach here, we have an intersection
    IntersectionResult result;
    result.intersects = true;
    result.distance = t_near;
    result.point = ray.getPoint(t_near);

    // Determine normal by matching the axis whose entry time equals t_near (within epsilon)
    if (std::abs(ray.direction.x) > eps && std::abs(t_near - t_min_axis[0]) < eps) {
        result.normal = Vector3(ray.direction.x > 0.0f ? -1.0f : 1.0f, 0.0f, 0.0f);
    } else if (std::abs(ray.direction.y) > eps && std::abs(t_near - t_min_axis[1]) < eps) {
        result.normal = Vector3(0.0f, ray.direction.y > 0.0f ? -1.0f : 1.0f, 0.0f);
    } else {
        result.normal = Vector3(0.0f, 0.0f, ray.direction.z > 0.0f ? -1.0f : 1.0f);
    }

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