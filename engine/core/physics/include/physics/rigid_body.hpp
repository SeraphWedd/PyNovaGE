#pragma once

#include "collision_shapes.hpp"
#include "vectors/vectors.hpp"
#include <memory>

namespace PyNovaGE {
namespace Physics {

/**
 * @brief Body types for physics simulation
 */
enum class BodyType {
    Static,    // Never moves (e.g., walls, platforms)
    Kinematic, // Moved by animation/script, not physics
    Dynamic    // Affected by forces and collisions
};

/**
 * @brief Material properties for physics bodies
 */
struct Material {
    float density = 1.0f;      // kg/m^2 for 2D
    float restitution = 0.3f;  // Bounciness (0 = perfectly inelastic, 1 = perfectly elastic)
    float friction = 0.5f;     // Surface friction coefficient
    float drag = 0.01f;        // Air/fluid drag coefficient
};

/**
 * @brief 2D Rigid Body for physics simulation
 * 
 * This class represents a physical object that can participate in collision detection
 * and physics simulation. It integrates with your existing SIMD math foundation.
 */
class RigidBody {
public:
    RigidBody(std::shared_ptr<CollisionShape> shape, BodyType type = BodyType::Dynamic);
    ~RigidBody() = default;

    // Basic properties
    void setPosition(const Vector2<float>& position) { position_ = position; }
    const Vector2<float>& getPosition() const { return position_; }
    
    void setRotation(float rotation) { rotation_ = rotation; }
    float getRotation() const { return rotation_; }
    
    void setBodyType(BodyType type) { type_ = type; updateMassProperties(); }
    BodyType getBodyType() const { return type_; }

    // Physics properties
    void setMass(float mass);
    float getMass() const { return mass_; }
    float getInverseMass() const { return inverse_mass_; }
    
    void setInertia(float inertia);
    float getInertia() const { return inertia_; }
    float getInverseInertia() const { return inverse_inertia_; }

    // Velocity and motion
    void setLinearVelocity(const Vector2<float>& velocity) { linear_velocity_ = velocity; }
    const Vector2<float>& getLinearVelocity() const { return linear_velocity_; }
    
    void setAngularVelocity(float velocity) { angular_velocity_ = velocity; }
    float getAngularVelocity() const { return angular_velocity_; }

    // Forces and impulses
    void applyForce(const Vector2<float>& force) { accumulated_force_ += force; }
    void applyForceAtPoint(const Vector2<float>& force, const Vector2<float>& point);
    void applyImpulse(const Vector2<float>& impulse) { linear_velocity_ += impulse * inverse_mass_; }
    void applyAngularImpulse(float impulse) { angular_velocity_ += impulse * inverse_inertia_; }
    
    void clearForces() { accumulated_force_ = Vector2<float>(0.0f); accumulated_torque_ = 0.0f; }

    // Material properties
    void setMaterial(const Material& material) { material_ = material; updateMassProperties(); }
    const Material& getMaterial() const { return material_; }

    // Collision shape
    const CollisionShape& getCollisionShape() const { return *collision_shape_; }
    std::shared_ptr<CollisionShape> getCollisionShapePtr() const { return collision_shape_; }
    void setCollisionShape(std::shared_ptr<CollisionShape> shape) { 
        collision_shape_ = shape; 
        updateMassProperties(); 
    }

    // World space bounds (uses existing SIMD AABB system)
    AABB<float> getWorldBounds() const;

    // Integration (called by physics world)
    void integrate(float deltaTime);

    // Collision response helpers
    Vector2<float> getVelocityAtPoint(const Vector2<float>& worldPoint) const;
    void resolveCollision(const Vector2<float>& normal, float penetration, const Vector2<float>& contactPoint, RigidBody& other);

    // State flags
    void setActive(bool active) { is_active_ = active; }
    bool isActive() const { return is_active_; }
    
    void setAwake(bool awake) { is_awake_ = awake; if (awake) sleep_time_ = 0.0f; }
    bool isAwake() const { return is_awake_; }

    // Debug/utility
    bool isStatic() const { return type_ == BodyType::Static; }
    bool isKinematic() const { return type_ == BodyType::Kinematic; }
    bool isDynamic() const { return type_ == BodyType::Dynamic; }

private:
    // Transform
    Vector2<float> position_{0.0f};
    float rotation_ = 0.0f;

    // Physics properties
    BodyType type_;
    float mass_ = 1.0f;
    float inverse_mass_ = 1.0f;
    float inertia_ = 1.0f;
    float inverse_inertia_ = 1.0f;

    // Motion state
    Vector2<float> linear_velocity_{0.0f};
    float angular_velocity_ = 0.0f;

    // Forces
    Vector2<float> accumulated_force_{0.0f};
    float accumulated_torque_ = 0.0f;

    // Material
    Material material_;

    // Collision
    std::shared_ptr<CollisionShape> collision_shape_;

    // State management
    bool is_active_ = true;
    bool is_awake_ = true;
    float sleep_time_ = 0.0f;

    // Internal methods
    void updateMassProperties();
    void updateSleepState(float deltaTime);
    
    // Physics constants
    static constexpr float SLEEP_LINEAR_THRESHOLD = 0.01f;
    static constexpr float SLEEP_ANGULAR_THRESHOLD = 0.01f;
    static constexpr float SLEEP_TIME_THRESHOLD = 0.5f;
};

/**
 * @brief Physics utility functions
 */
namespace PhysicsUtils {
    
    // Convert between angle and rotation matrix (for 2D rotation)
    Vector2<float> rotate(const Vector2<float>& vector, float angle);
    float cross2D(const Vector2<float>& a, const Vector2<float>& b);
    
    // Calculate mass properties from shape and material
    float calculateMass(const CollisionShape& shape, float density);
    float calculateInertia(const CollisionShape& shape, float mass);
}

} // namespace Physics
} // namespace PyNovaGE