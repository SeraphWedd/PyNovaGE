#include "../../include/geometry/path.hpp"
#include <gtest/gtest.h>

namespace pynovage {
namespace math {
namespace tests {

class PathTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple path with 3 points
        path_.addPoint(Vector3(0, 0, 0));
        path_.addPoint(Vector3(1, 1, 0));
        path_.addPoint(Vector3(2, 0, 0));
    }
    
    Path path_;
};

TEST_F(PathTest, DefaultConstructor) {
    Path path;
    EXPECT_EQ(path.getPointCount(), 0);
    EXPECT_EQ(path.getType(), PathType::CatmullRom);
    EXPECT_FALSE(path.isClosed());
    EXPECT_FLOAT_EQ(path.getTension(), 1.0f);
}

TEST_F(PathTest, AddPoints) {
    EXPECT_EQ(path_.getPointCount(), 3);
    EXPECT_EQ(path_.getPoint(0), Vector3(0, 0, 0));
    EXPECT_EQ(path_.getPoint(1), Vector3(1, 1, 0));
    EXPECT_EQ(path_.getPoint(2), Vector3(2, 0, 0));
}

TEST_F(PathTest, InsertPoint) {
    path_.insertPoint(Vector3(1.5f, 0.5f, 0), 2);
    EXPECT_EQ(path_.getPointCount(), 4);
    EXPECT_EQ(path_.getPoint(2), Vector3(1.5f, 0.5f, 0));
}

TEST_F(PathTest, RemovePoint) {
    path_.removePoint(1);
    EXPECT_EQ(path_.getPointCount(), 2);
    EXPECT_EQ(path_.getPoint(0), Vector3(0, 0, 0));
    EXPECT_EQ(path_.getPoint(1), Vector3(2, 0, 0));
}

TEST_F(PathTest, GetPosition) {
    // Test positions at different t values
    EXPECT_EQ(path_.getPosition(0.0f), Vector3(0, 0, 0));
    EXPECT_EQ(path_.getPosition(1.0f), Vector3(2, 0, 0));
    
    // Middle point should be somewhere between points depending on interpolation
    Vector3 mid = path_.getPosition(0.5f);
    EXPECT_GE(mid.y, 0.0f);  // Should be above baseline due to middle control point
    EXPECT_LE(mid.y, 1.0f);  // But not higher than middle control point
}

TEST_F(PathTest, GetTangent) {
    // Start tangent should point upward and right
    Vector3 startTan = path_.getTangent(0.0f);
    EXPECT_GT(startTan.x, 0.0f);
    EXPECT_GT(startTan.y, 0.0f);
    
    // End tangent should point downward and right
    Vector3 endTan = path_.getTangent(1.0f);
    EXPECT_GT(endTan.x, 0.0f);
    EXPECT_LT(endTan.y, 0.0f);
}

TEST_F(PathTest, GetFrame) {
    auto [pos, tan, norm, binorm] = path_.getFrame(0.5f);
    
    // Position should be between points
    EXPECT_GE(pos.x, 0.0f);
    EXPECT_LE(pos.x, 2.0f);
    
    // Tangent should be normalized
    EXPECT_NEAR(tan.length(), 1.0f, 1e-6f);
    
    // Normal should be normalized and perpendicular to tangent
    EXPECT_NEAR(norm.length(), 1.0f, 1e-6f);
    EXPECT_NEAR(tan.dot(norm), 0.0f, 1e-6f);
    
    // Binormal should complete right-handed system
    EXPECT_NEAR(binorm.length(), 1.0f, 1e-6f);
    EXPECT_NEAR(tan.dot(binorm), 0.0f, 1e-6f);
    EXPECT_NEAR(norm.dot(binorm), 0.0f, 1e-6f);
}

TEST_F(PathTest, PathTypes) {
    // Test all path types interpolate endpoints correctly
    for (PathType type : {PathType::CatmullRom, PathType::Bezier, PathType::BSpline, PathType::Linear}) {
        path_.setType(type);
        EXPECT_EQ(path_.getPosition(0.0f), Vector3(0, 0, 0)) << "Path type: " << static_cast<int>(type);
        EXPECT_EQ(path_.getPosition(1.0f), Vector3(2, 0, 0)) << "Path type: " << static_cast<int>(type);
    }
}

TEST_F(PathTest, ClosedPath) {
    path_.setClosed(true);
    
    // With closed path, end should connect to start
    Vector3 almostEnd = path_.getPosition(0.99f);
    Vector3 start = path_.getPosition(0.0f);
    
    // Should be moving toward start point
    Vector3 toStart = (start - almostEnd).normalized();
    Vector3 tangent = path_.getTangent(0.99f).normalized();
    
    EXPECT_GT(toStart.dot(tangent), 0.7f);  // Vectors should be similar
}

TEST_F(PathTest, PathLength) {
    float length = path_.getLength();
    EXPECT_GT(length, 0.0f);
    
    // Parameter at various distances
    EXPECT_FLOAT_EQ(path_.getParameterAtDistance(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(path_.getParameterAtDistance(length), 1.0f);
    EXPECT_GT(path_.getParameterAtDistance(length * 0.5f), 0.4f);
    EXPECT_LT(path_.getParameterAtDistance(length * 0.5f), 0.6f);
}

TEST_F(PathTest, Tension) {
    path_.setTension(0.5f);  // Reduce tension
    Vector3 normalPos = path_.getPosition(0.5f);
    printf("Normal tension (0.5): %.3f %.3f %.3f\n", normalPos.x, normalPos.y, normalPos.z);
    
    path_.setTension(2.0f);  // Increase tension
    Vector3 tightPos = path_.getPosition(0.5f);
    printf("High tension (2.0): %.3f %.3f %.3f\n", tightPos.x, tightPos.y, tightPos.z);
    
    // Higher tension should pull curve closer to control points
    EXPECT_GT(tightPos.y, normalPos.y);
}

} // namespace tests
} // namespace math
} // namespace pynovage