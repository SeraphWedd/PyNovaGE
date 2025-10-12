#pragma once

#include <scene/components.hpp>
#include <scene/scene.hpp>
#include <physics/rigid_body.hpp>
#include <renderer/sprite_renderer.hpp>
#include <physics/physics_world.hpp>

class Platform : public PyNovaGE::Scene::Component {
public:
    explicit Platform(PyNovaGE::Scene::EntityID entity_id,
             const PyNovaGE::Vector2<float>& position,
             const PyNovaGE::Vector2<float>& size);
    ~Platform();

    void Initialize(PyNovaGE::Scene::Scene* scene);
    void OnRender(PyNovaGE::Renderer::SpriteRenderer* renderer);
    
    // Platform properties
    const PyNovaGE::Vector2<float>& GetPosition() const;
    const PyNovaGE::Vector2<float>& GetSize() const;
    PyNovaGE::Scene::EntityID GetEntityID() const { return entity_id_; }
    
private:
    // Entity ID
    PyNovaGE::Scene::EntityID entity_id_;
    
    // Components
    PyNovaGE::Physics::RigidBody* body;
    PyNovaGE::Renderer::Sprite* sprite;
    
    // Properties
    PyNovaGE::Vector2<float> position_;
    PyNovaGE::Vector2<float> size_;
    
    // Reference to physics world
    PyNovaGE::Physics::PhysicsWorld* physicsWorld;
};
