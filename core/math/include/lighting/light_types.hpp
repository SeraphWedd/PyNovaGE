#ifndef PYNOVAGE_MATH_LIGHTING_TYPES_HPP
#define PYNOVAGE_MATH_LIGHTING_TYPES_HPP

#include "../vector3.hpp"
#include "../vector4.hpp"

namespace pynovage {
namespace math {
namespace lighting {

/**
 * @brief Common types and constants for lighting calculations
 * 
 * Provides fundamental types and constants used across all lighting calculations.
 * All components are optimized for SIMD operations and designed for
 * real-time rendering scenarios.
 *
 * Performance Characteristics:
 * - All structures are SIMD-aligned
 * - Cache-friendly memory layout
 * - Vectorized operations ready
 * - Minimal branching in core operations
 *
 * Usage Guidelines:
 * - Prefer batch operations for multiple lights
 * - Keep data aligned for SIMD operations
 * - Use appropriate attenuation model for distance
 * - Consider performance vs quality tradeoffs
 */

// Physical constants for lighting calculations
namespace constants {
    // Default light ranges (in world units)
    constexpr float DEFAULT_POINT_LIGHT_RANGE = 10.0f;
    constexpr float DEFAULT_SPOT_LIGHT_RANGE = 10.0f;
    constexpr float MIN_LIGHT_RANGE = 0.1f;
    constexpr float MAX_LIGHT_RANGE = 1000.0f;

    // Default attenuation factors
    constexpr float DEFAULT_CONSTANT_ATTENUATION = 1.0f;
    constexpr float DEFAULT_LINEAR_ATTENUATION = 0.09f;
    constexpr float DEFAULT_QUADRATIC_ATTENUATION = 0.032f;

    // Spot light default angles (in radians)
    constexpr float DEFAULT_SPOT_INNER_ANGLE = 0.91629073f;  // 52.5 degrees
    constexpr float DEFAULT_SPOT_OUTER_ANGLE = 0.95993109f;  // 55.0 degrees
    
    // Efficiency thresholds
    constexpr float MINIMUM_LIGHT_INTENSITY = 1.0f/256.0f;  // 1/256 for 8-bit precision
} // namespace constants

// Light type identifiers
enum class LightType {
    Point,
    Directional,
    Spot
};

// Light features/flags
enum class LightFeatures : uint32_t {
    None = 0,
    CastShadows = 1 << 0,
    UseInverseSquare = 1 << 1,
    VolumetricEnabled = 1 << 2,
    // Add more features as needed
};

// Attenuation calculation models
enum class AttenuationModel {
    // Physical model (1/r²)
    InverseSquare,
    // Smoothed physical model (1/(1+r+r²))
    Smooth,
    // Linear falloff model
    Linear,
    // No attenuation
    None
};

/**
 * @brief Light color and intensity, optimized for SIMD
 * 
 * Represents RGB color and intensity in a SIMD-friendly format.
 * The w component is used for intensity to allow SIMD operations
 * on all components simultaneously.
 */
struct alignas(16) LightColor {
    float r;  // Red component
    float g;  // Green component
    float b;  // Blue component
    float i;  // Intensity multiplier

    LightColor()
        : r(1.0f), g(1.0f), b(1.0f), i(1.0f) {}
    
    LightColor(float r_, float g_, float b_, float i_ = 1.0f)
        : r(r_), g(g_), b(b_), i(i_) {}
};

/**
 * @brief Attenuation parameters for light falloff calculations
 * 
 * Stores parameters for various attenuation models in a SIMD-friendly format.
 * Different attenuation models will use different combinations of these parameters.
 */
struct alignas(16) AttenuationParams {
    float constant;    // Constant term
    float linear;      // Linear term (distance)
    float quadratic;   // Quadratic term (distance²)
    float range;       // Maximum effective range

    AttenuationParams()
        : constant(constants::DEFAULT_CONSTANT_ATTENUATION)
        , linear(constants::DEFAULT_LINEAR_ATTENUATION)
        , quadratic(constants::DEFAULT_QUADRATIC_ATTENUATION)
        , range(constants::DEFAULT_POINT_LIGHT_RANGE) {}

    AttenuationParams(float constant_, float linear_, float quadratic_, float range_)
        : constant(constant_)
        , linear(linear_)
        , quadratic(quadratic_)
        , range(range_) {}

    static AttenuationParams ForRange(float range) {
        // Calculate attenuation factors to reach minimum intensity at range
        // Based on physically-based factors from Epic's research
        float constant = 1.0f;
        float linear = 4.0f / range;
        float quadratic = 8.0f / (range * range);
        return AttenuationParams(constant, linear, quadratic, range);
    }
};

// Common light properties base structure
struct alignas(16) LightProperties {
    LightColor color;
    AttenuationParams attenuation;
    LightType type;
    LightFeatures features;
    AttenuationModel attenuationModel;

    LightProperties()
        : color()
        , attenuation()
        , type(LightType::Point)
        , features(LightFeatures::None)
        , attenuationModel(AttenuationModel::Smooth) {}
};

} // namespace lighting
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_LIGHTING_TYPES_HPP