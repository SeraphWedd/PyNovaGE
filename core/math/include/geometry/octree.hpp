#pragma once

#include "spatial_partitioning.hpp"
#include "../vector3.hpp"
#include "../matrix4.hpp"
#include <memory>
#include <vector>
#include <array>

namespace pynovage {
namespace math {
namespace geometry {

template<typename T>
class Octree : public SpatialContainer<T> {
public:
    explicit Octree(const SpatialConfig& config = SpatialConfig())
        : config_(config), root_(nullptr), objectCount_(0) {}
    
    void insert(std::unique_ptr<SpatialObject<T>> object) override {
        const AABB& bounds = object->getBounds();
        if (!root_) {
            // First object defines initial bounds
            root_ = std::make_unique<Node>();
            root_->bounds = AABB(bounds.getCenter(),
                               bounds.getExtent() * config_.looseness);
        } else if (!root_->bounds.contains(bounds)) {
            // Expand root to fit new object
            growTree(bounds);
        }
        
        insertIntoNode(root_.get(), std::move(object));
        objectCount_++;
    }
    
    void remove(const SpatialObject<T>* object) override {
        if (!root_) return;
        removeFromNode(root_.get(), object);
        objectCount_--;
        
        // Shrink tree if it's too empty
        if (objectCount_ > 0 && shouldShrink()) {
            shrinkTree();
        }
    }
    
    void update(const SpatialObject<T>* object) override {
        if (!root_) return;
        
        // Find and update object
        auto it = std::find_if(objectMap_.begin(), objectMap_.end(),
            [object](const auto& pair) { return pair.second.get() == object; });
        
        if (it != objectMap_.end()) {
            auto obj = std::move(it->second);
            remove(object);
            insert(std::move(obj));
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
        
        // Collect all objects
        std::vector<std::unique_ptr<SpatialObject<T>>> objects;
        objects.reserve(objectCount_);
        collectObjects(root_.get(), objects);
        
        // Clear and rebuild
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
    static constexpr std::size_t NUM_CHILDREN = 8;

    struct Node {
        AABB bounds;
        std::array<std::unique_ptr<Node>, NUM_CHILDREN> children;
        std::vector<std::unique_ptr<SpatialObject<T>>> objects;
        bool isLeaf() const { 
            return std::all_of(children.begin(), children.end(),
                             [](const auto& child) { return !child; });
        }
    };
    
    void insertIntoNode(Node* node, std::unique_ptr<SpatialObject<T>> object) {
        const AABB& objBounds = object->getBounds();
        
        // Stop if we've reached max depth or node is too small
        if (calculateDepth(node) >= config_.maxDepth ||
            node->bounds.getMinExtent() <= config_.minNodeSize) {
            node->objects.push_back(std::move(object));
            return;
        }
        
        // Check if object fits entirely in a child node
        std::size_t index = getChildIndex(node->bounds, objBounds.getCenter());
        if (node->isLeaf()) {
            if (node->objects.size() >= config_.maxObjectsPerNode) {
                splitNode(node);
            } else {
                node->objects.push_back(std::move(object));
                return;
            }
        }
        
        // If child doesn't exist, create it
        if (!node->children[index]) {
            node->children[index] = std::make_unique<Node>();
            node->children[index]->bounds = computeChildBounds(node->bounds, index);
        }
        
        // Insert into child if object fits entirely
        if (node->children[index]->bounds.contains(objBounds)) {
            insertIntoNode(node->children[index].get(), std::move(object));
        } else {
            // Object spans multiple children, keep it in this node
            node->objects.push_back(std::move(object));
        }
    }
    
    void removeFromNode(Node* node, const SpatialObject<T>* object) {
        if (!node) return;
        
        // Check objects in this node
        auto it = std::find_if(node->objects.begin(), node->objects.end(),
            [object](const auto& obj) { return obj.get() == object; });
        
        if (it != node->objects.end()) {
            node->objects.erase(it);
            return;
        }
        
        // Recurse into children that might contain the object
        if (!node->isLeaf()) {
            for (auto& child : node->children) {
                if (child) removeFromNode(child.get(), object);
            }
        }
        
        // Try to merge node if it's now empty
        tryMergeNode(node);
    }
    
    void queryNode(const Node* node, const SpatialQuery<T>& query,
                  std::vector<const SpatialObject<T>*>& results) const {
        if (!node || !query.shouldTraverseNode(node->bounds)) return;
        
        // Check objects in this node
        for (const auto& obj : node->objects) {
            if (query.shouldAcceptObject(*obj)) {
                results.push_back(obj.get());
            }
        }
        
        // Recurse into children
        if (!node->isLeaf()) {
            for (const auto& child : node->children) {
                if (child) queryNode(child.get(), query, results);
            }
        }
    }
    
    void splitNode(Node* node) {
        if (!node->isLeaf()) return;
        
        // Create child nodes and distribute objects
        std::vector<std::unique_ptr<SpatialObject<T>>> remainingObjects;
        for (auto& obj : node->objects) {
            const AABB& objBounds = obj->getBounds();
            std::size_t index = getChildIndex(node->bounds, objBounds.getCenter());
            
            if (!node->children[index]) {
                node->children[index] = std::make_unique<Node>();
                node->children[index]->bounds = computeChildBounds(node->bounds, index);
            }
            
            if (node->children[index]->bounds.contains(objBounds)) {
                node->children[index]->objects.push_back(std::move(obj));
            } else {
                remainingObjects.push_back(std::move(obj));
            }
        }
        
        // Keep objects that span multiple children in this node
        node->objects = std::move(remainingObjects);
    }
    
    void tryMergeNode(Node* node) {
        if (node->isLeaf()) return;
        
        // Count total objects in children
        std::size_t totalObjects = node->objects.size();
        for (const auto& child : node->children) {
            if (child) totalObjects += child->objects.size();
        }
        
        // Merge if total objects is below threshold
        if (totalObjects <= config_.maxObjectsPerNode) {
            // Collect all objects
            std::vector<std::unique_ptr<SpatialObject<T>>> allObjects;
            allObjects.reserve(totalObjects);
            for (auto& obj : node->objects) {
                allObjects.push_back(std::move(obj));
            }
            for (auto& child : node->children) {
                if (child) {
                    for (auto& obj : child->objects) {
                        allObjects.push_back(std::move(obj));
                    }
                }
            }
            
            // Clear children and store all objects in this node
            for (auto& child : node->children) {
                child.reset();
            }
            node->objects = std::move(allObjects);
        }
    }
    
    void growTree(const AABB& bounds) {
        // Create new root
        auto newRoot = std::make_unique<Node>();
        
        // Compute new bounds to contain both old root and new bounds
        newRoot->bounds = root_->bounds;
        newRoot->bounds.extend(bounds);
        
        // Scale by looseness factor
        Vector3 center = newRoot->bounds.getCenter();
        Vector3 extent = newRoot->bounds.getExtent() * config_.looseness;
        newRoot->bounds = AABB(center, extent);
        
        // Move old root to appropriate child position
        std::size_t index = getChildIndex(newRoot->bounds, root_->bounds.getCenter());
        newRoot->children[index] = std::move(root_);
        root_ = std::move(newRoot);
    }
    
    void shrinkTree() {
        while (root_ && !root_->isLeaf()) {
            // Find the child containing all objects
            Node* target = nullptr;
            std::size_t count = 0;
            
            for (const auto& child : root_->children) {
                if (child) {
                    target = child.get();
                    count++;
                }
            }
            
            if (count != 1) break;
            
            // Only one child contains objects, make it the new root
            std::unique_ptr<Node> newRoot = std::move(root_->children[getChildIndex(root_->bounds, target->bounds.getCenter())]);
            root_ = std::move(newRoot);
        }
    }
    
    static std::size_t getChildIndex(const AABB& parentBounds, const Vector3& point) {
        const Vector3& center = parentBounds.getCenter();
        std::size_t index = 0;
        if (point.x >= center.x) index |= 1;
        if (point.y >= center.y) index |= 2;
        if (point.z >= center.z) index |= 4;
        return index;
    }
    
    static AABB computeChildBounds(const AABB& parentBounds, std::size_t index) {
        const Vector3& center = parentBounds.getCenter();
        const Vector3& extent = parentBounds.getExtent() * 0.5f;
        
        Vector3 offset(
            (index & 1) ? extent.x : -extent.x,
            (index & 2) ? extent.y : -extent.y,
            (index & 4) ? extent.z : -extent.z
        );
        
        return AABB(center + offset * 0.5f, extent);
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
        return static_cast<std::size_t>(std::log2(objectCount_ / config_.maxObjectsPerNode) / std::log2(8)) + 1;
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
        std::size_t maxChildDepth = 0;
        for (const auto& child : node->children) {
            if (child) {
                maxChildDepth = std::max(maxChildDepth, calculateMaxDepth(child.get()));
            }
        }
        return 1 + maxChildDepth;
    }
    
    void debugDrawNode(const Node* node, const std::function<void(const AABB&)>& drawAABB) const {
        if (!node) return;
        drawAABB(node->bounds);
        for (const auto& child : node->children) {
            if (child) debugDrawNode(child.get(), drawAABB);
        }
    }
    
    void collectObjects(Node* node, std::vector<std::unique_ptr<SpatialObject<T>>>& objects) {
        if (!node) return;
        
        // Move objects from this node
        for (auto& obj : node->objects) {
            objects.push_back(std::move(obj));
        }
        node->objects.clear();
        
        // Recurse into children
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