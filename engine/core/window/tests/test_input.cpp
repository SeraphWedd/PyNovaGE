#include <gtest/gtest.h>
#include "window/window.hpp"
#include "window/input.hpp"
#include <thread>
#include <chrono>

using namespace PyNovaGE::Window;

class InputTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(InitializeWindowSystem());
        
        // Create a hidden window for input testing
        WindowConfig config;
        config.visible = false;
        config.width = 800;
        config.height = 600;
        
        window_ = std::make_unique<Window>(config);
        input_manager_ = std::make_unique<InputManager>(window_->GetNativeWindow());
    }
    
    void TearDown() override {
        input_manager_.reset();
        window_.reset();
        ShutdownWindowSystem();
    }

    std::unique_ptr<Window> window_;
    std::unique_ptr<InputManager> input_manager_;
};

TEST_F(InputTest, Construction) {
    EXPECT_NO_THROW({
        InputManager input(window_->GetNativeWindow());
    });
}

TEST_F(InputTest, InitialState) {
    // All keys should be released initially
    EXPECT_FALSE(input_manager_->IsKeyPressed(Key::A));
    EXPECT_FALSE(input_manager_->IsKeyJustPressed(Key::A));
    EXPECT_FALSE(input_manager_->IsKeyJustReleased(Key::A));
    
    // All mouse buttons should be released initially
    EXPECT_FALSE(input_manager_->IsMouseButtonPressed(MouseButton::Left));
    EXPECT_FALSE(input_manager_->IsMouseButtonJustPressed(MouseButton::Left));
    EXPECT_FALSE(input_manager_->IsMouseButtonJustReleased(MouseButton::Left));
    
    // Mouse position should be valid
    auto mouse_pos = input_manager_->GetMousePosition();
    EXPECT_GE(mouse_pos.x, 0.0f);
    EXPECT_GE(mouse_pos.y, 0.0f);
    
    // Mouse delta should be zero initially
    auto mouse_delta = input_manager_->GetMouseDelta();
    EXPECT_EQ(mouse_delta.x, 0.0f);
    EXPECT_EQ(mouse_delta.y, 0.0f);
    
    // Scroll delta should be zero initially
    auto scroll_delta = input_manager_->GetScrollDelta();
    EXPECT_EQ(scroll_delta.x, 0.0f);
    EXPECT_EQ(scroll_delta.y, 0.0f);
}

TEST_F(InputTest, KeyStateLogic) {
    // Test key enum values
    EXPECT_EQ(static_cast<int>(Key::A), 65);
    EXPECT_EQ(static_cast<int>(Key::Space), 32);
    EXPECT_EQ(static_cast<int>(Key::Escape), 256);
    
    // Test invalid key handling
    EXPECT_FALSE(input_manager_->IsKeyPressed(Key::Unknown));
    EXPECT_FALSE(input_manager_->IsKeyPressed(static_cast<Key>(-1)));
    EXPECT_FALSE(input_manager_->IsKeyPressed(static_cast<Key>(1000)));
}

TEST_F(InputTest, MouseButtonStates) {
    // Test mouse button enum values
    EXPECT_EQ(static_cast<int>(MouseButton::Left), 0);
    EXPECT_EQ(static_cast<int>(MouseButton::Right), 1);
    EXPECT_EQ(static_cast<int>(MouseButton::Middle), 2);
    
    // Test invalid button handling
    EXPECT_FALSE(input_manager_->IsMouseButtonPressed(static_cast<MouseButton>(-1)));
    EXPECT_FALSE(input_manager_->IsMouseButtonPressed(static_cast<MouseButton>(100)));
}

TEST_F(InputTest, GamepadInitialState) {
    // Test gamepad connection (likely no gamepads in test environment)
    EXPECT_FALSE(input_manager_->IsGamepadConnected(0));
    EXPECT_FALSE(input_manager_->IsGamepadConnected(-1));
    EXPECT_FALSE(input_manager_->IsGamepadConnected(100));
    
    // Test gamepad button states
    EXPECT_FALSE(input_manager_->IsGamepadButtonPressed(0, GamepadButton::A));
    EXPECT_FALSE(input_manager_->IsGamepadButtonJustPressed(0, GamepadButton::A));
    EXPECT_FALSE(input_manager_->IsGamepadButtonJustReleased(0, GamepadButton::A));
    
    // Test gamepad axis states
    EXPECT_EQ(input_manager_->GetGamepadAxis(0, GamepadAxis::LeftX), 0.0f);
    EXPECT_EQ(input_manager_->GetGamepadAxis(0, GamepadAxis::LeftY), 0.0f);
    
    // Test invalid gamepad ID
    EXPECT_EQ(input_manager_->GetGamepadAxis(-1, GamepadAxis::LeftX), 0.0f);
    EXPECT_EQ(input_manager_->GetGamepadAxis(100, GamepadAxis::LeftX), 0.0f);
}

TEST_F(InputTest, GamepadState) {
    auto& state = input_manager_->GetGamepadState(0);
    EXPECT_FALSE(state.connected);
    EXPECT_TRUE(state.name.empty());
    
    // Test all buttons are initially false
    for (size_t i = 0; i < state.buttons.size(); ++i) {
        EXPECT_FALSE(state.buttons[i]);
    }
    
    // Test all axes are initially zero
    for (size_t i = 0; i < state.axes.size(); ++i) {
        EXPECT_EQ(state.axes[i], 0.0f);
    }
}

TEST_F(InputTest, InputCallbacks) {
    bool callback_called = false;
    InputEventType last_event_type;
    
    input_manager_->SetInputCallback([&](const InputEvent& event) {
        callback_called = true;
        last_event_type = event.type;
    });
    
    // The callback system is set up, but in a test environment
    // we can't easily simulate actual input events
    // This test mainly verifies the callback can be set without issues
}

TEST_F(InputTest, MouseCursorControl) {
    // Test initial cursor visibility
    EXPECT_TRUE(input_manager_->IsMouseCursorVisible());
    
    // Test hiding cursor
    input_manager_->SetMouseCursorVisible(false);
    EXPECT_FALSE(input_manager_->IsMouseCursorVisible());
    
    // Test showing cursor
    input_manager_->SetMouseCursorVisible(true);
    EXPECT_TRUE(input_manager_->IsMouseCursorVisible());
    
    // Test cursor mode setting (GLFW constants)
    input_manager_->SetMouseCursorMode(0x00034001); // GLFW_CURSOR_NORMAL
    EXPECT_TRUE(input_manager_->IsMouseCursorVisible());
    
    input_manager_->SetMouseCursorMode(0x00034002); // GLFW_CURSOR_HIDDEN
    EXPECT_FALSE(input_manager_->IsMouseCursorVisible());
}

TEST_F(InputTest, Update) {
    // Test that Update() doesn't crash
    EXPECT_NO_THROW(input_manager_->Update());
    
    // Update multiple times
    input_manager_->Update();
    input_manager_->Update();
    input_manager_->Update();
    
    // State should remain consistent
    EXPECT_FALSE(input_manager_->IsKeyPressed(Key::A));
    EXPECT_FALSE(input_manager_->IsMouseButtonPressed(MouseButton::Left));
}

TEST_F(InputTest, MoveSemantics) {
    // Test move constructor
    InputManager input1(window_->GetNativeWindow());
    InputManager input2(std::move(input1));
    
    // Should be able to use moved-to object
    EXPECT_NO_THROW(input2.Update());
    EXPECT_FALSE(input2.IsKeyPressed(Key::A));
    
    // Test move assignment
    InputManager input3(window_->GetNativeWindow());
    input2 = std::move(input3);
    
    EXPECT_NO_THROW(input2.Update());
}

TEST_F(InputTest, EnumValues) {
    // Test some key enum values match GLFW
    EXPECT_EQ(static_cast<int>(Key::Space), 32);
    EXPECT_EQ(static_cast<int>(Key::A), 65);
    EXPECT_EQ(static_cast<int>(Key::Escape), 256);
    EXPECT_EQ(static_cast<int>(Key::F1), 290);
    
    // Test mouse button values
    EXPECT_EQ(static_cast<int>(MouseButton::Left), 0);
    EXPECT_EQ(static_cast<int>(MouseButton::Right), 1);
    EXPECT_EQ(static_cast<int>(MouseButton::Middle), 2);
    
    // Test gamepad button values
    EXPECT_EQ(static_cast<int>(GamepadButton::A), 0);
    EXPECT_EQ(static_cast<int>(GamepadButton::B), 1);
    EXPECT_EQ(static_cast<int>(GamepadButton::X), 2);
    EXPECT_EQ(static_cast<int>(GamepadButton::Y), 3);
    
    // Test gamepad axis values
    EXPECT_EQ(static_cast<int>(GamepadAxis::LeftX), 0);
    EXPECT_EQ(static_cast<int>(GamepadAxis::LeftY), 1);
    EXPECT_EQ(static_cast<int>(GamepadAxis::RightX), 2);
    EXPECT_EQ(static_cast<int>(GamepadAxis::RightY), 3);
}

TEST_F(InputTest, InputStateEnum) {
    // Test InputState enum values
    EXPECT_EQ(static_cast<int>(InputState::Released), 0);
    EXPECT_EQ(static_cast<int>(InputState::Pressed), 1);
    EXPECT_EQ(static_cast<int>(InputState::Held), 2);
}

TEST_F(InputTest, InvalidInput) {
    // Test null window handling
    EXPECT_THROW({
        InputManager invalid_input(nullptr);
    }, std::runtime_error);
}