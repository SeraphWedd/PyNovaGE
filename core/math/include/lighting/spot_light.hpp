#ifndef PYNOVAGE_MATH_LIGHTING_SPOT_LIGHT_HPP
#define PYNOVAGE_MATH_LIGHTING_SPOT_LIGHT_HPP

#include "light_types.hpp"
#include "attenuation.hpp"
#include "../simd_utils.hpp"

namespace pynovage {
namespace math {
namespace lighting {

/**
 * @brief SIMD-optimized spotlight calculations
 * 
 * Fast spotlight computations for real-time rendering:
 * - Cone angle calculations
 * - Angular attenuation
 * - Distance attenuation
 * - Efficient cone-vector tests
 * 
 * Performance Characteristics:
 * - SIMD-optimized angle calculations
 * - Vectorized attenuation computations
 * - Cache-aligned data structures
 * - Optimized cone tests
 * 
 * Usage Guidelines:
 * - Use for focused light sources
 * - Consider cookie/gobo textures
 * - Batch process multiple lights
 * - Align data for SIMD operations
 *
 * Example:
 * @code
 * // Example code will go here
 * @endcode
 */

/**
 * @brief Spot light source with position, direction, and cone angle
 */
class alignas(16) SpotLight : public LightProperties {
public:
    Vector3 position;
    Vector3 direction;
    float outerAngle;  // Angle in radians from center to edge of cone
    float innerAngle;  // Angle in radians for smooth inner falloff

    /**
     * @brief Default constructor
     * Creates a spot light at origin pointing down with default angles
     */
    SpotLight()
        : LightProperties()
        , position(0.0f, 0.0f, 0.0f)
        , direction(0.0f, -1.0f, 0.0f)
        , outerAngle(constants::DEFAULT_SPOT_OUTER_ANGLE)
        , innerAngle(constants::DEFAULT_SPOT_INNER_ANGLE)
    {
        type = LightType::Spot;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::Smooth;
        attenuation = AttenuationParams::ForRange(constants::DEFAULT_SPOT_LIGHT_RANGE);
    }

    /**
     * @brief Constructs a spot light with position and direction
     * @param pos Light position in world space
     * @param dir Light direction (will be normalized)
     */
    SpotLight(const Vector3& pos, const Vector3& dir)
        : LightProperties()
        , position(pos)
        , direction(dir.normalized())
        , outerAngle(constants::DEFAULT_SPOT_OUTER_ANGLE)
        , innerAngle(constants::DEFAULT_SPOT_INNER_ANGLE)
    {
        type = LightType::Spot;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::Smooth;
        attenuation = AttenuationParams::ForRange(constants::DEFAULT_SPOT_LIGHT_RANGE);
    }

    /**
     * @brief Constructs a spot light with position, direction, and angles
     * @param pos Light position in world space
     * @param dir Light direction (will be normalized)
     * @param angle Outer cone angle in radians
     */
    SpotLight(const Vector3& pos, const Vector3& dir, float angle)
        : LightProperties()
        , position(pos)
        , direction(dir.normalized())
        , outerAngle(angle)
        , innerAngle(angle * 0.95f)  // Default inner angle at 95% of outer
    {
        type = LightType::Spot;
        features = LightFeatures::CastShadows;
        attenuationModel = AttenuationModel::Smooth;
        attenuation = AttenuationParams::ForRange(constants::DEFAULT_SPOT_LIGHT_RANGE);
    }

    /**
     * @brief Constructs a spot light with all parameters
     * @param pos Light position in world space
     * @param dir Light direction (will be normalized)
     * @param outer Outer cone angle in radians
     * @param inner Inner cone angle in radians
     * @param range Maximum effective range
     * @param col Light color and intensity
     */
    SpotLight(const Vector3& pos, const Vector3& dir,
              float outer, float inner,
              float range, const LightColor& col)
        : LightProperties()
        , position(pos)
        , direction(dir.normalized())
        , outerAngle(outer)
        , innerAngle(inner)
    {
        type = LightType::Spot;
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
     * @brief Sets the light direction
     * @param dir New direction (will be normalized)
     */
    void setDirection(const Vector3& dir) {
        direction = dir.normalized();
    }

    /**
     * @brief Sets the cone angles
     * @param outer Outer cone angle in radians
     * @param inner Inner cone angle in radians (must be <= outer)
     */
    void setAngles(float outer, float inner) {
        outerAngle = outer;
        innerAngle = std::min(inner, outer);
    }
};

} // namespace lighting
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_LIGHTING_SPOT_LIGHT_HPP