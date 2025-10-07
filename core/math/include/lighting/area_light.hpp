#ifndef PYNOVAGE_MATH_LIGHTING_AREA_LIGHT_HPP
#define PYNOVAGE_MATH_LIGHTING_AREA_LIGHT_HPP

#include "../vector3.hpp"
#include "../vector4.hpp"
#include "../matrix4.hpp"
#include "light_types.hpp"
#include "light_transforms.hpp"

namespace pynovage {
namespace math {
namespace lighting {

/**
 * @brief Defines the shape of an area light
 */
enum class AreaLightShape {
    Rectangle,  // Rectangular area light
    Disk,       // Circular area light
    Custom      // Custom shape defined by vertices
};

/**
 * @brief Parameters for area light sampling
 */
struct AreaSamplingParams {
    int num_samples = 16;           // Number of samples for area light integration
    float sample_spread = 0.5f;     // How spread out the samples are (0-1)
    bool stratified_sampling = true; // Whether to use stratified sampling
    bool importance_sampling = true; // Whether to use importance sampling
};

/**
 * @brief Parameters for a rectangular area light
 */
struct RectAreaLight {
    Vector3 position;     // Center position of the light
    Vector3 normal;       // Normal direction (facing direction)
    Vector3 up;          // Up vector for orientation
    float width;         // Width of the rectangle
    float height;        // Height of the rectangle
    Vector3 color;       // Light color and intensity
    float intensity;     // Light intensity multiplier
};

/**
 * @brief Parameters for a disk area light
 */
struct DiskAreaLight {
    Vector3 position;     // Center position of the light
    Vector3 normal;       // Normal direction (facing direction)
    float radius;        // Radius of the disk
    Vector3 color;       // Light color and intensity
    float intensity;     // Light intensity multiplier
};

/**
 * @brief Parameters for a custom shape area light
 */
struct CustomAreaLight {
    Vector3* vertices;    // Array of vertices defining the shape
    Vector3* normals;     // Array of normals for each vertex
    int vertex_count;     // Number of vertices
    Vector3 position;     // Center position of the light
    Vector3 color;       // Light color and intensity
    float intensity;     // Light intensity multiplier
};

/**
 * @brief Result of area light calculation
 */
struct AreaLightResult {
    Vector3 diffuse;     // Diffuse lighting contribution
    Vector3 specular;    // Specular lighting contribution
    float visibility;    // Overall visibility factor
};

/**
 * @brief Calculates lighting from a rectangular area light
 * 
 * @param surface_point Point being lit
 * @param surface_normal Surface normal at the point
 * @param view_direction Direction from point to viewer
 * @param material_roughness Surface roughness (0-1)
 * @param light Light parameters
 * @param sampling Sampling parameters
 * @return AreaLightResult containing diffuse and specular contributions
 */
AreaLightResult CalculateRectAreaLight(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const Vector3& view_direction,
    float material_roughness,
    const RectAreaLight& light,
    const AreaSamplingParams& sampling = AreaSamplingParams());

/**
 * @brief Calculates lighting from a disk area light
 * 
 * @param surface_point Point being lit
 * @param surface_normal Surface normal at the point
 * @param view_direction Direction from point to viewer
 * @param material_roughness Surface roughness (0-1)
 * @param light Light parameters
 * @param sampling Sampling parameters
 * @return AreaLightResult containing diffuse and specular contributions
 */
AreaLightResult CalculateDiskAreaLight(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const Vector3& view_direction,
    float material_roughness,
    const DiskAreaLight& light,
    const AreaSamplingParams& sampling = AreaSamplingParams());

/**
 * @brief Calculates lighting from a custom shape area light
 * 
 * @param surface_point Point being lit
 * @param surface_normal Surface normal at the point
 * @param view_direction Direction from point to viewer
 * @param material_roughness Surface roughness (0-1)
 * @param light Light parameters
 * @param sampling Sampling parameters
 * @return AreaLightResult containing diffuse and specular contributions
 */
AreaLightResult CalculateCustomAreaLight(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const Vector3& view_direction,
    float material_roughness,
    const CustomAreaLight& light,
    const AreaSamplingParams& sampling = AreaSamplingParams());

/**
 * @brief Calculates form factor between a point and a rectangular area light
 * 
 * @param surface_point Point to calculate form factor for
 * @param surface_normal Surface normal at the point
 * @param light Area light parameters
 * @return float Form factor value
 */
float CalculateRectFormFactor(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const RectAreaLight& light);

/**
 * @brief Calculates form factor between a point and a disk area light
 * 
 * @param surface_point Point to calculate form factor for
 * @param surface_normal Surface normal at the point
 * @param light Area light parameters
 * @return float Form factor value
 */
float CalculateDiskFormFactor(
    const Vector3& surface_point,
    const Vector3& surface_normal,
    const DiskAreaLight& light);

/**
 * @brief Generates sample points on a rectangular area light
 * 
 * @param light Light parameters
 * @param sampling Sampling parameters
 * @param samples Array to store generated sample points
 * @param sample_count Size of the samples array
 */
void GenerateRectLightSamples(
    const RectAreaLight& light,
    const AreaSamplingParams& sampling,
    Vector3* samples,
    int sample_count);

/**
 * @brief Generates sample points on a disk area light
 * 
 * @param light Light parameters
 * @param sampling Sampling parameters
 * @param samples Array to store generated sample points
 * @param sample_count Size of the samples array
 */
void GenerateDiskLightSamples(
    const DiskAreaLight& light,
    const AreaSamplingParams& sampling,
    Vector3* samples,
    int sample_count);

/**
 * @brief Calculates visibility between a point and an area light sample
 * 
 * @param surface_point Point to check visibility from
 * @param sample_point Point on the area light
 * @param light_normal Normal at the sample point
 * @return float Visibility factor (0 = occluded, 1 = fully visible)
 */
float CalculateAreaLightVisibility(
    const Vector3& surface_point,
    const Vector3& sample_point,
    const Vector3& light_normal);

} // namespace lighting
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_LIGHTING_AREA_LIGHT_HPP