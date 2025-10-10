#include "renderer/voxel/camera.hpp"
#include <vectors/vector4.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

Camera::Camera() {
    UpdateVectors();
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

Camera::Camera(const Vector3f& position, const Vector3f& target, const Vector3f& up) 
    : position_(position), world_up_(up) {
    // Calculate yaw and pitch from target
    Vector3f direction = (target - position).normalized();
    
    // Calculate yaw (Y rotation)
    yaw_ = static_cast<float>(std::atan2(direction.x, -direction.z) * 180.0 / M_PI);
    
    // Calculate pitch (X rotation)  
    pitch_ = static_cast<float>(std::asin(direction.y) * 180.0 / M_PI);
    
    UpdateVectors();
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

void Camera::SetPosition(const Vector3f& position) {
    position_ = position;
    view_matrix_dirty_ = true;
}

void Camera::SetTarget(const Vector3f& target) {
    Vector3f direction = (target - position_).normalized();
    
    // Calculate yaw and pitch from direction
    yaw_ = static_cast<float>(std::atan2(direction.x, -direction.z) * 180.0 / M_PI);
    pitch_ = static_cast<float>(std::asin(direction.y) * 180.0 / M_PI);
    
    // Clamp pitch
    pitch_ = std::max(min_pitch_, std::min(max_pitch_, pitch_));
    
    UpdateVectors();
    view_matrix_dirty_ = true;
}

void Camera::SetRotation(float yaw, float pitch) {
    yaw_ = yaw;
    pitch_ = std::max(min_pitch_, std::min(max_pitch_, pitch));
    
    UpdateVectors();
    view_matrix_dirty_ = true;
}

void Camera::MoveForward(float distance) {
    if (mode_ == Mode::Flying) {
        position_ += forward_ * distance;
    } else {
        // Walking mode - move on XZ plane only
        Vector3f horizontal_forward = Vector3f(forward_.x, 0.0f, forward_.z).normalized();
        position_ += horizontal_forward * distance;
    }
    view_matrix_dirty_ = true;
}

void Camera::MoveRight(float distance) {
    position_ += right_ * distance;
    view_matrix_dirty_ = true;
}

void Camera::MoveUp(float distance) {
    if (mode_ == Mode::Flying) {
        position_ += up_ * distance;
    } else {
        // Walking mode - only move in world Y direction
        position_ += world_up_ * distance;
    }
    view_matrix_dirty_ = true;
}

void Camera::Rotate(float delta_yaw, float delta_pitch) {
    yaw_ += delta_yaw * mouse_sensitivity_;
    pitch_ += delta_pitch * mouse_sensitivity_;
    
    // Clamp pitch to avoid flipping
    pitch_ = std::max(min_pitch_, std::min(max_pitch_, pitch_));
    
    UpdateVectors();
    view_matrix_dirty_ = true;
}

void Camera::SetPerspective(float fov, float aspect_ratio, float near_plane, float far_plane) {
    projection_type_ = Projection::Perspective;
    fov_ = fov;
    aspect_ratio_ = aspect_ratio;
    near_plane_ = near_plane;
    far_plane_ = far_plane;
    projection_matrix_dirty_ = true;
}

void Camera::SetOrthographic(float left, float right, float bottom, float top, 
                           float near_plane, float far_plane) {
    projection_type_ = Projection::Orthographic;
    ortho_left_ = left;
    ortho_right_ = right;
    ortho_bottom_ = bottom;
    ortho_top_ = top;
    near_plane_ = near_plane;
    far_plane_ = far_plane;
    projection_matrix_dirty_ = true;
}

void Camera::SetAspectRatio(float aspect_ratio) {
    aspect_ratio_ = aspect_ratio;
    if (projection_type_ == Projection::Perspective) {
        projection_matrix_dirty_ = true;
    }
}

const Matrix4f& Camera::GetViewMatrix() const {
    if (view_matrix_dirty_) {
        UpdateViewMatrix();
        view_matrix_dirty_ = false;
    }
    return view_matrix_;
}

const Matrix4f& Camera::GetProjectionMatrix() const {
    if (projection_matrix_dirty_) {
        UpdateProjectionMatrix();
        projection_matrix_dirty_ = false;
    }
    return projection_matrix_;
}

Matrix4f Camera::GetViewProjectionMatrix() const {
    return GetProjectionMatrix() * GetViewMatrix();
}

Camera::Frustum Camera::ExtractFrustum() {
    Frustum frustum;
    Matrix4f mvp = GetViewProjectionMatrix();
    
    // Extract frustum planes from the view-projection matrix
    // Left plane
    const float* m = mvp.data.data();
    frustum.planes[0] = Vector4f(
        m[3] + m[0],
        m[7] + m[4],
        m[11] + m[8],
        m[15] + m[12]
    ).normalized();
    
    // Right plane
    frustum.planes[1] = Vector4f(
        m[3] - m[0],
        m[7] - m[4],
        m[11] - m[8],
        m[15] - m[12]
    ).normalized();
    
    // Bottom plane
    frustum.planes[2] = Vector4f(
        m[3] + m[1],
        m[7] + m[5],
        m[11] + m[9],
        m[15] + m[13]
    ).normalized();
    
    // Top plane
    frustum.planes[3] = Vector4f(
        m[3] - m[1],
        m[7] - m[5],
        m[11] - m[9],
        m[15] - m[13]
    ).normalized();
    
    // Near plane
    frustum.planes[4] = Vector4f(
        m[3] + m[2],
        m[7] + m[6],
        m[11] + m[10],
        m[15] + m[14]
    ).normalized();
    
    // Far plane
    frustum.planes[5] = Vector4f(
        m[3] - m[2],
        m[7] - m[6],
        m[11] - m[10],
        m[15] - m[14]
    ).normalized();
    
    return frustum;
}

bool Camera::IsPointInFrustum(const Vector3f& point, const Frustum& frustum) const {
    for (int i = 0; i < 6; ++i) {
        const Vector4f& plane = frustum.planes[i];
        if (plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w < 0) {
            return false;
        }
    }
    return true;
}

bool Camera::IsAABBInFrustum(const Vector3f& min, const Vector3f& max, const Frustum& frustum) const {
    for (int i = 0; i < 6; ++i) {
        const Vector4f& plane = frustum.planes[i];
        
        // Find the positive vertex (farthest from plane)
        Vector3f positive_vertex;
        positive_vertex.x = (plane.x >= 0) ? max.x : min.x;
        positive_vertex.y = (plane.y >= 0) ? max.y : min.y;
        positive_vertex.z = (plane.z >= 0) ? max.z : min.z;
        
        // If positive vertex is outside the plane, AABB is outside frustum
        if (plane.x * positive_vertex.x + plane.y * positive_vertex.y + 
            plane.z * positive_vertex.z + plane.w < 0) {
            return false;
        }
    }
    return true;
}

void Camera::UpdateVectors() {
    // Convert degrees to radians
    float yaw_rad = static_cast<float>(yaw_ * M_PI / 180.0);
    float pitch_rad = static_cast<float>(pitch_ * M_PI / 180.0);
    
    // Calculate forward vector
    forward_.x = std::cos(yaw_rad) * std::cos(pitch_rad);
    forward_.y = std::sin(pitch_rad);
    forward_.z = std::sin(yaw_rad) * std::cos(pitch_rad);
    forward_ = forward_.normalized();
    
    // Calculate right vector (cross product of forward and world up)
    right_ = forward_.cross(world_up_).normalized();
    
    // Calculate up vector (cross product of right and forward)
    up_ = right_.cross(forward_).normalized();
}

void Camera::UpdateViewMatrix() const {
    // Create look-at matrix
    Vector3f center = position_ + forward_;
    
    // Calculate the view matrix using look-at
    Vector3f f = (center - position_).normalized();
    Vector3f s = f.cross(world_up_).normalized();
    Vector3f u = s.cross(f);
    
    view_matrix_ = Matrix4f(
        s.x,  s.y,  s.z, -s.dot(position_),
        u.x,  u.y,  u.z, -u.dot(position_),
       -f.x, -f.y, -f.z,  f.dot(position_),
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

void Camera::UpdateProjectionMatrix() const {
    if (projection_type_ == Projection::Perspective) {
        // Perspective projection matrix
        float tan_half_fov = static_cast<float>(std::tan(fov_ * M_PI / 360.0)); // fov/2 in radians
        float range = far_plane_ - near_plane_;
        
        projection_matrix_ = Matrix4f(
            1.0f / (aspect_ratio_ * tan_half_fov), 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / tan_half_fov, 0.0f, 0.0f,
            0.0f, 0.0f, -(far_plane_ + near_plane_) / range, -(2.0f * far_plane_ * near_plane_) / range,
            0.0f, 0.0f, -1.0f, 0.0f
        );
    } else {
        // Orthographic projection matrix
        float width = ortho_right_ - ortho_left_;
        float height = ortho_top_ - ortho_bottom_;
        float depth = far_plane_ - near_plane_;
        
        projection_matrix_ = Matrix4f(
            2.0f / width, 0.0f, 0.0f, -(ortho_right_ + ortho_left_) / width,
            0.0f, 2.0f / height, 0.0f, -(ortho_top_ + ortho_bottom_) / height,
            0.0f, 0.0f, -2.0f / depth, -(far_plane_ + near_plane_) / depth,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }
}

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE