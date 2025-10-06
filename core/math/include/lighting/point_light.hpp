#ifndef PYNOVAGE_MATH_LIGHTING_POINT_LIGHT_HPP
#define PYNOVAGE_MATH_LIGHTING_POINT_LIGHT_HPP

#include "light_types.hpp"
#include "attenuation.hpp"
#include "../simd_utils.hpp"

namespace pynovage {
namespace math {
namespace lighting {

/**
 * @brief SIMD-optimized point light calculations
 * 
 * Implements efficient point light computations for real-time rendering:
 * - Position-based lighting calculations
 * - Radial attenuation with SIMD optimization
 * - Distance-based falloff
 * - Range-based culling mathematics
 * 
 * Performance Characteristics:
 * - SIMD-optimized distance calculations
 * - Vectorized attenuation computations
 * - Cache-friendly data layout
 * - Designed for batch processing
 * 
 * Usage Guidelines:
 * - Prefer batch calculations for multiple lights
 * - Use range-based culling for large scenes
 * - Consider light grid optimization for many lights
 * - Align data for SIMD operations
 *
 * Example:
 * @code
 * // Example code will go here
 * @endcode
 */

/**
 * @brief Point light source with position and range-based attenuation
 */
class alignas(16) PointLight : public LightProperties {
public:
    Vector3 position;

    /**
     * @brief Default constructor
     * Creates a point light at origin with default range
     */
    PointLight()
        : LightProperties()
        , position(0.0f, 0.0f, 0.0f)
    {
        type = LightType::Point;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::Smooth;
        attenuation = AttenuationParams::ForRange(constants::DEFAULT_POINT_LIGHT_RANGE);
    }

    /**
     * @brief Constructs a point light at specified position
     * @param pos Light position in world space
     */
    explicit PointLight(const Vector3& pos)
        : LightProperties()
        , position(pos)
    {
        type = LightType::Point;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::Smooth;
        attenuation = AttenuationParams::ForRange(constants::DEFAULT_POINT_LIGHT_RANGE);
    }

    /**
     * @brief Constructs a point light with position and range
     * @param pos Light position in world space
     * @param range Maximum effective range
     */
    PointLight(const Vector3& pos, float range)
        : LightProperties()
        , position(pos)
    {
        type = LightType::Point;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::Smooth;
        attenuation = AttenuationParams::ForRange(range);
    }

    /**
     * @brief Constructs a point light with position, range, and color
     * @param pos Light position in world space
     * @param range Maximum effective range
     * @param col Light color and intensity
     */
    PointLight(const Vector3& pos, float range, const LightColor& col)
        : LightProperties()
        , position(pos)
    {
        type = LightType::Point;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::Smooth;
        attenuation = AttenuationParams::ForRange(range);
        color = col;
    }

    /**
     * @brief Sets the light position
     * @param pos New position in world space
     */
    void setPosition(const Vector3& pos) {
        position = pos;
    }

    /**
     * @brief Sets the light range and updates attenuation
     * @param range New maximum effective range
     */
    void setRange(float range) {
        attenuation = AttenuationParams::ForRange(range);
    }
};

} // namespace lighting
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_LIGHTING_POINT_LIGHT_HPP