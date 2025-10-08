#pragma once

#include "spatial_partitioning.hpp"
#include "../vector3.hpp"
#include "../matrix4.hpp"
#include <memory>
#include <vector>
#include <queue>

namespace pynovage {
namespace math {
namespace geometry {

template<typename T>
class BSPTree : public SpatialContainer<T> {
public:
    explicit BSPTree(const SpatialConfig& config = SpatialConfig())
        : config_(config), root_(nullptr), objectCount_(0) {}
    
    void insert(std::unique_ptr<SpatialObject<T>> object) override {
        if (!root_) {
            root_ = std::make_unique<Node>();
            root_->bounds = object->getBounds();
        } else {
            root_->bounds.extend(object->getBounds());
        }
        
        insertIntoNode(root_.get(), std::move(object));
        objectCount_++;
    }
    
    void remove(const SpatialObject<T>* object) override {
        if (!root_) return;
        removeFromNode(root_.get(), object);
        objectCount_--;
    }
    
    void update(const SpatialObject<T>* object) override {
        // For BSP trees, removal and reinsertion is often more efficient
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
        
        // Rebalance the tree if needed
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
    struct Node {
        AABB bounds;
        Plane splitPlane;
        std::unique_ptr<Node> front;
        std::unique_ptr<Node> back;
        std::vector<std::unique_ptr<SpatialObject<T>>> objects;
        bool isLeaf() const { return !front && !back; }
    };
    
    void insertIntoNode(Node* node, std::unique_ptr<SpatialObject<T>> object) {
        if (node->isLeaf()) {
            if (node->objects.size() < config_.maxTrianglesPerLeaf) {
                node->objects.push_back(std::move(object));
                return;
            }
            
            // Split node if it contains too many objects
            splitNode(node);
        }
        
        // Choose child based on split plane
        const AABB& objBounds = object->getBounds();
        if (node->splitPlane.isInFront(objBounds)) {
            insertIntoNode(node->front.get(), std::move(object));
        } else if (node->splitPlane.isBehind(objBounds)) {
            insertIntoNode(node->back.get(), std::move(object));
        } else {
            // Object spans the split plane, keep it in this node
            node->objects.push_back(std::move(object));
        }
    }
    
    void removeFromNode(Node* node, const SpatialObject<T>* object) {
        if (!node) return;
        
        auto it = std::find_if(node->objects.begin(), node->objects.end(),
            [object](const auto& obj) { return obj.get() == object; });
        
        if (it != node->objects.end()) {
            node->objects.erase(it);
            return;
        }
        
        // Recurse into children
        if (!node->isLeaf()) {
            removeFromNode(node->front.get(), object);
            removeFromNode(node->back.get(), object);
        }
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
        
        // Recurse into children if not a leaf
        if (!node->isLeaf()) {
            queryNode(node->front.get(), query, results);
            queryNode(node->back.get(), query, results);
        }
    }
    
    void splitNode(Node* node) {
        if (!node) return;
        
        // Choose split plane
        node->splitPlane = chooseSplitPlane(node);
        
        // Create child nodes
        node->front = std::make_unique<Node>();
        node->back = std::make_unique<Node>();
        
        // Distribute objects
        std::vector<std::unique_ptr<SpatialObject<T>>> remainingObjects;
        for (auto& obj : node->objects) {
            const AABB& bounds = obj->getBounds();
            if (node->splitPlane.isInFront(bounds)) {
                node->front->objects.push_back(std::move(obj));
            } else if (node->splitPlane.isBehind(bounds)) {
                node->back->objects.push_back(std::move(obj));
            } else {
                remainingObjects.push_back(std::move(obj));
            }
        }
        
        // Keep spanning objects in this node
        node->objects = std::move(remainingObjects);
        
        // Update child bounds
        node->front->bounds = node->bounds;
        node->back->bounds = node->bounds;
        node->splitPlane.split(node->front->bounds, node->back->bounds);
    }
    
    Plane chooseSplitPlane(const Node* node) const {
        // Simple middle split strategy
        const Vector3& center = node->bounds.getCenter();
        const Vector3& extent = node->bounds.getExtent();
        
        // Choose axis with largest extent
        if (extent.x > extent.y && extent.x > extent.z) {
            return Plane(Vector3(1, 0, 0), center);
        } else if (extent.y > extent.z) {
            return Plane(Vector3(0, 1, 0), center);
        } else {
            return Plane(Vector3(0, 0, 1), center);
        }
    }
    
    bool shouldRebalance() const {
        if (objectCount_ < 100) return false;
        float balance = calculateBalance(root_.get());
        return balance < 0.3f || balance > 0.7f;
    }
    
    float calculateBalance(const Node* node) const {
        if (!node || node->isLeaf()) return 0.5f;
        
        std::size_t frontCount = countObjects(node->front.get());
        std::size_t totalCount = frontCount + countObjects(node->back.get());
        return totalCount > 0 ? static_cast<float>(frontCount) / totalCount : 0.5f;
    }
    
    std::size_t countObjects(const Node* node) const {
        if (!node) return 0;
        std::size_t count = node->objects.size();
        if (!node->isLeaf()) {
            count += countObjects(node->front.get());
            count += countObjects(node->back.get());
        }
        return count;
    }
    
    std::size_t countNodes(const Node* node) const {
        if (!node) return 0;
        return 1 + countNodes(node->front.get()) + countNodes(node->back.get());
    }
    
    std::size_t calculateMaxDepth(const Node* node) const {
        if (!node) return 0;
        return 1 + std::max(calculateMaxDepth(node->front.get()),
                          calculateMaxDepth(node->back.get()));
    }
    
    void debugDrawNode(const Node* node, const std::function<void(const AABB&)>& drawAABB) const {
        if (!node) return;
        drawAABB(node->bounds);
        if (!node->isLeaf()) {
            debugDrawNode(node->front.get(), drawAABB);
            debugDrawNode(node->back.get(), drawAABB);
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
        if (!node->isLeaf()) {
            collectObjects(node->front.get(), objects);
            collectObjects(node->back.get(), objects);
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