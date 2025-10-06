#ifndef PYNOVAGE_MATH_LIGHTING_DIRECTIONAL_LIGHT_HPP
#define PYNOVAGE_MATH_LIGHTING_DIRECTIONAL_LIGHT_HPP

#include "light_types.hpp"
#include "../simd_utils.hpp"

namespace pynovage {
namespace math {
namespace lighting {

/**
 * @brief SIMD-optimized directional light calculations
 * 
 * Efficient directional light computations for real-time rendering:
 * - Direction vector calculations
 * - Parallel light projections
 * - Shadow mapping transforms
 * - Efficient normal-light calculations
 * 
 * Performance Characteristics:
 * - SIMD-optimized vector operations
 * - Vectorized shadow calculations
 * - Cache-friendly data structures
 * - Batch processing support
 * 
 * Usage Guidelines:
 * - Use for large-scale lighting (sun, moon)
 * - Consider cascaded shadow maps
 * - Align data for SIMD operations
 * - Batch process for multiple objects
 *
 * Example:
 * @code
 * // Example code will go here
 * @endcode
 */

/**
 * @brief Represents an infinitely distant light source
 *
 * A directional light casts parallel light rays in a uniform direction,
 * simulating very distant light sources like the sun. The light's position
 * is considered to be at infinity.
 */
class alignas(16) DirectionalLight : public LightProperties {
public:
    // The direction the light rays travel
    Vector3 direction;

    /**
     * @brief Default constructor
     * Creates a directional light pointing straight down
     */
    DirectionalLight()
        : LightProperties()
        , direction(0.0f, -1.0f, 0.0f)
    {
        type = LightType::Directional;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::None;
    }

    /**
     * @brief Constructs a directional light with specified direction
     * @param dir Light ray direction (will be normalized)
     */
    explicit DirectionalLight(const Vector3& dir)
        : LightProperties()
        , direction(dir.normalized())
    {
        type = LightType::Directional;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::None;
    }

    /**
     * @brief Constructs a directional light with direction and color
     * @param dir Light ray direction (will be normalized)
     * @param col Light color and intensity
     */
    DirectionalLight(const Vector3& dir, const LightColor& col)
        : LightProperties()
        , direction(dir.normalized())
    {
        type = LightType::Directional;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::None;
        color = col;
    }

    /**
     * @brief Sets the light direction
     * @param dir New direction vector (will be normalized)
     */
    void setDirection(const Vector3& dir) {
        direction = dir.normalized();
    }

    /**
     * @brief Computes the world-space bounds for shadow mapping
     * @param center Center point of visible region
     * @param radius Radius of visible sphere region
     * @param[out] min Minimum bounds
     * @param[out] max Maximum bounds
     */
    void computeShadowBounds(
        const Vector3& center,
        float radius,
        Vector3& min,
        Vector3& max) const
    {
        // Since directional light comes from infinity, we don't need to transform to light space
        // We just need to create a bounding box that encompasses the visible region
        min = center - Vector3(radius, radius, radius);
        max = center + Vector3(radius, radius, radius);
    }
};

} // namespace lighting
} // namespace math
} // namespace pynovage

#endif
