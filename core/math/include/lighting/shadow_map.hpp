#ifndef PYNOVAGE_MATH_LIGHTING_SHADOW_MAP_HPP
#define PYNOVAGE_MATH_LIGHTING_SHADOW_MAP_HPP

#include "../vector2.hpp"
#include "../vector3.hpp"
#include "../vector4.hpp"
#include "../matrix4.hpp"
#include "light_types.hpp"
#include <cstdint>
#include <vector>

namespace pynovage {
namespace math {
namespace lighting {

/**
 * @brief Types of shadow maps supported by the engine
 */
enum class ShadowMapType {
    Standard,   ///< Standard depth shadow map for directional and spot lights
    Cascade,    ///< Cascaded shadow map for directional lights (better quality at various distances)
    Cube        ///< Cubemap shadow for point lights (omnidirectional shadows)
};

/**
 * @brief Parameters for shadow map quality and bias settings
 * 
 * Contains configuration for shadow map resolution, filtering, and bias
 * parameters to reduce common shadow mapping artifacts.
 */
struct alignas(16) ShadowMapParameters {
    uint32_t resolution;         ///< Shadow map texture resolution (power of 2)
    float bias;                  ///< Basic depth bias to prevent shadow acne
    float normalBias;           ///< Normal-oriented bias for slope-scaled corrections
    float bleedReduction;       ///< Reduces light bleeding through thin objects
    uint32_t pcfSamples;        ///< Number of samples for percentage-closer filtering
    float pcfRadius;            ///< Radius for PCF sampling (in texels)
    
    /**
     * @brief Default constructor with common settings
     */
    ShadowMapParameters()
        : resolution(1024)
        , bias(0.005f)
        , normalBias(0.4f)
        , bleedReduction(0.2f)
        , pcfSamples(16)
        , pcfRadius(3.0f)
    {}
};

/**
 * @brief Structure holding cascade split distances for CSM
 */
struct alignas(16) CascadeConfig {
    static constexpr uint32_t MAX_CASCADES = 4;
    float splitDistances[MAX_CASCADES];    ///< Distance for each cascade split
    uint32_t numCascades;                  ///< Number of active cascades
    float cascadeBlendDistance;            ///< Distance to blend between cascades
    
    /**
     * @brief Default constructor for typical cascade configuration
     */
    CascadeConfig()
        : splitDistances{20.0f, 50.0f, 100.0f, 200.0f}
        , numCascades(4)
        , cascadeBlendDistance(5.0f)
    {}
};

/**
 * @brief Core shadow map class for real-time shadow calculations
 * 
 * Manages shadow map resources and provides functionality for:
 * - Shadow map creation and updates
 * - Transform matrix management
 * - Filtering and sampling operations
 * - Cascade handling for directional lights
 * - Cubemap faces for point lights
 */
class alignas(16) ShadowMap {
public:
    /**
     * @brief Creates a new shadow map with specified type
     * @param type The type of shadow map to create
     * @param params Shadow quality parameters
     */
    ShadowMap(ShadowMapType type = ShadowMapType::Standard,
              const ShadowMapParameters& params = ShadowMapParameters())
        : type_(type)
        , params_(params)
        , viewMatrix_(Matrix4::identity())
        , projMatrix_(Matrix4::identity())
    {
        if (type == ShadowMapType::Cube) {
            // For cubemaps, initialize 6 view matrices for each face
            cubeViewMatrices_.resize(6, Matrix4::identity());
        }
    }

    /**
     * @brief Updates the view matrix for the shadow map
     * @param lightPos World space light position
     * @param lightDir World space light direction (for directional/spot lights)
     */
    void updateViewMatrix(const Vector3& lightPos, const Vector3& lightDir = Vector3(0.0f, -1.0f, 0.0f)) {
        if (type_ == ShadowMapType::Cube) {
            updateCubeViewMatrices(lightPos);
        } else {
            // For standard shadow maps with directional lights, we need to:
            // 1. Transform positions relative to light source
            // 2. Ensure light direction maps to -Y in view space
            
            // First create rotation part - align light direction with -Y
            Vector3 dir = lightDir.normalized();
            
            // Create basis vectors for the view matrix
            // We want the light direction to be the "forward" vector (-Z in view space)
            Vector3 forward = dir;
            Vector3 right = Vector3(1.0f, 0.0f, 0.0f);
            if (std::fabs(forward.dot(right)) > 0.99f) {
                right = Vector3(0.0f, 1.0f, 0.0f);
            }
            
            Vector3 up = forward.cross(right).normalized();
            right = forward.cross(up).normalized();
            
            // Construct view matrix (column-major)
            viewMatrix_ = Matrix4(
                right.x,   right.y,   right.z,   0.0f,
                up.x,      up.y,      up.z,      0.0f,
                -dir.x,    -dir.y,    -dir.z,    0.0f,  // Light dir becomes -Z
                0.0f,      0.0f,      0.0f,      1.0f
            );
            
            // Apply translation
            viewMatrix_ = viewMatrix_ * Matrix4::translation(-lightPos.x, -lightPos.y, -lightPos.z);
        }
    }

    /**
     * @brief Updates the projection matrix for the shadow map
     * @param nearPlane Near plane distance
     * @param farPlane Far plane distance
     * @param fieldOfView Field of view in radians (for spot lights)
     */
void updateProjectionMatrix(float nearPlane, float farPlane, float fieldOfView = 90.0f) {
        if (type_ == ShadowMapType::Standard || type_ == ShadowMapType::Cascade) {
            // Check if we're a spot light (fieldOfView != 90)
            if (std::fabs(fieldOfView - 90.0f) > 1e-4f) {
                projMatrix_ = Matrix4::perspective(fieldOfView, 1.0f, nearPlane, farPlane);
            } else {
                // Orthographic for directional lights
                float size = 50.0f; // Size of the orthographic frustum
                projMatrix_ = Matrix4::orthographic(-size, size, -size, size, nearPlane, farPlane);
            }
        } else {
            // Perspective for point lights
            projMatrix_ = Matrix4::perspective(fieldOfView, 1.0f, nearPlane, farPlane);
        }
    }

    /**
     * @brief Gets the shadow map type
     */
    ShadowMapType getType() const { return type_; }

    /**
     * @brief Gets the shadow parameters
     */
    const ShadowMapParameters& getParameters() const { return params_; }

    /**
     * @brief Gets the view matrix
     */
    const Matrix4& getViewMatrix() const { return viewMatrix_; }

    /**
     * @brief Gets the projection matrix
     */
    const Matrix4& getProjectionMatrix() const { return projMatrix_; }

    /**
     * @brief Gets a specific cube face view matrix
     * @param face Face index (0-5)
     */
    const Matrix4& getCubeFaceViewMatrix(uint32_t face) const {
        return cubeViewMatrices_[face];
    }

private:
    /**
     * @brief Updates view matrices for all cube map faces
     * @param lightPos World space light position
     */
    void updateCubeViewMatrices(const Vector3& lightPos) {
        // Define the 6 view directions and up vectors for cube faces
        // Looking in each face's direction from the light position
        // Each entry represents where we're looking TO from the light
        static const Vector3 directions[6] = {
            Vector3(1.0f, 0.0f, 0.0f),    // Looking right (+X)
            Vector3(-1.0f, 0.0f, 0.0f),   // Looking left (-X)
            Vector3(0.0f, 1.0f, 0.0f),    // Looking up (+Y)
            Vector3(0.0f, -1.0f, 0.0f),   // Looking down (-Y)
            Vector3(0.0f, 0.0f, 1.0f),    // Looking forward (+Z)
            Vector3(0.0f, 0.0f, -1.0f)    // Looking back (-Z)
        };

        // Up vectors for each view to maintain correct orientation
        // These are the world-space up vectors that maintain proper orientation
        static const Vector3 ups[6] = {
            Vector3(0.0f, 1.0f, 0.0f),    // +X face: Y is up
            Vector3(0.0f, 1.0f, 0.0f),    // -X face: Y is up
            Vector3(0.0f, 0.0f, -1.0f),   // +Y face: -Z is up (looking up)
            Vector3(0.0f, 0.0f, 1.0f),    // -Y face: +Z is up (looking down)
            Vector3(0.0f, 1.0f, 0.0f),    // +Z face: Y is up
            Vector3(0.0f, 1.0f, 0.0f)     // -Z face: Y is up
        };

        // Update view matrix for each face
        for (int i = 0; i < 6; ++i) {
            // We want to look towards -Z in view space, so negate the direction
            Vector3 forward = -directions[i].normalized();
            Vector3 right = ups[i].cross(forward).normalized();
            Vector3 up = forward.cross(right).normalized();
            
            // Construct view matrix (column-major)
            cubeViewMatrices_[i] = Matrix4(
                right.x,   right.y,   right.z,   0.0f,
                up.x,      up.y,      up.z,      0.0f,
                forward.x, forward.y, forward.z, 0.0f,
                0.0f,      0.0f,      0.0f,      1.0f
            ) * Matrix4::translation(-lightPos.x, -lightPos.y, -lightPos.z);
        }
    }

    ShadowMapType type_;                    ///< Type of shadow map
    ShadowMapParameters params_;            ///< Shadow map parameters
    Matrix4 viewMatrix_;                    ///< View matrix for shadow rendering
    Matrix4 projMatrix_;                    ///< Projection matrix for shadow rendering
    std::vector<Matrix4> cubeViewMatrices_; ///< View matrices for cubemap faces
};

} // namespace lighting
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_LIGHTING_SHADOW_MAP_HPP