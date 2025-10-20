#include <gtest/gtest.h>
#include "camera/third_person_camera.hpp"
#include "vectors/vector3.hpp"
#include "matrices/matrix4.hpp"

using namespace PyNovaGE;
using namespace PyNovaGE::Camera;

class ThirdPersonCameraTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.default_distance = 10.0f;
        config_.min_distance = 2.0f;
        config_.max_distance = 50.0f;
        config_.mouse_sensitivity = 0.5f;
        camera_ = std::make_unique<ThirdPersonCamera>(config_);
    }

    ThirdPersonCamera::Config config_;
    std::unique_ptr<ThirdPersonCamera> camera_;
};

TEST_F(ThirdPersonCameraTest, InitializationTest) {
    EXPECT_EQ(camera_->GetZoom(), config_.default_distance);
    EXPECT_EQ(camera_->GetYaw(), 0.0f);
    EXPECT_EQ(camera_->GetPitch(), 0.0f);
}

TEST_F(ThirdPersonCameraTest, UpdateTest) {
    Vector3f player_position(5.0f, 0.0f, 5.0f);
    
    // Update camera
    camera_->Update(0.016f, player_position); // 60 FPS delta time
    
    // Camera should track player position
    Vector3f camera_pos = camera_->GetPosition();
    EXPECT_GT(camera_pos.length(), 0.0f);
}

TEST_F(ThirdPersonCameraTest, ScrollZoomTest) {
    float initial_zoom = camera_->GetZoom();
    
    // Scroll in (zoom in)
    camera_->HandleScrollInput(1.0);
    camera_->Update(1.0f, Vector3f(0, 0, 0)); // Apply smooth interpolation
    EXPECT_LT(camera_->GetZoom(), initial_zoom);
    
    // Scroll out (zoom out)  
    camera_->HandleScrollInput(-2.0);
    camera_->Update(1.0f, Vector3f(0, 0, 0)); // Apply smooth interpolation
    EXPECT_GT(camera_->GetZoom(), initial_zoom);
    
    // Test clamping
    camera_->HandleScrollInput(-100.0); // Extreme zoom out
    EXPECT_LE(camera_->GetZoom(), config_.max_distance);
    
    camera_->HandleScrollInput(100.0); // Extreme zoom in
    EXPECT_GE(camera_->GetZoom(), config_.min_distance);
}

TEST_F(ThirdPersonCameraTest, MouseRotationTest) {
    float initial_yaw = camera_->GetYaw();
    float initial_pitch = camera_->GetPitch();
    
    // Simulate mouse drag
    camera_->HandleMouseInput(100.0, 100.0, true);  // Start drag
    camera_->HandleMouseInput(150.0, 120.0, true);  // Move mouse
    camera_->Update(1.0f, Vector3f(0, 0, 0)); // Apply smooth interpolation
    
    // Should have changed rotation
    EXPECT_NE(camera_->GetYaw(), initial_yaw);
    EXPECT_NE(camera_->GetPitch(), initial_pitch);
    
    // End drag
    camera_->HandleMouseInput(150.0, 120.0, false);
}

TEST_F(ThirdPersonCameraTest, RecenterTest) {
    // Rotate camera away from default
    camera_->HandleMouseInput(100.0, 100.0, true);
    camera_->HandleMouseInput(200.0, 150.0, true);
    camera_->HandleScrollInput(-5.0);
    
    // Recenter
    camera_->RecenterBehindPlayer();
    
    // Should reset to defaults
    EXPECT_FLOAT_EQ(camera_->GetYaw(), 0.0f);
    EXPECT_FLOAT_EQ(camera_->GetPitch(), 0.0f);
    EXPECT_FLOAT_EQ(camera_->GetZoom(), config_.default_distance);
}

TEST_F(ThirdPersonCameraTest, ViewMatrixTest) {
    Vector3f player_position(0.0f, 0.0f, 0.0f);
    camera_->Update(0.016f, player_position);
    
    Matrix4<float> view_matrix = camera_->GetViewMatrix();
    
    // View matrix should be valid (not all zeros)
    bool has_non_zero = false;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (view_matrix[i][j] != 0.0f) {
                has_non_zero = true;
                break;
            }
        }
    }
    EXPECT_TRUE(has_non_zero);
}

TEST_F(ThirdPersonCameraTest, DirectionVectorsTest) {
    Vector3f player_position(0.0f, 0.0f, 0.0f);
    camera_->Update(0.016f, player_position);
    
    Vector3f forward = camera_->GetForward();
    Vector3f right = camera_->GetRight();
    Vector3f up = camera_->GetUp();
    
    // Vectors should be normalized
    EXPECT_FLOAT_EQ(forward.length(), 1.0f);
    EXPECT_FLOAT_EQ(right.length(), 1.0f);
    EXPECT_FLOAT_EQ(up.length(), 1.0f);
    
    // Vectors should be orthogonal (dot product near zero)
    EXPECT_NEAR(forward.dot(right), 0.0f, 0.01f);
    EXPECT_NEAR(forward.dot(up), 0.0f, 0.01f);
    EXPECT_NEAR(right.dot(up), 0.0f, 0.01f);
}

TEST_F(ThirdPersonCameraTest, ConfigurationTest) {
    ThirdPersonCamera::Config new_config;
    new_config.default_distance = 15.0f;
    new_config.mouse_sensitivity = 1.0f;
    
    camera_->SetConfig(new_config);
    
    const auto& retrieved_config = camera_->GetConfig();
    EXPECT_FLOAT_EQ(retrieved_config.default_distance, 15.0f);
    EXPECT_FLOAT_EQ(retrieved_config.mouse_sensitivity, 1.0f);
}