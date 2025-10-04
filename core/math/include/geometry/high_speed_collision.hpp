#ifndef PYNOVAGE_MATH_GEOMETRY_HIGH_SPEED_COLLISION_HPP
#define PYNOVAGE_MATH_GEOMETRY_HIGH_SPEED_COLLISION_HPP

#include "primitives.hpp"
#include "intersection.hpp"
#include "continuous_collision.hpp"
#include "../simd_utils.hpp"

namespace pynovage {
namespace math {
namespace geometry {

/**
 * @brief Result of a penetration test containing entry/exit points and surface information
 */
struct PenetrationResult {
    bool penetrated = false;      // Whether penetration occurred
    Vector3 entry_point;          // Point where penetration begins
    Vector3 exit_point;           // Point where penetration ends
    float thickness = 0.0f;       // Thickness of the penetrated object along the path
    Vector3 surface_normal;       // Surface normal at entry point
};

/**
 * @brief Parameters for penetration testing of high-velocity objects
 */
struct PenetrationTestParams {
    Vector3 ray_origin;          // Starting point of the projectile
    Vector3 ray_direction;       // Normalized direction of travel
    float velocity;              // Speed of the projectile (units/second)
    float projectile_radius;     // Radius of the projectile
};

/**
 * @brief Tests if a high-velocity projectile penetrates a sphere
 * 
 * Used for high-speed collision detection where standard continuous collision
 * detection might miss interactions. Accounts for object thickness and implements
 * bullet penetration mathematics.
 * 
 * @param sphere Target sphere to test penetration against
 * @param params Parameters describing the projectile's motion
 * @return PenetrationResult with entry/exit points if penetration occurs
 */
PenetrationResult TestSpherePenetration(
    const Sphere& sphere,
    const PenetrationTestParams& params);

/**
 * @brief Tests if a high-velocity projectile penetrates an AABB
 * 
 * Similar to sphere penetration but optimized for axis-aligned boxes.
 * Uses slab method for intersection and accounts for variable thickness
 * based on entry angle.
 * 
 * @param box Target AABB to test penetration against
 * @param params Parameters describing the projectile's motion
 * @return PenetrationResult with entry/exit points if penetration occurs
 */
PenetrationResult TestAABBPenetration(
    const AABB& box,
    const PenetrationTestParams& params);

} // namespace geometry
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_HIGH_SPEED_COLLISION_HPP