#pragma once

#include <vectors/vectors.hpp>
#include <matrices/matrices.hpp>
#include <cmath>

namespace PyNovaGE {
namespace Scene {

// Type aliases for convenience
using Vector2f = PyNovaGE::Vector2<float>;
using Vector3f = PyNovaGE::Vector3<float>;
using Matrix3f = PyNovaGE::Matrix3<float>;

/**
 * @brief 2D Transform for scene graph nodes
 * 
 * Represents position, rotation, and scale in 2D space.
 * Provides local-to-world and world-to-local transform matrices.
 */
class Transform2D {
public:
    Transform2D() = default;
    Transform2D(const Vector2f& position, float rotation = 0.0f, const Vector2f& scale = Vector2f(1.0f, 1.0f));

    // Local transform properties
    void SetPosition(const Vector2f& position);
    const Vector2f& GetPosition() const { return position_; }
    
    void SetRotation(float rotation);
    float GetRotation() const { return rotation_; }
    
    void SetScale(const Vector2f& scale);
    const Vector2f& GetScale() const { return scale_; }

    // Transform operations
    void Translate(const Vector2f& translation);
    void Rotate(float angle);
    void Scale(const Vector2f& scale);
    void Scale(float uniform_scale);

    // Matrix access (local to parent transform)
    const Matrix3f& GetLocalToParentMatrix() const;
    const Matrix3f& GetParentToLocalMatrix() const;

    // World transform (computed from parent hierarchy)
    void SetWorldMatrix(const Matrix3f& world_matrix);
    const Matrix3f& GetWorldMatrix() const { return world_matrix_; }
    const Matrix3f& GetInverseWorldMatrix() const;

    // World transform properties (computed from world matrix)
    Vector2f GetWorldPosition() const;
    float GetWorldRotation() const;
    Vector2f GetWorldScale() const;

    // Point transformation
    Vector2f TransformPoint(const Vector2f& local_point) const;
    Vector2f TransformDirection(const Vector2f& local_direction) const;
    Vector2f InverseTransformPoint(const Vector2f& world_point) const;
    Vector2f InverseTransformDirection(const Vector2f& world_direction) const;

    // Reset to identity
    void Reset();

    // Comparison
    bool operator==(const Transform2D& other) const;
    bool operator!=(const Transform2D& other) const { return !(*this == other); }

private:
    // Local transform properties
    Vector2f position_{0.0f, 0.0f};
    float rotation_ = 0.0f;      // In radians
    Vector2f scale_{1.0f, 1.0f};

    // Cached matrices (marked mutable for lazy evaluation)
    mutable Matrix3f local_to_parent_;
    mutable Matrix3f parent_to_local_;
    mutable bool local_matrix_dirty_ = true;
    mutable bool parent_to_local_dirty_ = true;

    // World transform (set by scene graph)
    Matrix3f world_matrix_ = Matrix3f::Identity();
    mutable Matrix3f inverse_world_matrix_ = Matrix3f::Identity();
    mutable bool inverse_world_dirty_ = true;

    // Internal methods
    void InvalidateLocalMatrix();
    void UpdateLocalMatrix() const;
    void UpdateParentToLocalMatrix() const;
    void UpdateInverseWorldMatrix() const;
};

/**
 * @brief Transform utility functions
 */
namespace TransformUtils {
    // Create transform matrices
    Matrix3f CreateTranslationMatrix(const Vector2f& translation);
    Matrix3f CreateRotationMatrix(float angle);
    Matrix3f CreateScaleMatrix(const Vector2f& scale);
    Matrix3f CreateTRSMatrix(const Vector2f& translation, float rotation, const Vector2f& scale);

    // Extract transform components from matrix
    Vector2f ExtractTranslation(const Matrix3f& matrix);
    float ExtractRotation(const Matrix3f& matrix);
    Vector2f ExtractScale(const Matrix3f& matrix);

    // Interpolation
    Transform2D Lerp(const Transform2D& a, const Transform2D& b, float t);
    Transform2D Slerp(const Transform2D& a, const Transform2D& b, float t); // Spherical interpolation for rotation
}

} // namespace Scene
} // namespace PyNovaGE