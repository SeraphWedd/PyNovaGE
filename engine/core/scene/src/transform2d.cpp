#include "scene/transform2d.hpp"
#include <cmath>
#include <algorithm>

namespace PyNovaGE {
namespace Scene {

// Transform2D Implementation

Transform2D::Transform2D(const Vector2f& position, float rotation, const Vector2f& scale)
    : position_(position), rotation_(rotation), scale_(scale) {
    InvalidateLocalMatrix();
}

void Transform2D::SetPosition(const Vector2f& position) {
    position_ = position;
    InvalidateLocalMatrix();
}

void Transform2D::SetRotation(float rotation) {
    rotation_ = rotation;
    InvalidateLocalMatrix();
}

void Transform2D::SetScale(const Vector2f& scale) {
    scale_ = scale;
    InvalidateLocalMatrix();
}

void Transform2D::Translate(const Vector2f& translation) {
    position_ += translation;
    InvalidateLocalMatrix();
}

void Transform2D::Rotate(float angle) {
    rotation_ += angle;
    InvalidateLocalMatrix();
}

void Transform2D::Scale(const Vector2f& scale) {
    scale_.x *= scale.x;
    scale_.y *= scale.y;
    InvalidateLocalMatrix();
}

void Transform2D::Scale(float uniform_scale) {
    scale_.x *= uniform_scale;
    scale_.y *= uniform_scale;
    InvalidateLocalMatrix();
}

const Matrix3f& Transform2D::GetLocalToParentMatrix() const {
    if (local_matrix_dirty_) {
        UpdateLocalMatrix();
    }
    return local_to_parent_;
}

const Matrix3f& Transform2D::GetParentToLocalMatrix() const {
    if (parent_to_local_dirty_) {
        UpdateParentToLocalMatrix();
    }
    return parent_to_local_;
}

void Transform2D::SetWorldMatrix(const Matrix3f& world_matrix) {
    world_matrix_ = world_matrix;
    inverse_world_dirty_ = true;
}

const Matrix3f& Transform2D::GetInverseWorldMatrix() const {
    if (inverse_world_dirty_) {
        UpdateInverseWorldMatrix();
    }
    return inverse_world_matrix_;
}

Vector2f Transform2D::GetWorldPosition() const {
    return TransformUtils::ExtractTranslation(world_matrix_);
}

float Transform2D::GetWorldRotation() const {
    return TransformUtils::ExtractRotation(world_matrix_);
}

Vector2f Transform2D::GetWorldScale() const {
    return TransformUtils::ExtractScale(world_matrix_);
}

Vector2f Transform2D::TransformPoint(const Vector2f& local_point) const {
    Vector3f homogeneous_point(local_point.x, local_point.y, 1.0f);
    Vector3f world_point = world_matrix_ * homogeneous_point;
    return Vector2f(world_point.x, world_point.y);
}

Vector2f Transform2D::TransformDirection(const Vector2f& local_direction) const {
    Vector3f homogeneous_dir(local_direction.x, local_direction.y, 0.0f);
    Vector3f world_dir = world_matrix_ * homogeneous_dir;
    return Vector2f(world_dir.x, world_dir.y);
}

Vector2f Transform2D::InverseTransformPoint(const Vector2f& world_point) const {
    if (inverse_world_dirty_) {
        UpdateInverseWorldMatrix();
    }
    Vector3f homogeneous_point(world_point.x, world_point.y, 1.0f);
    Vector3f local_point = inverse_world_matrix_ * homogeneous_point;
    return Vector2f(local_point.x, local_point.y);
}

Vector2f Transform2D::InverseTransformDirection(const Vector2f& world_direction) const {
    if (inverse_world_dirty_) {
        UpdateInverseWorldMatrix();
    }
    Vector3f homogeneous_dir(world_direction.x, world_direction.y, 0.0f);
    Vector3f local_dir = inverse_world_matrix_ * homogeneous_dir;
    return Vector2f(local_dir.x, local_dir.y);
}

void Transform2D::Reset() {
    position_ = Vector2f(0.0f, 0.0f);
    rotation_ = 0.0f;
    scale_ = Vector2f(1.0f, 1.0f);
    InvalidateLocalMatrix();
}

bool Transform2D::operator==(const Transform2D& other) const {
    const float epsilon = 1e-6f;
    return (position_ - other.position_).lengthSquared() < epsilon &&
           std::abs(rotation_ - other.rotation_) < epsilon &&
           (scale_ - other.scale_).lengthSquared() < epsilon;
}

void Transform2D::InvalidateLocalMatrix() {
    local_matrix_dirty_ = true;
    parent_to_local_dirty_ = true;
}

void Transform2D::UpdateLocalMatrix() const {
    local_to_parent_ = TransformUtils::CreateTRSMatrix(position_, rotation_, scale_);
    local_matrix_dirty_ = false;
}

void Transform2D::UpdateParentToLocalMatrix() const {
    if (local_matrix_dirty_) {
        UpdateLocalMatrix();
    }
    parent_to_local_ = local_to_parent_.inverse();
    parent_to_local_dirty_ = false;
}

void Transform2D::UpdateInverseWorldMatrix() const {
    inverse_world_matrix_ = world_matrix_.inverse();
    inverse_world_dirty_ = false;
}

// TransformUtils Implementation

namespace TransformUtils {

Matrix3f CreateTranslationMatrix(const Vector2f& translation) {
    return Matrix3f(
        1.0f, 0.0f, translation.x,
        0.0f, 1.0f, translation.y,
        0.0f, 0.0f, 1.0f
    );
}

Matrix3f CreateRotationMatrix(float angle) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    return Matrix3f(
        cos_a, -sin_a, 0.0f,
        sin_a,  cos_a, 0.0f,
        0.0f,   0.0f,  1.0f
    );
}

Matrix3f CreateScaleMatrix(const Vector2f& scale) {
    return Matrix3f(
        scale.x, 0.0f,    0.0f,
        0.0f,    scale.y, 0.0f,
        0.0f,    0.0f,    1.0f
    );
}

Matrix3f CreateTRSMatrix(const Vector2f& translation, float rotation, const Vector2f& scale) {
    float cos_r = std::cos(rotation);
    float sin_r = std::sin(rotation);
    
    return Matrix3f(
        scale.x * cos_r, -scale.y * sin_r, translation.x,
        scale.x * sin_r,  scale.y * cos_r, translation.y,
        0.0f,             0.0f,            1.0f
    );
}

Vector2f ExtractTranslation(const Matrix3f& matrix) {
    return Vector2f(matrix(0, 2), matrix(1, 2));
}

float ExtractRotation(const Matrix3f& matrix) {
    return std::atan2(matrix(1, 0), matrix(0, 0));
}

Vector2f ExtractScale(const Matrix3f& matrix) {
    float scale_x = std::sqrt(matrix(0, 0) * matrix(0, 0) + matrix(1, 0) * matrix(1, 0));
    float scale_y = std::sqrt(matrix(0, 1) * matrix(0, 1) + matrix(1, 1) * matrix(1, 1));
    
    // Handle negative scale (if determinant is negative)
    if (matrix.determinant() < 0.0f) {
        scale_x = -scale_x;
    }
    
    return Vector2f(scale_x, scale_y);
}

Transform2D Lerp(const Transform2D& a, const Transform2D& b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    
    Vector2f lerped_position = a.GetPosition() + (b.GetPosition() - a.GetPosition()) * t;
    Vector2f lerped_scale = a.GetScale() + (b.GetScale() - a.GetScale()) * t;
    
    // Linear interpolation for rotation (not shortest path)
    float lerped_rotation = a.GetRotation() + (b.GetRotation() - a.GetRotation()) * t;
    
    return Transform2D(lerped_position, lerped_rotation, lerped_scale);
}

Transform2D Slerp(const Transform2D& a, const Transform2D& b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    
    Vector2f lerped_position = a.GetPosition() + (b.GetPosition() - a.GetPosition()) * t;
    Vector2f lerped_scale = a.GetScale() + (b.GetScale() - a.GetScale()) * t;
    
    // Spherical interpolation for rotation (shortest path)
    float angle_a = a.GetRotation();
    float angle_b = b.GetRotation();
    
    // Find the shortest path
    float diff = angle_b - angle_a;
    if (diff > M_PI) {
        diff -= 2.0f * M_PI;
    } else if (diff < -M_PI) {
        diff += 2.0f * M_PI;
    }
    
    float lerped_rotation = angle_a + diff * t;
    
    return Transform2D(lerped_position, lerped_rotation, lerped_scale);
}

} // namespace TransformUtils

} // namespace Scene
} // namespace PyNovaGE