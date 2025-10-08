#include <gtest/gtest.h>
#include "../../include/geometry/frustum_culling.hpp"
#include "../../include/matrix4.hpp"
#include "../../include/vector3.hpp"
#include <chrono>

namespace pynovage {
namespace math {
namespace geometry {
namespace tests {

class FrustumCullingTest : public testing::Test {
protected:
    void SetUp() override {
        // Create a typical perspective view-projection matrix
        Matrix4 projection = Matrix4::perspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f);
        Matrix4 view = Matrix4::lookAt(Vector3(0, 0, -10), Vector3(0, 0, 0), Vector3(0, 1, 0));
        viewProjection_ = projection * view;
        frustum_ = std::make_unique<FrustumCulling>(viewProjection_);
    }
    
    Matrix4 viewProjection_;
    std::unique_ptr<FrustumCulling> frustum_;
};

TEST_F(FrustumCullingTest, PointTest) {
    // Point at origin (should be inside)
    EXPECT_EQ(frustum_->testPoint(Vector3(0, 0, 0)), FrustumCulling::TestResult::INSIDE);
    
    // Point far outside (should be outside)
    EXPECT_EQ(frustum_->testPoint(Vector3(0, 0, -1000)), FrustumCulling::TestResult::OUTSIDE);
}

TEST_F(FrustumCullingTest, SphereTest) {
    // Sphere at origin
    EXPECT_EQ(frustum_->testSphere(Vector3(0, 0, 0), 1.0f), FrustumCulling::TestResult::INSIDE);
    
    // Sphere partially intersecting
    EXPECT_EQ(frustum_->testSphere(Vector3(0, 5, 0), 2.0f), FrustumCulling::TestResult::INTERSECT);
    
    // Sphere completely outside
    EXPECT_EQ(frustum_->testSphere(Vector3(0, 20, 0), 1.0f), FrustumCulling::TestResult::OUTSIDE);
}

TEST_F(FrustumCullingTest, AABBTest) {
    // AABB at origin
    AABB aabb(Vector3(0, 0, 0), Vector3(1, 1, 1));
    EXPECT_EQ(frustum_->testAABB(aabb), FrustumCulling::TestResult::INSIDE);
    
    // AABB partially intersecting
    AABB intersectingAABB(Vector3(0, 5, 0), Vector3(2, 2, 2));
    EXPECT_EQ(frustum_->testAABB(intersectingAABB), FrustumCulling::TestResult::INTERSECT);
    
    // AABB completely outside
    AABB outsideAABB(Vector3(0, 20, 0), Vector3(1, 1, 1));
    EXPECT_EQ(frustum_->testAABB(outsideAABB), FrustumCulling::TestResult::OUTSIDE);
}

TEST_F(FrustumCullingTest, AABBSIMDTest) {
    // AABB at origin
    AABB aabb(Vector3(0, 0, 0), Vector3(1, 1, 1));
    EXPECT_EQ(frustum_->testAABBSIMD(aabb), FrustumCulling::TestResult::INSIDE);
    
    // AABB partially intersecting
    AABB intersectingAABB(Vector3(0, 5, 0), Vector3(2, 2, 2));
    EXPECT_EQ(frustum_->testAABBSIMD(intersectingAABB), FrustumCulling::TestResult::INTERSECT);
    
    // AABB completely outside
    AABB outsideAABB(Vector3(0, 20, 0), Vector3(1, 1, 1));
    EXPECT_EQ(frustum_->testAABBSIMD(outsideAABB), FrustumCulling::TestResult::OUTSIDE);
}

TEST_F(FrustumCullingTest, UpdateTest) {
    // Test initial state
    AABB aabb(Vector3(0, 0, 0), Vector3(1, 1, 1));
    EXPECT_EQ(frustum_->testAABB(aabb), FrustumCulling::TestResult::INSIDE);
    
    // Move camera and update frustum
    Matrix4 newView = Matrix4::lookAt(Vector3(20, 0, -10), Vector3(0, 0, 0), Vector3(0, 1, 0));
    Matrix4 newViewProj = viewProjection_ * newView;
    frustum_->update(newViewProj);
    
    // Same AABB should now be outside
    EXPECT_EQ(frustum_->testAABB(aabb), FrustumCulling::TestResult::OUTSIDE);
}

// Performance Tests
TEST_F(FrustumCullingTest, PointTestPerformance) {
    const int NUM_TESTS = 1000000;
    std::vector<Vector3> points;
    points.reserve(NUM_TESTS);
    
    // Generate random points
    for (int i = 0; i < NUM_TESTS; ++i) {
        points.emplace_back(
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f,
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f,
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f
        );
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& point : points) {
        frustum_->testPoint(point);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Point test performance: " << duration.count() / static_cast<float>(NUM_TESTS)
              << " microseconds per test" << std::endl;
}

TEST_F(FrustumCullingTest, AABBTestPerformance) {
    const int NUM_TESTS = 1000000;
    std::vector<AABB> boxes;
    boxes.reserve(NUM_TESTS);
    
    // Generate random AABBs
    for (int i = 0; i < NUM_TESTS; ++i) {
        Vector3 center(
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f,
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f,
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f
        );
        Vector3 extent(
            static_cast<float>(rand()) / RAND_MAX * 2.0f,
            static_cast<float>(rand()) / RAND_MAX * 2.0f,
            static_cast<float>(rand()) / RAND_MAX * 2.0f
        );
        boxes.emplace_back(center, extent);
    }
    
    // Test regular AABB intersection
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& box : boxes) {
        frustum_->testAABB(box);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "AABB test performance: " << duration.count() / static_cast<float>(NUM_TESTS)
              << " microseconds per test" << std::endl;
    
    // Test SIMD AABB intersection
    start = std::chrono::high_resolution_clock::now();
    for (const auto& box : boxes) {
        frustum_->testAABBSIMD(box);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "AABB SIMD test performance: " << duration.count() / static_cast<float>(NUM_TESTS)
              << " microseconds per test" << std::endl;
}

TEST_F(FrustumCullingTest, HierarchicalCullingTest) {
    // Mock node class for testing
    class MockNode {
    public:
        MockNode(const AABB& bounds) : bounds_(bounds) {}
        void addChild(std::unique_ptr<MockNode> child) { children_.push_back(std::move(child)); }
        const AABB& getBounds() const { return bounds_; }
        const std::vector<std::unique_ptr<MockNode>>& getChildren() const { return children_; }
    
    private:
        AABB bounds_;
        std::vector<std::unique_ptr<MockNode>> children_;
    };
    
    // Create a test hierarchy
    auto root = std::make_unique<MockNode>(AABB(Vector3(0, 0, 0), Vector3(10, 10, 10)));
    
    // Add some children
    root->addChild(std::make_unique<MockNode>(AABB(Vector3(-5, 0, 0), Vector3(2, 2, 2))));
    root->addChild(std::make_unique<MockNode>(AABB(Vector3(5, 0, 0), Vector3(2, 2, 2))));
    root->addChild(std::make_unique<MockNode>(AABB(Vector3(0, 5, 0), Vector3(2, 2, 2))));
    
    // Create hierarchical culling
    HierarchicalFrustumCulling hierarchicalCulling(viewProjection_);
    
    // Test culling
    int visibleCount = 0;
    hierarchicalCulling.testHierarchy(*root, [&visibleCount](const MockNode&) {
        visibleCount++;
    });
    
    EXPECT_GT(visibleCount, 0);
}

TEST_F(FrustumCullingTest, HierarchicalCullingPerformance) {
    // Create a large test hierarchy
    class MockNode {
    public:
        MockNode(const AABB& bounds) : bounds_(bounds) {}
        void addChild(std::unique_ptr<MockNode> child) { children_.push_back(std::move(child)); }
        const AABB& getBounds() const { return bounds_; }
        const std::vector<std::unique_ptr<MockNode>>& getChildren() const { return children_; }
    
    private:
        AABB bounds_;
        std::vector<std::unique_ptr<MockNode>> children_;
    };
    
    // Create a deep hierarchy with many nodes
    std::function<std::unique_ptr<MockNode>(const Vector3&, float, int)> createHierarchy;
    createHierarchy = [&](const Vector3& center, float size, int depth) -> std::unique_ptr<MockNode> {
        auto node = std::make_unique<MockNode>(AABB(center, Vector3(size)));
        
        if (depth > 0) {
            float childSize = size * 0.5f;
            float offset = childSize;
            
            // Create 8 children in octree-like pattern
            for (int x = -1; x <= 1; x += 2) {
                for (int y = -1; y <= 1; y += 2) {
                    for (int z = -1; z <= 1; z += 2) {
                        Vector3 childCenter = center + Vector3(x, y, z) * offset;
                        node->addChild(createHierarchy(childCenter, childSize, depth - 1));
                    }
                }
            }
        }
        
        return node;
    };
    
    // Create a 4-level hierarchy (4^3 = 64 leaf nodes)
    auto root = createHierarchy(Vector3(0), 10.0f, 4);
    
    // Test performance
    HierarchicalFrustumCulling hierarchicalCulling(viewProjection_);
    
    auto start = std::chrono::high_resolution_clock::now();
    int visibleCount = 0;
    for (int i = 0; i < 1000; ++i) {
        hierarchicalCulling.testHierarchy(*root, [&visibleCount](const MockNode&) {
            visibleCount++;
        });
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::microseconds>(end - start);
    
    std::cout << "Hierarchical culling performance: " << duration.count() / 1000.0f
              << " microseconds per traversal" << std::endl;
    std::cout << "Average visible nodes: " << visibleCount / 1000.0f << std::endl;
}

} // namespace tests
} // namespace geometry
} // namespace math
} // namespace pynovage