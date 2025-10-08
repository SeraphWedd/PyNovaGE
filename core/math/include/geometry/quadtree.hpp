#pragma once

#include "spatial_partitioning.hpp"
#include "../vector3.hpp"
#include "../vector2.hpp"
#include "../matrix4.hpp"
#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace pynovage {
namespace math {
namespace geometry {

template<typename T>
class Quadtree : public SpatialContainer<T> {
public:
    explicit Quadtree(const SpatialConfig& config = SpatialConfig())
        : config_(config), root_(nullptr), objectCount_(0) {}
    
    void insert(std::unique_ptr<SpatialObject<T>> object) override {
        // Project 3D bounds to 2D (using XZ plane by default)
        const AABB& bounds = object->getBounds();
        if (!root_) {
            root_ = std::make_unique<Node>();
            root_->bounds = AABB2D(bounds);
        } else {
            root_->bounds.extend(AABB2D(bounds));
        }
        
        // Store in object map
        const SpatialObject<T>* objPtr = object.get();
        std::size_t id = reinterpret_cast<std::size_t>(objPtr);
        objectMap_[id] = std::move(object);
        
        insertIntoNode(root_.get(), objPtr);
        objectCount_++;
    }
    
    void remove(const SpatialObject<T>* object) override {
        if (!root_) return;
        removeFromNode(root_.get(), object);
        objectCount_--;
        
        if (objectCount_ > 0 && shouldShrink()) {
            shrinkTree();
        }
    }
    
    void update(const SpatialObject<T>* object) override {
        if (!root_) return;
        
        std::size_t id = reinterpret_cast<std::size_t>(object);
        auto it = objectMap_.find(id);
        
        if (it != objectMap_.end()) {
            // Remove from spatial structure but keep in map
            auto obj = std::move(it->second);
            removeFromNode(root_.get(), object);
            
            // Reinsert into spatial structure
            insertIntoNode(root_.get(), object);
        }
    }
    
    void clear() override {
        root_.reset();
        objectMap_.clear();
        objectCount_ = 0;
    }
    
    void query(const SpatialQuery<T>& query,
               std::vector<const SpatialObject<T>*>& results) const override {
        if (!root_) return;
        queryNode(root_.get(), query, results);
    }
    
    void optimize() override {
        if (!root_) return;
        if (shouldRebalance()) {
            rebuild();
        }
    }
    
    void rebuild() override {
        if (objectCount_ == 0) return;
        
        std::vector<std::unique_ptr<SpatialObject<T>>> objects;
        objects.reserve(objectCount_);
        collectObjects(root_.get(), objects);
        
        clear();
        for (auto& obj : objects) {
            insert(std::move(obj));
        }
    }
    
    std::size_t getObjectCount() const override { return objectCount_; }
    std::size_t getNodeCount() const override { return countNodes(root_.get()); }
    std::size_t getMaxDepth() const override { return calculateMaxDepth(root_.get()); }
    float getAverageObjectsPerNode() const override {
        std::size_t nodeCount = getNodeCount();
        return nodeCount ? static_cast<float>(objectCount_) / nodeCount : 0.0f;
    }
    
    void debugDraw(const std::function<void(const AABB&)>& drawAABB) const override {
        if (!root_) return;
        debugDrawNode(root_.get(), drawAABB);
    }

private:
    static constexpr std::size_t NUM_CHILDREN = 4;
    
    // 2D AABB for quadtree partitioning (XZ plane)
    struct AABB2D {
        Vector2 center;
        Vector2 extent;
        
        AABB2D() = default;
        explicit AABB2D(const AABB& aabb) {
            center = Vector2(aabb.center().x, aabb.center().z);
            extent = Vector2(aabb.dimensions().x, aabb.dimensions().z);
        }
        
        void extend(const AABB2D& other) {
            Vector2 min = center - extent;
            Vector2 max = center + extent;
            Vector2 otherMin = other.center - other.extent;
            Vector2 otherMax = other.center + other.extent;
            
            min = math::min(min, otherMin);
            max = math::max(max, otherMax);
            
            center = (min + max) * 0.5f;
            extent = (max - min) * 0.5f;
        }
        
        bool contains(const AABB2D& other) const {
            Vector2 min = center - extent;
            Vector2 max = center + extent;
            Vector2 otherMin = other.center - other.extent;
            Vector2 otherMax = other.center + other.extent;
            
            return min.x <= otherMin.x && min.y <= otherMin.y &&
                   max.x >= otherMax.x && max.y >= otherMax.y;
        }
        
        bool contains(const Vector2& point) const {
            Vector2 min = center - extent;
            Vector2 max = center + extent;
            return point.x >= min.x && point.x <= max.x &&
                   point.y >= min.y && point.y <= max.y;
        }
        
        float getMinExtent() const {
            return std::min(extent.x, extent.y);
        }
        
        AABB to3D(float minY, float maxY) const {
            return AABB(
                Vector3(center.x, (minY + maxY) * 0.5f, center.y),
                Vector3(extent.x, (maxY - minY) * 0.5f, extent.y)
            );
        }
    };
    
    struct Node {
        AABB2D bounds;
        std::array<std::unique_ptr<Node>, NUM_CHILDREN> children;
        std::vector<const SpatialObject<T>*> objects;
        bool isLeaf() const {
            return std::all_of(children.begin(), children.end(),
                             [](const auto& child) { return !child; });
        }
    };
    
    void insertIntoNode(Node* node, const SpatialObject<T>* object) {
        const AABB2D objBounds(object->getBounds());
        
        if (calculateDepth(node) >= config_.maxDepth ||
            node->bounds.getMinExtent() <= config_.minNodeSize) {
            node->objects.push_back(object);
            return;
        }
        
        std::size_t index = getChildIndex(node->bounds, objBounds.center);
        if (node->isLeaf()) {
            if (node->objects.size() >= config_.maxObjectsPerNode) {
                splitNode(node);
            } else {
                node->objects.push_back(object);
                return;
            }
        }
        
        if (!node->children[index]) {
            node->children[index] = std::make_unique<Node>();
            node->children[index]->bounds = computeChildBounds(node->bounds, index);
        }
        
        // Store in current node if object is too big for child
        const AABB& obj3dBounds = object->getBounds();
        AABB childBounds3D = node->children[index]->bounds.to3D(
            obj3dBounds.min.y, obj3dBounds.max.y);
        
        if (containsAABB(childBounds3D, obj3dBounds)) {
            insertIntoNode(node->children[index].get(), std::move(object));
        } else {
            node->objects.push_back(object);
        }
    }
    
    void removeFromNode(Node* node, const SpatialObject<T>* object) {
        if (!node) return;
        
        auto it = std::find_if(node->objects.begin(), node->objects.end(),
            [object](const auto& obj) { return obj == object; });
        
        if (it != node->objects.end()) {
            node->objects.erase(it);
            return;
        }
        
        if (!node->isLeaf()) {
            for (auto& child : node->children) {
                if (child) removeFromNode(child.get(), object);
            }
        }
        
        tryMergeNode(node);
    }
    
    void queryNode(const Node* node, const SpatialQuery<T>& query,
                  std::vector<const SpatialObject<T>*>& results) const {
        if (!node) return;
        
        // For point queries, determine Y bounds from the query point
        float minY = -std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::max();
        
        if (auto* pointQuery = dynamic_cast<const PointQuery<T>*>(&query)) {
            const Vector3& point = pointQuery->getPoint();
            minY = point.y - config_.minNodeSize;
            maxY = point.y + config_.minNodeSize;
        } else if (auto* volumeQuery = dynamic_cast<const VolumeQuery<T>*>(&query)) {
            const AABB& bounds = volumeQuery->getBounds();
            minY = bounds.min.y;
            maxY = bounds.max.y;
        }
        
        // Convert 2D bounds to 3D for query
        AABB bounds3D = node->bounds.to3D(minY, maxY);
        
        if (!query.shouldTraverseNode(bounds3D)) return;
        
        for (const auto* obj : node->objects) {
            if (query.shouldAcceptObject(*obj)) {
                results.push_back(obj);
            }
        }
        
        if (!node->isLeaf()) {
            for (const auto& child : node->children) {
                if (child) queryNode(child.get(), query, results);
            }
        }
    }
    
    void splitNode(Node* node) {
        if (!node->isLeaf()) return;
        
        std::vector<const SpatialObject<T>*> remainingObjects;
        for (auto* obj : node->objects) {
            const AABB2D objBounds(obj->getBounds());
            std::size_t index = getChildIndex(node->bounds, objBounds.center);
            
            if (!node->children[index]) {
                node->children[index] = std::make_unique<Node>();
                node->children[index]->bounds = computeChildBounds(node->bounds, index);
            }
            
            if (node->children[index]->bounds.contains(objBounds)) {
                node->children[index]->objects.push_back(obj);
            } else {
                remainingObjects.push_back(obj);
            }
        }
        
        node->objects = std::move(remainingObjects);
    }
    
    void tryMergeNode(Node* node) {
        if (node->isLeaf()) return;
        
        std::size_t totalObjects = node->objects.size();
        for (const auto& child : node->children) {
            if (child) totalObjects += child->objects.size();
        }
        
        if (totalObjects <= config_.maxObjectsPerNode) {
            std::vector<const SpatialObject<T>*> allObjects;
            allObjects.reserve(totalObjects);
            
            for (auto* obj : node->objects) {
                allObjects.push_back(obj);
            }
            
            for (auto& child : node->children) {
                if (child) {
                    for (auto* obj : child->objects) {
                        allObjects.push_back(obj);
                    }
                    child.reset();
                }
            }
            
            node->objects = std::move(allObjects);
        }
    }
    
    static std::size_t getChildIndex(const AABB2D& parentBounds, const Vector2& point) {
        std::size_t index = 0;
        if (point.x >= parentBounds.center.x) index |= 1;
        if (point.y >= parentBounds.center.y) index |= 2;
        return index;
    }
    
    static AABB2D computeChildBounds(const AABB2D& parentBounds, std::size_t index) {
        Vector2 offset(
            (index & 1) ? parentBounds.extent.x : -parentBounds.extent.x,
            (index & 2) ? parentBounds.extent.y : -parentBounds.extent.y
        );
        
        AABB2D childBounds;
        childBounds.center = parentBounds.center + offset * 0.5f;
        childBounds.extent = parentBounds.extent * 0.5f;
        return childBounds;
    }
    
    std::size_t calculateDepth(const Node* node) const {
        std::size_t depth = 0;
        const Node* current = node;
        while (current != root_.get()) {
            current = findParent(current);
            if (!current) break;
            depth++;
        }
        return depth;
    }
    
    const Node* findParent(const Node* node) const {
        if (!root_ || node == root_.get()) return nullptr;
        return findParentRecursive(root_.get(), node);
    }
    
    const Node* findParentRecursive(const Node* parent, const Node* target) const {
        if (!parent) return nullptr;
        
        for (const auto& child : parent->children) {
            if (child.get() == target) return parent;
            const Node* result = findParentRecursive(child.get(), target);
            if (result) return result;
        }
        
        return nullptr;
    }
    
    bool shouldRebalance() const {
        if (objectCount_ < 100) return false;
        return getMaxDepth() > 2 * calculateOptimalDepth();
    }
    
    bool shouldShrink() const {
        if (!root_ || root_->isLeaf()) return false;
        return getMaxDepth() < calculateOptimalDepth() / 2;
    }
    
    std::size_t calculateOptimalDepth() const {
        return static_cast<std::size_t>(std::log2(objectCount_ / config_.maxObjectsPerNode) / std::log2(4)) + 1;
    }
    
    void shrinkTree() {
        while (root_ && !root_->isLeaf()) {
            Node* target = nullptr;
            std::size_t count = 0;
            
            for (const auto& child : root_->children) {
                if (child) {
                    target = child.get();
                    count++;
                }
            }
            
            if (count != 1) break;
            
            std::unique_ptr<Node> newRoot =
                std::move(root_->children[getChildIndex(root_->bounds, target->bounds.center)]);
            root_ = std::move(newRoot);
        }
    }
    
    std::size_t countNodes(const Node* node) const {
        if (!node) return 0;
        std::size_t count = 1;
        for (const auto& child : node->children) {
            if (child) count += countNodes(child.get());
        }
        return count;
    }
    
    std::size_t calculateMaxDepth(const Node* node) const {
        if (!node) return 0;
        std::size_t maxDepth = 0;
        for (const auto& child : node->children) {
            if (child) {
                maxDepth = std::max(maxDepth, calculateMaxDepth(child.get()));
            }
        }
        return 1 + maxDepth;
    }
    
    void debugDrawNode(const Node* node, const std::function<void(const AABB&)>& drawAABB) const {
        if (!node) return;
        
        // Convert 2D bounds to 3D for visualization
        drawAABB(node->bounds.to3D(-1.0f, 1.0f));  // Arbitrary Y range for visualization
        
        for (const auto& child : node->children) {
            if (child) debugDrawNode(child.get(), drawAABB);
        }
    }

    static bool containsAABB(const AABB& outer, const AABB& inner) {
        return outer.min.x <= inner.min.x && outer.max.x >= inner.max.x &&
               outer.min.y <= inner.min.y && outer.max.y >= inner.max.y &&
               outer.min.z <= inner.min.z && outer.max.z >= inner.max.z;
    }
    
    void collectObjects(Node* node, std::vector<std::unique_ptr<SpatialObject<T>>>& objects) {
        if (!node) return;
        
        // Move unique_ptrs from objectMap_ to objects vector
        for (auto* obj : node->objects) {
            std::size_t id = reinterpret_cast<std::size_t>(obj);
            auto it = objectMap_.find(id);
            if (it != objectMap_.end()) {
                objects.push_back(std::move(it->second));
                objectMap_.erase(it);
            }
        }
        node->objects.clear();
        
        for (auto& child : node->children) {
            if (child) collectObjects(child.get(), objects);
        }
    }
    
    SpatialConfig config_;
    std::unique_ptr<Node> root_;
    std::size_t objectCount_;
    std::unordered_map<std::size_t, std::unique_ptr<SpatialObject<T>>> objectMap_;
};

} // namespace geometry
} // namespace math
} // namespace pynovage