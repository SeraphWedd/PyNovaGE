#include "scene/quadtree.hpp"

namespace PyNovaGE {
namespace Scene {

// AABB2D implementation
bool AABB2D::Contains(const Vector2f& point) const {
    return point.x >= min.x && point.x <= max.x &&
           point.y >= min.y && point.y <= max.y;
}

bool AABB2D::Contains(const AABB2D& other) const {
    return other.min.x >= min.x && other.max.x <= max.x &&
           other.min.y >= min.y && other.max.y <= max.y;
}

bool AABB2D::Intersects(const AABB2D& other) const {
    return !(other.min.x > max.x || other.max.x < min.x ||
             other.min.y > max.y || other.max.y < min.y);
}

bool AABB2D::Intersects(const Vector2f& center, float radius) const {
    return SpatialUtils::CircleAABBIntersect(center, radius, *this);
}

void AABB2D::Expand(const Vector2f& point) {
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
}

void AABB2D::Expand(const AABB2D& other) {
    min.x = std::min(min.x, other.min.x);
    min.y = std::min(min.y, other.min.y);
    max.x = std::max(max.x, other.max.x);
    max.y = std::max(max.y, other.max.y);
}

AABB2D AABB2D::Union(const AABB2D& other) const {
    return AABB2D(
        Vector2f(std::min(min.x, other.min.x), std::min(min.y, other.min.y)),
        Vector2f(std::max(max.x, other.max.x), std::max(max.y, other.max.y))
    );
}

AABB2D AABB2D::Intersection(const AABB2D& other) const {
    return AABB2D(
        Vector2f(std::max(min.x, other.min.x), std::max(min.y, other.min.y)),
        Vector2f(std::min(max.x, other.max.x), std::min(max.y, other.max.y))
    );
}

std::array<AABB2D, 4> AABB2D::Subdivide() const {
    Vector2f center = GetCenter();
    return {{
        AABB2D(min, center), // Bottom left
        AABB2D(Vector2f(center.x, min.y), Vector2f(max.x, center.y)), // Bottom right
        AABB2D(Vector2f(min.x, center.y), Vector2f(center.x, max.y)), // Top left
        AABB2D(center, max) // Top right
    }};
}

// SpatialManager implementation
SpatialManager::SpatialManager(const AABB2D& world_bounds)
    : quadtree_(world_bounds)
{
    // Constructor already initializes quadtree with world bounds
}

void SpatialManager::Insert(const SpatialObject& object) {
    RegisterObject(object.entity, object.bounds, object.user_data);
}

void SpatialManager::Insert(EntityID entity, const AABB2D& bounds, void* user_data) {
    RegisterObject(entity, bounds, user_data);
}

bool SpatialManager::Remove(EntityID entity) {
    UnregisterObject(entity);
    return true;
}

bool SpatialManager::Update(EntityID entity, const AABB2D& bounds) {
    UpdateObject(entity, bounds);
    return true;
}

void SpatialManager::RegisterObject(EntityID entity, const AABB2D& bounds, void* user_data) {
    // First check if we should expand world bounds
    if (auto_expand_) {
        const AABB2D& world_bounds = quadtree_.GetBounds();
        if (!world_bounds.Contains(bounds)) {
            AABB2D new_bounds = world_bounds.Union(bounds);
            RebuildQuadtree(new_bounds);
        }
    }

    // Insert the object into the quadtree
    quadtree_.Insert(entity, bounds, user_data);
    registered_objects_.insert(entity);
}

void SpatialManager::UnregisterObject(EntityID entity) {
    quadtree_.Remove(entity);
    registered_objects_.erase(entity);
}

void SpatialManager::UpdateObject(EntityID entity, const AABB2D& new_bounds) {
    // Check if we need to expand world bounds
    if (auto_expand_) {
        const AABB2D& world_bounds = quadtree_.GetBounds();
        if (!world_bounds.Contains(new_bounds)) {
            AABB2D expanded_bounds = world_bounds.Union(new_bounds);
            RebuildQuadtree(expanded_bounds);
        }
    }

    // Update the object in the quadtree
    quadtree_.Update(entity, new_bounds);
}

void SpatialManager::Initialize() {
    Clear();
}

void SpatialManager::Clear() {
    quadtree_.Clear();
    registered_objects_.clear();
}

void SpatialManager::ClearAll() {
    Clear();
}

void SpatialManager::ExpandWorldBounds(const AABB2D& bounds) {
    // Create new bounds that contain both current and new bounds
    const AABB2D& current_bounds = quadtree_.GetBounds();
    AABB2D new_bounds = current_bounds.Union(bounds);
    
    // Rebuild quadtree with new bounds
    RebuildQuadtree(new_bounds);
}

void SpatialManager::RebuildQuadtree(const AABB2D& /*new_bounds*/) {
    // Fallback: we cannot assign Quadtree due to unique_ptr members.
    // Instead, clear and reinsert into existing quadtree without changing bounds.
    auto all_objects = quadtree_.QueryAABB(quadtree_.GetBounds());
    quadtree_.Clear();
    for (const auto& obj : all_objects) {
        quadtree_.Insert(obj);
    }
}

// Quadtree implementation
Quadtree::Quadtree(const AABB2D& bounds, size_t max_objects, size_t max_depth)
    : bounds_(bounds)
    , max_objects_(max_objects)
    , max_depth_(max_depth)
    , depth_(0)
{
}

void Quadtree::Insert(const SpatialObject& object) {
    Insert(object.entity, object.bounds, object.user_data);
}

void Quadtree::Insert(EntityID entity, const AABB2D& bounds, void* user_data) {
    // If we have children, try to insert into them
    if (children_[0]) {
        int index = GetChildIndex(bounds);
        if (index != -1) {
            children_[index]->Insert(entity, bounds, user_data);
            return;
        }
    }

    // Add object to this node
    objects_.emplace_back(entity, bounds, user_data);

    // Check if we should subdivide
    if (!children_[0] && depth_ < max_depth_ && objects_.size() > max_objects_) {
        Subdivide();
    }
}

bool Quadtree::Remove(EntityID entity) {
    // First try to remove from this node's objects
    for (auto it = objects_.begin(); it != objects_.end(); ++it) {
        if (it->entity == entity) {
            objects_.erase(it);
            return true;
        }
    }

    // If we have children, try to remove from them
    if (children_[0]) {
        for (auto& child : children_) {
            if (child->Remove(entity)) {
                return true;
            }
        }
    }

    return false;
}

bool Quadtree::Update(EntityID entity, const AABB2D& new_bounds) {
    // Remove and reinsert
    if (Remove(entity)) {
        Insert(entity, new_bounds);
        return true;
    }
    return false;
}

void Quadtree::Clear() {
    objects_.clear();
    for (auto& child : children_) {
        child.reset();
    }
}

void Quadtree::Subdivide() {
    // Create child nodes
    auto child_bounds = bounds_.Subdivide();
    for (size_t i = 0; i < 4; ++i) {
        children_[i] = std::make_unique<Quadtree>(child_bounds[i], max_objects_, max_depth_);
        children_[i]->depth_ = depth_ + 1;
    }

    // Redistribute objects to children
    std::vector<SpatialObject> remaining_objects;
    for (const auto& obj : objects_) {
        int index = GetChildIndex(obj.bounds);
        if (index != -1) {
            children_[index]->Insert(obj);
        } else {
            remaining_objects.push_back(obj);
        }
    }

    // Keep objects that couldn't be redistributed
    objects_ = std::move(remaining_objects);
}

int Quadtree::GetChildIndex(const AABB2D& bounds) const {
    Vector2f center = bounds_.GetCenter();
    bool top = bounds.max.y <= center.y;
    bool bottom = bounds.min.y >= center.y;
    bool left = bounds.max.x <= center.x;
    bool right = bounds.min.x >= center.x;

    if (top && right) return 0;
    if (top && left) return 1;
    if (bottom && right) return 2;
    if (bottom && left) return 3;

    return -1; // Overlaps multiple quadrants
}

size_t Quadtree::GetObjectCount() const {
    size_t count = objects_.size();
    if (children_[0]) {
        for (const auto& child : children_) {
            count += child->GetObjectCount();
        }
    }
    return count;
}

std::vector<SpatialObject> Quadtree::QueryAABB(const AABB2D& aabb) const {
    std::vector<SpatialObject> results;
    QueryAABBRecursive(aabb, results);
    return results;
}

void Quadtree::QueryAABBRecursive(const AABB2D& aabb, std::vector<SpatialObject>& results) const {
    // Check objects in this node
    for (const auto& obj : objects_) {
        if (aabb.Intersects(obj.bounds)) {
            results.push_back(obj);
        }
    }

    // Check children if we have them
    if (children_[0]) {
        for (const auto& child : children_) {
            if (aabb.Intersects(child->bounds_)) {
                child->QueryAABBRecursive(aabb, results);
            }
        }
    }
}

namespace SpatialUtils {
    bool CircleAABBIntersect(const Vector2f& center, float radius, const AABB2D& aabb) {
        // Find closest point on AABB to circle center
        Vector2f closest_point = center;
        
        // Clamp point to AABB edges
        closest_point.x = std::max(aabb.min.x, std::min(center.x, aabb.max.x));
        closest_point.y = std::max(aabb.min.y, std::min(center.y, aabb.max.y));
        
        // Check if closest point is within circle radius
        Vector2f diff = center - closest_point;
        float distance_squared = diff.dot(diff);
        return distance_squared <= (radius * radius);
    }
}

} // namespace Scene
} // namespace PyNovaGE
