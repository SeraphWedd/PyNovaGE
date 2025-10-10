#include "window/input.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace PyNovaGE {
namespace Window {

// InputManager implementation
InputManager::InputManager(GLFWwindow* window) : window_(window) {
    if (!window_) {
        throw std::runtime_error("InputManager requires a valid GLFW window");
    }
    
    // Store this instance in the window user pointer for callbacks
    glfwSetWindowUserPointer(window_, this);
    SetupCallbacks();
    
    // Initialize input states
    key_states_.fill(InputState::Released);
    prev_key_states_.fill(InputState::Released);
    mouse_states_.fill(InputState::Released);
    prev_mouse_states_.fill(InputState::Released);
    
    // Get initial mouse position
    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);
    mouse_position_ = {static_cast<float>(xpos), static_cast<float>(ypos)};
    prev_mouse_position_ = mouse_position_;
}

InputManager::~InputManager() = default;

InputManager::InputManager(InputManager&& other) noexcept
    : window_(other.window_)
    , input_callback_(std::move(other.input_callback_))
    , key_states_(std::move(other.key_states_))
    , prev_key_states_(std::move(other.prev_key_states_))
    , mouse_states_(std::move(other.mouse_states_))
    , prev_mouse_states_(std::move(other.prev_mouse_states_))
    , gamepad_states_(std::move(other.gamepad_states_))
    , prev_gamepad_states_(std::move(other.prev_gamepad_states_))
    , mouse_position_(other.mouse_position_)
    , prev_mouse_position_(other.prev_mouse_position_)
    , scroll_delta_(other.scroll_delta_)
    , cursor_visible_(other.cursor_visible_) {
    
    other.window_ = nullptr;
    
    if (window_) {
        glfwSetWindowUserPointer(window_, this);
    }
}

InputManager& InputManager::operator=(InputManager&& other) noexcept {
    if (this != &other) {
        window_ = other.window_;
        input_callback_ = std::move(other.input_callback_);
        key_states_ = std::move(other.key_states_);
        prev_key_states_ = std::move(other.prev_key_states_);
        mouse_states_ = std::move(other.mouse_states_);
        prev_mouse_states_ = std::move(other.prev_mouse_states_);
        gamepad_states_ = std::move(other.gamepad_states_);
        prev_gamepad_states_ = std::move(other.prev_gamepad_states_);
        mouse_position_ = other.mouse_position_;
        prev_mouse_position_ = other.prev_mouse_position_;
        scroll_delta_ = other.scroll_delta_;
        cursor_visible_ = other.cursor_visible_;
        
        other.window_ = nullptr;
        
        if (window_) {
            glfwSetWindowUserPointer(window_, this);
        }
    }
    return *this;
}

void InputManager::Update() {
    // Store previous states
    prev_key_states_ = key_states_;
    prev_mouse_states_ = mouse_states_;
    prev_gamepad_states_ = gamepad_states_;
    prev_mouse_position_ = mouse_position_;
    
    // Update current states - only check valid GLFW keys to avoid "Invalid key" errors
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

bool InputManager::IsKeyPressed(Key key) const {
    int keycode = static_cast<int>(key);
    if (keycode < 0 || keycode >= MAX_KEYS) return false;
    return key_states_[keycode] == InputState::Pressed || key_states_[keycode] == InputState::Held;
}

bool InputManager::IsKeyJustPressed(Key key) const {
    int keycode = static_cast<int>(key);
    if (keycode < 0 || keycode >= MAX_KEYS) return false;
    return key_states_[keycode] == InputState::Pressed;
}

bool InputManager::IsKeyJustReleased(Key key) const {
    int keycode = static_cast<int>(key);
    if (keycode < 0 || keycode >= MAX_KEYS) return false;
    return key_states_[keycode] == InputState::Released && 
           (prev_key_states_[keycode] == InputState::Pressed || prev_key_states_[keycode] == InputState::Held);
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= MAX_MOUSE_BUTTONS) return false;
    return mouse_states_[btn] == InputState::Pressed || mouse_states_[btn] == InputState::Held;
}

bool InputManager::IsMouseButtonJustPressed(MouseButton button) const {
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= MAX_MOUSE_BUTTONS) return false;
    return mouse_states_[btn] == InputState::Pressed;
}

bool InputManager::IsMouseButtonJustReleased(MouseButton button) const {
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= MAX_MOUSE_BUTTONS) return false;
    return mouse_states_[btn] == InputState::Released && 
           (prev_mouse_states_[btn] == InputState::Pressed || prev_mouse_states_[btn] == InputState::Held);
}

PyNovaGE::Vector2f InputManager::GetMousePosition() const {
    return mouse_position_;
}

PyNovaGE::Vector2f InputManager::GetMouseDelta() const {
    return mouse_position_ - prev_mouse_position_;
}

PyNovaGE::Vector2f InputManager::GetScrollDelta() const {
    return scroll_delta_;
}

bool InputManager::IsGamepadConnected(int gamepad_id) const {
    if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return false;
    return gamepad_states_[gamepad_id].connected;
}

const GamepadState& InputManager::GetGamepadState(int gamepad_id) const {
    static GamepadState empty_state{};
    if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return empty_state;
    return gamepad_states_[gamepad_id];
}

bool InputManager::IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const {
    if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return false;
    if (!gamepad_states_[gamepad_id].connected) return false;
    
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= static_cast<int>(gamepad_states_[gamepad_id].buttons.size())) return false;
    
    return gamepad_states_[gamepad_id].buttons[btn];
}

bool InputManager::IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const {
    if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return false;
    if (!gamepad_states_[gamepad_id].connected) return false;
    
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= static_cast<int>(gamepad_states_[gamepad_id].buttons.size())) return false;
    
    return gamepad_states_[gamepad_id].buttons[btn] && !prev_gamepad_states_[gamepad_id].buttons[btn];
}

bool InputManager::IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const {
    if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return false;
    if (!gamepad_states_[gamepad_id].connected) return false;
    
    int btn = static_cast<int>(button);
    if (btn < 0 || btn >= static_cast<int>(gamepad_states_[gamepad_id].buttons.size())) return false;
    
    return !gamepad_states_[gamepad_id].buttons[btn] && prev_gamepad_states_[gamepad_id].buttons[btn];
}

float InputManager::GetGamepadAxis(int gamepad_id, GamepadAxis axis) const {
    if (gamepad_id < 0 || gamepad_id >= MAX_GAMEPADS) return 0.0f;
    if (!gamepad_states_[gamepad_id].connected) return 0.0f;
    
    int ax = static_cast<int>(axis);
    if (ax < 0 || ax >= static_cast<int>(gamepad_states_[gamepad_id].axes.size())) return 0.0f;
    
    return gamepad_states_[gamepad_id].axes[ax];
}

void InputManager::SetInputCallback(InputCallback callback) {
    input_callback_ = std::move(callback);
}

void InputManager::SetMouseCursorVisible(bool visible) {
    if (window_) {
        glfwSetInputMode(window_, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
        cursor_visible_ = visible;
    }
}

bool InputManager::IsMouseCursorVisible() const {
    return cursor_visible_;
}

void InputManager::SetMouseCursorMode(int mode) {
    if (window_) {
        glfwSetInputMode(window_, GLFW_CURSOR, mode);
        cursor_visible_ = (mode == GLFW_CURSOR_NORMAL);
    }
}

void InputManager::SetupCallbacks() {
    if (!window_) return;
    
    glfwSetKeyCallback(window_, KeyCallback);
    glfwSetMouseButtonCallback(window_, MouseButtonCallback);
    glfwSetCursorPosCallback(window_, CursorPositionCallback);
    glfwSetScrollCallback(window_, ScrollCallback);
    glfwSetJoystickCallback(JoystickCallback);
}

void InputManager::UpdateGamepads() {
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

void InputManager::TriggerEvent(const InputEvent& event) {
    if (input_callback_) {
        input_callback_(event);
    }
}

InputState InputManager::GetInputState(bool current, bool previous) const {
    if (current && previous) {
        return InputState::Held;
    } else if (current && !previous) {
        return InputState::Pressed;
    } else {
        return InputState::Released;
    }
}

// Static GLFW callbacks
void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; // Unused parameter
    
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
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

void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
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

void InputManager::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (input) {
        InputEvent event{};
        event.type = InputEventType::MouseMove;
        event.mouse_position = {static_cast<float>(xpos), static_cast<float>(ypos)};
        input->TriggerEvent(event);
    }
}

void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (input) {
        input->scroll_delta_ = {static_cast<float>(xoffset), static_cast<float>(yoffset)};
        
        InputEvent event{};
        event.type = InputEventType::MouseScroll;
        event.scroll_offset = input->scroll_delta_;
        event.mouse_position = input->GetMousePosition();
        input->TriggerEvent(event);
    }
}

void InputManager::JoystickCallback(int jid, int event) {
    // This callback is triggered when gamepads are connected/disconnected
    // The actual state update happens in UpdateGamepads()
    (void)jid;  // Unused parameters
    (void)event;
}

} // namespace Window
} // namespace PyNovaGE