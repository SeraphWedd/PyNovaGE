#include "scene/transform2d.hpp"
#include <cmath>

namespace PyNovaGE {
namespace Scene {

Transform2D::Transform2D(const Vector2f& position, float rotation, const Vector2f& scale)
    : position_(position)
    , rotation_(rotation)
    , scale_(scale)
{
    InvalidateLocalMatrix();
}

void Transform2D::SetPosition(const Vector2f& position) {
    if (position_ != position) {
        position_ = position;
        InvalidateLocalMatrix();
    }
}

void Transform2D::SetRotation(float rotation) {
    if (rotation_ != rotation) {
        rotation_ = rotation;
        InvalidateLocalMatrix();
    }
}

void Transform2D::SetScale(const Vector2f& scale) {
    if (scale_ != scale) {
        scale_ = scale;
        InvalidateLocalMatrix();
    }
}

void Transform2D::Translate(const Vector2f& translation) {
    SetPosition(position_ + translation);
}

void Transform2D::Rotate(float angle) {
    SetRotation(rotation_ + angle);
}

void Transform2D::Scale(const Vector2f& scale) {
    SetScale(Vector2f(scale_.x * scale.x, scale_.y * scale.y));
}

void Transform2D::Scale(float uniform_scale) {
    Scale(Vector2f(uniform_scale, uniform_scale));
}

const Matrix3f& Transform2D::GetLocalToParentMatrix() const {
    if (local_matrix_dirty_) {
        UpdateLocalMatrix();
    }
    return local_to_parent_;
}

const Matrix3f& Transform2D::GetParentToLocalMatrix() const {
    if (local_matrix_dirty_ || parent_to_local_dirty_) {
        UpdateParentToLocalMatrix();
    }
    return parent_to_local_;
}

const Matrix3f& Transform2D::GetInverseWorldMatrix() const {
    if (inverse_world_dirty_) {
        UpdateInverseWorldMatrix();
    }
    return inverse_world_matrix_;
}

void Transform2D::SetWorldMatrix(const Matrix3f& world_matrix) {
    world_matrix_ = world_matrix;
    inverse_world_dirty_ = true;
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
    // Apply full transformation including translation
    return Vector2f(
        world_matrix_(0, 0) * local_point.x + world_matrix_(0, 1) * local_point.y + world_matrix_(0, 2),
        world_matrix_(1, 0) * local_point.x + world_matrix_(1, 1) * local_point.y + world_matrix_(1, 2)
    );
}

Vector2f Transform2D::TransformDirection(const Vector2f& local_direction) const {
    // Apply only rotation and scale, ignoring translation
    return Vector2f(
        world_matrix_(0, 0) * local_direction.x + world_matrix_(0, 1) * local_direction.y,
        world_matrix_(1, 0) * local_direction.x + world_matrix_(1, 1) * local_direction.y
    );
}

Vector2f Transform2D::InverseTransformPoint(const Vector2f& world_point) const {
    const Matrix3f& inv = GetInverseWorldMatrix();
    return Vector2f(
        inv(0, 0) * world_point.x + inv(0, 1) * world_point.y + inv(0, 2),
        inv(1, 0) * world_point.x + inv(1, 1) * world_point.y + inv(1, 2)
    );
}

Vector2f Transform2D::InverseTransformDirection(const Vector2f& world_direction) const {
    const Matrix3f& inv = GetInverseWorldMatrix();
    return Vector2f(
        inv(0, 0) * world_direction.x + inv(0, 1) * world_direction.y,
        inv(1, 0) * world_direction.x + inv(1, 1) * world_direction.y
    );
}

void Transform2D::Reset() {
    position_ = Vector2f(0.0f, 0.0f);
    rotation_ = 0.0f;
    scale_ = Vector2f(1.0f, 1.0f);
    InvalidateLocalMatrix();
    world_matrix_ = Matrix3f::Identity();
    inverse_world_matrix_ = Matrix3f::Identity();
    inverse_world_dirty_ = false;
}

bool Transform2D::operator==(const Transform2D& other) const {
    return position_ == other.position_ &&
           rotation_ == other.rotation_ &&
           scale_ == other.scale_;
}

void Transform2D::InvalidateLocalMatrix() {
    local_matrix_dirty_ = true;
    parent_to_local_dirty_ = true;
}

void Transform2D::UpdateLocalMatrix() const {
    // Build TRS matrix
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

namespace TransformUtils {

Matrix3f CreateTranslationMatrix(const Vector2f& translation) {
    return Matrix3f::Translation(translation.x, translation.y);
}

Matrix3f CreateRotationMatrix(float angle) {
    return Matrix3f::Rotation(angle);
}

Matrix3f CreateScaleMatrix(const Vector2f& scale) {
    return Matrix3f::Scale(scale.x, scale.y);
}

Matrix3f CreateTRSMatrix(const Vector2f& translation, float rotation, const Vector2f& scale) {
    Matrix3f T = CreateTranslationMatrix(translation);
    Matrix3f R = CreateRotationMatrix(rotation);
    Matrix3f S = CreateScaleMatrix(scale);

    // Multiply in reverse order (SRT) since we're using column vectors
    return T * R * S;
}

Vector2f ExtractTranslation(const Matrix3f& matrix) {
    return Vector2f(matrix(0, 2), matrix(1, 2));
}

float ExtractRotation(const Matrix3f& matrix) {
    // For 2D rotation, we can extract the angle from the upper 2x2 matrix
    // atan2(sin, cos) gives us the angle
    return std::atan2(matrix(1, 0), matrix(0, 0));
}

Vector2f ExtractScale(const Matrix3f& matrix) {
    // Scale is the length of the basis vectors in the upper 2x2 matrix
    float sx = std::sqrt(matrix(0, 0) * matrix(0, 0) + matrix(1, 0) * matrix(1, 0));
    float sy = std::sqrt(matrix(0, 1) * matrix(0, 1) + matrix(1, 1) * matrix(1, 1));
    return Vector2f(sx, sy);
}

Transform2D Lerp(const Transform2D& a, const Transform2D& b, float t) {
    // Linearly interpolate position and scale
    Vector2f position = a.GetPosition() + (b.GetPosition() - a.GetPosition()) * t;
    Vector2f scale = a.GetScale() + (b.GetScale() - a.GetScale()) * t;

    // Simple linear interpolation for small angle differences
    float rotation = a.GetRotation() + (b.GetRotation() - a.GetRotation()) * t;

    return Transform2D(position, rotation, scale);
}

Transform2D Slerp(const Transform2D& a, const Transform2D& b, float t) {
    // Linear interpolation for position and scale
    Vector2f position = a.GetPosition() + (b.GetPosition() - a.GetPosition()) * t;
    Vector2f scale = a.GetScale() + (b.GetScale() - a.GetScale()) * t;

    // Spherical interpolation for rotation
    float angle_a = a.GetRotation();
    float angle_b = b.GetRotation();

    // Ensure shortest path
    float diff = angle_b - angle_a;
    if (diff > M_PI) angle_b -= 2.0f * M_PI;
    else if (diff < -M_PI) angle_b += 2.0f * M_PI;

    float rotation = angle_a + (angle_b - angle_a) * t;

    return Transform2D(position, rotation, scale);
}

} // namespace TransformUtils

} // namespace Scene
} // namespace PyNovaGE