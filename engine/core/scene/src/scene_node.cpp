#include "scene/scene_node.hpp"
#include <algorithm>
#include <sstream>
#include <typeindex>

namespace PyNovaGE {
namespace Scene {

// Constructor
SceneNode::SceneNode(const std::string& name)
    : name_(name)
    , entity_()  // Construct with invalid entity ID
{
    // Initialize Transform2D with identity values
}

SceneNode::SceneNode(EntityID entity, const std::string& name)
    : name_(name)
    , entity_(entity)
{
    // Initialize Transform2D with identity values
}

SceneNode::~SceneNode() {
    // Clean up parent-child relationships
    RemoveFromParent();
    ClearChildren();
}

void SceneNode::AddChild(std::shared_ptr<SceneNode> child) {
    if (!child || child.get() == this) return;

    // Remove from old parent if any
    child->RemoveFromParent();

    // Set parent and add to children
    child->SetParent(this);
    children_.push_back(child);

    // Update transforms to reflect new hierarchy
    child->UpdateTransforms(transform_.GetWorldMatrix());
}

void SceneNode::RemoveChild(std::shared_ptr<SceneNode> child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        (*it)->SetParent(nullptr);
        children_.erase(it);
    }
}

void SceneNode::RemoveChild(SceneNode* child) {
    if (!child) return;
    auto it = std::find_if(children_.begin(), children_.end(),
        [child](const std::shared_ptr<SceneNode>& ptr) { return ptr.get() == child; });
    if (it != children_.end()) {
        (*it)->SetParent(nullptr);
        children_.erase(it);
    }
}

void SceneNode::RemoveFromParent() {
    if (parent_) {
        parent_->RemoveChild(this);
    }
}

void SceneNode::ClearChildren() {
    while (!children_.empty()) {
        RemoveChild(children_.back());
    }
}

std::shared_ptr<SceneNode> SceneNode::GetChild(const std::string& name) const {
    auto it = std::find_if(children_.begin(), children_.end(),
        [&name](const std::shared_ptr<SceneNode>& child) { return child->GetName() == name; });
    return (it != children_.end()) ? *it : nullptr;
}

std::shared_ptr<SceneNode> SceneNode::GetChild(size_t index) const {
    return (index < children_.size()) ? children_[index] : nullptr;
}

bool SceneNode::IsAncestorOf(const SceneNode* node) const {
    if (!node) return false;
    const SceneNode* parent = node->GetParent();
    while (parent) {
        if (parent == this) return true;
        parent = parent->GetParent();
    }
    return false;
}

bool SceneNode::IsDescendantOf(const SceneNode* node) const {
    return node ? node->IsAncestorOf(this) : false;
}

SceneNode* SceneNode::GetRoot() {
    return parent_ ? parent_->GetRoot() : this;
}

const SceneNode* SceneNode::GetRoot() const {
    return parent_ ? parent_->GetRoot() : this;
}

size_t SceneNode::GetDepth() const {
    size_t depth = 0;
    const SceneNode* node = this;
    while (node->parent_) {
        ++depth;
        node = node->parent_;
    }
    return depth;
}

void SceneNode::SetPosition(const Vector2f& position) {
    transform_.SetPosition(position);
    UpdateTransforms();
}

void SceneNode::SetRotation(float rotation) {
    transform_.SetRotation(rotation);
    UpdateTransforms();
}

void SceneNode::SetScale(const Vector2f& scale) {
    transform_.SetScale(scale);
    UpdateTransforms();
}

Vector2f SceneNode::GetWorldPosition() const {
    return transform_.GetWorldPosition();
}

float SceneNode::GetWorldRotation() const {
    return transform_.GetWorldRotation();
}

Vector2f SceneNode::GetWorldScale() const {
    return transform_.GetWorldScale();
}

void SceneNode::SetZOrder(int z_order) {
    if (z_order_ != z_order) {
        z_order_ = z_order;
        MarkZOrderDirty();
    }
}

bool SceneNode::IsWorldVisible() const {
    const SceneNode* node = this;
    while (node) {
        if (!node->IsVisible()) return false;
        node = node->GetParent();
    }
    return true;
}

void SceneNode::UpdateTransforms() {
    if (!parent_) {
        // Reset to identity world matrix
        transform_.SetWorldMatrix(Matrix3f::Identity());
    } else {
        // Update world matrix based on parent
        Matrix3f world_matrix = TransformUtils::CreateTRSMatrix(
            transform_.GetPosition(),
            transform_.GetRotation(),
            transform_.GetScale()
        );
        world_matrix = parent_->GetWorldMatrix() * world_matrix;
        transform_.SetWorldMatrix(world_matrix);
    }

    OnTransformChanged();
    OnWorldTransformChanged();

    for (auto& child : children_) {
        child->UpdateTransforms(transform_.GetWorldMatrix());
    }
}

void SceneNode::UpdateTransforms(const Matrix3f& parent_world_matrix) {
    // Calculate and set the world matrix
    Matrix3f local_matrix = TransformUtils::CreateTRSMatrix(
        transform_.GetPosition(),
        transform_.GetRotation(),
        transform_.GetScale()
    );
    Matrix3f world_matrix = parent_world_matrix * local_matrix;
    transform_.SetWorldMatrix(world_matrix);

    OnTransformChanged();
    OnWorldTransformChanged();

    for (auto& child : children_) {
        child->UpdateTransforms(transform_.GetWorldMatrix());
    }
}

void SceneNode::VisitChildren(const NodeVisitor& func) const {
    for (const auto& child : children_) {
        func(*child);
    }
}

void SceneNode::VisitDescendants(const NodeVisitor& func) const {
    // Visit children first
    for (const auto& child : children_) {
        func(*child);
        child->VisitDescendants(func);
    }
}


void SceneNode::SortChildrenByZOrder() {
    std::sort(children_.begin(), children_.end(), CompareZOrder);
    for (auto& child : children_) {
        if (child->z_order_dirty_) {
            child->SortChildrenByZOrder();
        }
    }
}

bool SceneNode::CompareZOrder(const std::shared_ptr<SceneNode>& a, const std::shared_ptr<SceneNode>& b) {
    return a->z_order_ < b->z_order_;
}

void SceneNode::PrintHierarchy(int indent) const {
    std::string indentation(indent * 2, ' ');
    printf("%s%s (z=%d, visible=%d, entity=%u)\n",
        indentation.c_str(), name_.c_str(), z_order_, visible_, entity_.GetID());
    for (const auto& child : children_) {
        child->PrintHierarchy(indent + 1);
    }
}

std::string SceneNode::GetPath() const {
    std::stringstream path;
    std::vector<const SceneNode*> nodes;
    const SceneNode* node = this;
    while (node) {
        nodes.push_back(node);
        node = node->parent_;
    }
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
        path << "/" << (*it)->name_;
    }
    return path.str();
}

void SceneNode::SetParent(SceneNode* parent) {
    parent_ = parent;
    if (parent) {
        UpdateTransforms(parent->GetWorldMatrix());
    } else {
        UpdateTransforms();
    }
}

void SceneNode::MarkZOrderDirty() {
    z_order_dirty_ = true;
    if (parent_) {
        parent_->MarkZOrderDirty();
    }
}

std::shared_ptr<SceneNode> SceneNode::GetSharedPtr() {
    return weak_self_.lock();
}

std::shared_ptr<const SceneNode> SceneNode::GetSharedPtr() const {
    return weak_self_.lock();
}

void SceneNode::UpdateWorldTransform() {
    if (!parent_) {
        transform_.SetWorldMatrix(Matrix3f::Identity());
    } else {
        UpdateWorldTransform(parent_->GetWorldMatrix());
    }
}

void SceneNode::UpdateWorldTransform(const Matrix3f& parent_world_matrix) {
    // Calculate local transform matrix
    Matrix3f local_matrix = TransformUtils::CreateTRSMatrix(
        transform_.GetPosition(),
        transform_.GetRotation(),
        transform_.GetScale()
    );
    
    // Combine with parent's world matrix
    Matrix3f world_matrix = parent_world_matrix * local_matrix;
    transform_.SetWorldMatrix(world_matrix);
}

// SceneUtils namespace implementation
namespace SceneUtils {

std::shared_ptr<SceneNode> CreateNode(const std::string& name) {
    auto node = std::make_shared<SceneNode>(name);
    return node;
}

std::shared_ptr<SceneNode> CreateNode(EntityID entity, const std::string& name) {
    auto node = std::make_shared<SceneNode>(entity, name);
    return node;
}

std::shared_ptr<SceneNode> FindNodeByName(SceneNode* root, const std::string& name) {
    if (!root) return nullptr;
    if (root->GetName() == name) {
        return root->GetSharedPtr();
    }
    for (const auto& child : root->GetChildren()) {
        if (auto found = FindNodeByName(child.get(), name)) {
            return found;
        }
    }
    return nullptr;
}

std::shared_ptr<SceneNode> FindNodeByEntity(SceneNode* root, EntityID entity) {
    if (!root) return nullptr;
    if (root->GetEntity() == entity) {
        return root->GetSharedPtr();
    }
    for (const auto& child : root->GetChildren()) {
        if (auto found = FindNodeByEntity(child.get(), entity)) {
            return found;
        }
    }
    return nullptr;
}

template<typename T>
std::vector<std::shared_ptr<SceneNode>> FindNodesWithComponentImpl(SceneNode* root, EntityManager* entity_manager) {
    std::vector<std::shared_ptr<SceneNode>> nodes;
    if (!root || !entity_manager) return nodes;

    // Check root node
    // Visit root first
    if (root->HasEntity() && entity_manager->HasComponent<T>(root->GetEntity())) {
        nodes.push_back(root->GetSharedPtr());
    }

    // Then visit all descendants
    for (const auto& child : root->GetChildren()) {
        auto child_nodes = FindNodesWithComponentImpl<T>(child.get(), entity_manager);
        nodes.insert(nodes.end(), child_nodes.begin(), child_nodes.end());
    }

    return nodes;
}

std::vector<std::shared_ptr<SceneNode>> FindNodesWithComponent(SceneNode* root, const std::type_index& component_type, EntityManager* entity_manager) {
    // This is a type lookup table for component types
    // Add more as needed
    #define HANDLE_COMPONENT_TYPE(T) \
        if (component_type == std::type_index(typeid(T))) { \
            return FindNodesWithComponentImpl<T>(root, entity_manager); \
        }

    // Add component types here
    HANDLE_COMPONENT_TYPE(Transform2D)

    #undef HANDLE_COMPONENT_TYPE

    // Return empty list for unknown types
    return std::vector<std::shared_ptr<SceneNode>>();
}

std::vector<SceneNode*> GetPathToRoot(SceneNode* node) {
    std::vector<SceneNode*> path;
    while (node) {
        path.push_back(node);
        node = node->GetParent();
    }
    std::reverse(path.begin(), path.end());
    return path;
}

SceneNode* FindCommonAncestor(SceneNode* a, SceneNode* b) {
    if (!a || !b) return nullptr;
    
    std::vector<SceneNode*> path_a = GetPathToRoot(a);
    std::vector<SceneNode*> path_b = GetPathToRoot(b);
    
    size_t i = 0;
    while (i < path_a.size() && i < path_b.size() && path_a[i] == path_b[i]) {
        ++i;
    }
    return i > 0 ? path_a[i - 1] : nullptr;
}

Vector2f TransformPointToNode(const Vector2f& point, const SceneNode* from, const SceneNode* to) {
    if (!from || !to) return point;

    // Transform to world space
    Vector2f world_point = point;
    if (from) {
        world_point = from->GetWorldMatrix().TransformPoint(point);
    }

    // Transform to target space
    if (to) {
        auto inv_matrix = to->GetWorldMatrix().GetInverse();
        return inv_matrix.TransformPoint(world_point);
    }

    return world_point;
}

Vector2f TransformDirectionToNode(const Vector2f& direction, const SceneNode* from, const SceneNode* to) {
    if (!from || !to) return direction;

    // Transform to world space first by removing translation effects
    Vector2f world_dir = direction;
    if (from) {
        // For directions, we only want rotation and scale effects
        Matrix3f rotation_scale = from->GetWorldMatrix();
        rotation_scale.SetTranslation(Vector2f(0.0f, 0.0f));
        world_dir = rotation_scale * direction;
    }

    // Transform to target space
    if (to) {
        auto inv_matrix = to->GetWorldMatrix().GetInverse();
        // Again, only use rotation and scale
        inv_matrix.SetTranslation(Vector2f(0.0f, 0.0f));
        return inv_matrix * world_dir;
    }

    return world_dir;
}

// Calculates axis-aligned bounds for a hierarchy
NodeBounds CalculateHierarchyBounds(const SceneNode* root) {
    if (!root) return NodeBounds{};

    NodeBounds bounds;
    bounds.valid = false;

    auto update_bounds = [&bounds](const Vector2f& point) {
        if (!bounds.valid) {
            bounds.min = point;
            bounds.max = point;
            bounds.valid = true;
        } else {
            bounds.min.x = std::min(bounds.min.x, point.x);
            bounds.min.y = std::min(bounds.min.y, point.y);
            bounds.max.x = std::max(bounds.max.x, point.x);
            bounds.max.y = std::max(bounds.max.y, point.y);
        }
    };

    // Manual DFS to avoid overload ambiguity
    std::function<void(const SceneNode*)> dfs = [&](const SceneNode* node){
        if (!node) return;
        update_bounds(node->GetWorldPosition());
        for (const auto& child : node->GetChildren()) {
            dfs(child.get());
        }
    };

    dfs(root);
    return bounds;
}

} // namespace SceneUtils

} // namespace Scene
} // namespace PyNovaGE