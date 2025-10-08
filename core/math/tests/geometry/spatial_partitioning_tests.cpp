#include <gtest/gtest.h>
#include "../../include/geometry/spatial_partitioning.hpp"
#include "../../include/geometry/bsp_tree.hpp"
#include "../../include/geometry/octree.hpp"
#include "../../include/geometry/quadtree.hpp"
#include "../../include/geometry/spatial_hash.hpp"

namespace pynovage {
namespace math {
namespace geometry {
namespace tests {

// Mock object for testing
class MockObject : public SpatialObject<int> {
public:
    explicit MockObject(const AABB& bounds, int data = 0)
        : bounds_(bounds), data_(data) {}
    
    AABB getBounds() const override { return bounds_; }
    bool intersects(const AABB& bounds) const override { return bounds_.intersects(bounds); }
    bool contains(const Vector3& point) const override { return bounds_.contains(point); }
    
    const int& getData() const override { return data_; }
    int& getData() override { return data_; }

private:
    AABB bounds_;
    int data_;
};

// Common test fixture for all spatial partitioning implementations
template<typename Container>
class SpatialPartitioningTest : public testing::Test {
protected:
    void SetUp() override {
        container_ = std::make_unique<Container>();
    }
    
    void addObjects(int count, float spacing = 2.0f) {
        for (int i = 0; i < count; ++i) {
            float x = static_cast<float>(i % 10) * spacing;
            float y = static_cast<float>((i / 10) % 10) * spacing;
            float z = static_cast<float>(i / 100) * spacing;
            
            AABB bounds(Vector3(x, y, z), Vector3(0.5f));
            container_->insert(std::make_unique<MockObject>(bounds, i));
        }
    }
    
    std::unique_ptr<Container> container_;
};

using SpatialPartitioningTypes = testing::Types<
    BSPTree<int>,
    Octree<int>,
    Quadtree<int>,
    SpatialHash<int>
>;

TYPED_TEST_SUITE(SpatialPartitioningTest, SpatialPartitioningTypes);

TYPED_TEST(SpatialPartitioningTest, InsertAndCount) {
    this->addObjects(10);
    EXPECT_EQ(this->container_->getObjectCount(), 10);
}

TYPED_TEST(SpatialPartitioningTest, Clear) {
    this->addObjects(10);
    this->container_->clear();
    EXPECT_EQ(this->container_->getObjectCount(), 0);
}

TYPED_TEST(SpatialPartitioningTest, PointQuery) {
    this->addObjects(1000, 2.0f);
    
    std::vector<const SpatialObject<int>*> results;
    PointQuery<int> query(Vector3(1.0f, 1.0f, 1.0f));
    this->container_->query(query, results);
    
    EXPECT_GT(results.size(), 0);
}

TYPED_TEST(SpatialPartitioningTest, VolumeQuery) {
    this->addObjects(1000, 2.0f);
    
    std::vector<const SpatialObject<int>*> results;
    AABB queryBounds(Vector3(5.0f, 5.0f, 5.0f), Vector3(2.0f));
    VolumeQuery<int> query(queryBounds);
    this->container_->query(query, results);
    
    EXPECT_GT(results.size(), 0);
}

TYPED_TEST(SpatialPartitioningTest, RayQuery) {
    this->addObjects(1000, 2.0f);
    
    std::vector<const SpatialObject<int>*> results;
    Ray ray(Vector3(0.0f), Vector3(1.0f, 1.0f, 1.0f).normalized());
    RayQuery<int> query(ray, 100.0f);
    this->container_->query(query, results);
    
    EXPECT_GT(results.size(), 0);
}

TYPED_TEST(SpatialPartitioningTest, Update) {
    // Add a single object
    AABB initialBounds(Vector3(0.0f), Vector3(0.5f));
    auto obj = std::make_unique<MockObject>(initialBounds, 0);
    const SpatialObject<int>* objPtr = obj.get();
    this->container_->insert(std::move(obj));
    
    // Query initial position
    std::vector<const SpatialObject<int>*> results;
    PointQuery<int> query(Vector3(0.0f));
    this->container_->query(query, results);
    EXPECT_EQ(results.size(), 1);
    
    // Move object and update
    AABB newBounds(Vector3(10.0f), Vector3(0.5f));
    obj = std::make_unique<MockObject>(newBounds, 0);
    this->container_->update(objPtr);
    
    // Query new position
    results.clear();
    PointQuery<int> newQuery(Vector3(10.0f));
    this->container_->query(newQuery, results);
    EXPECT_EQ(results.size(), 1);
}

// Performance tests
TYPED_TEST(SpatialPartitioningTest, InsertionPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    this->addObjects(10000);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Insertion time for " << typeid(TypeParam).name() << ": "
              << duration.count() << "ms" << std::endl;
}

TYPED_TEST(SpatialPartitioningTest, QueryPerformance) {
    this->addObjects(10000);
    
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<const SpatialObject<int>*> results;
    for (int i = 0; i < 1000; ++i) {
        results.clear();
        AABB queryBounds(Vector3(i % 10 * 2.0f), Vector3(1.0f));
        VolumeQuery<int> query(queryBounds);
        this->container_->query(query, results);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Query time for " << typeid(TypeParam).name() << ": "
              << duration.count() << "ms" << std::endl;
}

TYPED_TEST(SpatialPartitioningTest, UpdatePerformance) {
    this->addObjects(10000);
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        AABB queryBounds(Vector3(i % 10 * 2.0f), Vector3(1.0f));
        std::vector<const SpatialObject<int>*> results;
        VolumeQuery<int> query(queryBounds);
        this->container_->query(query, results);
        
        for (const auto* obj : results) {
            this->container_->update(obj);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Update time for " << typeid(TypeParam).name() << ": "
              << duration.count() << "ms" << std::endl;
}

} // namespace tests
} // namespace geometry
} // namespace math
} // namespace pynovage