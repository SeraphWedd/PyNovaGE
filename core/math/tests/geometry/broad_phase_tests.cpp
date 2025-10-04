#include <gtest/gtest.h>
#include "geometry/broad_phase.hpp"

namespace pynovage {
namespace math {
namespace geometry {
namespace {

TEST(BroadPhaseTest, ProxyCreationAndDestruction) {
    BroadPhase bp(10.0f);
    
    // Create a simple AABB
    AABB aabb;
    aabb.min = Vector3(-1.0f, -1.0f, -1.0f);
    aabb.max = Vector3(1.0f, 1.0f, 1.0f);
    
    // Test dynamic proxy
    ProxyId dynamicId = bp.createProxy(aabb, false);
    EXPECT_NE(dynamicId, static_cast<ProxyId>(-1));
    
    // Test static proxy
    ProxyId staticId = bp.createProxy(aabb, true);
    EXPECT_NE(staticId, static_cast<ProxyId>(-1));
    
    // Destroy proxies
    bp.destroyProxy(dynamicId);
    bp.destroyProxy(staticId);
}

TEST(BroadPhaseTest, ProxyUpdate) {
    BroadPhase bp(10.0f);
    
    // Create an AABB
    AABB aabb;
    aabb.min = Vector3(0.0f, 0.0f, 0.0f);
    aabb.max = Vector3(1.0f, 1.0f, 1.0f);
    
    ProxyId id = bp.createProxy(aabb, false);
    
    // Move the AABB
    AABB newAabb;
    newAabb.min = Vector3(5.0f, 5.0f, 5.0f);
    newAabb.max = Vector3(6.0f, 6.0f, 6.0f);
    
    bp.updateProxy(id, newAabb);
    bp.finalizeBroadPhase();  // Ensure updates are processed
}

TEST(BroadPhaseTest, CollisionDetection) {
    BroadPhase bp(10.0f);
    
    // Create two overlapping AABBs
    AABB aabb1, aabb2;
    aabb1.min = Vector3(0.0f, 0.0f, 0.0f);
    aabb1.max = Vector3(2.0f, 2.0f, 2.0f);
    
    aabb2.min = Vector3(1.0f, 1.0f, 1.0f);
    aabb2.max = Vector3(3.0f, 3.0f, 3.0f);
    
    ProxyId id1 = bp.createProxy(aabb1, false);
    ProxyId id2 = bp.createProxy(aabb2, false);
    
    // Find collisions
    bp.finalizeBroadPhase();
    auto pairs = bp.findPotentialCollisions();
    
    // Should find exactly one collision pair
    EXPECT_EQ(pairs.size(), 1u);
    
    if (!pairs.empty()) {
        const auto& pair = pairs[0];
        // Order doesn't matter, but both IDs should be present
        EXPECT_TRUE((pair.a == id1 && pair.b == id2) ||
                    (pair.a == id2 && pair.b == id1));
    }
}

TEST(BroadPhaseTest, StaticDynamicInteraction) {
    BroadPhase bp(10.0f);
    
    // Create one static and one dynamic AABB
    AABB staticAabb, dynamicAabb;
    staticAabb.min = Vector3(-1.0f, -1.0f, -1.0f);
    staticAabb.max = Vector3(1.0f, 1.0f, 1.0f);
    
    dynamicAabb.min = Vector3(0.0f, 0.0f, 0.0f);
    dynamicAabb.max = Vector3(2.0f, 2.0f, 2.0f);
    
    ProxyId staticId = bp.createProxy(staticAabb, true);
    ProxyId dynamicId = bp.createProxy(dynamicAabb, false);
    
    // Find collisions
    bp.finalizeBroadPhase();
    auto pairs = bp.findPotentialCollisions();
    
    // Should find exactly one collision pair
    EXPECT_EQ(pairs.size(), 1u);
    
    if (!pairs.empty()) {
        const auto& pair = pairs[0];
        // Order doesn't matter, but both IDs should be present
        EXPECT_TRUE((pair.a == staticId && pair.b == dynamicId) ||
                    (pair.a == dynamicId && pair.b == staticId));
    }
}

TEST(BroadPhaseTest, NoCollision) {
    BroadPhase bp(10.0f);
    
    // Create two non-overlapping AABBs
    AABB aabb1, aabb2;
    aabb1.min = Vector3(0.0f, 0.0f, 0.0f);
    aabb1.max = Vector3(1.0f, 1.0f, 1.0f);
    
    aabb2.min = Vector3(2.0f, 2.0f, 2.0f);
    aabb2.max = Vector3(3.0f, 3.0f, 3.0f);
    
    bp.createProxy(aabb1, false);
    bp.createProxy(aabb2, false);
    
    // Find collisions
    bp.finalizeBroadPhase();
    auto pairs = bp.findPotentialCollisions();
    
    // Should find no collisions
    EXPECT_TRUE(pairs.empty());
}

} // namespace
} // namespace geometry
} // namespace math
} // namespace pynovage
