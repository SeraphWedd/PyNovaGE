#pragma once

#include "scene/entity.hpp"
#include "scene/transform2d.hpp"
#include "scene/scene_node.hpp"
#include <vectors/vectors.hpp>
#include <memory>
#include <string>

// Forward declarations for integration with other systems
namespace PyNovaGE {
    namespace Renderer { class Texture; class Sprite; }
    namespace Physics { class RigidBody; }
    namespace Particles { class ParticleEmitter; }
}

namespace PyNovaGE {
namespace Scene {

// Type aliases for convenience
using Vector2f = PyNovaGE::Vector2<float>;
using Vector4f = PyNovaGE::Vector4<float>;

/**
 * @brief Transform component for ECS entities
 * 
 * Provides 2D transformation (position, rotation, scale) for entities.
 * Integrates with scene graph for hierarchical transforms.
 */
class Transform2DComponent : public Component {
public:
    Transform2DComponent() = default;
    Transform2DComponent(const Vector2f& position, float rotation = 0.0f, const Vector2f& scale = Vector2f(1.0f, 1.0f))
        : transform(position, rotation, scale) {}

    Transform2D transform;

    // Convenience accessors
    void SetPosition(const Vector2f& position) { transform.SetPosition(position); }
    const Vector2f& GetPosition() const { return transform.GetPosition(); }
    
    void SetRotation(float rotation) { transform.SetRotation(rotation); }
    float GetRotation() const { return transform.GetRotation(); }
    
    void SetScale(const Vector2f& scale) { transform.SetScale(scale); }
    const Vector2f& GetScale() const { return transform.GetScale(); }

    Vector2f GetWorldPosition() const { return transform.GetWorldPosition(); }
    float GetWorldRotation() const { return transform.GetWorldRotation(); }
    Vector2f GetWorldScale() const { return transform.GetWorldScale(); }
};

/**
 * @brief Sprite rendering component
 * 
 * Associates an entity with sprite rendering.
 * Integrates with the renderer's sprite system.
 */
class SpriteComponent : public Component {
public:
    SpriteComponent() = default;
    SpriteComponent(std::shared_ptr<Renderer::Texture> texture, const Vector4f& color = Vector4f(1.0f, 1.0f, 1.0f, 1.0f))
        : texture(texture), color(color) {}

    std::shared_ptr<Renderer::Texture> texture;
    Vector4f color{1.0f, 1.0f, 1.0f, 1.0f}; // RGBA tint
    Vector4f uv_rect{0.0f, 0.0f, 1.0f, 1.0f}; // Texture UV coordinates (for sprite sheets)
    Vector2f size{0.0f, 0.0f}; // Sprite size (0,0 = use texture size)
    Vector2f pivot{0.5f, 0.5f}; // Pivot point (0,0 = top-left, 0.5,0.5 = center)
    bool visible = true;
    int render_layer = 0; // For layered rendering
    float alpha = 1.0f; // Additional alpha multiplier

    // Convenience methods
    void SetColor(float r, float g, float b, float a = 1.0f) { color = Vector4f(r, g, b, a); }
    void SetUVRect(float u, float v, float w, float h) { uv_rect = Vector4f(u, v, w, h); }
    void SetSize(const Vector2f& new_size) { size = new_size; }
    void SetPivot(const Vector2f& new_pivot) { pivot = new_pivot; }
};

/**
 * @brief Physics body component
 * 
 * Associates an entity with 2D physics simulation.
 * Integrates with the physics system's RigidBody.
 */
class RigidBody2DComponent : public Component {
public:
    RigidBody2DComponent() = default;
    explicit RigidBody2DComponent(std::shared_ptr<Physics::RigidBody> body) 
        : body(body) {}

    std::shared_ptr<Physics::RigidBody> body;
    bool auto_sync_transform = true; // Automatically sync with Transform2DComponent
    
    // Convenience methods (delegate to underlying RigidBody)
    void SetPosition(const Vector2f& position);
    Vector2f GetPosition() const;
    void SetRotation(float rotation);
    float GetRotation() const;
    void SetLinearVelocity(const Vector2f& velocity);
    Vector2f GetLinearVelocity() const;
    void SetAngularVelocity(float velocity);
    float GetAngularVelocity() const;
    void ApplyForce(const Vector2f& force);
    void ApplyImpulse(const Vector2f& impulse);
};

/**
 * @brief Particle emitter component
 * 
 * Associates an entity with particle emission.
 * Integrates with the particle system.
 */
class ParticleEmitter2DComponent : public Component {
public:
    ParticleEmitter2DComponent() = default;
    explicit ParticleEmitter2DComponent(std::shared_ptr<Particles::ParticleEmitter> emitter)
        : emitter(emitter) {}

    std::shared_ptr<Particles::ParticleEmitter> emitter;
    bool auto_sync_position = true; // Automatically sync position with Transform2DComponent
    Vector2f position_offset{0.0f, 0.0f}; // Additional offset from transform position
    
    // Convenience methods
    void SetPosition(const Vector2f& position);
    Vector2f GetPosition() const;
    void Start();
    void Stop();
    void SetPaused(bool paused);
    bool IsActive() const;
    void EmitBurst(int count);
};

/**
 * @brief Name component for entity identification
 * 
 * Provides a human-readable name for entities.
 * Useful for debugging and editor tools.
 */
class NameComponent : public Component {
public:
    NameComponent() = default;
    explicit NameComponent(const std::string& entity_name) : name(entity_name) {}

    std::string name;
    
    void SetName(const std::string& new_name) { name = new_name; }
    const std::string& GetName() const { return name; }
    bool IsNamed(const std::string& search_name) const { return name == search_name; }
};

/**
 * @brief Hierarchy component for scene graph integration
 * 
 * Links entities to scene graph nodes for hierarchical transforms.
 * Allows ECS entities to participate in the scene hierarchy.
 */
class HierarchyComponent : public Component {
public:
    HierarchyComponent() = default;
    explicit HierarchyComponent(std::shared_ptr<SceneNode> node) : scene_node(node) {}

    std::weak_ptr<SceneNode> scene_node; // Weak reference to avoid circular dependencies
    
    // Convenience methods
    std::shared_ptr<SceneNode> GetSceneNode() const { return scene_node.lock(); }
    bool HasValidNode() const { return !scene_node.expired(); }
    void SetSceneNode(std::shared_ptr<SceneNode> node) { scene_node = node; }
    void ClearSceneNode() { scene_node.reset(); }
};

/**
 * @brief Camera component for viewport rendering
 * 
 * Defines a 2D camera for rendering the scene from a specific viewpoint.
 * Supports orthographic projection with zoom and viewport controls.
 */
class CameraComponent : public Component {
public:
    CameraComponent() = default;
    CameraComponent(const Vector2f& view_size, float zoom_level = 1.0f) 
        : viewport_size(view_size), zoom(zoom_level) {}

    Vector2f viewport_size{1280.0f, 720.0f}; // Viewport size in pixels
    float zoom = 1.0f; // Zoom level (1.0 = normal, 2.0 = 2x zoom)
    Vector2f offset{0.0f, 0.0f}; // Additional camera offset from transform position
    bool is_primary = false; // Primary camera for main rendering
    int render_order = 0; // Order for multi-camera rendering
    
    // View bounds calculation
    Vector2f GetViewSize() const { return viewport_size / zoom; }
    Vector2f GetViewMin(const Vector2f& camera_world_pos) const;
    Vector2f GetViewMax(const Vector2f& camera_world_pos) const;
    bool IsPointInView(const Vector2f& world_point, const Vector2f& camera_world_pos) const;
    bool IsRectInView(const Vector2f& rect_min, const Vector2f& rect_max, const Vector2f& camera_world_pos) const;
    
    // Screen-world conversion
    Vector2f ScreenToWorld(const Vector2f& screen_pos, const Vector2f& camera_world_pos) const;
    Vector2f WorldToScreen(const Vector2f& world_pos, const Vector2f& camera_world_pos) const;
};

} // namespace Scene
} // namespace PyNovaGE