#include "scene/scene.hpp"

namespace PyNovaGE {
namespace Scene {

// Constructor
Scene::Scene(const AABB2D& world_bounds) : spatial_manager_(world_bounds) {
    root_node_ = std::make_shared<SceneNode>("root");
}

// Entity creation helpers
EntityID Scene::CreateEntity(const std::string& name) {
    EntityID entity = entity_manager_.CreateEntity();
    if (!name.empty()) {
        entity_manager_.AddComponent<NameComponent>(entity, name);
    }
    return entity;
}

EntityID Scene::CreateEntityWithNode(const std::string& name, std::shared_ptr<SceneNode> parent) {
    EntityID entity = CreateEntity(name);
    
    auto node = std::make_shared<SceneNode>(entity, name);
    if (!parent) {
        parent = root_node_;
    }
    parent->AddChild(node);

    auto& hierarchy = entity_manager_.AddComponent<HierarchyComponent>(entity);
    hierarchy.scene_node = node;

    return entity;
}

void Scene::DestroyEntity(EntityID entity) {
    // Remove from scene graph if it has a node
    if (auto* hierarchy = GetComponent<HierarchyComponent>(entity)) {
        if (auto node = hierarchy->GetSceneNode()) {
            node->RemoveFromParent();
        }
    }

    // Unregister from spatial manager
    UnregisterEntityFromSpatialPartitioning(entity);
    
    // Destroy the entity
    entity_manager_.DestroyEntity(entity);
}

// Scene updates
void Scene::Update(float delta_time) {
    OnPreUpdate(delta_time);

    // Update transforms
    UpdateTransforms();

    // Update spatial partitioning
    UpdateSpatialPartitioning();

    // Physics update
    UpdatePhysics(delta_time);

    // Particles update
    UpdateParticles(delta_time);
    
    // User update callback
    if (update_callback_) {
        update_callback_(delta_time);
    }

    OnPostUpdate(delta_time);
}

void Scene::UpdateTransforms() {
    if (!root_node_) return;
    root_node_->UpdateTransforms();
    OnTransformsUpdated();
}

void Scene::UpdateSpatialPartitioning() {
    // Rebuild spatial partitioning structures
    spatial_manager_.Clear();

    // Update all entities
    std::vector<EntityID> entities = entity_manager_.GetAllEntities();
    for (const EntityID& entity : entities) {
        UpdateEntitySpatialBounds(entity);
    }

    OnSpatialPartitioningUpdated();
}

void Scene::UpdatePhysics([[maybe_unused]] float delta_time) {
    // TODO: Implement physics system update
}

void Scene::UpdateParticles([[maybe_unused]] float delta_time) {
    // TODO: Implement particle system update
}

// Camera management
void Scene::SetPrimaryCamera(EntityID camera_entity) {
    if (entity_manager_.HasComponent<CameraComponent>(camera_entity)) {
        primary_camera_ = camera_entity;
    }
}

CameraComponent* Scene::GetPrimaryCameraComponent() {
    return primary_camera_.IsValid() ? GetComponent<CameraComponent>(primary_camera_) : nullptr;
}

const CameraComponent* Scene::GetPrimaryCameraComponent() const {
    return primary_camera_.IsValid() ? GetComponent<CameraComponent>(primary_camera_) : nullptr;
}

std::vector<EntityID> Scene::GetAllCameras() const {
    return FindEntitiesWithComponent<CameraComponent>();
}

// Initialization and shutdown
void Scene::Initialize() {
    // Initialize components
    entity_manager_.Initialize();

    // Create root node if it doesn't exist
    if (!root_node_) {
        root_node_ = std::make_shared<SceneNode>("root");
    }

    // Initialize spatial manager
    spatial_manager_.Initialize();
}

void Scene::Shutdown() {
    // Clear all entities
    entity_manager_.Clear();

    // Clear scene graph
    if (root_node_) {
        root_node_->ClearChildren();
    }
    root_node_.reset();

    // Clear spatial manager
    spatial_manager_.Clear();

    // Clear callbacks
    update_callback_ = nullptr;
    render_callback_ = nullptr;
}

void Scene::Clear() {
    Shutdown();
    Initialize();
}

// Internal transform synchronization
void Scene::SyncTransformToNode(EntityID entity) {
    if (auto* transform = GetComponent<Transform2DComponent>(entity)) {
        if (auto* hierarchy = GetComponent<HierarchyComponent>(entity)) {
            if (auto node = hierarchy->GetSceneNode()) {
                node->GetTransform() = transform->transform;
            }
        }
    }
}

void Scene::SyncNodeToTransform(EntityID entity) {
    if (auto* transform = GetComponent<Transform2DComponent>(entity)) {
        if (auto* hierarchy = GetComponent<HierarchyComponent>(entity)) {
            if (auto node = hierarchy->GetSceneNode()) {
                transform->transform = node->GetTransform();
            }
        }
    }
}

void Scene::UpdateEntitySpatialBounds(EntityID entity) {
    if (auto bounds = CalculateEntityBounds(entity); bounds.IsValid()) {
        RegisterEntityForSpatialPartitioning(entity);
    }
}

AABB2D Scene::CalculateEntityBounds([[maybe_unused]] EntityID entity) const {
    // TODO: Implement bounds calculation based on components
    return AABB2D{};
}

void Scene::RegisterEntityForSpatialPartitioning([[maybe_unused]] EntityID entity) {
    auto bounds = CalculateEntityBounds(entity);
    if (bounds.IsValid()) {
        spatial_manager_.RegisterObject(entity, bounds);
    }
}

void Scene::UnregisterEntityFromSpatialPartitioning(EntityID entity) {
    spatial_manager_.UnregisterObject(entity);
}

std::vector<EntityID> Scene::FindEntitiesWithComponent(const std::type_index& component_type) const {
    std::vector<EntityID> result;
    for (const auto& entity : entity_manager_.GetAllEntities()) {
        // We need to get the component storage for this type index
        if (const auto& storages = entity_manager_.GetStorages(); storages.find(component_type) != storages.end()) {
            if (storages.at(component_type)->HasComponent(entity)) {
                result.push_back(entity);
            }
        }
    }
    return result;
}

} // namespace Scene
} // namespace PyNovaGE