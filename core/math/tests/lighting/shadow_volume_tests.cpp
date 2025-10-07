#include <gtest/gtest.h>
#include <chrono>
#include <cmath>
#include <limits>
#include "../../include/lighting/shadow_volume.hpp"

namespace pynovage {
namespace math {
namespace lighting {

// Helper function to create a simple cube mesh for testing
void createCubeMesh(
    std::vector<Vector3>& vertices,
    std::vector<uint32_t>& indices)
{
    // Cube vertices (1x1x1 centered at origin)
    vertices = {
        Vector3(-0.5f, -0.5f, -0.5f),  // 0
        Vector3( 0.5f, -0.5f, -0.5f),  // 1
        Vector3( 0.5f,  0.5f, -0.5f),  // 2
        Vector3(-0.5f,  0.5f, -0.5f),  // 3
        Vector3(-0.5f, -0.5f,  0.5f),  // 4
        Vector3( 0.5f, -0.5f,  0.5f),  // 5
        Vector3( 0.5f,  0.5f,  0.5f),  // 6
        Vector3(-0.5f,  0.5f,  0.5f)   // 7
    };

    // Cube triangles (CCW winding)
    indices = {
        // Front face (-Z)
        0, 1, 2,
        0, 2, 3,
        // Right face (+X)
        1, 5, 6,
        1, 6, 2,
        // Back face (+Z)
        5, 4, 7,
        5, 7, 6,
        // Left face (-X)
        4, 0, 3,
        4, 3, 7,
        // Top face (+Y)
        3, 2, 6,
        3, 6, 7,
        // Bottom face (-Y)
        4, 5, 1,
        4, 1, 0
    };
}

TEST(ShadowVolumeTest, ConstructionDirectional) {
    std::vector<Vector3> vertices;
    std::vector<uint32_t> indices;
    createCubeMesh(vertices, indices);

    Vector3 lightDir(0.0f, -1.0f, 0.0f);  // Light pointing down
    ShadowVolume shadow(vertices, indices, lightDir, true);

    // Check basic properties
    const auto& volumeVerts = shadow.getVolumeVertices();
    const auto& volumeIndices = shadow.getVolumeIndices();

    EXPECT_FALSE(volumeVerts.empty());
    EXPECT_FALSE(volumeIndices.empty());

    // For a cube lit from above, we expect:
    // - Bottom face as front cap (2 triangles)
    // - 4 edges from bottom face extruded (4 quads = 8 triangles)
    // - Top face as back cap (2 triangles)
    // Total: 12 triangles = 36 indices
    EXPECT_EQ(volumeIndices.size(), 36U);
}

TEST(ShadowVolumeTest, ConstructionPointLight) {
    std::vector<Vector3> vertices;
    std::vector<uint32_t> indices;
    createCubeMesh(vertices, indices);

    Vector3 lightPos(3.0f, 3.0f, 3.0f);  // Light above and to the side
    ShadowVolume shadow(vertices, indices, lightPos, false);

    const auto& volumeVerts = shadow.getVolumeVertices();
    const auto& volumeIndices = shadow.getVolumeIndices();

    EXPECT_FALSE(volumeVerts.empty());
    EXPECT_FALSE(volumeIndices.empty());

    // For point light, silhouette depends on light position
    // but we should still have closed volume with proper winding
    EXPECT_EQ(volumeIndices.size() % 3, 0U);  // Must be multiple of 3
}

TEST(ShadowVolumeTest, UpdateLight) {
    std::vector<Vector3> vertices;
    std::vector<uint32_t> indices;
    createCubeMesh(vertices, indices);

    Vector3 lightPos(3.0f, 3.0f, 3.0f);
    ShadowVolume shadow(vertices, indices, lightPos, false);

    const auto& origVerts = shadow.getVolumeVertices();
    const auto& origIndices = shadow.getVolumeIndices();
    size_t origNumVerts = origVerts.size();
    size_t origNumIndices = origIndices.size();

    // Move light to opposite side
    Vector3 newLightPos(-3.0f, -3.0f, -3.0f);
    shadow.updateLight(newLightPos);

    const auto& newVerts = shadow.getVolumeVertices();
    const auto& newIndices = shadow.getVolumeIndices();

    // Volume should be similar size but different vertices
    EXPECT_EQ(newVerts.size(), origNumVerts);
    EXPECT_EQ(newIndices.size(), origNumIndices);
    EXPECT_NE(newVerts[0], origVerts[0]);  // Vertices should differ
}

TEST(ShadowVolumeTest, SilhouetteEdges) {
    std::vector<Vector3> vertices;
    std::vector<uint32_t> indices;
    createCubeMesh(vertices, indices);

    // Test light positions that create known silhouettes
    Vector3 lightPositions[] = {
        Vector3(0.0f, 3.0f, 0.0f),   // Above
        Vector3(3.0f, 0.0f, 0.0f),   // Right
        Vector3(0.0f, 0.0f, 3.0f)    // Front
    };

    size_t expectedSilhouetteEdges[] = {
        4,  // Light above: 4 edges around top face
        4,  // Light right: 4 edges around right face
        4   // Light front: 4 edges around front face
    };

    for (size_t i = 0; i < 3; ++i) {
        ShadowVolume shadow(vertices, indices, lightPositions[i], false);
        const auto& volumeIndices = shadow.getVolumeIndices();
        
        // Each silhouette edge creates 2 triangles (6 indices)
        // plus 2 cap triangles (6 indices)
        size_t expectedIndices = expectedSilhouetteEdges[i] * 6 + 6;
        EXPECT_EQ(volumeIndices.size(), expectedIndices);
    }
}

TEST(ShadowVolumeTest, VolumeExtrusion) {
    std::vector<Vector3> vertices;
    std::vector<uint32_t> indices;
    createCubeMesh(vertices, indices);

    Vector3 lightPos(0.0f, 3.0f, 0.0f);  // Light above
    ShadowVolume shadow(vertices, indices, lightPos, false);

    const auto& volumeVerts = shadow.getVolumeVertices();

    // Find the maximum extent of the extruded volume
    float maxY = -std::numeric_limits<float>::max();
    for (const auto& v : volumeVerts) {
        maxY = std::max(maxY, v.y);
    }

    // Volume should extend far from the light
    float minExpectedLength = 100.0f;  // Shadow volume should be long enough
    EXPECT_GT(std::abs(maxY), minExpectedLength);
}

TEST(ShadowVolumeTest, VolumeCaps) {
    std::vector<Vector3> vertices;
    std::vector<uint32_t> indices;
    createCubeMesh(vertices, indices);

    Vector3 lightDir(0.0f, -1.0f, 0.0f);  // Directional light down
    ShadowVolume shadow(vertices, indices, lightDir, true);

    const auto& volumeVerts = shadow.getVolumeVertices();
    const auto& volumeIndices = shadow.getVolumeIndices();

    // Verify volume has front and back caps
    // For a cube lit from above:
    // - Bottom face forms front cap (2 tris)
    // - Silhouette edges form sides (8 tris)
    // - Extruded top face forms back cap (2 tris)
    size_t expectedTriangles = 12;  // 2 + 8 + 2
    EXPECT_EQ(volumeIndices.size(), expectedTriangles * 3);

    // Verify winding order
    // Front cap should be CCW, back cap CW
    for (size_t i = 0; i < volumeIndices.size(); i += 3) {
        const Vector3& v0 = volumeVerts[volumeIndices[i]];
        const Vector3& v1 = volumeVerts[volumeIndices[i + 1]];
        const Vector3& v2 = volumeVerts[volumeIndices[i + 2]];

        Vector3 normal = (v1 - v0).cross(v2 - v0);
        float dot = normal.dot(lightDir);

        // Normal should point away from volume
        EXPECT_NE(dot, 0.0f);  // Should not be degenerate
    }
}

TEST(ShadowVolumeTest, Performance) {
    std::vector<Vector3> vertices;
    std::vector<uint32_t> indices;
    createCubeMesh(vertices, indices);

    Vector3 lightPos(3.0f, 3.0f, 3.0f);
    const int numUpdates = 1000;

    auto start = std::chrono::high_resolution_clock::now();
    
    ShadowVolume shadow(vertices, indices, lightPos, false);
    for (int i = 0; i < numUpdates; ++i) {
        // Move light in a circle
        float angle = (float)i * 0.1f;
        Vector3 newPos(
            3.0f * std::cos(angle),
            3.0f,
            3.0f * std::sin(angle)
        );
        shadow.updateLight(newPos);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    float avgUpdateTime = duration.count() / (float)numUpdates;

    // Performance targets:
    // - Initial construction: < 100μs
    // - Light updates: < 50μs average
    EXPECT_LT(avgUpdateTime, 50.0f);
}

} // namespace lighting
} // namespace math
} // namespace pynovage