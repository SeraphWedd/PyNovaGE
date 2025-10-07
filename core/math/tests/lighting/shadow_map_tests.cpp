#include <gtest/gtest.h>
#include "lighting/shadow_map.hpp"
#include "vector3.hpp"
#include "matrix4.hpp"

using namespace pynovage::math;
using namespace pynovage::math::lighting;

TEST(ShadowMapTests, DefaultConstruction) {
    ShadowMap map;
    EXPECT_EQ(map.getType(), ShadowMapType::Standard);
    
    const auto& params = map.getParameters();
    EXPECT_EQ(params.resolution, 1024);
    EXPECT_FLOAT_EQ(params.bias, 0.005f);
    EXPECT_FLOAT_EQ(params.normalBias, 0.4f);
    EXPECT_FLOAT_EQ(params.bleedReduction, 0.2f);
    EXPECT_EQ(params.pcfSamples, 16);
    EXPECT_FLOAT_EQ(params.pcfRadius, 3.0f);
}

TEST(ShadowMapTests, DirectionalLightMatrices) {
    ShadowMap map(ShadowMapType::Standard);
    Vector3 lightPos(0.0f, 10.0f, 0.0f);
    Vector3 lightDir(0.0f, -1.0f, 0.0f);
    
    map.updateViewMatrix(lightPos, lightDir);
    map.updateProjectionMatrix(0.1f, 100.0f);
    
    const auto& viewMat = map.getViewMatrix();
    const auto& projMat = map.getProjectionMatrix();
    
    // Verify view matrices have expected properties
    
    // 1. View matrix is not identity
    EXPECT_NE(viewMat, Matrix4::identity());
    
    // 2. Projection matrix has equal scaling in X and Y (symmetric frustum)
    EXPECT_FLOAT_EQ(projMat[0][0], projMat[1][1]);
    
    // 3. Verify matrix transforms points along light direction
    Vector3 testPoint(0.0f, 0.0f, 0.0f);
    Vector3 transformed = viewMat.transformPoint(testPoint);
    Vector3 transformed2 = viewMat.transformPoint(testPoint + lightDir);
    Vector3 viewSpaceDir = (transformed2 - transformed).normalized();
    
    // Should map light direction to view space -Z
    EXPECT_NEAR(viewSpaceDir.z, -1.0f, 1e-3f);
    
    // Verify projection matrix is orthographic (common for directional shadow maps)
    EXPECT_NE(projMat, Matrix4::identity());
    EXPECT_FLOAT_EQ(projMat[0][0], projMat[1][1]); // Same scale X and Y
}

TEST(ShadowMapTests, PointLightCubemap) {
    ShadowMap map(ShadowMapType::Cube);
    Vector3 lightPos(0.0f, 0.0f, 0.0f);
    
    map.updateViewMatrix(lightPos);
    map.updateProjectionMatrix(0.1f, 100.0f, 90.0f);
    
    // Verify we have 6 unique view matrices for the cubemap faces
    for (int i = 0; i < 6; ++i) {
        for (int j = i + 1; j < 6; ++j) {
            const auto& matI = map.getCubeFaceViewMatrix(i);
            const auto& matJ = map.getCubeFaceViewMatrix(j);
            // Ensure matrices are different
            EXPECT_NE(matI, matJ);
        }
    }
    
    // Test projection matrix is perspective with 90-degree FOV
    const auto& projMat = map.getProjectionMatrix();
    // Aspect ratio should be 1.0 for cubemap faces
    EXPECT_FLOAT_EQ(projMat[0][0], projMat[1][1]);
}

TEST(ShadowMapTests, CascadeConfig) {
    CascadeConfig config;
    EXPECT_EQ(config.numCascades, 4);
    EXPECT_FLOAT_EQ(config.splitDistances[0], 20.0f);
    EXPECT_FLOAT_EQ(config.splitDistances[1], 50.0f);
    EXPECT_FLOAT_EQ(config.splitDistances[2], 100.0f);
    EXPECT_FLOAT_EQ(config.splitDistances[3], 200.0f);
    EXPECT_FLOAT_EQ(config.cascadeBlendDistance, 5.0f);
}

TEST(ShadowMapTests, ShadowMapParameters) {
    ShadowMapParameters params;
    // Test default values
    EXPECT_EQ(params.resolution, 1024);
    EXPECT_FLOAT_EQ(params.bias, 0.005f);
    EXPECT_FLOAT_EQ(params.normalBias, 0.4f);
    EXPECT_FLOAT_EQ(params.bleedReduction, 0.2f);
    EXPECT_EQ(params.pcfSamples, 16);
    EXPECT_FLOAT_EQ(params.pcfRadius, 3.0f);
}

TEST(ShadowMapTests, SpotLightShadowMap) {
    ShadowMap map(ShadowMapType::Standard);
    Vector3 lightPos(5.0f, 5.0f, 0.0f);
    Vector3 lightDir(-1.0f, -1.0f, 0.0f);
    float fov = 45.0f;
    
    map.updateViewMatrix(lightPos, lightDir);
    map.updateProjectionMatrix(0.1f, 50.0f, fov);
    
    const auto& viewMat = map.getViewMatrix();
    const auto& projMat = map.getProjectionMatrix();
    
    // View matrix should look at the correct direction
    Vector3 testPoint(0.0f, 0.0f, 0.0f);
    Vector3 transformed = viewMat.transformPoint(testPoint);
    Vector3 transformed2 = viewMat.transformPoint(testPoint + lightDir);
    Vector3 viewSpaceDir = (transformed2 - transformed).normalized();
    
    // Light direction should map to -Z in view space
    EXPECT_NEAR(viewSpaceDir.z, -1.0f, 1e-3f);
    
    // Projection matrix should be perspective for spot lights
    // Check perspective matrix properties
    EXPECT_GT(std::fabs(projMat[2][3]), 0.0f); // Should have perspective divide
    EXPECT_FLOAT_EQ(projMat[3][3], 0.0f);      // Perspective projection property
}

TEST(ShadowMapTests, CascadedShadowMapSetup) {
    ShadowMap map(ShadowMapType::Cascade);
    Vector3 lightDir(0.0f, -1.0f, 0.0f);
    
    // Test view matrix for each cascade
    for (uint32_t i = 0; i < 4; ++i) {
        Vector3 cascadeCenter(0.0f, 100.0f, 0.0f); // Example cascade center
        map.updateViewMatrix(cascadeCenter + lightDir * 100.0f, lightDir);
        
        const auto& viewMat = map.getViewMatrix();
        
        // Each cascade's view matrix should maintain light direction alignment
        Vector3 testPoint(0.0f, 0.0f, 0.0f);
        Vector3 transformed = viewMat.transformPoint(testPoint);
        Vector3 transformed2 = viewMat.transformPoint(testPoint + lightDir);
        Vector3 viewSpaceDir = (transformed2 - transformed).normalized();
        
        EXPECT_NEAR(viewSpaceDir.z, -1.0f, 1e-3f);
    }
}

TEST(ShadowMapTests, CubemapFaceOrientation) {
    ShadowMap map(ShadowMapType::Cube);
    Vector3 lightPos(0.0f, 0.0f, 0.0f);
    
    map.updateViewMatrix(lightPos);
    
    // Test points in all 6 directions
    Vector3 testPoints[6] = {
        Vector3(1.0f, 0.0f, 0.0f),   // +X
        Vector3(-1.0f, 0.0f, 0.0f),   // -X
        Vector3(0.0f, 1.0f, 0.0f),    // +Y
        Vector3(0.0f, -1.0f, 0.0f),   // -Y
        Vector3(0.0f, 0.0f, 1.0f),    // +Z
        Vector3(0.0f, 0.0f, -1.0f)    // -Z
    };
    
    // Verify each face's view matrix properly orients its respective direction to -Z
    for (int i = 0; i < 6; ++i) {
        const auto& viewMat = map.getCubeFaceViewMatrix(i);
        Vector3 origin(0.0f, 0.0f, 0.0f);
        Vector3 transformed = viewMat.transformPoint(origin);
        Vector3 transformedPoint = viewMat.transformPoint(testPoints[i]);
        Vector3 viewSpaceDir = (transformedPoint - transformed).normalized();
        
        // The test point should map to the -Z direction in view space
        EXPECT_NEAR(viewSpaceDir.z, -1.0f, 1e-3f);
    }
}

TEST(ShadowMapTests, ProjectionMatrixRanges) {
    ShadowMap map;
    
    // Test different near/far combinations
    struct TestRange {
        float near;
        float far;
    } ranges[] = {
        {0.1f, 100.0f},    // Standard range
        {0.01f, 1000.0f},   // Wide range
        {1.0f, 10.0f},      // Narrow range
        {0.001f, 10000.0f}  // Extreme range
    };
    
    for (const auto& range : ranges) {
        map.updateProjectionMatrix(range.near, range.far);
        const auto& projMat = map.getProjectionMatrix();
        
        // For orthographic projection
        EXPECT_FLOAT_EQ(projMat[3][3], 1.0f);  // No perspective divide
        
        // Matrix should properly handle the range
        float depthScale = 2.0f / (range.far - range.near);
        EXPECT_NEAR(projMat[2][2], -depthScale, 1e-3f);
    }
}
