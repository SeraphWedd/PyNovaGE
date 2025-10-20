#pragma once

#include "vectors/vector3.hpp"
#include "matrices/matrix4.hpp"

namespace PyNovaGE {
namespace Camera {

/**
 * @brief 3rd Person Camera Controller for MMO-style games
 * 
 * Features:
 * - Right mouse button hold & drag to rotate camera
 * - Scroll wheel zoom in/out with limits
 * - Pivot anchor behind player model
 * - Smooth interpolation for all movements
 * - Independent character/camera facing
 * - Recenter key to reset behind character
 * 
 * Formula: camera_pos = player_pos + rotate(offset_vector, yaw, pitch) * zoom_factor
 */
class ThirdPersonCamera {
public:
    /**
     * @brief Camera configuration settings
     */
    struct Config {
        // Distance and positioning
        float default_distance = 8.0f;        // Default camera distance from player
        float min_distance = 3.0f;            // Minimum zoom distance
        float max_distance = 20.0f;           // Maximum zoom distance
        
        // Rotation limits
        float min_pitch = -45.0f;             // Maximum downward angle (degrees)
        float max_pitch = 85.0f;              // Maximum upward angle (degrees)
        
        // Movement smoothing
        float rotation_smoothing = 8.0f;      // Rotation interpolation speed
        float zoom_smoothing = 10.0f;         // Zoom interpolation speed
        float position_smoothing = 6.0f;      // Position following smoothness
        
        // Input sensitivity
        float mouse_sensitivity = 0.3f;       // Mouse rotation sensitivity
        float scroll_sensitivity = 1.5f;      // Scroll wheel zoom sensitivity
        
        // Pivot offset
        Vector3f pivot_offset = Vector3f(0.0f, 1.8f, 0.0f);  // Anchor point above player
        
        // Collision
        bool enable_collision = true;         // Enable camera-world collision
        float collision_radius = 0.5f;       // Camera collision sphere radius
    };

    explicit ThirdPersonCamera(const Config& config = Config{});
    ~ThirdPersonCamera() = default;

    /**
     * @brief Update camera state
     * @param delta_time Frame delta time
     * @param player_position Current player world position
     */
    void Update(float delta_time, const Vector3f& player_position);

    /**
     * @brief Handle mouse input for camera rotation
     * @param xpos Current mouse X position
     * @param ypos Current mouse Y position
     * @param right_button_pressed Is right mouse button held down
     */
    void HandleMouseInput(double xpos, double ypos, bool right_button_pressed);

    /**
     * @brief Handle scroll input for zoom
     * @param y_offset Scroll wheel Y offset
     */
    void HandleScrollInput(double y_offset);

    /**
     * @brief Handle keyboard input for recenter functionality
     * @param key Key code (engine-agnostic)
     * @param action Action (1 = press, 0 = release)
     */
    void HandleKeyInput(int key, int action);
    
    /**
     * @brief Key constants for camera controls
     */
    enum KeyConstants {
        KEY_INSERT = 260,
        KEY_HOME = 268,
        ACTION_PRESS = 1,
        ACTION_RELEASE = 0
    };

    /**
     * @brief Get current view matrix for rendering
     * @return View matrix for current camera state
     */
    Matrix4<float> GetViewMatrix() const;

    /**
     * @brief Get camera world position
     */
    Vector3f GetPosition() const { return current_position_; }

    /**
     * @brief Get camera forward direction
     */
    Vector3f GetForward() const;

    /**
     * @brief Get camera right direction  
     */
    Vector3f GetRight() const;

    /**
     * @brief Get camera up direction
     */
    Vector3f GetUp() const;

    /**
     * @brief Reset camera to default position behind player
     */
    void RecenterBehindPlayer();

    /**
     * @brief Set configuration
     */
    void SetConfig(const Config& config) { config_ = config; }

    /**
     * @brief Get configuration
     */
    const Config& GetConfig() const { return config_; }

    /**
     * @brief Set target player position (for following)
     */
    void SetTargetPosition(const Vector3f& position) { target_player_position_ = position; }

    /**
     * @brief Get current yaw angle (degrees)
     */
    float GetYaw() const { return current_yaw_; }

    /**
     * @brief Get current pitch angle (degrees) 
     */
    float GetPitch() const { return current_pitch_; }

    /**
     * @brief Get current zoom distance
     */
    float GetZoom() const { return current_distance_; }

private:
    Config config_;

    // Camera state
    Vector3f current_position_;              // Current camera world position
    Vector3f target_player_position_;       // Player position to follow
    Vector3f smoothed_player_position_;     // Smoothed player position for following
    
    // Rotation state
    float target_yaw_;                      // Target horizontal rotation (degrees)
    float target_pitch_;                    // Target vertical rotation (degrees)
    float current_yaw_;                     // Current smoothed yaw
    float current_pitch_;                   // Current smoothed pitch
    
    // Distance/zoom state
    float target_distance_;                 // Target camera distance
    float current_distance_;                // Current smoothed distance
    
    // Input state
    bool is_dragging_;                      // Is right mouse button held
    double last_mouse_x_;                   // Last mouse X position
    double last_mouse_y_;                   // Last mouse Y position
    bool first_mouse_;                      // First mouse input (to prevent jump)

    // Internal methods
    void UpdateRotation(float delta_time);
    void UpdateDistance(float delta_time);
    void UpdatePosition(float delta_time);
    Vector3f CalculateDesiredPosition() const;
    Vector3f HandleCameraCollision(const Vector3f& desired_position) const;
    float ClampPitch(float pitch) const;
    void SmoothFloat(float& current, float target, float speed, float delta_time);
};

} // namespace Camera
} // namespace PyNovaGE