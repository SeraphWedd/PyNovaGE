#include "player.hpp"
#include <physics/collision_shapes.hpp>
#include <physics/rigid_body.hpp>
#include <algorithm>
#include <cmath>

Player::Player(PyNovaGE::Scene::EntityID entity_id)
    : entity_id_(entity_id) {}

Player::~Player() = default;

void Player::Initialize(PyNovaGE::Scene::Scene* scene) {
    scene_ = scene;
    // Transform for player starting position - start above the ground platform
    auto& transform = scene->AddComponent<PyNovaGE::Scene::Transform2DComponent>(entity_id_);
    transform.SetPosition(PyNovaGE::Vector2<float>(400.0f, 100.0f)); // Y position higher up from ground
    transform.SetScale(PyNovaGE::Vector2<float>(1.0f, 1.0f));

    // Sprite component
    auto& sprite = scene->AddComponent<PyNovaGE::Scene::SpriteComponent>(entity_id_);
    sprite.size = PyNovaGE::Vector2<float>(40.0f, 80.0f);
    sprite.color = PyNovaGE::Vector4<float>(0.2f, 0.8f, 0.3f, 1.0f); // Greenish for player

    // Rigid body - dynamic box matching sprite size
    auto& body = scene->AddComponent<PyNovaGE::Scene::RigidBody2DComponent>(entity_id_);
    auto shape = std::make_shared<PyNovaGE::Physics::RectangleShape>(sprite.size);
    auto rigid = std::make_shared<PyNovaGE::Physics::RigidBody>(shape, PyNovaGE::Physics::BodyType::Dynamic);
    rigid->setPosition(transform.GetPosition());
    rigid->setLinearVelocity(PyNovaGE::Vector2<float>(0.0f, 0.0f));
    // Slightly reduce bounciness and give some friction
    PyNovaGE::Physics::Material mat;
    mat.density = 1.0f;
    mat.restitution = 0.0f;
    mat.friction = 0.8f;
    mat.drag = 0.02f;
    rigid->setMaterial(mat);
    body.body = rigid;
}

void Player::OnUpdate(float deltaTime) {
    (void)deltaTime;
    // Movement handled via RigidBody2DComponent; input-driven in game loop if needed
}

void Player::OnRender(PyNovaGE::Renderer::SpriteRenderer* renderer) {
    (void)renderer;
    // Rendering handled by scene systems using SpriteComponent
}

void Player::MoveLeft() {
    if (auto* body = scene_->GetComponent<PyNovaGE::Scene::RigidBody2DComponent>(entity_id_)) {
        float force = move_speed_;
        if (!IsGrounded()) force *= 0.5f; // Reduced air control
        
        auto velocity = body->GetLinearVelocity();
        velocity.x = std::max(velocity.x - force, -move_speed_);
        body->SetLinearVelocity(velocity);
        
        facing_right_ = false;
    }
}

void Player::MoveRight() {
    if (auto* body = scene_->GetComponent<PyNovaGE::Scene::RigidBody2DComponent>(entity_id_)) {
        float force = move_speed_;
        if (!IsGrounded()) force *= 0.5f; // Reduced air control
        
        auto velocity = body->GetLinearVelocity();
        velocity.x = std::min(velocity.x + force, move_speed_);
        body->SetLinearVelocity(velocity);
        
        facing_right_ = true;
    }
}

void Player::Jump() {
    if (auto* body = scene_->GetComponent<PyNovaGE::Scene::RigidBody2DComponent>(entity_id_)) {
        if (IsGrounded()) {
            auto velocity = body->GetLinearVelocity();
            velocity.y = jump_force_;
            body->SetLinearVelocity(velocity);
            PlayAudio("jump");
        }
    }
}

void Player::Stop() {
    if (auto* body = scene_->GetComponent<PyNovaGE::Scene::RigidBody2DComponent>(entity_id_)) {
        auto velocity = body->GetLinearVelocity();
        velocity.x = 0.0f;
        body->SetLinearVelocity(velocity);
    }
}

bool Player::IsGrounded() const {
    if (auto* body = scene_->GetComponent<PyNovaGE::Scene::RigidBody2DComponent>(entity_id_)) {
        // Check if velocity is very small and we're near the bottom or a platform
        auto velocity = body->GetLinearVelocity();
        return std::abs(velocity.y) < 0.1f; // Consider grounded if vertical velocity is near zero
    }
    return false;
}

const PyNovaGE::Vector2<float>& Player::GetPosition() const {
    if (scene_) {
        if (auto* transform = scene_->GetComponent<PyNovaGE::Scene::Transform2DComponent>(entity_id_)) {
            return transform->GetPosition();
        }
    }
    static PyNovaGE::Vector2<float> fallback{0.0f, 0.0f};
    return fallback;
}

void Player::UpdateParticles() {}
void Player::PlayAudio(const std::string& soundName) {
    (void)soundName;
}
