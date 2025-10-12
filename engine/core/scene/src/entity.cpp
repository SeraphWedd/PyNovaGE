#include "scene/entity.hpp"

namespace PyNovaGE {
namespace Scene {

EntityID EntityManager::CreateEntity() {
    EntityID::IDType id = next_id_++;
    EntityID::GenerationType gen = next_generation_++;
    entities_[id] = gen;
    return EntityID(id, gen);
}

void EntityManager::DestroyEntity(EntityID entity) {
    auto id = entity.GetID();
    auto it = entities_.find(id);
    if (it != entities_.end() && it->second == entity.GetGeneration()) {
        // Entity exists and generation matches
        entities_.erase(it);

        // Remove all components
        for (auto& [type, storage] : component_storages_) {
            storage->RemoveComponent(entity);
        }
    }
}

bool EntityManager::IsEntityValid(EntityID entity) const {
    auto it = entities_.find(entity.GetID());
    return it != entities_.end() && it->second == entity.GetGeneration();
}

void EntityManager::Clear() {
    entities_.clear();
    component_storages_.clear();
    next_id_ = 1;
    next_generation_ = 1;
}

} // namespace Scene
} // namespace PyNovaGE