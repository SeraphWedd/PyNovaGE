#include "physics/rigid_body.hpp"
#include <cmath>
#include <algorithm>

namespace PyNovaGE {
namespace Physics {

//------------------------------------------------------------------------------
// RigidBody Implementation
//------------------------------------------------------------------------------

RigidBody::RigidBody(std::shared_ptr<CollisionShape> shape, BodyType type)
    : type_(type), collision_shape_(shape) {
    updateMassProperties();
}

void RigidBody::setMass(float mass) {
    if (mass <= 0.0f || type_ == BodyType::Static) {
        mass_ = 0.0f;
        inverse_mass_ = 0.0f;
    } else {
        mass_ = mass;
        inverse_mass_ = 1.0f / mass;
    }
    updateMassProperties();
}

void RigidBody::setInertia(float inertia) {
    if (inertia <= 0.0f || type_ == BodyType::Static) {
        inertia_ = 0.0f;
        inverse_inertia_ = 0.0f;
    } else {
        inertia_ = inertia;
        inverse_inertia_ = 1.0f / inertia;
    }
}

void RigidBody::applyForceAtPoint(const Vector2<float>& force, const Vector2<float>& point) {
    if (type_ != BodyType::Dynamic) return;
    
    accumulated_force_ += force;
    
    // Calculate torque from force at point
    Vector2<float> r = point - position_;
    float torque = PhysicsUtils::cross2D(r, force);
    accumulated_torque_ += torque;
}

AABB<float> RigidBody::getWorldBounds() const {
    return collision_shape_->getBounds(position_);
}

void RigidBody::integrate(float deltaTime) {
    if (type_ != BodyType::Dynamic || !is_awake_) {
        return;
    }
    
    // Apply gravity (if not kinematic)
    if (type_ == BodyType::Dynamic && mass_ > 0.0f) {
        // Gravity is applied by the physics world, not here
    }
    
    // Apply drag
    float drag_factor = std::pow(1.0f - material_.drag, deltaTime);
    linear_velocity_ = linear_velocity_ * drag_factor;
    angular_velocity_ *= drag_factor;
    
    // Integrate forces to velocity (F = ma, so a = F/m)
    if (inverse_mass_ > 0.0f) {
        linear_velocity_ += accumulated_force_ * inverse_mass_ * deltaTime;
    }
    
    if (inverse_inertia_ > 0.0f) {
        angular_velocity_ += accumulated_torque_ * inverse_inertia_ * deltaTime;
    }
    
    // Integrate velocity to position
    position_ += linear_velocity_ * deltaTime;
    rotation_ += angular_velocity_ * deltaTime;
    
    // Clear forces for next frame
    clearForces();
    
    // Update sleep state
    updateSleepState(deltaTime);
}

Vector2<float> RigidBody::getVelocityAtPoint(const Vector2<float>& worldPoint) const {
    Vector2<float> r = worldPoint - position_;
    Vector2<float> tangential_velocity = PhysicsUtils::rotate(Vector2<float>(-r.y, r.x), 0.0f) * angular_velocity_;
    return linear_velocity_ + tangential_velocity;
}

void RigidBody::resolveCollision(const Vector2<float>& normal, float penetration, const Vector2<float>& contactPoint, RigidBody& other) {
    if (type_ == BodyType::Static && other.type_ == BodyType::Static) {
        return; // Two static bodies don't collide
    }
    
    // Calculate relative velocity
    Vector2<float> r1 = contactPoint - position_;
    Vector2<float> r2 = contactPoint - other.position_;
    
    Vector2<float> vel1 = getVelocityAtPoint(contactPoint);
    Vector2<float> vel2 = other.getVelocityAtPoint(contactPoint);
    Vector2<float> relativeVelocity = vel2 - vel1;
    
    // Relative velocity along collision normal
    float velocityAlongNormal = relativeVelocity.dot(normal);
    
    // Don't resolve if objects are separating
    if (velocityAlongNormal > 0.0f) {
        return;
    }
    
    // Calculate restitution
    float restitution = std::min(material_.restitution, other.material_.restitution);
    
    // Calculate impulse scalar
    float j = -(1.0f + restitution) * velocityAlongNormal;
    
    // Calculate mass terms
    float invMassSum = inverse_mass_ + other.inverse_mass_;
    
    // Add rotational components
    float r1CrossN = PhysicsUtils::cross2D(r1, normal);
    float r2CrossN = PhysicsUtils::cross2D(r2, normal);
    invMassSum += r1CrossN * r1CrossN * inverse_inertia_ + r2CrossN * r2CrossN * other.inverse_inertia_;
    
    j /= invMassSum;
    
    // Apply impulse
    Vector2<float> impulse = normal * j;
    
    if (type_ == BodyType::Dynamic) {
        linear_velocity_ -= impulse * inverse_mass_;
        angular_velocity_ -= PhysicsUtils::cross2D(r1, impulse) * inverse_inertia_;
        setAwake(true);
    }
    
    if (other.type_ == BodyType::Dynamic) {
        other.linear_velocity_ += impulse * other.inverse_mass_;
        other.angular_velocity_ += PhysicsUtils::cross2D(r2, impulse) * other.inverse_inertia_;
        other.setAwake(true);
    }
    
    // Friction impulse
    Vector2<float> tangent = relativeVelocity - normal * relativeVelocity.dot(normal);
    float tangentLength = tangent.length();
    
    if (tangentLength > 0.0001f) {
        tangent = tangent / tangentLength;
        
        // Calculate friction impulse
        float jt = -relativeVelocity.dot(tangent);
        jt /= invMassSum;
        
        // Use Coulomb friction model
        float friction = std::sqrt(material_.friction * other.material_.friction);
        Vector2<float> frictionImpulse;
        
        if (std::abs(jt) < j * friction) {
            frictionImpulse = tangent * jt;
        } else {
            frictionImpulse = tangent * (-j * friction);
        }
        
        // Apply friction impulse
        if (type_ == BodyType::Dynamic) {
            linear_velocity_ -= frictionImpulse * inverse_mass_;
            angular_velocity_ -= PhysicsUtils::cross2D(r1, frictionImpulse) * inverse_inertia_;
        }
        
        if (other.type_ == BodyType::Dynamic) {
            other.linear_velocity_ += frictionImpulse * other.inverse_mass_;
            other.angular_velocity_ += PhysicsUtils::cross2D(r2, frictionImpulse) * other.inverse_inertia_;
        }
    }
    
    // Position correction to avoid sinking
    const float CORRECTION_PERCENT = 0.4f;
    const float CORRECTION_THRESHOLD = 0.01f;
    
    if (penetration > CORRECTION_THRESHOLD) {
        Vector2<float> correction = normal * (penetration * CORRECTION_PERCENT / invMassSum);
        
        if (type_ == BodyType::Dynamic) {
            position_ -= correction * inverse_mass_;
        }
        
        if (other.type_ == BodyType::Dynamic) {
            other.position_ += correction * other.inverse_mass_;
        }
    }
}

void RigidBody::updateMassProperties() {
    if (!collision_shape_) return;
    
    if (type_ == BodyType::Static) {
        mass_ = 0.0f;
        inverse_mass_ = 0.0f;
        inertia_ = 0.0f;
        inverse_inertia_ = 0.0f;
    } else {
        // Calculate mass from shape and material
        float calculatedMass = PhysicsUtils::calculateMass(*collision_shape_, material_.density);
        
        if (mass_ <= 0.0f) {
            mass_ = calculatedMass;
        }
        
        inverse_mass_ = (mass_ > 0.0f) ? 1.0f / mass_ : 0.0f;
        
        // Calculate inertia
        inertia_ = PhysicsUtils::calculateInertia(*collision_shape_, mass_);
        inverse_inertia_ = (inertia_ > 0.0f) ? 1.0f / inertia_ : 0.0f;
    }
}

void RigidBody::updateSleepState(float deltaTime) {
    if (type_ != BodyType::Dynamic) return;
    
    float linearKineticEnergy = linear_velocity_.dot(linear_velocity_);
    float angularKineticEnergy = angular_velocity_ * angular_velocity_;
    
    if (linearKineticEnergy < SLEEP_LINEAR_THRESHOLD && angularKineticEnergy < SLEEP_ANGULAR_THRESHOLD) {
        sleep_time_ += deltaTime;
        
        if (sleep_time_ >= SLEEP_TIME_THRESHOLD) {
            setAwake(false);
            linear_velocity_ = Vector2<float>(0.0f);
            angular_velocity_ = 0.0f;
        }
    } else {
        sleep_time_ = 0.0f;
        setAwake(true);
    }
}

//------------------------------------------------------------------------------
// PhysicsUtils Implementation
//------------------------------------------------------------------------------

Vector2<float> PhysicsUtils::rotate(const Vector2<float>& vector, float angle) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    
    return Vector2<float>(
        vector.x * cos_a - vector.y * sin_a,
        vector.x * sin_a + vector.y * cos_a
    );
}

float PhysicsUtils::cross2D(const Vector2<float>& a, const Vector2<float>& b) {
    return a.x * b.y - a.y * b.x;
}

float PhysicsUtils::calculateMass(const CollisionShape& shape, float density) {
    return shape.getArea() * density;
}

float PhysicsUtils::calculateInertia(const CollisionShape& shape, float mass) {
    return shape.getInertia(mass);
}

} // namespace Physics
} // namespace PyNovaGE