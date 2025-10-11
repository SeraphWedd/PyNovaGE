#pragma once

/**
 * @file scene_system.hpp
 * @brief PyNovaGE Scene System - 2D/2.5D Scene Graph with Lightweight ECS
 * 
 * This is the main header for the PyNovaGE Scene System, providing:
 * - Hierarchical scene graph with transforms
 * - Lightweight Entity-Component-System (ECS)
 * - Spatial partitioning with 2D quadtree
 * - Integration with renderer, physics, and particle systems
 * 
 * Usage:
 * @code
 * #include <scene/scene_system.hpp>
 * 
 * using namespace PyNovaGE::Scene;
 * 
 * // Create a scene
 * auto scene = SceneUtils::CreateScene();
 * 
 * // Create entities with components
 * auto entity = scene->CreateEntity("player");
 * scene->AddComponent<Transform2DComponent>(entity, Vector2f(100, 100));
 * scene->AddComponent<SpriteComponent>(entity, my_texture);
 * 
 * // Update scene
 * scene->Update(delta_time);
 * @endcode
 */

// Core scene system headers
#include "scene/entity.hpp"
#include "scene/transform2d.hpp"
#include "scene/scene_node.hpp"
#include "scene/components.hpp"
#include "scene/quadtree.hpp"
#include "scene/scene.hpp"

namespace PyNovaGE {
namespace Scene {

/**
 * @brief Scene System version information
 */
struct SceneSystemInfo {
    static constexpr int MAJOR_VERSION = 1;
    static constexpr int MINOR_VERSION = 0;
    static constexpr int PATCH_VERSION = 0;
    static constexpr const char* VERSION_STRING = "1.0.0";
    
    static constexpr const char* DESCRIPTION = "PyNovaGE Scene System - 2D/2.5D Scene Graph with Lightweight ECS";
    
    // Feature flags
    static constexpr bool HAS_SCENE_GRAPH = true;
    static constexpr bool HAS_ECS = true;
    static constexpr bool HAS_SPATIAL_PARTITIONING = true;
    static constexpr bool HAS_PHYSICS_INTEGRATION = true;
    static constexpr bool HAS_PARTICLE_INTEGRATION = true;
    static constexpr bool HAS_RENDERER_INTEGRATION = true;
};

/**
 * @brief Scene System initialization and cleanup
 */
class SceneSystem {
public:
    /**
     * @brief Initialize the scene system
     * @return true if initialization was successful
     */
    static bool Initialize();
    
    /**
     * @brief Shutdown the scene system
     */
    static void Shutdown();
    
    /**
     * @brief Check if the scene system is initialized
     */
    static bool IsInitialized() { return initialized_; }
    
    /**
     * @brief Get system information
     */
    static const SceneSystemInfo& GetInfo() { return info_; }

private:
    static bool initialized_;
    static SceneSystemInfo info_;
};

} // namespace Scene

// Type aliases for convenience
using ScenePtr = std::unique_ptr<Scene::Scene>;
using SceneNodePtr = std::shared_ptr<Scene::SceneNode>;
using EntityID = Scene::EntityID;

// Common component type aliases
using Transform2D = Scene::Transform2DComponent;
using Sprite = Scene::SpriteComponent;
using RigidBody2D = Scene::RigidBody2DComponent;
using ParticleEmitter2D = Scene::ParticleEmitter2DComponent;
using NameComponent = Scene::NameComponent;
using HierarchyComponent = Scene::HierarchyComponent;
using CameraComponent = Scene::CameraComponent;

} // namespace PyNovaGE