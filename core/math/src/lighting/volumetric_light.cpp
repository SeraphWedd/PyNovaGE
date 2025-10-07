#include "lighting/volumetric_light.hpp"
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <random>

namespace pynovage {
namespace math {
namespace lighting {

namespace {
    // Thread-local random number generator for jittering
    thread_local std::mt19937 rng(std::random_device{}());
    thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Constants for optimization
    constexpr float kMinStepSize = 0.01f;
    constexpr float kMaxStepSize = 10.0f;
    constexpr float kDensityThreshold = 0.01f;
    constexpr float kPhaseNormalization = 0.07957747154f;  // 1/(4π)
}

float CalculatePhaseFunction(float cos_angle, float asymmetry_factor) {
    // Implement Henyey-Greenstein phase function
    float g = std::clamp(asymmetry_factor, -1.0f, 1.0f);
    float g2 = g * g;
    
    float numerator = 1.0f - g2;
    float denominator = std::pow(1.0f + g2 - 2.0f * g * cos_angle, 1.5f);
    
    return kPhaseNormalization * numerator / denominator;
}

float CalculateMediumDensity(const Vector3& position, const VolumetricMedium& medium) {
    // Basic uniform density for now
    return medium.density;
}

float CalculateAdaptiveStepSize(
    const Vector3& current_position,
    const Vector3& light_position,
    const VolumetricMedium& medium,
    float base_step_size) {
    
    // Calculate distance to light
    float distance_to_light = (light_position - current_position).length();
    
    // Get local density
    float local_density = CalculateMediumDensity(current_position, medium);
    
    // Adjust step size based on density and distance to light
    float density_factor = std::max(kDensityThreshold, local_density);
    float distance_factor = std::max(1.0f, distance_to_light * 0.1f);
    
    float adaptive_step = base_step_size * distance_factor / density_factor;
    
    // Clamp to reasonable range
    return std::clamp(adaptive_step, kMinStepSize, kMaxStepSize);
}

float CalculateVolumetricShadow(
    const Vector3& sample_position,
    const Vector3& light_position,
    const VolumetricMedium& medium,
    const VolumeSamplingParams& sampling) {
    
    Vector3 to_light = light_position - sample_position;
    float distance = to_light.length();
    Vector3 light_dir = to_light / distance;
    
    float transmittance = 1.0f;
    float current_distance = 0.0f;
    
    while (current_distance < distance) {
        Vector3 current_pos = sample_position + light_dir * current_distance;
        float density = CalculateMediumDensity(current_pos, medium);
        
        float step_size = sampling.use_adaptive_sampling
            ? CalculateAdaptiveStepSize(current_pos, light_position, medium, sampling.step_size)
            : sampling.step_size;
            
        transmittance *= std::exp(-(medium.absorption_coefficient + medium.scattering_coefficient) 
            * density * step_size);
            
        if (transmittance < 0.001f) break;  // Early exit if fully shadowed
        
        current_distance += step_size;
    }
    
    return transmittance;
}

VolumetricResult CalculateVolumetricScattering(
    const Vector3& ray_origin,
    const Vector3& ray_direction,
    const Vector3& light_position,
    const Vector3& light_color,
    const VolumetricMedium& medium,
    const VolumeSamplingParams& sampling) {
    
    VolumetricResult result;
    result.scattered_light = Vector3(0.0f, 0.0f, 0.0f);
    result.transmittance = 1.0f;
    
    float current_distance = 0.0f;
    
    for (int i = 0; i < sampling.num_steps; ++i) {
        // Add random jitter to reduce banding
        float jitter = sampling.jitter_strength * dist(rng);
        Vector3 current_pos = ray_origin + ray_direction * (current_distance + jitter);
        
        // Calculate step size
        float step_size = sampling.use_adaptive_sampling
            ? CalculateAdaptiveStepSize(current_pos, light_position, medium, sampling.step_size)
            : sampling.step_size;
            
        // Get medium properties at current point
        float density = CalculateMediumDensity(current_pos, medium);
        
        // Skip if density is too low
        if (density > kDensityThreshold) {
            // Calculate light contribution
            Vector3 to_light = light_position - current_pos;
            float light_distance = to_light.length();
            Vector3 light_dir = to_light / light_distance;
            
            float cos_angle = ray_direction.dot(light_dir);
            float phase = CalculatePhaseFunction(cos_angle, medium.asymmetry_factor);
            
            // Calculate shadow
            float shadow = CalculateVolumetricShadow(current_pos, light_position, medium, sampling);
            
            // Calculate scattering
            float scatter_amount = medium.scattering_coefficient * density * step_size;
            float transmittance_step = std::exp(-(medium.absorption_coefficient + medium.scattering_coefficient) 
                * density * step_size);
            
            // Accumulate light
            Vector3 in_scatter = light_color * scatter_amount * phase * shadow 
                / (light_distance * light_distance);
            result.scattered_light += result.transmittance * in_scatter;
            
            // Update transmittance
            result.transmittance *= transmittance_step;
            
            // Early exit if transmittance is too low
            if (result.transmittance < 0.001f) break;
        }
        
        current_distance += step_size;
    }
    
    return result;
}

VolumetricResult CalculateMultiLightScattering(
    const Vector3& ray_origin,
    const Vector3& ray_direction,
    const Vector3* lights,
    const Vector3* light_colors,
    int light_count,
    const VolumetricMedium& medium,
    const VolumeSamplingParams& sampling) {
    
    VolumetricResult result;
    result.scattered_light = Vector3(0.0f, 0.0f, 0.0f);
    result.transmittance = 1.0f;
    
    // Process each light
    for (int i = 0; i < light_count; ++i) {
        VolumetricResult light_result = CalculateVolumetricScattering(
            ray_origin, ray_direction, lights[i], light_colors[i], medium, sampling);
            
        // Accumulate results
        result.scattered_light += light_result.scattered_light;
        result.transmittance *= light_result.transmittance;
    }
    
    return result;
}

} // namespace lighting
} // namespace math
} // namespace pynovage