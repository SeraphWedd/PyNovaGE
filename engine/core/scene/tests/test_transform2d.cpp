#include <gtest/gtest.h>
#include "scene/transform2d.hpp"
#include <cmath>

using namespace PyNovaGE::Scene;

class Transform2DTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Default transform for testing
        default_transform = Transform2D();
    }

    Transform2D default_transform;
    const float EPSILON = 1e-6f;
};

// Basic Construction Tests
TEST_F(Transform2DTest, DefaultConstruction) {
    Transform2D transform;
    
    EXPECT_EQ(transform.GetPosition(), Vector2f(0.0f, 0.0f));
    EXPECT_FLOAT_EQ(transform.GetRotation(), 0.0f);
    EXPECT_EQ(transform.GetScale(), Vector2f(1.0f, 1.0f));
}

TEST_F(Transform2DTest, ParameterizedConstruction) {
    Vector2f position(10.0f, 20.0f);
    float rotation = M_PI / 4.0f;
    Vector2f scale(2.0f, 3.0f);
    
    Transform2D transform(position, rotation, scale);
    
    EXPECT_EQ(transform.GetPosition(), position);
    EXPECT_FLOAT_EQ(transform.GetRotation(), rotation);
    EXPECT_EQ(transform.GetScale(), scale);
}

// Property Access Tests
TEST_F(Transform2DTest, SettersAndGetters) {
    Transform2D transform;
    
    Vector2f new_position(100.0f, 200.0f);
    float new_rotation = M_PI / 2.0f;
    Vector2f new_scale(0.5f, 1.5f);
    
    transform.SetPosition(new_position);
    transform.SetRotation(new_rotation);
    transform.SetScale(new_scale);
    
    EXPECT_EQ(transform.GetPosition(), new_position);
    EXPECT_FLOAT_EQ(transform.GetRotation(), new_rotation);
    EXPECT_EQ(transform.GetScale(), new_scale);
}

// Transform Operations Tests
TEST_F(Transform2DTest, TranslateOperation) {
    Transform2D transform(Vector2f(10.0f, 20.0f));
    
    transform.Translate(Vector2f(5.0f, -10.0f));
    
    EXPECT_EQ(transform.GetPosition(), Vector2f(15.0f, 10.0f));
}

TEST_F(Transform2DTest, RotateOperation) {
    Transform2D transform(Vector2f(0.0f), M_PI / 4.0f);
    
    transform.Rotate(M_PI / 4.0f);
    
    EXPECT_NEAR(transform.GetRotation(), M_PI / 2.0f, EPSILON);
}

TEST_F(Transform2DTest, ScaleOperations) {
    Transform2D transform(Vector2f(0.0f), 0.0f, Vector2f(2.0f, 3.0f));
    
    // Vector scale
    transform.Scale(Vector2f(1.5f, 0.5f));
    EXPECT_EQ(transform.GetScale(), Vector2f(3.0f, 1.5f));
    
    // Uniform scale
    transform.Scale(2.0f);
    EXPECT_EQ(transform.GetScale(), Vector2f(6.0f, 3.0f));
}

// Matrix Generation Tests
TEST_F(Transform2DTest, LocalToParentMatrix) {
    Transform2D transform(Vector2f(10.0f, 20.0f), 0.0f, Vector2f(1.0f, 1.0f));
    
    const Matrix3f& matrix = transform.GetLocalToParentMatrix();
    
    // Check translation components
    EXPECT_FLOAT_EQ(matrix(0, 2), 10.0f);
    EXPECT_FLOAT_EQ(matrix(1, 2), 20.0f);
    
    // Check that it's a valid transformation matrix
    EXPECT_FLOAT_EQ(matrix(2, 2), 1.0f);
}

TEST_F(Transform2DTest, RotationMatrix) {
    Transform2D transform(Vector2f(0.0f), M_PI / 2.0f, Vector2f(1.0f, 1.0f));
    
    const Matrix3f& matrix = transform.GetLocalToParentMatrix();
    
    // For 90 degree rotation: cos = 0, sin = 1
    EXPECT_NEAR(matrix(0, 0), 0.0f, EPSILON); // cos
    EXPECT_NEAR(matrix(0, 1), -1.0f, EPSILON); // -sin
    EXPECT_NEAR(matrix(1, 0), 1.0f, EPSILON); // sin
    EXPECT_NEAR(matrix(1, 1), 0.0f, EPSILON); // cos
}

TEST_F(Transform2DTest, ScaleMatrix) {
    Transform2D transform(Vector2f(0.0f), 0.0f, Vector2f(2.0f, 3.0f));
    
    const Matrix3f& matrix = transform.GetLocalToParentMatrix();
    
    // Check scale components
    EXPECT_FLOAT_EQ(matrix(0, 0), 2.0f);
    EXPECT_FLOAT_EQ(matrix(1, 1), 3.0f);
}

// World Transform Tests
TEST_F(Transform2DTest, WorldTransform) {
    Transform2D transform;
    
    Matrix3f world_matrix = TransformUtils::CreateTRSMatrix(
        Vector2f(100.0f, 200.0f), M_PI / 6.0f, Vector2f(2.0f, 2.0f)
    );
    
    transform.SetWorldMatrix(world_matrix);
    
    Vector2f world_pos = transform.GetWorldPosition();
    float world_rot = transform.GetWorldRotation();
    Vector2f world_scale = transform.GetWorldScale();
    
    EXPECT_NEAR(world_pos.x, 100.0f, EPSILON);
    EXPECT_NEAR(world_pos.y, 200.0f, EPSILON);
    EXPECT_NEAR(world_rot, M_PI / 6.0f, EPSILON);
    EXPECT_NEAR(world_scale.x, 2.0f, EPSILON);
    EXPECT_NEAR(world_scale.y, 2.0f, EPSILON);
}

// Point Transformation Tests
TEST_F(Transform2DTest, PointTransformation) {
    Transform2D transform(Vector2f(10.0f, 20.0f), 0.0f, Vector2f(1.0f, 1.0f));
    transform.SetWorldMatrix(transform.GetLocalToParentMatrix());
    
    Vector2f local_point(5.0f, 10.0f);
    Vector2f world_point = transform.TransformPoint(local_point);
    
    // With translation (10, 20), point (5, 10) should become (15, 30)
    EXPECT_NEAR(world_point.x, 15.0f, EPSILON);
    EXPECT_NEAR(world_point.y, 30.0f, EPSILON);
}

TEST_F(Transform2DTest, InversePointTransformation) {
    Transform2D transform(Vector2f(10.0f, 20.0f), 0.0f, Vector2f(1.0f, 1.0f));
    transform.SetWorldMatrix(transform.GetLocalToParentMatrix());
    
    Vector2f world_point(15.0f, 30.0f);
    Vector2f local_point = transform.InverseTransformPoint(world_point);
    
    // Inverse of above transformation
    EXPECT_NEAR(local_point.x, 5.0f, EPSILON);
    EXPECT_NEAR(local_point.y, 10.0f, EPSILON);
}

// Comparison Tests
TEST_F(Transform2DTest, EqualityComparison) {
    Transform2D transform1(Vector2f(1.0f, 2.0f), 0.5f, Vector2f(1.5f, 2.0f));
    Transform2D transform2(Vector2f(1.0f, 2.0f), 0.5f, Vector2f(1.5f, 2.0f));
    Transform2D transform3(Vector2f(1.1f, 2.0f), 0.5f, Vector2f(1.5f, 2.0f));
    
    EXPECT_TRUE(transform1 == transform2);
    EXPECT_FALSE(transform1 == transform3);
    EXPECT_TRUE(transform1 != transform3);
}

// Reset Test
TEST_F(Transform2DTest, Reset) {
    Transform2D transform(Vector2f(100.0f, 200.0f), M_PI, Vector2f(2.0f, 3.0f));
    
    transform.Reset();
    
    EXPECT_EQ(transform.GetPosition(), Vector2f(0.0f, 0.0f));
    EXPECT_FLOAT_EQ(transform.GetRotation(), 0.0f);
    EXPECT_EQ(transform.GetScale(), Vector2f(1.0f, 1.0f));
}

// TransformUtils Tests
class TransformUtilsTest : public ::testing::Test {
protected:
    const float EPSILON = 1e-6f;
};

TEST_F(TransformUtilsTest, MatrixCreation) {
    Vector2f translation(10.0f, 20.0f);
    float rotation = M_PI / 4.0f;
    Vector2f scale(2.0f, 3.0f);
    
    Matrix3f trs_matrix = TransformUtils::CreateTRSMatrix(translation, rotation, scale);
    
    // Verify translation components
    EXPECT_FLOAT_EQ(trs_matrix(0, 2), translation.x);
    EXPECT_FLOAT_EQ(trs_matrix(1, 2), translation.y);
    
    // Verify it's a valid homogeneous matrix
    EXPECT_FLOAT_EQ(trs_matrix(2, 2), 1.0f);
    EXPECT_FLOAT_EQ(trs_matrix(2, 0), 0.0f);
    EXPECT_FLOAT_EQ(trs_matrix(2, 1), 0.0f);
}

TEST_F(TransformUtilsTest, MatrixExtraction) {
    Vector2f original_translation(15.0f, 25.0f);
    float original_rotation = M_PI / 3.0f;
    Vector2f original_scale(1.5f, 2.5f);
    
    Matrix3f matrix = TransformUtils::CreateTRSMatrix(original_translation, original_rotation, original_scale);
    
    Vector2f extracted_translation = TransformUtils::ExtractTranslation(matrix);
    float extracted_rotation = TransformUtils::ExtractRotation(matrix);
    Vector2f extracted_scale = TransformUtils::ExtractScale(matrix);
    
    EXPECT_NEAR(extracted_translation.x, original_translation.x, EPSILON);
    EXPECT_NEAR(extracted_translation.y, original_translation.y, EPSILON);
    EXPECT_NEAR(extracted_rotation, original_rotation, EPSILON);
    EXPECT_NEAR(extracted_scale.x, original_scale.x, EPSILON);
    EXPECT_NEAR(extracted_scale.y, original_scale.y, EPSILON);
}

TEST_F(TransformUtilsTest, InterpolationLerp) {
    Transform2D start(Vector2f(0.0f, 0.0f), 0.0f, Vector2f(1.0f, 1.0f));
    Transform2D end(Vector2f(10.0f, 20.0f), M_PI, Vector2f(2.0f, 3.0f));
    
    Transform2D mid = TransformUtils::Lerp(start, end, 0.5f);
    
    EXPECT_EQ(mid.GetPosition(), Vector2f(5.0f, 10.0f));
    EXPECT_NEAR(mid.GetRotation(), M_PI / 2.0f, EPSILON);
    EXPECT_EQ(mid.GetScale(), Vector2f(1.5f, 2.0f));
}

TEST_F(TransformUtilsTest, InterpolationSlerp) {
    Transform2D start(Vector2f(0.0f, 0.0f), -M_PI * 0.9f, Vector2f(1.0f, 1.0f));
    Transform2D end(Vector2f(0.0f, 0.0f), M_PI * 0.9f, Vector2f(1.0f, 1.0f));
    
    // Slerp should take the shortest path (through 0) rather than the long way around
    Transform2D mid = TransformUtils::Slerp(start, end, 0.5f);
    
    // The result should be close to 0 (shortest path) rather than ±π
    EXPECT_NEAR(mid.GetRotation(), 0.0f, EPSILON);
}