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
            const AABB& b = object->getBounds();
            Vector3 newMin(
                std::min(root_->bounds.min.x, b.min.x),
                std::min(root_->bounds.min.y, b.min.y),
                std::min(root_->bounds.min.z, b.min.z)
            );
            Vector3 newMax(
                std::max(root_->bounds.max.x, b.max.x),
                std::max(root_->bounds.max.y, b.max.y),
                std::max(root_->bounds.max.z, b.max.z)
            );
            root_->bounds = AABB(newMin, newMax);
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
        // Find the object in the map
        std::size_t id = reinterpret_cast<std::size_t>(object);
        auto it = objectMap_.find(id);
        
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
        std::vector<const SpatialObject<T>*> objects;
        bool isLeaf() const { return !front && !back; }
    };
    
    void insertIntoNode(Node* node, std::unique_ptr<SpatialObject<T>> object) {
        if (!node) return;
        
        const SpatialObject<T>* objPtr = object.get();
        std::size_t id = reinterpret_cast<std::size_t>(objPtr);
        objectMap_[id] = std::move(object);

        if (node->isLeaf()) {
            if (node->objects.size() < config_.maxTrianglesPerLeaf) {
                node->objects.push_back(objPtr);
                return;
            }
            
            // Split node if it contains too many objects
            splitNode(node);
        }
        
        // Choose child based on split plane
        const AABB& objBounds = objPtr->getBounds();
        float dist = node->splitPlane.signedDistance(objBounds.center());
        float epsilon = 1e-6f;  // Small epsilon to prevent floating point issues
        
        if (dist > epsilon) {
            node->objects.push_back(objPtr);
            if (node->front) {
                node->front->objects.push_back(objPtr);
            }
        } else if (dist < -epsilon) {
            node->objects.push_back(objPtr);
            if (node->back) {
                node->back->objects.push_back(objPtr);
            }
        } else {
            // Object spans the split plane, keep it in this node
            node->objects.push_back(objPtr);
        }
    }
    
    void removeFromNode(Node* node, const SpatialObject<T>* object) {
        if (!node) return;
        
        auto it = std::find(node->objects.begin(), node->objects.end(), object);
        if (it != node->objects.end()) {
            node->objects.erase(it);
        }
        
        // Remove from children
        if (!node->isLeaf()) {
            removeFromNode(node->front.get(), object);
            removeFromNode(node->back.get(), object);
        }
        
        // Remove from objectMap_
        std::size_t id = reinterpret_cast<std::size_t>(object);
        objectMap_.erase(id);
    }
    
    void queryNode(const Node* node, const SpatialQuery<T>& query,
                  std::vector<const SpatialObject<T>*>& results) const {
        if (!node || !query.shouldTraverseNode(node->bounds)) return;
        
        // Check objects in this node
        for (const auto* obj : node->objects) {
            if (query.shouldAcceptObject(*obj)) {
                // Avoid duplicates
                auto it = std::find(results.begin(), results.end(), obj);
                if (it == results.end()) {
                    results.push_back(obj);
                }
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
        
        // Create child nodes with divided bounds
        node->front = std::make_unique<Node>();
        node->back = std::make_unique<Node>();
        
        // Compute bounds for front and back based on split plane
        Vector3 center = node->bounds.center();
        Vector3 extent = node->bounds.dimensions() * 0.5f;
        Vector3 split_normal = node->splitPlane.normal;
        
        // Front node bounds
        Vector3 frontMin = node->bounds.min;
        Vector3 frontMax = node->bounds.max;
        if (split_normal.x != 0) frontMin.x = center.x;
        if (split_normal.y != 0) frontMin.y = center.y;
        if (split_normal.z != 0) frontMin.z = center.z;
        node->front->bounds = AABB(frontMin, frontMax);
        
        // Back node bounds
        Vector3 backMin = node->bounds.min;
        Vector3 backMax = node->bounds.max;
        if (split_normal.x != 0) backMax.x = center.x;
        if (split_normal.y != 0) backMax.y = center.y;
        if (split_normal.z != 0) backMax.z = center.z;
        node->back->bounds = AABB(backMin, backMax);
        
        // Distribute objects
        for (auto* obj : node->objects) {
            const AABB& bounds = obj->getBounds();
            float dist = node->splitPlane.signedDistance(bounds.center());
            float epsilon = 1e-6f;
            
            if (dist > epsilon) {
                node->front->objects.push_back(obj);
            } else if (dist < -epsilon) {
                node->back->objects.push_back(obj);
            }
            // Objects near the split plane stay in the current node
        }
    }
    
    Plane chooseSplitPlane(const Node* node) const {
        // Simple middle split strategy
        Vector3 center = node->bounds.center();
        Vector3 dims = node->bounds.dimensions();
        
        // Choose axis with largest dimension
        if (dims.x > dims.y && dims.x > dims.z) {
            return Plane::fromPointAndNormal(center, Vector3(1, 0, 0));
        } else if (dims.y > dims.z) {
            return Plane::fromPointAndNormal(center, Vector3(0, 1, 0));
        } else {
            return Plane::fromPointAndNormal(center, Vector3(0, 0, 1));
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