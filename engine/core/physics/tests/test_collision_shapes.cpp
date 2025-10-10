#include <gtest/gtest.h>
#include "physics/collision_shapes.hpp"
#include <memory>
#include <chrono>

using namespace PyNovaGE::Physics;

class CollisionShapesTest : public ::testing::Test {
protected:
    void SetUp() override {
        rect_shape = std::make_shared<RectangleShape>(Vector2<float>(4.0f, 2.0f));
        circle_shape = std::make_shared<CircleShape>(2.5f);
    }
    
    std::shared_ptr<RectangleShape> rect_shape;
    std::shared_ptr<CircleShape> circle_shape;
};

// Rectangle Shape Tests
TEST_F(CollisionShapesTest, RectangleBasicProperties) {
    EXPECT_EQ(rect_shape->getType(), ShapeType::Rectangle);
    EXPECT_FLOAT_EQ(rect_shape->getHalfSize().x, 2.0f);
    EXPECT_FLOAT_EQ(rect_shape->getHalfSize().y, 1.0f);
    EXPECT_FLOAT_EQ(rect_shape->getSize().x, 4.0f);
    EXPECT_FLOAT_EQ(rect_shape->getSize().y, 2.0f);
    EXPECT_FLOAT_EQ(rect_shape->getArea(), 8.0f);
}

TEST_F(CollisionShapesTest, RectangleInertia) {
    float mass = 10.0f;
    float expected_inertia = mass * (4.0f * 4.0f + 2.0f * 2.0f) / 12.0f; // mass * (w^2 + h^2) / 12
    EXPECT_FLOAT_EQ(rect_shape->getInertia(mass), expected_inertia);
}

TEST_F(CollisionShapesTest, RectangleBounds) {
    Vector2<float> position(5.0f, 3.0f);
    auto bounds = rect_shape->getBounds(position);
    
    // Check bounds are correct
    EXPECT_FLOAT_EQ(bounds.min[0], 3.0f);  // 5 - 2 (half width)
    EXPECT_FLOAT_EQ(bounds.min[1], 2.0f);  // 3 - 1 (half height)
    EXPECT_FLOAT_EQ(bounds.max[0], 7.0f);  // 5 + 2
    EXPECT_FLOAT_EQ(bounds.max[1], 4.0f);  // 3 + 1
}

TEST_F(CollisionShapesTest, RectangleClosestPoint) {
    Vector2<float> position(0.0f, 0.0f);
    
    // Point outside rectangle
    Vector2<float> outside_point(5.0f, 3.0f);
    Vector2<float> closest = rect_shape->getClosestPoint(outside_point, position);
    EXPECT_FLOAT_EQ(closest.x, 2.0f);  // Clamped to half_size.x
    EXPECT_FLOAT_EQ(closest.y, 1.0f);  // Clamped to half_size.y
    
    // Point inside rectangle
    Vector2<float> inside_point(1.0f, 0.5f);
    closest = rect_shape->getClosestPoint(inside_point, position);
    EXPECT_FLOAT_EQ(closest.x, 1.0f);   // Unchanged
    EXPECT_FLOAT_EQ(closest.y, 0.5f);   // Unchanged
}

// Circle Shape Tests
TEST_F(CollisionShapesTest, CircleBasicProperties) {
    EXPECT_EQ(circle_shape->getType(), ShapeType::Circle);
    EXPECT_FLOAT_EQ(circle_shape->getRadius(), 2.5f);
    EXPECT_NEAR(circle_shape->getArea(), 3.14159265359f * 2.5f * 2.5f, 0.0001f);
}

TEST_F(CollisionShapesTest, CircleInertia) {
    float mass = 10.0f;
    float radius = 2.5f;
    float expected_inertia = 0.5f * mass * radius * radius;
    EXPECT_FLOAT_EQ(circle_shape->getInertia(mass), expected_inertia);
}

TEST_F(CollisionShapesTest, CircleBounds) {
    Vector2<float> position(3.0f, 4.0f);
    auto bounds = circle_shape->getBounds(position);
    
    // Check bounds are correct
    EXPECT_FLOAT_EQ(bounds.min[0], 0.5f);  // 3 - 2.5
    EXPECT_FLOAT_EQ(bounds.min[1], 1.5f);  // 4 - 2.5
    EXPECT_FLOAT_EQ(bounds.max[0], 5.5f);  // 3 + 2.5
    EXPECT_FLOAT_EQ(bounds.max[1], 6.5f);  // 4 + 2.5
}

TEST_F(CollisionShapesTest, CircleClosestPoint) {
    Vector2<float> position(0.0f, 0.0f);
    float radius = 2.5f;
    
    // Point outside circle
    Vector2<float> outside_point(5.0f, 0.0f);
    Vector2<float> closest = circle_shape->getClosestPoint(outside_point, position);
    EXPECT_FLOAT_EQ(closest.x, radius);  // On circle edge
    EXPECT_FLOAT_EQ(closest.y, 0.0f);
    
    // Point inside circle
    Vector2<float> inside_point(1.0f, 1.0f);
    closest = circle_shape->getClosestPoint(inside_point, position);
    EXPECT_FLOAT_EQ(closest.x, 1.0f);   // Unchanged (inside circle)
    EXPECT_FLOAT_EQ(closest.y, 1.0f);   // Unchanged
}

// Collision Detection Tests
TEST_F(CollisionShapesTest, RectangleVsRectangleIntersection) {
    auto rect1 = std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f));
    auto rect2 = std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f));
    
    Vector2<float> pos1(0.0f, 0.0f);
    Vector2<float> pos2(1.5f, 0.0f); // Overlapping
    
    EXPECT_TRUE(CollisionDetection::intersects(*rect1, pos1, *rect2, pos2));
    
    pos2 = Vector2<float>(3.0f, 0.0f); // Not overlapping
    EXPECT_FALSE(CollisionDetection::intersects(*rect1, pos1, *rect2, pos2));
}

TEST_F(CollisionShapesTest, CircleVsCircleIntersection) {
    auto circle1 = std::make_shared<CircleShape>(1.0f);
    auto circle2 = std::make_shared<CircleShape>(1.5f);
    
    Vector2<float> pos1(0.0f, 0.0f);
    Vector2<float> pos2(2.0f, 0.0f); // Overlapping (distance 2.0 < 1.0 + 1.5)
    
    EXPECT_TRUE(CollisionDetection::intersects(*circle1, pos1, *circle2, pos2));
    
    pos2 = Vector2<float>(3.0f, 0.0f); // Not overlapping (distance 3.0 > 2.5)
    EXPECT_FALSE(CollisionDetection::intersects(*circle1, pos1, *circle2, pos2));
}

TEST_F(CollisionShapesTest, RectangleVsCircleIntersection) {
    auto rect = std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f));
    auto circle = std::make_shared<CircleShape>(1.0f);
    
    Vector2<float> rectPos(0.0f, 0.0f);
    Vector2<float> circlePos(1.5f, 0.0f); // Circle overlapping with rectangle edge
    
    EXPECT_TRUE(CollisionDetection::intersects(*rect, rectPos, *circle, circlePos));
    
    circlePos = Vector2<float>(3.0f, 0.0f); // Circle far from rectangle
    EXPECT_FALSE(CollisionDetection::intersects(*rect, rectPos, *circle, circlePos));
}

TEST_F(CollisionShapesTest, ContainmentTests) {
    Vector2<float> rectPos(0.0f, 0.0f);
    Vector2<float> circlePos(0.0f, 0.0f);
    
    // Point inside rectangle
    EXPECT_TRUE(CollisionDetection::contains(*rect_shape, rectPos, Vector2<float>(1.0f, 0.5f)));
    // Point outside rectangle
    EXPECT_FALSE(CollisionDetection::contains(*rect_shape, rectPos, Vector2<float>(3.0f, 0.0f)));
    
    // Point inside circle
    EXPECT_TRUE(CollisionDetection::contains(*circle_shape, circlePos, Vector2<float>(1.0f, 1.0f)));
    // Point outside circle
    EXPECT_FALSE(CollisionDetection::contains(*circle_shape, circlePos, Vector2<float>(3.0f, 0.0f)));
}

// Collision Manifold Tests
TEST_F(CollisionShapesTest, ManifoldGeneration_RectangleVsRectangle) {
    auto rect1 = std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f));
    auto rect2 = std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f));
    
    Vector2<float> pos1(0.0f, 0.0f);
    Vector2<float> pos2(1.0f, 0.0f); // Overlapping by 1.0 unit horizontally
    
    auto manifold = CollisionDetection::generateManifold(*rect1, pos1, *rect2, pos2);
    
    EXPECT_TRUE(manifold.hasCollision);
    EXPECT_FLOAT_EQ(manifold.normal.x, 1.0f);
    EXPECT_FLOAT_EQ(manifold.normal.y, 0.0f);
    EXPECT_FLOAT_EQ(manifold.penetration, 1.0f);
}

TEST_F(CollisionShapesTest, ManifoldGeneration_CircleVsCircle) {
    auto circle1 = std::make_shared<CircleShape>(1.0f);
    auto circle2 = std::make_shared<CircleShape>(1.0f);
    
    Vector2<float> pos1(0.0f, 0.0f);
    Vector2<float> pos2(1.5f, 0.0f); // Overlapping by 0.5 units
    
    auto manifold = CollisionDetection::generateManifold(*circle1, pos1, *circle2, pos2);
    
    EXPECT_TRUE(manifold.hasCollision);
    EXPECT_FLOAT_EQ(manifold.normal.x, 1.0f);
    EXPECT_FLOAT_EQ(manifold.normal.y, 0.0f);
    EXPECT_FLOAT_EQ(manifold.penetration, 0.5f);
}

TEST_F(CollisionShapesTest, ManifoldGeneration_NoCollision) {
    auto rect1 = std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f));
    auto rect2 = std::make_shared<RectangleShape>(Vector2<float>(1.0f, 1.0f));
    
    Vector2<float> pos1(0.0f, 0.0f);
    Vector2<float> pos2(3.0f, 0.0f); // No overlap
    
    auto manifold = CollisionDetection::generateManifold(*rect1, pos1, *rect2, pos2);
    
    EXPECT_FALSE(manifold.hasCollision);
}

// Performance regression tests
TEST_F(CollisionShapesTest, PerformanceRegression_ManyIntersectionTests) {
    auto rect1 = std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f));
    auto rect2 = std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f));
    
    Vector2<float> pos1(0.0f, 0.0f);
    Vector2<float> pos2(1.0f, 0.0f);
    
    // This should be very fast due to SIMD optimizations
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        bool result = CollisionDetection::intersects(*rect1, pos1, *rect2, pos2);
        (void)result; // Prevent optimization
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should complete very quickly with SIMD optimizations
    EXPECT_LT(duration.count(), 5000); // Less than 5ms for 10k tests
}

TEST_F(CollisionShapesTest, EdgeCase_ZeroSizedShapes) {
    auto tiny_rect = std::make_shared<RectangleShape>(Vector2<float>(0.0f, 0.0f));
    auto tiny_circle = std::make_shared<CircleShape>(0.0f);
    
    EXPECT_FLOAT_EQ(tiny_rect->getArea(), 0.0f);
    EXPECT_FLOAT_EQ(tiny_circle->getArea(), 0.0f);
    
    // These should not crash
    Vector2<float> pos(0.0f, 0.0f);
    auto bounds1 = tiny_rect->getBounds(pos);
    auto bounds2 = tiny_circle->getBounds(pos);
    
    EXPECT_FLOAT_EQ(bounds1.min[0], bounds1.max[0]);
    EXPECT_FLOAT_EQ(bounds2.min[0], bounds2.max[0]);
}

TEST_F(CollisionShapesTest, EdgeCase_VeryLargeShapes) {
    auto large_rect = std::make_shared<RectangleShape>(Vector2<float>(1000.0f, 1000.0f));
    auto large_circle = std::make_shared<CircleShape>(1000.0f);
    
    Vector2<float> pos(0.0f, 0.0f);
    
    // Should handle large shapes without issues
    auto bounds1 = large_rect->getBounds(pos);
    auto bounds2 = large_circle->getBounds(pos);
    
    EXPECT_FLOAT_EQ(bounds1.max[0] - bounds1.min[0], 1000.0f);
    EXPECT_FLOAT_EQ(bounds2.max[0] - bounds2.min[0], 2000.0f);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}