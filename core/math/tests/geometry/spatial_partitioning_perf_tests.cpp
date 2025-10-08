#include <gtest/gtest.h>
#include "geometry/spatial_partitioning.hpp"
#include "geometry/bsp_tree.hpp"
#include "geometry/octree.hpp"
#include "geometry/quadtree.hpp"
#include "geometry/spatial_hash.hpp"
#include <chrono>
#include <random>
#include <vector>

using namespace pynovage::math;
using namespace pynovage::math::geometry;

namespace {

class MockObject : public SpatialObject<int> {
public:
    explicit MockObject(const AABB& bounds, int data = 0)
        : bounds_(bounds), data_(data) {}
    
    AABB getBounds() const override { return bounds_; }
    bool intersects(const AABB& bounds) const override {
        return aabbAABBIntersection(bounds_, bounds).has_value();
    }
    bool contains(const Vector3& point) const override {
        return point.x >= bounds_.min.x && point.x <= bounds_.max.x &&
               point.y >= bounds_.min.y && point.y <= bounds_.max.y &&
               point.z >= bounds_.min.z && point.z <= bounds_.max.z;
    }
    bool intersectsRay(const Ray3D& ray, float& t) const override {
        auto result = rayAABBIntersection(ray, bounds_);
        if (result) {
            t = result->distance;
            return true;
        }
        return false;
    }
    bool intersectsFrustum(const FrustumCulling& frustum) const override {
        return frustum.testAABB(bounds_) != FrustumCulling::TestResult::OUTSIDE;
    }
    
    const int& getData() const override { return data_; }
    int& getData() override { return data_; }

private:
    AABB bounds_;
    int data_;
};

template<typename Container>
class SpatialPartitioningPerfTest : public testing::Test {
protected:
    void SetUp() override {
        container_ = std::make_unique<Container>();
    }
    
    void generateRandomObjects(int count, float minPos = -100.0f, float maxPos = 100.0f, 
                             float minSize = 0.5f, float maxSize = 5.0f) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(minPos, maxPos);
        std::uniform_real_distribution<float> sizeDist(minSize, maxSize);
        
        objects_.clear();
        objects_.reserve(count);
        
        for (int i = 0; i < count; ++i) {
            Vector3 center(posDist(gen), posDist(gen), posDist(gen));
            float size = sizeDist(gen);
            Vector3 halfExtent(size, size, size);
            AABB bounds(center - halfExtent, center + halfExtent);
            auto obj = std::make_unique<MockObject>(bounds, i);
            objects_.push_back(obj.get());
            container_->insert(std::move(obj));
        }
    }
    
    void testBulkInsert(int count) {
        auto start = std::chrono::steady_clock::now();
        generateRandomObjects(count);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Bulk insert of " << count << " objects: "
                  << duration.count() << "us (" 
                  << (duration.count() / static_cast<float>(count)) << "us per object)\n";
    }
    
    void testRandomQueries(int queryCount) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-100.0f, 100.0f);
        std::uniform_real_distribution<float> sizeDist(1.0f, 10.0f);
        
        auto start = std::chrono::steady_clock::now();
        std::vector<const SpatialObject<int>*> results;
        
        for (int i = 0; i < queryCount; ++i) {
            Vector3 center(posDist(gen), posDist(gen), posDist(gen));
            float size = sizeDist(gen);
            Vector3 halfExtent(size, size, size);
            AABB queryBounds(center - halfExtent, center + halfExtent);
            
            results.clear();
            VolumeQuery<int> query(queryBounds);
            container_->query(query, results);
        }
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << queryCount << " volume queries: "
                  << duration.count() << "us ("
                  << (duration.count() / static_cast<float>(queryCount)) << "us per query)\n";
    }
    
    void testRandomUpdates(int updateCount) {
        if (objects_.empty()) return;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> offsetDist(-10.0f, 10.0f);
        std::uniform_int_distribution<size_t> objDist(0, objects_.size() - 1);
        
        auto start = std::chrono::steady_clock::now();
        
        for (int i = 0; i < updateCount; ++i) {
            const SpatialObject<int>* obj = objects_[objDist(gen)];
            Vector3 offset(offsetDist(gen), offsetDist(gen), offsetDist(gen));
            
            AABB bounds = obj->getBounds();
            Vector3 center = bounds.center() + offset;
            Vector3 halfExtent = bounds.dimensions() * 0.5f;
            AABB newBounds(center - halfExtent, center + halfExtent);
            
            auto newObj = std::make_unique<MockObject>(newBounds, obj->getData());
            container_->update(obj);
        }
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << updateCount << " updates: "
                  << duration.count() << "us ("
                  << (duration.count() / static_cast<float>(updateCount)) << "us per update)\n";
    }

    std::unique_ptr<Container> container_;
    std::vector<const SpatialObject<int>*> objects_;
};

using SpatialPartitioningTypes = testing::Types<
    BSPTree<int>,
    Octree<int>,
    Quadtree<int>,
    SpatialHash<int>
>;

TYPED_TEST_SUITE(SpatialPartitioningPerfTest, SpatialPartitioningTypes);

TYPED_TEST(SpatialPartitioningPerfTest, BulkInsertPerformance) {
    std::cout << "\nBulk Insert Performance for " << typeid(TypeParam).name() << ":\n";
    this->testBulkInsert(100);
    this->testBulkInsert(1000);
    this->testBulkInsert(10000);
}

TYPED_TEST(SpatialPartitioningPerfTest, QueryPerformance) {
    std::cout << "\nQuery Performance for " << typeid(TypeParam).name() << ":\n";
    
    // Test with different dataset sizes
    this->generateRandomObjects(1000);
    this->testRandomQueries(1000);
    
    this->generateRandomObjects(10000);
    this->testRandomQueries(1000);
    
    this->generateRandomObjects(100000);
    this->testRandomQueries(1000);
}

TYPED_TEST(SpatialPartitioningPerfTest, UpdatePerformance) {
    std::cout << "\nUpdate Performance for " << typeid(TypeParam).name() << ":\n";
    
    // Test with different dataset sizes
    this->generateRandomObjects(1000);
    this->testRandomUpdates(1000);
    
    this->generateRandomObjects(10000);
    this->testRandomUpdates(1000);
    
    this->generateRandomObjects(100000);
    this->testRandomUpdates(1000);
}

} // namespace