#include "platform.hpp"
#include <physics/collision_shapes.hpp>

Platform::Platform(PyNovaGE::Scene::EntityID entity_id,
                  const PyNovaGE::Vector2<float>& position,
                  const PyNovaGE::Vector2<float>& size)
    : entity_id_(entity_id)
    , position_(position)
    , size_(size)
{
}

Platform::~Platform() = default;

void Platform::Initialize(PyNovaGE::Scene::Scene* scene) {
    // Add transform component
    auto& transform = scene->AddComponent<PyNovaGE::Scene::Transform2DComponent>(entity_id_);
    transform.SetPosition(position_);

    // Add sprite component
    auto& sprite_comp = scene->AddComponent<PyNovaGE::Scene::SpriteComponent>(entity_id_);
    sprite_comp.size = size_;
    sprite_comp.color = PyNovaGE::Vector4<float>(0.8f, 0.3f, 0.2f, 1.0f); // Reddish for platforms

    // Add rigid body component with static box shape
    auto& body_comp = scene->AddComponent<PyNovaGE::Scene::RigidBody2DComponent>(entity_id_);
    auto shape = std::make_shared<PyNovaGE::Physics::RectangleShape>(size_);
    auto rigid = std::make_shared<PyNovaGE::Physics::RigidBody>(shape, PyNovaGE::Physics::BodyType::Static);
    rigid->setPosition(transform.GetPosition());
    body_comp.body = rigid;
    body_comp.SetRotation(0.0f);
}

void Platform::OnRender(PyNovaGE::Renderer::SpriteRenderer* renderer) {
    (void)renderer;
    // Rendering is handled by the scene's sprite component system
}

const PyNovaGE::Vector2<float>& Platform::GetPosition() const {
    // TODO: Get position from transform component via scene
    static PyNovaGE::Vector2<float> pos{0.0f, 0.0f};
    return pos;
}

const PyNovaGE::Vector2<float>& Platform::GetSize() const {
    return size_;
}
