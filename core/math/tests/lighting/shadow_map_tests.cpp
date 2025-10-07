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

TEST(ShadowMapTests, EdgeCases) {
    // Test very small near plane
    ShadowMap map1(ShadowMapType::Standard);
    map1.updateProjectionMatrix(1e-6f, 100.0f);
    auto proj1 = map1.getProjectionMatrix();
    EXPECT_GT(std::fabs(proj1[2][2]), 0.0f);
    
    // Test very large far plane
    ShadowMap map2(ShadowMapType::Standard);
    map2.updateProjectionMatrix(0.1f, 1e6f);
    auto proj2 = map2.getProjectionMatrix();
    EXPECT_GT(std::fabs(proj2[2][2]), 0.0f);
    
    // Test degenerate light direction (aligned with up)
    ShadowMap map3(ShadowMapType::Standard);
    Vector3 lightPos(0.0f, 0.0f, 0.0f);
    Vector3 lightDir(0.0f, 1.0f, 0.0f); // Aligned with world up
    map3.updateViewMatrix(lightPos, lightDir);
    auto view3 = map3.getViewMatrix();
    EXPECT_NE(view3, Matrix4::identity());
    
    // Test zero FOV for spot light
    ShadowMap map4(ShadowMapType::Standard);
    map4.updateProjectionMatrix(0.1f, 100.0f, 0.0f);
    auto proj4 = map4.getProjectionMatrix();
    EXPECT_NE(proj4, Matrix4::identity());
    
    // Test negative FOV handling
    ShadowMap map5(ShadowMapType::Standard);
    map5.updateProjectionMatrix(0.1f, 100.0f, -45.0f);
    auto proj5 = map5.getProjectionMatrix();
    EXPECT_GT(std::fabs(proj5[0][0]), 0.0f);
    EXPECT_GT(std::fabs(proj5[1][1]), 0.0f);
}

TEST(ShadowMapTests, CascadeDetails) {
    ShadowMap map(ShadowMapType::Cascade);
    Vector3 lightDir(0.0f, -1.0f, 0.0f);
    
    // Test each cascade's view matrix maintains light direction
    for (uint32_t i = 0; i < 4; ++i) {
        Vector3 cascadeCenter(0.0f, 100.0f * (i + 1), 0.0f);
        map.updateViewMatrix(cascadeCenter + lightDir * 100.0f, lightDir);
        
        const auto& viewMat = map.getViewMatrix();
        
        // Test point at cascade center
        Vector3 transformedCenter = viewMat.transformPoint(cascadeCenter);
        Vector3 transformedPoint = viewMat.transformPoint(cascadeCenter + lightDir);
        Vector3 viewSpaceDir = (transformedPoint - transformedCenter).normalized();
        
        // Direction should map to -Z
        EXPECT_NEAR(viewSpaceDir.z, -1.0f, 1e-3f);
        
        // Center point should be reasonable
        EXPECT_GT(std::fabs(transformedCenter.z), 0.0f);
    }
}

TEST(ShadowMapTests, ProjectionProperties) {
    // Test directional light projection
    ShadowMap dirMap(ShadowMapType::Standard);
    dirMap.updateProjectionMatrix(0.1f, 100.0f);
    const auto& dirProj = dirMap.getProjectionMatrix();
    
    // Should be symmetric
    EXPECT_FLOAT_EQ(dirProj[0][0], dirProj[1][1]);
    
    // Should be orthographic
    EXPECT_FLOAT_EQ(dirProj[3][3], 1.0f);
    
    // Test spot light projection
    ShadowMap spotMap(ShadowMapType::Standard);
    spotMap.updateProjectionMatrix(0.1f, 100.0f, 45.0f);
    const auto& spotProj = spotMap.getProjectionMatrix();
    
    // Should be perspective
    EXPECT_FLOAT_EQ(spotProj[3][3], 0.0f);
    EXPECT_GT(std::fabs(spotProj[2][3]), 0.0f);
    
    // Should be symmetric (aspect ratio 1.0)
    EXPECT_FLOAT_EQ(spotProj[0][0], spotProj[1][1]);
    
    // Test point light projection (cubemap face)
    ShadowMap pointMap(ShadowMapType::Cube);
    pointMap.updateProjectionMatrix(0.1f, 100.0f, 90.0f);
    const auto& pointProj = pointMap.getProjectionMatrix();
    
    // Should be perspective
    EXPECT_FLOAT_EQ(pointProj[3][3], 0.0f);
    EXPECT_GT(std::fabs(pointProj[2][3]), 0.0f);
    
    // Should be square (90-degree FOV, aspect ratio 1.0)
    EXPECT_FLOAT_EQ(pointProj[0][0], pointProj[1][1]);
}
