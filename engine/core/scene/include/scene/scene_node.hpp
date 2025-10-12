#pragma once

#include "scene/transform2d.hpp"
#include "scene/entity.hpp"
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <typeindex>
#include <algorithm>

namespace PyNovaGE {
namespace Scene {

/**
 * @brief Scene graph node for hierarchical transform management
 * 
 * Represents a node in the scene graph tree with parent/child relationships.
 * Each node has a local transform and computes world transform from parent chain.
 * Optionally associated with an Entity for ECS component storage.
 */
class SceneNode {
public:
    using NodeVisitor = std::function<void(const SceneNode&)>;

    /**
     * @brief Constructor
     * @param name Optional name for debugging/identification
     */
    explicit SceneNode(const std::string& name = "");

    /**
     * @brief Constructor with entity association
     * @param entity Entity ID to associate with this node
     * @param name Optional name for debugging/identification
     */
    SceneNode(EntityID entity, const std::string& name = "");

    /**
     * @brief Destructor - removes from parent automatically
     */
    ~SceneNode();

    // Node properties
    void SetName(const std::string& name) { name_ = name; }
    const std::string& GetName() const { return name_; }

    void SetEntity(EntityID entity) { entity_ = entity; }
    EntityID GetEntity() const { return entity_; }
    bool HasEntity() const { return entity_.IsValid(); }

    // Hierarchy management
    void AddChild(std::shared_ptr<SceneNode> child);
    void RemoveChild(std::shared_ptr<SceneNode> child);
    void RemoveChild(SceneNode* child);
    void RemoveFromParent();
    void ClearChildren();

    std::shared_ptr<SceneNode> GetChild(const std::string& name) const;
    std::shared_ptr<SceneNode> GetChild(size_t index) const;
    size_t GetChildCount() const { return children_.size(); }
    const std::vector<std::shared_ptr<SceneNode>>& GetChildren() const { return children_; }

    SceneNode* GetParent() const { return parent_; }
    std::shared_ptr<SceneNode> GetSharedPtr();
    std::shared_ptr<const SceneNode> GetSharedPtr() const;

    // Hierarchy queries
    bool IsRoot() const { return parent_ == nullptr; }
    bool IsLeaf() const { return children_.empty(); }
    bool IsAncestorOf(const SceneNode* node) const;
    bool IsDescendantOf(const SceneNode* node) const;
    SceneNode* GetRoot();
    const SceneNode* GetRoot() const;
    size_t GetDepth() const;

    // Transform access
    Transform2D& GetTransform() { return transform_; }
    const Transform2D& GetTransform() const { return transform_; }

    // Transform convenience methods
    void SetPosition(const Vector2f& position);
    Vector2f GetPosition() const { return transform_.GetPosition(); }
    Vector2f GetWorldPosition() const;

    void SetRotation(float rotation);
    float GetRotation() const { return transform_.GetRotation(); }
    float GetWorldRotation() const;

    void SetScale(const Vector2f& scale);
    Vector2f GetScale() const { return transform_.GetScale(); }
    Vector2f GetWorldScale() const;

    const Matrix3f& GetWorldMatrix() const { return transform_.GetWorldMatrix(); }

    // Visibility and rendering
    void SetVisible(bool visible) { visible_ = visible; }
    bool IsVisible() const { return visible_; }
    bool IsWorldVisible() const; // Considers parent visibility

    void SetZOrder(int z_order);
    int GetZOrder() const { return z_order_; }

    // Update and traversal
    void UpdateTransforms();
    void UpdateTransforms(const Matrix3f& parent_world_matrix);

    void VisitChildren(const NodeVisitor& func) const;
    void VisitDescendants(const NodeVisitor& func) const;

    // Z-order sorting for rendering
    void SortChildrenByZOrder();
    static bool CompareZOrder(const std::shared_ptr<SceneNode>& a, const std::shared_ptr<SceneNode>& b);

    // Debug
    void PrintHierarchy(int indent = 0) const;
    std::string GetPath() const; // Returns full path from root like "/root/child/grandchild"

protected:
    // Called when transforms are updated
    virtual void OnTransformChanged() {}
    virtual void OnWorldTransformChanged() {}

private:
    std::string name_;
    EntityID entity_;
    Transform2D transform_;
    
    bool visible_ = true;
    int z_order_ = 0;
    bool z_order_dirty_ = false;

    SceneNode* parent_ = nullptr;
    std::vector<std::shared_ptr<SceneNode>> children_;
    std::weak_ptr<SceneNode> weak_self_;

    // Internal methods
    void SetParent(SceneNode* parent);
    void MarkZOrderDirty();
    void UpdateWorldTransform();
    void UpdateWorldTransform(const Matrix3f& parent_world_matrix);
};

/**
 * @brief Scene graph utilities
 */
namespace SceneUtils {
    // Factory functions
    std::shared_ptr<SceneNode> CreateNode(const std::string& name = "");
    std::shared_ptr<SceneNode> CreateNode(EntityID entity, const std::string& name = "");

    // Search functions
    std::shared_ptr<SceneNode> FindNodeByName(SceneNode* root, const std::string& name);
    std::shared_ptr<SceneNode> FindNodeByEntity(SceneNode* root, EntityID entity);
    std::vector<std::shared_ptr<SceneNode>> FindNodesWithComponent(SceneNode* root, const std::type_index& component_type, EntityManager* entity_manager);

    // Transform utilities
    Vector2f TransformPointToNode(const Vector2f& point, const SceneNode* from, const SceneNode* to);
    Vector2f TransformDirectionToNode(const Vector2f& direction, const SceneNode* from, const SceneNode* to);

    // Hierarchy utilities
    std::vector<SceneNode*> GetPathToRoot(SceneNode* node);
    SceneNode* FindCommonAncestor(SceneNode* a, SceneNode* b);

    // Bounds calculation (requires components to be checked externally)
    struct NodeBounds {
        Vector2f min{0.0f, 0.0f};
        Vector2f max{0.0f, 0.0f};
        bool valid = false;
    };
    NodeBounds CalculateHierarchyBounds(const SceneNode* root);
}

} // namespace Scene
} // namespace PyNovaGE