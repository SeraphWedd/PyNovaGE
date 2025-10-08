#include <benchmark/benchmark.h>
#include "geometry/spatial_partitioning.hpp"
#include "geometry/bsp_tree.hpp"
#include "geometry/octree.hpp"
#include "geometry/quadtree.hpp"
#include "geometry/spatial_hash.hpp"
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
void BM_BulkInsert(benchmark::State& state) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> sizeDist(0.5f, 5.0f);
    const int count = state.range(0);
    
    for (auto _ : state) {
        state.PauseTiming();
        Container container;
        container.reserveObjects(count);
        std::vector<std::unique_ptr<MockObject>> objects;
        objects.reserve(count);
        
        for (int i = 0; i < count; ++i) {
            Vector3 center(posDist(gen), posDist(gen), posDist(gen));
            float size = sizeDist(gen);
            Vector3 halfExtent(size, size, size);
            AABB bounds(center - halfExtent, center + halfExtent);
            objects.push_back(std::make_unique<MockObject>(bounds, i));
        }
        state.ResumeTiming();
        
        for (auto& obj : objects) {
            container.insert(std::move(obj));
        }
    }
    
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * count);
}

template<typename Container>
void BM_Query(benchmark::State& state) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> sizeDist(0.5f, 5.0f);
    const int datasetSize = state.range(0);
    
    Container container;
    std::vector<std::unique_ptr<MockObject>> objects;
    objects.reserve(datasetSize);
    
    // Setup dataset
    for (int i = 0; i < datasetSize; ++i) {
        Vector3 center(posDist(gen), posDist(gen), posDist(gen));
        float size = sizeDist(gen);
        Vector3 halfExtent(size, size, size);
        AABB bounds(center - halfExtent, center + halfExtent);
        auto obj = std::make_unique<MockObject>(bounds, i);
        container.insert(std::move(obj));
    }
    
    std::vector<const SpatialObject<int>*> results;
    for (auto _ : state) {
        Vector3 center(posDist(gen), posDist(gen), posDist(gen));
        float size = sizeDist(gen);
        Vector3 halfExtent(size, size, size);
        AABB queryBounds(center - halfExtent, center + halfExtent);
        
        results.clear();
        VolumeQuery<int> query(queryBounds);
        container.query(query, results);
    }
}

template<typename Container>
void BM_Update(benchmark::State& state) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> sizeDist(0.5f, 5.0f);
    std::uniform_real_distribution<float> offsetDist(-10.0f, 10.0f);
    const int datasetSize = state.range(0);
    
    Container container;
    std::vector<const SpatialObject<int>*> objects;
    objects.reserve(datasetSize);
    
    // Setup dataset
    for (int i = 0; i < datasetSize; ++i) {
        Vector3 center(posDist(gen), posDist(gen), posDist(gen));
        float size = sizeDist(gen);
        Vector3 halfExtent(size, size, size);
        AABB bounds(center - halfExtent, center + halfExtent);
        auto obj = std::make_unique<MockObject>(bounds, i);
        objects.push_back(obj.get());
        container.insert(std::move(obj));
    }
    
    std::uniform_int_distribution<size_t> objDist(0, objects.size() - 1);
    
    for (auto _ : state) {
        const SpatialObject<int>* obj = objects[objDist(gen)];
        Vector3 offset(offsetDist(gen), offsetDist(gen), offsetDist(gen));
        
        AABB bounds = obj->getBounds();
        Vector3 center = bounds.center() + offset;
        Vector3 halfExtent = bounds.dimensions() * 0.5f;
        AABB newBounds(center - halfExtent, center + halfExtent);
        
        // Create new object in the objectMap by re-inserting
        container.update(obj);
    }
}

} // namespace

// Register BSP Tree benchmarks
BENCHMARK_TEMPLATE(BM_BulkInsert, BSPTree<int>)
    ->Arg(1)
    ->Arg(10)
    ->Arg(100)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Query, BSPTree<int>)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Update, BSPTree<int>)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);

// Register Octree benchmarks
BENCHMARK_TEMPLATE(BM_BulkInsert, Octree<int>)
    ->Arg(1)
    ->Arg(10)
    ->Arg(100)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Query, Octree<int>)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Update, Octree<int>)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);

// Register Quadtree benchmarks
BENCHMARK_TEMPLATE(BM_BulkInsert, Quadtree<int>)
    ->Arg(1)
    ->Arg(10)
    ->Arg(100)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Query, Quadtree<int>)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Update, Quadtree<int>)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);

// Register SpatialHash benchmarks
BENCHMARK_TEMPLATE(BM_BulkInsert, SpatialHash<int>)
    ->Arg(1)
    ->Arg(10)
    ->Arg(100)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Query, SpatialHash<int>)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_Update, SpatialHash<int>)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);
