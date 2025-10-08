#include "../../include/geometry/path.hpp"
#include "../../include/geometry/catmull_rom_path.hpp"
#include <gtest/gtest.h>

namespace pynovage {
namespace math {
namespace tests {

class PathTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Define test control points that form a square path
        points = {
            Vector3(0.0f, 0.0f, 0.0f),    // Start
            Vector3(1.0f, 0.0f, 0.0f),    // Right
            Vector3(1.0f, 1.0f, 0.0f),    // Up
            Vector3(0.0f, 1.0f, 0.0f),    // Left
            Vector3(0.0f, 0.0f, 0.0f)     // Back to start
        };

        // Create path with constant speed movement
        path = std::make_unique<CatmullRomPath>(points, Path::MovementMode::ConstantSpeed);
    }

    std::vector<Vector3> points;
    std::unique_ptr<Path> path;
};

TEST_F(PathTest, InitialState) {
    ASSERT_TRUE(path != nullptr);
    EXPECT_FLOAT_EQ(path->getLength(), 4.0f);  // Square perimeter
    EXPECT_TRUE(path->isClosed());
}

TEST_F(PathTest, Evaluation) {
    // Test corners
    Path::State state = path->getState(0.0f);  // Start
    EXPECT_NEAR(state.position.x, 0.0f, 1e-4f);
    EXPECT_NEAR(state.position.y, 0.0f, 1e-4f);

    state = path->getState(0.25f);  // First corner
    EXPECT_NEAR(state.position.x, 1.0f, 1e-4f);
    EXPECT_NEAR(state.position.y, 0.0f, 1e-4f);

    state = path->getState(0.5f);   // Second corner
    EXPECT_NEAR(state.position.x, 1.0f, 1e-4f);
    EXPECT_NEAR(state.position.y, 1.0f, 1e-4f);

    state = path->getState(0.75f);  // Third corner
    EXPECT_NEAR(state.position.x, 0.0f, 1e-4f);
    EXPECT_NEAR(state.position.y, 1.0f, 1e-4f);

    state = path->getState(1.0f);   // Back to start
    EXPECT_NEAR(state.position.x, 0.0f, 1e-4f);
    EXPECT_NEAR(state.position.y, 0.0f, 1e-4f);
}

TEST_F(PathTest, ConstantSpeed) {
    Path::State startState;
    startState.position = points[0];
    startState.time = 0.0f;
    startState.distance = 0.0f;
    startState.speed = 1.0f;  // 1 unit per second

    float dt = 0.1f;  // 0.1 second steps
    float expectedDist = 0.1f;  // Should move 0.1 units each step at speed 1

    Path::State state = startState;
    for (int i = 0; i < 10; i++) {  // Test first second of movement
        state = path->updateConstantSpeed(state, dt);
        EXPECT_NEAR(state.distance, expectedDist, 1e-4f);
        expectedDist += 0.1f;
    }
}

TEST_F(PathTest, Orientation) {
    // Test orientations at key points
    Path::State state;

    // At start - should face right (+X)
    state = path->getState(0.0f);
    Vector3 forward = state.rotation.RotateVector(Vector3::unitX());
    EXPECT_NEAR(forward.x, 1.0f, 1e-4f);
    EXPECT_NEAR(forward.y, 0.0f, 1e-4f);

    // At first corner - should face up (+Y)
    state = path->getState(0.25f);
    forward = state.rotation.RotateVector(Vector3::unitX());
    EXPECT_NEAR(forward.x, 0.0f, 1e-4f);
    EXPECT_NEAR(forward.y, 1.0f, 1e-4f);

    // At second corner - should face left (-X)
    state = path->getState(0.5f);
    forward = state.rotation.RotateVector(Vector3::unitX());
    EXPECT_NEAR(forward.x, -1.0f, 1e-4f);
    EXPECT_NEAR(forward.y, 0.0f, 1e-4f);
}

TEST_F(PathTest, ClosestPoint) {
    Vector3 query(0.5f, -0.5f, 0.0f);  // Below first segment
    Path::State closest = path->getClosestPoint(query);

    // Should snap to bottom segment
    EXPECT_NEAR(closest.position.x, 0.5f, 1e-4f);
    EXPECT_NEAR(closest.position.y, 0.0f, 1e-4f);
    EXPECT_NEAR(closest.time, 0.125f, 1e-4f);  // Quarter through first segment
}

TEST_F(PathTest, Curvature) {
    // At corners, curvature should be high
    float cornerCurvature = path->getCurvature(0.25f);  // First corner
    EXPECT_GT(cornerCurvature, 1.0f);

    // At straight segments, curvature should be near zero
    float midSegmentCurvature = path->getCurvature(0.125f);  // Middle of first segment
    EXPECT_NEAR(midSegmentCurvature, 0.0f, 1e-2f);
}

} // namespace tests
} // namespace math
} // namespace pynovage