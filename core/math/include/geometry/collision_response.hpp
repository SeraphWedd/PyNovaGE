#ifndef PYNOVAGE_MATH_GEOMETRY_COLLISION_RESPONSE_HPP
#define PYNOVAGE_MATH_GEOMETRY_COLLISION_RESPONSE_HPP

#include "primitives.hpp"
#include "intersection.hpp"
#include "../vector3.hpp"
#include "../matrix3.hpp"
#include "../quaternion.hpp"

namespace pynovage {
namespace math {
namespace geometry {

/**
 * @brief Physical properties of a material affecting collision response
 */
struct MaterialProperties {
    float restitution = 0.5f;     // Coefficient of restitution (bounciness)
    float friction = 0.5f;        // Coefficient of friction
    float density = 1.0f;         // Mass per unit volume
};

/**
 * @brief Physical properties of a rigid body
 */
struct RigidBodyProperties {
    float mass = 1.0f;                     // Mass of the body
    Matrix3x3 inertia_tensor;              // Inertia tensor (for rotational dynamics)
    Matrix3x3 inverse_inertia_tensor;      // Inverse of inertia tensor
    MaterialProperties material;           // Material properties

    // Velocities
    Vector3 linear_velocity;             // Linear velocity
    Vector3 angular_velocity;            // Angular velocity (in radians/second)
    
    // Constructor for common shapes with uniform density
    static RigidBodyProperties forSphere(float radius, const MaterialProperties& material);
    static RigidBodyProperties forBox(const Vector3& dimensions, const MaterialProperties& material);
};

/**
 * @brief Result of an impulse-based collision response calculation
 */
struct CollisionResponse {
    Vector3 linear_impulse;          // Magnitude and direction of impulse (along normal)
    Vector3 angular_impulse;         // Angular impulse computed for body 1 contact arm
    Vector3 friction_impulse;        // Tangential impulse
    Vector3 normal;                  // Contact normal (used to determine impulse sign per body)
    float energy_loss = 0.0f;        // Energy dissipated in collision
};

/**
 * @brief Calculates collision response between two spheres
 * @param sphere1 First sphere
 * @param sphere2 Second sphere
 * @param props1 Physical properties of first sphere
 * @param props2 Physical properties of second sphere
 * @param contact Contact point information
 * @return Collision response data
 */
CollisionResponse calculateSphereResponse(
    const Sphere& sphere1,
    const Sphere& sphere2,
    const RigidBodyProperties& props1,
    const RigidBodyProperties& props2,
    const IntersectionResult& contact
);

/**
 * @brief Calculates collision response between a sphere and an AABB
 * @param sphere The sphere
 * @param box The AABB
 * @param sphere_props Physical properties of the sphere
 * @param box_props Physical properties of the box
 * @param contact Contact point information
 * @return Collision response data
 */
CollisionResponse calculateSphereBoxResponse(
    const Sphere& sphere,
    const AABB& box,
    const RigidBodyProperties& sphere_props,
    const RigidBodyProperties& box_props,
    const IntersectionResult& contact
);

/**
 * @brief Calculates collision response between two AABBs
 * @param box1 First AABB
 * @param box2 Second AABB
 * @param props1 Physical properties of first box
 * @param props2 Physical properties of second box
 * @param contact Contact point information
 * @return Collision response data
 */
CollisionResponse calculateBoxResponse(
    const AABB& box1,
    const AABB& box2,
    const RigidBodyProperties& props1,
    const RigidBodyProperties& props2,
    const IntersectionResult& contact
);

/**
 * @brief Applies collision response to update object velocities
 * @param response The calculated collision response
 * @param props Properties to update
 * @param dt Time step
 */
void applyCollisionResponse(
    const CollisionResponse& response,
    RigidBodyProperties& props,
    float dt
);

} // namespace geometry
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_COLLISION_RESPONSE_HPP