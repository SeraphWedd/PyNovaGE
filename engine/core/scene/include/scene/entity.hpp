#pragma once

#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace PyNovaGE {
namespace Scene {

/**
 * @brief Lightweight entity ID
 * 
 * Simple integer-based entity identifier for ECS system.
 * Provides unique IDs with generation counter to detect stale references.
 */
class EntityID {
public:
    using IDType = uint32_t;
    using GenerationType = uint16_t;

    static constexpr IDType NULL_ID = 0;
    static constexpr GenerationType NULL_GENERATION = 0;

    EntityID() : id_(NULL_ID), generation_(NULL_GENERATION) {}
    EntityID(IDType id, GenerationType generation) : id_(id), generation_(generation) {}

    IDType GetID() const { return id_; }
    GenerationType GetGeneration() const { return generation_; }

    bool IsValid() const { return id_ != NULL_ID; }
    void Invalidate() { id_ = NULL_ID; generation_ = NULL_GENERATION; }

    bool operator==(const EntityID& other) const {
        return id_ == other.id_ && generation_ == other.generation_;
    }
    bool operator!=(const EntityID& other) const { return !(*this == other); }
    bool operator<(const EntityID& other) const {
        if (id_ != other.id_) return id_ < other.id_;
        return generation_ < other.generation_;
    }

    // Hash support for unordered containers
    struct Hash {
        std::size_t operator()(const EntityID& entity) const {
            return std::hash<uint64_t>{}((uint64_t(entity.id_) << 16) | entity.generation_);
        }
    };

private:
    IDType id_;
    GenerationType generation_;
};

/**
 * @brief Base class for ECS components
 * 
 * All components must inherit from this base class.
 * Provides type information and virtual destructor.
 */
class Component {
public:
    virtual ~Component() = default;
    
    template<typename T>
    bool IsType() const {
        return typeid(*this) == typeid(T);
    }
    
    template<typename T>
    T* As() {
        return dynamic_cast<T*>(this);
    }
    
    template<typename T>
    const T* As() const {
        return dynamic_cast<const T*>(this);
    }
};

/**
 * @brief Component storage interface
 * 
 * Type-erased storage for components of any type.
 * Allows the entity manager to store different component types uniformly.
 */
class IComponentStorage {
public:
    virtual ~IComponentStorage() = default;
    virtual void RemoveComponent(EntityID entity) = 0;
    virtual bool HasComponent(EntityID entity) const = 0;
    virtual Component* GetComponent(EntityID entity) = 0;
    virtual const Component* GetComponent(EntityID entity) const = 0;
};

/**
 * @brief Template component storage implementation
 * 
 * Stores components of a specific type T in a hash map.
 */
template<typename T>
class ComponentStorage : public IComponentStorage {
    static_assert(std::is_base_of_v<Component, T>, "T must be a Component");

public:
    void AddComponent(EntityID entity, T&& component) {
        components_[entity] = std::make_unique<T>(std::move(component));
    }
    
    template<typename... Args>
    T& EmplaceComponent(EntityID entity, Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *ptr;
        components_[entity] = std::move(ptr);
        return ref;
    }

    void RemoveComponent(EntityID entity) override {
        components_.erase(entity);
    }

    bool HasComponent(EntityID entity) const override {
        return components_.find(entity) != components_.end();
    }

    Component* GetComponent(EntityID entity) override {
        auto it = components_.find(entity);
        return (it != components_.end()) ? it->second.get() : nullptr;
    }

    const Component* GetComponent(EntityID entity) const override {
        auto it = components_.find(entity);
        return (it != components_.end()) ? it->second.get() : nullptr;
    }

    T* GetTypedComponent(EntityID entity) {
        auto it = components_.find(entity);
        return (it != components_.end()) ? it->second.get() : nullptr;
    }

    const T* GetTypedComponent(EntityID entity) const {
        auto it = components_.find(entity);
        return (it != components_.end()) ? it->second.get() : nullptr;
    }

    // Iterator support for system queries
    auto begin() { return components_.begin(); }
    auto end() { return components_.end(); }
    auto begin() const { return components_.begin(); }
    auto end() const { return components_.end(); }

    size_t Size() const { return components_.size(); }
    void Clear() { components_.clear(); }

private:
    std::unordered_map<EntityID, std::unique_ptr<T>, EntityID::Hash> components_;
};

/**
 * @brief Entity manager for lightweight ECS
 * 
 * Manages entity creation/destruction and component storage.
 * Uses type-erased storage to handle different component types.
 */
class EntityManager {
public:
    // Entity management
    EntityID CreateEntity();
    void DestroyEntity(EntityID entity);
    bool IsEntityValid(EntityID entity) const;

    // Component management
    template<typename T, typename... Args>
    T& AddComponent(EntityID entity, Args&&... args) {
        if (!IsEntityValid(entity)) {
            throw std::runtime_error("Invalid entity ID");
        }

        auto type_index = std::type_index(typeid(T));
        auto it = component_storages_.find(type_index);
        
        if (it == component_storages_.end()) {
            auto storage = std::make_unique<ComponentStorage<T>>();
            auto* storage_ptr = storage.get();
            component_storages_[type_index] = std::move(storage);
            return storage_ptr->EmplaceComponent(entity, std::forward<Args>(args)...);
        }
        
        auto* storage = static_cast<ComponentStorage<T>*>(it->second.get());
        return storage->EmplaceComponent(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    void RemoveComponent(EntityID entity) {
        auto type_index = std::type_index(typeid(T));
        auto it = component_storages_.find(type_index);
        if (it != component_storages_.end()) {
            it->second->RemoveComponent(entity);
        }
    }

    template<typename T>
    bool HasComponent(EntityID entity) const {
        auto type_index = std::type_index(typeid(T));
        auto it = component_storages_.find(type_index);
        return (it != component_storages_.end()) && it->second->HasComponent(entity);
    }

    template<typename T>
    T* GetComponent(EntityID entity) {
        auto type_index = std::type_index(typeid(T));
        auto it = component_storages_.find(type_index);
        if (it != component_storages_.end()) {
            auto* storage = static_cast<ComponentStorage<T>*>(it->second.get());
            return storage->GetTypedComponent(entity);
        }
        return nullptr;
    }

    template<typename T>
    const T* GetComponent(EntityID entity) const {
        auto type_index = std::type_index(typeid(T));
        auto it = component_storages_.find(type_index);
        if (it != component_storages_.end()) {
            const auto* storage = static_cast<const ComponentStorage<T>*>(it->second.get());
            return storage->GetTypedComponent(entity);
        }
        return nullptr;
    }

    template<typename T>
    ComponentStorage<T>* GetComponentStorage() {
        auto type_index = std::type_index(typeid(T));
        auto it = component_storages_.find(type_index);
        return (it != component_storages_.end()) ? 
               static_cast<ComponentStorage<T>*>(it->second.get()) : nullptr;
    }

    // Utility
    size_t GetEntityCount() const { return entities_.size(); }
    void Clear();

private:
    EntityID::IDType next_id_ = 1;
    EntityID::GenerationType next_generation_ = 1;
    std::unordered_map<EntityID::IDType, EntityID::GenerationType> entities_;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> component_storages_;
};

} // namespace Scene
} // namespace PyNovaGE