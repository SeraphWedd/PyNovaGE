#ifndef PYNOVAGE_MATH_LIGHTING_VOLUMETRIC_LIGHT_HPP
#define PYNOVAGE_MATH_LIGHTING_VOLUMETRIC_LIGHT_HPP

#include "../vector3.hpp"
#include "../vector4.hpp"
#include "../matrix4.hpp"
#include "light_types.hpp"
#include "light_transforms.hpp"

namespace pynovage {
namespace math {
namespace lighting {

/**
 * @brief Parameters for volumetric medium properties
 */
struct VolumetricMedium {
    float scattering_coefficient = 0.1f;    // How much light is scattered
    float absorption_coefficient = 0.01f;    // How much light is absorbed
    float asymmetry_factor = 0.0f;          // Phase function asymmetry (-1 to 1)
    float density = 1.0f;                   // Medium density multiplier
};

/**
 * @brief Parameters for volume sampling
 */
struct VolumeSamplingParams {
    int num_steps = 64;                     // Number of steps for ray marching
    float step_size = 1.0f;                 // Size of each step
    float jitter_strength = 0.1f;           // Random jitter for sampling
    bool use_adaptive_sampling = true;      // Whether to use adaptive sampling
};

/**
 * @brief Result of volumetric light calculation
 */
struct VolumetricResult {
    Vector3 scattered_light;                // Amount of scattered light
    float transmittance;                    // How much light passes through
};

/**
 * @brief Calculates volumetric scattering for a ray through participating media
 * 
 * Uses ray marching with optional adaptive sampling to calculate light scattering
 * through a participating medium.
 * 
 * @param ray_origin Starting point of the ray
 * @param ray_direction Direction of the ray (normalized)
 * @param light_position Position of the light source
 * @param light_color Color and intensity of the light
 * @param medium Properties of the participating medium
 * @param sampling Sampling parameters for ray marching
 * @return VolumetricResult containing scattered light and transmittance
 */
VolumetricResult CalculateVolumetricScattering(
    const Vector3& ray_origin,
    const Vector3& ray_direction,
    const Vector3& light_position,
    const Vector3& light_color,
    const VolumetricMedium& medium,
    const VolumeSamplingParams& sampling = VolumeSamplingParams());

/**
 * @brief Calculates phase function value for given angle
 * 
 * Implements the Henyey-Greenstein phase function for anisotropic scattering.
 * 
 * @param cos_angle Cosine of the angle between incoming and outgoing light
 * @param asymmetry_factor Phase function asymmetry parameter (-1 to 1)
 * @return float Phase function value
 */
float CalculatePhaseFunction(float cos_angle, float asymmetry_factor);

/**
 * @brief Calculates participating media density at a point
 * 
 * @param position Point to evaluate density at
 * @param medium Medium properties
 * @return float Density at the point
 */
float CalculateMediumDensity(const Vector3& position, const VolumetricMedium& medium);

/**
 * @brief Calculates volumetric shadows for a light source
 * 
 * @param sample_position Position to calculate shadow for
 * @param light_position Position of the light source
 * @param medium Medium properties
 * @param sampling Sampling parameters
 * @return float Shadow factor (0 = full shadow, 1 = no shadow)
 */
float CalculateVolumetricShadow(
    const Vector3& sample_position,
    const Vector3& light_position,
    const VolumetricMedium& medium,
    const VolumeSamplingParams& sampling = VolumeSamplingParams());

/**
 * @brief Performs adaptive sampling for volumetric lighting
 * 
 * Adjusts step size based on estimated contribution to final result.
 * 
 * @param current_position Current sampling position
 * @param light_position Light source position
 * @param medium Medium properties
 * @param base_step_size Base step size to adjust
 * @return float Adjusted step size
 */
float CalculateAdaptiveStepSize(
    const Vector3& current_position,
    const Vector3& light_position,
    const VolumetricMedium& medium,
    float base_step_size);

/**
 * @brief Calculates volumetric lighting for multiple lights
 * 
 * @param ray_origin Starting point of the ray
 * @param ray_direction Direction of the ray (normalized)
 * @param lights Array of light positions
 * @param light_colors Array of light colors/intensities
 * @param light_count Number of lights
 * @param medium Medium properties
 * @param sampling Sampling parameters
 * @return VolumetricResult Combined result from all lights
 */
VolumetricResult CalculateMultiLightScattering(
    const Vector3& ray_origin,
    const Vector3& ray_direction,
    const Vector3* lights,
    const Vector3* light_colors,
    int light_count,
    const VolumetricMedium& medium,
    const VolumeSamplingParams& sampling = VolumeSamplingParams());

} // namespace lighting
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_LIGHTING_VOLUMETRIC_LIGHT_HPP