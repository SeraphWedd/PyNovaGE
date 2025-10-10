#include "window/input.hpp"
#include "window/input_platform.hpp"
#include <stdexcept>

namespace PyNovaGE {
namespace Window {

// InputManager implementation
InputManager::InputManager(GLFWwindow* window) {
    if (!window) {
        throw std::runtime_error("InputManager requires a valid GLFW window");
    }
    
    // Create the platform-specific input backend
    platform_ = CreateInputPlatform(window);
    if (!platform_) {
        throw std::runtime_error("Failed to create input platform");
    }
    
    // Initialize the platform
    if (!platform_->Initialize()) {
        throw std::runtime_error("Failed to initialize input platform");
    }
}

InputManager::~InputManager() {
    if (platform_) {
        platform_->Shutdown();
    }
}

InputManager::InputManager(InputManager&& other) noexcept
    : platform_(std::move(other.platform_))
    , input_callback_(std::move(other.input_callback_))
    , touch_callback_(std::move(other.touch_callback_)) {
}

InputManager& InputManager::operator=(InputManager&& other) noexcept {
    if (this != &other) {
        if (platform_) {
            platform_->Shutdown();
        }
        platform_ = std::move(other.platform_);
        input_callback_ = std::move(other.input_callback_);
        touch_callback_ = std::move(other.touch_callback_);
    }
    return *this;
}

void InputManager::Update() {
    if (platform_) {
        platform_->Update();
    }
}

bool InputManager::IsKeyPressed(Key key) const {
    return platform_ ? platform_->IsKeyPressed(key) : false;
}

bool InputManager::IsKeyJustPressed(Key key) const {
    return platform_ ? platform_->IsKeyJustPressed(key) : false;
}

bool InputManager::IsKeyJustReleased(Key key) const {
    return platform_ ? platform_->IsKeyJustReleased(key) : false;
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    return platform_ ? platform_->IsMouseButtonPressed(button) : false;
}

bool InputManager::IsMouseButtonJustPressed(MouseButton button) const {
    return platform_ ? platform_->IsMouseButtonJustPressed(button) : false;
}

bool InputManager::IsMouseButtonJustReleased(MouseButton button) const {
    return platform_ ? platform_->IsMouseButtonJustReleased(button) : false;
}

PyNovaGE::Vector2f InputManager::GetMousePosition() const {
    return platform_ ? platform_->GetMousePosition() : PyNovaGE::Vector2f{0.0f, 0.0f};
}

PyNovaGE::Vector2f InputManager::GetMouseDelta() const {
    return platform_ ? platform_->GetMouseDelta() : PyNovaGE::Vector2f{0.0f, 0.0f};
}

PyNovaGE::Vector2f InputManager::GetScrollDelta() const {
    return platform_ ? platform_->GetScrollDelta() : PyNovaGE::Vector2f{0.0f, 0.0f};
}

bool InputManager::IsGamepadConnected(int gamepad_id) const {
    return platform_ ? platform_->IsGamepadConnected(gamepad_id) : false;
}

const GamepadState& InputManager::GetGamepadState(int gamepad_id) const {
    static GamepadState empty_state{};
    return platform_ ? platform_->GetGamepadState(gamepad_id) : empty_state;
}

bool InputManager::IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const {
    return platform_ ? platform_->IsGamepadButtonPressed(gamepad_id, button) : false;
}

bool InputManager::IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const {
    return platform_ ? platform_->IsGamepadButtonJustPressed(gamepad_id, button) : false;
}

bool InputManager::IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const {
    return platform_ ? platform_->IsGamepadButtonJustReleased(gamepad_id, button) : false;
}

float InputManager::GetGamepadAxis(int gamepad_id, GamepadAxis axis) const {
    return platform_ ? platform_->GetGamepadAxis(gamepad_id, axis) : 0.0f;
}

void InputManager::SetInputCallback(InputCallback callback) {
    input_callback_ = std::move(callback);
    if (platform_) {
        platform_->SetInputCallback(input_callback_);
    }
}

void InputManager::SetTouchCallback(TouchCallback callback) {
    touch_callback_ = std::move(callback);
    if (platform_) {
        platform_->SetTouchCallback(touch_callback_);
    }
}

void InputManager::SetMouseCursorVisible(bool visible) {
    if (platform_) {
        platform_->SetMouseCursorVisible(visible);
    }
}

bool InputManager::IsMouseCursorVisible() const {
    return platform_ ? platform_->IsMouseCursorVisible() : false;
}

void InputManager::SetMouseCursorMode(int mode) {
    if (platform_) {
        platform_->SetMouseCursorMode(mode);
    }
}

bool InputManager::SupportsTouchInput() const {
    return platform_ ? platform_->SupportsTouchInput() : false;
}

int InputManager::GetActiveTouchCount() const {
    return platform_ ? platform_->GetActiveTouchCount() : 0;
}

TouchData InputManager::GetTouch(int touch_id) const {
    return platform_ ? platform_->GetTouch(touch_id) : TouchData{};
}

std::vector<TouchData> InputManager::GetActiveTouches() const {
    return platform_ ? platform_->GetActiveTouches() : std::vector<TouchData>{};
}

bool InputManager::SupportsKeyboard() const {
    return platform_ ? platform_->SupportsKeyboard() : false;
}

bool InputManager::SupportsMouse() const {
    return platform_ ? platform_->SupportsMouse() : false;
}

bool InputManager::SupportsGamepad() const {
    return platform_ ? platform_->SupportsGamepad() : false;
}

std::string InputManager::GetPlatformName() const {
    return platform_ ? platform_->GetPlatformName() : "Unknown";
}

} // namespace Window
} // namespace PyNovaGE
