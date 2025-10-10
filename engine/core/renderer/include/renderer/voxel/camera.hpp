#pragma once

#include <vectors/vector3.hpp>
#include <vectors/vector4.hpp>
#include <matrices/matrix4.hpp>
#include <cmath>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

// Type aliases for convenience
using Vector3f = Vector3<float>;
using Vector4f = Vector4<float>;
using Matrix4f = Matrix4<float>;

/**
 * @brief 3D Camera for voxel world navigation
 * 
 * Provides first-person camera controls with view and projection matrices
 * optimized for voxel world rendering. Supports both flying and walking modes.
 */
class Camera {
public:
    /**
     * @brief Camera movement modes
     */
    enum class Mode {
        Flying,     // Free-flying camera (noclip)
        Walking     // Ground-based walking with gravity
    };

    /**
     * @brief Camera projection types
     */
    enum class Projection {
        Perspective,
        Orthographic
    };

    /**
     * @brief Default camera constructor
     * Creates camera at origin looking down negative Z axis
     */
    Camera();

    /**
     * @brief Create camera at specific position
     * @param position Initial camera position
     * @param target Initial look target
     * @param up Up vector (usually {0,1,0})
     */
    Camera(const Vector3f& position, const Vector3f& target, const Vector3f& up = {0.0f, 1.0f, 0.0f});

    // === Position and Orientation ===
    
    /**
     * @brief Set camera position
     */
    void SetPosition(const Vector3f& position);
    
    /**
     * @brief Get camera position
     */
    const Vector3f& GetPosition() const { return position_; }

    /**
     * @brief Set camera target (look-at point)
     */
    void SetTarget(const Vector3f& target);
    
    /**
     * @brief Get camera forward direction
     */
    const Vector3f& GetForward() const { return forward_; }
    
    /**
     * @brief Get camera right direction
     */
    const Vector3f& GetRight() const { return right_; }
    
    /**
     * @brief Get camera up direction
     */
    const Vector3f& GetUp() const { return up_; }

    /**
     * @brief Set camera rotation using Euler angles
     * @param yaw Rotation around Y axis (degrees)
     * @param pitch Rotation around X axis (degrees)
     */
    void SetRotation(float yaw, float pitch);
    
    /**
     * @brief Get yaw angle in degrees
     */
    float GetYaw() const { return yaw_; }
    
    /**
     * @brief Get pitch angle in degrees
     */
    float GetPitch() const { return pitch_; }

    // === Camera Movement ===
    
    /**
     * @brief Move camera forward/backward
     * @param distance Distance to move (positive = forward, negative = backward)
     */
    void MoveForward(float distance);
    
    /**
     * @brief Move camera right/left
     * @param distance Distance to move (positive = right, negative = left)
     */
    void MoveRight(float distance);
    
    /**
     * @brief Move camera up/down
     * @param distance Distance to move (positive = up, negative = down)
     */
    void MoveUp(float distance);

    /**
     * @brief Rotate camera using mouse delta
     * @param delta_yaw Yaw rotation change (degrees)
     * @param delta_pitch Pitch rotation change (degrees)
     */
    void Rotate(float delta_yaw, float delta_pitch);

    /**
     * @brief Set camera mode
     */
    void SetMode(Mode mode) { mode_ = mode; }
    
    /**
     * @brief Get camera mode
     */
    Mode GetMode() const { return mode_; }

    // === Projection Settings ===
    
    /**
     * @brief Set perspective projection
     * @param fov Field of view in degrees
     * @param aspect_ratio Aspect ratio (width/height)
     * @param near_plane Near clipping plane
     * @param far_plane Far clipping plane
     */
    void SetPerspective(float fov, float aspect_ratio, float near_plane, float far_plane);
    
    /**
     * @brief Set orthographic projection
     * @param left Left boundary
     * @param right Right boundary
     * @param bottom Bottom boundary
     * @param top Top boundary
     * @param near_plane Near clipping plane
     * @param far_plane Far clipping plane
     */
    void SetOrthographic(float left, float right, float bottom, float top, 
                        float near_plane, float far_plane);

    /**
     * @brief Update aspect ratio (call when window resizes)
     */
    void SetAspectRatio(float aspect_ratio);

    // === Matrix Access ===
    
    /**
     * @brief Get view matrix
     */
    const Matrix4f& GetViewMatrix() const;
    
    /**
     * @brief Get projection matrix  
     */
    const Matrix4f& GetProjectionMatrix() const;
    
    /**
     * @brief Get view-projection matrix (projection * view)
     */
    Matrix4f GetViewProjectionMatrix() const;

    // === Frustum and Culling ===
    
    /**
     * @brief Frustum planes for culling
     */
    struct Frustum {
        Vector4f planes[6]; // Left, Right, Bottom, Top, Near, Far
    };
    
    /**
     * @brief Extract frustum planes from view-projection matrix
     */
    Frustum ExtractFrustum();
    
    /**
     * @brief Check if a point is inside the frustum
     */
    bool IsPointInFrustum(const Vector3f& point, const Frustum& frustum) const;
    
    /**
     * @brief Check if an AABB is inside the frustum
     */
    bool IsAABBInFrustum(const Vector3f& min, const Vector3f& max, const Frustum& frustum) const;

    // === Camera Properties ===
    
    /**
     * @brief Get field of view (perspective only)
     */
    float GetFOV() const { return fov_; }
    
    /**
     * @brief Get aspect ratio
     */
    float GetAspectRatio() const { return aspect_ratio_; }
    
    /**
     * @brief Get near plane distance
     */
    float GetNearPlane() const { return near_plane_; }
    
    /**
     * @brief Get far plane distance
     */
    float GetFarPlane() const { return far_plane_; }

    // === Camera Controls Configuration ===
    
    /**
     * @brief Set mouse sensitivity
     */
    void SetMouseSensitivity(float sensitivity) { mouse_sensitivity_ = sensitivity; }
    
    /**
     * @brief Get mouse sensitivity
     */
    float GetMouseSensitivity() const { return mouse_sensitivity_; }
    
    /**
     * @brief Set movement speed
     */
    void SetMovementSpeed(float speed) { movement_speed_ = speed; }
    
    /**
     * @brief Get movement speed
     */
    float GetMovementSpeed() const { return movement_speed_; }

private:
    /**
     * @brief Update camera vectors from yaw and pitch
     */
    void UpdateVectors();
    
    /**
     * @brief Update view matrix
     */
    void UpdateViewMatrix() const;
    
    /**
     * @brief Update projection matrix
     */
    void UpdateProjectionMatrix() const;

private:
    // Position and orientation
    Vector3f position_{0.0f, 0.0f, 0.0f};
    Vector3f forward_{0.0f, 0.0f, -1.0f};
    Vector3f right_{1.0f, 0.0f, 0.0f};
    Vector3f up_{0.0f, 1.0f, 0.0f};
    Vector3f world_up_{0.0f, 1.0f, 0.0f};
    
    // Rotation angles (degrees)
    float yaw_ = -90.0f;    // Initialized to look down negative Z
    float pitch_ = 0.0f;
    
    // Constraints
    float max_pitch_ = 89.0f;
    float min_pitch_ = -89.0f;
    
    // Projection settings
    Projection projection_type_ = Projection::Perspective;
    float fov_ = 45.0f;
    float aspect_ratio_ = 16.0f / 9.0f;
    float near_plane_ = 0.1f;
    float far_plane_ = 1000.0f;
    
    // Orthographic settings
    float ortho_left_ = -10.0f;
    float ortho_right_ = 10.0f;
    float ortho_bottom_ = -10.0f;
    float ortho_top_ = 10.0f;
    
    // Matrices
    mutable Matrix4f view_matrix_;
    mutable Matrix4f projection_matrix_;
    mutable bool view_matrix_dirty_ = true;
    mutable bool projection_matrix_dirty_ = true;
    
    // Movement settings
    Mode mode_ = Mode::Flying;
    float movement_speed_ = 10.0f;      // Units per second
    float mouse_sensitivity_ = 0.1f;    // Degrees per pixel
};

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE