#include "window/input_platform.hpp"
#include "window/input.hpp"
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <algorithm>
#include <cstring>

namespace PyNovaGE {
namespace Window {

// Constants
static constexpr int MAX_KEYS = 512;
static constexpr int MAX_MOUSE_BUTTONS = 8;
static constexpr int MAX_GAMEPADS = 16;

// DesktopInputPlatform implementation
class DesktopInputPlatform::DesktopInputImpl {
public:
    explicit DesktopInputImpl(GLFWwindow* window) : window_(window) {
        if (window_) {
            // Store this instance in the window user pointer for callbacks
            glfwSetWindowUserPointer(window_, this);
        }
        
        // Initialize input states
        key_states_.fill(InputState::Released);
        prev_key_states_.fill(InputState::Released);
        mouse_states_.fill(InputState::Released);
        prev_mouse_states_.fill(InputState::Released);
    }
    
    bool Initialize() {
        if (!window_) return false;
        
        // Set up GLFW callbacks
        glfwSetKeyCallback(window_, KeyCallback);
        glfwSetMouseButtonCallback(window_, MouseButtonCallback);
        glfwSetCursorPosCallback(window_, CursorPositionCallback);
        glfwSetScrollCallback(window_, ScrollCallback);
        glfwSetJoystickCallback(JoystickCallback);
        
        // Get initial mouse position
        double xpos, ypos;
        glfwGetCursorPos(window_, &xpos, &ypos);
        mouse_position_ = {static_cast<float>(xpos), static_cast<float>(ypos)};
        prev_mouse_position_ = mouse_position_;
        
        return true;
    }
    
    void Shutdown() {
        if (window_) {
            glfwSetKeyCallback(window_, nullptr);
            glfwSetMouseButtonCallback(window_, nullptr);
            glfwSetCursorPosCallback(window_, nullptr);
            glfwSetScrollCallback(window_, nullptr);
        }
    }
    
    void Update() {
        if (!window_) return;
        
        // Store previous states
        prev_key_states_ = key_states_;
        prev_mouse_states_ = mouse_states_;
        prev_gamepad_states_ = gamepad_states_;
        prev_mouse_position_ = mouse_position_;
        
        // Update keyboard state - only check valid GLFW keys
        static const std::vector<int> valid_keys = {
            // Printable keys
            32,  // Space
            39,  // Apostrophe
            44,  // Comma
            45,  // Minus
            46,  // Period
            47,  // Slash
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57,  // 0-9
            59,  // Semicolon
            61,  // Equal
            65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,  // A-Z
            91,  // Left bracket
            92,  // Backslash
            93,  // Right bracket
            96,  // Grave accent
            
            // Function keys
            256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268,  // Escape, Enter, Tab, Backspace, Insert, Delete, Right, Left, Down, Up, PageUp, PageDown, Home, End
            280, 281, 282, 283, 284,  // CapsLock, ScrollLock, NumLock, PrintScreen, Pause
            290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301,  // F1-F12
            302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314,  // F13-F25
            
            // Keypad
            320, 321, 322, 323, 324, 325, 326, 327, 328, 329,  // Keypad 0-9
            330, 331, 332, 333, 334, 335, 336,  // KeypadDecimal, Divide, Multiply, Subtract, Add, Enter, Equal
            
            // Modifiers
            340, 341, 342, 343, 344, 345, 346, 347, 348  // LeftShift, LeftControl, LeftAlt, LeftSuper, RightShift, RightControl, RightAlt, RightSuper, Menu
        };
        
        for (int keycode : valid_keys) {
            int state = glfwGetKey(window_, keycode);
            bool current = (state == GLFW_PRESS);
            bool previous = (prev_key_states_[keycode] == InputState::Pressed || prev_key_states_[keycode] == InputState::Held);
            key_states_[keycode] = GetInputState(current, previous);
        }
        
        // Update mouse button states
        for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
            int state = glfwGetMouseButton(window_, i);
            bool current = (state == GLFW_PRESS);
            bool previous = (prev_mouse_states_[i] == InputState::Pressed || prev_mouse_states_[i] == InputState::Held);
            mouse_states_[i] = GetInputState(current, previous);
        }
        
        // Update mouse position
        double xpos, ypos;
        glfwGetCursorPos(window_, &xpos, &ypos);
        mouse_position_ = {static_cast<float>(xpos), static_cast<float>(ypos)};
        
        // Update gamepads
        UpdateGamepads();
        
        // Reset scroll delta (it's set by the callback)
        scroll_delta_ = {0.0f, 0.0f};
    }
    
    void SetInputCallback(InputCallback callback) {
        input_callback_ = std::move(callback);
    }
    
    bool IsKeyPressed(Key key) const {
        int keycode = static_cast<int>(key);
        if (keycode < 0 || keycode >= MAX_KEYS) return false;
        return key_states_[keycode] == InputState::Pressed || key_states_[keycode] == InputState::Held;
    }
    
    bool IsKeyJustPressed(Key key) const {
        int keycode = static_cast<int>(key);
        if (keycode < 0 || keycode >= MAX_KEYS) return false;
        return key_states_[keycode] == InputState::Pressed;
    }
    
    bool IsKeyJustReleased(Key key) const {
        int keycode = static_cast<int>(key);
        if (keycode < 0 || keycode >= MAX_KEYS) return false;
        return key_states_[keycode] == InputState::Released && 
               (prev_key_states_[keycode] == InputState::Pressed || prev_key_states_[keycode] == InputState::Held);
    }
    
    bool IsMouseButtonPressed(MouseButton button) const {
        int btn = static_cast<int>(button);
        if (btn < 0 || btn >= MAX_MOUSE_BUTTONS) return false;
        return mouse_states_[btn] == InputState::Pressed || mouse_states_[btn] == InputState::Held;
    }
    
    bool IsMouseButtonJustPressed(MouseButton button) const {
        int btn = static_cast<int>(button);
        if (btn < 0 || btn >= MAX_MOUSE_BUTTONS) return false;
        return mouse_states_[btn] == InputState::Pressed;
    }
    
    bool IsMouseButtonJustReleased(MouseButton button) const {
        int btn = static_cast<int>(button);
        if (btn < 0 || btn >= MAX_MOUSE_BUTTONS) return false;
        return mouse_states_[btn] == InputState::Released && 
               (prev_mouse_states_[btn] == InputState::Pressed || prev_mouse_states_[btn] == InputState::Held);
    }
    
    PyNovaGE::Vector2f GetMousePosition() const {
        return mouse_position_;
    }
    
    PyNovaGE::Vector2f GetMouseDelta() const {
        return mouse_position_ - prev_mouse_position_;
    }
    
    PyNovaGE::Vector2f GetScrollDelta() const {
        return scroll_delta_;
    }
    
    void SetMouseCursorVisible(bool visible) {
        if (window_) {
            glfwSetInputMode(window_, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
            cursor_visible_ = visible;
        }
    }
    
    bool IsMouseCursorVisible() const {
        return cursor_visible_;
    }
    
    void SetMouseCursorMode(int mode) {
        if (window_) {
            glfwSetInputMode(window_, GLFW_CURSOR, mode);
            cursor_visible_ = (mode == GLFW_CURSOR_NORMAL);
        }
    }
    
    bool IsGamepadConnected(int gamepad_id) const {
        if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return false;
        return gamepad_states_[gamepad_id].connected;
    }
    
    const GamepadState& GetGamepadState(int gamepad_id) const {
        if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return empty_state_;
        return gamepad_states_[gamepad_id];
    }
    
    bool IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const {
        if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return false;
        if (!gamepad_states_[gamepad_id].connected) return false;
        
        int btn = static_cast<int>(button);
        if (btn < 0 || btn >= static_cast<int>(gamepad_states_[gamepad_id].buttons.size())) return false;
        
        return gamepad_states_[gamepad_id].buttons[btn];
    }
    
    bool IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const {
        if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return false;
        if (!gamepad_states_[gamepad_id].connected) return false;
        
        int btn = static_cast<int>(button);
        if (btn < 0 || btn >= static_cast<int>(gamepad_states_[gamepad_id].buttons.size())) return false;
        
        return gamepad_states_[gamepad_id].buttons[btn] && !prev_gamepad_states_[gamepad_id].buttons[btn];
    }
    
    bool IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const {
        if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return false;
        if (!gamepad_states_[gamepad_id].connected) return false;
        
        int btn = static_cast<int>(button);
        if (btn < 0 || btn >= static_cast<int>(gamepad_states_[gamepad_id].buttons.size())) return false;
        
        return !gamepad_states_[gamepad_id].buttons[btn] && prev_gamepad_states_[gamepad_id].buttons[btn];
    }
    
    float GetGamepadAxis(int gamepad_id, GamepadAxis axis) const {
        if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return 0.0f;
        if (!gamepad_states_[gamepad_id].connected) return 0.0f;
        
        int ax = static_cast<int>(axis);
        if (ax < 0 || ax >= static_cast<int>(gamepad_states_[gamepad_id].axes.size())) return 0.0f;
        
        return gamepad_states_[gamepad_id].axes[ax];
    }

private:
    GLFWwindow* window_;
    InputCallback input_callback_;
    
    // Input states
    std::array<InputState, MAX_KEYS> key_states_{};
    std::array<InputState, MAX_KEYS> prev_key_states_{};
    std::array<InputState, MAX_MOUSE_BUTTONS> mouse_states_{};
    std::array<InputState, MAX_MOUSE_BUTTONS> prev_mouse_states_{};
    std::array<GamepadState, MAX_GAMEPADS> gamepad_states_{};
    std::array<GamepadState, MAX_GAMEPADS> prev_gamepad_states_{};
    
    // Mouse state
    PyNovaGE::Vector2f mouse_position_{0.0f, 0.0f};
    PyNovaGE::Vector2f prev_mouse_position_{0.0f, 0.0f};
    PyNovaGE::Vector2f scroll_delta_{0.0f, 0.0f};
    bool cursor_visible_ = true;
    
    // Empty state for invalid gamepad queries
    GamepadState empty_state_{};
    
    // Helper methods
    InputState GetInputState(bool current, bool previous) const {
        if (current && previous) {
            return InputState::Held;
        } else if (current && !previous) {
            return InputState::Pressed;
        } else {
            return InputState::Released;
        }
    }
    
    void UpdateGamepads() {
        for (int i = 0; i < MAX_GAMEPADS; ++i) {
            bool connected = glfwJoystickPresent(i) == GLFW_TRUE;
            gamepad_states_[i].connected = connected;
            
            if (connected) {
                // Get gamepad name
                const char* name = glfwGetJoystickName(i);
                if (name) {
                    gamepad_states_[i].name = name;
                }
                
                // Check if it's a gamepad (has mapping)
                if (glfwJoystickIsGamepad(i)) {
                    GLFWgamepadstate state;
                    if (glfwGetGamepadState(i, &state)) {
                        // Copy button states
                        for (int j = 0; j < static_cast<int>(gamepad_states_[i].buttons.size()) && j < 15; ++j) {
                            gamepad_states_[i].buttons[j] = (state.buttons[j] == GLFW_PRESS);
                        }
                        
                        // Copy axis states
                        for (int j = 0; j < static_cast<int>(gamepad_states_[i].axes.size()) && j < 6; ++j) {
                            gamepad_states_[i].axes[j] = state.axes[j];
                        }
                    }
                }
            }
        }
    }
    
    void TriggerEvent(const InputEvent& event) {
        if (input_callback_) {
            input_callback_(event);
        }
    }
    
    // Static GLFW callbacks
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        (void)scancode; // Unused parameter
        
        DesktopInputImpl* input = static_cast<DesktopInputImpl*>(glfwGetWindowUserPointer(window));
        if (input) {
            InputEvent event{};
            event.key = static_cast<Key>(key);
            event.shift = (mods & GLFW_MOD_SHIFT) != 0;
            event.control = (mods & GLFW_MOD_CONTROL) != 0;
            event.alt = (mods & GLFW_MOD_ALT) != 0;
            event.super = (mods & GLFW_MOD_SUPER) != 0;
            
            if (action == GLFW_PRESS) {
                event.type = InputEventType::KeyPress;
            } else if (action == GLFW_RELEASE) {
                event.type = InputEventType::KeyRelease;
            }
            
            if (action != GLFW_REPEAT) {  // Don't send repeat events
                input->TriggerEvent(event);
            }
        }
    }
    
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        DesktopInputImpl* input = static_cast<DesktopInputImpl*>(glfwGetWindowUserPointer(window));
        if (input) {
            InputEvent event{};
            event.mouse_button = static_cast<MouseButton>(button);
            event.mouse_position = input->GetMousePosition();
            event.shift = (mods & GLFW_MOD_SHIFT) != 0;
            event.control = (mods & GLFW_MOD_CONTROL) != 0;
            event.alt = (mods & GLFW_MOD_ALT) != 0;
            event.super = (mods & GLFW_MOD_SUPER) != 0;
            
            if (action == GLFW_PRESS) {
                event.type = InputEventType::MousePress;
            } else if (action == GLFW_RELEASE) {
                event.type = InputEventType::MouseRelease;
            }
            
            input->TriggerEvent(event);
        }
    }
    
    static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
        DesktopInputImpl* input = static_cast<DesktopInputImpl*>(glfwGetWindowUserPointer(window));
        if (input) {
            InputEvent event{};
            event.type = InputEventType::MouseMove;
            event.mouse_position = {static_cast<float>(xpos), static_cast<float>(ypos)};
            input->TriggerEvent(event);
        }
    }
    
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        DesktopInputImpl* input = static_cast<DesktopInputImpl*>(glfwGetWindowUserPointer(window));
        if (input) {
            input->scroll_delta_ = {static_cast<float>(xoffset), static_cast<float>(yoffset)};
            
            InputEvent event{};
            event.type = InputEventType::MouseScroll;
            event.scroll_offset = input->scroll_delta_;
            event.mouse_position = input->GetMousePosition();
            input->TriggerEvent(event);
        }
    }
    
    static void JoystickCallback(int jid, int event) {
        // This callback is triggered when gamepads are connected/disconnected
        // The actual state update happens in UpdateGamepads()
        (void)jid;  // Unused parameters
        (void)event;
    }
};

// DesktopInputPlatform implementation
DesktopInputPlatform::DesktopInputPlatform(void* native_window) 
    : impl_(std::make_unique<DesktopInputImpl>(static_cast<GLFWwindow*>(native_window))) {
}

DesktopInputPlatform::~DesktopInputPlatform() = default;

bool DesktopInputPlatform::Initialize() {
    return impl_->Initialize();
}

void DesktopInputPlatform::Shutdown() {
    impl_->Shutdown();
}

void DesktopInputPlatform::Update() {
    impl_->Update();
}

void DesktopInputPlatform::SetInputCallback(InputCallback callback) {
    impl_->SetInputCallback(std::move(callback));
}

void DesktopInputPlatform::SetTouchCallback(TouchCallback callback) {
    // Desktop doesn't support touch, so just ignore
    (void)callback;
}

bool DesktopInputPlatform::IsKeyPressed(Key key) const {
    return impl_->IsKeyPressed(key);
}

bool DesktopInputPlatform::IsKeyJustPressed(Key key) const {
    return impl_->IsKeyJustPressed(key);
}

bool DesktopInputPlatform::IsKeyJustReleased(Key key) const {
    return impl_->IsKeyJustReleased(key);
}

bool DesktopInputPlatform::IsMouseButtonPressed(MouseButton button) const {
    return impl_->IsMouseButtonPressed(button);
}

bool DesktopInputPlatform::IsMouseButtonJustPressed(MouseButton button) const {
    return impl_->IsMouseButtonJustPressed(button);
}

bool DesktopInputPlatform::IsMouseButtonJustReleased(MouseButton button) const {
    return impl_->IsMouseButtonJustReleased(button);
}

PyNovaGE::Vector2f DesktopInputPlatform::GetMousePosition() const {
    return impl_->GetMousePosition();
}

PyNovaGE::Vector2f DesktopInputPlatform::GetMouseDelta() const {
    return impl_->GetMouseDelta();
}

PyNovaGE::Vector2f DesktopInputPlatform::GetScrollDelta() const {
    return impl_->GetScrollDelta();
}

void DesktopInputPlatform::SetMouseCursorVisible(bool visible) {
    impl_->SetMouseCursorVisible(visible);
}

bool DesktopInputPlatform::IsMouseCursorVisible() const {
    return impl_->IsMouseCursorVisible();
}

void DesktopInputPlatform::SetMouseCursorMode(int mode) {
    impl_->SetMouseCursorMode(mode);
}

TouchData DesktopInputPlatform::GetTouch(int touch_id) const {
    (void)touch_id;
    return TouchData{}; // Desktop doesn't support touch
}

bool DesktopInputPlatform::IsGamepadConnected(int gamepad_id) const {
    return impl_->IsGamepadConnected(gamepad_id);
}

const GamepadState& DesktopInputPlatform::GetGamepadState(int gamepad_id) const {
    return impl_->GetGamepadState(gamepad_id);
}

bool DesktopInputPlatform::IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const {
    return impl_->IsGamepadButtonPressed(gamepad_id, button);
}

bool DesktopInputPlatform::IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const {
    return impl_->IsGamepadButtonJustPressed(gamepad_id, button);
}

bool DesktopInputPlatform::IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const {
    return impl_->IsGamepadButtonJustReleased(gamepad_id, button);
}

float DesktopInputPlatform::GetGamepadAxis(int gamepad_id, GamepadAxis axis) const {
    return impl_->GetGamepadAxis(gamepad_id, axis);
}

// Factory function
std::unique_ptr<IInputPlatform> CreateInputPlatform(void* native_handle) {
#ifdef PYNOVAGE_PLATFORM_ANDROID
    return std::make_unique<AndroidInputPlatform>(native_handle);
#else
    // Default to desktop platform
    return std::make_unique<DesktopInputPlatform>(native_handle);
#endif
}

} // namespace Window  
} // namespace PyNovaGE