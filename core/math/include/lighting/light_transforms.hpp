#ifndef PYNOVAGE_MATH_LIGHTING_TRANSFORMS_HPP
#define PYNOVAGE_MATH_LIGHTING_TRANSFORMS_HPP

#include "../matrix4.hpp"
#include "../vector3.hpp"
#include "light_types.hpp"
#include "directional_light.hpp"
#include "point_light.hpp"
#include "spot_light.hpp"

namespace pynovage {
namespace math {
namespace lighting {

/**
 * @brief Helper class for computing light space transforms for shadow mapping
 * 
 * Provides utilities for creating view and projection matrices for different light types:
 * - Directional lights (orthographic shadow mapping)
 * - Point lights (perspective shadow mapping)
 * - Spot lights (perspective shadow mapping)
 * 
 * All projection matrices use reversed Z depth mapping for better precision, where:
 * - Near plane maps to Z=1
 * - Far plane maps to Z=0
 */
class LightSpaceTransform {
public:
    /**
     * @brief Creates a view matrix for directional light shadow mapping
     * @param light The directional light
     * @param center Center point of the visible region
     * @param radius Radius of the visible sphere region
     * @return View matrix transforming world space to light space
     */
    static Matrix4 createDirectionalLightView(
        const DirectionalLight& light,
        const Vector3& center,
        float radius);
        
    /**
     * @brief Creates a view matrix for point light shadow mapping
     * @param light The point light source
     * @return View matrix for one face of the cubemap
     */
    static Matrix4 createPointLightView(
        const PointLight& light,
        int face);  // 0-5 for +X,-X,+Y,-Y,+Z,-Z
        
    /**
     * @brief Creates a view matrix for spot light shadow mapping
     * @param light The spot light source
     * @return View matrix transforming world space to light space
     */
    static Matrix4 createSpotLightView(
        const SpotLight& light);

    /**
     * @brief Creates a projection matrix for directional light shadow mapping
     * @param light The directional light
     * @param center Center point of the visible region
     * @param radius Radius of the visible sphere region
     * @param nearPlane Near plane distance
     * @param farPlane Far plane distance
     * @return Orthographic projection matrix with reversed Z
     */
    static Matrix4 createDirectionalLightProjection(
        const DirectionalLight& light,
        const Vector3& center,
        float radius,
        float nearPlane,
        float farPlane);
        
    /**
     * @brief Creates a projection matrix for point light shadow mapping
     * @param light The point light source
     * @param nearPlane Near plane distance (should match light's min radius)
     * @return Perspective projection matrix with reversed Z and 90-degree FOV
     */
    static Matrix4 createPointLightProjection(
        const PointLight& light,
        float nearPlane);
        
    /**
     * @brief Creates a projection matrix for spot light shadow mapping
     * @param light The spot light
     * @param nearPlane Near plane distance (should match light's min radius)
     * @return Perspective projection matrix with reversed Z
     */
    static Matrix4 createSpotLightProjection(
        const SpotLight& light,
        float nearPlane);

    /**
     * @brief Creates a combined view-projection matrix for directional shadow mapping
     * @param light The directional light
     * @param center Center of visible region
     * @param radius Radius of visible region
     * @param nearPlane Near plane distance
     * @param farPlane Far plane distance
     * @return Combined view-projection matrix
     */
    static Matrix4 createLightSpaceTransform(
        const DirectionalLight& light,
        const Vector3& center = Vector3(),
        float radius = 1.0f,
        float nearPlane = 0.1f,
        float farPlane = 100.0f);
        
    /**
     * @brief Creates a combined view-projection matrix for point light shadow mapping
     * @param light The point light
     * @param face Cubemap face index (0-5)
     * @param nearPlane Near plane distance
     * @return Combined view-projection matrix for the specified face
     */
    static Matrix4 createLightSpaceTransform(
        const PointLight& light,
        int face,
        float nearPlane = 0.1f);
        
    /**
     * @brief Creates a combined view-projection matrix for spot light shadow mapping
     * @param light The spot light
     * @param nearPlane Near plane distance
     * @return Combined view-projection matrix
     */
    static Matrix4 createLightSpaceTransform(
        const SpotLight& light,
        float nearPlane = 0.1f);

private:
    /**
     * @brief Calculate the bounds of the visible region for directional shadows
     * @param direction Light direction
     * @param center Center of visible region
     * @param radius Radius of visible region
     * @param[out] min Minimum bounds in light space
     * @param[out] max Maximum bounds in light space
     */
    static void calculateDirectionalBounds(
        const Vector3& direction,
        const Vector3& center,
        float radius,
        Vector3& min,
        Vector3& max);

    /**
     * @brief Calculate the view transform for a cubemap face
     * @param position Light position
     * @param face Cubemap face index (0-5)
     * @return View matrix for the specified face
     */
    static Matrix4 calculateCubemapFaceView(
        const Vector3& position,
        int face);

    /**
     * @brief Calculate field of view and aspect ratio for spot light projection
     * @param light The spot light
     * @param[out] fovY Vertical field of view in radians
     * @param[out] aspect Aspect ratio (width/height)
     */
    static void calculateSpotLightFrustum(
        const SpotLight& light,
        float& fovY,
        float& aspect);

    /**
     * @brief Creates a normal bias matrix for shadow mapping
     * @param light The light source
     * @param normalBias Normal-based bias factor (typically small, like 0.005)
     * @return Matrix that offsets positions along their normals
     */
    template<typename LightType>
    static Matrix4 createNormalBiasMatrix(
        const LightType& light,
        float normalBias);

    /**
     * @brief Creates a constant depth bias matrix for shadow mapping
     * @param depthBias Constant depth bias factor (typically small, like 0.0001)
     * @param slopeScale Slope scale factor for slope-scaled depth bias
     * @return Matrix that adds a constant bias to depth
     */
    static Matrix4 createDepthBiasMatrix(
        float depthBias = 0.0001f,
        float slopeScale = 1.0f);
};

} // namespace lighting
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_LIGHTING_TRANSFORMS_HPP
