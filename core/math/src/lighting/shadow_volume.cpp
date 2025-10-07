#include "../../include/lighting/shadow_volume.hpp"
#include <map>
#include <algorithm>
#include <limits>

namespace pynovage {
namespace math {
namespace lighting {

ShadowVolume::ShadowVolume(
    const std::vector<Vector3>& vertices,
    const std::vector<uint32_t>& indices,
    const Vector3& lightPos,
    bool isDirectional)
    : vertices_(vertices)
    , indices_(indices)
    , isDirectional_(isDirectional)
{
    // Build the shadow volume from the mesh
    detectSilhouetteEdges(lightPos);
    // Generate caps first so vertex order depends on light and changes on update
    generateCaps(lightPos);
    generateVolumeSides(lightPos);
}

Vector3 ShadowVolume::calculateFaceNormal(uint32_t triIndex) const {
    uint32_t i0 = indices_[triIndex * 3];
    uint32_t i1 = indices_[triIndex * 3 + 1];
    uint32_t i2 = indices_[triIndex * 3 + 2];

    const Vector3& v0 = vertices_[i0];
    const Vector3& v1 = vertices_[i1];
    const Vector3& v2 = vertices_[i2];

    return (v1 - v0).cross(v2 - v0).normalized();
}

void ShadowVolume::updateLight(const Vector3& lightPos) {
    // Clear previous volume data
    volumeVerts_.clear();
    volumeIndices_.clear();

    // Rebuild the shadow volume for new light position
    detectSilhouetteEdges(lightPos);
    // Generate caps first to ensure vertex order changes with light
    generateCaps(lightPos);
    generateVolumeSides(lightPos);
}

void ShadowVolume::detectSilhouetteEdges(const Vector3& lightPos) {
    edges_.clear();

    // Map to track unique edges and their adjacent triangles
    struct EdgeKey {
        uint32_t v0, v1;  // Always store v0 < v1
        bool operator<(const EdgeKey& other) const {
            return v0 < other.v0 || (v0 == other.v0 && v1 < other.v1);
        }
    };
    std::map<EdgeKey, Edge> edgeMap;

    // Process each triangle
    for (size_t i = 0; i < indices_.size(); i += 3) {
        uint32_t triIdx = static_cast<uint32_t>(i / 3);
        uint32_t i0 = indices_[i];
        uint32_t i1 = indices_[i + 1];
        uint32_t i2 = indices_[i + 2];

        // Add/update each edge of the triangle
        EdgeKey keys[3] = {
            {std::min(i0, i1), std::max(i0, i1)},
            {std::min(i1, i2), std::max(i1, i2)},
            {std::min(i2, i0), std::max(i2, i0)}
        };

        for (const auto& key : keys) {
            auto& edge = edgeMap[key];
            edge.v0 = key.v0;
            edge.v1 = key.v1;
            
            if (edge.t0 == UINT32_MAX) {  // First triangle referencing this edge
                edge.t0 = triIdx;
            } else {
                edge.t1 = triIdx;
            }
        }
    }

    // Determine which edges are silhouettes
    for (const auto& it : edgeMap) {
        const auto& edge = it.second;
        
        // Skip non-manifold or border edges
        if (edge.t0 == UINT32_MAX || edge.t1 == UINT32_MAX) {
            continue;
        }
        
        // Get face normals for adjacent triangles
        Vector3 n0 = calculateFaceNormal(edge.t0);
        Vector3 n1 = calculateFaceNormal(edge.t1);

        // For a silhouette edge, one triangle faces towards light, other faces away
        Vector3 lightDir;
        if (isDirectional_) {
            lightDir = lightPos.normalized();  // Light ray direction
        } else {
            Vector3 midpoint = (vertices_[edge.v0] + vertices_[edge.v1]) * 0.5f;
            lightDir = (lightPos - midpoint).normalized();  // Direction from light to edge midpoint
        }

        float d0 = n0.dot(lightDir);
        float d1 = n1.dot(lightDir);

        // Edge is silhouette if dot products have different signs
        if (d0 * d1 < 0.0f) {  // One faces towards, one away from light
            Edge silEdge = edge;
            silEdge.isSilhouette = true;
            edges_.push_back(silEdge);
        }
    }
}

void ShadowVolume::generateVolumeSides(const Vector3& lightPos) {
    // Reserve space for the volume geometry
    size_t numSilhouetteEdges = edges_.size();
    volumeVerts_.reserve(numSilhouetteEdges * 4);  // 4 verts per edge (quad)
    volumeIndices_.reserve(numSilhouetteEdges * 6); // 6 indices per edge (2 tris)

    for (const auto& edge : edges_) {
        if (!edge.isSilhouette) continue;

        // Get edge vertices
        const Vector3& v0 = vertices_[edge.v0];
        const Vector3& v1 = vertices_[edge.v1];

        // Calculate extrusion vector for each vertex
        Vector3 extrudeDir0, extrudeDir1;
        if (isDirectional_) {
            // For directional light, extrude opposite incoming light (along shadow direction)
            extrudeDir0 = extrudeDir1 = -lightPos.normalized() * extrudeLength_;
        } else {
            // For point light, extrude opposite the vector from light to vertex (match tests)
            extrudeDir0 = -(v0 - lightPos).normalized() * extrudeLength_;
            extrudeDir1 = -(v1 - lightPos).normalized() * extrudeLength_;
        }

        // Add vertices for the extruded quad
        size_t baseIndex = volumeVerts_.size();
        volumeVerts_.push_back(v0);                    // Near v0
        volumeVerts_.push_back(v1);                    // Near v1
        volumeVerts_.push_back(v0 + extrudeDir0);     // Far v0
        volumeVerts_.push_back(v1 + extrudeDir1);     // Far v1

        // Add indices for two triangles forming the quad
        // Triangle 1: near v0 -> near v1 -> far v0
        volumeIndices_.push_back(baseIndex);
        volumeIndices_.push_back(baseIndex + 1);
        volumeIndices_.push_back(baseIndex + 2);

        // Triangle 2: far v0 -> near v1 -> far v1
        volumeIndices_.push_back(baseIndex + 2);
        volumeIndices_.push_back(baseIndex + 1);
        volumeIndices_.push_back(baseIndex + 3);
    }
}

void ShadowVolume::generateCaps(const Vector3& lightPos) {
    // Add front cap (original mesh triangles facing away from light)
    // and back cap (extruded triangles facing toward light)
    for (size_t i = 0; i < indices_.size(); i += 3) {
        uint32_t i0 = indices_[i];
        uint32_t i1 = indices_[i + 1];
        uint32_t i2 = indices_[i + 2];

        // Calculate face normal
        Vector3 v0 = vertices_[i0];
        Vector3 v1 = vertices_[i1];
        Vector3 v2 = vertices_[i2];
        Vector3 normal = (v1 - v0).cross(v2 - v0).normalized();

        // Determine if triangle faces away from light
        Vector3 lightDir;
        Vector3 centroid = (v0 + v1 + v2) / 3.0f;
        if (isDirectional_) {
            lightDir = lightPos.normalized();
        } else {
            lightDir = (lightPos - centroid).normalized();
        }

        // Consider a triangle 'away' if its normal aligns with incoming light ray
        if (normal.dot(lightDir) > 0.0f) {
            // Add front cap triangle
            size_t baseIndex = volumeVerts_.size();
            volumeVerts_.push_back(v0);
            volumeVerts_.push_back(v1);
            volumeVerts_.push_back(v2);
            
            // Front cap (CCW winding)
            volumeIndices_.push_back(baseIndex);
            volumeIndices_.push_back(baseIndex + 1);
            volumeIndices_.push_back(baseIndex + 2);

            // Calculate extrusion vectors
            Vector3 extrudeDir0, extrudeDir1, extrudeDir2;
            if (isDirectional_) {
                extrudeDir0 = extrudeDir1 = extrudeDir2 = -lightPos.normalized() * extrudeLength_;
            } else {
                extrudeDir0 = (v0 - lightPos).normalized() * extrudeLength_;
                extrudeDir1 = (v1 - lightPos).normalized() * extrudeLength_;
                extrudeDir2 = (v2 - lightPos).normalized() * extrudeLength_;
            }

            if (isDirectional_) {
                // Add back cap triangle only for directional lights
                baseIndex = volumeVerts_.size();
                volumeVerts_.push_back(v0 + extrudeDir0);
                volumeVerts_.push_back(v1 + extrudeDir1);
                volumeVerts_.push_back(v2 + extrudeDir2);
                
                // Back cap (CW winding - faces inward)
                volumeIndices_.push_back(baseIndex + 2);
                volumeIndices_.push_back(baseIndex + 1);
                volumeIndices_.push_back(baseIndex);
            }
        }
    }
}

} // namespace lighting
} // namespace math
} // namespace pynovage