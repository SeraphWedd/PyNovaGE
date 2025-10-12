#include "scene/components.hpp"
#include "physics/rigid_body.hpp"

namespace PyNovaGE {
namespace Scene {

void RigidBody2DComponent::SetPosition(const Vector2f& position) {
    if (body) {
        body->setPosition(position);
    }
}

Vector2f RigidBody2DComponent::GetPosition() const {
    if (body) {
        return body->getPosition();
    }
    return Vector2f{0.0f, 0.0f};
}

void RigidBody2DComponent::SetRotation(float rotation) {
    if (body) {
        body->setRotation(rotation);
    }
}

float RigidBody2DComponent::GetRotation() const {
    if (body) {
        return body->getRotation();
    }
    return 0.0f;
}

void RigidBody2DComponent::SetLinearVelocity(const Vector2f& velocity) {
    if (body) {
        body->setLinearVelocity(velocity);
    }
}

Vector2f RigidBody2DComponent::GetLinearVelocity() const {
    if (body) {
        return body->getLinearVelocity();
    }
    return Vector2f{0.0f, 0.0f};
}

void RigidBody2DComponent::SetAngularVelocity(float velocity) {
    if (body) {
        body->setAngularVelocity(velocity);
    }
}

float RigidBody2DComponent::GetAngularVelocity() const {
    if (body) {
        return body->getAngularVelocity();
    }
    return 0.0f;
}

void RigidBody2DComponent::ApplyForce(const Vector2f& force) {
    if (body) {
        body->applyForce(force);
    }
}

void RigidBody2DComponent::ApplyImpulse(const Vector2f& impulse) {
    if (body) {
        body->applyImpulse(impulse);
    }
}

} // namespace Scene
} // namespace PyNovaGE