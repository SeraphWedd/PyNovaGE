#pragma once

#include <vectors/vector2.hpp>
#include <memory>
#include <functional>
#include <array>
#include <string>

struct GLFWwindow;

namespace PyNovaGE {
namespace Window {

/**
 * @brief Key codes (mapped from GLFW)
 */
enum class Key {
    Unknown = -1,
    
    // Printable keys
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    
    Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    
    Semicolon = 59,
    Equal = 61,
    
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    LeftBracket = 91,
    Backslash = 92,
    RightBracket = 93,
    GraveAccent = 96,
    
    // Function keys
    Escape = 256,
    Enter, Tab, Backspace, Insert, Delete,
    Right, Left, Down, Up,
    PageUp, PageDown, Home, End,
    CapsLock = 280, ScrollLock, NumLock, PrintScreen, Pause,
    F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,
    
    // Keypad
    Keypad0 = 320, Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
    KeypadDecimal, KeypadDivide, KeypadMultiply, KeypadSubtract, KeypadAdd, KeypadEnter, KeypadEqual,
    
    // Modifiers
    LeftShift = 340, LeftControl, LeftAlt, LeftSuper,
    RightShift, RightControl, RightAlt, RightSuper,
    Menu
};

/**
 * @brief Mouse button codes
 */
enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7
};

/**
 * @brief Gamepad button codes
 */
enum class GamepadButton {
    A = 0, B, X, Y,
    LeftBumper, RightBumper,
    Back, Start, Guide,
    LeftThumb, RightThumb,
    DpadUp, DpadRight, DpadDown, DpadLeft
};

/**
 * @brief Gamepad axis codes
 */
enum class GamepadAxis {
    LeftX = 0, LeftY,
    RightX, RightY,
    LeftTrigger, RightTrigger
};

/**
 * @brief Input action state
 */
enum class InputState {
    Released = 0,
    Pressed = 1,
    Held = 2
};

/**
 * @brief Input event types
 */
enum class InputEventType {
    KeyPress,
    KeyRelease,
    MousePress,
    MouseRelease,
    MouseMove,
    MouseScroll,
    GamepadConnect,
    GamepadDisconnect,
    GamepadButtonPress,
    GamepadButtonRelease
};

/**
 * @brief Input event data
 */
struct InputEvent {
    InputEventType type;
    
    // Key/mouse data
    Key key = Key::Unknown;
    MouseButton mouse_button = MouseButton::Left;
    PyNovaGE::Vector2f mouse_position{0.0f, 0.0f};
    PyNovaGE::Vector2f scroll_offset{0.0f, 0.0f};
    
    // Gamepad data
    int gamepad_id = -1;
    GamepadButton gamepad_button = GamepadButton::A;
    
    // Modifier keys
    bool shift = false;
    bool control = false;
    bool alt = false;
    bool super = false;
};

/**
 * @brief Gamepad state
 */
struct GamepadState {
    bool connected = false;
    std::string name;
    std::array<bool, 15> buttons{};  // GamepadButton enum size
    std::array<float, 6> axes{};     // GamepadAxis enum size
};

/**
 * @brief Input manager for handling keyboard, mouse, and gamepad input
 */
class InputManager {
public:
    using InputCallback = std::function<void(const InputEvent&)>;
    
    /**
     * @brief Constructor
     */
    explicit InputManager(GLFWwindow* window);
    
    /**
     * @brief Destructor
     */
    ~InputManager();
    
    // Non-copyable but movable
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager(InputManager&&) noexcept;
    InputManager& operator=(InputManager&&) noexcept;
    
    /**
     * @brief Update input state (call once per frame)
     */
    void Update();
    
    /**
     * @brief Check if a key is currently pressed
     */
    bool IsKeyPressed(Key key) const;
    
    /**
     * @brief Check if a key was just pressed this frame
     */
    bool IsKeyJustPressed(Key key) const;
    
    /**
     * @brief Check if a key was just released this frame
     */
    bool IsKeyJustReleased(Key key) const;
    
    /**
     * @brief Check if a mouse button is currently pressed
     */
    bool IsMouseButtonPressed(MouseButton button) const;
    
    /**
     * @brief Check if a mouse button was just pressed this frame
     */
    bool IsMouseButtonJustPressed(MouseButton button) const;
    
    /**
     * @brief Check if a mouse button was just released this frame
     */
    bool IsMouseButtonJustReleased(MouseButton button) const;
    
    /**
     * @brief Get mouse position in window coordinates
     */
    PyNovaGE::Vector2f GetMousePosition() const;
    
    /**
     * @brief Get mouse delta since last frame
     */
    PyNovaGE::Vector2f GetMouseDelta() const;
    
    /**
     * @brief Get scroll delta this frame
     */
    PyNovaGE::Vector2f GetScrollDelta() const;
    
    /**
     * @brief Check if a gamepad is connected
     */
    bool IsGamepadConnected(int gamepad_id) const;
    
    /**
     * @brief Get gamepad state
     */
    const GamepadState& GetGamepadState(int gamepad_id) const;
    
    /**
     * @brief Check if a gamepad button is pressed
     */
    bool IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const;
    
    /**
     * @brief Check if a gamepad button was just pressed this frame
     */
    bool IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const;
    
    /**
     * @brief Check if a gamepad button was just released this frame
     */
    bool IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const;
    
    /**
     * @brief Get gamepad axis value (-1.0 to 1.0)
     */
    float GetGamepadAxis(int gamepad_id, GamepadAxis axis) const;
    
    /**
     * @brief Set input event callback
     */
    void SetInputCallback(InputCallback callback);
    
    /**
     * @brief Enable/disable mouse cursor
     */
    void SetMouseCursorVisible(bool visible);
    
    /**
     * @brief Check if mouse cursor is visible
     */
    bool IsMouseCursorVisible() const;
    
    /**
     * @brief Set mouse cursor mode (normal, hidden, disabled)
     */
    void SetMouseCursorMode(int mode); // GLFW_CURSOR_NORMAL, etc.

private:
    static constexpr int MAX_KEYS = 512;
    static constexpr int MAX_MOUSE_BUTTONS = 8;
    static constexpr int MAX_GAMEPADS = 16;
    
    GLFWwindow* window_;
    InputCallback input_callback_;
    
    // Input state tracking
    std::array<InputState, MAX_KEYS> key_states_{};
    std::array<InputState, MAX_KEYS> prev_key_states_{};
    
    std::array<InputState, MAX_MOUSE_BUTTONS> mouse_states_{};
    std::array<InputState, MAX_MOUSE_BUTTONS> prev_mouse_states_{};
    
    std::array<GamepadState, MAX_GAMEPADS> gamepad_states_{};
    std::array<GamepadState, MAX_GAMEPADS> prev_gamepad_states_{};
    
    PyNovaGE::Vector2f mouse_position_{0.0f, 0.0f};
    PyNovaGE::Vector2f prev_mouse_position_{0.0f, 0.0f};
    PyNovaGE::Vector2f scroll_delta_{0.0f, 0.0f};
    
    bool cursor_visible_ = true;
    
    // GLFW callbacks
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void JoystickCallback(int jid, int event);
    
    void SetupCallbacks();
    void UpdateGamepads();
    void TriggerEvent(const InputEvent& event);
    InputState GetInputState(bool current, bool previous) const;
};

} // namespace Window
} // namespace PyNovaGE