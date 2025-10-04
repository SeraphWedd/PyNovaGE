#ifndef PYNOVAGE_MATH_GEOMETRY_CONTINUOUS_COLLISION_HPP
#define PYNOVAGE_MATH_GEOMETRY_CONTINUOUS_COLLISION_HPP

#include "primitives.hpp"
#include "intersection.hpp"
#include <optional>

namespace pynovage {
namespace math {
namespace geometry {

/**
 * @brief Result of a continuous collision test
 * 
 * Extends IntersectionResult to add time of impact
 */
struct ContinuousCollisionResult : public IntersectionResult {
    float timeOfImpact = 1.0f;  // Time of impact (0 to 1, where 1 means no collision in the time step)
};

/**
 * @brief Tests continuous collision between a moving sphere and a static sphere
 * @param movingSphere The sphere being moved
 * @param staticSphere The static sphere to test against
 * @param start Start position of moving sphere's center
 * @param end End position of moving sphere's center
 * @param timeStep Duration of movement
 * @return Optional collision result, empty if no collision
 */
std::optional<ContinuousCollisionResult> testMovingSphereSphere(
    const Sphere& movingSphere,
    const Sphere& staticSphere,
    const Vector3& start,
    const Vector3& end,
    float timeStep);

/**
 * @brief Tests continuous collision between a moving sphere and a static AABB
 * @param sphere The sphere being moved
 * @param aabb The static AABB to test against
 * @param start Start position of sphere's center
 * @param end End position of sphere's center
 * @param timeStep Duration of movement
 * @return Optional collision result, empty if no collision
 */
std::optional<ContinuousCollisionResult> testMovingSphereAABB(
    const Sphere& sphere,
    const AABB& aabb,
    const Vector3& start,
    const Vector3& end,
    float timeStep);

} // namespace geometry
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_CONTINUOUS_COLLISION_HPP