#include "lighting/area_light.hpp"
#include <gtest/gtest.h>

using namespace pynovage::math;
using namespace pynovage::math::lighting;

TEST(AreaLightTests, RectFormFactor) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    
    RectAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.up = Vector3(0, 0, 1);
    light.width = 2.0f;
    light.height = 2.0f;
    
    float form_factor = CalculateRectFormFactor(surface_point, surface_normal, light);
    EXPECT_GT(form_factor, 0.0f);
    
    // Test with varying distances
    light.position = Vector3(0, 10, 0);
    float far_form_factor = CalculateRectFormFactor(surface_point, surface_normal, light);
    EXPECT_LT(far_form_factor, form_factor);
}

TEST(AreaLightTests, DiskFormFactor) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    
    DiskAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.radius = 1.0f;
    
    float form_factor = CalculateDiskFormFactor(surface_point, surface_normal, light);
    EXPECT_GT(form_factor, 0.0f);
    
    // Test with varying distances
    light.position = Vector3(0, 10, 0);
    float far_form_factor = CalculateDiskFormFactor(surface_point, surface_normal, light);
    EXPECT_LT(far_form_factor, form_factor);
}

TEST(AreaLightTests, RectLightSampling) {
    RectAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.up = Vector3(0, 0, 1);
    light.width = 2.0f;
    light.height = 2.0f;
    
    AreaSamplingParams sampling;
    sampling.num_samples = 16;
    
    Vector3* samples = new Vector3[sampling.num_samples];
    
    // Test stratified sampling
    sampling.stratified_sampling = true;
    GenerateRectLightSamples(light, sampling, samples, sampling.num_samples);
    
    // Check that samples are within light bounds
    for (int i = 0; i < sampling.num_samples; ++i) {
        Vector3 to_sample = samples[i] - light.position;
        EXPECT_LE(std::abs(to_sample.dot(light.up)), light.height * 0.5f);
        Vector3 right = light.up.cross(light.normal).normalized();
        EXPECT_LE(std::abs(to_sample.dot(right)), light.width * 0.5f);
    }
    
    // Test random sampling
    sampling.stratified_sampling = false;
    GenerateRectLightSamples(light, sampling, samples, sampling.num_samples);
    
    // Check that samples are within light bounds
    for (int i = 0; i < sampling.num_samples; ++i) {
        Vector3 to_sample = samples[i] - light.position;
        EXPECT_LE(std::abs(to_sample.dot(light.up)), light.height * 0.5f);
        Vector3 right = light.up.cross(light.normal).normalized();
        EXPECT_LE(std::abs(to_sample.dot(right)), light.width * 0.5f);
    }
    
    delete[] samples;
}

TEST(AreaLightTests, DiskLightSampling) {
    DiskAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.radius = 1.0f;
    
    AreaSamplingParams sampling;
    sampling.num_samples = 16;
    
    Vector3* samples = new Vector3[sampling.num_samples];
    
    // Test stratified sampling
    sampling.stratified_sampling = true;
    GenerateDiskLightSamples(light, sampling, samples, sampling.num_samples);
    
    // Check that samples are within light bounds
    for (int i = 0; i < sampling.num_samples; ++i) {
        Vector3 to_sample = samples[i] - light.position;
        EXPECT_LE(to_sample.length(), light.radius);
    }
    
    // Test random sampling
    sampling.stratified_sampling = false;
    GenerateDiskLightSamples(light, sampling, samples, sampling.num_samples);
    
    // Check that samples are within light bounds
    for (int i = 0; i < sampling.num_samples; ++i) {
        Vector3 to_sample = samples[i] - light.position;
        EXPECT_LE(to_sample.length(), light.radius);
    }
    
    delete[] samples;
}

TEST(AreaLightTests, RectAreaLighting) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    Vector3 view_direction(0, 1, 0);
    float material_roughness = 0.5f;
    
    RectAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.up = Vector3(0, 0, 1);
    light.width = 2.0f;
    light.height = 2.0f;
    light.color = Vector3(1, 1, 1);
    light.intensity = 1.0f;
    
    AreaSamplingParams sampling;
    
    // Test basic lighting calculation
    auto result = CalculateRectAreaLight(surface_point, surface_normal,
        view_direction, material_roughness, light, sampling);
    
    EXPECT_GE(result.diffuse.x, 0.0f);
    EXPECT_GE(result.specular.x, 0.0f);
    EXPECT_GE(result.visibility, 0.0f);
    EXPECT_LE(result.visibility, 1.0f);
    
    // Test with varying roughness
    float rough_result = CalculateRectAreaLight(surface_point, surface_normal,
        view_direction, 1.0f, light, sampling).specular.x;
    float smooth_result = CalculateRectAreaLight(surface_point, surface_normal,
        view_direction, 0.1f, light, sampling).specular.x;
    
    EXPECT_GT(smooth_result, rough_result);
}

TEST(AreaLightTests, DiskAreaLighting) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    Vector3 view_direction(0, 1, 0);
    float material_roughness = 0.5f;
    
    DiskAreaLight light;
    light.position = Vector3(0, 5, 0);
    light.normal = Vector3(0, -1, 0);
    light.radius = 1.0f;
    light.color = Vector3(1, 1, 1);
    light.intensity = 1.0f;
    
    AreaSamplingParams sampling;
    
    // Test basic lighting calculation
    auto result = CalculateDiskAreaLight(surface_point, surface_normal,
        view_direction, material_roughness, light, sampling);
    
    EXPECT_GE(result.diffuse.x, 0.0f);
    EXPECT_GE(result.specular.x, 0.0f);
    EXPECT_GE(result.visibility, 0.0f);
    EXPECT_LE(result.visibility, 1.0f);
    
    // Test with varying roughness
    float rough_result = CalculateDiskAreaLight(surface_point, surface_normal,
        view_direction, 1.0f, light, sampling).specular.x;
    float smooth_result = CalculateDiskAreaLight(surface_point, surface_normal,
        view_direction, 0.1f, light, sampling).specular.x;
    
    EXPECT_GT(smooth_result, rough_result);
}

TEST(AreaLightTests, CustomAreaLighting) {
    Vector3 surface_point(0, 0, 0);
    Vector3 surface_normal(0, 1, 0);
    Vector3 view_direction(0, 1, 0);
    float material_roughness = 0.5f;
    
    // Create a simple triangular light
    Vector3 vertices[] = {
        Vector3(-1, 5, -1),
        Vector3(1, 5, -1),
        Vector3(0, 5, 1)
    };
    Vector3 normals[] = {
        Vector3(0, -1, 0),
        Vector3(0, -1, 0),
        Vector3(0, -1, 0)
    };
    
    CustomAreaLight light;
    light.vertices = vertices;
    light.normals = normals;
    light.vertex_count = 3;
    light.position = Vector3(0, 5, 0);
    light.color = Vector3(1, 1, 1);
    light.intensity = 1.0f;
    
    AreaSamplingParams sampling;
    
    // Test basic lighting calculation
    auto result = CalculateCustomAreaLight(surface_point, surface_normal,
        view_direction, material_roughness, light, sampling);
    
    EXPECT_GE(result.diffuse.x, 0.0f);
    EXPECT_GE(result.specular.x, 0.0f);
    EXPECT_GE(result.visibility, 0.0f);
    EXPECT_LE(result.visibility, 1.0f);
    
    // Test with invalid geometry
    light.vertex_count = 0;
    result = CalculateCustomAreaLight(surface_point, surface_normal,
        view_direction, material_roughness, light, sampling);
    
    EXPECT_EQ(result.diffuse.x, 0.0f);
    EXPECT_EQ(result.specular.x, 0.0f);
    EXPECT_EQ(result.visibility, 0.0f);
}

TEST(AreaLightTests, Visibility) {
    Vector3 surface_point(0, 0, 0);
    Vector3 sample_point(0, 5, 0);
    Vector3 light_normal(0, -1, 0);
    
    // Test basic visibility
    float visibility = CalculateAreaLightVisibility(surface_point, sample_point, light_normal);
    EXPECT_GE(visibility, 0.0f);
    EXPECT_LE(visibility, 1.0f);
    
    // Test when light faces away
    light_normal = Vector3(0, 1, 0);
    visibility = CalculateAreaLightVisibility(surface_point, sample_point, light_normal);
    EXPECT_EQ(visibility, 0.0f);
    
    // Test when sample is too close
    sample_point = Vector3(0, 0.0001f, 0);
    visibility = CalculateAreaLightVisibility(surface_point, sample_point, light_normal);
    EXPECT_EQ(visibility, 0.0f);
}