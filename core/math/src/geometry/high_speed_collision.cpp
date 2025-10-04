#include "geometry/high_speed_collision.hpp"
#include "geometry/intersection.hpp"
#include "../../include/math_constants.hpp"
#include <limits>

namespace pynovage {
namespace math {
namespace geometry {

namespace {

/**
 * @brief Calculates thickness along a penetration path
 * @param entry Entry point of penetration
 * @param exit Exit point of penetration
 * @return Thickness along penetration path
 */
float CalculateThickness(const Vector3& entry, const Vector3& exit) {
    return (exit - entry).length();
}

/**
 * @brief Validates if penetration is physically possible given thickness and velocity
 * @param thickness Object thickness along penetration path
 * @param velocity Object velocity
 * @return true if penetration is valid
 */
bool ValidatePenetration(float thickness, float velocity) {
    // For now, simple validation based on minimum thickness and maximum expected velocity
    // In a real implementation, this would consider material properties and physics calculations
    return thickness > constants::epsilon && velocity > 0.0f;
}

} // anonymous namespace

PenetrationResult TestSpherePenetration(
    const Sphere& sphere,
    const PenetrationTestParams& params) 
{
    PenetrationResult result;
    
    // First, get the base ray intersection
    Ray3D ray{params.ray_origin, params.ray_direction};
    auto intersection = raySphereIntersection(ray, sphere);
    
    if (!intersection) {
        return result; // No penetration
    }
    
    // Get second intersection point (exit point)
    float t0 = intersection->distance;
    Vector3 entry = intersection->point;
    Vector3 entry_normal = intersection->normal;
    
    // Create reversed ray from just after entry point
    Ray3D reverse_ray{
        entry + params.ray_direction * constants::epsilon,
        params.ray_direction
    };
    
    auto exit_intersection = raySphereIntersection(reverse_ray, sphere);
    if (!exit_intersection) {
        return result; // Should never happen for a sphere
    }
    
    Vector3 exit = exit_intersection->point;
    float thickness = CalculateThickness(entry, exit);
    
    // Validate the penetration is physically possible
    if (!ValidatePenetration(thickness, params.velocity)) {
        return result;
    }
    
    // Fill out the result
    result.penetrated = true;
    result.entry_point = entry;
    result.exit_point = exit;
    result.thickness = thickness;
    result.surface_normal = entry_normal;
    
    return result;
}

PenetrationResult TestAABBPenetration(
    const AABB& box,
    const PenetrationTestParams& params)
{
    PenetrationResult result;

    // Normalize the direction; early out if invalid
    Vector3 dir = params.ray_direction.normalized();
    if (dir.lengthSquared() < constants::epsilon) {
        return result;
    }


    // Compute slab intersections against the original box to get entry/exit
    float t_near = 0.0f;
    float t_far = std::numeric_limits<float>::infinity();
    float eps = constants::epsilon;

    auto process_axis = [&](float o, float d, float minv, float maxv) -> bool {
        if (std::abs(d) < eps) {
            // Parallel to slab: must already be within
            return (o >= minv - eps && o <= maxv + eps);
        }
        float t1 = (minv - o) / d;
        float t2 = (maxv - o) / d;
        if (t1 > t2) std::swap(t1, t2);
        t_near = std::max(t_near, t1);
        t_far = std::min(t_far, t2);
        return (t_near <= t_far && t_far >= 0.0f);
    };

    if (!process_axis(params.ray_origin.x, dir.x, box.min.x, box.max.x)) return result;
    if (!process_axis(params.ray_origin.y, dir.y, box.min.y, box.max.y)) return result;
    if (!process_axis(params.ray_origin.z, dir.z, box.min.z, box.max.z)) return result;

    // Entry/exit points on original box
    Vector3 entry_point = params.ray_origin + dir * t_near;
    Vector3 exit_point = params.ray_origin + dir * t_far;

    // Determine surface normal by which axis produced t_near
    Vector3 normal(0.0f, 0.0f, 0.0f);
    // Recompute t1 per axis for comparison
    if (std::abs(dir.x) > eps) {
        float tx1 = (box.min.x - params.ray_origin.x) / dir.x;
        float tx2 = (box.max.x - params.ray_origin.x) / dir.x;
        if (tx1 > tx2) std::swap(tx1, tx2);
        if (std::abs(t_near - tx1) < eps) normal = Vector3(dir.x > 0.0f ? -1.0f : 1.0f, 0.0f, 0.0f);
    }
    if (normal.isZero() && std::abs(dir.y) > eps) {
        float ty1 = (box.min.y - params.ray_origin.y) / dir.y;
        float ty2 = (box.max.y - params.ray_origin.y) / dir.y;
        if (ty1 > ty2) std::swap(ty1, ty2);
        if (std::abs(t_near - ty1) < eps) normal = Vector3(0.0f, dir.y > 0.0f ? -1.0f : 1.0f, 0.0f);
    }
    if (normal.isZero() && std::abs(dir.z) > eps) {
        float tz1 = (box.min.z - params.ray_origin.z) / dir.z;
        float tz2 = (box.max.z - params.ray_origin.z) / dir.z;
        if (tz1 > tz2) std::swap(tz1, tz2);
        if (std::abs(t_near - tz1) < eps) normal = Vector3(0.0f, 0.0f, dir.z > 0.0f ? -1.0f : 1.0f);
    }

    float thickness = CalculateThickness(entry_point, exit_point);
    if (!ValidatePenetration(thickness, params.velocity)) {
        return result;
    }

    result.penetrated = true;
    result.entry_point = entry_point;
    result.exit_point = exit_point;
    result.thickness = thickness;
    result.surface_normal = normal;

    return result;
}

} // namespace geometry
} // namespace math
} // namespace pynovage