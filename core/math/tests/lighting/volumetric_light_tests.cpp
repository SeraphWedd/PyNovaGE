#include "lighting/volumetric_light.hpp"
#include <gtest/gtest.h>
#define _USE_MATH_DEFINES
#include <cmath>

using namespace pynovage::math;
using namespace pynovage::math::lighting;

TEST(VolumetricLightTests, PhaseFunction) {
    // Test isotropic scattering (g = 0)
    float cos_angle = 0.5f;
    const float kInvFourPi = 0.07957747154f;  // 1/(4π)
    float phase = CalculatePhaseFunction(cos_angle, 0.0f);
    EXPECT_NEAR(phase, kInvFourPi, 1e-6);

    // Test forward scattering (g > 0)
    phase = CalculatePhaseFunction(1.0f, 0.8f);
    EXPECT_GT(phase, kInvFourPi);

    // Test backward scattering (g < 0)
    phase = CalculatePhaseFunction(-1.0f, -0.8f);
    EXPECT_GT(phase, kInvFourPi);
}

TEST(VolumetricLightTests, AdaptiveStepSize) {
    Vector3 current_pos(0, 0, 0);
    Vector3 light_pos(10, 0, 0);
    VolumetricMedium medium;
    
    // Test base case
    float step = CalculateAdaptiveStepSize(current_pos, light_pos, medium, 1.0f);
    EXPECT_GT(step, 0.0f);
    
    // Test with varying densities
    medium.density = 0.1f;
    float step_low_density = CalculateAdaptiveStepSize(current_pos, light_pos, medium, 1.0f);
    
    medium.density = 10.0f;
    float step_high_density = CalculateAdaptiveStepSize(current_pos, light_pos, medium, 1.0f);
    
    EXPECT_GT(step_low_density, step_high_density);
}

TEST(VolumetricLightTests, SingleScattering) {
    Vector3 ray_origin(0, 0, 0);
    Vector3 ray_direction(1, 0, 0);
    Vector3 light_pos(0, 5, 0);
    Vector3 light_color(1, 1, 1);
    VolumetricMedium medium;
    VolumeSamplingParams sampling;
    
    // Test base case
    auto result = CalculateVolumetricScattering(ray_origin, ray_direction, 
        light_pos, light_color, medium, sampling);
    
    EXPECT_GE(result.transmittance, 0.0f);
    EXPECT_LE(result.transmittance, 1.0f);
    EXPECT_GE(result.scattered_light.x, 0.0f);
    EXPECT_GE(result.scattered_light.y, 0.0f);
    EXPECT_GE(result.scattered_light.z, 0.0f);
    
    // Test with varying medium properties
    medium.scattering_coefficient = 0.5f;
    medium.absorption_coefficient = 0.1f;
    auto result2 = CalculateVolumetricScattering(ray_origin, ray_direction, 
        light_pos, light_color, medium, sampling);
    
    EXPECT_LT(result2.transmittance, result.transmittance);
}

TEST(VolumetricLightTests, MultiLightScattering) {
    Vector3 ray_origin(0, 0, 0);
    Vector3 ray_direction(1, 0, 0);
    Vector3 lights[] = {
        Vector3(0, 5, 0),
        Vector3(0, -5, 0)
    };
    Vector3 colors[] = {
        Vector3(1, 0, 0),
        Vector3(0, 1, 0)
    };
    VolumetricMedium medium;
    VolumeSamplingParams sampling;
    
    auto result = CalculateMultiLightScattering(ray_origin, ray_direction,
        lights, colors, 2, medium, sampling);
    
    EXPECT_GE(result.transmittance, 0.0f);
    EXPECT_LE(result.transmittance, 1.0f);
    
    // Both red and green components should be present
    EXPECT_GT(result.scattered_light.x, 0.0f);
    EXPECT_GT(result.scattered_light.y, 0.0f);
    EXPECT_EQ(result.scattered_light.z, 0.0f);
}

TEST(VolumetricLightTests, VolumetricShadow) {
    Vector3 sample_pos(0, 0, 0);
    Vector3 light_pos(10, 0, 0);
    VolumetricMedium medium;
    VolumeSamplingParams sampling;
    
    // Test with varying medium properties
    float shadow1 = CalculateVolumetricShadow(sample_pos, light_pos, medium, sampling);
    EXPECT_GE(shadow1, 0.0f);
    EXPECT_LE(shadow1, 1.0f);
    
    medium.absorption_coefficient = 1.0f;
    float shadow2 = CalculateVolumetricShadow(sample_pos, light_pos, medium, sampling);
    EXPECT_LT(shadow2, shadow1);
}

TEST(VolumetricLightTests, AdaptiveSampling) {
    Vector3 ray_origin(0, 0, 0);
    Vector3 ray_direction(1, 0, 0);
    Vector3 light_pos(0, 5, 0);
    Vector3 light_color(1, 1, 1);
    VolumetricMedium medium;
    VolumeSamplingParams sampling;
    
    // Test with and without adaptive sampling
    sampling.use_adaptive_sampling = true;
    auto result1 = CalculateVolumetricScattering(ray_origin, ray_direction,
        light_pos, light_color, medium, sampling);
    
    sampling.use_adaptive_sampling = false;
    auto result2 = CalculateVolumetricScattering(ray_origin, ray_direction,
        light_pos, light_color, medium, sampling);
    
    // Results should be similar but not identical
    EXPECT_NEAR(result1.transmittance, result2.transmittance, 0.1f);
}

TEST(VolumetricLightTests, EdgeCases) {
    Vector3 ray_origin(0, 0, 0);
    Vector3 ray_direction(1, 0, 0);
    Vector3 light_pos(0, 0, 0);  // Light at same position as ray origin
    Vector3 light_color(1, 1, 1);
    VolumetricMedium medium;
    VolumeSamplingParams sampling;
    
    // Test with light at ray origin
    auto result = CalculateVolumetricScattering(ray_origin, ray_direction,
        light_pos, light_color, medium, sampling);
    EXPECT_FALSE(std::isnan(result.transmittance));
    EXPECT_FALSE(std::isnan(result.scattered_light.x));
    
    // Test with zero coefficients
    medium.scattering_coefficient = 0.0f;
    medium.absorption_coefficient = 0.0f;
    result = CalculateVolumetricScattering(ray_origin, ray_direction,
        light_pos, light_color, medium, sampling);
    EXPECT_NEAR(result.transmittance, 1.0f, 1e-6);
    
    // Test with very high coefficients
    medium.scattering_coefficient = 1000.0f;
    medium.absorption_coefficient = 1000.0f;
    result = CalculateVolumetricScattering(ray_origin, ray_direction,
        light_pos, light_color, medium, sampling);
    EXPECT_NEAR(result.transmittance, 0.0f, 1e-6);
}