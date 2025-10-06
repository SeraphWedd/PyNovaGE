#include "../../include/lighting/light_transforms.hpp"
#include "../../include/math_constants.hpp"
#include "../../include/lighting/directional_light.hpp"
#include "../../include/lighting/point_light.hpp"
#include "../../include/lighting/spot_light.hpp"

namespace pynovage {
namespace math {

using math::Matrix4;
using math::Vector3;

namespace lighting {

Matrix4 LightSpaceTransform::createDirectionalLightView(
    const DirectionalLight& light,
    const Vector3& center,
    float radius)
{
    // Use light's direction as forward vector (negative Z axis)
    Vector3 forward = -light.direction.normalized();
    
    // Find a suitable up vector (avoid parallel with forward)
    Vector3 right = Vector3(0, 1, 0).cross(forward);
    if (right.lengthSquared() < 1e-6f) {
        right = Vector3(1, 0, 0);
    }
    right.normalize();
    
    Vector3 up = forward.cross(right);
    
    // Create view matrix looking from center along light direction
    return Matrix4(
        right.x,   right.y,   right.z,   -center.dot(right),
        up.x,      up.y,      up.z,      -center.dot(up),
        forward.x, forward.y, forward.z, -center.dot(forward),
        0.0f,      0.0f,      0.0f,      1.0f
    );
}

Matrix4 LightSpaceTransform::createPointLightView(
    const PointLight& light,
    int face)
{
    return calculateCubemapFaceView(light.position, face);
}

Matrix4 LightSpaceTransform::createSpotLightView(
    const SpotLight& light)
{
    // Use spot light direction and position
    Vector3 forward = light.direction.normalized();
    Vector3 right = Vector3(0, 1, 0).cross(forward);
    if (right.lengthSquared() < 1e-6f) {
        right = Vector3(1, 0, 0);
    }
    right.normalize();
    Vector3 up = forward.cross(right);
    
    // Create view matrix looking from light position along direction
    return Matrix4(
        right.x,   right.y,   right.z,   -light.position.dot(right),
        up.x,      up.y,      up.z,      -light.position.dot(up),
        forward.x, forward.y, forward.z, -light.position.dot(forward),
        0.0f,      0.0f,      0.0f,      1.0f
    );
}

Matrix4 LightSpaceTransform::createDirectionalLightProjection(
    const DirectionalLight& light,
    const Vector3& center,
    float radius,
    float nearPlane,
    float farPlane)
{
    Vector3 min, max;
    calculateDirectionalBounds(light.direction, center, radius, min, max);
    
    // Create orthographic projection with reversed Z
    return Matrix4::orthographicZeroOne(
        min.x, max.x, min.y, max.y, nearPlane, farPlane
    );
}

Matrix4 LightSpaceTransform::createPointLightProjection(
    const PointLight& light,
    float nearPlane)
{
    // Use 90 degree FOV for cube faces (square aspect ratio)
    return Matrix4::perspectiveReversedZ(
        constants::half_pi,  // 90 degrees
        1.0f,               // 1:1 aspect ratio
        nearPlane,
        light.attenuation.range
    );
}

Matrix4 LightSpaceTransform::createSpotLightProjection(
    const SpotLight& light,
    float nearPlane)
{
    float fovY, aspect;
    calculateSpotLightFrustum(light, fovY, aspect);
    
    return Matrix4::perspectiveReversedZ(
        fovY,
        aspect,
        nearPlane,
        light.attenuation.range
    );
}

Matrix4 LightSpaceTransform::createLightSpaceTransform(
    const DirectionalLight& light,
    const Vector3& center,
    float radius,
    float nearPlane,
    float farPlane)
{
    Matrix4 view = createDirectionalLightView(light, center, radius);
    Matrix4 proj = createDirectionalLightProjection(light, center, radius, nearPlane, farPlane);
    return proj * view;
}

Matrix4 LightSpaceTransform::createLightSpaceTransform(
    const PointLight& light,
    int face,
    float nearPlane)
{
    Matrix4 view = createPointLightView(light, face);
    Matrix4 proj = createPointLightProjection(light, nearPlane);
    return proj * view;
}

Matrix4 LightSpaceTransform::createLightSpaceTransform(
    const SpotLight& light,
    float nearPlane)
{
    Matrix4 view = createSpotLightView(light);
    Matrix4 proj = createSpotLightProjection(light, nearPlane);
    return proj * view;
}

void LightSpaceTransform::calculateDirectionalBounds(
    const Vector3& direction,
    const Vector3& center,
    float radius,
    Vector3& min,
    Vector3& max)
{
    // Transform to light space
    Vector3 forward = direction.normalized();
    Vector3 right = Vector3(0, 1, 0).cross(forward);
    if (right.lengthSquared() < 1e-6f) {
        right = Vector3(1, 0, 0);
    }
    right.normalize();
    Vector3 up = forward.cross(right);
    
    // Calculate AABB in light space by transforming sphere bounds
    min = center;
    max = center;
    
    // Expand by radius in all directions
    min -= right * radius;
    min -= up * radius;
    min -= forward * radius;
    
    max += right * radius;
    max += up * radius;
    max += forward * radius;
}

Matrix4 LightSpaceTransform::calculateCubemapFaceView(
    const Vector3& position,
    int face)
{
    // Define view directions and up vectors for each cube face
    Vector3 forward, up;
    switch (face) {
        case 0: // +X
            forward = Vector3(1, 0, 0);
            up = Vector3(0, -1, 0);
            break;
        case 1: // -X
            forward = Vector3(-1, 0, 0);
            up = Vector3(0, -1, 0);
            break;
        case 2: // +Y
            forward = Vector3(0, 1, 0);
            up = Vector3(0, 0, 1);
            break;
        case 3: // -Y
            forward = Vector3(0, -1, 0);
            up = Vector3(0, 0, -1);
            break;
        case 4: // +Z
            forward = Vector3(0, 0, 1);
            up = Vector3(0, -1, 0);
            break;
        default: // -Z
            forward = Vector3(0, 0, -1);
            up = Vector3(0, -1, 0);
            break;
    }
    
    // Calculate right vector from forward and up
    Vector3 right = up.cross(forward);
    
    // Create view matrix for cube face
    return Matrix4(
        right.x,   right.y,   right.z,   -position.dot(right),
        up.x,      up.y,      up.z,      -position.dot(up),
        forward.x, forward.y, forward.z, -position.dot(forward),
        0.0f,      0.0f,      0.0f,      1.0f
    );
}

void LightSpaceTransform::calculateSpotLightFrustum(
    const SpotLight& light,
    float& fovY,
    float& aspect)
{
    // Calculate vertical FOV from outer angle
    fovY = 2.0f * light.outerAngle;
    
    // For now, use a square aspect ratio (can be adjusted based on shadow map resolution)
    aspect = 1.0f;
}


Matrix4 LightSpaceTransform::createDepthBiasMatrix(
    float depthBias,
    float slopeScale)
{
    // Create a matrix that shifts depth values by a constant bias
    // and adds slope-scaled bias based on surface orientation
    return Matrix4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, depthBias,
        0.0f, 0.0f, slopeScale, 1.0f
    );
}

template<>
Matrix4 LightSpaceTransform::createNormalBiasMatrix<DirectionalLight>(
    const DirectionalLight& light,
    float normalBias)
{
    Vector3 biasDir = light.direction.normalized() * normalBias;
    return Matrix4(
        1.0f, 0.0f, 0.0f, biasDir.x,
        0.0f, 1.0f, 0.0f, biasDir.y,
        0.0f, 0.0f, 1.0f, biasDir.z,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

template<>
Matrix4 LightSpaceTransform::createNormalBiasMatrix<PointLight>(
    const PointLight& light,
    float normalBias)
{
    // For point lights, bias is applied radially outward
    return Matrix4::scale(
        1.0f + normalBias,
        1.0f + normalBias,
        1.0f + normalBias
    );
}

template<>
Matrix4 LightSpaceTransform::createNormalBiasMatrix<SpotLight>(
    const SpotLight& light,
    float normalBias)
{
    Vector3 biasDir = light.direction.normalized() * normalBias;
    return Matrix4(
        1.0f, 0.0f, 0.0f, biasDir.x,
        0.0f, 1.0f, 0.0f, biasDir.y,
        0.0f, 0.0f, 1.0f, biasDir.z,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

} // namespace lighting
} // namespace math
} // namespace pynovage
