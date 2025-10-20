#include "camera/third_person_camera.hpp"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PyNovaGE {
namespace Camera {

// Helper function for converting degrees to radians
constexpr float ToRadians(float degrees) {
    return degrees * (static_cast<float>(M_PI) / 180.0f);
}

// Helper function for converting radians to degrees
constexpr float ToDegrees(float radians) {
    return radians * (180.0f / static_cast<float>(M_PI));
}

ThirdPersonCamera::ThirdPersonCamera(const Config& config)
    : config_(config)
    , current_position_(0.0f, 2.0f, 8.0f)
    , target_player_position_(0.0f, 0.0f, 0.0f)
    , smoothed_player_position_(0.0f, 0.0f, 0.0f)
    , target_yaw_(0.0f)
    , target_pitch_(0.0f)
    , current_yaw_(0.0f)
    , current_pitch_(0.0f)
    , target_distance_(config.default_distance)
    , current_distance_(config.default_distance)
    , is_dragging_(false)
    , last_mouse_x_(0.0)
    , last_mouse_y_(0.0)
    , first_mouse_(true)
{
}

void ThirdPersonCamera::Update(float delta_time, const Vector3f& player_position) {
    // Update target player position
    target_player_position_ = player_position;
    
    // Smooth player position following
    SmoothFloat(smoothed_player_position_.x, target_player_position_.x, config_.position_smoothing, delta_time);
    SmoothFloat(smoothed_player_position_.y, target_player_position_.y, config_.position_smoothing, delta_time);
    SmoothFloat(smoothed_player_position_.z, target_player_position_.z, config_.position_smoothing, delta_time);
    
    // Update camera components
    UpdateRotation(delta_time);
    UpdateDistance(delta_time);
    UpdatePosition(delta_time);
}

void ThirdPersonCamera::HandleMouseInput(double xpos, double ypos, bool right_button_pressed) {
    // Handle first mouse input to prevent camera jump
    if (first_mouse_) {
        last_mouse_x_ = xpos;
        last_mouse_y_ = ypos;
        first_mouse_ = false;
    }
    
    if (right_button_pressed) {
        if (!is_dragging_) {
            // Start dragging - record initial position
            last_mouse_x_ = xpos;
            last_mouse_y_ = ypos;
            is_dragging_ = true;
        } else {
            // Calculate mouse delta
            double delta_x = xpos - last_mouse_x_;
            double delta_y = ypos - last_mouse_y_;
            
            // Apply rotation with sensitivity
            target_yaw_ += static_cast<float>(delta_x) * config_.mouse_sensitivity;
            target_pitch_ -= static_cast<float>(delta_y) * config_.mouse_sensitivity; // Inverted Y
            
            // Clamp pitch to limits
            target_pitch_ = ClampPitch(target_pitch_);
            
            // Normalize yaw to 0-360 range
            while (target_yaw_ < 0.0f) target_yaw_ += 360.0f;
            while (target_yaw_ >= 360.0f) target_yaw_ -= 360.0f;
        }
        
        // Update last mouse position
        last_mouse_x_ = xpos;
        last_mouse_y_ = ypos;
    } else {
        // Stop dragging
        is_dragging_ = false;
    }
}

void ThirdPersonCamera::HandleScrollInput(double y_offset) {
    // Adjust target distance based on scroll
    float zoom_delta = -static_cast<float>(y_offset) * config_.scroll_sensitivity;
    target_distance_ += zoom_delta;
    
    // Clamp to distance limits
    target_distance_ = std::clamp(target_distance_, config_.min_distance, config_.max_distance);
}

void ThirdPersonCamera::HandleKeyInput(int key, int action) {
    if (action == ACTION_PRESS) {
        switch (key) {
            case KEY_INSERT:
            case KEY_HOME:
                // Recenter camera behind player
                RecenterBehindPlayer();
                break;
        }
    }
}

Matrix4<float> ThirdPersonCamera::GetViewMatrix() const {
    // Calculate look-at target (pivot point + offset)
    Vector3f target = smoothed_player_position_ + config_.pivot_offset;
    
    // Create view matrix looking at the target
    return Matrix4<float>::LookAt(current_position_, target, Vector3f(0.0f, 1.0f, 0.0f));
}

Vector3f ThirdPersonCamera::GetForward() const {
    // Calculate forward direction from camera to target
    Vector3f target = smoothed_player_position_ + config_.pivot_offset;
    Vector3f forward = target - current_position_;
    return forward.normalized();
}

Vector3f ThirdPersonCamera::GetRight() const {
    Vector3f forward = GetForward();
    Vector3f up(0.0f, 1.0f, 0.0f);
    Vector3f right = forward.cross(up);
    return right.normalized();
}

Vector3f ThirdPersonCamera::GetUp() const {
    Vector3f forward = GetForward();
    Vector3f right = GetRight();
    Vector3f up = right.cross(forward);
    return up.normalized();
}

void ThirdPersonCamera::RecenterBehindPlayer() {
    // Reset rotation to behind player (yaw = 0, pitch = 0)
    target_yaw_ = 0.0f;
    target_pitch_ = 0.0f;
    target_distance_ = config_.default_distance;
}

void ThirdPersonCamera::UpdateRotation(float delta_time) {
    // Smooth rotation towards target
    SmoothFloat(current_yaw_, target_yaw_, config_.rotation_smoothing, delta_time);
    SmoothFloat(current_pitch_, target_pitch_, config_.rotation_smoothing, delta_time);
}

void ThirdPersonCamera::UpdateDistance(float delta_time) {
    // Smooth distance towards target
    SmoothFloat(current_distance_, target_distance_, config_.zoom_smoothing, delta_time);
}

void ThirdPersonCamera::UpdatePosition(float /*delta_time*/) {
    // Calculate desired camera position
    Vector3f desired_position = CalculateDesiredPosition();
    
    // Handle collision if enabled
    if (config_.enable_collision) {
        desired_position = HandleCameraCollision(desired_position);
    }
    
    // Update current position
    current_position_ = desired_position;
}

Vector3f ThirdPersonCamera::CalculateDesiredPosition() const {
    // Get pivot point (player position + offset)
    Vector3f pivot = smoothed_player_position_ + config_.pivot_offset;
    
    // Convert angles to radians
    float yaw_rad = ToRadians(current_yaw_);
    float pitch_rad = ToRadians(current_pitch_);
    
    // Calculate camera offset from pivot
    float cos_pitch = std::cos(pitch_rad);
    float sin_pitch = std::sin(pitch_rad);
    float cos_yaw = std::cos(yaw_rad);
    float sin_yaw = std::sin(yaw_rad);
    
    // Camera offset vector (spherical coordinates)
    Vector3f offset(
        current_distance_ * cos_pitch * sin_yaw,    // X
        current_distance_ * sin_pitch,              // Y  
        current_distance_ * cos_pitch * cos_yaw     // Z
    );
    
    return pivot + offset;
}

Vector3f ThirdPersonCamera::HandleCameraCollision(const Vector3f& desired_position) const {
    // Simple collision detection - raycast from pivot to desired position
    Vector3f pivot = smoothed_player_position_ + config_.pivot_offset;
    Vector3f direction = desired_position - pivot;
    float distance = direction.length();
    
    if (distance < 0.001f) {
        return desired_position; // No movement
    }
    
    Vector3f normalized_direction = direction / distance;
    
    // For now, just return desired position
    // TODO: Implement proper world collision detection
    // This would involve raycasting against world geometry
    
    return desired_position;
}

float ThirdPersonCamera::ClampPitch(float pitch) const {
    return std::clamp(pitch, config_.min_pitch, config_.max_pitch);
}

void ThirdPersonCamera::SmoothFloat(float& current, float target, float speed, float delta_time) {
    // Exponential smoothing
    float factor = 1.0f - std::exp(-speed * delta_time);
    current = current + (target - current) * factor;
}

} // namespace Camera
} // namespace PyNovaGE