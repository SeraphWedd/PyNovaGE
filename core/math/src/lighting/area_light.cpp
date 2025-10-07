#include "lighting/area_light.hpp"
#include <random>

namespace pynovage {
namespace math {
namespace lighting {

namespace {
    // Thread-local random number generator for sampling
    thread_local std::mt19937 rng(std::random_device{}());
    thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Constants for optimization
    constexpr float kMinVisibility = 0.001f;
    constexpr float kPi = 3.14159265359f;
    constexpr float kInvPi = 1.0f / kPi;
    
    // Helper function to calculate Fresnel term
    float CalculateFresnel(float cos_theta, float F0) {
        return F0 + (1.0f - F0) * std::pow(1.0f - cos_theta, 5.0f);
    }
    
    // Helper function to calculate GGX distribution
    float CalculateGGX(float NoH, float roughness) {
        float a = roughness * roughness;
        float a2 = a * a;
        float NoH2 = NoH * NoH;
        float denom = NoH2 * (a2 - 1.0f) + 1.0f;
        return a2 * kInvPi / (denom * denom);
    }
    
    // Helper function to calculate visibility term
    float CalculateVisibilityTerm(float NoL, float NoV, float roughness) {
        float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
        float one_minus_k = 1.0f - k;
        float vis_l = NoL * one_minus_k + k;
        float vis_v = NoV * one_minus_k + k;
        return 1.0f / (vis_l * vis_v);
    }
}

void GenerateRectLightSamples(
    const RectAreaLight& light,
    const AreaSamplingParams& sampling,
    Vector3* samples,
    int sample_count) {
    
    // Calculate right vector from up and normal
    Vector3 right = light.up.cross(light.normal).normalized();
    
    if (sampling.stratified_sampling) {
        // Stratified sampling
        int sqrt_samples = static_cast<int>(std::sqrt(sample_count));
        float step = 1.0f / sqrt_samples;
        
        for (int i = 0; i < sqrt_samples; ++i) {
            for (int j = 0; j < sqrt_samples; ++j) {
                float u = (i + dist(rng)) * step;
                float v = (j + dist(rng)) * step;
                
                // Transform to [-1,1] range
                u = u * 2.0f - 1.0f;
                v = v * 2.0f - 1.0f;
                
                // Apply sample spread
                u *= sampling.sample_spread;
                v *= sampling.sample_spread;
                
                // Calculate sample position
                Vector3 sample = light.position + 
                    right * (u * light.width * 0.5f) +
                    light.up * (v * light.height * 0.5f);
                
                samples[i * sqrt_samples + j] = sample;
            }
        }
    } else {
        // Random sampling
        for (int i = 0; i < sample_count; ++i) {
            float u = dist(rng) * 2.0f - 1.0f;
            float v = dist(rng) * 2.0f - 1.0f;
            
            // Apply sample spread
            u *= sampling.sample_spread;
            v *= sampling.sample_spread;
            
            // Calculate sample position
            samples[i] = light.position + 
                right * (u * light.width * 0.5f) +
                light.up * (v * light.height * 0.5f);
        }
    }
}

void GenerateDiskLightSamples(
    const DiskAreaLight& light,
    const AreaSamplingParams& sampling,
    Vector3* samples,
    int sample_count) {
    
    // Find orthogonal vectors for disk sampling
    Vector3 u, v;
    if (std::abs(light.normal.x) < 0.99f) {
        u = Vector3(1, 0, 0).cross(light.normal).normalized();
    } else {
        u = Vector3(0, 1, 0).cross(light.normal).normalized();
    }
    v = light.normal.cross(u);
    
    if (sampling.stratified_sampling) {
        // Stratified sampling using concentric disk mapping
        int sqrt_samples = static_cast<int>(std::sqrt(sample_count));
        float step = 1.0f / sqrt_samples;
        
        for (int i = 0; i < sqrt_samples; ++i) {
            for (int j = 0; j < sqrt_samples; ++j) {
                float x = (i + dist(rng)) * step * 2.0f - 1.0f;
                float y = (j + dist(rng)) * step * 2.0f - 1.0f;
                
                // Map square to disk using concentric mapping
                float phi, r;
                if (x == 0.0f && y == 0.0f) {
                    r = phi = 0.0f;
                } else if (std::abs(x) > std::abs(y)) {
                    r = x;
                    phi = (y/x) * kPi * 0.25f;
                } else {
                    r = y;
                    phi = kPi * 0.5f - (x/y) * kPi * 0.25f;
                }
                
                // Apply sample spread
                r *= sampling.sample_spread * light.radius;
                
                float cos_phi = std::cos(phi);
                float sin_phi = std::sin(phi);
                
                samples[i * sqrt_samples + j] = light.position + 
                    u * (r * cos_phi) + v * (r * sin_phi);
            }
        }
    } else {
        // Random sampling using polar coordinates
        for (int i = 0; i < sample_count; ++i) {
            float theta = dist(rng) * 2.0f * kPi;
            float r = std::sqrt(dist(rng)) * light.radius * sampling.sample_spread;
            
            samples[i] = light.position + 
                u * (r * std::cos(theta)) + 
                v * (r * std::sin(theta));
        }
    }
}

float CalculateAreaLightVisibility(
    const Vector3& surface_point,
    const Vector3& sample_point,
    const Vector3& light_normal) {
    
    Vector3 to_light = sample_point - surface_point;
    float distance = to_light.length();
    if (distance < 0.001f) return 0.0f;  // Too close to light
    
    Vector3 L = to_light / distance;
    float NoL = -light_normal.dot(L);  // Negative because light normal points outward
    
    return NoL > 0.0f ? 1.0f : 0.0f;  // Simple visibility check
}

float CalculateRectFormFactor(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const RectAreaLight& light) {
    
    Vector3 to_center = light.position - surface_point;
    float distance = to_center.length();
    if (distance < 0.001f) return 0.0f;
    
    // Calculate projected solid angle (form factor)
    float NoL = std::max(0.0f, -light.normal.dot(to_center) / distance);
    float area = light.width * light.height;
    
    return area * NoL / (kPi * distance * distance);
}

float CalculateDiskFormFactor(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const DiskAreaLight& light) {
    
    Vector3 to_center = light.position - surface_point;
    float distance = to_center.length();
    if (distance < 0.001f) return 0.0f;
    
    // Calculate projected solid angle (form factor)
    float NoL = std::max(0.0f, -light.normal.dot(to_center) / distance);
    float area = kPi * light.radius * light.radius;
    
    return area * NoL / (kPi * distance * distance);
}

AreaLightResult CalculateRectAreaLight(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const Vector3& view_direction,
    float material_roughness,
    const RectAreaLight& light,
    const AreaSamplingParams& sampling) {
    
    AreaLightResult result = {Vector3(0,0,0), Vector3(0,0,0), 0.0f};
    
    // Allocate sample points
    Vector3* samples = new Vector3[sampling.num_samples];
    GenerateRectLightSamples(light, sampling, samples, sampling.num_samples);
    
    float total_visibility = 0.0f;
    Vector3 total_diffuse(0,0,0);
    Vector3 total_specular(0,0,0);
    
    for (int i = 0; i < sampling.num_samples; ++i) {
        Vector3 to_light = samples[i] - surface_point;
        float distance = to_light.length();
        if (distance < 0.001f) continue;
        
        Vector3 L = to_light / distance;
        float visibility = CalculateAreaLightVisibility(surface_point, samples[i], light.normal);
        if (visibility < kMinVisibility) continue;
        
        // Calculate lighting vectors
        Vector3 H = (L + view_direction).normalized();
        float NoL = std::max(0.0f, surface_normal.dot(L));
        float NoV = std::max(0.0f, surface_normal.dot(view_direction));
        float NoH = std::max(0.0f, surface_normal.dot(H));
        float LoH = std::max(0.0f, L.dot(H));
        
        // Calculate lighting terms
        float D = CalculateGGX(NoH, material_roughness);
        float G = CalculateVisibilityTerm(NoL, NoV, material_roughness);
        float F = CalculateFresnel(LoH, 0.04f);
        
        // Calculate contribution
        float inv_dist_sq = 1.0f / (distance * distance);
        Vector3 radiance = light.color * light.intensity * inv_dist_sq;
        
        // Accumulate lighting
        float spec = D * G * F / (4.0f * NoV + 0.001f);
        total_specular += radiance * spec * NoL * visibility;
        total_diffuse += radiance * NoL * visibility * kInvPi;
        total_visibility += visibility;
    }
    
    // Average results
    float inv_samples = 1.0f / sampling.num_samples;
    result.diffuse = total_diffuse * inv_samples;
    result.specular = total_specular * inv_samples;
    result.visibility = total_visibility * inv_samples;
    
    delete[] samples;
    return result;
}

AreaLightResult CalculateDiskAreaLight(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const Vector3& view_direction,
    float material_roughness,
    const DiskAreaLight& light,
    const AreaSamplingParams& sampling) {
    
    AreaLightResult result = {Vector3(0,0,0), Vector3(0,0,0), 0.0f};
    
    // Allocate sample points
    Vector3* samples = new Vector3[sampling.num_samples];
    GenerateDiskLightSamples(light, sampling, samples, sampling.num_samples);
    
    float total_visibility = 0.0f;
    Vector3 total_diffuse(0,0,0);
    Vector3 total_specular(0,0,0);
    
    for (int i = 0; i < sampling.num_samples; ++i) {
        Vector3 to_light = samples[i] - surface_point;
        float distance = to_light.length();
        if (distance < 0.001f) continue;
        
        Vector3 L = to_light / distance;
        float visibility = CalculateAreaLightVisibility(surface_point, samples[i], light.normal);
        if (visibility < kMinVisibility) continue;
        
        // Calculate lighting vectors
        Vector3 H = (L + view_direction).normalized();
        float NoL = std::max(0.0f, surface_normal.dot(L));
        float NoV = std::max(0.0f, surface_normal.dot(view_direction));
        float NoH = std::max(0.0f, surface_normal.dot(H));
        float LoH = std::max(0.0f, L.dot(H));
        
        // Calculate lighting terms
        float D = CalculateGGX(NoH, material_roughness);
        float G = CalculateVisibilityTerm(NoL, NoV, material_roughness);
        float F = CalculateFresnel(LoH, 0.04f);
        
        // Calculate contribution
        float inv_dist_sq = 1.0f / (distance * distance);
        Vector3 radiance = light.color * light.intensity * inv_dist_sq;
        
        // Accumulate lighting
        float spec = D * G * F / (4.0f * NoV + 0.001f);
        total_specular += radiance * spec * NoL * visibility;
        total_diffuse += radiance * NoL * visibility * kInvPi;
        total_visibility += visibility;
    }
    
    // Average results
    float inv_samples = 1.0f / sampling.num_samples;
    result.diffuse = total_diffuse * inv_samples;
    result.specular = total_specular * inv_samples;
    result.visibility = total_visibility * inv_samples;
    
    delete[] samples;
    return result;
}

AreaLightResult CalculateCustomAreaLight(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const Vector3& view_direction,
    float material_roughness,
    const CustomAreaLight& light,
    const AreaSamplingParams& sampling) {
    
    AreaLightResult result = {Vector3(0,0,0), Vector3(0,0,0), 0.0f};
    
    if (light.vertex_count < 3 || !light.vertices || !light.normals) {
        return result;  // Invalid light geometry
    }
    
    float total_visibility = 0.0f;
    Vector3 total_diffuse(0,0,0);
    Vector3 total_specular(0,0,0);
    
    // Sample points on the mesh using barycentric coordinates
    for (int i = 0; i < sampling.num_samples; ++i) {
        // Select random triangle
        int tri_idx = static_cast<int>(dist(rng) * (light.vertex_count - 2));
        
        // Generate barycentric coordinates
        float u = dist(rng);
        float v = dist(rng) * (1.0f - u);
        float w = 1.0f - u - v;
        
        // Calculate sample position and normal
        Vector3 sample_pos = light.vertices[0] * u + 
                           light.vertices[tri_idx + 1] * v + 
                           light.vertices[tri_idx + 2] * w;
        
        Vector3 sample_normal = light.normals[0] * u + 
                              light.normals[tri_idx + 1] * v + 
                              light.normals[tri_idx + 2] * w;
        sample_normal.normalize();
        
        Vector3 to_light = sample_pos - surface_point;
        float distance = to_light.length();
        if (distance < 0.001f) continue;
        
        Vector3 L = to_light / distance;
        float visibility = CalculateAreaLightVisibility(surface_point, sample_pos, sample_normal);
        if (visibility < kMinVisibility) continue;
        
        // Calculate lighting vectors
        Vector3 H = (L + view_direction).normalized();
        float NoL = std::max(0.0f, surface_normal.dot(L));
        float NoV = std::max(0.0f, surface_normal.dot(view_direction));
        float NoH = std::max(0.0f, surface_normal.dot(H));
        float LoH = std::max(0.0f, L.dot(H));
        
        // Calculate lighting terms
        float D = CalculateGGX(NoH, material_roughness);
        float G = CalculateVisibilityTerm(NoL, NoV, material_roughness);
        float F = CalculateFresnel(LoH, 0.04f);
        
        // Calculate contribution
        float inv_dist_sq = 1.0f / (distance * distance);
        Vector3 radiance = light.color * light.intensity * inv_dist_sq;
        
        // Accumulate lighting
        float spec = D * G * F / (4.0f * NoV + 0.001f);
        total_specular += radiance * spec * NoL * visibility;
        total_diffuse += radiance * NoL * visibility * kInvPi;
        total_visibility += visibility;
    }
    
    // Average results
    float inv_samples = 1.0f / sampling.num_samples;
    result.diffuse = total_diffuse * inv_samples;
    result.specular = total_specular * inv_samples;
    result.visibility = total_visibility * inv_samples;
    
    return result;
}

} // namespace lighting
} // namespace math
} // namespace pynovage