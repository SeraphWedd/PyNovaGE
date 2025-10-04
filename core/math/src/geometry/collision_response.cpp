#include "geometry/collision_response.hpp"
#include "../../include/math_constants.hpp"

namespace pynovage {
namespace math {
namespace geometry {

RigidBodyProperties RigidBodyProperties::forSphere(float radius, const MaterialProperties& material) {
    RigidBodyProperties props;
    props.material = material;
    
    // Calculate mass from density and volume
    float volume = (4.0f / 3.0f) * constants::pi * radius * radius * radius;
    props.mass = volume * material.density;
    
    // For a solid sphere, inertia tensor is diagonal with I = 2/5 * m * r^2
    float inertia = (2.0f / 5.0f) * props.mass * radius * radius;
    props.inertia_tensor = Matrix3::identity() * inertia;
    props.inverse_inertia_tensor = Matrix3::identity() * (1.0f / inertia);
    
    return props;
}

RigidBodyProperties RigidBodyProperties::forBox(const Vector3& dimensions, const MaterialProperties& material) {
    RigidBodyProperties props;
    props.material = material;
    
    // Calculate mass from density and volume
    float volume = dimensions.x * dimensions.y * dimensions.z;
    props.mass = volume * material.density;
    
    // For a box, inertia tensor is diagonal with different values for each axis
    float x2 = dimensions.x * dimensions.x;
    float y2 = dimensions.y * dimensions.y;
    float z2 = dimensions.z * dimensions.z;
    
    // I = m/12 * (y^2 + z^2) for x axis, cyclic for others
    float ix = props.mass / 12.0f * (y2 + z2);
    float iy = props.mass / 12.0f * (x2 + z2);
    float iz = props.mass / 12.0f * (x2 + y2);
    
    props.inertia_tensor = Matrix3(
        ix, 0.0f, 0.0f,
        0.0f, iy, 0.0f,
        0.0f, 0.0f, iz
    );
    
    // Inverse tensor (just reciprocal for diagonal matrix)
    props.inverse_inertia_tensor = Matrix3(
        1.0f/ix, 0.0f, 0.0f,
        0.0f, 1.0f/iy, 0.0f,
        0.0f, 0.0f, 1.0f/iz
    );
    
    return props;
}

CollisionResponse calculateSphereResponse(
    const Sphere& sphere1,
    const Sphere& sphere2,
    const RigidBodyProperties& props1,
    const RigidBodyProperties& props2,
    const IntersectionResult& contact)
{
    CollisionResponse response;
    
    // Calculate relative velocity at contact point
    Vector3 r1 = contact.point - sphere1.center;
    Vector3 r2 = contact.point - sphere2.center;
    
    Vector3 v1 = props1.linear_velocity + props1.angular_velocity.cross(r1);
    Vector3 v2 = props2.linear_velocity + props2.angular_velocity.cross(r2);
    Vector3 relative_velocity = v1 - v2;
    
    // If objects are moving apart, no response needed
    float normal_velocity = relative_velocity.dot(contact.normal);
    if (normal_velocity > 0) {
        return response;
    }
    
    // Calculate coefficients
    float restitution = std::min(props1.material.restitution, props2.material.restitution);
    float friction = std::sqrt(props1.material.friction * props2.material.friction);
    
    // Calculate effective mass
    float inv_mass1 = 1.0f / props1.mass;
    float inv_mass2 = 1.0f / props2.mass;
    
    Vector3 r1_cross_n = r1.cross(contact.normal);
    Vector3 r2_cross_n = r2.cross(contact.normal);
    
    Vector3 ang_term1 = (props1.inverse_inertia_tensor * r1_cross_n).cross(r1);
    Vector3 ang_term2 = (props2.inverse_inertia_tensor * r2_cross_n).cross(r2);
    
    float effective_mass = inv_mass1 + inv_mass2 +
        contact.normal.dot(ang_term1 + ang_term2);
    
    // Calculate impulse magnitude
    float j = -(1.0f + restitution) * normal_velocity / effective_mass;
    
    // Calculate linear and angular impulses
    response.linear_impulse = contact.normal * j;
    response.angular_impulse = r1.cross(response.linear_impulse);
    
    // Calculate friction impulse
    Vector3 tangent_velocity = relative_velocity - (contact.normal * normal_velocity);
    float tangent_speed = tangent_velocity.length();
    
    if (tangent_speed > constants::epsilon) {
        Vector3 tangent_direction = tangent_velocity / tangent_speed;
        float friction_impulse = -friction * j;
        response.friction_impulse = tangent_direction * friction_impulse;
    }
    
    // Calculate energy loss
    response.energy_loss = 0.5f * j * normal_velocity * (1.0f - restitution * restitution);
    
    return response;
}

CollisionResponse calculateSphereBoxResponse(
    const Sphere& sphere,
    const AABB& box,
    const RigidBodyProperties& sphere_props,
    const RigidBodyProperties& box_props,
    const IntersectionResult& contact)
{
    CollisionResponse response;
    
    // Similar to sphere-sphere but with box inertia tensor
    Vector3 r1 = contact.point - sphere.center;
    Vector3 r2 = contact.point - box.center();
    
    Vector3 v1 = sphere_props.linear_velocity + sphere_props.angular_velocity.cross(r1);
    Vector3 v2 = box_props.linear_velocity + box_props.angular_velocity.cross(r2);
    Vector3 relative_velocity = v1 - v2;
    
    float normal_velocity = relative_velocity.dot(contact.normal);
    if (normal_velocity > 0) {
        return response;
    }
    
    float restitution = std::min(sphere_props.material.restitution, box_props.material.restitution);
    float friction = std::sqrt(sphere_props.material.friction * box_props.material.friction);
    
    float inv_mass1 = 1.0f / sphere_props.mass;
    float inv_mass2 = 1.0f / box_props.mass;
    
    Vector3 r1_cross_n = r1.cross(contact.normal);
    Vector3 r2_cross_n = r2.cross(contact.normal);
    
    Vector3 ang_term1 = (sphere_props.inverse_inertia_tensor * r1_cross_n).cross(r1);
    Vector3 ang_term2 = (box_props.inverse_inertia_tensor * r2_cross_n).cross(r2);
    
    float effective_mass = inv_mass1 + inv_mass2 +
        contact.normal.dot(ang_term1 + ang_term2);
    
    float j = -(1.0f + restitution) * normal_velocity / effective_mass;
    
    response.linear_impulse = contact.normal * j;
    response.angular_impulse = r1.cross(response.linear_impulse);
    
    Vector3 tangent_velocity = relative_velocity - (contact.normal * normal_velocity);
    float tangent_speed = tangent_velocity.length();
    
    if (tangent_speed > constants::epsilon) {
        Vector3 tangent_direction = tangent_velocity / tangent_speed;
        float friction_impulse = -friction * j;
        response.friction_impulse = tangent_direction * friction_impulse;
    }
    
    response.energy_loss = 0.5f * j * normal_velocity * (1.0f - restitution * restitution);
    
    return response;
}

CollisionResponse calculateBoxResponse(
    const AABB& box1,
    const AABB& box2,
    const RigidBodyProperties& props1,
    const RigidBodyProperties& props2,
    const IntersectionResult& contact)
{
    CollisionResponse response;
    
    // Similar to sphere-sphere but with box inertia tensors
    Vector3 r1 = contact.point - box1.center();
    Vector3 r2 = contact.point - box2.center();
    
    Vector3 v1 = props1.linear_velocity + props1.angular_velocity.cross(r1);
    Vector3 v2 = props2.linear_velocity + props2.angular_velocity.cross(r2);
    Vector3 relative_velocity = v1 - v2;
    
    float normal_velocity = relative_velocity.dot(contact.normal);
    if (normal_velocity > 0) {
        return response;
    }
    
    float restitution = std::min(props1.material.restitution, props2.material.restitution);
    float friction = std::sqrt(props1.material.friction * props2.material.friction);
    
    float inv_mass1 = 1.0f / props1.mass;
    float inv_mass2 = 1.0f / props2.mass;
    
    Vector3 r1_cross_n = r1.cross(contact.normal);
    Vector3 r2_cross_n = r2.cross(contact.normal);
    
    Vector3 ang_term1 = (props1.inverse_inertia_tensor * r1_cross_n).cross(r1);
    Vector3 ang_term2 = (props2.inverse_inertia_tensor * r2_cross_n).cross(r2);
    
    float effective_mass = inv_mass1 + inv_mass2 +
        contact.normal.dot(ang_term1 + ang_term2);
    
    float j = -(1.0f + restitution) * normal_velocity / effective_mass;
    
    response.linear_impulse = contact.normal * j;
    response.angular_impulse = r1.cross(response.linear_impulse);
    
    Vector3 tangent_velocity = relative_velocity - (contact.normal * normal_velocity);
    float tangent_speed = tangent_velocity.length();
    
    if (tangent_speed > constants::epsilon) {
        Vector3 tangent_direction = tangent_velocity / tangent_speed;
        float friction_impulse = -friction * j;
        response.friction_impulse = tangent_direction * friction_impulse;
    }
    
    response.energy_loss = 0.5f * j * normal_velocity * (1.0f - restitution * restitution);
    
    return response;
}

void applyCollisionResponse(
    const CollisionResponse& response,
    RigidBodyProperties& props,
    float dt)
{
    // Apply linear impulse
    Vector3 delta_v = response.linear_impulse * (1.0f / props.mass);
    props.linear_velocity += delta_v;
    
    // Apply angular impulse
    Vector3 delta_w = props.inverse_inertia_tensor * response.angular_impulse;
    props.angular_velocity += delta_w;
    
    // Apply friction impulse
    props.linear_velocity += response.friction_impulse * (1.0f / props.mass);
}

} // namespace geometry
} // namespace math
} // namespace pynovage