#include <gtest/gtest.h>
#include "../../include/lighting/light_transforms.hpp"
#include "../../include/math_constants.hpp"

namespace pynovage {
namespace math {
namespace lighting {

using math::Vector3;
using math::Vector4;
using math::Matrix4;
using math::constants::half_pi;

// Helper function to compare matrices with floating point tolerance
bool matrixNear(const Matrix4& m1, const Matrix4& m2, float epsilon = 1e-5f) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (std::abs(m1[i][j] - m2[i][j]) > epsilon) {
                return false;
            }
        }
    }
    return true;
}

TEST(LightTransformTest, DirectionalLightView) {
    DirectionalLight light;
    light.direction = Vector3(0.0f, -1.0f, 0.0f); // pointing down
    Vector3 center(0.0f, 0.0f, 0.0f);
    float radius = 10.0f;
    
    Matrix4 view = LightSpaceTransform::createDirectionalLightView(light, center, radius);
    
    // View should look down along Y axis
    // Forward should be (0, -1, 0)
    // Up could be (0, 0, -1)
    // Right could be (1, 0, 0)
    Matrix4 expected(
        1.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f
    );
    
    EXPECT_TRUE(matrixNear(view, expected));
}

TEST(LightTransformTest, PointLightView) {
    PointLight light;
    light.position = Vector3(5.0f, 0.0f, 0.0f);
    
    // Test +X face (face 0)
    Matrix4 view = LightSpaceTransform::createPointLightView(light, 0);
    
    // Should be looking along +X axis from light position
    Matrix4 expected(
        0.0f,  0.0f, -1.0f, -5.0f,
        0.0f, -1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f
    );
    
    EXPECT_TRUE(matrixNear(view, expected));
}

TEST(LightTransformTest, SpotLightView) {
    SpotLight light;
    light.position = Vector3(0.0f, 5.0f, 0.0f);
    light.direction = Vector3(0.0f, -1.0f, 0.0f); // pointing down
    
    Matrix4 view = LightSpaceTransform::createSpotLightView(light);
    
    // Should be looking down from light position
    Matrix4 expected(
        1.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f, -1.0f, -5.0f,
        0.0f,  1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f
    );
    
    EXPECT_TRUE(matrixNear(view, expected));
}

TEST(LightTransformTest, DirectionalLightProjection) {
    DirectionalLight light;
    light.direction = Vector3(0.0f, -1.0f, 0.0f);
    Vector3 center(0.0f, 0.0f, 0.0f);
    float radius = 10.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    Matrix4 proj = LightSpaceTransform::createDirectionalLightProjection(
        light, center, radius, near, far
    );
    
    // Check that projection maps bounds correctly
    Vector3 testPoint = center + Vector3(radius, 0.0f, radius); // Corner of the light space
    Vector4 projectedPoint = proj * Vector4(testPoint.x, testPoint.y, testPoint.z, 1.0f);
    
    // Should be mapped to corner of NDC cube [0,1] with reversed Z
    EXPECT_NEAR(projectedPoint.x, 1.0f, 1e-5f);
    EXPECT_NEAR(projectedPoint.y, 1.0f, 1e-5f);
    EXPECT_NEAR(projectedPoint.z, 0.0f, 1e-5f); // far plane maps to 0 in reversed Z
}

TEST(LightTransformTest, PointLightProjection) {
    PointLight light;
    light.position = Vector3(0.0f, 0.0f, 0.0f);
    light.attenuation.range = 100.0f;
    float near = 0.1f;
    
    Matrix4 proj = LightSpaceTransform::createPointLightProjection(light, near);
    
    // Test point at near plane, centered
    Vector4 nearPoint(0.0f, 0.0f, near, 1.0f);
    Vector4 projectedNear = proj * nearPoint;
    projectedNear /= projectedNear.w; // Perspective divide
    
    // Should map to Z=1 (reversed Z)
    EXPECT_NEAR(projectedNear.z, 1.0f, 1e-5f);
    
    // Test point at far plane (range)
    Vector4 farPoint(0.0f, 0.0f, light.attenuation.range, 1.0f);
    Vector4 projectedFar = proj * farPoint;
    projectedFar /= projectedFar.w;
    
    // Should map to Z=0 (reversed Z)
    EXPECT_NEAR(projectedFar.z, 0.0f, 1e-5f);
}

TEST(LightTransformTest, SpotLightProjection) {
    SpotLight light;
    light.position = Vector3(0.0f, 0.0f, 0.0f);
    light.direction = Vector3(0.0f, 0.0f, 1.0f);
    light.outerAngle = half_pi * 0.5f; // 45 degrees
    light.attenuation.range = 100.0f;
    float near = 0.1f;
    
    Matrix4 proj = LightSpaceTransform::createSpotLightProjection(light, near);
    
    // Test point at near plane, centered
    Vector4 nearPoint(0.0f, 0.0f, near, 1.0f);
    Vector4 projectedNear = proj * nearPoint;
    projectedNear /= projectedNear.w;
    
    // Should map to Z=1 (reversed Z)
    EXPECT_NEAR(projectedNear.z, 1.0f, 1e-5f);
    
    // Test point at far plane (range)
    Vector4 farPoint(0.0f, 0.0f, light.attenuation.range, 1.0f);
    Vector4 projectedFar = proj * farPoint;
    projectedFar /= projectedFar.w;
    
    // Should map to Z=0 (reversed Z)
    EXPECT_NEAR(projectedFar.z, 0.0f, 1e-5f);
}

TEST(LightTransformTest, DirectionalLightTransform) {
    DirectionalLight light;
    light.direction = Vector3(0.0f, -1.0f, 0.0f);
    Vector3 center(0.0f, 0.0f, 0.0f);
    float radius = 10.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    Matrix4 transform = LightSpaceTransform::createLightSpaceTransform(
        light, center, radius, near, far
    );
    
    // Test that combined transform correctly maps points
Vector4 worldPoint(5.0f, 0.0f, 5.0f, 1.0f);
    Vector4 lightSpace = transform * worldPoint;
    lightSpace /= lightSpace.w;
    
    // Should be in [0,1] NDC with reversed Z
    EXPECT_GE(lightSpace.x, 0.0f);
    EXPECT_LE(lightSpace.x, 1.0f);
    EXPECT_GE(lightSpace.y, 0.0f);
    EXPECT_LE(lightSpace.y, 1.0f);
    EXPECT_GE(lightSpace.z, 0.0f);
    EXPECT_LE(lightSpace.z, 1.0f);
}

TEST(LightTransformTest, AllCubemapFaces) {
    PointLight light;
    light.position = Vector3(1.0f, 2.0f, 3.0f);
    
    // Test that all 6 faces form a valid cubemap
    for (int face = 0; face < 6; ++face) {
        Matrix4 view = LightSpaceTransform::createPointLightView(light, face);
        
        // Each view matrix should be orthogonal (inverse = transpose)
        Matrix4 viewRotation = view;
        viewRotation[0][3] = viewRotation[1][3] = viewRotation[2][3] = 0.0f;
        Matrix4 viewRotationTranspose = viewRotation.transposed();
        
        EXPECT_TRUE(matrixNear(viewRotation * viewRotationTranspose, Matrix4::identity()));
    }
}

TEST(LightTransformTest, DirectionalLightNormalBias) {
    DirectionalLight light;
    light.direction = Vector3(0.0f, -1.0f, 0.0f); // pointing down
    float normalBias = 0.005f;
    
    Matrix4 bias = LightSpaceTransform::createNormalBiasMatrix(light, normalBias);
    
    // Test a point - should be shifted along light direction
    Vector4 point(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 biased = bias * point;
    
    EXPECT_NEAR(biased.x, point.x, 1e-5f);
    EXPECT_NEAR(biased.y, point.y - normalBias, 1e-5f); // Shifted down
    EXPECT_NEAR(biased.z, point.z, 1e-5f);
    EXPECT_NEAR(biased.w, point.w, 1e-5f);
}

TEST(LightTransformTest, PointLightNormalBias) {
    PointLight light;
    light.position = Vector3(0.0f, 0.0f, 0.0f);
    float normalBias = 0.005f;
    
    Matrix4 bias = LightSpaceTransform::createNormalBiasMatrix(light, normalBias);
    
    // Test a point - should be scaled outward
    Vector4 point(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 biased = bias * point;
    float scale = 1.0f + normalBias;
    
    EXPECT_NEAR(biased.x, point.x * scale, 1e-5f);
    EXPECT_NEAR(biased.y, point.y * scale, 1e-5f);
    EXPECT_NEAR(biased.z, point.z * scale, 1e-5f);
    EXPECT_NEAR(biased.w, point.w, 1e-5f);
}

TEST(LightTransformTest, SpotLightNormalBias) {
    SpotLight light;
    light.position = Vector3(0.0f, 0.0f, 0.0f);
    light.direction = Vector3(0.0f, 0.0f, 1.0f); // pointing forward
    float normalBias = 0.005f;
    
    Matrix4 bias = LightSpaceTransform::createNormalBiasMatrix(light, normalBias);
    
    // Test a point - should be shifted along light direction
    Vector4 point(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 biased = bias * point;
    
    EXPECT_NEAR(biased.x, point.x, 1e-5f);
    EXPECT_NEAR(biased.y, point.y, 1e-5f);
    EXPECT_NEAR(biased.z, point.z + normalBias, 1e-5f); // Shifted forward
    EXPECT_NEAR(biased.w, point.w, 1e-5f);
}

TEST(LightTransformTest, DepthBias) {
    float depthBias = 0.0001f;
    float slopeScale = 1.0f;
    
    Matrix4 bias = LightSpaceTransform::createDepthBiasMatrix(depthBias, slopeScale);
    
    // Test a point at different depths
    Vector4 nearPoint(0.0f, 0.0f, 0.1f, 1.0f);
    Vector4 farPoint(0.0f, 0.0f, 100.0f, 1.0f);
    
    Vector4 biasedNear = bias * nearPoint;
    Vector4 biasedFar = bias * farPoint;
    
    // Depth should be offset by constant bias plus slope-scaled amount
    // Using 2e-5 tolerance to account for floating point rounding differences
    // between z*(1+s) + b and z + b + s*z
    EXPECT_NEAR(biasedNear.z, nearPoint.z + depthBias + slopeScale * nearPoint.z, 2e-5f);
    EXPECT_NEAR(biasedFar.z, farPoint.z + depthBias + slopeScale * farPoint.z, 2e-5f);
}

} // namespace lighting
} // namespace math
} // namespace pynovage
