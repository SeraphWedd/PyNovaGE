#pragma once

#include <vector>
#include <cstdint>
#include "../vector3.hpp"
#include "../matrix4.hpp"

namespace pynovage {
namespace math {
namespace lighting {

/**
 * @brief Represents a shadow volume for real-time shadow calculation
 * 
 * A shadow volume is a 3D geometric representation of the shadow cast by an
 * occluder. It is constructed by extruding the silhouette edges of the occluder
 * away from the light source.
 */
class ShadowVolume {
public:
    /**
     * @brief Creates an empty shadow volume
     */
    ShadowVolume() = default;

    /**
     * @brief Constructs a shadow volume from mesh data
     * 
     * @param vertices The mesh vertices in world space
     * @param indices The mesh triangle indices (must be closed manifold)
     * @param lightPos The world space position of the light
     * @param isDirectional True if the light is directional (parallel rays)
     */
    ShadowVolume(
        const std::vector<Vector3>& vertices,
        const std::vector<uint32_t>& indices,
        const Vector3& lightPos,
        bool isDirectional = false
    );

    /**
     * @brief Updates the shadow volume for a moving light
     * 
     * @param lightPos The new light position
     */
    void updateLight(const Vector3& lightPos);

    /**
     * @brief Gets the shadow volume vertices for rendering
     * @return Vector of vertices defining the shadow volume geometry
     */
    const std::vector<Vector3>& getVolumeVertices() const { return volumeVerts_; }

    /**
     * @brief Gets the shadow volume indices for rendering
     * @return Vector of indices defining the shadow volume triangles
     */
    const std::vector<uint32_t>& getVolumeIndices() const { return volumeIndices_; }

private:
    // Detects silhouette edges of the mesh relative to light position
    void detectSilhouetteEdges(const Vector3& lightPos);

    // Extrudes the silhouette edges to create the volume sides
    void generateVolumeSides(const Vector3& lightPos);

    // Generates front and back caps to close the volume
    void generateCaps(const Vector3& lightPos);

    // Original mesh data
    std::vector<Vector3> vertices_;
    std::vector<uint32_t> indices_;

    // Generated shadow volume geometry
    std::vector<Vector3> volumeVerts_;
    std::vector<uint32_t> volumeIndices_;

    // Silhouette edge data
    struct Edge {
        uint32_t v0 = 0, v1 = 0;       // Vertex indices
        uint32_t t0 = UINT32_MAX;      // Adjacent triangle indices (initialize invalid)
        uint32_t t1 = UINT32_MAX;
        bool isSilhouette = false;     // Is this a silhouette edge?
    };
    std::vector<Edge> edges_;

    // Shadow volume parameters
    float extrudeLength_ = 1000.0f;  // How far to extrude the volume
    bool isDirectional_ = false;     // Is this for a directional light?

    // Helpers
    Vector3 calculateFaceNormal(uint32_t triIndex) const;
};

} // namespace lighting
} // namespace math
} // namespace pynovage