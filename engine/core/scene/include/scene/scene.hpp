#pragma once

#include "scene/scene_node.hpp"
#include "scene/entity.hpp"
#include "scene/components.hpp"
#include "scene/quadtree.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace PyNovaGE {
namespace Scene {

/**
 * @brief Main scene class for 2D/2.5D scene management
 * 
 * Integrates scene graph, ECS, and spatial partitioning systems.
 * Manages scene updates, rendering order, and system coordination.
 */
class Scene {
public:
    using UpdateCallback = std::function<void(float)>;
    using RenderCallback = std::function<void(const CameraComponent*)>;

    /**
     * @brief Constructor
     * @param world_bounds Spatial bounds for the scene
     */
    explicit Scene(const AABB2D& world_bounds = AABB2D(-10000.0f, -10000.0f, 20000.0f, 20000.0f));

    /**
     * @brief Destructor
     */
    ~Scene() = default;

    // Scene graph access
    std::shared_ptr<SceneNode> GetRootNode() const { return root_node_; }
    void SetRootNode(std::shared_ptr<SceneNode> root) { root_node_ = root; }

    // Entity management
    EntityManager& GetEntityManager() { return entity_manager_; }
    const EntityManager& GetEntityManager() const { return entity_manager_; }

    // Spatial partitioning
    SpatialManager& GetSpatialManager() { return spatial_manager_; }
    const SpatialManager& GetSpatialManager() const { return spatial_manager_; }

    // Entity creation helpers
    EntityID CreateEntity(const std::string& name = "");
    EntityID CreateEntityWithNode(const std::string& name = "", std::shared_ptr<SceneNode> parent = nullptr);
    void DestroyEntity(EntityID entity);

    // Component shortcuts
    template<typename T, typename... Args>
    T& AddComponent(EntityID entity, Args&&... args) {
        return entity_manager_.AddComponent<T>(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    T* GetComponent(EntityID entity) {
        return entity_manager_.GetComponent<T>(entity);
    }

    template<typename T>
    const T* GetComponent(EntityID entity) const {
        return entity_manager_.GetComponent<T>(entity);
    }

    template<typename T>
    bool HasComponent(EntityID entity) const {
        return entity_manager_.HasComponent<T>(entity);
    }

    template<typename T>
    void RemoveComponent(EntityID entity) {
        entity_manager_.RemoveComponent<T>(entity);
    }

    // Scene updates
    void Update(float delta_time);
    void UpdateTransforms();
    void UpdateSpatialPartitioning();
    void UpdatePhysics(float delta_time);
    void UpdateParticles(float delta_time);

    // Camera management
    void SetPrimaryCamera(EntityID camera_entity);
    EntityID GetPrimaryCamera() const { return primary_camera_; }
    CameraComponent* GetPrimaryCameraComponent();
    const CameraComponent* GetPrimaryCameraComponent() const;
    std::vector<EntityID> GetAllCameras() const;

    // Culling and rendering support
    std::vector<EntityID> GetVisibleEntities(const CameraComponent* camera) const;
    std::vector<EntityID> GetEntitiesInBounds(const AABB2D& bounds) const;
    void GetRenderableSprites(const CameraComponent* camera, std::vector<std::pair<EntityID, SpriteComponent*>>& sprites) const;

    // Scene queries
    EntityID FindEntityByName(const std::string& name) const;
    std::vector<EntityID> FindEntitiesByName(const std::string& name) const;
    std::vector<EntityID> FindEntitiesWithComponent(const std::type_index& component_type) const;

    template<typename T>
    std::vector<EntityID> FindEntitiesWithComponent() const {
        return FindEntitiesWithComponent(std::type_index(typeid(T)));
    }

    // Spatial queries
    std::vector<EntityID> QueryPoint(const Vector2f& point) const;
    std::vector<EntityID> QueryAABB(const AABB2D& aabb) const;
    std::vector<EntityID> QueryCircle(const Vector2f& center, float radius) const;
    std::vector<Quadtree::RayHit> Raycast(const Vector2f& origin, const Vector2f& direction, float max_distance = std::numeric_limits<float>::infinity()) const;

    // Scene hierarchy utilities
    void AttachEntityToNode(EntityID entity, std::shared_ptr<SceneNode> node);
    void DetachEntityFromNode(EntityID entity);
    std::shared_ptr<SceneNode> GetEntityNode(EntityID entity) const;

    // System integration callbacks
    void SetUpdateCallback(const UpdateCallback& callback) { update_callback_ = callback; }
    void SetRenderCallback(const RenderCallback& callback) { render_callback_ = callback; }

    // Scene lifecycle
    void Initialize();
    void Shutdown();
    void Clear();

    // Statistics
    size_t GetEntityCount() const { return entity_manager_.GetEntityCount(); }
    size_t GetSpatialObjectCount() const { return spatial_manager_.GetObjectCount(); }
    const AABB2D& GetWorldBounds() const { return spatial_manager_.GetWorldBounds(); }

    // Debug
    void PrintSceneGraph(int max_depth = -1) const;
    void GetDebugInfo(std::string& info) const;

protected:
    // Internal update phases
    virtual void OnPreUpdate(float delta_time) {}
    virtual void OnPostUpdate(float delta_time) {}
    virtual void OnTransformsUpdated() {}
    virtual void OnSpatialPartitioningUpdated() {}

private:
    std::shared_ptr<SceneNode> root_node_;
    EntityManager entity_manager_;
    SpatialManager spatial_manager_;
    
    EntityID primary_camera_;
    UpdateCallback update_callback_;
    RenderCallback render_callback_;

    // Internal methods
    void SyncTransformToNode(EntityID entity);
    void SyncNodeToTransform(EntityID entity);
    void SyncPhysicsToTransform(EntityID entity);
    void SyncTransformToPhysics(EntityID entity);
    void SyncParticleEmitterPosition(EntityID entity);
    void UpdateEntitySpatialBounds(EntityID entity);
    
    AABB2D CalculateEntityBounds(EntityID entity) const;
    void RegisterEntityForSpatialPartitioning(EntityID entity);
    void UnregisterEntityFromSpatialPartitioning(EntityID entity);
};

/**
 * @brief Scene management utilities
 */
namespace SceneUtils {
    // Factory functions
    std::unique_ptr<Scene> CreateScene(const AABB2D& world_bounds = AABB2D(-1000.0f, -1000.0f, 2000.0f, 2000.0f));
    
    // Entity creation helpers
    EntityID CreateSpriteEntity(Scene& scene, const std::string& name, const Vector2f& position, 
                               std::shared_ptr<Renderer::Texture> texture = nullptr);
    EntityID CreateCameraEntity(Scene& scene, const std::string& name, const Vector2f& position,
                               const Vector2f& viewport_size, float zoom = 1.0f);
    
    // Component synchronization
    void SyncTransformWithPhysics(Scene& scene, EntityID entity);
    void SyncTransformWithParticles(Scene& scene, EntityID entity);
    void SyncHierarchyTransforms(Scene& scene);
    
    // Batch operations
    void UpdateAllEntitySpatialBounds(Scene& scene);
    void SortSpritesByRenderOrder(std::vector<std::pair<EntityID, SpriteComponent*>>& sprites);
    
    // Scene serialization helpers (for future extension)
    struct SceneData {
        std::vector<EntityID> entities;
        // More serialization data can be added here
    };
    SceneData CaptureSceneState(const Scene& scene);
    void RestoreSceneState(Scene& scene, const SceneData& data);
}

} // namespace Scene
} // namespace PyNovaGE